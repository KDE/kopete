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

#include "libyahoo2/yahoo2.h"
#include "libyahoo2/yahoo2_callbacks.h"

// Local Includes
#include "yahooprefs.h"
#include "kyahoo.h"

// Kopete Includes

// QT Includes
#include <qpixmap.h>
#include <qmap.h>

// KDE Includes
#include "kopeteprotocol.h"

class YahooContact;
class KPopupMenu;
class KActionMenu;
class KAction;
class KopeteMetaContact;
class KopeteMessage;

// Yahoo Protocol
class YahooProtocol : public KopeteProtocol
{
	Q_OBJECT

public:
	static YahooProtocol *protocol();

	/* Plugin reimplementation */
	void init();
	virtual void deserializeContact( KopeteMetaContact *metaContact,
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData );

	YahooProtocol( QObject *parent, const char *name, const QStringList &args );
	~YahooProtocol();

	KopeteContact* myself() const;
	bool addContactToMetaContact(const QString &contactId, const QString &displayName,
	KopeteMetaContact *parentContact);

	virtual KActionMenu* protocolActions();
	YahooContact *contact( const QString &id );
	YahooSession *yahooSession();
	virtual const QString protocolIcon();
	
	bool isOnServer(const QString &id) { return IDs.contains(id); }	// returns true is contact id is on SS contact list
	bool haveContactList() { return theHaveContactList; }
	
public slots:
	void connect();			// Connect to server
	void disconnect();		// Disconnect from server
	void setAvailable();	// Set user Available
	void setAway();			// Set user away

	bool isConnected() const;	// Return true if connected
	bool isAway() const;		// Return true if away

	AddContactPage *createAddContactWidget(QWidget * parent); // Return "add contact" dialog

	void slotSettingsChanged(void);
						// Callback when settings changed
	//void slotConnect();
	void slotGoOffline();
	
	void slotLoginResponse( int succ, const QString &url);
	void slotGotBuddies(const YList * buds);
	void slotGotBuddy(const QString &userid, const QString &alias, const QString &group);
	void slotGotIgnore( YList * igns);
	void slotGotIdentities( const QStringList &);
	void slotStatusChanged( const QString &who, int stat, const QString &msg, int away);
	void slotGotIm( const QString &who, const QString &msg, long tm, int stat);
	void slotGotConfInvite( const QString &who, const QString &room, const QString &msg, const QStringList &members);
	void slotConfUserDecline( const QString &who, const QString &room, const QString &msg);
	void slotConfUserJoin( const QString &who, const QString &room);
	void slotConfUserLeave( const QString &who, const QString &room);
	void slotConfMessage( const QString &who, const QString &room, const QString &msg);
	void slotGotFile( const QString &who, const QString &url, long expires, const QString &msg, const QString &fname, unsigned long fesize);
	void slotContactAdded( const QString &myid, const QString &who, const QString &msg);
	void slotRejected( const QString &, const QString &);
	void slotGameNotify( const QString &, int);
	/**
	 * Mail Notification
	 */
	void slotMailNotify( const QString &, const QString &, int);
	void slotSystemMessage( const QString &);
	void slotError( const QString &, int);
	void slotRemoveHandler( int fd);
	//void slotHostConnect(const QString &host, int port);

	void slotGotBuddiesTimeout();				// timeout for reception of buddies list

signals:
//	void protocolUnloading();	// Unload Protocol

protected slots:
	void slotConnected();

private:
	QMap <QString, YahooContact *> m_contactsMap;
	QMap<QString, QPair<QString, QString> > IDs;	// this should be kept in sync with server - if a buddy is removed, this should be changed accordingly.
	bool theHaveContactList;

	int m_sessionId;	
	
	bool m_isConnected;				// Am I connected ?
	QString m_userId, m_password, m_server; int m_port;
									// Configuration data
	YahooPreferences *m_prefs;		// Preferences Object
	YahooSession *m_session;			// Connection Object
	YahooContact *m_myself;

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

	static YahooProtocol* s_protocolStatic_;
	
	/** The contact's idle time */
	int m_idle;
	/** Timer for sending typing notifications */
	QTimer* m_typingTimer;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

