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

#include <qapplication.h>
#include <qcursor.h>
#include <qfont.h>
#include <qptrlist.h>
#include <qstrlist.h>

#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>

#include "tasks.h"
#include "types.h"

#include "dlgjabbervcard.h"
#include "jabbercontact.h"
#include "jabberprotocol.h"
#include "jabberresource.h"
#include "kopetestdaction.h"
#include "kopetemessage.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "kopeteview.h"
#include "kopetemetacontact.h"
#include "kopetegroup.h"
#include "kopetecontactlist.h"

/**
 * JabberContact constructor
 */
JabberContact::JabberContact (QString userId, QString nickname, QStringList groups, JabberAccount * p, KopeteMetaContact * mc, QString /* identity */, const QString & icon)
				: KopeteContact (p, userId.lower(), mc, icon)
{

	parentMetaContact = mc;

	resourceOverride = false;

	messageManager = 0L;

	rosterItem.setJid (Jabber::Jid (userId));
	rosterItem.setName (nickname);
	rosterItem.setGroups (groups);

	// create a default (empty) resource for the contact
	JabberResource *defaultResource = new JabberResource (QString::null, -1, QDateTime::currentDateTime (),
														  static_cast<JabberProtocol *>(protocol())->JabberKOSOffline, "");

	//JabberProtocol::JabberKOSOffline(), "");

	resources.append (defaultResource);

	activeResource = defaultResource;

	// update the displayed name
	setDisplayName (rosterItem.name ());

	// specifically cause this instance to update this contact as offline
	slotUpdatePresence (static_cast<JabberProtocol *>(protocol())->JabberKOSOffline, QString::null);

	connect(this, SIGNAL(displayNameChanged(const QString &, const QString &)), this, SLOT(slotRenameContact(const QString &, const QString &)));

}

/* Return the user ID */
QString JabberContact::userId () const
{
	return rosterItem.jid ().userHost ();
}

/* Return the currently used resource for this contact */
QString JabberContact::resource () const
{
	return activeResource->resource ();
}

KopeteMessageManager *JabberContact::manager (bool)
{

	// create a new message manager if there is none
	if (!messageManager)
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberContact] Creating new message manager." << endl;

		KopeteContactPtrList contactList;

		contactList.append (this);

		messageManager = KopeteMessageManagerFactory::factory ()->create (account()->myself (), contactList, protocol());

		QObject::connect (messageManager, SIGNAL (destroyed ()), this, SLOT (slotMessageManagerDeleted ()));
		QObject::connect (messageManager, SIGNAL (messageSent (KopeteMessage &, KopeteMessageManager *)), this, SLOT (slotSendMessage (KopeteMessage &)));
	}

	return static_cast < KopeteMessageManager * >(messageManager);

}

void JabberContact::slotMessageManagerDeleted ()
{

	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberContact] Message manager has been deleted." << endl;

	messageManager = 0L;

}


/* Return the reason why we are away */
QString JabberContact::reason () const
{
	return awayReason;
}

/* Return if we are reachable (defaults to true because
   we can send on- and offline */
bool JabberContact::isReachable ()
{
	return true;
}

KActionCollection *JabberContact::customContextMenuActions ()
{
	KActionCollection *actionCollection = new KActionCollection (this);

	new KAction (i18n ("(Re)send Authorization To"), "mail_forward", 0, this, SLOT (slotSendAuth ()), actionCollection, "actionSendAuth");
	new KAction (i18n ("(Re)request Authorization From"), "mail_reply", 0, this, SLOT (slotRequestAuth ()), actionCollection, "actionRequestAuth");
	// FIXME wrong position in the popup menu
	new KAction (i18n ("Remove Authorization From"), "mail_delete", 0, this, SLOT (slotRemoveAuth ()), actionCollection, "actionRemoveAuth");

	KActionMenu *actionSetAvailability = new KActionMenu (i18n ("Set Availability"), "kopeteavailable", actionCollection, "jabber_online");

	actionSetAvailability->insert(new KAction (i18n ("Online"),         static_cast<JabberProtocol *>(protocol())->JabberKOSOnline.iconFor(this), 0, this, SLOT (slotStatusOnline ()), actionSetAvailability, "actionOnline"));
	actionSetAvailability->insert(new KAction (i18n ("Free to Chat"),   static_cast<JabberProtocol *>(protocol())->JabberKOSChatty.iconFor(this), 0, this, SLOT (slotStatusChatty ()), actionSetAvailability, "actionChatty"));
	actionSetAvailability->insert(new KAction (i18n ("Away"),           static_cast<JabberProtocol *>(protocol())->JabberKOSAway.iconFor(this), 0, this, SLOT (slotStatusAway ()), actionSetAvailability, "actionAway"));
	actionSetAvailability->insert(new KAction (i18n ("Extended Away"),  static_cast<JabberProtocol *>(protocol())->JabberKOSXA.iconFor(this), 0, this, SLOT (slotStatusXA ()), actionSetAvailability, "actionXA"));
	actionSetAvailability->insert(new KAction (i18n ("Do Not Disturb"), static_cast<JabberProtocol *>(protocol())->JabberKOSDND.iconFor(this), 0, this, SLOT (slotStatusDND ()), actionSetAvailability, "actionDND"));
	actionSetAvailability->insert(new KAction (i18n ("Invisible"),      static_cast<JabberProtocol *>(protocol())->JabberKOSInvisible.iconFor(this), 0, this, SLOT (slotStatusInvisible ()), actionSetAvailability, "actionInvisible"));

	KGlobal::config ()->setGroup ("Jabber");

	// if the contact is online,
	// display the resources we have for it
	if (onlineStatus ().status () != KopeteOnlineStatus::Offline)
	{
		QStringList items;
		int activeItem = 0;
		JabberResource *tmpBestResource = bestResource ();

		/*
		 * FIXME: This is a workaround for the automatic garbage collection
		 *        in KopeteContact. The reparenting of the menu actions
		 *        makes the signal/slot mechanism break in the case of
		 *        a KSelectAction. Due to this, a KActionMenu is instantiated
		 *        instead as a replacement below.
		 */
		KActionMenu *actionSelectResource = new KActionMenu (i18n ("Select Resource"), "connect_no", actionCollection, "actionSelectResource");

		// put best resource first
		items.append (i18n ("Automatic (best resource)"));

		if (!tmpBestResource->resource ().isNull ())
			items.append (tmpBestResource->resource ());

		// iterate through available resources
		int i = 1;

		for (JabberResource * tmpResource = resources.first (); tmpResource; tmpResource = resources.next (), i++)
		{
			// skip the default (empty) resource
			if (tmpResource->resource ().isNull ())
			{
				i--;
				continue;
			}

			// only add the item if it is not the best resource
			// (which we already added above)
			if (tmpResource != tmpBestResource)
				items.append (tmpResource->resource ());

			// mark the currently active resource
			if (tmpResource->resource () == activeResource->resource () && resourceOverride)
			{
				kdDebug (14130) << "[JabberContact] Activating item " << i << " as active resource." << endl;
				activeItem = i;
			}
		}

		// now go through the string list and add the resources with their icons
		i = 0;
		for(QStringList::iterator it = items.begin(); it != items.end(); it++)
		{
			if(i == activeItem)
				actionSelectResource->insert(new KAction ((*it), "button_ok", 0, this, SLOT (slotSelectResource ()), actionSelectResource, QString("%1").arg(i)));
			else
				actionSelectResource->insert(new KAction ((*it), "", 0, this, SLOT (slotSelectResource ()), actionSelectResource, QString("%1").arg(i)));

			i++;
		}

	}

	return actionCollection;

}

void JabberContact::slotUpdatePresence (const KopeteOnlineStatus & newStatus, const QString & reason)
{
	awayReason = reason;

	setOnlineStatus (newStatus);
}

void JabberContact::slotUpdateContact (const Jabber::RosterItem & item)
{

	rosterItem = item;

	// only update the nickname if its not empty
	if (!item.name ().isEmpty () && !item.name ().isNull ())
		setDisplayName (item.name ());

}

void JabberContact::slotRenameContact (const QString &oldName, const QString &newName)
{
	QString name = newName;

	kdDebug (14130) << k_funcinfo << "Renaming contact " << oldName << " to " << newName << endl;

	// if the name has been deleted, revert
	// to using the user ID instead
	if (name.isEmpty())
		name = userId ();

	rosterItem.setName (name);

	// send rename request to protocol backend
	if (!account()->isConnected())
	{
		static_cast<JabberAccount *>(account())->errorConnectFirst();
		return;
	}

	Jabber::JT_Roster * rosterTask = new Jabber::JT_Roster (static_cast<JabberAccount *>(account())->client()->rootTask ());

	rosterTask->set (rosterItem.jid (), rosterItem.name (), rosterItem.groups ());
	rosterTask->go (true);

}

void JabberContact::slotDeleteContact ()
{
	kdDebug (14130) << k_funcinfo << "Removing user " << userId () << endl;

	// unsubscribe
	if (!account()->isConnected ())
	{
		static_cast<JabberAccount *>(account())->errorConnectFirst ();
		return;
	}

	Jabber::JT_Roster * rosterTask = new Jabber::JT_Roster (static_cast<JabberAccount *>(account())->client()->rootTask ());

	rosterTask->remove (rosterItem.jid ());
	rosterTask->go (true);

}

void JabberContact::slotSendAuth ()
{
	kdDebug (14130) << "[JabberContact] (Re)send auth " << userId () << endl;
	sendSubscription ("subscribed");
}

void JabberContact::slotRequestAuth ()
{
	kdDebug (14130) << "[JabberContact] (Re)request auth " << userId () << endl;
	sendSubscription ("subscribe");
}

void JabberContact::slotRemoveAuth ()
{
	kdDebug (14130) << "[JabberContact] Remove auth " << userId () << endl;
	sendSubscription ("unsubscribed");
}

void JabberContact::sendSubscription (const QString& subType)
{
	if (!account()->isConnected())
	{
		static_cast<JabberAccount *>(account())->errorConnectFirst();
		return;
	}

	Jabber::JT_Presence * task = new Jabber::JT_Presence (static_cast<JabberAccount *>(account())->client()->rootTask ());

	task->sub (userId(), subType);
	task->go (true);

}

void JabberContact::syncGroups ()
{
	QStringList groups;
	KopeteGroupList groupList = metaContact ()->groups ();

	if (!account()->isConnected())
	{
		static_cast<JabberAccount *>(account())->errorConnectFirst();
		return;
	}

	for (KopeteGroup * g = groupList.first (); g; g = groupList.next ())
		groups += g->displayName();

	rosterItem.setGroups (groups);

	Jabber::JT_Roster * rosterTask = new Jabber::JT_Roster (static_cast<JabberAccount *>(account())->client()->rootTask ());

	rosterTask->set (rosterItem.jid (), rosterItem.name (), rosterItem.groups ());
	rosterTask->go (true);

}

void JabberContact::km2jm (const KopeteMessage & km, Jabber::Message & jm)
{
	JabberContact *to = static_cast < JabberContact * >(km.to ().first ());
	const JabberContact *from = static_cast < const JabberContact * >(km.from ());

	if (!to || !from)
		return;

	// ugly hack, Jabber::Message does not have a setFrom() method
	Jabber::Message jabMessage (Jabber::Jid (from->userId ()));

	// if a resource has been selected for this contact,
	// send to this special resource - otherwise,
	// just send to the server and let the server decide
	if (!to->resource ().isNull ())
		jabMessage.setTo (Jabber::Jid (QString ("%1/%2").arg (to->userId (), 1).arg (to->resource (), 2)));
	else
		jabMessage.setTo (Jabber::Jid (to->userId ()));

	//jabMessage.setFrom(from->userId();
	jabMessage.setSubject (km.subject ());
	jabMessage.setTimeStamp (km.timestamp ());

	if(km.plainBody().find("-----BEGIN PGP MESSAGE-----") != -1)
	{
		// this message is encrypted

		// send dummy header (please don't translate)
		jabMessage.setBody ("This message is encrypted.", false);

		QString encryptedBody = km.plainBody();

		// remove footer
		encryptedBody.truncate(encryptedBody.length() - QString("-----END PGP MESSAGE-----").length() - 2);
		encryptedBody = encryptedBody.right(encryptedBody.length() - encryptedBody.find("\n\n") - 2);
		jabMessage.setXEncrypted (encryptedBody);
	}
	else
	{
		// this message is not encrypted
		jabMessage.setBody (km.plainBody (), false);
	}

	// determine type of the widget and set message type accordingly
	//if (km.type() == KopeteMessage::Chat)
	if (messageManager->view ()->viewType () == KopeteMessage::Chat)
		jabMessage.setType ("chat");
	else
		jabMessage.setType ("normal");

	jm = jabMessage;

}

