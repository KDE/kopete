 /*
  * jabbercontact.cpp  -  Regular Kopete Jabber protocol contact
  *
  * Copyright (c) 2002-2004 by Till Gerken <till@tantalo.net>
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

#include "jabbercontact.h"

#include "xmpp_tasks.h"

#include <qtimer.h>
#include <qdatetime.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kaction.h>
#include <kapplication.h>

#include "kopetegroup.h"
#include "kopeteuiglobal.h"
#include "kopetemessagemanagerfactory.h"
#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabbermessagemanager.h"
#include "jabberresourcepool.h"
#include "jabberfiletransfer.h"
#include "dlgjabbervcard.h"


/**
 * JabberContact constructor
 */
JabberContact::JabberContact (const XMPP::RosterItem &rosterItem, JabberAccount *account, KopeteMetaContact * mc)
				: JabberBaseContact ( rosterItem, account, mc)
{

	// this contact is able to transfer files
	setFileCapable ( true );
	
	/*
	 * Catch when we're going online for the first time to
	 * update our properties from a vCard. (properties are
	 * not available during startup, so we need to read
	 * them later - this also serves as a random update
	 * feature)
	 */
	connect ( account->myself (), SIGNAL ( onlineStatusChanged ( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus & ) ),
			  this, SLOT ( slotCheckVCard () ) );

	/*
	 * Trigger update once if we're already connected for contacts
	 * that are being added while we are online.
	 */
	if ( ( account->myself() != 0)
		&& ( ( account->myself()->onlineStatus().status () == KopeteOnlineStatus::Online ) ||
		 ( account->myself()->onlineStatus().status () == KopeteOnlineStatus::Away ) ) )
		slotCheckVCard ();

}

QPtrList<KAction> *JabberContact::customContextMenuActions ()
{

	QPtrList<KAction> *actionCollection = new QPtrList<KAction>();

	KActionMenu *actionAuthorization = new KActionMenu ( i18n ("Authorization"), "connect_established", this, "jabber_authorization");

	actionAuthorization->insert (new KAction (i18n ("(Re)send Authorization To"), "mail_forward", 0,
								 this, SLOT (slotSendAuth ()), actionAuthorization, "actionSendAuth"));
	actionAuthorization->insert (new KAction (i18n ("(Re)request Authorization From"), "mail_reply", 0,
								 this, SLOT (slotRequestAuth ()), actionAuthorization, "actionRequestAuth"));
	actionAuthorization->insert (new KAction (i18n ("Remove Authorization From"), "mail_delete", 0,
								 this, SLOT (slotRemoveAuth ()), actionAuthorization, "actionRemoveAuth"));

	KActionMenu *actionSetAvailability = new KActionMenu (i18n ("Set Availability"), "kopeteavailable", this, "jabber_online");

	actionSetAvailability->insert(new KAction (i18n ("Online"), protocol()->JabberKOSOnline.iconFor(this),
								  0, this, SLOT (slotStatusOnline ()), actionSetAvailability, "actionOnline"));
	actionSetAvailability->insert(new KAction (i18n ("Free to Chat"), protocol()->JabberKOSChatty.iconFor(this),
								  0, this, SLOT (slotStatusChatty ()), actionSetAvailability, "actionChatty"));
	actionSetAvailability->insert(new KAction (i18n ("Away"), protocol()->JabberKOSAway.iconFor(this),
								  0, this, SLOT (slotStatusAway ()), actionSetAvailability, "actionAway"));
	actionSetAvailability->insert(new KAction (i18n ("Extended Away"), protocol()->JabberKOSXA.iconFor(this),
								  0, this, SLOT (slotStatusXA ()), actionSetAvailability, "actionXA"));
	actionSetAvailability->insert(new KAction (i18n ("Do Not Disturb"), protocol()->JabberKOSDND.iconFor(this),
								  0, this, SLOT (slotStatusDND ()), actionSetAvailability, "actionDND"));
	actionSetAvailability->insert(new KAction (i18n ("Invisible"), protocol()->JabberKOSInvisible.iconFor(this),
								  0, this, SLOT (slotStatusInvisible ()), actionSetAvailability, "actionInvisible"));

	KActionMenu *actionSelectResource = new KActionMenu (i18n ("Select Resource"), "connect_no", this, "actionSelectResource");

	// if the contact is online, display the resources we have for it,
	// otherwise disable the menu
	if (onlineStatus ().status () == KopeteOnlineStatus::Offline)
	{
		actionSelectResource->setEnabled ( false );
	}
	else
	{
		QStringList items;
		ResourceList availableResources;

		int activeItem = 0, i = 1;
		const XMPP::Resource lockedResource = account()->resourcePool()->lockedResource ( mRosterItem.jid () );

		// put default resource first
		items.append (i18n ("Automatic (best/default resource)"));

		account()->resourcePool()->findResources ( mRosterItem.jid (), availableResources );

		for (ResourceList::iterator it = availableResources.begin(); it != availableResources.end(); ++it, i++)
		{
			items.append ( (*it).name() );

			if ( (*it).name() == lockedResource.name() )
				activeItem = i;
		}

		// now go through the string list and add the resources with their icons
		i = 0;
		for(QStringList::iterator it = items.begin(); it != items.end(); it++)
		{
			if( i == activeItem )
			{
				actionSelectResource->insert ( new KAction( ( *it ), "button_ok", 0, this, SLOT( slotSelectResource() ),
											   actionSelectResource, QString::number( i ).latin1() ) );
			}
			else
			{
				actionSelectResource->insert ( new KAction( ( *it ), "", 0, this, SLOT( slotSelectResource() ),
											   actionSelectResource, QString::number( i ).latin1() ) );
			}

			i++;
		}

	}

	actionCollection->append( actionAuthorization );
	actionCollection->append( actionSetAvailability );
	actionCollection->append( actionSelectResource );

	return actionCollection;

}
void JabberContact::rename ( const QString &newName )
{

	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Renaming " << contactId () << " to " << newName << endl;

	// our display name has been changed, forward the change to the roster
	if (!account()->isConnected())
	{
		account()->errorConnectFirst();
		return;
	}

	// only forward change if this is not a temporary contact
	if ( !metaContact()->isTemporary () )
	{
		XMPP::JT_Roster * rosterTask = new XMPP::JT_Roster ( account()->client()->rootTask () );

		rosterTask->set ( mRosterItem.jid (), newName, mRosterItem.groups ());
		rosterTask->go ( true );
	}
	else
	{
		setDisplayName ( newName );
	}

}

void JabberContact::handleIncomingMessage (const XMPP::Message & message)
{
	KopeteMessage::MessageType type;
	KopeteMessage *newMessage = 0L;

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Received Message Type:" << message.type () << endl;

	// fetch message manager
	JabberMessageManager *mManager = manager ( message.from().resource (), true );

	// evaluate typing notifications
	if ( message.type () != "error" )
	{
		if ( message.containsEvent ( CancelEvent ) )
			mManager->receivedTypingMsg ( this, false );
		else
			if ( message.containsEvent ( ComposingEvent ) )
				mManager->receivedTypingMsg ( this, true );
	}

	/**
	 * Don't display empty messages, these were most likely just carrying
	 * event notifications or other payload.
	 */
	if ( message.body().isEmpty () )
		return;

	// determine message type
	if (message.type () == "chat")
		type = KopeteMessage::Chat;
	else
		type = KopeteMessage::Email;

	KopeteContactPtrList contactList;
	contactList.append ( account()->myself () );

	// check for errors
	if ( message.type () == "error" )
	{
		newMessage = new KopeteMessage( message.timeStamp (), this, contactList,
										i18n("Your message could not be delivered: \"%1\", Reason: \"%2\"").
										arg ( message.body () ).arg ( message.error().text ),
										message.subject(), KopeteMessage::Inbound, KopeteMessage::PlainText, type );
	}
	else
	{
		// retrieve and reformat body
		QString body = message.body ();

		if( !message.xencrypted().isEmpty () )
		{
			body = QString ("-----BEGIN PGP MESSAGE-----\n\n") + message.xencrypted () + QString ("\n-----END PGP MESSAGE-----\n");
		}

		// convert XMPP::Message into KopeteMessage
		newMessage = new KopeteMessage ( message.timeStamp (), this, contactList, body,
										 message.subject (), KopeteMessage::Inbound,
										 KopeteMessage::PlainText, type );
	}

	// append message to (eventually new) manager and preselect the originating resource
	mManager->appendMessage ( *newMessage, message.from().resource () );

	delete newMessage;

}

