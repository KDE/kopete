/*
    kyahoo.cpp - QT libyahoo2 wrapper

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

// Local Includes
#include "yahoodebug.h"
#include "kyahoo.h"

// Kopete Includes
#include <statusbaricon.h>

// QT Includes
#include <qregexp.h>
#include <qsocketnotifier.h>

// KDE Includes
#include <kdebug.h>
#include <kiconloader.h>

#include "libyahoo2/yahoo2.h"
#include "libyahoo2/yahoo2_callbacks.h"
#include <iostream.h>
#include <errno.h>

/* Those gave me undefined reference errors */
#define MAX_PREF_LEN 255
char pager_host[MAX_PREF_LEN]="scs.yahoo.com";
char pager_port[MAX_PREF_LEN]="5050";
char filetransfer_host[MAX_PREF_LEN]="filetransfer.msg.yahoo.com";
char filetransfer_port[MAX_PREF_LEN]="80";

YahooSessionManager::YahooSessionManager()
{
	if ( managerStatic_ )
		kdDebug() << "Yahoo manager already initialized" << endl;
	else
		managerStatic_ = this;
}

YahooSessionManager::~YahooSessionManager()
{
	managerStatic_ = 0L;
}

YahooSession* YahooSessionManager::login(const QString username, const QString password, int initial)
{
	int id;
    YahooSession *session;
	kdDebug() << "[YahooSessionManager::login] login!!!..."<< endl;
	id = yahoo_login( username.latin1() , password.latin1(), initial);
    kdDebug() << "[YahooSessionManager::login] got id "<< id << " !, creating session"<< endl;
	session = new YahooSession( id, username, password, initial);
	m_sessionsMap[id] = session;
	m_fdMap[m_fd]=id;
//	m_fd=0;
	session->addHandler(m_fd);
	return session;
}

bool YahooSessionManager::logout()
{
    QMap< int, YahooSession*>::iterator it;
	for ( it=m_sessionsMap.begin(); it != m_sessionsMap.end(); it++)
	{
		it.data()->logOff();
		delete it.data();
		m_socketsMap[ socketDescriptor(it.key())]->close();
		delete m_socketsMap[ socketDescriptor(it.key())];
		m_sessionsMap.remove(it.key());
		m_socketsMap.remove(socketDescriptor(it.key()));
		kdDebug() << "[YahooSessionManager::logout] "<< it.key() << endl;      
	}

	return true;
}

YahooSession* YahooSessionManager::getSession(int id)
{
	return m_sessionsMap[id] ? m_sessionsMap[id] : 0L;
}

int YahooSessionManager::getSessionCount(){
    return m_sessionsMap.count();
}

YahooSessionManager* YahooSessionManager::managerStatic_ = 0L;

YahooSessionManager* YahooSessionManager::manager()
{
	if ( managerStatic_ )
		return managerStatic_;
	else
		return ( new YahooSessionManager );
}

YahooSession::YahooSession(int id, const QString username, const QString password, int initial)
{
	m_Username = username;
	m_Password = password;
	m_Status = initial;
	m_connId = id;
	QObject::connect ( this,SIGNAL(loginResponse(int,char*)),this,SLOT(loginResponseReceiver(int,char*)));
}

YahooSession::~YahooSession()
{
	
}

int YahooSession::setLogLevel(enum yahoo_log_level level)
{
	return yahoo_set_log_level(level);
}

void YahooSession::logOff()
{
	kdDebug()<<"[YahooSession::logOff]"<<m_connId<<endl;
	yahoo_logoff(m_connId);
}

void YahooSession::refresh()
{
	yahoo_refresh(m_connId);
}

void YahooSession::setIdentityStatus( const char * identity, int active)
{
	yahoo_set_identity_status(m_connId, identity, active);
}

void YahooSession::getList()
{
	yahoo_get_list(m_connId);
}

