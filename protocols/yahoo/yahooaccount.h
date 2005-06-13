/*
    yahooaccount.h - Manages a single Yahoo account

    Copyright (c) 2003 by Gav Wood               <gav@kde.org>
    Copyright (c) 2003 by Matt Rogers            <mattrogers@sbcglobal.net>
    Based on code by Olivier Goffart             <ogoffart @ kde.org>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#ifndef YAHOOIDENTITY_H
#define YAHOOIDENTITY_H

// Qt
#include <qobject.h>
#include <qmap.h>

// Kopete
#include "kopetepasswordedaccount.h"
#include "kopeteawaydialog.h"

// Local
#include "yahooconferencemessagemanager.h"
#include "yahooprotocol.h"

class QColor;
class KAction;
class KActionMenu;
class YahooContact;
class YahooAccount;
class YahooProtocol;
class KTempFile;
struct KURL;

class YahooAwayDialog : public KopeteAwayDialog
{
public:
	YahooAwayDialog(YahooAccount *account, QWidget *parent = 0, const char *name = 0);
	virtual void setAway(int awayType);

private:
	YahooAccount *theAccount;
};

class YahooAccount : public Kopete::PasswordedAccount
{
	Q_OBJECT

public:

	enum SignalConnectionType { MakeConnections, DeleteConnections };

	YahooAccount(YahooProtocol *parent,const QString& accountID, const char *name = 0L);
	~YahooAccount();

	/*
	 * Returns a contact of name @p id
	 */
	YahooContact *contact(const QString &id);

	virtual KActionMenu* actionMenu();

	/**
	 * Sets the yahoo away status
	 */
	virtual void setAway(bool, const QString &);

	/**
	 * The session
	 */
	YahooSession *yahooSession();

	/**
	 * Returns true if contact @p id is on the server-side contact list
	 */
	bool isOnServer(const QString &id) { return IDs.contains(id); }

	/**
	 * Returns true if we have the server-side contact list
	 */
	bool haveContactList() const { return theHaveContactList; }

	void setUseServerGroups(bool newSetting);

	void setImportContacts(bool newSetting);

	/**
	 * Set the pager server
	 */
	void setServer( const QString &server );
	
	/**
	 * Set the port of the pager server
	 */
	void setPort( int port );
public slots:
	/**
	 * Connect to the Yahoo service
	 */
	virtual void connectWithPassword( const QString & );
	/**
	 * Disconnect from the Yahoo service
	 */
	virtual void disconnect();

	/** Reimplemented from Kopete::Account */
	void setOnlineStatus( const Kopete::OnlineStatus&, const QString &reason = QString::null);


signals:
	/**
	 * Emitted when we receive notification that the person we're talking to is typing
	 */
	void receivedTypingMsg(const QString &contactId, bool isTyping);

protected:
	/**
	 * Adds our Yahoo contact to a metacontact
	 */
	virtual bool createContact(const QString &contactId,  Kopete::MetaContact *parentContact);

	/**
	 * Gets the just-received message color
	 */
	QColor getMsgColor(const QString& msg);
	/**
	 * Remove color codes from a message
	 */
	QString stripMsgColorCodes(const QString& msg);

protected slots:
	void slotConnected();
	void slotGoOnline();
	void slotGoOffline();
	void slotOpenInbox();				// Open Yahoo Mailbox in browser

	void slotGoStatus(int status, const QString &awayMessage = QString::null);
	void slotLoginResponse(int succ, const QString &url);
	void slotGotBuddies(const YList * buds);
	void slotGotBuddy(const QString &userid, const QString &alias, const QString &group);
	void slotGotIgnore(const QStringList &);
	void slotGotIdentities(const QStringList &);
	void slotStatusChanged(const QString &who, int stat, const QString &msg, int away);
	void slotGotIm(const QString &who, const QString &msg, long tm, int stat);
	void slotGotBuzz(const QString &who, long tm);
	void slotGotConfInvite(const QString &who, const QString &room, const QString &msg, const QStringList &members);
	void slotConfUserDecline(const QString &who, const QString &room, const QString &msg);
	void slotConfUserJoin(const QString &who, const QString &room);
	void slotConfUserLeave(const QString &who, const QString &room);
	void slotConfMessage(const QString &who, const QString &room, const QString &msg);
	void slotGotFile(const QString &who, const QString &url, long expires, const QString &msg, const QString &fname, unsigned long fesize);
	void slotContactAdded(const QString &myid, const QString &who, const QString &msg);
	void slotRejected(const QString &, const QString &);
	void slotTypingNotify(const QString &, int );
	void slotGameNotify(const QString &, int);
	void slotMailNotify(const QString &, const QString &, int);
	void slotSystemMessage(const QString &);
	void slotError(const QString &, int);
	void slotRemoveHandler(int fd);
	//void slotHostConnect(const QString &host, int port);
	void slotGotWebcamInvite(const QString &);
	void slotGotWebcamImage(const QString&, const QPixmap&);
	void slotWebcamClosed(const QString&, int);
	void slotGotBuddyIcon(const QString&, KTempFile*, int);
	void slotGotBuddyIconInfo(const QString&, KURL, int);
	void slotGotBuddyIconChecksum(const QString&, int);

	void slotBuddyListFetched( int numBuddies );

	void slotReceiveFileAccepted( Kopete::Transfer *trans, const QString& fileName );

private slots:
	// various status slots for the action menu
	void slotGoStatus001() { slotGoStatus(1); } // Be Right Back
	void slotGoStatus002() { slotGoStatus(2); } // Busy
	void slotGoStatus003() { slotGoStatus(3); } // Not At Home
	void slotGoStatus004() { slotGoStatus(4); } // Not At My Desk
	void slotGoStatus005() { slotGoStatus(5); } // Not In The Office
	void slotGoStatus006() { slotGoStatus(6); } // On The Phone
	void slotGoStatus007() { slotGoStatus(7); } // On Vacation
	void slotGoStatus008() { slotGoStatus(8); } // Out To Lunch
	void slotGoStatus009() { slotGoStatus(9); } // Stepped Out
	void slotGoStatus012() { slotGoStatus(12); } // Invisible
	void slotGoStatus099() { theAwayDialog->show(99); } // Custom
	void slotGoStatus999() { slotGoStatus(999); } // Idle

private:

	/**
	 * Handle the signal and slot connections and disconnects
	 */
	void initConnectionSignals( enum SignalConnectionType sct );

	/**
	 * internal (to the plugin) controls/flags
	 * This should be kept in sync with server - if a buddy is removed, this should be changed accordingly.
	 */
	QMap<QString, QPair<QString, QString> > IDs;

	/**
	 * Conferences list, maped by room name (id)
	 */
	QMap<QString, YahooConferenceChatSession *> m_conferences;

	bool theHaveContactList;	// Do we have the full server-side contact list yet?
	int stateOnConnection;		// The state to change to on connection

	/**
	 * External Settings and Descriptors
	 */
	bool m_useServerGroups;		// Use the groups on the server for import
	bool m_importContacts;		// Import the contacts from the server
	int m_sessionId;		// The Yahoo session descriptor
	int m_lastDisconnectCode;	// The last disconnect code.
	int m_currentMailCount;
	YahooSession *m_session;	// Connection Object
	YahooProtocol *m_protocol;	// The Protocol Object


	YahooAwayDialog *theAwayDialog;	// Our away message dialog
};


#endif