void JabberContact::slotReceivedMessage (const Jabber::Message & message)
{
	KopeteMessage::MessageType type;
	KopeteContactPtrList contactList;

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Received Message Type:" << message.type () << endl;

	// determine message type
	if (message.type () == "chat")
		type = KopeteMessage::Chat;
	else
		type = KopeteMessage::Email;

	contactList.append (account()->myself ());

	// retrieve and reformat body
	QString body = message.body();

	if(!message.xencrypted().isEmpty())
	{
		body = QString("-----BEGIN PGP MESSAGE-----\n\n") + message.xencrypted() + QString("\n-----END PGP MESSAGE-----\n");
	}

	// convert Jabber::Message into KopeteMessage
	KopeteMessage newMessage (message.timeStamp (), this, contactList, body,
				  message.subject (), KopeteMessage::Inbound,
				  KopeteMessage::PlainText, type);

	// add it to the manager
	manager ()->appendMessage (newMessage);

}

void JabberContact::slotSendMessage (KopeteMessage & message)
{
	Jabber::Message jabberMessage;

	if (account()->isConnected ())
	{
		// convert the message
		km2jm (message, jabberMessage);

		// send it
		static_cast<JabberAccount *>(account())->client()->sendMessage (jabberMessage);

		// append the message to the manager
		manager ()->appendMessage (message);

		// tell the manager that we sent successfully
		manager ()->messageSucceeded ();
	}
	else
	{
		static_cast<JabberAccount *>(account())->errorConnectFirst ();

		// FIXME: there is no messageFailed() yet,
		// but we need to stop the animation etc.
		manager ()->messageSucceeded ();
	}

}

/**
 * Determine the currently best resource for the contact
 */
JabberResource *JabberContact::bestResource ()
{
	JabberResource *resource, *tmpResource;

	// iterate through all available resources
	for (resource = tmpResource = resources.first (); tmpResource; tmpResource = resources.next ())
	{
		kdDebug (14130) << "[JabberContact] Processing resource " << tmpResource->resource () << endl;

		if (tmpResource->priority () > resource->priority ())
		{
			kdDebug (14130) << "[JabberContact] Got better resource " << tmpResource->resource () << " through better priority." << endl;
			resource = tmpResource;
		}
		else
		{
			if (tmpResource->priority () == resource->priority ())
			{
				if (tmpResource->timestamp () >= resource->timestamp ())
				{
					kdDebug (14130) << "[JabberContact] Got better resource " << tmpResource->resource () << " through newer timestamp." << endl;
					resource = tmpResource;
				}
				else
				{
					kdDebug (14130) << "[JabberContact] Discarding resource " << tmpResource->resource () << " with older timestamp." << endl;
				}
			}
			else
			{
				kdDebug (14130) << "[JabberContact] Discarding resource " << tmpResource->resource () << " with worse priority." << endl;
			}
		}
	}

	return resource;

}

