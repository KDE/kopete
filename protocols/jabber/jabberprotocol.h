 /*
  * jabberprotocol.h  -  Base class for the Kopete Jabber protocol
  *
  * Copyright (c) 2002 by Daniel Stone <dstone@kde.org>
  * Copyright (c) 2002 by Till Gerken <till@tantalo.net>
  *
  * Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>
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

#ifndef JABBERPROTOCOL_H
#define JABBERPROTOCOL_H

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qmovie.h>
#include <kaction.h>
#include <kpopupmenu.h>

#include "client.h"
#include "types.h"
#include "jid.h"

#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "addcontactpage.h"
#include "jabbermap.h"

#define JABBER_DEBUG_GLOBAL		14130
#define JABBER_DEBUG_PROTOCOL	14131

class JabberContact;
class dlgJabberStatus;
class dlgJabberSendRaw;

using namespace Jabber;

class JabberProtocol:public KopeteProtocol
{
	Q_OBJECT
		/* Friends can touch each other's private parts. */
	friend class JabberAccount;

	friend class JabberContact;
	friend class JabberGroupChat;
	friend class dlgJabberServices;
	friend class dlgJabberRegister;
	friend class dlgJabberBrowse;
	friend class dlgJabberChatJoin;
	friend class dlgJabberStatus;

  public:

	/*********************************************************************
	 *
	 * KopeteProtocol reimplementation start
	 *
	 ********************************************************************/

	/**
	 * Object constructor and destructor
	 */
	  JabberProtocol (QObject * parent, const char *name, const QStringList &);
	 ~JabberProtocol ();

	KActionMenu *protocolActions ();


	static KopeteOnlineStatus getJabberOnline ()
	{
		return JabberOnline;
	};
	static KopeteOnlineStatus getJabberChatty ()
	{
		return JabberChatty;
	};
	static KopeteOnlineStatus getJabberAway ()
	{
		return JabberAway;
	};
	static KopeteOnlineStatus getJabberXA ()
	{
		return JabberXA;
	};
	static KopeteOnlineStatus getJabberDND ()
	{
		return JabberDND;
	};
	static KopeteOnlineStatus getJabberOffline ()
	{
		return JabberOffline;
	};
	static KopeteOnlineStatus getJabberInvisible ()
	{
		return JabberInvisible;
	};



	/**
	 * Creates the "add contact" dialog specific to this protocol
	 */
	//AddContactPage *createAddContactWidget(QWidget * parent);
	virtual AddContactPage *createAddContactWidget (QWidget * parent, KopeteAccount * i);
	virtual EditAccountWidget *createEditAccountWidget (KopeteAccount * account, QWidget * parent);
	virtual KopeteAccount *createNewAccount (const QString & accountId);

	/**
	 * Set status to "Online"/"Available"
	 */
	void setAvailable ();

	/**
	 * Are we able to relay messages to offline users?
	 */
	bool canSendOffline () const;

	/**
	 * Deserialize contact data
	 */
	virtual void deserializeContact (KopeteMetaContact * metaContact,
									 const QMap < QString, QString > &serializedData, const QMap < QString, QString > &addressBookData);

	/*********************************************************************
	 *
	 * KopeteProtocol reimplementation end
	 *
	 ********************************************************************/

	/**
	 * This returns our protocol instance
	 */
	static JabberProtocol *protocol ();

	/**
	 * Function called by the configuration dialog,
	 * it will register the account currently specified
	 * in the dialog.
	 */
	void registerUser ();

	public slots:

	/**
	 * Function to connect to the server
	 */
	  virtual void connectAll ();

	/**
	 * Function to disconnect from server
	 */
	virtual void disconnectAll ();

	void setPresenceAll (const KopeteOnlineStatus & status, const QString & reason = 0, int priority = 5);

	/**
	 * Sends a presence packet to a node
	 */
	//void sendPresenceToNode(const KopeteOnlineStatus & status,
//              const QString & reason);

  private slots:
	/*
	 * Slot to connect to the server
	 */
	//void slotConnect();

	/*
	 * Slot to disconnect from the server
	 */
	//void slotDisconnect();

	/*
	 * Slot called upon successful connection (called by Psi backend)
	 */
	// void slotConnected(bool success, int statusCode,
//             const QString & statusString);

	/*
	 * This slot is called from connect() if there has
	 * been a successful connection to a Jabber server.
	 * This function is then responsible for logging in.
	 */
	//void slotHandshaken();

	/*
	 * Slot called upon successful disconnection (called by Psi backend)
	 */
	//void slotDisconnected();

	//void slotPsiDebug(const QString & msg);

	/*
	 * Slot called if there was a protocol error (called by Psi backend)
	 */
	//void slotError(const StreamError &);

	/*
	 * Slot for going online
	 */
	//void slotGoOnline();

	/*
	 * Slot for going offline
	 */
	//void slotGoOffline();

	/*
	 * Slot for going "chatty"
	 */
	//void slotGoChatty();

	/*
	 * Slot for going "away"
	 */
	//void slotGoAway();

	/*
	 * Slot for going "not available"
	 */
	//void slotGoXA();

	/*
	 * Slot for going "do not disturb"
	 */
	//void slotGoDND();

	/*
	 * Slot for going to invisible mode
	 */
	//void slotGoInvisible();

	/*
	 * Slot for sending a raw message to the server
	 */
	//void slotSendRaw();

	/*
	 * Slot for creating a new empty email window
	 */
	//void slotEmptyMail();
	//void slotOpenEmptyMail();

	/*
	 * Slots for handling group chats
	 */
	//void slotJoinNewChat();
	//void slotGroupChatJoined(const Jid & jid);
	//void slotGroupChatLeft(const Jid & jid);
	//void slotGroupChatPresence(const Jid & jid, const Status & status);
	//void slotGroupChatError(const Jid & jid, int error, QString & reason);

	/*
	 * Incoming subscription request
	 */
	//void slotSubscription(const Jid & jid, const QString & type);

	/*
	 * A new item was added to our roster, update contact
	 * list. If this is a new subscription, make sure we
	 * validate it.
	 */
	//void slotNewContact(const RosterItem &);

	/*
	 * Update a contact's details
	 */
	//void slotContactUpdated(const RosterItem &);

	/*
	 * A user deleted you from his contact list (call from Psi backend)
	 */
	//void slotContactDeleted(const RosterItem &);

	/*
	 * Slot for notifying the availability of another resource for a contact
	 * (called from Psi backend)
	 */
	//void slotResourceAvailable(const Jid &, const Resource &);

	/*
	 * Slot for notifying the removal of a certain resource for a contact
	 * (called from Psi backend)
	 */
	//oid slotResourceUnavailable(const Jid &, const Resource &);

	/*
	 * Slot for displaying a new message
	 */
	//void slotReceivedMessage(const Message &);

	/*
	 * Evaluate results of account registration
	 */
	//void slotRegisterUserDone();

	/*
	 * User wishes to edit his own vCard
	 */
	//void slotEditVCard();

	/*
	 * The user wants to manage services
	 */
	//void slotGetServices();

  private:
	/*
	 * Singleton instance of our protocol class
	 */
	static JabberProtocol *protocolInstance;

	static KopeteOnlineStatus JabberOnline;
	static KopeteOnlineStatus JabberChatty;
	static KopeteOnlineStatus JabberAway;
	static KopeteOnlineStatus JabberXA;
	static KopeteOnlineStatus JabberDND;
	static KopeteOnlineStatus JabberOffline;
	static KopeteOnlineStatus JabberInvisible;

	/*
	 * Initial presence to set after connecting
	 */
	KopeteOnlineStatus initialPresence;

	/*
	 * Jabber client classes per identity
	 */
	Jabber::Client * jabberClient;

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
	void errorConnectFirst ();

	/*
	 * Create actions
	 */
	void initActions ();

	/*
	 * Create a new JabberContact
	 */
	JabberContact *createContact (const QString & jid, const QString & alias,
								  const QStringList & groups, KopeteMetaContact * metaContact, const QString & identity);

	/*
	 * Add new contact to the Kopete contact list
	 * Note: this does not affect the Jabber roster at all
	 */
	//void createAddContact(KopeteMetaContact * mc,
//            const Jabber::RosterItem & item);

	/*
	 * Sends a presence element with
	 * type="subscribe" to ask for authorization
	 */
	void subscribe (const Jabber::Jid & jid);

	/*
	 * Sends a presence element with
	 * type="subscribed" to acknowledge authorization
	 */
	void subscribed (const Jabber::Jid & jid);

};

#endif
