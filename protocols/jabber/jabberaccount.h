
/***************************************************************************
                jabberaccount.h  -  core Jabber account class
                             -------------------
    begin                : Sat Mar 8 2003
    copyright            : (C) 2003 by Till Gerken <till@tantalo.net>
							Based on JabberProtocol by Daniel Stone <dstone@kde.org>
							and Till Gerken <till@tantalo.net>.

			   Kopete (C) 2001-2003 Kopete developers
			   <kopete-devel@kde.org>.
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
#include "jabberawaydialog.h"
#include "jabbercontact.h"
#include "jabberprotocol.h"
#include "kopeteonlinestatus.h"
#include "ui/dlgjabbersendraw.h"
#include "im.h"
#include "xmpp.h"

// forward declaration required due to cyclic includes
class JabberAwayDialog;

/* @author Daniel Stone, Till Gerken */

class JabberAccount:public KopeteAccount
{
	Q_OBJECT

public:
	  JabberAccount (JabberProtocol * parent, const QString & accountID, const char *name = 0L);
	/* Standard null destructor. */
	 ~JabberAccount ();

	/* Returns the action menu for this account. */
	virtual KActionMenu *actionMenu ();

	virtual void setAway (bool away, const QString & reason = QString::null);

	void setPresence (const KopeteOnlineStatus & status, const QString & reason = 0, int priority = 5);

	/* Return the resource of the client */
	const QString resource () const;
	const QString server () const;
	const int port () const;

	XMPP::Client *client();

	/* to get the protocol from the account */
	JabberProtocol *protocol () const
	{
		return mProtocol;
	}

	/* Tells the user to connect first before they can do whatever it is
	 * that they want to do. */
	void errorConnectFirst ();

	/*
	 * Handle TLS warnings. Displays a dialog and returns the user's choice.
	 * Parameters: Warning code from QCA::TLS, server name
	 * Returns KMessageBox::ButtonCode
	 */
	static int handleTLSWarning (int warning, QString server);

	/*
	 * Handle stream errors. Displays a dialog and returns.
	 */
	static void handleStreamError (int streamError, int streamCondition, int connectorCode, QString server);

public slots:
	/* Connects to the server. */
	void connect ();

	/* Disconnects from the server. */
	void disconnect ();

	/* Sends a presence packet to a node. */
	void sendPresenceToNode (const KopeteOnlineStatus & status, const QString & reason);

	/**
	 * Adds a contact to this protocol with the specified details
	 *
	 * @param contactId The unique ID for this protocol
	 * @param displayName The displayname of the contact (may equal userId for some protocols
	 * @param parentContact The metacontact to add this contact to
	 * @param groupName The name of the group to add the contact to
	 * @param isTemporary If this is a temporary contact
	 * @return Pointer to the KopeteContact object which was added
	 */
	bool addContact( const QString &contactId, const QString &displayName = QString::null,
		KopeteMetaContact *parentContact = 0L, const KopeteAccount::AddMode mode = KopeteAccount::DontChangeKABC,
		const QString &groupName = QString::null, bool isTemporary = false) ;

protected:
	/* Create a new contact in the specified metacontact.
	 * You shouldn't call this method yourself; for adding contacts, see @ref addContact().
	 *
	 * @param contactID The unique ID for this protocol.
	 * @param displayName The display name of the contact (may equal @param contactID).
	 * @param parentContact The metacontact to add this contact to
	 */
	virtual bool addContactToMetaContact (const QString & contactID, const QString & displayName, KopeteMetaContact * parentContact);

private:
	/* Psi backend for this account. */
	XMPP::Client *jabberClient;
	XMPP::ClientStream *jabberClientStream;
	XMPP::AdvancedConnector *jabberClientConnector;
	QCA::TLS *jabberTLS;
	XMPP::QCATLSHandler *jabberTLSHandler;

	/* away dialog instance */
	JabberAwayDialog *awayDialog;

	/* Asks the specified JID for authorization. */
	void subscribe (const Jid & jid);

	/* Accepts another JID's request for authorization. */
	void subscribed (const Jid & jid);

	/* Removes another JID's authorization. */
	void unsubscribed (const Jid & jid);

	void setAvailable ();
	void removeContact (const RosterItem &);

	/* Set up our actions for the status menu. */
	void initActions ();

	void cleanup ();

	JabberProtocol *mProtocol;

	/* Initial presence to set after connecting. */
	KopeteOnlineStatus initialPresence;

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

	/* Login if the connection was OK. */
	void slotCSNeedAuthParams (bool user, bool pass, bool realm);

	/* Called from Psi: tells us when we're logged in OK. */
	void slotCSAuthenticated ();

	/* Called from Psi: tells us when we've been disconnected from the server. */
	void slotCSDisconnected ();

	/* Called from Psi: alerts us to a protocol warning. */
	void slotCSWarning (int);

	/* Called from Psi: alerts us to a protocol error. */
	void slotCSError (int);

	/* Called from Psi: report certificate status */
	void slotTLSHandshaken ();

	/* Called from Psi: debug messages from the backend. */
	void slotPsiDebug (const QString & msg);

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
	void slotGroupChatError (const Jid & jid, int error,const QString & reason);

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

	/* Gets the user's vCard from the server for editing. */
	void slotEditVCard ();

	/* Get the services list from the server for management. */
	void slotGetServices ();

};

#endif
