 /*
    jabberprotocol.h  -  Base class for the Kopete Jabber protocol

    Copyright (c) 2002 by Daniel Stone <dstone@kde.org>
    Copyright (c) 2002 by Till Gerken <till@tantalo.net>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
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
#include <psi/client.h>
#include <psi/types.h>
#include <psi/jid.h>
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "addcontactpage.h"
#include "jabberprefs.h"
#include "jabbermap.h"
#include "jabbermessagemanager.h"

class JabberContact;
class dlgJabberStatus;
class dlgJabberSendRaw;

using namespace Jabber;

class JabberProtocol : public KopeteProtocol
{
		Q_OBJECT

		/* Friends can touch each other's private parts. */
		friend class JabberContact; 
		friend class DlgJabberServices;
		friend class DlgJabberRegister;
		friend class DlgJabberBrowse;
		friend class DlgJabberChatJoin;

public:
		/*********************************************************************
		 *
		 * KopeteProtocol reimplementation start
		 *
		 ********************************************************************/
	 
		/**
		 * Object constructor and destructor
		 */
		JabberProtocol(QObject *parent, QString name, QStringList);
		~JabberProtocol();

		/**
		 * Our own Jabber contact associated with the Jabber account
		 */
		KopeteContact *myself() const;

		KActionMenu *protocolActions();
	
		/**
		 * Initialization/deinitialization routines called upon
		 * loading/unloading the plugin
		 */
		void init();
		bool unload();

		/**
		 * Creates the "add contact" dialog specific to this protocol
		 */
		AddContactPage *createAddContactWidget(QWidget * parent);

		/**
		 * Set Stats to "Away"
		 */
		void setAway();

		/**
		 * Set status to "Online"/"Available"
		 */
		void setAvailable();

		/**
		 * Return connection status
		 */
		bool isConnected() const;


		/**
		 * Determine away status
		 */
		bool isAway() const;

		/**
		 * Are we able to relay messages to offline users?
		 */
		bool canSendOffline() const;
	
		/**
		 * Deserialize contact data
		 */
		virtual void deserializeContact( KopeteMetaContact *metaContact,
						const QMap<QString, QString> &serializedData,
						const QMap<QString, QString> &addressBookData );

		/**
		 * addressBookFieldChanged() is a notification slot for changes
		 */
// 		virtual void addressBookFieldChanged(KopeteMetaContact *contact,
// 						const QString &key);

		/*********************************************************************
		 *
		 * KopeteProtocol reimplementation end
		 *
		 ********************************************************************/

		/**
		 * Internal enum for passing on the status as a constant
		 * (instead of a string, how Psi handles it)
		 */
		enum Presence
		{
				STATUS_ONLINE,
				STATUS_AWAY,
				STATUS_XA,
				STATUS_DND,
				STATUS_INVISIBLE,
				STATUS_OFFLINE
		};

		/**
		 * This returns our protocol instance
		 */
		static JabberProtocol *protocol();

		/**
		 * Function called by the configuration dialog,
		 * it will register the account currently specified
		 * in the dialog.
		 */
		void registerUser();

		/**
		 * Function called by the add contact widget,
		 * it will send a subscription request to the
		 * specified user.
		 */
		void addContact(KopeteMetaContact *mc, const QString &userId);

		/**
		 * Function called by JabberContact via
		 * the various widgetd to update roster
		 * information (groups, name)
		 */
		void updateContact(const Jabber::RosterItem &item);

		/**
		 * Removes a contact from the roster
		 */
		void removeContact(const Jabber::RosterItem &item);

		/**
		 * Slot for sending a message
		 */
		void slotSendMessage(Message);
	
		/**
		 * Save a vCard.
		 */
		void slotSaveVCard(QDomElement &);
		
		virtual const QString protocolIcon();

public slots:
		/**
		 * Function to connect to the server
		 */
		virtual void connect();

		/**
		 * Function to disconnect from server
		 */
		virtual void disconnect();

		void setPresence(Presence status, const QString &reason = 0,
						int priority = 5);

		/**
		 * Send a raw message to the server
		 * (usually called by the dialog widget)
		 */
// 		void sendRawMessage(const QString &packet);

		/**
		 * Sends some sort of presence thing....I guess
		 * I wish someone would write docs when they write
		 * code - Ph0nK
		 */
		void sendPresenceToNode(const Presence &, const QString &);

		/**
		 * Slot for retrieving a vCard
		 */
		void slotRetrieveVCard (const Jid &);

signals:		
		void settingsChanged();

private slots:


	/

	 * Slot to connect to the serve

	 *

	void slotConnect()



	/

	 * Slot to disconnect from the serve

	 *

	void slotDisconnect()



	/*

	 * Slot called upon successful connection (called by Psi backend

	 *

	void slotConnected(bool success, int statusCode, const QString &statusString)



	/*

	 * This slot is called from connect() if there ha

	 * been a successful connection to a Jabber server

	 * This function is then responsible for logging in

	 *

	void slotHandshaken()



	/*

	 * Slot called upon successful disconnection (called by Psi backend

	 *

	void slotDisconnected()



	void slotPsiDebug(const QString &msg)



	/*

	 * Slot called if there was a protocol error (called by Psi backend

	 *

	void slotError(const StreamError &)



	/*

	 * Slot for going onlin

	 *

	void slotGoOnline()



	/*

	 * Slot for going offlin

	 *

	void slotGoOffline()



	/*

	 * Slot for going "away

	 *

	void slotGoAway()



	/*

	 * Slot for going "not available

	 *

	void slotGoXA()



	/*

	 * Slot for going "do not disturb

	 *

	void slotGoDND()



	/*

	 * Slot for going to invisible mod

	 *

	void slotGoInvisible()



	/*

	 * Slot for sending a raw message to the serve

	 *

	void slotSendRaw()



	/*

	 * Slot for creating a new empty email windo

	 *

	void slotEmptyMail()

	void slotOpenEmptyMail()



	/*

	 * Slot for joining a new group cha

	 *

	void slotJoinNewChat()

	void slotJoinChat()

	void slotGroupChatJoined(const Jid &jid)

	void slotGroupChatLeave()

	void slotGroupChatLeft(const Jid &jid)

	void slotGroupChatPresence(const Jid &jid, const Status &status)

	void slotGroupChatError(const Jid &jid, int error, QString &reason)



	/*

	 * Slot for sending a chat message to a use

	 *

	void slotChatUser(JabberContact *contact)



	/*

	 * Slot for sending an email message to a use

	 *

	void slotEmailUser(JabberContact *contact)



	/*

	 * Slot to catch a dying message manage

	 *

	void slotMessageManagerDeleted(KopeteMessageManager *manager)



	/*

	 * Cleans up when a contact is destroye

	 *

	void slotContactDestroyed(KopeteContact *c)



	/*

	 * Incoming subscription reques

	 *

	void slotSubscription(const Jid &jid, const QString &type)



	/*

	 * A new item was added to our roster, update contac

	 * list. If this is a new subscription, make sure w

	 * validate it

	 *

	void slotNewContact(const RosterItem &)



	/*

	 * Update a contact's detail

	 *

	void slotContactUpdated(const RosterItem &)



	/*

	 * A user deleted you from his contact list (call from Psi backend)

	 *

	void slotContactDeleted(const RosterItem &)



	/*

	 * Slot to update the configuration dat

	 *

	void slotSettingsChanged(void)



	/*

	 * Slot for notifying the availability of another resource for a contac

	 * (called from Psi backend

	 *

	void slotResourceAvailable(const Jid &, const Resource &)



	/*

	 * Slot for notifying the removal of a certain resource for a contac

	 * (called from Psi backend

	 *

	void slotResourceUnavailable(const Jid &, const Resource &)



	/*

	 * Slot for displaying a new messag

	 *

	void slotNewMessage(const Message &)



	/*

	 * Evaluate results of account registratio

	 *

	void slotRegisterUserDone()



	/*

	 * Slot being called when a vCard arrive

	 *

	void slotGotVCard()



	void slotEditVCard()



	/*

	 * The user wants to manage service

	 *

	void slotGetServices()



private

	typedef QMap<QString, JabberMessageManager*> JabberMessageManagerMap



	/*

	 * Singleton instance of our protocol clas

	 *

	static JabberProtocol *protocolInstance



	/*

	 * This map associates message managers t

	 * a contact's (or chatroom's) userhos

	 *

	JabberMessageManagerMap messageManagerMap



	KAction *actionGoOnline

	KAction *actionGoAway

	KAction *actionGoXA

	KAction *actionGoDND

	KAction *actionGoInvisible

	KAction *actionGoOffline

	KAction *actionJoinChat

	KAction *actionServices

	KAction *actionSendRaw

	KAction *actionEditVCard

	KAction *actionEmptyMail

	KActionMenu *actionStatusMenu



	dlgJabberStatus *reasonDialog

	dlgJabberSendRaw *sendRawDialog



	JabberPreferences *preferences



	JabberContact *myContact



	/*

	 * Initial presence to set after connectin

	 *

	Presence initialPresence



	/*

	 * Jabber client classes per identit

	 *

    Jabber::Client *jabberClient



    /*

     * Flag whether we are to register upon connec

     *

    int registerFlag

   

	/*

	 * Cache for the title ID of the status bar contex

	 * menu to reflect changes in the user@host setting

	 *

	int menuTitleId



	/*

	 * Little helper function to tell the user to connec

	 *

	void errorConnectFirst()



	/*

	 * Create action

	 *

	void initActions()



	/*

	 * Create a new JabberContac

	 *

	JabberContact *createContact(const QString &jid, const QString &alias, const QStringList &groups, KopeteMetaContact *metaContact, const QString &identity)



	/*

	 * Add new contact to the Kopete contact lis

	 * Note: this does not affect the Jabber roster at al

	 *

	void createAddContact(KopeteMetaContact *mc, const Jabber::RosterItem &item)



	/*

	 * Creates a new message manage

	 *

	JabberMessageManager *createMessageManager(JabberContact *to, KopeteMessageManager::WidgetType type)



	/*

	 * Sends a presence element wit

	 * type="subscribe" to ask for authorizatio

	 *

	void subscribe(const Jabber::Jid &jid)



	/*

	 * Sends a presence element wit

	 * type="subscribed" to acknowledge authorizatio

	 *

	void subscribed(const Jabber::Jid &jid)






};

#endif


/*
 * Local variables:
 * mode: c++
 * c-indentation-style: k&r
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