void JabberContact::slotResourceAvailable (const Jabber::Jid &, const Jabber::Resource & resource)
{

	kdDebug (14130) << "[JabberContact] Adding new resource '" << resource.
		name () << "' for " << userId () << ", name [" << resource.
		name () << "], priority " << resource.priority () << ", status [" << resource.status ().status () << endl;

	/*
	 * if the new resource already exists, remove the current
	 * instance of it (Psi will emit resourceAvailable()
	 * whenever a certain resource changes status)
	 * removing the current instance will prevent double entries
	 * of the same resource.
	 * updated resource will be re-added below
	 * FIXME: this should be done using the QPtrList methods! (needs checking of pointer values)
	 */
	for (JabberResource * tmpResource = resources.first (); tmpResource; tmpResource = resources.next ())
	{
		if (tmpResource->resource () == resource.name ())
		{
			kdDebug (14130) << "[JabberContact] Resource " << tmpResource->resource () << " already added, removing instance with older timestamp" << endl;
			resources.remove ();
		}
	}

	KopeteOnlineStatus status = static_cast<JabberProtocol *>(protocol())->JabberKOSOnline;

	if (resource.status ().show () == "chat")
	{
		status = static_cast<JabberProtocol *>(protocol())->JabberKOSChatty;
	}
	else if (resource.status ().show () == "away")
	{
		status = static_cast<JabberProtocol *>(protocol())->JabberKOSAway;
	}
	else if (resource.status ().show () == "xa")
	{
		status = static_cast<JabberProtocol *>(protocol())->JabberKOSXA;
	}
	else if (resource.status ().show () == "dnd")
	{
		status = static_cast<JabberProtocol *>(protocol())->JabberKOSDND;
	}

	JabberResource *newResource = new JabberResource (resource.name (), resource.priority (),
													  resource.status ().timeStamp (), status,
													  resource.status ().status ());

	resources.append (newResource);

	JabberResource *tmpBestResource = bestResource ();

	kdDebug (14130) << "[JabberContact] Best resource is now " << tmpBestResource->resource () << "." << endl;

	slotUpdatePresence (tmpBestResource->status (), tmpBestResource->reason ());

	// switch active resource if override is not in effect
	if (!resourceOverride)
		activeResource = tmpBestResource;

}

void JabberContact::slotResourceUnavailable (const Jabber::Jid & jid, const Jabber::Resource & resource)
{
	JabberResource *tmpResource;

	kdDebug (14130) << "[JabberContact] Removing resource '" << jid.resource () << "' for " << userId () << endl;

	for (tmpResource = resources.first (); tmpResource; tmpResource = resources.next ())
	{
		if (tmpResource->resource () == resource.name ())
		{
			kdDebug (14130) << "[JabberContact] Got a match in " << tmpResource->resource () << ", removing." << endl;

			if (resources.remove ())
				kdDebug (14130) << "[JabberContact] Successfully removed, there are now " << resources.count () << " resources!" << endl;
			else
				kdDebug (14130) << "[JabberContact] Ack! Couldn't remove the resource. Bugger!" << endl;

			break;
		}
	}

	JabberResource *newResource = bestResource ();

	kdDebug (14130) << "[JabberContact] Best resource is now " << newResource->resource () << "." << endl;
	slotUpdatePresence (newResource->status (), newResource->reason ());

	// if override was in effect or we just deleted the current
	// resource, switch to new resource
	if (resourceOverride || (activeResource->resource () == resource.name ()))
	{
		resourceOverride = false;
		activeResource = newResource;
	}

}

void JabberContact::slotSelectResource ()
{
	int currentItem = QString(static_cast<const KAction *>(sender())->name()).toUInt();

	if (currentItem == 0)
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Removing active resource, trusting bestResource()." << endl;

		resourceOverride = false;
		activeResource = bestResource ();
	}
	else
	{
		QString selectedResource = static_cast<const KAction *>(sender())->text();

		kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Moving to resource " << selectedResource << endl;

		resourceOverride = true;

		for (JabberResource * resource = resources.first (); resource; resource = resources.next ())
		{
			if (resource->resource () == selectedResource)
			{
				kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "New active resource is " << resource->resource () << endl;

				activeResource = resource;

				break;
			}
		}
	}

}

void JabberContact::slotUserInfo ()
{

	if (!static_cast<JabberAccount *>(account())->isConnected ())
	{
		static_cast<JabberAccount *>(account())->errorConnectFirst ();
		return;
	}

	Jabber::JT_VCard * task = new Jabber::JT_VCard (static_cast<JabberAccount *>(account())->client()->rootTask ());

	// signal to ourselves when the vCard data arrived
	QObject::connect (task, SIGNAL (finished ()), this, SLOT (slotGotVCard ()));

	task->get (userId ());

	task->go (true);

}

void JabberContact::slotGotVCard ()
{
	Jabber::JT_VCard * vCard = (Jabber::JT_VCard *) sender ();

	if (!vCard->success () && !vCard->vcard ().isIncomplete ())
	{
		// unsuccessful, or incomplete
		KMessageBox::error (qApp->mainWidget (), i18n ("Unable to retrieve vCard for %1").arg (vCard->jid ().userHost ()));
		return;
	}

	kdDebug (14130) << "[JabberContact] Got vCard for user " << vCard->jid ().userHost () << ", displaying." << endl;

	dlgVCard = new dlgJabberVCard (qApp->mainWidget (), "dlgJabberVCard", vCard);

	if (mEditingVCard)
	{
		connect (dlgVCard, SIGNAL (saveAsXML (QDomElement &)), this, SLOT (slotSaveVCard (QDomElement &)));
		dlgVCard->setReadOnly (false);
		mEditingVCard = false;
	}
//	else
//		connect (dlgVCard, SIGNAL (updateNickname (const QString &)), this, SLOT (slotDoRenameContact (const QString &)));

	dlgVCard->show ();
	dlgVCard->raise ();

}

void JabberContact::slotEditVCard ()
{

	mEditingVCard = true;
	slotUserInfo ();

}

void JabberContact::slotSaveVCard (QDomElement & vCardXML)
{

	if (!static_cast<JabberAccount *>(account())->isConnected ())
	{
		static_cast<JabberAccount *>(account())->errorConnectFirst ();
		return;
	}

	Jabber::JT_VCard * task = new Jabber::JT_VCard (static_cast<JabberAccount *>(account())->client()->rootTask ());
	Jabber::VCard vCard = Jabber::VCard ();

	vCard.fromXml (vCardXML);

	task->set (vCard);
	task->go (true);

}

void JabberContact::slotStatusOnline ()
{

	QString id = userId ();

	if (resourceOverride)
		id += activeResource->resource ();

	static_cast<JabberAccount *>(account())->sendPresenceToNode (static_cast<JabberProtocol *>(protocol())->JabberKOSOnline, id);

}

void JabberContact::slotStatusChatty ()
{

	QString id = userId ();

	if (resourceOverride)
		id += activeResource->resource ();

	static_cast<JabberAccount *>(account())->sendPresenceToNode (static_cast<JabberProtocol *>(protocol())->JabberKOSChatty, id);

}

void JabberContact::slotStatusAway ()
{

	QString id = userId ();

	if (resourceOverride)
		id += activeResource->resource ();

	static_cast<JabberAccount *>(account())->sendPresenceToNode (static_cast<JabberProtocol *>(protocol())->JabberKOSAway, id);

}

void JabberContact::slotStatusXA ()
{

	QString id = userId ();

	if (resourceOverride)
		id += activeResource->resource ();

	static_cast<JabberAccount *>(account())->sendPresenceToNode (static_cast<JabberProtocol *>(protocol())->JabberKOSXA, id);

}

void JabberContact::slotStatusDND ()
{
	QString id = userId ();

	if (resourceOverride)
		id += activeResource->resource ();

	static_cast<JabberAccount *>(account())->sendPresenceToNode (static_cast<JabberProtocol *>(protocol())->JabberKOSDND, id);

}

void JabberContact::slotStatusInvisible ()
{
	QString id = userId ();

	if (resourceOverride)
		id += activeResource->resource ();

	static_cast<JabberAccount *>(account())->sendPresenceToNode (static_cast<JabberProtocol *>(protocol())->JabberKOSInvisible, id);
}

void JabberContact::serialize (QMap < QString, QString > &serializedData, QMap < QString, QString > & /* addressBookData */ )
{

	// Contact id and display name are already set for us, only add the rest
	serializedData["identityId"] = account()->accountId();

	serializedData["groups"] = rosterItem.groups ().join (QString::fromLatin1 (","));
}

#include "jabbercontact.moc"

// vim: set noet ts=4 sts=4 sw=4:
