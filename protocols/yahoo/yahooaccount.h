/*
    yahooaccount.h - Manages a single Yahoo account

    Copyright (c) 2003 by Gav Wood               <gav@kde.org>
    Copyright (c) 2003 by Matt Rogers            <mattrogers@sbcglobal.net>
    Based on code by Olivier Goffart             <ogoffart@kde.org>

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


#ifndef YAHOOACCOUNT_H
#define YAHOOACCOUNT_H

// Qt
#include <qobject.h>
#include <qmap.h>
#include <QPixmap>
#include <QPair>

// KDE
#include <kurl.h>

// Kopete
#include "kopetepasswordedaccount.h"

// Local
#include "yahooprotocol.h"
#include "yahootypes.h"

class QColor;
class KAction;
class KActionMenu;
class YahooContact;
class YahooAccount;
class YahooProtocol;
class YahooWebcam;
class YahooConferenceChatSession;
class YahooChatChatSession;
class KTemporaryFile;

namespace Kopete{
class Transfer;
class ChatSession;
class StatusMessage;
class FileTransferInfo;
}
namespace KYahoo {
	class Client;
}
struct YABEntry;
class KJob;
namespace KIO{
	class Job;
}

class YahooAccount : public Kopete::PasswordedAccount
{
	Q_OBJECT

public:

	enum SignalConnectionType { MakeConnections, DeleteConnections };

	YahooAccount(YahooProtocol *parent,const QString& accountID);
	~YahooAccount();

	/*
	 * Returns a contact of name @p id
	 */
	YahooContact *contact(const QString &id);

	virtual void fillActionMenu( KActionMenu *actionMenu );

	/**
	 * Sets the yahoo away status
	 */
	virtual void setAway(bool, const QString &);

	/**
	 * The session
	 */
	KYahoo::Client *yahooSession();

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

	/**
	 * Set Buddy Icon
	 */
	void setBuddyIcon( const KUrl &url );

	void verifyAccount( const QString &word );

	void sendConfMessage( YahooConferenceChatSession *s, const Kopete::Message &message );
	void sendChatMessage( const Kopete::Message &msg, const QString &handle );
	void prepareConference( const QString &who );
	void sendFile( YahooContact *to, const KUrl &url );
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
	void setOnlineStatus( const Kopete::OnlineStatus&, const Kopete::StatusMessage &reason = Kopete::StatusMessage(),
	                      const OnlineStatusOptions& options = None );
	void setStatusMessage(const Kopete::StatusMessage&);
signals:
	/**
	 * Emitted when we receive notification that the person we're talking to is typing
	 */
	void receivedTypingMsg(const QString &contactId, bool isTyping);

	/**
	 * Emitted when our Buddy Icon has changed
	 */
	void signalBuddyIconChanged( int type );

protected:
	/**
	 * Adds our Yahoo contact to a metacontact
	 */
	virtual bool createContact(const QString &contactId,  Kopete::MetaContact *parentContact);

	virtual bool createChatContact( const QString &nick );

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
	void slotOpenInbox();			// Open Yahoo Mailbox in browser
	void slotOpenYAB();			// Open Yahoo Addressbook in browser
	void slotEditOwnYABEntry();		// Show own Yahoo Addressbook entry
	void slotJoinChatRoom();
	void slotChatCategorySelected( const Yahoo::ChatCategory &category );

	void slotGoStatus(int status, const QString &awayMessage = QString());
	void slotLoginResponse(int succ, const QString &url);
	void slotDisconnected();
	void slotLoginFailed();
	void slotGotBuddy(const QString &userid, const QString &alias, const QString &group);
	void slotBuddyAddResult(const QString &userid, const QString &group, bool success);
	void slotBuddyRemoveResult(const QString &userid, const QString &group, bool success);
	void slotBuddyChangeGroupResult(const QString &userid, const QString &group, bool success);
	void slotAuthorizationAccepted( const QString &who );
	void slotAuthorizationRejected( const QString &who, const QString &msg );
	void slotgotAuthorizationRequest( const QString &, const QString &, const QString & );
	void slotAddedInfoEventActionActivated( uint actionId );
	void slotGotIgnore(const QStringList &);
	void slotGotIdentities(const QStringList &);
	void slotStatusChanged(const QString &who, int stat, const QString &msg, int away, int idle, int pictureChecksum);
	void slotStealthStatusChanged(const QString &who, Yahoo::StealthStatus state);
	void slotGotIm(const QString &who, const QString &msg, long tm, int stat);
	void slotGotBuzz(const QString &who, long tm);
	void slotGotConfInvite(const QString &who, const QString &room, const QString &msg, const QStringList &members);
	void slotConfUserDecline(const QString &who, const QString &room, const QString &msg);
	void slotConfUserJoin(const QString &who, const QString &room);
	void slotConfUserLeave(const QString &who, const QString &room);
	void slotConfMessage(const QString &who, const QString &room, const QString &msg);
	void slotConfLeave( YahooConferenceChatSession *s );
	void slotInviteConference( const QString &room, const QStringList &who, const QStringList &members, const QString &msg );
	void slotAddInviteConference( const QString &room, const QStringList &who, const QStringList &members, const QString &msg );
	void slotGotFile(const QString &who, const QString &url, long expires, const QString &msg, const QString &fname, unsigned long fesize, const QPixmap &);
	void slotContactAdded(const QString &myid, const QString &who, const QString &msg);
	void slotRejected(const QString &, const QString &);
	void slotTypingNotify(const QString &, int );
	void slotGameNotify(const QString &, int);
	void slotMailNotify(const QString &, const QString &, int);
	void slotSystemMessage(const QString &);
	void slotRemoveHandler(int fd);
	//void slotHostConnect(const QString &host, int port);
	void slotGotWebcamInvite(const QString &);
	void slotWebcamNotAvailable( const QString &who );
	void slotGotWebcamImage(const QString&, const QPixmap&);
	void slotWebcamReadyForTransmission();
	void slotWebcamStopTransmission();
	void slotOutgoingWebcamClosing();
	void slotWebcamClosed(const QString&, int);
	void slotWebcamPaused(const QString&);
	void slotWebcamViewerJoined( const QString & );
	void slotWebcamViewerLeft( const QString & );
	void slotWebcamViewerRequest( const QString & );
	void slotPictureStatusNotify( const QString&, int);
	void slotGotBuddyIcon(const QString&, const QByteArray&, int);
	void slotGotBuddyIconInfo(const QString&, KUrl, int);
	void slotGotBuddyIconChecksum(const QString&, int);
	void slotGotBuddyIconRequest(const QString &);
	void slotBuddyIconChanged(const QString&, int);
	void slotGotYABEntry( YABEntry *entry );
	void slotGotYABRevision( long revision, bool merged );
	void slotSaveYABEntry( YABEntry &entry );
	void slotModifyYABEntryError( YABEntry *entry, const QString & );
	void slotChatJoined( int roomId, int categoryId, const QString &comment, const QString &handle );
	void slotChatBuddyHasJoined( const QString &nick, const QString &handle, bool suppressNotification );
	void slotChatBuddyHasLeft( const QString &nick, const QString &handle );
	void slotChatMessageReceived( const QString &nick, const QString &message, const QString &handle );
	void slotLeavChat();

	void slotReceiveFileAccepted( Kopete::Transfer *trans, const QString& fileName );
	void slotReceiveFileRefused( const Kopete::FileTransferInfo& info );
	void slotFileTransferComplete( unsigned int id );
	void slotFileTransferError( unsigned int id, int error, const QString &desc );
	void slotFileTransferBytesProcessed( unsigned int id, unsigned int bytes );
	void slotFileTransferResult( KJob * );
	void slotError( int level );

private:

	/**
	 * Handle the signal and slot connections and disconnects
	 */
	void initConnectionSignals( enum SignalConnectionType sct );

	void setupActions( bool connected );

	QString prepareIncomingMessage( const QString &msg );

	/**
	 * internal (to the plugin) controls/flags
	 * This should be kept in sync with server - if a buddy is removed, this should be changed accordingly.
	 */
	QMap<QString, QPair<QString, QString> > IDs;

	/**
	 * Conferences list, maped by room name (id)
	 */
	QMap<QString, YahooConferenceChatSession *> m_conferences;
	YahooChatChatSession * m_chatChatSession;
	QStringList m_pendingConfInvites;
	QStringList m_pendingWebcamInvites;
	QStringList m_pendingFileTransfers;

	QMap<unsigned int, Kopete::Transfer *> m_fileTransfers;

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
	long m_YABLastMerge;		// The YAB Revision on which the last merge was done
	long m_YABLastRemoteRevision;	// The last remote YAB Revision on which a sync was done
	YahooProtocol *m_protocol;	// The Protocol Object

	YahooWebcam *m_webcam;

	KAction *m_openInboxAction;	// Menu item openInbox
	KAction *m_openYABAction;	// Menu item openYahooAddressbook
	KAction *m_editOwnYABEntry;	// Menu item editOwnYABEntry
	KAction *m_joinChatAction;	// Menu item joinChatAction
	
	KYahoo::Client *m_session;		// The Connection object
};

#endif // YAHOOACCOUNT_H
