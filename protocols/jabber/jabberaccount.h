/***************************************************************************
                jabberaccount.h  -  core Jabber account class
                             -------------------
    begin                : Sat Mar 8 2003
    copyright            : (C) 2003 by Daniel Stone and Till Gerken
    email                : dstone@kde.org, till@tantalo.net
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
#include <kopeteaccount.h>
#include "jabbercontact.h"

/* @author Daniel Stone, Till Gerken */

class JabberAccount : public KopeteAccount {
	Q_OBJECT
	
	/* Friends can touch each other's private parts. */
	friend class JabberProtocol;
	friend class JabberContact;
	friend class JabberGroupChat;
	friend class dlgJabberServices;
	friend class dlgJabberRegister;
	friend class dlgJabberBrowse;
	friend class dlgJabberChatJoin;
	friend class dlgJabberStatus;

public:
	/* Standard constructor - takes our JabberProtocol as the parent, plus
	 * a unique account ID. */
	JabberAccount(KopeteProtocol *parent, const QString &accountID, const char *name = 0L);

	/* Standard null destructor. */
	~JabberAccount();

	/* Sets the user away, or, conversely, back online (called by the main
	 * away manager).
	 * @param away True: set this account away, false: set this account
	 * as available. */
	virtual void setStatus(KopeteOnlineStatus status);

	/* Returns the contact for this account, must be created in ctor. */
	virtual KopeteContact* myself() const;

	/* Returns the action menu for this account. */
	virtual KActionMenu* actionMenu();
	
	/* Creates the add contact dialog for this account.
	 * @param parent The dialog to add our widget to. */
	AddContactPage *createAddContactWidget(QWidget *parent);

	/* Are we able to relay messages to offline users? */
	bool canSendOffline() const { return true; }

	/* Deserialize contact data.
	 * @param metaContact The metacontact to deserialize.
	 * @param serializedData Output - serialized data.
	 * @param addressBookData Output - serialized address book data. */
	virtual void deserializeContact(KopeteMetaContact *metaContact,
					const QMap<QString, QString> &serializedData,
					const QMap<QString, QString> &addressBookData);

public slots:
	/* Connects to the server. */
	void connect();

	/* Disconnects from the server. */
	void disconnect();

	void setPresence(const KopeteOnlineStatus &status, const QString &reason = 0,
			 int priority = 5);

	/* Sends a presence packet to a node. */
	void sendPresenceToNode(const KopeteOnlineStatus &status, const QString
				&reason);

signals:
	void settingsChanged();
	void statusChanged(KopeteOnlineStatus status);

protected:
	/* Create a new contact in the specified metacontact.
	 * You shouldn't call this method yourself; for adding contacts, see @ref addContact().
	 *
	 * @param contactID The unique ID for this protocol.
	 * @param displayName The display name of the contact (may equal @param contactID).
	 * @param parentContact The metacontact to add this contact to
	 */
	virtual bool addContactToMetaContact(const QString &contactID, const QString &displayName,
		 KopeteMetaContact *parentContact);

private:
	/* JabberContact for this account. */
	JabberContact *myContact;

	/* Psi backend for this account. */
	Jabber::Client *client;

	/* Set up our actions for the status menu. */
	void initActions();

	/* Actions for the menu. */
        KAction *actionGoOnline;
        KAction *actionGoChatty;
        KAction *actionGoAway;
        KAction *actionGoXA;
        KAction *actionGoDND;
        KAction *actionGoInvisible;
        KAction *actionGoOffline;
        KAction *actionJoinChat;
        KAction *actionServices;
        KAction *actionSendRaw;
        KAction *actionEditVCard;
        KAction *actionEmptyMail;
        KActionMenu *actionStatusMenu;

        dlgJabberStatus *reasonDialog;
        dlgJabberSendRaw *sendRawDialog;

	const KopeteOnlineStatus JabberOnline;
	const KopeteOnlineStatus JabberChatty;
	const KopeteOnlineStatus JabberAway;
	const KopeteOnlineStatus JabberXA;
	const KopeteOnlineStatus JabberDND;
	const KopeteOnlineStatus JabberOffline;
	const KopeteOnlineStatus JabberInvisible;

	JabberPreferences *preferences;

	/* Initial presence to set after connecting. */
	KopeteOnlineStatus initialPresence;

	/* Psi backend class, for communication with the server. */
	Jabber::Client *jabberClient;

	/* Do we need to register on connection? */
	int registerFlag;

	/* Caches the title ID of the account context menu. */
	int menuTitleId;

	/* Tells the user to connect first before they can do whatever it is
	 * that they want to do. */
	void errorConnectFirst();

	/*
	 * Create a new JabberContact
	 */
	JabberContact *createContact(const QString &jid, const QString &alias,
				     const QStringList &groups,
				     KopeteMetaContact *metaContact,
				     const QString &identity);

	/* Add new contact to the Kopete contact list; this doesn't actually
	 * affect the Jabber roster, it's just an internal method. */
	void createAddContact(KopeteMetaContact *mc, const Jabber::RosterItem &item);

	/* Asks the specified JID for authorization. */
	void subscribe(const Jabber::Jid &jid);

	/* Accepts another JID's request for authorization. */
	void subscribed(const Jabber::Jid &jid);


private slots:
	/* Connects to the server. */
	void slotConnect();

	/* Disconnects from the server. */
	void slotDisconnect();

	/* Called from Psi: tells us when we've connected OK. */
	void slotConnected(bool success, int statusCode, const QString &statusString);

	/* Login if the connection was OK. */
	void slotHandshaken();

	/* Called from Psi: tells us when we've been disconnected from the server. */
	void slotDisconnected();

	/* Called from Psi: debug messages from the backend. */
	void slotPsiDebug(const QString &msg);

	/* Called from Psi: alerts us to a protocol error. */
	void slotError(const StreamError &);

	/* Set online mode (presence-wise, and connection-wise). */
	 void slotGoOnline();

	/* Set global presence to "free for chat", no reason.. */
	void slotGoChatty();

	/* Set global presence to "away", no reason. */
	void slotGoAway();

	/* Set global presence to "extended away", no reason. */
	void slotGoXA();

	/* Set global presence to "do not disturb", no reason. */
	void slotGoDND();

	/* Set global presence to "invisible", no reason. */
	void slotGoInvisible();

	/* Sends a raw message to the server (warning! use with caution - if your
	 * session screws up after using this, it's seriously not our fault).
	 * "</stream:stream>" is a fun message to send. */
	 void slotSendRaw();

	/* Creates a new empty one-shot window. */
	void slotOneShot();
	void slotOpenEmptyMail();

	/* Slots for handling group chats. */
	void slotJoinNewChat();
	void slotGroupChatJoined(const Jid &jid);
	void slotGroupChatLeft(const Jid &jid);
	void slotGroupChatPresence(const Jid &jid, const Status &status);
	void slotGroupChatError(const Jid &jid, int error, QString &reason);

	/* Incoming subscription request. */
	void slotSubscription(const Jid &jid, const QString &type);

	/* A new item was added to our roster, so update our contact list.
	 * If this is a new subscription, make sure we action it. */
	void slotNewContact(const RosterItem &);

	/* Update a contact's details. */
	void slotContactUpdated(const RosterItem &);

	/* Someone on our contact list revoked their authorization. */
	void slotContactDeleted(const RosterItem &);

	/* Updates the configuration data. */
	void slotSettingsChanged(void);

	/* Someone on our contact list had (another) resource come online. */
	void slotResourceAvailable(const Jid &, const Resource &);

	/* Someone on our contact list had (another) resource go offline. */
	void slotResourceUnavailable(const Jid &, const Resource &);

	/* Displays a new message. */
	void slotReceivedMessage(const Message &);

	/* Checks if we registered OK; proceeds if we did. */
	void slotRegisterUserDone();

	/* Gets the user's vCard from the server for editing. */
	void slotEditVCard();

	/* Get the services list from the server for management. */
	void slotGetServices();

};

#endif