void JabberContact::slotCheckVCard ()
{
	QDateTime cacheDate;
	Kopete::ContactProperty cacheDateString = property ( protocol()->propVCardCacheTimeStamp );

	// avoid warning if key does not exist in configuration file
	if ( cacheDateString.isNull () )
		cacheDate = QDateTime::currentDateTime().addDays ( -2 );
	else
		cacheDate = QDateTime::fromString ( cacheDateString.value().toString (), Qt::ISODate );

	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Cached vCard data for " << contactId () << " from " << cacheDate.toString () << endl;

	if ( cacheDate.addDays ( 1 ) < QDateTime::currentDateTime () )
	{
		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Scheduling update." << endl;

		// current data is older than 24 hours, request a new one
		QTimer::singleShot ( account()->getPenaltyTime () * 1000, this, SLOT ( slotGetTimedVCard () ) );
	}

}

void JabberContact::slotGetTimedVCard ()
{

	// check if we are connected
	if ( ( account()->myself()->onlineStatus().status () != KopeteOnlineStatus::Online ) &&
		 ( account()->myself()->onlineStatus().status () != KopeteOnlineStatus::Away ) )
	{
		// we are not connected, discard this update
		return;
	}

	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Requesting vCard for " << contactId () << " from update timer." << endl;

	// request vCard
	XMPP::JT_VCard *task = new XMPP::JT_VCard ( account()->client()->rootTask () );
	// signal to ourselves when the vCard data arrived
	QObject::connect ( task, SIGNAL ( finished () ), this, SLOT ( slotGotVCard () ) );
	task->get ( mRosterItem.jid () );
	task->go ( true );

}

void JabberContact::slotGotVCard ()
{

	XMPP::JT_VCard * vCard = (XMPP::JT_VCard *) sender ();

	if ( !vCard->success() )
	{
		/*
		 * A vCard for the user does not exist or the
		 * request was unsuccessful or incomplete.
		 * Manually set the timestamp here so that
		 * a new request won't be triggered too early.
		 */
		setProperty ( protocol()->propVCardCacheTimeStamp, QDateTime::currentDateTime().toString ( Qt::ISODate ) );
		return;
	}

	setPropertiesFromVCard ( vCard->vcard () );

}

void JabberContact::setPropertiesFromVCard ( const XMPP::VCard &vCard )
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Updating vCard for " << contactId () << endl;

	// update vCard cache timestamp if this is not a temporary contact
	if ( !metaContact()->isTemporary () )
	{
		setProperty ( protocol()->propVCardCacheTimeStamp, QDateTime::currentDateTime().toString ( Qt::ISODate ) );
	}

	/*
	 * If a nick name is present in the vCard and if
	 * no nick name has been set so far, we'll update
	 * the contact's nick name to the one set in the vCard.
	 * The check for the display name has to be done before
	 * we set the actual nick name property, as these are
	 * connected and displayName() will be updated behind
	 * the scenes.
	 * The rename request will update it again by getting
	 * feedback from the contact list.
	 */
	if ( !vCard.nickName().isEmpty () )
	{
		if ( displayName().isEmpty () || displayName () == contactId () )
		{
			/*
			 * This will cause the nick name property
			 * to be updated once we get feedback from
			 * the server, so no need to set the property
			 * twice.
			 */
			rename ( vCard.nickName () );
		}
		else
		{
			/*
			 * The user already specified a nick name
			 * in the contact list but we want the
			 * official nick name to show up in the
			 * contact list tooltip.
			 */
			setProperty ( protocol()->propNickName, vCard.nickName () );
		}
	}
	else
	{
		// no nickname has been set, remove it from the prop list
		removeProperty ( protocol()->propNickName );
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

	if ( !vCard.fullName().isEmpty () && vCard.givenName().isEmpty () && vCard.familyName().isEmpty () )
	{
		QString firstName = vCard.fullName().section ( ' ', 0, 0 );
		QString lastName = vCard.fullName().section ( ' ', 1, 1 );
		
		// if a "," is found, remove it and switch names
		// (format was then probably "Lastname, Firstname")
		if ( firstName.find ( ',' ) != -1 )
		{
			firstName = firstName.remove ( ',' );
			QString tmp = lastName;
			lastName = firstName;
			firstName = tmp;
		}
		
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

/**
 * Currently unsupported properties (to be implemented)
 *
	m_mainWidget->leJID->setText (vCard.jid());
	m_mainWidget->leBirthday->setText (vCard.bdayStr());
	m_mainWidget->leTimezone->setText (vCard.timezone());
	m_mainWidget->leHomepage->setText (vCard.url());
	m_mainWidget->urlHomepage->setText (vCard.url());
	m_mainWidget->urlHomepage->setURL (vCard.url());
	m_mainWidget->urlHomepage->setUseCursor ( !vCard.url().isEmpty () );

	// work information tab
	m_mainWidget->leCompany->setText (vCard.org().name);
	m_mainWidget->leDepartment->setText (vCard.org().unit.join(","));
	m_mainWidget->lePosition->setText (vCard.title());
	m_mainWidget->leRole->setText (vCard.role());

	// about tab
	m_mainWidget->teAbout->setText (vCard.desc());
*/
/**
 * Adresses - currently unsupported
 *
	for(XMPP::VCard::AddressList::const_iterator it = vCard.addressList().begin(); it != vCard.addressList().end(); it++)
	{
		XMPP::VCard::Address address = (*it);

		if(address.work)
		{
			m_mainWidget->leWorkStreet->setText (address.street);
			m_mainWidget->leWorkExtAddr->setText (address.extaddr);
			m_mainWidget->leWorkPOBox->setText (address.pobox);
			m_mainWidget->leWorkCity->setText (address.locality);
			m_mainWidget->leWorkPostalCode->setText (address.pcode);
			m_mainWidget->leWorkCountry->setText (address.country);
		}
		else
		if(address.home)
		{
			m_mainWidget->leHomeStreet->setText (address.street);
			m_mainWidget->leHomeExtAddr->setText (address.extaddr);
			m_mainWidget->leHomePOBox->setText (address.pobox);
			m_mainWidget->leHomeCity->setText (address.locality);
			m_mainWidget->leHomePostalCode->setText (address.pcode);
			m_mainWidget->leHomeCountry->setText (address.country);
		}
	}
*/

	/*
	 * Delete emails first, they might not be present
	 * in the vCard at all anymore.
	 */
	removeProperty ( protocol()->propEmailAddress );

	/*
	 * Note: the following code gives the private email
	 * preference over the work email if both are given.
	 * This might not be the desired behaviour for all.
	 */
	for(XMPP::VCard::EmailList::const_iterator it = vCard.emailList().begin(); it != vCard.emailList().end(); it++)
	{
		XMPP::VCard::Email email = (*it);

		if(email.work)
		{
			setProperty ( protocol()->propEmailAddress, email.userid );
		}
		else
		if(email.home)
		{
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
	for(XMPP::VCard::PhoneList::const_iterator it = vCard.phoneList().begin(); it != vCard.phoneList().end(); it++)
	{
		XMPP::VCard::Phone phone = (*it);

		if(phone.work)
		{
			setProperty ( protocol()->propWorkPhone, phone.number );
		}
		else
/*		if(phone.fax)
		{
			m_mainWidget->lePhoneFax->setText (phone.number);
		}
		else*/
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

}

void JabberContact::slotMessageManagerDeleted ( QObject *sender )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Message manager deleted, collecting the pieces..." << endl;

	JabberMessageManager *manager = static_cast<JabberMessageManager *>(sender);

	mManagers.remove ( mManagers.find ( manager ) );

}

JabberMessageManager *JabberContact::manager ( KopeteContactPtrList chatMembers, bool canCreate )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "called, canCreate: " << canCreate << endl;

	KopeteMessageManager *_manager = KopeteMessageManagerFactory::factory()->findKopeteMessageManager ( account()->myself(), chatMembers, protocol() );
	JabberMessageManager *manager = dynamic_cast<JabberMessageManager*>( _manager );

	/*
	 * If we didn't find a message manager for this contact,
	 * instantiate a new one if we are allowed to. (otherwise return 0)
	 */
	if ( !manager &&  canCreate )
	{
		XMPP::Jid jid ( contactId () );

		/*
		 * If we have no hardwired JID, set any eventually
		 * locked resource as preselected resource.
		 * If there is no locked resource, the resource field
		 * will stay empty.
		 */
		if ( jid.resource().isEmpty () )
			jid.setResource ( account()->resourcePool()->lockedResource ( jid ).name () );

		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "No manager found, creating a new one with resource '" << jid.resource () << "'" << endl;

		manager = new JabberMessageManager ( protocol(), static_cast<JabberBaseContact *>(account()->myself()), chatMembers, jid.resource () );
		connect ( manager, SIGNAL ( destroyed ( QObject * ) ), this, SLOT ( slotMessageManagerDeleted ( QObject * ) ) );
		mManagers.append ( manager );
	}

	return manager;

}

KopeteMessageManager *JabberContact::manager ( bool canCreate )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "called, canCreate: " << canCreate << endl;

	KopeteContactPtrList chatMembers;
	chatMembers.append ( this );

	return manager ( chatMembers, canCreate );

}

JabberMessageManager *JabberContact::manager ( const QString &resource, bool canCreate )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "called, canCreate: " << canCreate << ", Resource: '" << resource << "'" << endl;

	/*
	 * First of all, see if we already have a manager matching
	 * the requested resource or if there are any managers with
	 * an empty resource.
	 */
	if ( !resource.isEmpty () )
	{
		for ( JabberMessageManager *mManager = mManagers.first (); mManager; mManager = mManagers.next () )
		{
			if ( mManager->resource().isEmpty () || ( mManager->resource () == resource ) )
			{
				// we found a matching manager, return this one
				kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Found an existing message manager for this resource." << endl;
				return mManager;
			}
		}

		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "No manager found for this resource, creating a new one." << endl;

		/*
		 * If we have come this far, we were either supposed to create
		 * a manager with a preselected resource but have found
		 * no available manager. (not even one with an empty resource)
		 * This means, we will have to create a new one with a
		 * preselected resource.
		 */
		KopeteContactPtrList chatmembers;
		chatmembers.append ( this );
		JabberMessageManager *manager = new JabberMessageManager ( protocol(),
																   static_cast<JabberBaseContact *>(account()->myself()),
																   chatmembers, resource );
		connect ( manager, SIGNAL ( destroyed ( QObject * ) ), this, SLOT ( slotMessageManagerDeleted ( QObject * ) ) );
		mManagers.append ( manager );

		return manager;
	}

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Resource is empty, grabbing first available manager." << endl;

	/*
	 * The resource is empty, so just return first available manager.
	 */
	return dynamic_cast<JabberMessageManager *>( manager ( canCreate ) );

}

void JabberContact::slotDeleteContact ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Removing user " << contactId () << endl;

	if ( !account()->isConnected () )
	{
		account()->errorConnectFirst ();
		return;
	}

	XMPP::JT_Roster * rosterTask = new XMPP::JT_Roster ( account()->client()->rootTask () );

	rosterTask->remove ( mRosterItem.jid () );
	rosterTask->go ( true );

}

void JabberContact::syncGroups ()
{
	QStringList groups;
	KopeteGroupList groupList = metaContact ()->groups ();

	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Synchronizing groups for " << contactId () << endl;

	if ( !account()->isConnected() )
	{
		account()->errorConnectFirst ();
		return;
	}

	// if this is a temporary contact, don't bother
	if ( metaContact()->isTemporary () )
		return;

	for ( KopeteGroup * g = groupList.first (); g; g = groupList.next () )
	{
		groups += g->displayName ();
	}

	mRosterItem.setGroups ( groups );

	XMPP::JT_Roster * rosterTask = new XMPP::JT_Roster ( account()->client()->rootTask () );

	rosterTask->set ( mRosterItem.jid (), displayName (), mRosterItem.groups () );
	rosterTask->go (true);

}

void JabberContact::sendFile ( const KURL &sourceURL, const QString &/*fileName*/, uint /*fileSize*/ )
{
	QString filePath;

	// if the file location is null, then get it from a file open dialog
	if ( !sourceURL.isValid () )
		filePath = KFileDialog::getOpenFileName( QString::null , "*", 0L, i18n ( "Kopete File Transfer" ) );
	else
		filePath = sourceURL.path(-1);

	QFile file ( filePath );

	if ( file.exists () )
	{
		// send the file
		new JabberFileTransfer ( account (), this, filePath );
	}

}

