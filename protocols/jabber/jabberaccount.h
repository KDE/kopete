/***************************************************************************
                          jabberaccount.h  -  description
                             -------------------
    begin                : Sat Mar 8 2003
    copyright            : (C) 2003 by Till Gerken (till@tantalo.net)
    email                : kopete-devel@kde.org
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

/**
  *@author Till Gerken, Daniel Stone
  */

class JabberAccount : public KopeteAccount
{
	Q_OBJECT

public:
	/****************************************
	 * KopeteAccount reimplementation start *
	 ****************************************/

	JabberAccount(KopeteProtocol *parent, const QString& accountID, const char *name=0L);
	~JabberAccount();

	/**
	 * this will be called if main-kopete wants
	 * the plugin to set the user's mode to away
	 */
	virtual void setAway(bool);

	/**
	 * Function has to be reimplemented in every single protocol
	 * and return the KopeteContact associated with the 'home' user.
	 * the myself contact MUST be created in the account constructor!
	 *
	 * @return contact associated with the currently logged in user
	 */
	virtual KopeteContact* myself() const;

	/**
	 * return the menu for this account
	 */
	virtual KActionMenu* actionMenu();

	/****************************************
	 * KopeteAccount reimplementation end   *
	 ****************************************/

protected:
	/**
	 * Create a new contact in the specified metacontact
	 * You shouldn't call yourself this method, for adding contact see @ref addContact()
	 *
	 * @param contactId The unique ID for this protocol
	 * @param displayName The displayname of the contact (may equal userId for some protocols
	 * @param parentContact The metacontact to add this contact to
	 */
	virtual bool addContactToMetaContact( const QString &contactId, const QString &displayName,
		 KopeteMetaContact *parentContact );

private:
	/**
	 * JabberContact for this identity
	 */
	JabberContact *myContact;

	/**
	 * Psi backend for this identity
	 */
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

		friend class JabberContact;
	friend class JabberGroupChat;
	friend class dlgJabberServices;
	friend class dlgJabberRegister;
	friend class dlgJabberBrowse;
	friend class dlgJabberChatJoin;
	friend class dlgJabberStatus;

	/* Creates the "add contact" dialog specific to this account. */
	AddContactPage *createAddContactWidget(QWidget * parent);

	/* Sets status to "Away". */
	void setAway();

	/* Sets status to "Online"/"Available". */
	void setAvailable();

	/* Are we able to relay messages to offline users? */
	bool canSendOffline() const { return true; }

	/* Deserialize contact data. */
	virtual void deserializeContact(KopeteMetaContact *metaContact,
					const QMap<QString, QString> &serializedData,
					const QMap<QString, QString> &addressBookData);

	/* Function called by the configuration dialog,
	 * it will register the account currently specified
	 * in the dialog. */
	void registerUser();

public slots:
	/* Connects to the server. */
	virtual void connect();

	/* Disconnects from the server. */
	virtual void disconnect();

	void setPresence(const KopeteOnlineStatus &status, const QString &reason = 0,
    				  int priority = 5);

	/* Sends a presence packet to a node. */
	void sendPresenceToNode(const KopeteOnlineStatus &status, const QString
						&reason);signals:

	void settingsChanged();

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

	/*
	 * Slot for going "not available"
	 */
	void slotGoXA();

	/*
	 * Slot for going "do not disturb"
	 */
	void slotGoDND();

	/*
	 * Slot for going to invisible mode
	 */
	void slotGoInvisible();

	/*
	 * Slot for sending a raw message to the server
	 */
	void slotSendRaw();

	/*
	 * Slot for creating a new empty email window
	 */
	void slotEmptyMail();
	void slotOpenEmptyMail();

	/*
	 * Slots for handling group chats
	 */
	void slotJoinNewChat();
	void slotGroupChatJoined(const Jid &jid);
	void slotGroupChatLeft(const Jid &jid);
	void slotGroupChatPresence(const Jid &jid, const Status &status);
	void slotGroupChatError(const Jid &jid, int error, QString &reason);

	/*
	 * Incoming subscription request
	 */
	void slotSubscription(const Jid &jid, const QString &type);

	/*
	 * A new item was added to our roster, update contact
	 * list. If this is a new subscription, make sure we
	 * validate it.
	 */
	void slotNewContact(const RosterItem &);

	/*
	 * Update a contact's details
	 */
	void slotContactUpdated(const RosterItem &);

	/*
	 * A user deleted you from his contact list (call from Psi backend)
	 */
	void slotContactDeleted(const RosterItem &);

	/*
	 * Slot to update the configuration data
	 */
	void slotSettingsChanged(void);

	/*
	 * Slot for notifying the availability of another resource for a contact
	 * (called from Psi backend)
	 */
	void slotResourceAvailable(const Jid &, const Resource &);

	/*
	 * Slot for notifying the removal of a certain resource for a contact
	 * (called from Psi backend)
	 */
	void slotResourceUnavailable(const Jid &, const Resource &);

	/*
	 * Slot for displaying a new message
	 */
	void slotReceivedMessage(const Message &);

	/*
	 * Evaluate results of account registration
	 */
	void slotRegisterUserDone();

	/*
	 * User wishes to edit his own vCard
	 */
	void slotEditVCard();

	/*
	 * The user wants to manage services
	 */
	void slotGetServices();

private:
	/*
	 * Singleton instance of our protocol class
	 */
	static JabberProtocol *protocolInstance;

	const KopeteOnlineStatus JabberOnline;
	const KopeteOnlineStatus JabberChatty;
	const KopeteOnlineStatus JabberAway;
	const KopeteOnlineStatus JabberXA;
	const KopeteOnlineStatus JabberDND;
	const KopeteOnlineStatus JabberOffline;
	const KopeteOnlineStatus JabberInvisible;

	JabberPreferences *preferences;

	/*
	 * Initial presence to set after connecting
	 */
	KopeteOnlineStatus initialPresence;

	/*
	 * Jabber client classes per identity
	 */
	Jabber::Client *jabberClient;

	/*
	 * Flag whether we are to register upon connect
	 */
	int registerFlag;

	/*
	 * Cache for the title ID of the status bar context
	 * menu to reflect changes in the user@host setting
	 */
	int menuTitleId;

	/*
	 * Little helper function to tell the user to connect
	 */
	void errorConnectFirst();

	/*
	 * Create actions
	 */
	void initActions();

	/*
	 * Create a new JabberContact
	 */
	JabberContact *createContact(const QString &jid, const QString &alias, const
QStringList &groups, KopeteMetaContact *metaContact, const QString &identity);
	/*
	 * Add new contact to the Kopete contact list
	 * Note: this does not affect the Jabber roster at all
	 */
	void createAddContact(KopeteMetaContact *mc, const Jabber::RosterItem &item);

	/*
	 * Sends a presence element with
	 * type="subscribe" to ask for authorization
	 */
	void subscribe(const Jabber::Jid &jid);

	/*
	 * Sends a presence element with
	 * type="subscribed" to acknowledge authorization
	 */
	void subscribed(const Jabber::Jid &jid);

};

#endif
