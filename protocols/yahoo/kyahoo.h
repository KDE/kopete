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


// Local Includes

// Kopete Includes

// QT Includes
#include <qobject.h>
#include <kextendedsocket.h>
#include <qstring.h>
#include <qmap.h>

#include "libyahoo2/yahoo2.h"
#include "libyahoo2/yahoo2_callbacks.h"

// KDE Includes

class YahooSession;
class KExtendedSocket;

/* Yahoo Protocol Connection Manager */
class YahooSessionManager : public QObject
{
	Q_OBJECT
public:
	static YahooSessionManager *manager();
	YahooSessionManager();
	~YahooSessionManager();
	YahooSession* login(const QString username, const QString password, int initial);	
	YahooSession* getSession(int id);

	/* Receivers for libyahoo signals, resolve connection id and emit signal
		for the correct session */
	void loginResponseReceiver( int id, int succ, char *url);
	void gotIgnoreReceiver(int id, YList * igns);
	void gotBuddiesReceiver(int id, YList * buds);
	void gotidentitiesReceiver(int id, char *who, int stat, char *msg, int away);

	void gotIdentitiesReceiver(int id, YList * ids);
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
	QMap< int, YahooSession*> m_sessionsMap;
	QMap< int, KExtendedSocket *> m_socketsMap;
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
	
	int getFd();
	int setLogLevel(enum yahoo_log_level level);
	void logOff();
	void refresh();
	void setIdentityStatus( const char * identity, int active);
	void getList();
	void keepalive();
	void sendIm( const char *from, const char *who, const char *msg);
	void sendTyping( const char *from, const char *who, int typ);
	void setAway( enum yahoo_status state, const char *msg, int away);
	void addBuddy( const char *who, const char *group);
	void removeBuddy( const char *who, const char *group);
	void rejectBuddy( const char *who, const char *msg);
	void ignoreBuddy( const char *who, int unignore);
	void changeBuddyGroup( const char *who, const char *old_group, const char *new_group);
	void conferenceInvite( const char * from, YList *who, const char *room, const char *msg);
	void conferenceAddinvite( const char * from, const char *who, const char *room, const YList * members, const char *msg);
	void conferenceDecline( const char * from, YList *who, const char *room, const char *msg);
	void conferenceMessage( const char * from, YList *who, const char *room, const char *msg);
	void conferenceLogon( const char * from, YList *who, const char *room);
	void conferenceLogoff( const char * from, YList *who, const char *room);
	int sendFile( const char *who, const char *msg, const char *name, long size);
	int getUrlHandle( const char *url, char *filename, unsigned long *filesize);
	int readReady( int fd);
	int writeReady( int fd);
	enum yahoo_status currentStatus();
	const YList * getBuddylist();
	const YList * getIgnorelist();
	const YList * getIdentities();
	const char  * getCookie( const char *which);
	const char  * getProfile_url( void );

signals:

	void loginResponse( int succ, char *url);
	void gotBuddies(YList * buds);
	void gotIgnore( YList * igns);
	void gotIdentities( YList * ids);
	void statusChanged( char *who, int stat, char *msg, int away);
	void gotIm( char *who, char *msg, long tm, int stat);
	void gotConfInvite( char *who, char *room, char *msg, YList *members);
	void confUserDecline( char *who, char *room, char *msg);
	void confUserJoin( char *who, char *room);
	void confUserLeave( char *who, char *room);
	void confMessage( char *who, char *room, char *msg);
	void gotFile( char *who, char *url, long expires, char *msg, char *fname, unsigned long fesize);
	void contactAdded( char *myid, char *who, char *msg);
	void rejected( char *who, char *msg);
	void typingNotify( char *who, int stat);
	void gameNotify( char *who, int stat);
	void mailNotify( char *from, char *subj, int cnt);
	void systemMessage( char *msg);
	void error( char *err, int fatal);
	void addHandler( int fd, yahoo_input_condition cond);
	void removeHandler( int fd);
	void hostConnect(char *host, int port);

	private:

	YahooSession(int id, const QString username, const QString password, int initial);
	QString m_Username, m_Password, m_Server; // User data
	int m_Port;
	int m_Status;
	int m_connId;

	QString m_BuddyListServer; // Buddy List server
	int m_BuddyListPort;

	static YahooSession* sessionStatic_;

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