void JabberContact::slotUserInfo ()
{

	if ( !account()->isConnected () )
	{
		account()->errorConnectFirst ();
		return;
	}

	new dlgJabberVCard ( account(), mRosterItem.jid().full (), Kopete::UI::Global::mainWidget () );

}

void JabberContact::slotSendAuth ()
{

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "(Re)send auth " << contactId () << endl;

	sendSubscription ("subscribed");

}

void JabberContact::slotRequestAuth ()
{

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "(Re)request auth " << contactId () << endl;

	sendSubscription ("subscribe");

}

void JabberContact::slotRemoveAuth ()
{

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Remove auth " << contactId () << endl;

	sendSubscription ("unsubscribed");

}

void JabberContact::sendSubscription ( const QString& subType )
{

	if ( !account()->isConnected () )
	{
		account()->errorConnectFirst ();
		return;
	}

	XMPP::JT_Presence * task = new XMPP::JT_Presence ( account()->client()->rootTask () );

	task->sub ( mRosterItem.jid().full (), subType );
	task->go ( true );

}

void JabberContact::slotSelectResource ()
{
	int currentItem = QString ( static_cast<const KAction *>( sender() )->name () ).toUInt ();

	/*
	 * Warn the user if there is already an active chat window.
	 * The resource selection will only apply for newly opened
	 * windows.
	 */
	if ( manager ( false ) != 0 )
	{
		KMessageBox::queuedMessageBox ( Kopete::UI::Global::mainWidget (),
										KMessageBox::Information,
										i18n ("You have preselected a resource for contact %1, "
										"but you still have open chat windows for this contact. "
										"The preselected resource will only apply to newly opened "
										"chat windows.").arg ( contactId () ),
										i18n ("Jabber Resource Selector") );
	}

	if (currentItem == 0)
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Removing active resource, trusting bestResource()." << endl;

		account()->resourcePool()->removeLock ( XMPP::Jid ( contactId () ) );
	}
	else
	{
		QString selectedResource = static_cast<const KAction *>(sender())->text();

		kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Moving to resource " << selectedResource << endl;

		account()->resourcePool()->lockToResource ( XMPP::Jid ( contactId () ), XMPP::Resource ( selectedResource ) );
	}

}

void JabberContact::sendPresence ( const XMPP::Status status )
{

	if ( !account()->isConnected () )
	{
		account()->errorConnectFirst ();
		return;
	}

	XMPP::Status newStatus = status;

	// honour our priority
	newStatus.setPriority ( account()->pluginData ( protocol (), "Priority" ).toInt () );

	XMPP::JT_Presence * task = new XMPP::JT_Presence ( account()->client()->rootTask () );

	task->pres ( bestAddress (), newStatus);
	task->go ( true );

}


void JabberContact::slotStatusOnline ()
{

	XMPP::Status status;
	status.setShow("");

	sendPresence ( status );

}

void JabberContact::slotStatusChatty ()
{

	XMPP::Status status;
	status.setShow ("chat");

	sendPresence ( status );

}

void JabberContact::slotStatusAway ()
{

	XMPP::Status status;
	status.setShow ("away");

	sendPresence ( status );

}

void JabberContact::slotStatusXA ()
{

	XMPP::Status status;
	status.setShow ("xa");

	sendPresence ( status );

}

void JabberContact::slotStatusDND ()
{

	XMPP::Status status;
	status.setShow ("dnd");

	sendPresence ( status );


}

void JabberContact::slotStatusInvisible ()
{

	XMPP::Status status;
	status.setShow ("away");
	status.setIsInvisible ( true );

	sendPresence ( status );

}

#include "jabbercontact.moc"
