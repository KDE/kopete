/*
    kopetemetacontact.cpp - Kopete Meta Contact

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart@ kde.org>
    Copyright (c) 2002-2004 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetemetacontact.h"

#include <kapplication.h>

#include <kabc/addressbook.h>
#include <kabc/addressee.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdeversion.h>

#include "kabcpersistence.h"
#include "kopetecontactlist.h"
#include "kopetecontact.h"
#include "kopeteaccountmanager.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetepluginmanager.h"
#include "kopetegroup.h"
#include "kopeteglobal.h"
#include "kopeteprefs.h"
#include "kopeteuiglobal.h"
//Added by qt3to4:
#include <QPixmap>
#include <Q3ValueList>
#include <Q3PtrList>

namespace Kopete {

const QString NSCID_ELEM = QString::fromLatin1( "nameSourceContactId" );
const QString NSPID_ELEM = QString::fromLatin1( "nameSourcePluginId" );
const QString NSAID_ELEM = QString::fromLatin1( "nameSourceAccountId" );
const QString PSCID_ELEM = QString::fromLatin1( "photoSourceContactId" );
const QString PSPID_ELEM = QString::fromLatin1( "photoSourcePluginId" );
const QString PSAID_ELEM = QString::fromLatin1( "photoSourceAccountId" );

class  MetaContact::Private
{ public:

	Q3PtrList<Contact> contacts;
	QString displayName;
	QString nameSourceCID;
	QString nameSourcePID;
	QString nameSourceAID;
	QString photoSourceCID;
	QString photoSourcePID;
	QString photoSourceAID;
	Q3PtrList<Group> groups;
	QMap<QString, QMap<QString, QString> > addressBook;
	bool temporary;
	QString metaContactId;
	OnlineStatus::StatusType onlineStatus;
	bool photoSyncedWithKABC;
};

MetaContact::MetaContact()
	: ContactListElement( ContactList::self() )
{
	d = new Private;

	setNameSource( 0 );
	setPhotoSource( 0 );
	d->temporary = false;
	d->photoSyncedWithKABC=false;

	d->onlineStatus = Kopete::OnlineStatus::Offline;

	connect( this, SIGNAL( pluginDataChanged() ), SIGNAL( persistentDataChanged() ) );
	connect( this, SIGNAL( iconChanged( Kopete::ContactListElement::IconState, const QString & ) ), SIGNAL( persistentDataChanged() ) );
	connect( this, SIGNAL( useCustomIconChanged( bool ) ), SIGNAL( persistentDataChanged() ) );
	connect( this, SIGNAL( displayNameChanged( const QString &, const QString & ) ), SIGNAL( persistentDataChanged() ) );
	connect( this, SIGNAL( movedToGroup( Kopete::MetaContact *, Kopete::Group *, Kopete::Group * ) ), SIGNAL( persistentDataChanged() ) );
	connect( this, SIGNAL( removedFromGroup( Kopete::MetaContact *, Kopete::Group * ) ), SIGNAL( persistentDataChanged() ) );
	connect( this, SIGNAL( addedToGroup( Kopete::MetaContact *, Kopete::Group * ) ), SIGNAL( persistentDataChanged() ) );
	connect( this, SIGNAL( contactAdded( Kopete::Contact * ) ), SIGNAL( persistentDataChanged() ) );
	connect( this, SIGNAL( contactRemoved( Kopete::Contact * ) ), SIGNAL( persistentDataChanged() ) );


	// make sure MetaContact is at least in one group
	addToGroup( Group::topLevel() );
			 //i'm not sure this is correct -Olivier
			 // we probably should do the check in groups() instead
}

MetaContact::~MetaContact()
{
	delete d;
}


void MetaContact::addContact( Contact *c )
{
	if( d->contacts.contains( c ) )
	{
		kdWarning(14010) << "Ignoring attempt to add duplicate contact " << c->contactId() << "!" << endl;
	}
	else
	{
		d->contacts.append( c );

		connect( c, SIGNAL( onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ),
			SLOT( slotContactStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ) );

		connect( c, SIGNAL( propertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ),
			this, SLOT( slotPropertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ) ) ;

		connect( c, SIGNAL( contactDestroyed( Kopete::Contact * ) ),
			this, SLOT( slotContactDestroyed( Kopete::Contact * ) ) );

		connect( c, SIGNAL( idleStateChanged( Kopete::Contact * ) ),
			this, SIGNAL( contactIdleStateChanged( Kopete::Contact * ) ) );

		if( d->displayName.isEmpty() )
		{
			/*kdDebug(14010) << k_funcinfo <<
				"empty displayname, using contacts display" << endl;*/
			QString nick=c->property( Global::Properties::self()->nickName()).value().toString();
			setDisplayName( nick.isEmpty() ? c->contactId() : nick );
			setNameSource( c );
		}

		emit contactAdded(c);

		updateOnlineStatus();
	}
}

