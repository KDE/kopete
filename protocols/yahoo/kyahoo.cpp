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

// QT Includes
#include <qregexp.h>
#include <qsocketnotifier.h>
#include <qstringlist.h>
#include <qfile.h>

// KDE Includes
#include <kdebug.h>
#include <kiconloader.h>

// System Includes
#include <cstdlib>
#include <cstring>

#include "libyahoo2/yahoo2.h"
#include "libyahoo2/yahoo2_callbacks.h"
#include "libyahoo2/yahoo2_types.h"
#include "libyahoo2/yahoo_list.h"
#include <iostream>
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
		kdDebug(14180) << "Yahoo manager already initialized" << endl;
	else
		managerStatic_ = this;
}

YahooSessionManager::~YahooSessionManager()
{
	managerStatic_ = 0L;
}

void YahooSessionManager::setPager(QString host, int port)
{
	strcpy(pager_host, host);
	QString portString = QString().setNum(port);
	strcpy(pager_port, portString);
}

void YahooSessionManager::setFileTransfer(QString host, int port)
{
	strcpy(filetransfer_host, host);
	QString portString = QString().setNum(port);
	strcpy(filetransfer_port, portString);
}

int YahooSessionManager::socketDescriptor( int session_id )
{ 
	return m_fdMap[session_id] ? m_fdMap[session_id] : 0L; 
}

YahooSession* YahooSessionManager::createSession(const QString username, const QString password, int initial)
{
	int id;
	YahooSession *session;
	kdDebug(14180) << "[YahooSessionManager::createSession] login!!!..."<< endl;
	session = new YahooSession();
	id = session->login( username, password, initial );
	kdDebug(14180) << "[YahooSessionManager::createSession] Session created, got id "<< id << " !"<< endl;

	if (id > 0)
	{
		m_sessionsMap[id] = session;
		//YAHOO_CALLBACK(ext_yahoo_add_handler)( m_connId, yahoo_get_fd(m_connId), YAHOO_INPUT_READ);
		YahooSessionManager::manager()->addHandlerReceiver( id, yahoo_get_fd(id), YAHOO_INPUT_READ);
	}
	return session;
}


int YahooSession::login(const QString username, const QString password, int initial)
{
	m_Username = username;
	m_Password = password;
	m_Status = initial;

	/* We try to login */
	m_connId = yahoo_login( username.local8Bit() , password.local8Bit(), initial);

	if ( m_connId > 0 )
	{
		//QObject::connect ( this,SIGNAL(loginResponse(int,char*)),YahooSessionManager::manager(),SLOT(slotLoginResponseReceiver(int,char*)));
	}
	return m_connId;
}


bool YahooSessionManager::cleanSessions()
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
		kdDebug(14180) << "[YahooSessionManager::logout] "<< it.key() << endl;
	}

	return true;
}

YahooSession* YahooSessionManager::getSession(int id)
{
	return m_sessionsMap[id] ? m_sessionsMap[id] : 0L;
}

int YahooSessionManager::getSessionCount()
{
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

YahooSession::YahooSession()
{
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
	kdDebug(14180)<<"[YahooSession::logOff]"<<m_connId<<endl;
	yahoo_logoff(m_connId);
}

void YahooSession::refresh()
{
	yahoo_refresh(m_connId);
}

void YahooSession::setIdentityStatus( const QString &identity, int active)
{
	yahoo_set_identity_status(m_connId, identity.local8Bit(), active);
}

void YahooSession::getList()
{
	yahoo_get_list(m_connId);
}

void YahooSession::keepalive()
{
	yahoo_keepalive(m_connId);
}

void YahooSession::sendIm( const QString &from, const QString &who, const QString &msg)
{
	yahoo_send_im(m_connId, from.local8Bit(), who.local8Bit(), msg.local8Bit());
}

void YahooSession::sendTyping( const QString &from, const QString &who, int typ)
{
	yahoo_send_typing(m_connId, from.local8Bit(), who.local8Bit(), typ);
}

void YahooSession::setAway( enum yahoo_status state, const QString &msg, int away)
{
	yahoo_set_away(m_connId,state, msg.local8Bit() , away);
}

void YahooSession::addBuddy( const QString &who, const QString &group)
{
	yahoo_add_buddy(m_connId, who.local8Bit(), group.local8Bit());
}

void YahooSession::removeBuddy( const QString &who, const QString &group)
{
	yahoo_remove_buddy( m_connId, who.local8Bit(), group.local8Bit());
}

void YahooSession::rejectBuddy( const QString &who, const QString &msg)
{
	yahoo_reject_buddy( m_connId , who.local8Bit(), msg.local8Bit());
}

void YahooSession::ignoreBuddy( const QString &who, int unignore)
{
	yahoo_ignore_buddy( m_connId, who.local8Bit(), unignore);

}

void YahooSession::changeBuddyGroup( const QString &who, const QString &old_group, const QString &new_group)
{
	yahoo_change_buddy_group(m_connId,who.local8Bit(),old_group.local8Bit(),new_group.local8Bit());
}

void YahooSession::conferenceInvite( const QString & from, const QStringList &who, const QString &room, const QString &msg)
{
	YList *tmplist;
	tmplist = (YList *) malloc(sizeof(YList));

	for ( QStringList::ConstIterator it = who.begin(); it != who.end(); ++it )
	{
		char *member;
		member = strdup((*it).local8Bit());
		y_list_append( tmplist, member );
	}

	yahoo_conference_invite( m_connId, from.local8Bit(), tmplist, room.local8Bit(), msg.local8Bit());

	y_list_free_1(tmplist);
	y_list_free(tmplist);
}

void YahooSession::conferenceAddinvite( const QString & from, const QString &who, const QString &room, const QStringList &members, const QString &msg)
{

	YList *tmplist;
	tmplist = (YList *) malloc(sizeof(YList));

	for ( QStringList::ConstIterator it = members.begin(); it != members.end(); ++it )
	{

		char *member;
		member = strdup((*it).local8Bit());
		y_list_append( tmplist, member );
	}

	yahoo_conference_addinvite(m_connId, from.local8Bit(), who.local8Bit(), room.local8Bit(), tmplist, msg.local8Bit());

	y_list_free_1(tmplist);
	y_list_free(tmplist);
}

void YahooSession::conferenceDecline( const QString & from, const QStringList &who, const QString &room, const QString &msg)
{
	YList *tmplist;
	tmplist = (YList *) malloc(sizeof(YList));

	for ( QStringList::ConstIterator it = who.begin(); it != who.end(); ++it )
	{
		char *member;
		member = strdup((*it).local8Bit());
		y_list_append( tmplist, member );
	}

	yahoo_conference_decline(m_connId, from.local8Bit(), tmplist, room.local8Bit(), msg.local8Bit());

	y_list_free_1(tmplist);
	y_list_free(tmplist);
}

void YahooSession::conferenceMessage( const QString & from, const QStringList &who, const QString &room, const QString &msg)
{
	YList *tmplist;
	tmplist = (YList *) malloc(sizeof(YList));

	for ( QStringList::ConstIterator it = who.begin(); it != who.end(); ++it )
	{
		char *member;
		member = strdup((*it).local8Bit());
		y_list_append( tmplist, member );
	}

	yahoo_conference_message(m_connId, from.local8Bit(), tmplist, room.local8Bit(), msg.local8Bit());

	y_list_free_1(tmplist);
	y_list_free(tmplist);
}

void YahooSession::YahooSession::conferenceLogon( const QString & from, const QStringList &who, const QString &room)
{
	YList *tmplist;
	tmplist = (YList *) malloc(sizeof(YList));

	for ( QStringList::ConstIterator it = who.begin(); it != who.end(); ++it )
	{
		char *member;
		member = strdup((*it).local8Bit());
		y_list_append( tmplist, member );
	}

	yahoo_conference_logon(m_connId, from.local8Bit(), tmplist, room.local8Bit());

	y_list_free_1(tmplist);
	y_list_free(tmplist);
}

void YahooSession::conferenceLogoff( const QString &from, const QStringList &who, const QString &room)
{
	YList *tmplist;
	tmplist = (YList *) malloc(sizeof(YList));

	for ( QStringList::ConstIterator it = who.begin(); it != who.end(); ++it )
	{
		char *member;
		member = strdup((*it).local8Bit());
		y_list_append( tmplist, member );
	}

	yahoo_conference_logoff(m_connId, from.local8Bit(), tmplist, room.local8Bit());

	y_list_free_1(tmplist);
	y_list_free(tmplist);
}

int YahooSession::sendFile( const QString &who, const QString &msg, const QString &name, long size)
{
	return yahoo_send_file(m_connId, who.local8Bit(), msg.local8Bit(), name.local8Bit(), size);

}

int YahooSession::getUrlHandle( const QString &url, const QString &filename, unsigned long *filesize)
{
	char *_url;
	char *_filename;
	int result;

	_url = strdup(url.local8Bit());
	_filename = strdup(QFile::encodeName(filename));

	result = yahoo_get_url_handle(m_connId, _url, _filename, filesize);

	free(_url);
	free(_filename);

	return result;
}

enum yahoo_status YahooSession::currentStatus()
{
	return yahoo_current_status(m_connId);
}

const YList *YahooSession::getLegacyBuddyList()
{
	return yahoo_get_buddylist(m_connId);
}

QStringList YahooSession::getBuddylist()
{
	//return yahoo_get_buddylist(m_connId);
	return QStringList();
}

QStringList YahooSession::getIgnorelist()
{
	//return yahoo_get_ignorelist(m_connId);
	return QStringList();
}

QStringList YahooSession::getIdentities()
{
	//return yahoo_get_identities(m_connId);
	return QStringList();
}

QString YahooSession::getCookie( const QString &which)
{
	return QString(yahoo_get_cookie(m_connId, which));
}

QString YahooSession::getProfile_url( void )
{
	return QString(yahoo_get_profile_url());
}

void YahooSession::slotLoginResponseReceiver(int succ, char *url)
{
	char buff[1024];

	kdDebug(14180)<<"[YahooSession::slotLoginResponseReceiver]"<<endl;
	if (succ == YAHOO_LOGIN_OK)
	{
		m_Status = yahoo_current_status(m_connId);
		kdDebug(14180) << "logged in" << endl;
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
	kdDebug(14180)<<buff<<endl;
        //yahoo_logout();
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

int YAHOO_CALLBACK_TYPE(ext_yahoo_log)(char */*fmt*/, ...)
{
	/* Do nothing? */
	return 0;
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
	kdDebug(14180) << "[YahooSessionManager::loginResponseReceiver]" << endl;
	YahooSession *session = getSession(id);
	emit session->loginResponse(succ, QString(url));
}

void YahooSessionManager::gotIgnoreReceiver(int id, YList * igns)
{
	kdDebug(14180) << "[YahooSessionManager::gotIgnoreReceiver]" << endl;
	YahooSession *session = getSession(id);

	YList *l;
	QStringList ign_list;

	for (l = igns; l; l = l->next)
	{
		struct yahoo_buddy *bud = (yahoo_buddy *) l->data;

		if(! bud)
		{
			kdDebug(14180) << "[YahooSessionManager::gotBuddiesReceiver] Null Id" << endl;
			continue;
		}
		else
		{
			kdDebug(14180) << "[YahooSessionManager::gotBuddiesReceiver] " << bud->id << endl;
			ign_list.append(QString(bud->id));
		}
	}


	emit session->gotIgnore(ign_list);
}

void YahooSessionManager::gotBuddiesReceiver(int id, YList * buds)
{
	kdDebug(14180) << "[YahooSessionManager::gotBuddiesReceiver]" << endl;
	YahooSession *session = getSession(id);

	YList *l;

	for (l = buds; l; l = l->next)
	{
		struct yahoo_buddy *bud = (yahoo_buddy *)l->data;

		if(!bud)
		{
			kdDebug(14180) << "[YahooSessionManager::gotBuddiesReceiver] Null Buddy" << endl;
			continue;
		}
		else
		{
			kdDebug(14180) << "[YahooSessionManager::gotBuddiesReceiver] " << bud->id << endl;
			emit session->gotBuddy( QString(bud->id) , QString::fromLocal8Bit(bud->real_name),
									QString::fromLocal8Bit(bud->group) );
		}
	}
}

void YahooSessionManager::gotIdentitiesReceiver(int id, YList *ids)
{
	kdDebug(14180) << "[YahooSessionManager::gotIdentitiesReceiver]" << endl;
	YahooSession *session = getSession(id);

	YList *l;
	QStringList idslist;

	for (l = ids; l; l = l->next)
	{
		char *userid = (char *) l->data;

		if ( !userid )
		{
			kdDebug(14180) << "[YahooSessionManager::gotBuddiesReceiver] Null Id" << endl;
			continue;
		}
		else
		{
			kdDebug(14180) << "[YahooSessionManager::gotBuddiesReceiver] " << userid << endl;
			idslist.append(QString(userid));
		}
	}

	emit session->gotIdentities(idslist);
}

void YahooSessionManager::statusChangedReceiver(int id, char *who, int stat, char *msg, int away)
{
	kdDebug(14180) << "[YahooSessionManager::statusChangedReceiver]" << endl;
	YahooSession *session = getSession(id);
	emit session->statusChanged( QString::fromLocal8Bit(who), stat, QString::fromLocal8Bit(msg), away);
}

void YahooSessionManager::gotImReceiver(int id, char *who, char *msg, long tm, int stat)
{
	kdDebug(14180) << "[YahooSessionManager::gotImReceiver]" << endl;

	YahooSession *session = getSession(id);
	kdDebug(14180)<<"got IM"<<endl;
	emit session->gotIm( QString::fromLocal8Bit(who), QString::fromLocal8Bit(msg), tm, stat);
}

void YahooSessionManager::gotConfInviteReceiver(int id, char *who, char *room, char *msg, YList *members)
{
	kdDebug(14180) << "[YahooSessionManager::gotConfInviteReceiver]" << endl;

	YahooSession *session = getSession(id);

	YList *l;
	QStringList member_list;

	for (l = members; l; l = l->next)
	{
		struct yahoo_buddy *bud = (yahoo_buddy *) l->data;

		if ( !bud )
		{
			kdDebug(14180) << "[YahooSessionManager::gotBuddiesReceiver] Null Id" << endl;
			continue;
		}
		else
		{
			kdDebug(14180) << "[YahooSessionManager::gotBuddiesReceiver] " << bud->id << endl;
			member_list.append(QString(bud->id));
		}
	}


	emit session->gotConfInvite( QString::fromLocal8Bit(who), QString::fromLocal8Bit(room),
								 QString::fromLocal8Bit(msg), member_list);
}

void YahooSessionManager::confUserDeclineReceiver(int id, char *who, char *room, char *msg)
{
	kdDebug(14180) << "[YahooSessionManager::confUserDeclineReceiver]" << endl;

	YahooSession *session = getSession(id);
	emit session->confUserDecline( QString::fromLocal8Bit(who), QString::fromLocal8Bit(room),
								   QString::fromLocal8Bit(msg));
}

void YahooSessionManager::confUserJoinReceiver(int id, char *who, char *room)
{
	YahooSession *session = getSession(id);
	emit session->confUserJoin( QString::fromLocal8Bit(who), QString::fromLocal8Bit(room));
}

void YahooSessionManager::confUserLeaveReceiver(int id, char *who, char *room)
{
	YahooSession *session = getSession(id);
	emit session->confUserLeave( QString::fromLocal8Bit(who), QString::fromLocal8Bit(room));
}

void YahooSessionManager::confMessageReceiver(int id, char *who, char *room, char *msg)
{
	YahooSession *session = getSession(id);
	emit session->confMessage( QString::fromLocal8Bit(who), QString::fromLocal8Bit(room),
							   QString::fromLocal8Bit(msg));
}

void YahooSessionManager::gotFileReceiver(int id, char *who, char *url, long expires, char *msg, char *fname, unsigned long fesize)
{
	YahooSession *session = getSession(id);
	emit session->gotFile( QString::fromLocal8Bit(who), QString::fromLocal8Bit(url), expires,
						   QString::fromLocal8Bit(msg), QString::fromLocal8Bit(fname), fesize);
}

void YahooSessionManager::contactAddedReceiver(int id, char *myid, char *who, char *msg)
{
	YahooSession *session = getSession(id);
	emit session->contactAdded( QString::fromLocal8Bit(myid), QString::fromLocal8Bit(who),
								QString::fromLocal8Bit(msg));
}

void YahooSessionManager::rejectedReceiver(int id, char *who, char *msg)
{
	YahooSession *session = getSession(id);
	emit session->rejected( QString::fromLocal8Bit(who), QString::fromLocal8Bit(msg));
}

void YahooSessionManager::typingNotifyReceiver(int id, char *who, int stat)
{
	YahooSession *session = getSession(id);
	emit session->typingNotify( QString::fromLocal8Bit(who), stat);
}

void YahooSessionManager::gameNotifyReceiver(int id, char *who, int stat)
{
	YahooSession *session = getSession(id);
	emit session->gameNotify(QString::fromLocal8Bit(who), stat);
}

void YahooSessionManager::mailNotifyReceiver(int id, char *from, char *subj, int cnt)
{
	kdDebug(14180) << "[YahooSessionManager::mailNotifyReceiver] session: " << id <<  endl;
	YahooSession *session = getSession(id);
	emit session->mailNotify(QString::fromLocal8Bit(from), QString::fromLocal8Bit(subj),cnt);
}

void YahooSessionManager::systemMessageReceiver(int id, char *msg)
{
	kdDebug(14180) << "[YahooSessionManager::systemMessageReceiver] session: " << id << endl;
	YahooSession *session = getSession(id);
	emit session->systemMessage(QString::fromLocal8Bit(msg));
}

void YahooSessionManager::errorReceiver(int id, char *err, int fatal)
{
	kdDebug(14180) << "[YahooSessionManager::errorReceiver] session: " << id <<  endl;
	YahooSession *session = getSession(id);
	emit session->error(err, fatal);
}

int YahooSessionManager::logReceiver(char *fmt, ...)
{
	kdDebug(14180) << "[YahooSessionManager::logReceiver]" << endl;
	//emit session->
	return 0;
}

void YahooSessionManager::addHandlerReceiver(int id, int fd, yahoo_input_condition cond)
{
	KExtendedSocket *_socket = m_socketsMap[fd];

	m_idMap[fd] = id;
	m_fdMap[id] = fd;

	kdDebug(14180) << "[YahooSessionManager::addHandlerReceiver] Session: " <<id<< " Socket: " << fd<< endl;

	YahooSession *_session = m_sessionsMap[id];
	_session->setSocket(fd);

	if ( cond == YAHOO_INPUT_READ && _session )
	{
		kdDebug(14180) << "[YahooSessionManager::addHandlerReceiver] add handler read" << endl;
		_socket->enableRead(true);
		connect (_socket,SIGNAL(readyRead()),_session,SLOT(slotReadReady()));
	}
	else if ( cond == YAHOO_INPUT_WRITE )
	{
		kdDebug(14180) << "[YahooSessionManager::addHandlerReceiver] add handler write" << endl;
		_socket->enableWrite(true);
		connect (_socket,SIGNAL(readyWrite()),_session,SLOT(slotWriteReady()));
	}
}

void YahooSession::addHandler(int /*fd*/, yahoo_input_condition /*cond*/)
{
}

void YahooSessionManager::removeHandlerReceiver(int id, int fd)
{
	kdDebug(14180) << "[YahooSessionManager::removeHandlerReceiver]" << endl;
//	YahooSession *session = getSession(id);

	KExtendedSocket *_socket = m_socketsMap[fd];
	YahooSession *_session = m_sessionsMap[id];

	m_idMap.remove(fd);
	m_fdMap.remove(id);

	kdDebug(14180) << "[YahooSessionManager::removeReceiver] Session: " <<id<< " Socket: " << fd<< endl;

	if ( _session && _socket )
	{
		kdDebug(14180) << "[YahooSessionManager::removeHandlerReceiver] read off";
		_socket->enableRead(false);
		disconnect (_socket,SIGNAL(readyRead()),_session,SLOT(slotReadReady()));

		kdDebug(14180) << "[YahooSessionManager::removeHandlerReceiver] write off";
		_socket->enableRead(false);
		disconnect (_socket,SIGNAL(readyWrite()),_session,SLOT(slotWriteReady()));
	}
	else
	{
		kdDebug(14180) << "[YahooSessionManager::removeHandlerReceiver] FATAL ERROR: socket or session NULL";
	}
}

void YahooSession::removeHandler(int /*fd*/)
{
}

int YahooSessionManager::hostConnectReceiver(char *host, int port)
{
	kdDebug(14180) << "[YahooSessionManager::hostConnectReceiver]" << endl;
	KExtendedSocket *_socket;
	_socket = new KExtendedSocket( host, port );

	if (! _socket->connect() )
	{
		kdDebug(14180) << "[YahooSessionManager::hostConnectReceiver] Connected! fd "<< _socket->fd() << endl;

		kdDebug(14180) << "[YahooSessionManager::hostConnectReceiver] Adding socket " << _socket->fd() << " to map" << endl;
		m_socketsMap[_socket->fd()] =_socket;

		return _socket->fd();
	}
	else
	{
		kdDebug(14180) << "[YahooSessionManager::hostConnectReceiver] Failed!" << endl;
		return -1;
	}
}


void YahooSession::setSocket(int fd)
{
	kdDebug(14180)<<"[YahooSession::setSocket] Setting socket " << fd << " for Session "<< m_connId << endl;
	m_fd = fd;
}

void YahooSession::slotReadReady()
{
	int ret=1;
	/* FIXME!!!!! */
    //int fd = (const_cast<KExtendedSocket*>(QObject::sender()))->fd();
	int fd = YahooSessionManager::manager()->socketDescriptor( m_connId );

	kdDebug(14180) << "YahooSession::slotDataReceived" << fd << endl;
	ret = yahoo_read_ready( m_connId , m_fd );

	if ( ret == -1)
		kdDebug(14180) <<"Read Error (" << errno << ": " << strerror(errno) << endl;
	else if ( ret == 0)
		kdDebug(14180) << "Server closed socket" << endl;
}

void YahooSession::slotWriteReady()
{
	int ret=1;
	/* FIXME!!!!! */
    //int fd = (const_cast<KExtendedSocket*>(QObject::sender()))->fd();
	int fd = YahooSessionManager::manager()->socketDescriptor( m_connId );

	kdDebug(14180) << "YahooSession::slotSendReady" << fd << endl;
	ret = yahoo_write_ready( m_connId , m_fd );

	if ( ret == -1)
		kdDebug(14180) <<"Read Error (" << errno << ": " << strerror(errno) << endl;
	else if ( ret == 0)
		kdDebug(14180) << "Server closed socket" << endl;
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

