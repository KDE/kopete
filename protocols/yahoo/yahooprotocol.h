/*
    yahooprotocol.h - Yahoo Plugin for Kopete

    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Copyright (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef YAHOOPROTOCOL_H
#define YAHOOPROTOCOL_H


// Local Includes
#include "yahooprefs.h"
#include "kyahoo.h"

// Kopete Includes

// QT Includes
#include <qpixmap.h>

// KDE Includes
#include "kopeteprotocol.h"

class StatusBarIcon;	// libkopete::ui::statusbaricon
class KPopupMenu;
class KActionMenu;
class KAction;
class KopeteMetaContact;

// Yahoo Protocol
class YahooProtocol : public KopeteProtocol
{
	Q_OBJECT

public:

	static YahooProtocol *protocol();

	/* Plugin reimplementation */
	void init();
	bool unload();
	bool serialize( KopeteMetaContact *metaContact, QStringList &strList ) const;
	void deserialize( KopeteMetaContact *metaContact, const QStringList &strList );
	QStringList addressBookFields() const;

	YahooProtocol( QObject *parent, const char *name, const QStringList &args );
	~YahooProtocol();	// Destructor

	KopeteContact* myself() const;
	void addContact(QString);

public slots:
	void Connect();			// Connect to server
	void Disconnect();		// Disconnect from server
	void setAvailable();	// Set user Available
	void setAway();			// Set user away

	bool isConnected() const;	// Return true if connected
	bool isAway() const;		// Return true if away

	QString protocolIcon() const;	// Return protocol icon name
	AddContactPage *createAddContactWidget(QWidget * parent);
									// Return "add contact" dialog

	void slotIconRightClicked(const QPoint&);
						// CallBack when clicking on statusbar icon
	void slotSettingsChanged(void);
						// Callback when settings changed
	void slotNewContact(QString, QString, QString);
						// XXX ?

	// XXX void slotConnect();
	// XXX void slotDisconnect();

signals:
	void protocolUnloading();	// Unload Protocol

protected slots:
	void slotConnected();

private:
	bool mIsConnected;				// Am I connected ?
	QString mUsername, mPassword, mServer; int mPort;
									// Configuration data
	StatusBarIcon *statusBarIcon;	// Statusbar Icon Object
	KPopupMenu *popup;				// Statusbar Popup
	YahooPreferences *mPrefs;		// Preferences Object
	YahooSession *m_session;			// Connection Object

	QPixmap onlineIcon;				// Icons
	QPixmap offlineIcon;
	QPixmap busyIcon;
	QPixmap idleIcon;
	QPixmap mobileIcon;

	void initIcons();	// Load Icons
	void initActions();	// Load Status Actions

	KActionMenu *actionStatusMenu; // Statusbar Popup
	KAction *actionGoOnline;	// Available
	KAction *actionGoOffline;	// Disconnected
	KAction *actionGoStatus001; // Be Right Back
	KAction *actionGoStatus002; // Busy
	KAction *actionGoStatus003; // Not At Home
	KAction *actionGoStatus004; // Not At My Desk
	KAction *actionGoStatus005; // Not In The Office
	KAction *actionGoStatus006; // On The Phone
	KAction *actionGoStatus007; // On Vacation
	KAction *actionGoStatus008; // Out To Lunch
	KAction *actionGoStatus009; // Stepped Out
	KAction *actionGoStatus012; // Invisible
	KAction *actionGoStatus099; // Custom
	KAction *actionGoStatus999; // Idle

	static YahooProtocol* protocolStatic_;

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