void MetaContact::updateOnlineStatus()
{
	Kopete::OnlineStatus::StatusType newStatus = Kopete::OnlineStatus::Unknown;
	Kopete::OnlineStatus mostSignificantStatus;

	for ( Q3PtrListIterator<Contact> it( d->contacts ); it.current(); ++it )
	{
		// find most significant status
		if ( it.current()->onlineStatus() > mostSignificantStatus )
			mostSignificantStatus = it.current()->onlineStatus();
	}

	newStatus = mostSignificantStatus.status();

	if( newStatus != d->onlineStatus )
	{
		d->onlineStatus = newStatus;
		emit onlineStatusChanged( this, d->onlineStatus );
	}
}


void MetaContact::removeContact(Contact *c, bool deleted)
{
	if( !d->contacts.contains( c ) )
	{
		kdDebug(14010) << k_funcinfo << " Contact is not in this metaContact " << endl;
	}
	else
	{
		// must check before removing, or will always be false
		bool wasTrackingName = ( nameSource() == c );
		// must check before removing, or will always be false
		bool wasTrackingPhoto = ( photoSource() == c );

		d->contacts.remove( c );

		// Set new name and photo tracking (or disable if no subcontacts left -- implicit
		if( wasTrackingName )
			setNameSource( d->contacts.first() );

		if( wasTrackingPhoto )
			setPhotoSource( 0L );

		if(!deleted)
		{  //If this function is tell by slotContactRemoved, c is maybe just a QObject
			disconnect( c, SIGNAL( onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ),
				this, SLOT( slotContactStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ) );
			disconnect( c, SIGNAL( propertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ),
				this, SLOT( slotPropertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ) ) ;
			disconnect( c, SIGNAL( contactDestroyed( Kopete::Contact * ) ),
				this, SLOT( slotContactDestroyed( Kopete::Contact * ) ) );
			disconnect( c, SIGNAL( idleStateChanged( Kopete::Contact * ) ),
				this, SIGNAL( contactIdleStateChanged( Kopete::Contact *) ) );

			kdDebug( 14010 ) << k_funcinfo << "Contact disconnected" << endl;

			KABCPersistence::self()->write( this );
		}

		// Reparent the contact
		removeChild( c );

		emit contactRemoved( c );
	}
	updateOnlineStatus();
}

Contact *MetaContact::findContact( const QString &protocolId, const QString &accountId, const QString &contactId )
{
	//kdDebug( 14010 ) << k_funcinfo << "Num contacts: " << d->contacts.count() << endl;
	Q3PtrListIterator<Contact> it( d->contacts );
	for( ; it.current(); ++it )
	{
		//kdDebug( 14010 ) << k_funcinfo << "Trying " << it.current()->contactId() << ", proto "
		//<< it.current()->protocol()->pluginId() << ", account " << it.current()->accountId() << endl;
		if( ( it.current()->contactId() == contactId ) && ( it.current()->protocol()->pluginId() == protocolId || protocolId.isNull() ) )
		{
			if ( accountId.isNull() )
				return it.current();

			if(it.current()->account())
			{
				if(it.current()->account()->accountId() == accountId)
					return it.current();
			}
		}
	}

	// Contact not found
	return 0L;
}

Contact *MetaContact::sendMessage()
{
	Contact *c = preferredContact();

	if( !c )
	{
		KMessageBox::queuedMessageBox( UI::Global::mainWidget(), KMessageBox::Sorry,
			i18n( "This user is not reachable at the moment. Please make sure you are connected and using a protocol that supports offline sending, or wait "
			"until this user comes online." ), i18n( "User is Not Reachable" ) );
	}
	else
	{
		c->sendMessage();
		return c;
	}
	return 0L;
}

Contact *MetaContact::startChat()
{
	Contact *c = preferredContact();

	if( !c )
	{
		KMessageBox::queuedMessageBox( UI::Global::mainWidget(), KMessageBox::Sorry,
			i18n( "This user is not reachable at the moment. Please make sure you are connected and using a protocol that supports offline sending, or wait "
			"until this user comes online." ), i18n( "User is Not Reachable" ) );
	}
	else
	{
		c->startChat();
		return c;
	}
	return 0L;
}

Contact *MetaContact::preferredContact()
{
	/*
		This function will determine what contact will be used to reach the contact.

		The prefered contact is choose with the following criterias:  (in that order)
		1) If a contact was an open chatwindow already, we will use that one.
		2) The contact with the better online status is used. But if that
		    contact is not reachable, we prefer return no contact.
		3) If all the criterias aboxe still gives ex-eaquo, we use the preffered
		    account as selected in the account preferances (with the arrows)
	*/

	Contact *contact = 0;
	bool hasOpenView=false; //has the selected contact already an open chatwindow
	for ( Q3PtrListIterator<Contact> it( d->contacts ); it.current(); ++it )
	{
		Contact *c=it.current();

		//Does the contact an open chatwindow?
		if( c->manager( Contact::CannotCreate ) )
		{ //no need to check the view. having a manager is enough
			if( !hasOpenView )
			{
				contact=c;
				hasOpenView=true;
				if( c->isReachable() )
					continue;
			} //else, several contact might have an open view, uses following criterias
		}
		else if( hasOpenView && contact->isReachable() )
			continue; //This contact has not open view, but the selected contact has, and is reachable

		// FIXME: The isConnected call should be handled in Contact::isReachable
		//        after KDE 3.2 - Martijn
		if ( !c->account() || !c->account()->isConnected() || !c->isReachable() )
			continue; //if this contact is not reachable, we ignore it.

		if ( !contact )
		{  //this is the first contact.
			contact= c;
			continue;
		}

		if( c->onlineStatus().status() > contact->onlineStatus().status()  )
			contact=c; //this contact has a better status
		else if ( c->onlineStatus().status() == contact->onlineStatus().status() )
		{
			if( c->account()->priority() > contact->account()->priority() )
				contact=c;
			else if(  c->account()->priority() == contact->account()->priority()
					&& c->onlineStatus().weight() > contact->onlineStatus().weight() )
				contact = c;  //the weight is not supposed to follow the same scale for each protocol
		}
	}
	return contact;
}

Contact *MetaContact::execute()
{
	Contact *c = preferredContact();

	if( !c )
	{
		KMessageBox::queuedMessageBox( UI::Global::mainWidget(), KMessageBox::Sorry,
			i18n( "This user is not reachable at the moment. Please make sure you are connected and using a protocol that supports offline sending, or wait "
			"until this user comes online." ), i18n( "User is Not Reachable" ) );
	}
	else
	{
		c->execute();
		return c;
	}

	return 0L;
}

unsigned long int MetaContact::idleTime() const
{
	unsigned long int time = 0;
	Q3PtrListIterator<Contact> it( d->contacts );
	for( ; it.current(); ++it )
	{
		unsigned long int i = it.current()->idleTime();
		if( i < time || time == 0 )
		{
			time = i;
		}
	}
	return time;
}

QString MetaContact::statusIcon() const
{
	switch( status() )
	{
		case OnlineStatus::Online:
			if( useCustomIcon() )
				return icon( ContactListElement::Online );
			else
				return QString::fromLatin1( "metacontact_online" );
		case OnlineStatus::Away:
			if( useCustomIcon() )
				return icon( ContactListElement::Away );
			else
				return QString::fromLatin1( "metacontact_away" );

		case OnlineStatus::Unknown:
			if( useCustomIcon() )
				return icon( ContactListElement::Unknown );
			if ( d->contacts.isEmpty() )
				return QString::fromLatin1( "metacontact_unknown" );
			else
				return QString::fromLatin1( "metacontact_offline" );

		case OnlineStatus::Offline:
		default:
			if( useCustomIcon() )
				return icon( ContactListElement::Offline );
			else
				return QString::fromLatin1( "metacontact_offline" );
	}
}

QString MetaContact::statusString() const
{
	switch( status() )
	{
		case OnlineStatus::Online:
			return i18n( "Online" );
		case OnlineStatus::Away:
			return i18n( "Away" );
		case OnlineStatus::Offline:
			return i18n( "Offline" );
		case OnlineStatus::Unknown:
		default:
			return i18n( "Status not available" );
	}
}

OnlineStatus::StatusType MetaContact::status() const
{
	return d->onlineStatus;
}

bool MetaContact::isOnline() const
{
	Q3PtrListIterator<Contact> it( d->contacts );
	for( ; it.current(); ++it )
	{
		if( it.current()->isOnline() )
			return true;
	}
	return false;
}

bool MetaContact::isReachable() const
{
	if ( isOnline() )
		return true;

	for ( Q3PtrListIterator<Contact> it( d->contacts ); it.current(); ++it )
	{
		if ( it.current()->account()->isConnected() && it.current()->isReachable() )
			return true;
	}
	return false;
}

//Determine if we are capable of accepting file transfers
bool MetaContact::canAcceptFiles() const
{
	if( !isOnline() )
		return false;

	Q3PtrListIterator<Contact> it( d->contacts );
	for( ; it.current(); ++it )
	{
		if( it.current()->canAcceptFiles() )
			return true;
	}
	return false;
}

//Slot for sending files
void MetaContact::sendFile( const KURL &sourceURL, const QString &altFileName, unsigned long fileSize )
{
	//If we can't send any files then exit
	if( d->contacts.isEmpty() || !canAcceptFiles() )
		return;

	//Find the highest ranked protocol that can accept files
	Contact *contact = d->contacts.first();
	for( Q3PtrListIterator<Contact> it( d->contacts ) ; it.current(); ++it )
	{
		if( ( *it )->onlineStatus() > contact->onlineStatus() && ( *it )->canAcceptFiles() )
			contact = *it;
	}

	//Call the sendFile slot of this protocol
	contact->sendFile( sourceURL, altFileName, fileSize );
}


void MetaContact::slotContactStatusChanged( Contact * c, const OnlineStatus &status, const OnlineStatus &/*oldstatus*/  )
{
	updateOnlineStatus();
	emit contactStatusChanged( c, status );
}

void MetaContact::setDisplayName( const QString &name )
{
	/*kdDebug( 14010 ) << k_funcinfo << "Change displayName from " << d->displayName <<
		" to " << name  << ", d->trackChildNameChanges=" << d->trackChildNameChanges << endl;
	kdDebug(14010) << kdBacktrace(6) << endl;*/

	if( name == d->displayName )
		return;

	const QString old = d->displayName;
	d->displayName = name;

	//The name is set by the user, disable tracking
	setNameSource( 0 );

	emit displayNameChanged( old , name );

	for( Q3PtrListIterator<Kopete::Contact> it( d->contacts ) ; it.current(); ++it )
		( *it )->sync(Contact::DisplayNameChanged);

}

QString MetaContact::displayName() const
{
	return d->displayName;
}