void YahooSession::keepalive()
{
	yahoo_keepalive(m_connId);
}

void YahooSession::sendIm( const char *from, const char *who, const char *msg)
{
	yahoo_send_im(m_connId, from, who, msg);
}

void YahooSession::sendTyping( const char *from, const char *who, int typ)
{
	yahoo_send_typing(m_connId, from, who, typ);
}

void YahooSession::setAway( enum yahoo_status state, const char *msg, int away)
{
	yahoo_set_away(m_connId,state,msg,away);
}

void YahooSession::addBuddy( const char *who, const char *group)
{
	yahoo_add_buddy(m_connId, who, group);
}

void YahooSession::removeBuddy( const char *who, const char *group)
{
	yahoo_remove_buddy( m_connId, who, group);
}

void YahooSession::rejectBuddy( const char *who, const char *msg)
{
	yahoo_reject_buddy( m_connId , who, msg);
}

void YahooSession::ignoreBuddy( const char *who, int unignore)
{
	yahoo_ignore_buddy( m_connId, who, unignore);

}

void YahooSession::changeBuddyGroup( const char *who, const char *old_group, const char *new_group)
{
	yahoo_change_buddy_group(m_connId,who,old_group,new_group);	
}

void YahooSession::conferenceInvite( const char * from, YList *who, const char *room, const char *msg)
{
	yahoo_conference_invite( m_connId, from, who, room, msg);
}

void YahooSession::conferenceAddinvite( const char * from, const char *who, const char *room, const YList * members, const char *msg)
{
	yahoo_conference_addinvite(m_connId, from, who, room, members, msg);
}

void YahooSession::conferenceDecline( const char * from, YList *who, const char *room, const char *msg)
{
	yahoo_conference_decline(m_connId, from, who, room, msg);
}

void YahooSession::conferenceMessage( const char * from, YList *who, const char *room, const char *msg)
{
	yahoo_conference_message(m_connId, from, who, room, msg);
}

void YahooSession::YahooSession::conferenceLogon( const char * from, YList *who, const char *room)
{
	yahoo_conference_logon(m_connId, from, who, room);
}

void YahooSession::conferenceLogoff( const char * from, YList *who, const char *room)
{
	yahoo_conference_logoff(m_connId, from, who, room);
}

int YahooSession::sendFile( const char *who, const char *msg, const char *name, long size)
{
	return yahoo_send_file(m_connId, who, msg, name, size);

}

int YahooSession::getUrlHandle( const char *url, char *filename, unsigned long *filesize)
{
	return yahoo_get_url_handle(m_connId, url, filename, filesize);
}

int YahooSession::readReady( int fd)
{
	return yahoo_read_ready(m_connId,fd);
}

int YahooSession::writeReady( int fd)
{
	return yahoo_write_ready(m_connId,fd);
}

enum yahoo_status YahooSession::currentStatus()
{
	return yahoo_current_status(m_connId);
}

const YList * YahooSession::getBuddylist()
{
	return yahoo_get_buddylist(m_connId);
}

const YList * YahooSession::getIgnorelist()
{
	return yahoo_get_ignorelist(m_connId);
}

const YList * YahooSession::getIdentities()
{
	return yahoo_get_identities(m_connId);
}

const char  * YahooSession::getCookie( const char *which)
{
	return yahoo_get_cookie(m_connId, which);
}

const char  * YahooSession::getProfile_url( void )
{
	return yahoo_get_profile_url();
}

void YahooSession::loginResponseReceiver(int succ, char *url)
{
	char buff[1024];

	kdDebug()<<"[YahooSession::loginResponseReceiver]"<<endl;
	if (succ == YAHOO_LOGIN_OK)
	{
		m_Status = yahoo_current_status(m_connId);
		kdDebug() << "logged in" << endl;
		return;
	}
	else if(succ == YAHOO_LOGIN_PASSWD)
	{
		snprintf(buff, sizeof(buff), "Could not log into Yahoo service.  Please verify that your username and password are correctly typed.");
	}
	else if(succ == YAHOO_LOGIN_LOCK)
	{
		snprintf(buff, sizeof(buff), "Could not log into Yahoo service.  Your account has been locked.\nVisit %s to reactivate it.", url);
	}
	else if(succ == YAHOO_LOGIN_DUPL)
	{
		snprintf(buff, sizeof(buff), "You have been logged out of the yahoo service, possibly due to a duplicate login.");
	}
	m_Status = YAHOO_STATUS_OFFLINE;
	kdDebug()<<buff<<endl;
        //yahoo_logout();
}

void YahooSession::addHandler(int fd)
{
	kdDebug()<<"YahooSession::addHandler WHAT THE FUCK IS THIS!";
	m_fd = fd;
}

void YahooSession::dataReceived()
{
	int ret=1;
	/* FIXME!!!!! */
    //int fd = (const_cast<KExtendedSocket*>(QObject::sender()))->fd();
	int fd = YahooSessionManager::manager()->socketDescriptor( m_connId );

	kdDebug() << "YahooSession::dataReceived" << fd << endl;
	ret = yahoo_read_ready( m_connId , fd );

	if ( ret == -1)
		kdDebug() <<"Read Error (" << errno << ": " << strerror(errno) << endl;
	else if ( ret == 0)
		kdDebug() << "Server closed socket" << endl;
}
/* Callbacks implementation */

extern "C" {


void YAHOO_CALLBACK_TYPE(ext_yahoo_login_response)(int id, int succ, char *url)
{
	(YahooSessionManager::manager())->loginResponseReceiver(id,succ,url);
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_got_buddies)(int id, YList * buds)
{
	(YahooSessionManager::manager())->gotBuddiesReceiver(id, buds);
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_got_ignore)(int id, YList * igns)
{
	(YahooSessionManager::manager())->gotIgnoreReceiver(id,igns);
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_got_identities)(int id, YList * ids)
{
	(YahooSessionManager::manager())->gotIdentitiesReceiver(id, ids);
	
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_status_changed)(int id, char *who, int stat, char *msg, int away)
{
	YahooSessionManager::manager()->statusChangedReceiver(id,who,stat,msg,away);
	
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_got_im)(int id, char *who, char *msg, long tm, int stat)
{
	YahooSessionManager::manager()->gotImReceiver(id,who,msg,tm,stat);
	
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_got_conf_invite)(int id, char *who, char *room, char *msg, YList *members)
{
	YahooSessionManager::manager()->gotConfInviteReceiver(id,who,room,msg,members);
	
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_conf_userdecline)(int id, char *who, char *room, char *msg)
{
	YahooSessionManager::manager()->confUserDeclineReceiver(id,who,room,msg);
	
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_conf_userjoin)(int id, char *who, char *room)
{
	YahooSessionManager::manager()->confUserJoinReceiver(id,who,room);
	
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_conf_userleave)(int id, char *who, char *room)
{
	YahooSessionManager::manager()->confUserLeaveReceiver(id,who,room);
	
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_conf_message)(int id, char *who, char *room, char *msg)
{
	YahooSessionManager::manager()->confMessageReceiver(id,who,room,msg);
	
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_got_file)(int id, char *who, char *url, long expires, char *msg, char *fname, unsigned long fesize)
{
	YahooSessionManager::manager()->gotFileReceiver(id,who,url,expires,msg,fname,fesize);
	
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_contact_added)(int id, char *myid, char *who, char *msg)
{
	YahooSessionManager::manager()->contactAddedReceiver(id,myid,who,msg);
	
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_rejected)(int id, char *who, char *msg)
{
	YahooSessionManager::manager()->rejectedReceiver(id,who,msg);
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_typing_notify)(int id, char *who, int stat)
{
	YahooSessionManager::manager()->typingNotifyReceiver(id,who,stat);
	
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_game_notify)(int id, char *who, int stat)
{
	YahooSessionManager::manager()->gameNotifyReceiver(id,who,stat);
	
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_mail_notify)(int id, char *from, char *subj, int cnt)
{
	YahooSessionManager::manager()->mailNotifyReceiver(id,from,subj,cnt);
	
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_system_message)(int id, char *msg)
{
	YahooSessionManager::manager()->systemMessageReceiver(id, msg);
	
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_error)(int id, char *err, int fatal)
{
	YahooSessionManager::manager()->errorReceiver(id,err,fatal);
	
}

int YAHOO_CALLBACK_TYPE(ext_yahoo_log)(char *fmt, ...)
{
	/* Do nothing? */	
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_add_handler)(int id, int fd, yahoo_input_condition cond)
{
	YahooSessionManager::manager()->addHandlerReceiver(id,fd,cond);
	
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_remove_handler)(int id, int fd)
{
	YahooSessionManager::manager()->removeHandlerReceiver(id,fd);
	
}

int YAHOO_CALLBACK_TYPE(ext_yahoo_connect)(char *host, int port)
{
	return YahooSessionManager::manager()->hostConnectReceiver(host,port);
}

/* End of extern C */
}

/* Session Manager receiver functions needed to emit signals in YahooSession
	since it ahs access being a friend class */

void YahooSessionManager::loginResponseReceiver( int id, int succ, char *url)
{
    kdDebug() << "[YahooSessionManager::loginResponseReceiver]" << endl;
	YahooSession *session = getSession(id);
	emit session->loginResponse(succ, url);
}

void YahooSessionManager::gotIgnoreReceiver(int id, YList * igns)
{
    kdDebug() << "[YahooSessionManager::removeHandlerReceiver]1" << endl;
	YahooSession *session = getSession(id);
	emit session->gotIgnore(igns);
}

void YahooSessionManager::gotBuddiesReceiver(int id, YList * buds)
{
    kdDebug() << "[YahooSessionManager::removeHandlerReceiver]2" << endl;
	YahooSession *session = getSession(id);
	emit session->gotBuddies(buds);
}

void YahooSessionManager::gotIdentitiesReceiver(int id, YList *ids)
{
    kdDebug() << "[YahooSessionManager::removeHandlerReceiver]3" << endl;
	YahooSession *session = getSession(id);
	emit session->gotIdentities(ids);
}

void YahooSessionManager::statusChangedReceiver(int id, char *who, int stat, char *msg, int away)
{
    kdDebug() << "[YahooSessionManager::removeHandlerReceiver]4" << endl;
	YahooSession *session = getSession(id);
	emit session->statusChanged(who, stat, msg, away);
}

void YahooSessionManager::gotImReceiver(int id, char *who, char *msg, long tm, int stat)
{
	YahooSession *session = getSession(id);
	kdDebug()<<"got IM"<<endl;
	emit session->gotIm(who, msg, tm, stat);	
}

void YahooSessionManager::gotConfInviteReceiver(int id, char *who, char *room, char *msg, YList *members)
{
	YahooSession *session = getSession(id);
	emit session->gotConfInvite(who, room, msg, members);	
}

void YahooSessionManager::confUserDeclineReceiver(int id, char *who, char *room, char *msg)
{
	YahooSession *session = getSession(id);
	emit session->confUserDecline(who, room, msg);	
}

void YahooSessionManager::confUserJoinReceiver(int id, char *who, char *room)
{
	YahooSession *session = getSession(id);
	emit session->confUserJoin(who,room);	
}

void YahooSessionManager::confUserLeaveReceiver(int id, char *who, char *room)
{
	YahooSession *session = getSession(id);
	emit session->confUserLeave(who, room);	
}

void YahooSessionManager::confMessageReceiver(int id, char *who, char *room, char *msg)
{
	YahooSession *session = getSession(id);
	emit session->confMessage(who, room, msg);	
}

void YahooSessionManager::gotFileReceiver(int id, char *who, char *url, long expires, char *msg, char *fname, unsigned long fesize)
{
	YahooSession *session = getSession(id);
	emit session->gotFile(who, url, expires,msg,fname,fesize);	

}

void YahooSessionManager::contactAddedReceiver(int id, char *myid, char *who, char *msg)
{
	YahooSession *session = getSession(id);
	emit session->contactAdded(myid, who, msg);	
}

void YahooSessionManager::rejectedReceiver(int id, char *who, char *msg)
{
	YahooSession *session = getSession(id);
	emit session->rejected(who,msg);	
}

void YahooSessionManager::typingNotifyReceiver(int id, char *who, int stat)
{
	YahooSession *session = getSession(id);
	emit session->typingNotify(who,stat);	
}

void YahooSessionManager::gameNotifyReceiver(int id, char *who, int stat)
{
	YahooSession *session = getSession(id);
	emit session->gameNotify(who, stat);	
}

void YahooSessionManager::mailNotifyReceiver(int id, char *from, char *subj, int cnt)
{
    kdDebug() << "[YahooSessionManager::removeHandlerReceiver]5" << endl;
	YahooSession *session = getSession(id);
	emit session->mailNotify(from, subj,cnt);	
}

void YahooSessionManager::systemMessageReceiver(int id, char *msg)
{
    kdDebug() << "[YahooSessionManager::removeHandlerReceiver]6" << endl;
	YahooSession *session = getSession(id);
	emit session->systemMessage(msg);	
}

void YahooSessionManager::errorReceiver(int id, char *err, int fatal)
{
    kdDebug() << "[YahooSessionManager::removeHandlerReceiver]7" << endl;
	YahooSession *session = getSession(id);
	emit session->error(err, fatal);	
}

int YahooSessionManager::logReceiver(char *fmt, ...)
{
    kdDebug() << "[YahooSessionManager::removeHandlerReceiver]8" << endl;
	//emit session->	
}

void YahooSessionManager::addHandlerReceiver(int id, int fd, yahoo_input_condition cond)
{
	KExtendedSocket *_socket = m_socketsMap[fd];

	m_idMap[fd] = id;
	m_fdMap[id] = fd;

    kdDebug() << "[YahooSessionManager::addHandlerReceiver]" <<id<<" "<<fd<< endl;

    YahooSession *_session = m_sessionsMap[id];
	if ( cond == YAHOO_INPUT_READ && _session )
	{
    	kdDebug() << "[YahooSessionManager::addHandlerReceiver] Socket connected to sessions data handler!";
		_socket->enableRead(true);
		connect (_socket,SIGNAL(readyRead()),_session,SLOT(dataReceived()));
	}
	else if ( cond == YAHOO_INPUT_WRITE )
	{
		// cachar que hacer al reves
    }
}

void YahooSessionManager::removeHandlerReceiver(int id, int fd)
{
    kdDebug() << "[YahooSessionManager::removeHandlerReceiver]" << endl;
	YahooSession *session = getSession(id);
	//emit session->removeHandler(fd);	
}

int YahooSessionManager::hostConnectReceiver(char *host, int port)
{
	kdDebug() << "[YahooSessionManager::hostConnectReceiver]" << endl;
    KExtendedSocket *_socket;
	_socket = new KExtendedSocket( host, port );
	m_socketsMap[_socket->fd()] =_socket;

	if (! _socket->connect() )
	{
		kdDebug() << "[YahooSessionManager::hostConnectReceiver] Connected! fd "<< _socket->fd() << endl;
		return _socket->fd();
	}
	else
	{
		kdDebug() << "[YahooSessionManager::hostConnectReceiver] Failed!" << endl;
		return -1;
	}
}


#include "kyahoo.moc"


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

