/*
    kopetemetacontact.cpp - Kopete Meta Contact

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart@ kde.org>
    Copyright (c) 2002-2004 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2005      by Michaël Larouche       <michael.larouche@kdemail.net>

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
#include "kopetepicture.h"

namespace Kopete {

// this is just to save typing
const QString NSCID_ELEM = QString::fromUtf8("nameSourceContactId" );
const QString NSPID_ELEM = QString::fromUtf8( "nameSourcePluginId" );
const QString NSAID_ELEM = QString::fromUtf8( "nameSourceAccountId" );
const QString PSCID_ELEM = QString::fromUtf8( "photoSourceContactId" );
const QString PSPID_ELEM = QString::fromUtf8( "photoSourcePluginId" );
const QString PSAID_ELEM = QString::fromUtf8( "photoSourceAccountId" );

class  MetaContact::Private
{ public:
	Private() :
		photoSource(MetaContact::SourceCustom), displayNameSource(MetaContact::SourceCustom),
		displayNameSourceContact(0L),  photoSourceContact(0L), temporary(false),
		onlineStatus(Kopete::OnlineStatus::Offline), photoSyncedWithKABC(false)
	{}

	~Private()
	{}

	QPtrList<Contact> contacts;

	// property sources	
	PropertySource photoSource;
	PropertySource displayNameSource;

	// when source is contact
	Contact *displayNameSourceContact;
	Contact *photoSourceContact;
	
	// used when source is kabc
	QString metaContactId;
	
	// used when source is custom
	QString displayName;
	KURL photoUrl;

	QPtrList<Group> groups;
	QMap<QString, QMap<QString, QString> > addressBook;
	bool temporary;
	
	OnlineStatus::StatusType onlineStatus;
	bool photoSyncedWithKABC;

	// Used to set contact source at load.
	QString nameSourcePID, nameSourceAID, nameSourceCID;
	QString photoSourcePID, photoSourceAID, photoSourceCID;

	// The photo cache. Reduce disk access and CPU usage.
	Picture customPicture, contactPicture, kabcPicture;
};

MetaContact::MetaContact()
	: ContactListElement( ContactList::self() )
{
	d = new Private;

	connect( this, SIGNAL( pluginDataChanged() ), SIGNAL( persistentDataChanged() ) );
	connect( this, SIGNAL( iconChanged( Kopete::ContactListElement::IconState, const QString & ) ), SIGNAL( persistentDataChanged() ) );
	connect( this, SIGNAL( useCustomIconChanged( bool ) ), SIGNAL( persistentDataChanged() ) );
	connect( this, SIGNAL( displayNameChanged( const QString &, const QString & ) ), SIGNAL( persistentDataChanged() ) );
	connect( this, SIGNAL( movedToGroup( Kopete::MetaContact *, Kopete::Group *, Kopete::Group * ) ), SIGNAL( persistentDataChanged() ) );
	connect( this, SIGNAL( removedFromGroup( Kopete::MetaContact *, Kopete::Group * ) ), SIGNAL( persistentDataChanged() ) );
	connect( this, SIGNAL( addedToGroup( Kopete::MetaContact *, Kopete::Group * ) ), SIGNAL( persistentDataChanged() ) );
	connect( this, SIGNAL( contactAdded( Kopete::Contact * ) ), SIGNAL( persistentDataChanged() ) );
	connect( this, SIGNAL( contactRemoved( Kopete::Contact * ) ), SIGNAL( persistentDataChanged() ) );

	// Update the KABC picture when the KDE Address book change.
	connect(KABCPersistence::self()->addressBook(), SIGNAL(addressBookChanged(AddressBook *)), this, SLOT(slotUpdateAddressBookPicture()));

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

		emit contactAdded(c);

		updateOnlineStatus();
		
		// if this is the first contact, probbaly was created by a protocol
		// so it has empty custom properties, then set sources to the contact
		if ( d->contacts.count() == 1 )
		{
			if ( displayName().isEmpty() )
			{
				setDisplayNameSourceContact(c);
				setDisplayNameSource(SourceContact);
			}
			if ( photo().isNull() )
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

	for ( QPtrListIterator<Contact> it( d->contacts ); it.current(); ++it )
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
		bool wasTrackingName = ( !displayNameSourceContact() && (displayNameSource() == SourceContact) );
		bool wasTrackingPhoto = ( !photoSourceContact() && (photoSource() == SourceContact) );
		// save for later use
		QString currDisplayName = displayName();

		d->contacts.remove( c );
		
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
	QPtrListIterator<Contact> it( d->contacts );
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

void MetaContact::setDisplayNameSource(PropertySource source)
{
	QString oldName = displayName();
	d->displayNameSource = source;
	QString newName = displayName();
	if ( oldName != newName)
		emit displayNameChanged( oldName, newName );
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

		The prefered contact is choose with the following criterias:  (in that order)
		1) If a contact was an open chatwindow already, we will use that one.
		2) The contact with the better online status is used. But if that
		    contact is not reachable, we prefer return no contact.
		3) If all the criterias aboxe still gives ex-eaquo, we use the preffered
		    account as selected in the account preferances (with the arrows)
	*/

	Contact *contact = 0;
	bool hasOpenView=false; //has the selected contact already an open chatwindow
	for ( QPtrListIterator<Contact> it( d->contacts ); it.current(); ++it )
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
	QPtrListIterator<Contact> it( d->contacts );
	for( ; it.current(); ++it )
	{
		unsigned long int i = it.current()->idleTime();
		if( it.current()->isOnline() && i < time || time == 0 )
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
				return QString::fromUtf8( "metacontact_online" );
		case OnlineStatus::Away:
			if( useCustomIcon() )
				return icon( ContactListElement::Away );
			else
				return QString::fromUtf8( "metacontact_away" );

		case OnlineStatus::Unknown:
			if( useCustomIcon() )
				return icon( ContactListElement::Unknown );
			if ( d->contacts.isEmpty() )
				return QString::fromUtf8( "metacontact_unknown" );
			else
				return QString::fromUtf8( "metacontact_offline" );

		case OnlineStatus::Offline:
		default:
			if( useCustomIcon() )
				return icon( ContactListElement::Offline );
			else
				return QString::fromUtf8( "metacontact_offline" );
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
	QPtrListIterator<Contact> it( d->contacts );
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

	for ( QPtrListIterator<Contact> it( d->contacts ); it.current(); ++it )
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

	QPtrListIterator<Contact> it( d->contacts );
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
	for( QPtrListIterator<Contact> it( d->contacts ) ; it.current(); ++it )
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

	emit displayNameChanged( old , name );

	for( QPtrListIterator<Kopete::Contact> it( d->contacts ) ; it.current(); ++it )
		( *it )->sync(Contact::DisplayNameChanged);

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
		if ( !metaContactId().isEmpty() )
			return nameFromKABC(metaContactId());
	}
	else if ( source == SourceContact )
	{
		if ( d->displayNameSourceContact==0 )
		{
			if( d->contacts.count() >= 1 )
			{// don't call setDisplayNameSource , or there will probably be an infinite loop
				d->displayNameSourceContact=d->contacts.first();
//				kdDebug( 14010 ) << k_funcinfo << " setting displayname source for " << metaContactId()  << endl;
			}
		}
		if ( displayNameSourceContact() != 0L )
		{
			return nameFromContact(displayNameSourceContact());
		}
		else
		{
//			kdDebug( 14010 ) << k_funcinfo << " source == SourceContact , but there is no displayNameSourceContact for contact " << metaContactId() << endl;
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
			kdDebug( 14010 ) << k_funcinfo << "no KABC::Addressee found for ( " << id << " ) " << " in current address book" << endl;
		}
		else
		{
			return theAddressee.formattedName();
		}
	}
	// no kabc association, return null image
	return QString::null;
}

QString nameFromContact( Kopete::Contact *c) /*const*/
{
	if ( !c ) 
		return QString::null;

	QString contactName;
	if ( c->hasProperty( Kopete::Global::Properties::self()->nickName().key() ) )
		contactName = c->property( Global::Properties::self()->nickName()).value().toString();

				//the replace is there to workaround the Bug 95444
	return contactName.isEmpty() ? c->contactId() : contactName.replace('\n',QString::fromUtf8(""));
}

KURL MetaContact::customPhoto() const
{
	return d->photoUrl;
}

void MetaContact::setPhoto( const KURL &url )
{
	d->photoUrl = url;
	d->customPicture.setPicture(url.path());

	if ( photoSource() == SourceCustom )
	{
		emit photoChanged();
	}
}

QImage MetaContact::photo() const
{
  if( picture().image().width() > 96 && picture().image().height() > 96 )
    {
      kdDebug( 14010 )  << k_funcinfo << "Resizing image from " << picture().image().width() << " x " << picture().image().height() << endl;
      return picture().image().smoothScale(96,96,QImage::ScaleMin);
    }
  else
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
	if(photoProp.canCast( QVariant::Image ))
		img=photoProp.toImage();
	else if(photoProp.canCast( QVariant::Pixmap ))
		img=photoProp.toPixmap().convertToImage();
	else if(!photoProp.asString().isEmpty())
	{
		img=QPixmap( photoProp.toString() ).convertToImage();
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
			kdDebug( 14010 ) << k_funcinfo << "no KABC::Addressee found for ( " << id << " ) " << " in current address book" << endl;
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
	if ( displayNameSource() == SourceContact )
	{
		emit displayNameChanged( nameFromContact(old), nameFromContact(contact));
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
			photoProp = contact->property( Kopete::Global::Properties::self()->photo().key() ).value();

		if(photoProp.canCast( QVariant::Image ))
			d->contactPicture.setPicture(photoProp.toImage());
		else if(photoProp.canCast( QVariant::Pixmap ))
			d->contactPicture.setPicture(photoProp.toPixmap().convertToImage());
		else if(!photoProp.asString().isEmpty())
		{
			d->contactPicture.setPicture(photoProp.toString());
		}
	}

	if ( photoSource() == SourceContact )
	{
		emit photoChanged();
	}
}

void MetaContact::slotPropertyChanged( Contact* subcontact, const QString &key,
		const QVariant &oldValue, const QVariant &newValue  )
{
	if ( displayNameSource() == SourceContact )
	{
		if( key == Global::Properties::self()->nickName().key() )
		{
			if (displayNameSourceContact() == subcontact)
			{
				emit displayNameChanged( oldValue.toString(), newValue.toString());
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

	if (photoSource() == SourceContact)
	{
		if ( key == Global::Properties::self()->photo().key() )
		{
			if (photoSourceContact() != subcontact)
			{
				// HACK the displayName that changed is not from the contact we are tracking, but
				// as the current one is null, lets use this new one
				if (photo().isNull())
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

QPtrList<Group> MetaContact::groups() const
{
	return d->groups;
}

void MetaContact::slotContactDestroyed( Contact *contact )
{
	removeContact(contact,true);
}

const QDomElement MetaContact::toXML(bool minimal)
{
	// This causes each Kopete::Protocol subclass to serialise its contacts' data into the metacontact's plugin data and address book data
	emit aboutToSave(this);

	QDomDocument metaContact;
	metaContact.appendChild( metaContact.createElement( QString::fromUtf8( "meta-contact" ) ) );
	metaContact.documentElement().setAttribute( QString::fromUtf8( "contactId" ), metaContactId() );

	// the custom display name, used for the custom name source
	QDomElement displayName = metaContact.createElement( QString::fromUtf8("display-name" ) );
	displayName.appendChild( metaContact.createTextNode( d->displayName ) );
	metaContact.documentElement().appendChild( displayName );
	QDomElement photo = metaContact.createElement( QString::fromUtf8("photo" ) );
	KURL photoUrl = d->photoUrl;
	photo.appendChild( metaContact.createTextNode( photoUrl.url() ) );
	metaContact.documentElement().appendChild( photo );

	// Property sources
	QDomElement propertySources = metaContact.createElement( QString::fromUtf8("property-sources" ) );
	QDomElement _nameSource = metaContact.createElement( QString::fromUtf8("name") );
	QDomElement _photoSource = metaContact.createElement( QString::fromUtf8("photo") );

	// set the contact source for display name
	_nameSource.setAttribute(QString::fromUtf8("source"), sourceToString(displayNameSource()));
	
	// set contact source metadata
	if (displayNameSourceContact())
	{
		QDomElement contactNameSource = metaContact.createElement( QString::fromUtf8("contact-source") );
		contactNameSource.setAttribute( NSCID_ELEM, displayNameSourceContact()->contactId() );
		contactNameSource.setAttribute( NSPID_ELEM, displayNameSourceContact()->protocol()->pluginId() );
		contactNameSource.setAttribute( NSAID_ELEM, displayNameSourceContact()->account()->accountId() );
		_nameSource.appendChild( contactNameSource );
	}

	// set the contact source for photo
	_photoSource.setAttribute(QString::fromUtf8("source"), sourceToString(photoSource()));

	if( !d->metaContactId.isEmpty()  )
		photo.setAttribute( QString::fromUtf8("syncWithKABC") , QString::fromUtf8( d->photoSyncedWithKABC ? "true" : "false" ) );

	if (photoSourceContact())
	{
		//kdDebug(14010) << k_funcinfo << "serializing photo source " << nameFromContact(photoSourceContact()) << endl;
		// set contact source metadata for photo
		QDomElement contactPhotoSource = metaContact.createElement( QString::fromUtf8("contact-source") );
		contactPhotoSource.setAttribute( NSCID_ELEM, photoSourceContact()->contactId() );
		contactPhotoSource.setAttribute( NSPID_ELEM, photoSourceContact()->protocol()->pluginId() );
		contactPhotoSource.setAttribute( NSAID_ELEM, photoSourceContact()->account()->accountId() );
		_photoSource.appendChild( contactPhotoSource );
	}
	// apend name and photo sources to property sources
	propertySources.appendChild(_nameSource);
	propertySources.appendChild(_photoSource);
	
	metaContact.documentElement().appendChild(propertySources);

	// Don't store these information in minimal mode.
	if(!minimal)
	{
		// Store groups
		if ( !d->groups.isEmpty() )
		{
			QDomElement groups = metaContact.createElement( QString::fromUtf8("groups") );
			Group *g;
			for ( g = d->groups.first(); g; g = d->groups.next() )
			{
				QDomElement group = metaContact.createElement( QString::fromUtf8("group") );
				group.setAttribute( QString::fromUtf8("id"), g->groupId() );
				groups.appendChild( group );
			}
			metaContact.documentElement().appendChild( groups );
		}
	
		// Store other plugin data
		QValueList<QDomElement> pluginData = Kopete::ContactListElement::toXML();
		for( QValueList<QDomElement>::Iterator it = pluginData.begin(); it != pluginData.end(); ++it )
			metaContact.documentElement().appendChild( metaContact.importNode( *it, true ) );
	
		// Store custom notification data
		QDomElement notifyData = NotifyDataObject::notifyDataToXML();
		if ( notifyData.hasChildNodes() )
			metaContact.documentElement().appendChild( metaContact.importNode( notifyData, true ) );
	}
	return metaContact.documentElement();
}

bool MetaContact::fromXML( const QDomElement& element )
{
	if( !element.hasChildNodes() )
		return false;

	bool oldPhotoTracking = false;
	bool oldNameTracking = false;

	QString strContactId = element.attribute( QString::fromUtf8("contactId") );
	if( !strContactId.isEmpty() )
	{
		d->metaContactId = strContactId;
		// Set the KABC Picture
		slotUpdateAddressBookPicture();
	}

	QDomElement contactElement = element.firstChild().toElement();
	while( !contactElement.isNull() )
	{
		
		if( contactElement.tagName() == QString::fromUtf8( "display-name" ) )
		{ // custom display name, used for the custom name source
			
			// WTF, why were we not loading the metacontact if nickname was empty.
			//if ( contactElement.text().isEmpty() )
			//	return false;
			
			//the replace is there to workaround the Bug 95444
			d->displayName = contactElement.text().replace('\n',QString::fromUtf8(""));

			if ( contactElement.hasAttribute(NSCID_ELEM) && contactElement.hasAttribute(NSPID_ELEM) && contactElement.hasAttribute(NSAID_ELEM))
			{
				oldNameTracking = true;
				//kdDebug(14010) << k_funcinfo << "old name tracking" << endl;
				// retrieve deprecated data (now stored in property-sources)
				// save temporarely, we will find a Contact* with this later
				d->nameSourceCID = contactElement.attribute( NSCID_ELEM );
				d->nameSourcePID = contactElement.attribute( NSPID_ELEM );
				d->nameSourceAID = contactElement.attribute( NSAID_ELEM );
			}
// 			else
// 				kdDebug(14010) << k_funcinfo << "no old name tracking" << endl;
		}
		else if( contactElement.tagName() == QString::fromUtf8( "photo" ) )
		{
			// custom photo, used for custom photo source
			setPhoto( KURL(contactElement.text()) );

			d->photoSyncedWithKABC = (contactElement.attribute(QString::fromUtf8("syncWithKABC")) == QString::fromUtf8("1")) || (contactElement.attribute(QString::fromUtf8("syncWithKABC")) == QString::fromUtf8("true"));

			// retrieve deprecated data (now stored in property-sources)
			// save temporarely, we will find a Contact* with this later
			if ( contactElement.hasAttribute(PSCID_ELEM) && contactElement.hasAttribute(PSPID_ELEM) && contactElement.hasAttribute(PSAID_ELEM))
			{
				oldPhotoTracking = true;
// 				kdDebug(14010) << k_funcinfo << "old photo tracking" << endl;
				d->photoSourceCID = contactElement.attribute( PSCID_ELEM );
				d->photoSourcePID = contactElement.attribute( PSPID_ELEM );
				d->photoSourceAID = contactElement.attribute( PSAID_ELEM );
			}
// 			else
// 				kdDebug(14010) << k_funcinfo << "no old photo tracking" << endl;
		}
		else if( contactElement.tagName() == QString::fromUtf8( "property-sources" ) )
		{
			QDomNode property = contactElement.firstChild();
			while( !property.isNull() )
			{
				QDomElement propertyElement = property.toElement();

				if( propertyElement.tagName() == QString::fromUtf8( "name" ) )
				{
					QString source = propertyElement.attribute( QString::fromUtf8("source") );
					setDisplayNameSource(stringToSource(source));
					// find contact sources now.
					QDomNode propertyParam = propertyElement.firstChild();
					while( !propertyParam.isNull() )
					{
						QDomElement propertyParamElement = propertyParam.toElement();
						if( propertyParamElement.tagName() == QString::fromUtf8( "contact-source" ) )
						{
							d->nameSourceCID = propertyParamElement.attribute( NSCID_ELEM );
							d->nameSourcePID = propertyParamElement.attribute( NSPID_ELEM );
							d->nameSourceAID = propertyParamElement.attribute( NSAID_ELEM );
						}
						propertyParam = propertyParam.nextSibling();
					}
				}
				if( propertyElement.tagName() == QString::fromUtf8( "photo" ) )
				{
					QString source = propertyElement.attribute( QString::fromUtf8("source") );
					setPhotoSource(stringToSource(source));
					// find contact sources now.
					QDomNode propertyParam = propertyElement.firstChild();
					while( !propertyParam.isNull() )
					{
						QDomElement propertyParamElement = propertyParam.toElement();
						if( propertyParamElement.tagName() == QString::fromUtf8( "contact-source" ) )
						{
							d->photoSourceCID = propertyParamElement.attribute( NSCID_ELEM );
							d->photoSourcePID = propertyParamElement.attribute( NSPID_ELEM );
							d->photoSourceAID = propertyParamElement.attribute( NSAID_ELEM );
						}
						propertyParam = propertyParam.nextSibling();
					}
				}
				property = property.nextSibling();
			}
		}
		else if( contactElement.tagName() == QString::fromUtf8( "groups" ) )
		{
			QDomNode group = contactElement.firstChild();
			while( !group.isNull() )
			{
				QDomElement groupElement = group.toElement();

				if( groupElement.tagName() == QString::fromUtf8( "group" ) )
				{
					QString strGroupId = groupElement.attribute( QString::fromUtf8("id") );
					if( !strGroupId.isEmpty() )
						addToGroup( Kopete::ContactList::self()->group( strGroupId.toUInt() ) );
					else //kopete 0.6 contactlist
						addToGroup( Kopete::ContactList::self()->findGroup( groupElement.text() ) );
				}
				else if( groupElement.tagName() == QString::fromUtf8( "top-level" ) ) //kopete 0.6 contactlist
					addToGroup( Kopete::Group::topLevel() );

				group = group.nextSibling();
			}
		}
		else if( contactElement.tagName() == QString::fromUtf8( "address-book-field" ) )
		{
			QString app = contactElement.attribute( QString::fromUtf8( "app" ), QString::null );
			QString key = contactElement.attribute( QString::fromUtf8( "key" ), QString::null );
			QString val = contactElement.text();
			d->addressBook[ app ][ key ] = val;
		}
		else if( contactElement.tagName() == QString::fromUtf8( "custom-notifications" ) )
		{
			Kopete::NotifyDataObject::notifyDataFromXML( contactElement );
		}
		else //if( groupElement.tagName() == QString::fromUtf8( "plugin-data" ) || groupElement.tagName() == QString::fromUtf8("custom-icons" ))
		{
			Kopete::ContactListElement::fromXML(contactElement);
		}
		contactElement = contactElement.nextSibling().toElement();
	}

	if( oldNameTracking )
	{
		/* if (displayNameSourceContact() )  <- doesn't work because the contact is only set up when all plugin are loaded (BUG 111956) */
		if ( !d->nameSourceCID.isEmpty() )
		{
// 			kdDebug(14010) << k_funcinfo << "Converting old name source" << endl;
			// even if the old tracking attributes exists, they could have been null, that means custom
				setDisplayNameSource(SourceContact);
		}
		else
		{
			// lets do the best conversion for the old name tracking
			// if the custom display name is the same as kabc name, set the source to kabc
			if ( !d->metaContactId.isEmpty() && ( d->displayName == nameFromKABC(d->metaContactId)) )
				setDisplayNameSource(SourceKABC);
			else
				setDisplayNameSource(SourceCustom);
		}
	}

	if ( oldPhotoTracking )
	{
// 		kdDebug(14010) << k_funcinfo << "Converting old photo source" << endl;
		if ( !d->photoSourceCID.isEmpty() )   
		{
			setPhotoSource(SourceContact);
		}
		else
		{
			if ( !d->metaContactId.isEmpty() && !photoFromKABC(d->metaContactId).isNull())
				setPhotoSource(SourceKABC);
			else
				setPhotoSource(SourceCustom);
		}
	}
	
	// If a plugin is loaded, load data cached
	connect( Kopete::PluginManager::self(), SIGNAL( pluginLoaded(Kopete::Plugin*) ),
		this, SLOT( slotPluginLoaded(Kopete::Plugin*) ) );

	// All plugins are already loaded, call manually the contact setting slot.
	if( Kopete::PluginManager::self()->isAllPluginsLoaded() )
		slotAllPluginsLoaded();
	else
		// When all plugins are loaded, set the source contact.
		connect( Kopete::PluginManager::self(), SIGNAL( allPluginsLoaded() ), 
			this, SLOT( slotAllPluginsLoaded() ) );

	// track changes only works if ONE Contact is inside the MetaContact
//	if (d->contacts.count() > 1) // Does NOT work as intended
//		d->trackChildNameChanges=false;

// 	kdDebug(14010) << k_funcinfo << "END" << endl;
	return true;
}

QString MetaContact::sourceToString(PropertySource source) const
{
	if ( source == SourceCustom )
		return QString::fromUtf8("custom");
	else if ( source == SourceKABC )
		return QString::fromUtf8("addressbook");
	else if ( source == SourceContact )
		return QString::fromUtf8("contact");
	else // recovery
		return sourceToString(SourceCustom);
}

MetaContact::PropertySource MetaContact::stringToSource(const QString &name) const
{
	if ( name == QString::fromUtf8("custom") )
		return SourceCustom;
	else if ( name == QString::fromUtf8("addressbook") )
		return SourceKABC;
	else if ( name == QString::fromUtf8("contact") )
		return SourceContact;
	else // recovery
		return SourceCustom;
}

QString MetaContact::addressBookField( Kopete::Plugin * /* p */, const QString &app, const QString & key ) const
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

void MetaContact::slotAllPluginsLoaded()
{
	// Now that the plugins and subcontacts are loaded, set the source contact.
	setDisplayNameSourceContact( findContact( d->nameSourcePID, d->nameSourceAID, d->nameSourceCID) ); 
	setPhotoSourceContact( findContact( d->photoSourcePID, d->photoSourceAID, d->photoSourceCID) );
}

void MetaContact::slotUpdateAddressBookPicture()
{
	KABC::AddressBook* ab = KABCPersistence::self()->addressBook();
	QString id = metaContactId();
	if ( !id.isEmpty() && !id.contains(':') )
	{
		KABC::Addressee theAddressee = ab->findByUid(id);
		if ( theAddressee.isEmpty() )
		{
			kdDebug( 14010 ) << k_funcinfo << "no KABC::Addressee found for ( " << id << " ) " << " in current address book" << endl;
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
		return c->protocol()->pluginId()+QString::fromUtf8(":")+c->account()->accountId()+QString::fromUtf8(":") + c->contactId() ;
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

		if ( !d->metaContactId.isEmpty() && !newValue.isNull())
		{
			KABC::Addressee theAddressee = KABCPersistence::self()->addressBook()->findByUid( metaContactId() );

			if ( !theAddressee.isEmpty() )
			{
				QImage img;
				if(newValue.canCast( QVariant::Image ))
					img=newValue.toImage();
				else if(newValue.canCast( QVariant::Pixmap ))
					img=newValue.toPixmap().convertToImage();

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

QPtrList<Contact> MetaContact::contacts() const
{
	return d->contacts;
}
} //END namespace Kopete

#include "kopetemetacontact.moc"

// vim: set noet ts=4 sts=4 sw=4:
