/***************************************************************************
     jabberprotocol.h  -  Base class for the Kopete Jabber protocol
                             -------------------
    begin                : Thu Aug 22 2002
    copyright            : (C) 2002 by Till Gerken / The Kopete Dev Team
    email                : till@tantalo.net
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
#include "statusbaricon.h"
#include "jabberprefs.h"
#include "jabbermap.h"

class JabberContact;
class dlgJabberStatus;
class dlgJabberSendRaw;

using namespace Jabber;

class JabberProtocol : public KopeteProtocol
{

	Q_OBJECT

	friend class JabberContact; /* Friends can touch each other's private parts. */

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

	/**
	 * Return the protocol icon
	 */
	QString protocolIcon() const;

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
	 * Function to connect to the server
	 */
	void Connect();

	/**
	 * Function to disconnect from server
	 */
	void Disconnect();

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
	 * Serialize  contact data
	 */
	virtual bool serialize(KopeteMetaContact *contact, QStringList &data) const;

	/**
	 * Deserialize contact data
	 */
	virtual void deserialize(KopeteMetaContact *contact, const QStringList &data);

	/**
	 * addressBookFields() returns a list of fields we are interested in
	 * addressBookFieldChanged() is a notification slot for changes
	 */
	/*
	virtual QStringList addressBookFields() const;
	virtual void addressBookFieldChanged(KopeteMetaContact *contact, const QString &key);
	*/

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
	static const JabberProtocol *protocol();

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

public slots:
	void setPresence(Presence status, const QString &reason = 0, int priority = 5);

	/**
	 * Send a raw message to the server (usually called by the dialog widget)
	 */
	void sendRawMessage(const QString &packet);
	void sendPresenceToNode(const Presence &, const QString &);

	/**
	 * Slot for retrieving a vCard
	 */
	void slotRetrieveVCard (const Jid &);

signals:
	void protocolUnloading();
	void settingsChanged();

private slots:

	/*
	 * Slot to connect to the server
	 */
	void slotConnect();

	/*
	 * Slot to disconnect from the server
	 */
	void slotDisconnect();
	
	/**
	 * Slot called upon successful connection (called by Psi backend)
	 */
	void slotConnected(bool success, int statusCode, const QString &statusString);

	/**
	 * This slot is called from connect() if there has
	 * been a successful connection to a Jabber server.
	 * This function is then responsible for logging in.
	 */
	void slotHandshaken();

	/**
	 * Slot called upon successful disconnection (called by Psi backend)
	 */
	void slotDisconnected();

	void slotPsiDebug(const QString &msg);

	/**
	 * Slot called if there was a protocol error (called by Psi backend)
	 */
	void slotError(const StreamError &);

	/**
	 * Slot for going online
	 */
	void slotGoOnline();

	/**
	 * Slot for going offline
	 */
	void slotGoOffline();

	/**
	 * Slot for going "away"
	 */
	void slotGoAway();

	/**
	 * Slot for going "not available"
	 */
	void slotGoXA();

	/**
	 * Slot for going "do not disturb"
	 */
	void slotGoDND();

	/**
	 * Slot for going to invisible mode
	 */
	void slotGoInvisible();

	/**
	 * Slot for sending a raw message to the server
	 */
	void slotSendRaw();

	/**
	 * Create popup menu for right clicks on status icon
	 */
	void slotIconRightClicked(const QPoint&);

	/**
	 * Cleans up when a contact is destroyed
	 */
	void slotContactDestroyed(KopeteContact *c);

	/**
	 * Incoming subscription request
	 */
	void slotSubscription(const Jid &jid, const QString &type);

	/**
	 * A new item was added to our roster, update contact
	 * list. If this is a new subscription, make sure we
	 * validate it.
	 */
	void slotNewContact(const RosterItem &);

	/**
	 * Update a contact's details
	 */
	void slotContactUpdated(const RosterItem &);

	/**
	 * A user deleted you from his contact list (call from Psi backend) 
	 */
	void slotContactDeleted(const RosterItem &);

	/**
	 * Slot to update the configuration data
	 */
	void slotSettingsChanged(void);

	/**
	 * Slot for notifying the availability of another resource for a contact
	 * (called from Psi backend)
	 */
	void slotResourceAvailable(const Jid &, const Resource &);

	/**
	 * Slot for notifying the removal of a certain resource for a contact
	 * (called from Psi backend)
	 */
	void slotResourceUnavailable(const Jid &, const Resource &);

	/**
	 * Slot for displaying a new message
	 */
	void slotNewMessage(const Message &);

	/**
	 * Evaluate results of account registration
	 */
	void slotRegisterUserDone();

	/**
	 * Slot being called when a vCard arrives
	 */
	void slotGotVCard();
	
	void slotEditVCard();

private:
	typedef JabberMap<JabberContact*, KopeteMetaContact*> JabberMetaContactMap;
	typedef QMap<QString, JabberContact*> JabberContactMap;

	/**
	 * Singleton instance of our protocol class
	 */
	static const JabberProtocol *protocolInstance;
	
	/**
	 * This is the local contact list used to keep JabberContacts in
	 * synch with the related metacontacts.
	 */
	JabberMetaContactMap metaContactMap;

	/**
	 * This is the local contact list used to keep the Psi
	 * roster items in synch with the related JabberContacts
	 */
	JabberContactMap contactMap;

	QPixmap onlineIcon;
	QPixmap offlineIcon;
	QPixmap awayIcon;
	QPixmap naIcon;
	QMovie connectingIcon;

	KAction *actionGoOnline;
	KAction *actionGoAway;
	KAction *actionGoXA;
	KAction *actionGoDND;
	KAction *actionGoInvisible;
	KAction *actionGoOffline;
	KAction *actionSendRaw;
	KAction *actionEditVCard;
	KPopupMenu *popup;
	KActionMenu *actionStatusMenu;

	StatusBarIcon *statusBarIcon;

	dlgJabberStatus *reasonDialog;
	dlgJabberSendRaw *sendRawDialog;
	
	JabberPreferences *preferences;

	JabberContact *myContact;

	/**
	 * Initial presence to set after connecting
	 */
	Presence initialPresence;
	
	/**
	 * Jabber client classes per identity
	 */
    Jabber::Client *jabberClient;

	/**
	 * Cache for the title ID of the status bar context
	 * menu to reflect changes in the user@host settings
	 */
	int menuTitleId;

	/**
	 * Load and create icons
	 */
	void initIcons();

	/**
	 * Create actions
	 */
	void initActions();

	/**
	 * Add new contact to the Kopete contact list
	 * Note: this does not affect the Jabber roster at all
	 */
	void createAddContact(KopeteMetaContact *mc, const Jabber::RosterItem &item);

	/**
	 * Sends a presence element with
	 * type="subscribe" to ask for authorization
	 */
	void subscribe(const Jabber::Jid &jid);

	/**
	 * Sends a presence element with
	 * type="subscribed" to acknowledge authorization
	 */
	void subscribed(const Jabber::Jid &jid);

};

#endif


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

