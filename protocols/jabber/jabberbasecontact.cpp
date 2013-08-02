 /*
  * jabbercontact.cpp  -  Base class for the Kopete Jabber protocol contact
  *
  * Copyright (c) 2002-2004 by Till Gerken <till@tantalo.net>
  * Copyright (c)      2006 by Olivier Goffart <ogoffart at kde.org>
  *
  * Kopete    (c) by the Kopete developers  <kopete-devel@kde.org>
  *
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either version 2 of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */

#include "jabberbasecontact.h"

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <qtimer.h>
#include <qimage.h>
#include <qregexp.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>


#include <kopetegroup.h>
#include <kopetecontactlist.h>
#include <kopeteavatarmanager.h>

#include "xmpp_tasks.h"

#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabberresource.h"
#include "jabberresourcepool.h"
#include "jabbercapabilitiesmanager.h"
#include "kopetemetacontact.h"
#include "kopetemessage.h"
#include "kopeteuiglobal.h"
#include "jabbertransport.h"
#include "dlgjabbervcard.h"


/**
 * JabberBaseContact constructor
 */
JabberBaseContact::JabberBaseContact (const XMPP::RosterItem &rosterItem, Kopete::Account *account, Kopete::MetaContact * mc, const QString &legacyId)
	: Kopete::Contact (account, legacyId.isEmpty() ? rosterItem.jid().full() : legacyId , mc )
{
	mSendsDeliveredEvent = false;
	setDontSync ( false );
	
	JabberTransport *t=transport();
	m_account= t ? t->account() : static_cast<JabberAccount *>(Kopete::Contact::account());


	// take roster item and update display name
	updateContact ( rosterItem );

}


JabberProtocol *JabberBaseContact::protocol ()
{

	return static_cast<JabberProtocol *>(Kopete::Contact::protocol ());
}


JabberTransport * JabberBaseContact::transport( )
{
	return dynamic_cast<JabberTransport*>(Kopete::Contact::account());
}


/* Return if we are reachable (defaults to true because
   we can send on- and offline, only return false if the
   account itself is offline, too) */
bool JabberBaseContact::isReachable ()
{
	if (account()->isConnected())
		return true;

	return false;

}

void JabberBaseContact::updateContact ( const XMPP::RosterItem & item )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "Synchronizing local copy of " << contactId() << " with information received from server.  (name='" << item.name() << "' groups='" << item.groups() << "')";

	mRosterItem = item;

	// if we don't have a meta contact yet, stop processing here
	if ( !metaContact () )
		return;

	/*
	 * We received the information from the server, as such,
	 * don't attempt to synch while we update our local copy.
	 */
	setDontSync ( true );

	/*
	 * Set the contact's subscription status
	 */
	switch ( item.subscription().type () )
	{
		case XMPP::Subscription::None:
			setProperty ( protocol()->propSubscriptionStatus,
						  i18n ( "You cannot see each others' status." ) );
			break;
		case XMPP::Subscription::To:
			setProperty ( protocol()->propSubscriptionStatus,
						  i18n ( "You can see this contact's status, but he/she cannot see your status." ) );
			break;
		case XMPP::Subscription::From:
			setProperty ( protocol()->propSubscriptionStatus,
						  i18n ( "This contact can see your status, but you cannot see his/her status." ) );
			break;
		case XMPP::Subscription::Both:
			setProperty ( protocol()->propSubscriptionStatus,
						  i18n ( "You can see each others' status." ) );
			break;
	}

	if( !metaContact()->isTemporary() )
	{
		/*
		* In this method, as opposed to KC::syncGroups(),
		* the group list from the server is authoritative.
		* As such, we need to find a list of all groups
		* that the meta contact resides in but does not
		* reside in on the server anymore, as well as all
		* groups that the meta contact does not reside in,
		* but resides in on the server.
		* Then, we'll have to synchronize the KMC using
		* that information.
		*/
		Kopete::GroupList groupsToRemoveFrom, groupsToAddTo;
	
		// find all groups our contact is in but that are not in the server side roster
		for ( int i = 0; i < metaContact()->groups().count (); i++ )
		{
			Kopete::Group *gr = metaContact()->groups().at ( i );
			if ( gr->type () == Kopete::Group::Normal && !item.groups().contains ( gr->displayName () ) )
				groupsToRemoveFrom.append ( gr );
			else if ( gr->type () == Kopete::Group::TopLevel && !item.groups().contains ( QString () ) )
				groupsToRemoveFrom.append ( gr );
		}
	
		// now find all groups that are in the server side roster but not in the local group
		for ( int i = 0; i < item.groups().count (); i++ )
		{
			bool found = false;
			for ( int j = 0; j < metaContact()->groups().count (); j++)
			{
				Kopete::Group *gr = metaContact()->groups().at ( j );
				if ( gr->type () == Kopete::Group::Normal && gr->displayName () == item.groups().at(i) )
				{
					found = true;
					break;
				}
				else if ( gr->type () == Kopete::Group::TopLevel && item.groups().at(i).isEmpty() )
				{
					found = true;
					break;
				}
			}
			
			if ( !found )
			{
				if ( item.groups().at(i).isEmpty() )
					groupsToAddTo.append ( Kopete::Group::topLevel() );
				else
					groupsToAddTo.append ( Kopete::ContactList::self()->findGroup ( item.groups().at(i) ) );
			}
		}
	
		/*
		* Special case: if we don't add the contact to any group and the
		* list of groups to remove from contains the top level group, we
		* risk removing the contact from the visible contact list. In this
		* case, we need to make sure at least the top level group stays.
		*/
		if ( ( groupsToAddTo.count () == 0 ) && ( groupsToRemoveFrom.contains ( Kopete::Group::topLevel () ) ) )
		{
			groupsToRemoveFrom.removeAll ( Kopete::Group::topLevel () );
		}
	
		foreach ( Kopete::Group *group, groupsToRemoveFrom )
		{
			kDebug ( JABBER_DEBUG_GLOBAL ) << "Removing " << contactId() << " from group " << group->displayName ();
			metaContact()->removeFromGroup ( group );
		}
	
		foreach ( Kopete::Group *group, groupsToAddTo )
		{
			kDebug ( JABBER_DEBUG_GLOBAL ) << "Adding " << contactId() << " to group " << group->displayName ();
			metaContact()->addToGroup ( group );
		}

		//Set custom name from roster
		setCustomName ( item.name() );
	}

	/*
	 * Enable updates for the server again.
	 */
	setDontSync ( false );
	
	//can't do it now because it's called from contructor at a point some virtual function are not available
	QTimer::singleShot(0, this, SLOT(reevaluateStatus()));

}

void JabberBaseContact::updateResourceList ()
{
	/*
	 * Set available resources.
	 * This is a bit more complicated: We need to generate
	 * all images dynamically from the KOS icons and store
	 * them into the mime factory, then plug them into
	 * the richtext.
	 */
	JabberResourcePool::ResourceList resourceList;
	account()->resourcePool()->findResources ( rosterItem().jid() , resourceList );

	if ( resourceList.isEmpty () )
	{
		removeProperty ( protocol()->propAvailableResources );
		return;
	}

	QString resourceListStr = "<table cellspacing=\"0\">";

	for ( JabberResourcePool::ResourceList::iterator it = resourceList.begin (); it != resourceList.end (); ++it )
	{
		// icon, resource name and priority
		resourceListStr += QString ( "<tr><td><img src=\"kopete-onlinestatus-icon:%1\" /> <b>%2</b> (Priority: %3)</td></tr>" ).
						   arg ( protocol()->resourceToKOS((*it)->resource()).mimeSourceFor ( account () ),
								 (*it)->resource().name (), QString::number ( (*it)->resource().priority () ) );

		// client name, version, OS
		QString clientName = (*it)->clientName ();
		QString clientVersion = (*it)->clientVersion ();
		QString clientSystem = (*it)->clientSystem ();

		if ( clientName.isEmpty () )
		{
			clientName = account()->protocol()->capabilitiesManager()->clientName ( (*it)->jid () );
		}

		if ( clientVersion.isEmpty () )
		{
			clientVersion = account()->protocol()->capabilitiesManager()->clientVersion ( (*it)->jid () );
		}

		if ( !clientName.isEmpty () )
		{
			QString clientNameVersion = clientName;
			if ( !clientVersion.isEmpty () )
			{
				clientNameVersion += ' ' + clientVersion;
			}
			resourceListStr += QString ( "<tr><td>%1: %2" ).arg ( i18n ( "Client" ), clientNameVersion );
			if ( !clientSystem.isEmpty () )
			{
				resourceListStr += QString ( " (%1)" ).arg ( clientSystem );
			}
			resourceListStr += QString ( "</td></tr>" );
		}
		
		// Supported features
		QStringList features;
		QStringList supportedFeatures = (*it)->features().list();
		QStringList::ConstIterator featuresIt, featuresItEnd = supportedFeatures.constEnd();
		for( featuresIt = supportedFeatures.constBegin(); featuresIt != featuresItEnd; ++featuresIt )
		{
			XMPP::Features tempFeature(*featuresIt);
			QString name;
			if ( tempFeature.id() > XMPP::Features::FID_None )
				name = tempFeature.name();
			if ( name.isEmpty() ) {
				if ( *featuresIt == "http://jabber.org/protocol/mood" )
					name = i18n("User Mood");
				else if ( *featuresIt == "http://jabber.org/protocol/muc" )
					name = i18n("Groupchat");
				else if ( *featuresIt == "http://jabber.org/protocol/xhtml-im" )
					name = i18n("Rich text messages");
				else if ( *featuresIt == "jabber:x:data" )
					name = i18n("Data Forms");
				else if ( *featuresIt == "jabber:x:encrypted" || *featuresIt == "jabber:x:signed" )
					name = i18n("OpenPGP");
				else if ( *featuresIt == "jabber:x:event" )
					name = i18n("Message Events");
				else if ( *featuresIt == "urn:xmpp:receipts" )
					name = i18n("Message Delivery Receipts");
				else if ( featuresIt->startsWith("http://jabber.org/protocol/si") )
					name = i18n("File transfers");
				else if ( featuresIt->startsWith("http://jabber.org/protocol/disco") )
					name = i18n("Service Discovery");
			}
			if ( !name.isEmpty() )
				features << name;
		}
		if( !features.empty() ) {
			features.removeDuplicates();
			resourceListStr += QString( "<tr><td>%1: %2</td></tr>" ).arg ( i18n ( "Supported Features" ), features.join( ", " ) );
		}
		
		// resource timestamp
		resourceListStr += QString ( "<tr><td>%1: %2</td></tr>" ).
						   arg ( i18n ( "Timestamp" ), KGlobal::locale()->formatDateTime ( (*it)->resource().status().timeStamp(), KLocale::ShortDate, true ) );

		// message, if any
		if ( !(*it)->resource().status().status().trimmed().isEmpty () )
		{
			resourceListStr += QString ( "<tr><td>%1: %2</td></tr>" ).
							   arg ( 
								i18n ( "Message" ), 
								Kopete::Message::escape( (*it)->resource().status().status () ) 
								);
		}
	}
	
	resourceListStr += "</table>";
	
	setProperty ( protocol()->propAvailableResources, resourceListStr );
}

void JabberBaseContact::reevaluateStatus ()
{
	kDebug (JABBER_DEBUG_GLOBAL) << "Determining new status for " << contactId ();

	Kopete::OnlineStatus status;
	XMPP::Resource resource = account()->resourcePool()->bestResource ( mRosterItem.jid () );

	status = protocol()->resourceToKOS ( resource );
	
	
	/* Add some icon to show the subscription */ 
	if( ( mRosterItem.subscription().type() == XMPP::Subscription::None || mRosterItem.subscription().type() == XMPP::Subscription::From)
			 && inherits ( "JabberContact" ) && metaContact() != Kopete::ContactList::self()->myself() && account()->isConnected() )
	{
		status = Kopete::OnlineStatus(status.status() ,
									  status.weight() ,
									  protocol() ,
									  status.internalStatus() | 0x0100,
									  status.overlayIcons() + QStringList("status_unknown_overlay") , //FIXME: find better icon
									  status.description() );
	}
	

	updateResourceList ();

	kDebug (JABBER_DEBUG_GLOBAL) << "New status for " << contactId () << " is " << status.description ();
	setOnlineStatus ( status );

	/*
	 * Set away message property.
	 * We just need to read it from the current resource.
	 */
	setStatusMessage( resource.status().status() );

}

QString JabberBaseContact::fullAddress ()
{

	XMPP::Jid jid = rosterItem().jid();

	if ( jid.resource().isEmpty () )
	{
		jid = jid.withResource ( account()->resourcePool()->bestResource ( jid ).name () );
	}

	return jid.full ();

}

XMPP::Jid JabberBaseContact::bestAddress ()
{

	// see if we are subscribed with a preselected resource
	if ( !mRosterItem.jid().resource().isEmpty () )
	{
		// we have a preselected resource, so return our default full address
		return mRosterItem.jid ();
	}

	// construct address out of user@host and current best resource
	XMPP::Jid jid = mRosterItem.jid ();
	jid = jid.withResource ( account()->resourcePool()->bestResource( mRosterItem.jid() ).name () );

	return jid;

}

void JabberBaseContact::setDontSync ( bool flag )
{

	mDontSync = flag;

}

bool JabberBaseContact::dontSync ()
{

	return mDontSync;

}

void JabberBaseContact::serialize (QMap < QString, QString > &serializedData, QMap < QString, QString > & /* addressBookData */ )
{

	// Contact id and display name are already set for us, only add the rest
	serializedData["JID"] = mRosterItem.jid().full();

	serializedData["groups"] = mRosterItem.groups ().join (QString::fromLatin1 (","));
}

void JabberBaseContact::slotUserInfo( )
{
	if ( !account()->isConnected () )
	{
		account()->errorConnectFirst ();
		return;
	}
	
	// Update the vCard
	//slotGetTimedVCard();

	new dlgJabberVCard ( account(), this, Kopete::UI::Global::mainWidget () );
}

void JabberBaseContact::setPropertiesFromVCard ( const XMPP::VCard &vCard )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "Updating vCard for " << contactId ();

	// update vCard cache timestamp if this is not a temporary contact
	if ( metaContact() && !metaContact()->isTemporary () )
	{
		setProperty ( protocol()->propVCardCacheTimeStamp, QDateTime::currentDateTime().toString ( Qt::ISODate ) );
	}

	
	/*
	* Set the nickname property.
	*  but ignore it if we are in a groupchat, or it will clash with the normal nickname
	*/
	if(inherits ( "JabberContact" ))
	{
		if ( !vCard.nickName().isEmpty () )
		{
			setProperty ( protocol()->propNickName, vCard.nickName () );
		}
		else if ( !vCard.fullName().isEmpty () ) // google talk contacts for example do not have a nickname; better show fullname instead of jabber id
		{
			setProperty ( protocol()->propNickName, vCard.fullName () );
		}
		else
		{
			removeProperty ( protocol()->propNickName );
		}
	}

	/**
	 * Kopete does not allow a modification of the "full name"
	 * property. However, some vCards specify only the full name,
	 * some specify only first and last name.
	 * Due to these inconsistencies, if first and last name don't
	 * exist, it is attempted to parse the full name.
	 */

	// remove all properties first
	removeProperty ( protocol()->propFirstName );
	removeProperty ( protocol()->propLastName );
	removeProperty ( protocol()->propFullName );

	if ( !vCard.fullName().isEmpty () && vCard.givenName().isEmpty () && vCard.familyName().isEmpty () )
	{
		QString lastName = vCard.fullName().section ( ' ', 0, -1 );
		QString firstName = vCard.fullName().left(vCard.fullName().length () - lastName.length ()).trimmed ();

		setProperty ( protocol()->propFirstName, firstName );
		setProperty ( protocol()->propLastName, lastName );
	}
	else
	{
		if ( !vCard.givenName().isEmpty () )
			setProperty ( protocol()->propFirstName, vCard.givenName () );

		if ( !vCard.familyName().isEmpty () )
			setProperty ( protocol()->propLastName, vCard.familyName () );
	}
	if( !vCard.fullName().isEmpty() )
		setProperty ( protocol()->propFullName, vCard.fullName() );

	/* 
	* Set the general information 
	*/
	removeProperty( protocol()->propJid );
	removeProperty( protocol()->propBirthday );
	removeProperty( protocol()->propTimezone );
	removeProperty( protocol()->propHomepage );

	setProperty( protocol()->propJid, vCard.jid() );
	
	if( !vCard.bdayStr().isEmpty () )
		setProperty( protocol()->propBirthday, vCard.bdayStr() );
	if( !vCard.timezone().isEmpty () )
		setProperty( protocol()->propTimezone, vCard.timezone() );
	if( !vCard.url().isEmpty () )
		setProperty( protocol()->propHomepage, vCard.url() );

	/*
	* Set the work information.
	*/
	removeProperty( protocol()->propCompanyName );
	removeProperty( protocol()->propCompanyDepartement );
	removeProperty( protocol()->propCompanyPosition );
	removeProperty( protocol()->propCompanyRole );
	
	if( !vCard.org().name.isEmpty() )
		setProperty( protocol()->propCompanyName, vCard.org().name );
	if( !vCard.org().unit.join(",").isEmpty() )
		setProperty( protocol()->propCompanyDepartement, vCard.org().unit.join(",")) ;
	if( !vCard.title().isEmpty() )
		setProperty( protocol()->propCompanyPosition, vCard.title() );
	if( !vCard.role().isEmpty() )
		setProperty( protocol()->propCompanyRole, vCard.role() );

	/*
	* Set the about information
	*/
	removeProperty( protocol()->propAbout );

	if( !vCard.desc().isEmpty() )
		setProperty( protocol()->propAbout, vCard.desc() );

	
	/*
	* Set the work and home addresses information
	*/
	removeProperty( protocol()->propWorkStreet );
	removeProperty( protocol()->propWorkExtAddr );
	removeProperty( protocol()->propWorkPOBox );
	removeProperty( protocol()->propWorkCity );
	removeProperty( protocol()->propWorkPostalCode );
	removeProperty( protocol()->propWorkCountry );

	removeProperty( protocol()->propHomeStreet );
	removeProperty( protocol()->propHomeExtAddr );
	removeProperty( protocol()->propHomePOBox );
	removeProperty( protocol()->propHomeCity );
	removeProperty( protocol()->propHomePostalCode );
	removeProperty( protocol()->propHomeCountry );

	for(XMPP::VCard::AddressList::const_iterator it = vCard.addressList().begin(); it != vCard.addressList().end(); ++it)
	{
		XMPP::VCard::Address address = (*it);

		if(address.work)
		{
			setProperty( protocol()->propWorkStreet, address.street );
			setProperty( protocol()->propWorkExtAddr, address.extaddr );
			setProperty( protocol()->propWorkPOBox, address.pobox );
			setProperty( protocol()->propWorkCity, address.locality );
			setProperty( protocol()->propWorkPostalCode, address.pcode );
			setProperty( protocol()->propWorkCountry, address.country );
		}
		else
			if(address.home)
		{
			setProperty( protocol()->propHomeStreet, address.street );
			setProperty( protocol()->propHomeExtAddr, address.extaddr );
			setProperty( protocol()->propHomePOBox, address.pobox );
			setProperty( protocol()->propHomeCity, address.locality );
			setProperty( protocol()->propHomePostalCode, address.pcode );
			setProperty( protocol()->propHomeCountry, address.country );
		}
	}


	/*
	* Delete emails first, they might not be present
	* in the vCard at all anymore.
	*/
	removeProperty ( protocol()->propEmailAddress );
	removeProperty ( protocol()->propWorkEmailAddress );

	/*
	* Set the home and work email information.
	*/
	XMPP::VCard::EmailList::const_iterator emailEnd = vCard.emailList().end ();
	for(XMPP::VCard::EmailList::const_iterator it = vCard.emailList().begin(); it != emailEnd; ++it)
	{
		XMPP::VCard::Email email = (*it);
		
		if(email.work)
		{
			if( !email.userid.isEmpty() )
				setProperty ( protocol()->propWorkEmailAddress, email.userid );
		}
		else
			if(email.home)
		{	
			if( !email.userid.isEmpty() )
				setProperty ( protocol()->propEmailAddress, email.userid );
		}
	}

	/*
	* Delete phone number properties first as they might have
	* been unset during an update and are not present in
	* the vCard at all anymore.
	*/
	removeProperty ( protocol()->propPrivatePhone );
	removeProperty ( protocol()->propPrivateMobilePhone );
	removeProperty ( protocol()->propWorkPhone );
	removeProperty ( protocol()->propWorkMobilePhone );

	/*
	* Set phone numbers. Note that if a mobile phone number
	* is specified, it's assigned to the private mobile
	* phone number property. This might not be the desired
	* behavior for all users.
	*/
	XMPP::VCard::PhoneList::const_iterator phoneEnd = vCard.phoneList().end ();
	for(XMPP::VCard::PhoneList::const_iterator it = vCard.phoneList().begin(); it != phoneEnd; ++it)
	{
		XMPP::VCard::Phone phone = (*it);

		if(phone.work)
		{
			setProperty ( protocol()->propWorkPhone, phone.number );
		}
		else
			if(phone.fax)
		{
			setProperty ( protocol()->propPhoneFax, phone.number);
		}
		else
			if(phone.cell)
		{
			setProperty ( protocol()->propPrivateMobilePhone, phone.number );
		}
		else
			if(phone.home)
		{
			setProperty ( protocol()->propPrivatePhone, phone.number );
		}

	}

	/*
	* Set photo/avatar property.
	*/
	removeProperty( protocol()->propPhoto );

	QImage contactPhoto;
	
	// photo() is a QByteArray
	if ( !vCard.photo().isEmpty() )
	{
		kDebug( JABBER_DEBUG_GLOBAL ) << "Contact has a photo embedded into his vCard.";

		// QImage is used to save to disk in PNG later.
		contactPhoto = QImage::fromData( vCard.photo() );
	}
	// Contact photo is a URI.
	else if( !vCard.photoURI().isEmpty() )
	{
		QString tempPhotoPath = 0;
		
		// Downalod photo from URI.
		if( !KIO::NetAccess::download( vCard.photoURI(), tempPhotoPath, 0) ) 
		{
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget (), KMessageBox::Sorry, i18n( "Failed to download Jabber contact photo." ) );
			return;
		}


		kDebug( JABBER_DEBUG_GLOBAL ) << "Contact photo is a URI.";

		contactPhoto = QImage( tempPhotoPath );
		
		KIO::NetAccess::removeTempFile(  tempPhotoPath );
	}

	if ( !contactId().isEmpty() )
	{
		// add the entry using the avatar manager
		Kopete::AvatarManager::AvatarEntry entry;
		entry.name = contactId();
		entry.image = contactPhoto;
		entry.category = Kopete::AvatarManager::Contact;
		entry.contact = this;	
		entry = Kopete::AvatarManager::self()->add(entry);

		// Save the image to the disk, then set the property.
		if(!entry.dataPath.isNull())
		{
			kDebug( JABBER_DEBUG_GLOBAL ) << "Setting photo for contact: " << contactId();
			setProperty( protocol()->propPhoto, entry.dataPath );
		}
	}
}




#include "jabberbasecontact.moc"

// vim: set noet ts=4 sts=4 sw=4:
