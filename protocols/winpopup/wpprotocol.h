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


// Local Includes
#include "wppreferences.h"
#include "libwinpopup.h"
#include "wpcontact.h"

// Kopete Includes

// QT Includes
#include <qpixmap.h>

// KDE Includes
#include "kopeteprotocol.h"


class StatusBarIcon;	// libkopete::ui::statusbaricon
class KPopupMenu;
class KActionMenu;
class KAction;
class WPContact;

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
	KopeteWinPopup(const QString &SMBClientPath, const QString &InitialSearchHost, const QString &HostName) :
		KWinPopup(SMBClientPath, InitialSearchHost, HostName) {}
};

// WP Protocol
class WPProtocol : public KopeteProtocol
{
	Q_OBJECT

public:
	WPProtocol(QObject *parent, QString name, QStringList);		// Constructor
	~WPProtocol();		// Destructor
	bool unload();		// Unload statusbar icon
	
	WPContact *addContact(const QString &Name);	// Return the contact named "Name", adding one if neccessary
	WPContact *myself();	// Return the user's contact object
	bool checkHost(const QString &Name) { return theInterface->checkHost(Name); }

	const QStringList getGroups() { return theInterface->getGroups(); }
	const QStringList getHosts(const QString &Group) { return theInterface->getHosts(Group); }

	static const WPProtocol *protocol();
	
public slots:
	void Connect();			// Connect to server
	void Disconnect();		// Disconnect from server
	void setAvailable();	// Set user Available
	void setAway();			// Set user away

	void installSamba();	// Modify smb.conf

	bool isConnected() const;	// Return true if connected
	bool isAway() const;		// Return true if away

	QString protocolIcon() const;	// Return protocol icon name

	AddContactPage *createAddContactWidget(QWidget * parent);
							// Return "add contact" dialog
	void slotIconRightClicked(const QPoint);	
							// CallBack when clicking on statusbar icon
	void slotSettingsChanged(void);
							// Callback when settings changed
	void slotNewContact(const QString &userID, const QString &name, const QString &group = "");
							// XXX ?
	void slotSendMessage(const QString &Body, const QString &Destination);
							// Send a message (Body) to a machine (Destination)

private slots:
	void slotGotNewMessage(const QString &Body, const QDateTime &Arrival, const QString &From);

signals:
	void protocolUnloading();	// Unload Protocol

private:
	StatusBarIcon *statusBarIcon;	// Statusbar Icon Object
	KPopupMenu *popup;				// Statusbar Popup
	WPPreferences *mPrefs;			// Preferences Object
	bool available, online;			// true if we're available/online
	QMap<QString, WPContact *> contactList;	// Master contact list
	KopeteWinPopup *theInterface;
	WPContact *theMyself;
	static const WPProtocol *sProtocol;

	QPixmap iconAway;				// Icons
	QPixmap iconAvailable;
	QPixmap iconOffline;
		
	void initIcons();	// Load Icons
	void initActions();	// Load Status Actions

	KActionMenu *actionStatusMenu;	// Statusbar Popup
	KAction *actionGoAvailable;		// Go into normal mode
	KAction *actionGoAway;			// Go into away mode
	KAction *actionGoOffline;		// Go into offline mode
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

