 /*
  * jabbercontact.cpp  -  Base class for the Kopete Jabber protocol contact
  *
  * Copyright (c) 2002-2003 by Till Gerken <till@tantalo.net>
  * Copyright (c) 2002 by Daniel Stone <dstone@kde.org>
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

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kfiledialog.h>

#include "xmpp_tasks.h"

#include "dlgjabbervcard.h"
#include "jabbercontact.h"
#include "jabberprotocol.h"
#include "jabberresourcepool.h"
#include "jabberfiletransfer.h"
#include "kopetemessage.h"
#include "kopetemessagemanagerfactory.h"
#include "kopeteuiglobal.h"
#include "kopetemetacontact.h"
#include "kopetegroup.h"
#include "kopetecontactlist.h"

#include "kopeteuiglobal.h"
#include "jabbermessagemanager.h"

/**
 * JabberContact constructor
 */
JabberContact::JabberContact (const XMPP::RosterItem &rosterItem, JabberAccount *account, KopeteMetaContact * mc)
				: KopeteContact (account, rosterItem.jid().full(), mc)
{

	// take roster item and update display name
	updateContact ( rosterItem );

	// since we're not in the account's contact pool yet
	// (we'll only be once we returned from this constructor),
	// we need to force an update to our status here
	reevaluateStatus ();

	setFileCapable ( true );

}

JabberProtocol *JabberContact::protocol ()
{

	return static_cast<JabberProtocol *>(KopeteContact::protocol ());

}

JabberAccount *JabberContact::account ()
{

	return static_cast<JabberAccount *>(KopeteContact::account ());

}

KopeteMessageManager *JabberContact::manager ( bool canCreate )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "called, canCreate: " << canCreate << endl;

	KopeteContactPtrList chatmembers;
	chatmembers.append ( this );

	KopeteMessageManager *_manager = KopeteMessageManagerFactory::factory()->findKopeteMessageManager ( account()->myself(), chatmembers, protocol() );
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

		manager = new JabberMessageManager ( protocol(), static_cast<JabberContact *>(account()->myself()), chatmembers, jid.resource () );
		connect ( manager, SIGNAL ( destroyed ( QObject * ) ), this, SLOT ( slotMessageManagerDeleted ( QObject * ) ) );
		mManagers.append ( manager );
	}

	return manager;

}

void JabberContact::slotMessageManagerDeleted ( QObject *sender )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Message manager deleted, collecting the pieces..." << endl;

	JabberMessageManager *manager = static_cast<JabberMessageManager *>(sender);

	mManagers.remove ( mManagers.find ( manager ) );

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
																   static_cast<JabberContact *>(account()->myself()),
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

/* Return if we are reachable (defaults to true because
   we can send on- and offline, only return false if the
   account itself is offline, too) */
bool JabberContact::isReachable ()
{
	if (account()->isConnected())
		return true;

	return false;

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

void JabberContact::updateContact ( const XMPP::RosterItem & item )
{

	mRosterItem = item;

	// only update the nickname if its not empty
	if ( !item.name ().isEmpty () )
	{
		setDisplayName ( item.name () );
	}
	else
	{
		setDisplayName ( item.jid().full () );
	}

}

void JabberContact::rename ( const QString &newName )
{

	// our display name has been changed, forward the change to the roster
	if (!account()->isConnected())
	{
		account()->errorConnectFirst();
		return;
	}

	XMPP::JT_Roster * rosterTask = new XMPP::JT_Roster ( account()->client()->rootTask () );

	rosterTask->set ( mRosterItem.jid (), newName, mRosterItem.groups ());
	rosterTask->go ( true );
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

void JabberContact::syncGroups ()
{
	QStringList groups;
	KopeteGroupList groupList = metaContact ()->groups ();

	if ( !account()->isConnected() )
	{
		account()->errorConnectFirst ();
		return;
	}

	for ( KopeteGroup * g = groupList.first (); g; g = groupList.next () )
	{
		groups += g->displayName ();
	}

	mRosterItem.setGroups ( groups );

	XMPP::JT_Roster * rosterTask = new XMPP::JT_Roster ( account()->client()->rootTask () );

	rosterTask->set ( mRosterItem.jid (), mRosterItem.name (), mRosterItem.groups () );
	rosterTask->go (true);

}

void JabberContact::sendFile ( const KURL &sourceURL, const QString &fileName, uint fileSize )
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

void JabberContact::handleIncomingMessage (const XMPP::Message & message)
{
	KopeteMessage::MessageType type;
	KopeteContactPtrList contactList;
	KopeteMessage *newMessage = 0L;

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Received Message Type:" << message.type () << endl;

	// FIXME: bugfix for current libxmpp which reports typing events
	// as regular message
	if ( message.type().isEmpty () && message.body().isEmpty () )
		return;

	// determine message type
	if (message.type () == "chat")
		type = KopeteMessage::Chat;
	else
		type = KopeteMessage::Email;

	contactList.append ( account()->myself () );

	// check for errors
	if ( message.type () == "error" )
	{
		newMessage = new KopeteMessage( message.timeStamp (), this, contactList,
										i18n("Your message could not be delivered: \"%1\"").arg ( message.body () ),
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

	// fetch message manager
	JabberMessageManager *mManager = manager ( message.from().resource (), true );

	// append message to (eventually new) manager and preselect the originating resource
	mManager->appendMessage ( *newMessage, message.from().resource () );

	delete newMessage;

}

void JabberContact::reevaluateStatus ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Determining new status for " << contactId () << endl;

	KopeteOnlineStatus status;
	XMPP::Resource resource = account()->resourcePool()->bestResource ( mRosterItem.jid () );

	// update to online by default
	status = protocol()->JabberKOSOffline;

	if ( !resource.status().isAvailable () )
	{
		// resource is offline
		status = protocol()->JabberKOSOffline;
	}
	else
	{
		if (resource.status ().show () == "")
		{
			status = protocol()->JabberKOSOnline;
		}
		else
		if (resource.status ().show () == "chat")
		{
			status = protocol()->JabberKOSChatty;
		}
		else if (resource.status ().show () == "away")
		{
			status = protocol()->JabberKOSAway;
		}
		else if (resource.status ().show () == "xa")
		{
			status = protocol()->JabberKOSXA;
		}
		else if (resource.status ().show () == "dnd")
		{
			status = protocol()->JabberKOSDND;
		}
		else if (resource.status ().show () == "connecting")
		{
			status = protocol()->JabberKOSConnecting;
		}
	}

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "New status for " << contactId () << " is " << status.description () << endl;
	setOnlineStatus ( status );

}

QString JabberContact::fullAddress ()
{

	XMPP::Jid jid ( contactId () );

	if ( jid.resource().isEmpty () )
	{
		jid.setResource ( account()->resourcePool()->bestResource ( jid ).name () );
	}

	return jid.full ();

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

void JabberContact::slotUserInfo ()
{

	if ( !account()->isConnected () )
	{
		account()->errorConnectFirst ();
		return;
	}

	new dlgJabberVCard ( account(), mRosterItem.jid().full (), Kopete::UI::Global::mainWidget () );

}

XMPP::Jid JabberContact::bestAddress ()
{

	// see if we are subscribed with a preselected resource
	if ( !mRosterItem.jid().resource().isEmpty () )
	{
		// we have a preselected resource, so return our default full address
		return mRosterItem.jid ();
	}

	// construct address out of user@host and current best resource
	XMPP::Jid jid = mRosterItem.jid ();
	jid.setResource ( account()->resourcePool()->bestResource( mRosterItem.jid() ).name () );

	return jid;

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

void JabberContact::serialize (QMap < QString, QString > &serializedData, QMap < QString, QString > & /* addressBookData */ )
{

	// Contact id and display name are already set for us, only add the rest
	serializedData["identityId"] = account()->accountId();

	serializedData["groups"] = mRosterItem.groups ().join (QString::fromLatin1 (","));
}

#include "jabbercontact.moc"

// vim: set noet ts=4 sts=4 sw=4:
