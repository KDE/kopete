/***************************************************************************
     wpprotocol.h  -  Base class for the Kopete WP protocol
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@indigoarchive.net

    Based on code from   : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __WPPROTOCOL_H
#define __WPPROTOCOL_H


// QT Includes
#include <qpixmap.h>

// KDE Includes

// Kopete Includes
#include <kopetemetacontact.h>
#include <kopeteprotocol.h>

// Local Includes
#include "wppreferences.h"
#include "libwinpopup.h"
#include "wpcontact.h"
#include "wpaddcontact.h"

class KPopupMenu;
class KActionMenu;
class KAction;
class WPContact;

/**
 * This is a subclass of the KWinPopup class needed in order to use the virtual
 * methods and communicate nicely with Kopete.
 */
class KopeteWinPopup: public KWinPopup
{
	Q_OBJECT

public slots:
	void slotSendMessage(const QString &Body, const QString &Destination) { sendMessage(Body, Destination); }

signals:
	void newMessage(const QString &Body, const QDateTime &Arrival, const QString &From);

protected:
	virtual void receivedMessage(const QString &Body, const QDateTime &Arrival, const QString &From) { emit newMessage(Body, Arrival, From); }

public:
	KopeteWinPopup(const QString &SMBClientPath, const QString &InitialSearchHost, const QString &HostName, int HostCheckFrequency, int MessageCheckFrequency) :
		KWinPopup(SMBClientPath, InitialSearchHost, HostName, HostCheckFrequency, MessageCheckFrequency) {}
};

/**
 * The actual Protocol class used by Kopete.
 */
class WPProtocol : public KopeteProtocol
{
	Q_OBJECT

// Stuff used internally & by colleague classes
public:
	/**
	 * Returns either the existing contact for Name, or creates a new one if not existant.
	 * Creates a new metacontact with Name, if one doesn't already exist.
	 * USE FOR SAFELY CREATING CONTACTS WITHOUT NECESSARILY HAVING A METACONTACT
	 */
	WPContact *getContact(const QString &Name, KopeteMetaContact* theMetaContact = 0);
	const QStringList getGroups() { return theInterface->getGroups(); }
	const QStringList getHosts(const QString &Group) { return theInterface->getHosts(Group); }
	virtual KActionMenu* protocolActions();	// Per-protocol actions for the systray and the status bar

public slots:
	void slotSettingsChanged(void);			// Callback when settings changed
	void installSamba();				// Modify smb.conf to use winpopup-send.sh script

private:
	void initActions();			// Load Status Actions
	KActionMenu *actionStatusMenu;		// Statusbar Popup
	KAction *actionGoAvailable, *actionGoOffline, *actionGoAway;	// Go into normal/away/offline mode

	bool available, online;			// true if we're available/online

	KopeteWinPopup *theInterface;		// Our KopeteWinPopup instance
	WPPreferences *mPrefs;			// Preferences Object
	WPContact *theMyself;			// A contact to return for the API

//	static WPProtocol *protocol() { return sProtocol; }
//	static WPProtocol *sProtocol;

private slots:
	/**
	 * Called when a new message arrives with the message's data.
	 */
	void slotGotNewMessage(const QString &Body, const QDateTime &Arrival, const QString &From);

// KopeteProtocol overloading
public:
	WPProtocol(QObject *parent, QString name, QStringList);
	~WPProtocol();

	AddContactPage *createAddContactWidget(QWidget *parent) { return new WPAddContact(this, parent); }	// Return "add contact" dialog
	KopeteContact *myself() const {	return (KopeteContact *)theMyself; } 					// Return the user's contact object

	bool isConnected() const { return online; }		// Return true if connected
	bool isAway() const { return !available; }		// Return true if away

public slots:
	void Connect();						// Connect to server
	void Disconnect();					// Disconnect from server
	void setAvailable();					// Set user Available
	void setAway();						// Set user away

// KopetePlugin overloading
public:
	QStringList addressBookFields() const;	// Returns the address book fields we're interested in
	bool unload();				// Unload statusbar icon

	void deserialize(KopeteMetaContact *metaContact, const QStringList &strList);	// Deserialises a strlist into a metacontact

// Stuff used by WPContact
public:
	/**
	 * Returns whether or not the named host is online.
	 */
	bool checkHost(const QString &Name) { return theInterface->checkHost(Name); }

public slots:
	/**
	 * Despatches said message to the destination.
	 */
	void slotSendMessage(const QString &Body, const QString &Destination);

	void serialize(KopeteMetaContact *metaContact);	// Serialises a given metacontact
};

#endif

	// Creates a contact from the serialised data
//	KopeteContact *createContact(KopeteMetaContact *parent, const QString &serializedData);

	// Returns a WP contact under the given MetaContact for Name. If theMetaContact is invalid, returns 0.
	// If Name already exists, returns that. If Name doesn't exist, creates a new contact.
	// *** USE FOR SAFELY CREATING CONTACTS UNDER AN EXISTING METACONTACT ***
	// OBSELETE
//	WPContact *addContact(const QString &Name, KopeteMetaContact* theMetaContact);


