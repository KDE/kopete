/*
    kyahoo.h - QT libyahoo2 wrapper

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

#ifndef KYAHOO_H
#define KYAHOO_H

// QT Includes
#include <qobject.h>
#include <kextendedsocket.h>
#include <qstring.h>
#include <qmap.h>
#include <qpixmap.h>

#include "libyahoo2/yahoo2.h"
#include "libyahoo2/yahoo2_callbacks.h"

// KDE Includes

class YahooSession;
class KExtendedSocket;
class QSocketNotifier;

/* Yahoo Protocol Connection Manager */
class YahooSessionManager : public QObject
{
	Q_OBJECT

public:
	static YahooSessionManager *manager();
	YahooSessionManager();
	~YahooSessionManager();

	int socketDescriptor( int session_id );
	
	/* Creates a new session */
	YahooSession* createSession(const QString username, const QString password, int initial);
	bool cleanSessions();
	YahooSession* getSession(int id);
	int getSessionCount();
	
	/* Sets the host and port for connection to the pager and f.t. servers */
	void setPager(QString host, int port);
	void setFileTransfer(QString host, int port);
	
	/* Receivers for libyahoo callbacks, resolve connection id and emit signal for the correct session */
	void loginResponseReceiver( int id, int succ, char *url);
	void gotIgnoreReceiver(int id, YList *igns);
	void gotBuddiesReceiver(int id, YList *buds);
	void gotidentitiesReceiver(int id, char *who, int stat, char *msg, int away);
	void gotIdentitiesReceiver(int id, YList *ids);
	void statusChangedReceiver(int id, char *who, int stat, char *msg, int away);
	void gotImReceiver(int id, char *who, char *msg, long tm, int stat);
	void gotConfInviteReceiver(int id, char *who, char *room, char *msg, YList *members);
	void confUserDeclineReceiver(int id, char *who, char *room, char *msg);
	void confUserJoinReceiver(int id, char *who, char *room);
	void confUserLeaveReceiver(int id, char *who, char *room);
	void confMessageReceiver(int id, char *who, char *room, char *msg);
	void gotFileReceiver(int id, char *who, char *url, long expires, char *msg, char *fname, unsigned long fesize);
	void contactAddedReceiver(int id, char *myid, char *who, char *msg);
	void rejectedReceiver(int id, char *who, char *msg);
	void typingNotifyReceiver(int id, char *who, int stat);
	void gameNotifyReceiver(int id, char *who, int stat);
	void mailNotifyReceiver(int id, char *from, char *subj, int cnt);
	void systemMessageReceiver(int id, char *msg);
	void errorReceiver(int id, char *err, int fatal);
	int logReceiver(char *fmt, ...);
	void addHandlerReceiver(int id, int fd, yahoo_input_condition cond);
	void removeHandlerReceiver(int id, int fd);
	int hostConnectReceiver(char *host, int port);

private:
	/* id to session */
	QMap< int, YahooSession*> m_sessionsMap;
	/* fd to sockets */
	QMap< int, KExtendedSocket *> m_socketsMap;
	/* id to fd */
	QMap< int,int> m_fdMap;
	/* fd to id */
	QMap< int,int> m_idMap;

	static YahooSessionManager *managerStatic_;
};

// Yahoo Protocol Connection
class YahooSession : public QObject
{
friend class YahooSessionManager;
	Q_OBJECT

public:
	~YahooSession();
	int Id()
	{ return m_connId; };

	int socketDescriptor()
	{ return m_fd; };

	int setLogLevel(enum yahoo_log_level level);

	int login(const QString username, const QString password, int initial);

	void logOff();
	void refresh();
	void setIdentityStatus( const QString &identity, int active);
	void getList();
	void keepalive();
	void sendIm( const QString &from, const QString &who, const QString &msg);
	void sendTyping( const QString &from, const QString &who, int typ);
	void setAway( enum yahoo_status state, const QString &msg, int away);
	void addBuddy( const QString &who, const QString &group);
	void removeBuddy( const QString &who, const QString &group);
	void rejectBuddy( const QString &who, const QString &msg);
	void ignoreBuddy( const QString &who, int unignore);
	void changeBuddyGroup( const QString &who, const QString &old_group, const QString &new_group);
	void conferenceInvite( const QString & from, const QStringList &who, const QString &room, const QString &msg);
	void conferenceAddinvite( const QString & from, const QString &who, const QString &room, const QStringList & members, const QString &msg);
	void conferenceDecline( const QString & from, const QStringList &who, const QString &room, const QString &msg);
	void conferenceMessage( const QString & from, const QStringList &who, const QString &room, const QString &msg);
	void conferenceLogon( const QString & from, const QStringList &who, const QString &room);
	void conferenceLogoff( const QString & from, const QStringList &who, const QString &room);
	int sendFile( const QString &who, const QString &msg, const QString &name, long size);
	int getUrlHandle( const QString &url, const QString &filename, unsigned long *filesize);
	enum yahoo_status currentStatus();
	const YList *getLegacyBuddyList();
	QStringList getBuddylist();
	QStringList getIgnorelist();
	QStringList getIdentities();
	QString getCookie( const QString &which);
	QString getProfile_url( void );

signals:
	/**
	 * emitted when server says login OK
	 */
	void loginResponse( int succ, const QString &url);

	/**
	 * emitted when servers send us our contact list
	 */
	void gotBuddy(const QString &userid, const QString &alias, const QString &group);

	/**
	 * emitted when server notifies us our ignore list
	 */
	void gotIgnore( const QStringList &igns);

	/**
	 * emitted when server notify us our identities
	 */
	void gotIdentities( const QStringList &ids);

	/**
	 * emitted when a contact changes status
	 */
	void statusChanged( const QString &who, int stat, const QString &msg, int away);

	/**
	 * emitted when someone send us a message
	 */
	void gotIm( const QString &who, const QString &msg, long tm, int stat);

	/**
	 * emitted when someone invites us into a conference room
	 */
	void gotConfInvite( const QString &who, const QString &room, const QString &msg, const QStringList &members);

	/**
	 * emitted when someone declines joining a conference room
	 */
	void confUserDecline( const QString &who, const QString &room, const QString &msg);

	/**
	 * emitted when someone joins a conference
	 */
	void confUserJoin( const QString &who, const QString &room);

	/**
	 * emitted when someone leaves a conference
	 */
	void confUserLeave( const QString &who, const QString &room);

	/**
	 * emitted when someone send us a Conference message
	 */
	void confMessage( const QString &who, const QString &room, const QString &msg);

	/**
	 * emitted when someone wants to send us a file
	 */
	void gotFile( const QString &who, const QString &url, long expires, const QString &msg, const QString &fname, unsigned long fesize);

	/**
	 * emitted when a contact is added
	 */
	void contactAdded( const QString &myid, const QString &who, const QString &msg);

	/**
	 * emitted when someone rejects our auth request
	 */
	void rejected( const QString &who, const QString &msg);

	/**
	 * emitted when someone is typing a message
	 */
	void typingNotify( const QString &who, int stat);

	/**
	 * emitted when someone invites us to join a game
	 */
	void gameNotify( const QString &who, int stat);
	/**
	 * Notify that we have mail
	 */
	void mailNotify( const QString &from, const QString &subject, int cnt);

	/**
	 * emitted when Yahoo servers send us a admin message
	 */
	void systemMessage( const QString &msg);

	/**
	 * emitted when error
	 */
	void error( const QString &err, int fatal);
	//void hostConnect(char *host, int port);

	private slots:
	void slotLoginResponseReceiver( int succ, char *url);
	void slotReadReady();
	void slotWriteReady();
	private:
	/* Private constructor */
	YahooSession();

	void addHandler(int fd, yahoo_input_condition cond);
	void removeHandler(int fd);

	KExtendedSocket *m_socket;

	void setSocket(int fd);
	QString m_Username, m_Password, m_Server; // User data
	int m_Port;
	int m_Status;
	int m_connId;
	int m_fd;
	QString m_BuddyListServer; // Buddy List server
	int m_BuddyListPort;
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

