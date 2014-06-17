/*
    kopetemetacontact.cpp - Kopete Meta Contact

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2002-2004 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2005      by Michaël Larouche       <larouche@kde.org>

    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

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
#include "kopetemetacontact_p.h"

#include <QTextDocument>

#include <kabc/addressbook.h>
#include <kabc/addressee.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdeversion.h>
#include <knotification.h>

#include "kabcpersistence.h"
#include "kopetecontactlist.h"
#include "kopetecontact.h"
#include "kopeteaccountmanager.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetepluginmanager.h"
#include "kopetegroup.h"
#include "kopeteglobal.h"
#include "kopeteuiglobal.h"
#include "kopetebehaviorsettings.h"
#include "kopeteemoticons.h"

namespace Kopete {

MetaContact::MetaContact()
	: ContactListElement( ContactList::self() ), d(new Private())
{
	d->metaContactId = QUuid::createUuid();

	connect( this, SIGNAL(pluginDataChanged()), SIGNAL(persistentDataChanged()) );
	connect( this, SIGNAL(iconChanged(Kopete::ContactListElement::IconState,QString)), SIGNAL(persistentDataChanged()) );
	connect( this, SIGNAL(useCustomIconChanged(bool)), SIGNAL(persistentDataChanged()) );
	connect( this, SIGNAL(displayNameChanged(QString,QString)), SIGNAL(persistentDataChanged()) );
	connect( this, SIGNAL(movedToGroup(Kopete::MetaContact*,Kopete::Group*,Kopete::Group*)), SIGNAL(persistentDataChanged()) );
	connect( this, SIGNAL(removedFromGroup(Kopete::MetaContact*,Kopete::Group*)), SIGNAL(persistentDataChanged()) );
	connect( this, SIGNAL(addedToGroup(Kopete::MetaContact*,Kopete::Group*)), SIGNAL(persistentDataChanged()) );
	connect( this, SIGNAL(contactAdded(Kopete::Contact*)), SIGNAL(persistentDataChanged()) );
	connect( this, SIGNAL(contactRemoved(Kopete::Contact*)), SIGNAL(persistentDataChanged()) );

	// TODO: speed up: this slot is called when any kabc contact is changed and is called in *every* metacontact instance. also slot is slow because it finding kabc id
	// Update the KABC picture when the KDE Address book change.
	connect(KABCPersistence::self()->addressBook(), SIGNAL(addressBookChanged(AddressBook*)), this, SLOT(slotUpdateAddressBookPicture()));

	// make sure MetaContact is at least in one group
	addToGroup( Group::topLevel() );
			 // I'm not sure this is correct -Olivier
			 // we probably should do the check in groups() instead

}

MetaContact::~MetaContact()
{
	delete d;
}

QUuid MetaContact::metaContactId() const
{
	return d->metaContactId;
}

void MetaContact::setMetaContactId( const QUuid& newUuid)
{
	d->metaContactId = newUuid;
}

void MetaContact::addContact( Contact *c )
{
	if( d->contacts.contains( c ) )
	{
		kWarning(14010) << "Ignoring attempt to add duplicate contact " << c->contactId() << "!";
	}
	else
	{
		const QString oldDisplayName = displayName();

		d->contacts.append( c );
		connect( c, SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
			SLOT(slotContactStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)) );

		connect( c, SIGNAL(propertyChanged(Kopete::PropertyContainer*,QString,QVariant,QVariant)),
			this, SLOT(slotPropertyChanged(Kopete::PropertyContainer*,QString,QVariant,QVariant)) ) ;

		connect( c, SIGNAL(displayNameChanged(QString,QString)),
			this, SLOT(slotContactDisplayNameChanged(QString,QString)) );

		connect( c, SIGNAL(contactDestroyed(Kopete::Contact*)),
			this, SLOT(slotContactDestroyed(Kopete::Contact*)) );

		connect( c, SIGNAL(idleStateChanged(Kopete::Contact*)),
			this, SIGNAL(contactIdleStateChanged(Kopete::Contact*)) );

		emit contactAdded(c);

		updateOnlineStatus();
		
		// if this is the first contact, probbaly was created by a protocol
		// so it has empty custom properties, then set sources to the contact
		if ( d->contacts.count() == 1 )
		{
			const QString newDisplayName = displayName();
			if ( oldDisplayName != newDisplayName )
			{
				emit displayNameChanged( oldDisplayName , displayName() );
				QListIterator<Kopete::Contact *> it( d->contacts );
				while (  it.hasNext() )
					( it.next() )->sync(Contact::DisplayNameChanged);
			}
			if ( picture().isNull() )
			{
				setPhotoSourceContact(c);
				setPhotoSource(SourceContact);
			}
		}
	}
}

void MetaContact::updateOnlineStatus()
{
	Kopete::OnlineStatus::StatusType newStatus = Kopete::OnlineStatus::Unknown;
	Kopete::OnlineStatus mostSignificantStatus;

	QListIterator<Contact *> it(d->contacts);
	while ( it.hasNext() )
	{
		Contact *c = it.next();
		// find most significant status
		if ( c->onlineStatus() > mostSignificantStatus )
			mostSignificantStatus = c->onlineStatus();
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
		kDebug(14010) << " Contact is not in this metaContact ";
	}
	else
	{
		// must check before removing, or will always be false
		bool wasTrackingName = ( !displayNameSourceContact() && (displayNameSource() == SourceContact) );
		bool wasTrackingPhoto = ( !photoSourceContact() && (photoSource() == SourceContact) );
		// save for later use
		QString currDisplayName = displayName();

		d->contacts.removeAll( c );

		// if the contact was a source of property data, clean
		if (displayNameSourceContact() == c)
			setDisplayNameSourceContact(0L);
		if (photoSourceContact() == c)
			setPhotoSourceContact(0L);


		if ( wasTrackingName )
		{
			// Oh! this contact was the source for the metacontact's name
			// lets do something
			// is this the only contact?
			if ( d->contacts.isEmpty() )
			{
				// fallback to a custom name as we don't have
				// more contacts to chose as source.
				setDisplayNameSource(SourceCustom);
				// perhaps the custom display name was empty
				// no problems baby, I saved the old one.
				setDisplayName(currDisplayName);
			}
			else
			{
				// we didn't fallback to SourceCustom above so lets use the next
				// contact as source
				setDisplayNameSourceContact( d->contacts.first() );
			}
		}

		if ( wasTrackingPhoto )
		{
			// Oh! this contact was the source for the metacontact's photo
			// lets do something
			// is this the only contact?
			if ( d->contacts.isEmpty() )
			{
				// fallback to a custom photo as we don't have
				// more contacts to chose as source.
				setPhotoSource(SourceCustom);
				// FIXME set the custom photo
			}
			else
			{
				// we didn't fallback to SourceCustom above so lets use the next
				// contact as source
				setPhotoSourceContact( d->contacts.first() );
			}
		}

		if(!deleted)
		{  //If this function is tell by slotContactRemoved, c is maybe just a QObject
			disconnect( c, SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
				this, SLOT(slotContactStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)) );
			disconnect( c, SIGNAL(propertyChanged(Kopete::PropertyContainer*,QString,QVariant,QVariant)),
				this, SLOT(slotPropertyChanged(Kopete::PropertyContainer*,QString,QVariant,QVariant)) ) ;
			disconnect( c, SIGNAL(displayNameChanged(QString,QString)),
				this, SLOT(slotContactDisplayNameChanged(QString,QString)) );
			disconnect( c, SIGNAL(contactDestroyed(Kopete::Contact*)),
				this, SLOT(slotContactDestroyed(Kopete::Contact*)) );
			disconnect( c, SIGNAL(idleStateChanged(Kopete::Contact*)),
				this, SIGNAL(contactIdleStateChanged(Kopete::Contact*)) );

			kDebug( 14010 ) << "Contact disconnected";

			KABCPersistence::self()->write( this );
		}

		// Reparent the contact
		c->setParent( 0 );

		emit contactRemoved( c );
	}
	updateOnlineStatus();
}

Contact *MetaContact::findContact( const QString &protocolId, const QString &accountId, const QString &contactId )
{
	//kDebug( 14010 ) << "Num contacts: " << d->contacts.count();
	QListIterator<Contact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Contact *c = it.next();
		//kDebug( 14010 ) << "Trying " << it.current()->contactId() << ", proto "
		//<< it.current()->protocol()->pluginId() << ", account " << it.current()->accountId() << endl;
		if( ( c->contactId() == contactId ) && ( c->protocol()->pluginId() == protocolId || protocolId.isNull() ) )
		{
			if ( accountId.isNull() )
				return c;

			if(c->account())
			{
				if(c->account()->accountId() == accountId)
					return c;
			}
		}
	}

	// Contact not found
	return 0L;
}

void MetaContact::setDisplayNameSource(PropertySource source)
{
	QString oldName = displayName();
	d->displayNameSource = source;
	QString newName = displayName();
	if ( oldName != newName) {
		emit displayNameChanged( oldName, newName );
		QListIterator<Kopete::Contact *> it( d->contacts );
		while (  it.hasNext() )
			( it.next() )->sync(Contact::DisplayNameChanged);
	}
}

void MetaContact::setDisplayNameSource( const QString &nameSourcePID, const QString &nameSourceAID, const QString &nameSourceCID )
{
	d->nameSourcePID = nameSourcePID;
	d->nameSourceAID = nameSourceAID;
	d->nameSourceCID = nameSourceCID;
}

MetaContact::PropertySource MetaContact::displayNameSource() const
{
	return d->displayNameSource;
}

void MetaContact::setPhotoSource(PropertySource source)
{
	PropertySource oldSource = photoSource();
	d->photoSource = source;
	if ( source != oldSource )
	{
		emit photoChanged();
	}
}

void MetaContact::setPhotoSource( const QString &photoSourcePID, const QString &photoSourceAID, const QString &photoSourceCID )
{
	d->photoSourcePID = photoSourcePID;
	d->photoSourceAID = photoSourceAID;
	d->photoSourceCID = photoSourceCID;
}

MetaContact::PropertySource MetaContact::photoSource() const
{
	return d->photoSource;
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

		The preferred contact is choose with the following criterias:  (in that order)
		1) If a contact was an open chatwindow already, we will use that one.
		2) The contact with the better online status is used. But if that
		    contact is not reachable, we prefer return no contact.
		3) If all the criterias aboxe still gives ex-eaquo, we use the preffered
		    account as selected in the account preferances (with the arrows)
	*/

	Contact *contact = 0;
	bool hasOpenView=false; //has the selected contact already an open chatwindow
	QListIterator<Contact *> it(d->contacts);
	while ( it.hasNext() )
	{
		Contact *c=it.next();

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

quint32 MetaContact::idleTime() const
{
	unsigned long int time = 0;
	QListIterator<Contact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Contact *c = it.next();
		unsigned long int i = c->idleTime();
		if( (c->isOnline() && i < time) || time == 0 )
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
				return QString::fromUtf8( "user-online" );
		case OnlineStatus::Away:
			if( useCustomIcon() )
				return icon( ContactListElement::Away );
			else
				return QString::fromUtf8( "user-away" );
		case OnlineStatus::Busy:
			if( useCustomIcon() )
				return icon( ContactListElement::Away ); //Might want to create custom for busy too
			else
				return QString::fromUtf8( "user-busy" );

		case OnlineStatus::Unknown:
			if( useCustomIcon() )
				return icon( ContactListElement::Unknown );
			if ( d->contacts.isEmpty() )
				return QString::fromUtf8( "metacontact_unknown" );
			else
				return QString::fromUtf8( "user-offline" );

		case OnlineStatus::Offline:
		default:
			if( useCustomIcon() )
				return icon( ContactListElement::Offline );
			else
				return QString::fromUtf8( "user-offline" );
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
		case OnlineStatus::Busy:
			return i18n( "Busy" );
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
	QListIterator<Contact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Contact* c = it.next();
		if( c && c->isOnline() )
			return true;
	}
	return false;
}

bool MetaContact::isAlwaysVisible() const
{
	QListIterator<Contact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Contact* c = it.next();
		if ( c && c->property( Kopete::Global::Properties::self()->isAlwaysVisible() ).value().toBool() )
			return true;
	}
	return false;
}

bool MetaContact::isReachable() const
{
	if ( isOnline() )
		return true;

	QListIterator<Contact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Contact *c = it.next();
		if ( c->account()->isConnected() && c->isReachable() )
			return true;
	}
	return false;
}

//Determine if we are capable of accepting file transfers
bool MetaContact::canAcceptFiles() const
{
	if( !isOnline() )
		return false;

	QListIterator<Contact *> it( d->contacts );
	while ( it.hasNext() )
	{
		if( it.next()->canAcceptFiles() )
			return true;
	}
	return false;
}

//Slot for sending files
void MetaContact::sendFile( const KUrl &sourceURL, const QString &altFileName, unsigned long fileSize )
{
	//If we can't send any files then exit
	if( d->contacts.isEmpty() || !canAcceptFiles() )
		return;

	//Find the highest ranked protocol that can accept files
	Contact *contact = d->contacts.first();
	QListIterator<Contact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Contact *curr = it.next();
		if( (curr)->onlineStatus() > contact->onlineStatus() && (curr)->canAcceptFiles() )
			contact = curr;
	}

	//Call the sendFile slot of this protocol
	contact->sendFile( sourceURL, altFileName, fileSize );
}

void MetaContact::serialize()
{
	clearPluginContactData();

	QSet<Kopete::Protocol*> protocolSet;
	foreach ( Kopete::Contact* c, contacts() )
	{
		Kopete::Protocol* protocol = c->protocol();
		if ( !protocolSet.contains( protocol ) )
		{
			protocolSet.insert( protocol );
			protocol->serialize( this );
		}
	}
}

void MetaContact::slotContactStatusChanged( Contact * c, const OnlineStatus &status, const OnlineStatus &/*oldstatus*/  )
{
	updateOnlineStatus();
	emit contactStatusChanged( c, status );

	if ( c != c->account()->myself() )
		onlineStatusNotification( c );
}

void MetaContact::setDisplayName( const QString &name )
{
	/*kDebug( 14010 ) << "Change displayName from " << d->displayName <<
		" to " << name  << ", d->trackChildNameChanges=" << d->trackChildNameChanges << endl;
	kDebug(14010) << kBacktrace(6);*/

	if( name == d->displayName )
		return;

	if ( loading() )
	{
		d->displayName = name;
	}
	else
	{
		//check if there is another contact with the same display name.
		//if this is the case, merge them
		if(!this->d->temporary && !name.isEmpty())
			foreach(MetaContact *m, ContactList::self()->metaContacts())
		{
			if( !m->d->temporary && m != this && m->customDisplayName() == name)
			{
				//merge
				while(!m->d->contacts.isEmpty())
				{
					m->d->contacts.first()->setMetaContact(this);
				}
				//the contact will be automatically removed when the last contact is removed
				//that's why we merge othe other into this one and not the opposite;
				break;
			}
		}

		const QString old = d->displayName;
		d->displayName = name;

		emit displayNameChanged( old , name );
		QListIterator<Kopete::Contact *> it( d->contacts );
		while (  it.hasNext() )
			( it.next() )->sync(Contact::DisplayNameChanged);
	}
}

QString MetaContact::customDisplayName() const
{
	return d->displayName;
}

QString MetaContact::displayName() const
{
	PropertySource source = displayNameSource();
	if ( source == SourceKABC )
	{
		// kabc source, try to get from addressbook
		// if the metacontact has a kabc association
		if ( !kabcId().isEmpty() )
			return nameFromKABC(kabcId());
	}
	else if ( source == SourceContact || d->displayName.isEmpty())
	{
		if ( d->displayNameSourceContact==0 )
		{
			if( d->contacts.count() >= 1 )
			{// don't call setDisplayNameSource , or there will probably be an infinite loop
				d->displayNameSourceContact=d->contacts.first();
//				kDebug( 14010 ) << " setting displayname source for " << metaContactId();
			}
		}
		if ( displayNameSourceContact() != 0L )
		{
			return nameFromContact(displayNameSourceContact());
		}
		else
		{
//			kDebug( 14010 ) << " source == SourceContact , but there is no displayNameSourceContact for contact " << metaContactId();
		}
	}
	return d->displayName;
}

QString nameFromKABC( const QString &id ) /*const*/
{
	KABC::AddressBook* ab = KABCPersistence::self()->addressBook();
	if ( ! id.isEmpty() && !id.contains(':') )
	{
		KABC::Addressee theAddressee = ab->findByUid(id);
		if ( theAddressee.isEmpty() )
		{
			kDebug( 14010 ) << "no KABC::Addressee found for ( " << id << " ) " << " in current address book";
		}
		else
		{
			return theAddressee.formattedName();
		}
	}
	// no kabc association, return null image
	return QString();
}

QString nameFromContact( Kopete::Contact *c) /*const*/
{
	if ( !c )
		return QString();

	QString contactName = c->displayName();
				//the remove is there to workaround the Bug 95444
	return contactName.remove('\n');
}

KUrl MetaContact::customPhoto() const
{
	return KUrl(d->customPicture.path());
}

void MetaContact::setPhoto( const KUrl &url )
{
	d->photoUrl = url;
	d->customPicture.setPicture(url.toLocalFile());

	if ( photoSource() == SourceCustom )
	{
		emit photoChanged();
	}
}

QImage MetaContact::photo() const
{
	return picture().image();
}

Picture &MetaContact::picture() const
{
	if ( photoSource() == SourceKABC )
	{
		return d->kabcPicture;
	}
	else if ( photoSource() == SourceContact )
	{
		return d->contactPicture;
	}

	return d->customPicture;
}

QImage MetaContact::photoFromCustom() const
{
	return d->customPicture.image();
}

QImage photoFromContact( Kopete::Contact *contact) /*const*/
{
	if ( contact == 0L )
		return QImage();

	QVariant photoProp;
	if ( contact->hasProperty( Kopete::Global::Properties::self()->photo().key() ) )
		photoProp = contact->property( Kopete::Global::Properties::self()->photo().key() ).value();

	QImage img;
	if(photoProp.canConvert( QVariant::Image ))
		img= photoProp.value<QImage>();
	else if(photoProp.canConvert( QVariant::Pixmap ))
		img=photoProp.value<QPixmap>().toImage();
	else if(!photoProp.toString().isEmpty())
	{
		img=QPixmap( photoProp.toString() ).toImage();
	}
	return img;
}

QImage photoFromKABC( const QString &id ) /*const*/
{
	KABC::AddressBook* ab = KABCPersistence::self()->addressBook();
	if ( ! id.isEmpty() && !id.contains(':') )
	{
		KABC::Addressee theAddressee = ab->findByUid(id);
		if ( theAddressee.isEmpty() )
		{
			kDebug( 14010 ) << "no KABC::Addressee found for ( " << id << " ) " << " in current address book";
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
				return QPixmap( pic.url() ).toImage();
			}
		}
	}
	// no kabc association, return null image
	return QImage();
}

Contact *MetaContact::displayNameSourceContact() const
{
	return d->displayNameSourceContact;
}

Contact *MetaContact::photoSourceContact() const
{
	return d->photoSourceContact;
}

void MetaContact::setDisplayNameSourceContact( Contact *contact )
{
	Contact *old = d->displayNameSourceContact;
	d->displayNameSourceContact = contact;
	if ( contact && displayNameSource() == SourceContact ) {
		emit displayNameChanged( nameFromContact(old), nameFromContact(contact));
		QListIterator<Kopete::Contact *> it( d->contacts );
		while (  it.hasNext() )
			( it.next() )->sync(Contact::DisplayNameChanged);
	}
}

void MetaContact::setPhotoSourceContact( Contact *contact )
{
	d->photoSourceContact = contact;

	// Create a cache for the contact photo.
	if(d->photoSourceContact != 0L)
	{
		QVariant photoProp;
		if ( contact->hasProperty( Kopete::Global::Properties::self()->photo().key() ) )
		{
			photoProp = contact->property( Kopete::Global::Properties::self()->photo().key() ).value();

			if(photoProp.canConvert( QVariant::Image ))
			{
				d->contactPicture.setPicture( photoProp.value<QImage>() );
			}
			else if(photoProp.canConvert( QVariant::Pixmap ))
			{
				d->contactPicture.setPicture( photoProp.value<QPixmap>().toImage() );
			}
			else if(!photoProp.toString().isEmpty())
			{
				d->contactPicture.setPicture(photoProp.toString());
			}
			else
			{
				d->contactPicture.clear();
			}
		}
		else
		{
			d->contactPicture.clear();
		}
	}
	else
	{
		d->contactPicture.clear();
	}

	if ( photoSource() == SourceContact )
	{
		emit photoChanged();
	}
}

void MetaContact::slotContactDisplayNameChanged(const QString &oldName, const QString &newName)
{
	Contact *subcontact = static_cast<Contact *>(sender());
	if ( displayNameSource() == SourceContact || 
			(d->displayName.isEmpty() && displayNameSource() == SourceCustom) )
	{
		if (displayNameSourceContact() == subcontact)
		{
			emit displayNameChanged(oldName, newName);
			QListIterator<Kopete::Contact *> it( d->contacts );
			while (  it.hasNext() )
				( it.next() )->sync(Contact::DisplayNameChanged);
		}
		else
		{
			// HACK the displayName that changed is not from the contact we are tracking, but
			// as the current one is null, lets use this new one
			if (displayName().isEmpty())
				setDisplayNameSourceContact(subcontact);
		}
	}
}

void MetaContact::slotPropertyChanged( PropertyContainer* _subcontact, const QString &key,
		const QVariant &oldValue, const QVariant &newValue  )
{
	Contact *subcontact=static_cast<Contact*>(_subcontact);

	if (photoSource() == SourceContact)
	{
		if ( key == Global::Properties::self()->photo().key() )
		{
			if (photoSourceContact() != subcontact)
			{
				// HACK the displayName that changed is not from the contact we are tracking, but
				// as the current one is null, lets use this new one
				if (picture().isNull())
					setPhotoSourceContact(subcontact);

			}
			else if(photoSourceContact() == subcontact)
			{
				if(d->photoSyncedWithKABC)
					setPhotoSyncedWithKABC(true);

				setPhotoSourceContact(subcontact);
			}
		}
	}

	if ( (key == Kopete::Global::Properties::self()->statusMessage().key() ||
	     key == Kopete::Global::Properties::self()->statusTitle().key()) && oldValue != newValue )
	{
		QString statusText;

		bool allOffline = !this->isOnline();
		if ( newValue.toString().isEmpty() || ( !subcontact->isOnline() && !allOffline ) )
		{
			// try to find a more suitable away message to be displayed when:
			// -new away message is empty or
			// -contact who set it is offline and there are contacts online in the metacontact
			foreach ( Kopete::Contact *c, d->contacts )
			{
				QString curStatusText( c->property( key ).value().toString() );
				if ( ( allOffline || c->isOnline() ) && !curStatusText.isEmpty() )
				{
					// display this contact's away message when:
					// -this contact's away message is not empty and
					// -this contact is online or there are no contacts online at all
					statusText = curStatusText;
					break;
				}
			}
		}
		else
		{
			// just use new away message when:
			// -new away message is not empty and
			// -contact who set it is online or there are no contacts online at all
			statusText = newValue.toString();
		}

		if ( key == Kopete::Global::Properties::self()->statusMessage().key() )
			d->statusMessage.setMessage( statusText );
		else
			d->statusMessage.setTitle( statusText );

		emit statusMessageChanged( this );
	}

	// Here we abuse the onlineStatusChanged signal to force the contact list to refresh and hide the
	// contact if he is offline and his isAlwaysVisible property changed to false
	if ( key == Kopete::Global::Properties::self()->isAlwaysVisible().key() && oldValue != newValue )
		emit onlineStatusChanged( this, d->onlineStatus );
}

void MetaContact::moveToGroup( Group *from, Group *to )
{
	if ( !from || !groups().contains( from )  )
	{
		// We're adding, not moving, because 'from' is illegal
		addToGroup( to );
		return;
	}

	// Do nothing (same group)
	if ( from == to )
		return;

	if ( !to || groups().contains( to ) )
	{
		// We're removing, not moving, because 'to' is illegal
		removeFromGroup( from );
		return;
	}

	if ( to->type() == Group::Offline )
		return;

	if ( isTemporary() && to->type() != Group::Temporary )
		return;


	//kDebug( 14010 ) << from->displayName() << " => " << to->displayName();

	d->groups.removeAll( from );
	d->groups.append( to );

	QListIterator<Contact *> it(d->contacts);
	while( it.hasNext() )
		it.next()->sync(Contact::MovedBetweenGroup);

	emit movedToGroup( this, from, to );
}

void MetaContact::removeFromGroup( Group *group )
{
	if ( !group || !groups().contains( group ) || ( isTemporary() && group->type() == Group::Temporary ) )
	{
		return;
	}

	d->groups.removeAll( group );

	// make sure MetaContact is at least in one group
	if ( d->groups.isEmpty() )
	{
		d->groups.append( Group::topLevel() );
		emit addedToGroup( this, Group::topLevel() );
	}

	QListIterator<Contact *> it(d->contacts);
	while( it.hasNext() )
		it.next()->sync(Contact::MovedBetweenGroup);

	emit removedFromGroup( this, group );
}

void MetaContact::addToGroup( Group *to )
{
	if ( !to || groups().contains( to )  )
		return;

	if ( d->temporary && to->type() != Group::Temporary )
		return;

	// FIXME: This breaks copying MC with drag&drop from topLevel group
	if ( d->groups.contains( Group::topLevel() ) )
	{
		d->groups.removeAll( Group::topLevel() );
		emit removedFromGroup( this, Group::topLevel() );
	}

	d->groups.append( to );

	QListIterator<Contact *> it(d->contacts);
	while( it.hasNext() )
		it.next()->sync(Contact::MovedBetweenGroup);

	emit addedToGroup( this, to );
}

QList<Group *> MetaContact::groups() const
{
	return d->groups;
}

void MetaContact::slotContactDestroyed( Contact *contact )
{
	removeContact(contact,true);
}

QString MetaContact::addressBookField( Kopete::Plugin * /* p */, const QString &app, const QString & key ) const
{
	return d->addressBook[ app ][ key ];
}

void Kopete::MetaContact::setAddressBookField( Kopete::Plugin * /* p */, const QString &app, const QString &key, const QString &value )
{
	d->addressBook[ app ][ key ] = value;
}

Kopete::StatusMessage MetaContact::statusMessage() const
{
	return d->statusMessage;
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

void MetaContact::slotProtocolLoaded( Protocol *p )
{
	if( !p )
		return;

	ContactDataList dataList = pluginContactData( p );
	if ( !dataList.isEmpty() )
		p->deserializeContactList( this, dataList );
}

void MetaContact::slotAllPluginsLoaded()
{
	// Now that the plugins and subcontacts are loaded, set the source contact.
	setDisplayNameSourceContact( findContact( d->nameSourcePID, d->nameSourceAID, d->nameSourceCID) );
	setPhotoSourceContact( findContact( d->photoSourcePID, d->photoSourceAID, d->photoSourceCID) );
}

void MetaContact::slotUpdateAddressBookPicture()
{
	KABC::AddressBook* ab = KABCPersistence::self()->addressBook();
	QString id = kabcId();
	if ( !id.isEmpty() && !id.contains(':') )
	{
		KABC::Addressee theAddressee = ab->findByUid(id);
		if ( theAddressee.isEmpty() )
		{
			kDebug( 14010 ) << "no KABC::Addressee found for ( " << id << " ) " << " in current address book";
		}
		else
		{
			KABC::Picture pic = theAddressee.photo();
			if ( pic.data().isNull() && pic.url().isEmpty() )
				pic = theAddressee.logo();

			d->kabcPicture.setPicture(pic);
		}
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
		QListIterator<Group *> it(d->groups);
		while ( it.hasNext() )
		{
			Group *g = it.next();
			if(g != temporaryGroup)
				removeFromGroup(g);
		}
	}
	else
		moveToGroup(temporaryGroup, group ? group : Group::topLevel());
}

QString MetaContact::kabcId() const
{
	if(d->kabcId.isEmpty())
	{
		if(d->contacts.isEmpty())
			return QString();
		Contact *c=d->contacts.first();
		if(!c)
			return QString();
		return c->protocol()->pluginId()+QString::fromUtf8(":")+c->account()->accountId()+QString::fromUtf8(":") + c->contactId() ;
	}
	return d->kabcId;
}

void MetaContact::setKabcId( const QString& newKabcId )
{
	if(newKabcId == d->kabcId)
		return;

	// 1) Check the Id is not already used by another contact
	// 2) cause a kabc write ( only in response to metacontactLVIProps calling this, or will
	//      write be called twice when creating a brand new MC? )
	// 3) What about changing from one valid kabc to another, are kabc fields removed?
	// 4) May be called with Null to remove an invalid kabc uid by KMC::toKABC()
	// 5) Is called when reading the saved contact list

	// Don't remove IM addresses from kabc if we are changing contacts;
	// other programs may have written that data and depend on it
	d->kabcId = newKabcId;
	if ( loading() )
	{
		// TODO: speed up: this slot is called in *every* metacontact instance and is slow because it finding kabc id
		slotUpdateAddressBookPicture();
	}
	else
	{
		KABCPersistence::self()->write( this );
		emit onlineStatusChanged( this, d->onlineStatus );
		emit persistentDataChanged();
	}
}

bool MetaContact::isPhotoSyncedWithKABC() const
{
	return d->photoSyncedWithKABC;
}

void MetaContact::setPhotoSyncedWithKABC(bool b)
{
	d->photoSyncedWithKABC=b;
	if( b && !loading() )
	{
		QVariant newValue;

		switch( photoSource() )
		{
			case SourceContact:
			{
				Contact *source = photoSourceContact();
				if(source != 0L)
					newValue = source->property( Kopete::Global::Properties::self()->photo() ).value();
				break;
			}
			case SourceCustom:
			{
				if( !d->customPicture.isNull() )
					newValue = d->customPicture.path();
				break;
			}
			// Don't sync the photo with KABC if the source is KABC !
			default:
				return;
		}

		if ( !d->kabcId.isEmpty() && !newValue.isNull())
		{
			KABC::Addressee theAddressee = KABCPersistence::self()->addressBook()->findByUid( kabcId() );

			if ( !theAddressee.isEmpty() )
			{
				QImage img;
				if(newValue.canConvert( QVariant::Image ))
					img=newValue.value<QImage>();
				else if(newValue.canConvert( QVariant::Pixmap ))
					img=newValue.value<QPixmap>().toImage();

				if(img.isNull())
				{
					// Some protocols like MSN save the photo as a url in
					// contact properties, we should not use this url
					// to sync with kabc but try first to embed the
					// photo data in the kabc addressee, because it could
					// be remote resource and the local url makes no sense
					QImage fallBackImage = QImage(newValue.toString());
					if(fallBackImage.isNull())
						theAddressee.setPhoto(newValue.toString());
					else
						theAddressee.setPhoto(fallBackImage);
				}
				else
					theAddressee.setPhoto(img);

				KABCPersistence::self()->addressBook()->insertAddressee(theAddressee);
				KABCPersistence::self()->writeAddressBook( theAddressee.resource() );
			}

		}
	}
}

QList<Contact *> MetaContact::contacts() const
{
	return d->contacts;
}

void MetaContact::onlineStatusNotification( Kopete::Contact * c )
{
	// comparing the status of the previous and new preferred contact is the determining factor in deciding to notify
	Kopete::OnlineStatus newNotifyOnlineStatus;

	Kopete::Contact * pc = preferredContact();
	if ( pc )
		newNotifyOnlineStatus = pc->onlineStatus();
	else // the last child contact has gone offline or otherwise unreachable, so take the changed contact's online status
		newNotifyOnlineStatus = c->onlineStatus();

	// ensure we are not suppressing notifications, because connecting or disconnected
	if ( !c->account()->suppressStatusNotification() && c->account()->isConnected()
	     && c->account()->myself()->onlineStatus().status() != OnlineStatus::Connecting
	     && c->account()->myself()->onlineStatus().status() != OnlineStatus::Busy
	     && (Kopete::BehaviorSettings::self()->enableEventsWhileAway() || !c->account()->isAway()) )
	{
		// figure out what's happened
		enum ChangeType { noChange, noEvent, signedIn, changedStatus, signedOut };
		ChangeType t = noChange;

		// first, exclude changes due to blocking or subscription changes at the protocol level
		if ( d->notifyOnlineStatus.status() == Kopete::OnlineStatus::Unknown || newNotifyOnlineStatus.status() == Kopete::OnlineStatus::Unknown )
		{
			t = noEvent; // This means the contact's changed from or to unknown - due to a protocol state change, not a contact state change
		}
		else
		{ // we're dealing with a genuine contact state change
			if ( d->notifyOnlineStatus.status() == Kopete::OnlineStatus::Offline )
			{
				if ( newNotifyOnlineStatus.status() != Kopete::OnlineStatus::Offline )
				{
					t = signedIn;	// contact has gone from offline to something else, it's a sign-in
				}
			}
			else if ( d->notifyOnlineStatus.isDefinitelyOnline()) //Is that correct?
			{
				if ( newNotifyOnlineStatus.status() == Kopete::OnlineStatus::Offline )
				{
					t = signedOut;	// contact has gone from an online state to an offline state, it's a sign out
				}
				else if ( d->notifyOnlineStatus > newNotifyOnlineStatus || d->notifyOnlineStatus < newNotifyOnlineStatus ) // operator!= is useless because it's an identity operator, not an equivalence operator
				{
					// contact has changed online states, it's a status change,
					// and the preferredContact changed status, or there is a new preferredContacat
					// so it's worth notifying
					t = changedStatus;
				}
			}
			else if ( d->notifyOnlineStatus != newNotifyOnlineStatus )
			{
				// catch-all for any other status change we don't know about
				t = noEvent;
			}
		}

		// now issue the appropriate notification
		KNotification *notify = 0;
		switch ( t )
		{
		case noEvent:
		case noChange:
			break;
		case signedIn:
			notify = new KNotification( QString("kopete_contact_online"), Kopete::UI::Global::mainWidget() );
			notify->setActions( QStringList( i18nc("@action", "Chat") ) );
			break;
		case changedStatus:
			notify = new KNotification( QString("kopete_contact_status_change"), Kopete::UI::Global::mainWidget() );
			notify->setActions( QStringList( i18nc("@action", "Chat") ) );
			break;
		case signedOut:
			notify = new KNotification( QString("kopete_contact_offline"), Kopete::UI::Global::mainWidget() );
			break;
		}

		if( notify )
		{
			QString text = i18n( "<qt><i>%1</i> is now %2.</qt>",
			                     Kopete::Emoticons::parseEmoticons( Qt::escape( displayName() ) ),
			                     Qt::escape( c->onlineStatus().description() ) );

			notify->setText( text );
			notify->setPixmap( QPixmap::fromImage( picture().image() ) );
			connect( notify, SIGNAL(activated(uint)) , this, SLOT(execute()) );

			notify->addContext( qMakePair( QString::fromLatin1("contact"), metaContactId().toString() ) );
			foreach( Kopete::Group *g , groups() )
			{
				notify->addContext( qMakePair( QString::fromLatin1("group") , QString::number( g->groupId() ) ) );
			}
			notify->sendEvent();
		}
	}
	d->notifyOnlineStatus = newNotifyOnlineStatus;
}


} //END namespace Kopete

#include "kopetemetacontact.moc"

// vim: set noet ts=4 sts=4 sw=4:
