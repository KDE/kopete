
/***************************************************************************
                jabberaccount.h  -  core Jabber account class
                             -------------------
    begin                : Sat Mar 8 2003
    copyright           : (C) 2003 by Daniel Stone and Till Gerken
    email                 : dstone@kde.org, till@tantalo.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JABBERACCOUNT_H
#define JABBERACCOUNT_H

#include <qwidget.h>
#include <kaction.h>
#include "kopeteaccount.h"
#include "jabbercontact.h"
#include "jabberprotocol.h"
#include "kopeteonlinestatus.h"
#include "ui/dlgjabbersendraw.h"
#include "jid.h"
#include "client.h"
#include "types.h"

/* @author Daniel Stone, Till Gerken */

using namespace Jabber;

class JabberAccount:public KopeteAccount
{
	Q_OBJECT

public:
	  JabberAccount (JabberProtocol * parent, const QString & accountID, const char *name = 0L);
	/* Standard null destructor. */
	 ~JabberAccount ();

	/* Returns the contact for this account, must be created in ctor. */
	virtual KopeteContact *myself () const;

	/* Returns the action menu for this account. */
	virtual KActionMenu *actionMenu ();

	void registerUser ();

	virtual void setAway (bool away, const QString & reason = QString::null);

	/* Return the resource of the client */
	QString resource ();
	QString server ();
	int port ();

	Jabber::Client *client();

	/* Tells the user to connect first before they can do whatever it is
	 * that they want to do. */
	void errorConnectFirst ();

	/* Asks the specified JID for authorization. */
	void subscribe (const Jid & jid);

	/* Accepts another JID's request for authorization. */
	void subscribed (const Jid & jid);

public slots:
	/* Connects to the server. */
	void connect ();

	/* Disconnects from the server. */
	void disconnect ();

	void setPresence (const KopeteOnlineStatus & status, const QString & reason = 0, int priority = 5);

	/* Sends a presence packet to a node. */
	void sendPresenceToNode (const KopeteOnlineStatus & status, const QString & reason);

signals:
	void settingsChanged ();
	void statusChanged (KopeteOnlineStatus status);
	void connected ();
	void disconnected ();
	void connectionAttempt ();

protected:
	/* Create a new contact in the specified metacontact.
	 * You shouldn't call this method yourself; for adding contacts, see @ref addContact().
	 *
	 * @param contactID The unique ID for this protocol.
	 * @param displayName The display name of the contact (may equal @param contactID).
	 * @param parentContact The metacontact to add this contact to
	 */
	virtual bool addContactToMetaContact (const QString & contactID, const QString & displayName, KopeteMetaContact * parentContact);

	//void addContact(KopeteMetaContact*, const QString&);

private:
	/* JabberContact for this account. */
	JabberContact * myContact;

	/* Psi backend for this account. */
	Jabber::Client *jabberClient;

	void setAvailable ();
	void updateContact (const RosterItem &);
	void removeContact (const RosterItem &);

	/* Set up our actions for the status menu. */
	void initActions ();

	/* to get the protocol from the account */
	JabberProtocol *protocol ()
	{
		return mProtocol;
	}

	JabberProtocol *mProtocol;

	dlgJabberStatus *reasonDialog;
	dlgJabberSendRaw *sendRawDialog;

	//JabberPreferences *preferences;

	/* Initial presence to set after connecting. */
	KopeteOnlineStatus initialPresence;

	/* Do we need to register on connection? */
	int registerFlag;

	/* Caches the title ID of the account context menu. */
	int menuTitleId;

	/*
	 * Create a new JabberContact
	 */
	JabberContact *createContact (const QString & jid, const QString & alias,
								  const QStringList & groups, KopeteMetaContact * metaContact);

	/* Add new contact to the Kopete contact list; this doesn't actually
	 * affect the Jabber roster, it's just an internal method. */
	void createAddContact (KopeteMetaContact * mc, const RosterItem & item);

private slots:
		/* Connects to the server. */
	void slotConnect ();

	void slotGoOffline ();

	/* Disconnects from the server. */
	void slotDisconnect ();

	/* Called from Psi: tells us when we've connected OK. */
	void slotConnected (bool success, int statusCode, const QString & statusString);

	/* Login if the connection was OK. */
	void slotHandshaken ();

	/* Called from Psi: tells us when we've been disconnected from the server. */
	void slotDisconnected ();

	/* Called from Psi: debug messages from the backend. */
	void slotPsiDebug (const QString & msg);

	/* Called from Psi: alerts us to a protocol error. */
	void slotError (const StreamError &);

	/* Set online mode (presence-wise, and connection-wise). */
	void slotGoOnline ();

	/* Set global presence to "free for chat", no reason.. */
	void slotGoChatty ();

	/* Set global presence to "away", no reason. */
	void slotGoAway ();

	/* Set global presence to "extended away", no reason. */
	void slotGoXA ();

	/* Set global presence to "do not disturb", no reason. */
	void slotGoDND ();

	/* Set global presence to "invisible", no reason. */
	void slotGoInvisible ();

	/* Sends a raw message to the server (use with caution) */
	void slotSendRaw ();

	/* Slots for handling group chats. */
	void slotJoinNewChat ();
	void slotGroupChatJoined (const Jid & jid);
	void slotGroupChatLeft (const Jid & jid);
	void slotGroupChatPresence (const Jid & jid, const Status & status);
	void slotGroupChatError (const Jid & jid, int error, QString & reason);

	/* Incoming subscription request. */
	void slotSubscription (const Jid & jid, const QString & type);

	/* A new item was added to our roster, so update our contact list.
	 * If this is a new subscription, make sure we action it. */
	void slotNewContact (const RosterItem &);

	/* Update a contact's details. */
	void slotContactUpdated (const RosterItem &);

	/* Someone on our contact list revoked their authorization. */
	void slotContactDeleted (const RosterItem &);

	/* Someone on our contact list had (another) resource come online. */
	void slotResourceAvailable (const Jid &, const Resource &);

	/* Someone on our contact list had (another) resource go offline. */
	void slotResourceUnavailable (const Jid &, const Resource &);

	/* Displays a new message. */
	void slotReceivedMessage (const Message &);

	/* Checks if we registered OK; proceeds if we did. */
	void slotRegisterUserDone ();

	/* Gets the user's vCard from the server for editing. */
	void slotEditVCard ();

	/* Get the services list from the server for management. */
	void slotGetServices ();

};

#endif