QImage MetaContact::photo() const
{
	if ( photoSource() == 0L )
	{
		// no photo source, try to get from addressbook
		// if the metacontact has a kabc association

		KABC::AddressBook* ab = KABCPersistence::self()->addressBook();

		// If the metacontact is linked to a kabc entry
		if ( !d->metaContactId.isEmpty() && !d->metaContactId.contains(':') )
		{
			KABC::Addressee theAddressee = ab->findByUid( metaContactId() );
			if ( theAddressee.isEmpty() )
			{
				kdDebug( 14010 ) << k_funcinfo << "no KABC::Addressee found for ( " << d->metaContactId << " ) " <<  displayName() << " in current address book" << endl;
			}
			else
			{
				KABC::Picture pic = theAddressee.photo();
				if ( pic.data().isNull() && pic.url().isEmpty() )
					pic = theAddressee.logo();

				if ( pic.isIntern())
				{
					return pic.data();
				}
				else
				{
					return QPixmap( pic.url() ).convertToImage();
				}
			}
		}
	}
	else
	{
		#warning FIXME reenable the photo suport when Trolltech made is mind in QImage/QPixmap support in QVariant
/*
		QVariant photoProp=photoSource()->property( Kopete::Global::Properties::self()->photo().key() ).value();
		QImage img;
		if(photoProp.canCast( QVariant::Image ))
			img=photoProp.toImage();
		else if(photoProp.canCast( QVariant::Pixmap ))
			img=photoProp.toPixmap().convertToImage();
		else if(!photoProp.asString().isEmpty())
		{
			img=QPixmap( photoProp.toString() ).convertToImage();
		}
		return img;
*/
	}

	// no kabc association, return null image
	return QImage();
}

Contact *MetaContact::nameSource() const
{
	// quick-out for contacts not tracking
	if( d->nameSourceCID.isEmpty() )
		return 0;

	for( Q3PtrListIterator< Contact > it ( d->contacts ); it.current(); ++it )
	{
		if( d->nameSourceCID == it.current()->contactId() &&
			d->nameSourcePID == it.current()->protocol()->pluginId() &&
			d->nameSourceAID == it.current()->account()->accountId() )
		{
			return it;
		}
	}

	// Invalid tracking information.  We don't clear the tracking  it in case the contact
	// is only temporarily unavailable (ie. plugin was disabled / broken).
	return 0;
}

Contact *MetaContact::photoSource() const
{
	// quick-out for contacts not tracking
	if( d->photoSourceCID.isEmpty() )
		return 0;

	for( Q3PtrListIterator< Contact > it ( d->contacts ); it.current(); ++it )
	{
		if( d->photoSourceCID == it.current()->contactId() &&
			d->photoSourcePID == it.current()->protocol()->pluginId() &&
			d->photoSourceAID == it.current()->account()->accountId() )
		{
			return it;
		}
	}

	// Invalid tracking information.  We don't clear the tracking  it in case the contact
	// is only temporarily unavailable (ie. plugin was disabled / broken).
	return 0;
}

void MetaContact::setNameSource( Contact *contact )
{
	if ( contact != 0 )
	{
		QString nick = contact->property( Global::Properties::self()->nickName() ).value().toString();
		setDisplayName( nick.isEmpty() ? contact->contactId() : nick );
		// We do this after, since setDisplayName clears it.
		d->nameSourceCID = contact->contactId();
		d->nameSourcePID = contact->protocol()->pluginId();
		d->nameSourceAID = contact->account()->accountId();
	}
	else
	{
		// Clear our name tracking
		d->nameSourceCID = "";
		d->nameSourcePID = "";
		d->nameSourceAID = "";
	}
	emit persistentDataChanged();
}

void MetaContact::setPhotoSource( Contact *contact )
{
	if ( contact != 0 )
	{
		d->photoSourceCID = contact->contactId();
		d->photoSourcePID = contact->protocol()->pluginId();
		d->photoSourceAID = contact->account()->accountId();
	}
	else
	{
		// Clear our name tracking
		d->photoSourceCID = "";
		d->photoSourcePID = "";
		d->photoSourceAID = "";
	}
	emit persistentDataChanged();
	emit photoChanged();
}

void MetaContact::slotPropertyChanged( Contact* subcontact, const QString &key,
		const QVariant&, const QVariant &newValue  )
{
	if( key == Global::Properties::self()->nickName().key() )
	{
		Contact* ns = nameSource();
		bool isTrackedSubcontact = ( subcontact == ns );
		QString newNick=newValue.toString();

		if( isTrackedSubcontact && !newNick.isEmpty() )
		{
			// The subcontact we are tracking just changed its name.
			setDisplayName( newNick );
			//because nameSource is removed in setDisplayName
			setNameSource( ns );
		}
	}
	else if ( key == Global::Properties::self()->photo().key() )
	{
		if(photoSource() == 0L && !newValue.isNull() &&   photo().isNull() )
		{
			setPhotoSource(subcontact);
		}
		else if(photoSource() == subcontact)
		{
			if(d->photoSyncedWithKABC)
				setPhotoSyncedWithKABC(true);
			emit photoChanged();
		}
	}

	//TODO:  check if the property was persistent, and emit, not only when it's the displayname
	emit persistentDataChanged();
}

void MetaContact::moveToGroup( Group *from, Group *to )
{
	if ( !from || !groups().contains( from )  )
	{
		// We're adding, not moving, because 'from' is illegal
		addToGroup( to );
		return;
	}

	if ( !to || groups().contains( to )  )
	{
		// We're removing, not moving, because 'to' is illegal
		removeFromGroup( from );
		return;
	}

	if ( isTemporary() && to->type() != Group::Temporary )
		return;


	//kdDebug( 14010 ) << k_funcinfo << from->displayName() << " => " << to->displayName() << endl;

	d->groups.remove( from );
	d->groups.append( to );

	for( Contact *c = d->contacts.first(); c ; c = d->contacts.next() )
		c->sync(Contact::MovedBetweenGroup);

	emit movedToGroup( this, from, to );
}

void MetaContact::removeFromGroup( Group *group )
{
	if ( !group || !groups().contains( group ) || ( isTemporary() && group->type() == Group::Temporary ) )
	{
		return;
	}

	d->groups.remove( group );

	// make sure MetaContact is at least in one group
	if ( d->groups.isEmpty() )
	{
		d->groups.append( Group::topLevel() );
		emit addedToGroup( this, Group::topLevel() );
	}

	for( Contact *c = d->contacts.first(); c ; c = d->contacts.next() )
		c->sync(Contact::MovedBetweenGroup);

	emit removedFromGroup( this, group );
}

void MetaContact::addToGroup( Group *to )
{
	if ( !to || groups().contains( to )  )
		return;

	if ( d->temporary && to->type() != Group::Temporary )
		return;

	if ( d->groups.contains( Group::topLevel() ) )
	{
		d->groups.remove( Group::topLevel() );
		emit removedFromGroup( this, Group::topLevel() );
	}

	d->groups.append( to );

	for( Contact *c = d->contacts.first(); c ; c = d->contacts.next() )
		c->sync(Contact::MovedBetweenGroup);

	emit addedToGroup( this, to );
}

Q3PtrList<Group> MetaContact::groups() const
{
	return d->groups;
}

void MetaContact::slotContactDestroyed( Contact *contact )
{
	removeContact(contact,true);
}

const QDomElement MetaContact::toXML()
{
	// This causes each Kopete::Protocol subclass to serialise its contacts' data into the metacontact's plugin data and address book data
	emit aboutToSave(this);

	QDomDocument metaContact;
	metaContact.appendChild( metaContact.createElement( QString::fromLatin1( "meta-contact" ) ) );
	metaContact.documentElement().setAttribute( QString::fromLatin1( "contactId" ), metaContactId() );

	QDomElement displayName = metaContact.createElement( QString::fromLatin1("display-name" ) );
	displayName.appendChild( metaContact.createTextNode( d->displayName ) );
	if ( !d->nameSourceCID.isEmpty() )
	{
		displayName.setAttribute( NSCID_ELEM, d->nameSourceCID );
		displayName.setAttribute( NSPID_ELEM, d->nameSourcePID );
		displayName.setAttribute( NSAID_ELEM, d->nameSourceAID );
	}
	metaContact.documentElement().appendChild( displayName );

	if( !d->metaContactId.isEmpty()  )
	{
		QDomElement photo = metaContact.createElement( QString::fromLatin1("photo" ) );
		photo.setAttribute( QString::fromLatin1("syncWithKABC") , QString::fromLatin1( d->photoSyncedWithKABC ? "1" : "0" ) );
		photo.setAttribute( PSCID_ELEM, d->photoSourceCID );
		photo.setAttribute( PSPID_ELEM, d->photoSourcePID );
		photo.setAttribute( PSAID_ELEM, d->photoSourceAID );
		metaContact.documentElement().appendChild( photo );
	}

	// Store groups
	if ( !d->groups.isEmpty() )
	{
		QDomElement groups = metaContact.createElement( QString::fromLatin1("groups") );
		Group *g;
		for ( g = d->groups.first(); g; g = d->groups.next() )
		{
			QDomElement group = metaContact.createElement( QString::fromLatin1("group") );
			group.setAttribute( QString::fromLatin1("id"), g->groupId() );
			groups.appendChild( group );
		}
		metaContact.documentElement().appendChild( groups );
	}

	// Store other plugin data
	Q3ValueList<QDomElement> pluginData = Kopete::ContactListElement::toXML();
	for( Q3ValueList<QDomElement>::Iterator it = pluginData.begin(); it != pluginData.end(); ++it )
		metaContact.documentElement().appendChild( metaContact.importNode( *it, true ) );

	// Store custom notification data
	QDomElement notifyData = NotifyDataObject::notifyDataToXML();
	if ( notifyData.hasChildNodes() )
		metaContact.documentElement().appendChild( metaContact.importNode( notifyData, true ) );
	return metaContact.documentElement();
}

bool MetaContact::fromXML( const QDomElement& element )
{
	if( !element.hasChildNodes() )
		return false;

	QString strContactId = element.attribute( QString::fromLatin1("contactId") );
	if( !strContactId.isEmpty() )
		d->metaContactId = strContactId;

	QDomElement contactElement = element.firstChild().toElement();
	while( !contactElement.isNull() )
	{
		if( contactElement.tagName() == QString::fromLatin1( "display-name" ) )
		{
			if ( contactElement.text().isEmpty() )
				return false;
			d->displayName = contactElement.text();

			d->nameSourceCID = contactElement.attribute( NSCID_ELEM );
			d->nameSourcePID = contactElement.attribute( NSPID_ELEM );
			d->nameSourceAID = contactElement.attribute( NSAID_ELEM );

		}
		else if( contactElement.tagName() == QString::fromLatin1( "photo" ) )
		{
			d->photoSourceCID = contactElement.attribute( PSCID_ELEM );
			d->photoSourcePID = contactElement.attribute( PSPID_ELEM );
			d->photoSourceAID = contactElement.attribute( PSAID_ELEM );
			d->photoSyncedWithKABC = contactElement.attribute( QString::fromLatin1( "syncWithKABC" ) ) == QString::fromLatin1("1") ;
		}
		else if( contactElement.tagName() == QString::fromLatin1( "groups" ) )
		{
			QDomNode group = contactElement.firstChild();
			while( !group.isNull() )
			{
				QDomElement groupElement = group.toElement();

				if( groupElement.tagName() == QString::fromLatin1( "group" ) )
				{
					QString strGroupId = groupElement.attribute( QString::fromLatin1("id") );
					if( !strGroupId.isEmpty() )
						addToGroup( Kopete::ContactList::self()->group( strGroupId.toUInt() ) );
					else //kopete 0.6 contactlist
						addToGroup( Kopete::ContactList::self()->findGroup( groupElement.text() ) );
				}
				else if( groupElement.tagName() == QString::fromLatin1( "top-level" ) ) //kopete 0.6 contactlist
					addToGroup( Kopete::Group::topLevel() );

				group = group.nextSibling();
			}
		}
		else if( contactElement.tagName() == QString::fromLatin1( "address-book-field" ) )
		{
			QString app = contactElement.attribute( QString::fromLatin1( "app" ), QString::null );
			QString key = contactElement.attribute( QString::fromLatin1( "key" ), QString::null );
			QString val = contactElement.text();
			d->addressBook[ app ][ key ] = val;
		}
		else if( contactElement.tagName() == QString::fromLatin1( "custom-notifications" ) )
		{
			Kopete::NotifyDataObject::notifyDataFromXML( contactElement );
		}
		else //if( groupElement.tagName() == QString::fromLatin1( "plugin-data" ) || groupElement.tagName() == QString::fromLatin1("custom-icons" ))
		{
			Kopete::ContactListElement::fromXML(contactElement);
		}
		contactElement = contactElement.nextSibling().toElement();
	}

	// If a plugin is loaded, load data cached
	connect( Kopete::PluginManager::self(), SIGNAL( pluginLoaded(Kopete::Plugin*) ),
		this, SLOT( slotPluginLoaded(Kopete::Plugin*) ) );

	// track changes only works if ONE Contact is inside the MetaContact
//	if (d->contacts.count() > 1) // Does NOT work as intended
//		d->trackChildNameChanges=false;

//	kdDebug(14010) << "[Kopete::MetaContact] END fromXML(), d->trackChildNameChanges=" << d->trackChildNameChanges << "." << endl;
	return true;
}

QString Kopete::MetaContact::addressBookField( Kopete::Plugin * /* p */, const QString &app, const QString & key ) const
{
	return d->addressBook[ app ][ key ];
}

void Kopete::MetaContact::setAddressBookField( Kopete::Plugin * /* p */, const QString &app, const QString &key, const QString &value )
{
	d->addressBook[ app ][ key ] = value;
}

void MetaContact::slotPluginLoaded( Plugin *p )
{
	if( !p )
		return;

	QMap<QString, QString> map= pluginData( p );
	if(!map.isEmpty())
	{
		p->deserialize(this,map);
	}
}




bool MetaContact::isTemporary() const
{
	return d->temporary;
}

void MetaContact::setTemporary( bool isTemporary, Group *group )
{
	d->temporary = isTemporary;
	Group *temporaryGroup = Group::temporary();
	if ( d->temporary )
	{
		addToGroup (temporaryGroup);
		Group *g;
		for( g = d->groups.first(); g; g = d->groups.next() )
		{
			if(g != temporaryGroup)
				removeFromGroup(g);
		}
	}
	else
		moveToGroup(temporaryGroup, group ? group : Group::topLevel());
}

QString MetaContact::metaContactId() const
{
	if(d->metaContactId.isEmpty())
	{
		Contact *c=d->contacts.first();
		if(!c)
			return QString::null;
		return c->protocol()->pluginId()+QString::fromLatin1(":")+c->account()->accountId()+QString::fromLatin1(":") + c->contactId() ;
	}
	return d->metaContactId;
}

void MetaContact::setMetaContactId( const QString& newMetaContactId )
{
	if(newMetaContactId == d->metaContactId)
		return;

	// 1) Check the Id is not already used by another contact
	// 2) cause a kabc write ( only in response to metacontactLVIProps calling this, or will
	//      write be called twice when creating a brand new MC? )
	// 3) What about changing from one valid kabc to another, are kabc fields removed?
	// 4) May be called with Null to remove an invalid kabc uid by KMC::toKABC()
	// 5) Is called when reading the saved contact list

	// Don't remove IM addresses from kabc if we are changing contacts; 
	// other programs may have written that data and depend on it
	d->metaContactId = newMetaContactId;
	KABCPersistence::self()->write( this );
	emit onlineStatusChanged( this, d->onlineStatus );
	emit persistentDataChanged();
}

bool MetaContact::isPhotoSyncedWithKABC() const
{
	return d->photoSyncedWithKABC;
}

void MetaContact::setPhotoSyncedWithKABC(bool b)
{
	d->photoSyncedWithKABC=b;
	if(b)
	{
		Contact *source=photoSource();
		if(!source)
			return;
		QVariant newValue=source->property( Kopete::Global::Properties::self()->photo() ).value();
		if ( !d->metaContactId.isEmpty() && !newValue.isNull())
		{
			KABC::Addressee theAddressee = KABCPersistence::self()->addressBook()->findByUid( metaContactId() );

			if ( !theAddressee.isEmpty() )
			{
				#warning FIXME Photo support
/*
				QImage img;
				if(newValue.canCast( QVariant::Image ))
					img=newValue.toImage();
				else if(newValue.canCast( QVariant::Pixmap ))
					img=newValue.toPixmap().convertToImage();

				if(img.isNull())
					theAddressee.setPhoto(newValue.toString());
				else
					theAddressee.setPhoto(img);

				KABCPersistence::self()->addressBook()->insertAddressee(theAddressee);
				KABCPersistence::self()->writeAddressBook( theAddressee.resource() );
*/
			}
		}
	}
}

Q3PtrList<Contact> MetaContact::contacts() const
{
	return d->contacts;
}
} //END namespace Kopete

#include "kopetemetacontact.moc"

// vim: set noet ts=4 sts=4 sw=4:
