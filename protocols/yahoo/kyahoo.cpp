 /*
    kyahoo.cpp - Qt Based libyahoo2 wrapper II

    Copyright (c) 2002-2003 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2003 by Matt Rogers <mattrogers@sbcglobal.net>

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
#include "kyahoo.h"
#include "kopeteuiglobal.h"
#include "yahoouserinfo.h"

// QT Includes
#include <qfile.h>
#include <qtimer.h>
#include <qdom.h>

// KDE Includes
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <krun.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kimageio.h>

// System Includes
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <errno.h>

/* exported to libyahoo */
#define MAX_PREF_LEN 255
char pager_host[MAX_PREF_LEN] = "scs.msg.yahoo.com";
char pager_port[MAX_PREF_LEN] = "5050";
char filetransfer_host[MAX_PREF_LEN] = "filetransfer.msg.yahoo.com";
char filetransfer_port[MAX_PREF_LEN] = "80";

char webcam_host[MAX_PREF_LEN] = "webcam.yahoo.com";
char webcam_port[MAX_PREF_LEN] = "5100";
char webcam_description[MAX_PREF_LEN] = "Philips ToUcam Pro";
char local_host[MAX_PREF_LEN] = "";
int conn_type = 1;

struct connect_callback_data
{
	yahoo_connect_callback callback;
	void * callback_data;
	int id;
};

YahooConnectionManager::YahooConnectionManager()
{
}

YahooConnectionManager::~ YahooConnectionManager()
{
}

void YahooConnectionManager::addConnection( KExtendedSocket* socket )
{
	kdDebug(14181) << k_funcinfo << "Adding socket with fd " << socket->fd() << endl;
	m_connectionList.append( socket );
}

KExtendedSocket* YahooConnectionManager::connectionForFD( int fd )
{
	kdDebug(14181) << k_funcinfo << "Looking for socket with fd " << fd << endl;
	QValueList<KExtendedSocket*>::const_iterator it, ycEnd = m_connectionList.constEnd();
	for ( it = m_connectionList.begin(); it != ycEnd; ++it )
	{
		if ( ( *it )->fd() == fd )
		{
			kdDebug(14181) << k_funcinfo << "Found socket" << endl;
			KExtendedSocket* socket = ( *it );
			return socket;
		}
	}
	
	return 0L;
}

void YahooConnectionManager::remove( KExtendedSocket* socket )
{
	QValueList<KExtendedSocket*>::iterator it, ycEnd = m_connectionList.end();
	for ( it = m_connectionList.begin(); it != ycEnd; )
	{
		if ( ( *it ) == socket )
		{
			it = m_connectionList.remove( it );
			socket->reset();
			delete socket;
		}
	}
}

void YahooConnectionManager::reset()
{
	QValueList<KExtendedSocket*>::iterator it = m_connectionList.begin();
	
	while ( it != m_connectionList.end() && m_connectionList.count() > 0 )
	{
		KExtendedSocket* socket = ( *it );
		it = m_connectionList.remove( it );
		socket->reset();
		delete socket;
	}
}


YahooSessionManager::YahooSessionManager()
{
	if ( managerStatic_ )
		kdDebug(14181) << "Yahoo manager already initialized" << endl;
	else
		managerStatic_ = this;
}

YahooSessionManager::~YahooSessionManager()
{
	managerStatic_ = 0L;
}

void YahooSessionManager::setPager( QString host, int port )
{
	strcpy( pager_host, host.utf8() );
	strcpy( pager_port, QString::number( port ).latin1() );
}

void YahooSessionManager::setFileTransfer( QString host, int port )
{
	strcpy( filetransfer_host, host.utf8() );
	strcpy( filetransfer_port, QString::number( port ).latin1() );
}

YahooSession* YahooSessionManager::createSession( const QString username, const QString password )
{
	int id;
	YahooSession *session;

	kdDebug(14181) << k_funcinfo << " Initializing" << endl;
	id = yahoo_init( username.local8Bit(), password.local8Bit() );

	session = new YahooSession(id, username, password);

	kdDebug(14181) << k_funcinfo << " Session created, got id "<< id << " !"<< endl;
	m_sessionsMap[id] = session;

	return session;

}

bool YahooSessionManager::cleanSessions()
{
	QMap< int, YahooSession* >::iterator it;
	for ( it=m_sessionsMap.begin(); it != m_sessionsMap.end(); it++)
	{
		it.data()->logOff();
		delete it.data();
		m_sessionsMap.remove( it.key() );
		kdDebug(14181) << k_funcinfo << " logout " << it.key() << endl;
	}
	return true;
}

YahooSession* YahooSessionManager::session(int id)
{
	return m_sessionsMap[id] ? m_sessionsMap[id] : 0L;
}

YahooSessionManager* YahooSessionManager::managerStatic_ = 0L;

YahooSessionManager* YahooSessionManager::manager()
{
	if ( managerStatic_ )
		return managerStatic_;
	else
		return ( new YahooSessionManager );
}

 /*
    *************************************************************************
    * YahooSession Methods                                                  *
    *************************************************************************
*/

YahooSession::YahooSession(int id, const QString username, const QString password)
{
	kdDebug(14181) << k_funcinfo << endl;
	m_connId = id;
	m_Username = username;
	m_Password = password;
	m_lastWebcamTimestamp = 0;
}

int YahooSession::sessionId() const
{
	return m_connId;
}

void YahooSession::login(int initial)
{
	kdDebug(14181) << k_funcinfo << endl;
	m_Status = initial;

	/* We try to login */
	yahoo_login( m_connId, initial );
}

YahooSession::~YahooSession()
{
	kdDebug(14181) << k_funcinfo << endl;
	yahoo_logoff( m_connId );
	yahoo_close( m_connId );
	m_connManager.reset();
}

int YahooSession::setLogLevel(enum yahoo_log_level level)
{
	kdDebug(14181) << k_funcinfo << endl;
	return yahoo_set_log_level( level );
}

void YahooSession::logOff()
{
	kdDebug(14181)<< k_funcinfo << " " << m_connId <<endl;
	yahoo_logoff( m_connId );

	m_connManager.reset();

}

void YahooSession::refresh()
{
	kdDebug(14181) << k_funcinfo << endl;
	yahoo_refresh( m_connId );
}

void YahooSession::setIdentityStatus( const QString &identity, int active)
{
	kdDebug(14181) << k_funcinfo << endl;
	yahoo_set_identity_status( m_connId, identity.local8Bit(), active );
}

void YahooSession::getList()
{
	kdDebug(14181) << k_funcinfo << endl;
	yahoo_get_list( m_connId );
}

void YahooSession::keepalive()
{
	kdDebug(14181) << k_funcinfo << endl;
	yahoo_keepalive( m_connId );
}

void YahooSession::sendIm( const QString &from, const QString &who, const QString &msg)
{
	kdDebug(14181) << k_funcinfo << endl;
	yahoo_send_im( m_connId, from.local8Bit(), who.local8Bit(), (const char *)msg.utf8(), 1 );
}

void YahooSession::sendTyping( const QString &from, const QString &who, int typ)
{
	kdDebug(14181) << k_funcinfo << endl;
	yahoo_send_typing( m_connId, from.local8Bit(), who.local8Bit(), typ );
}

void YahooSession::setAway( enum yahoo_status state, const QString &msg, int away)
{
	kdDebug(14181)<< k_funcinfo << state << ", " << msg << ", " << away << "]" << m_connId << endl;

	yahoo_set_away( m_connId, state, msg.isNull() ? QCString() : msg.local8Bit(), away );
}

void YahooSession::addBuddy( const QString &who, const QString &group)
{
	kdDebug(14181) << k_funcinfo << endl;
	yahoo_add_buddy( m_connId, who.local8Bit(), group.local8Bit() );
}

void YahooSession::removeBuddy( const QString &who, const QString &group)
{
	kdDebug(14181) << k_funcinfo << endl;
	yahoo_remove_buddy( m_connId, who.local8Bit(), group.local8Bit() );
}

void YahooSession::rejectBuddy( const QString &who, const QString &msg)
{
	yahoo_reject_buddy( m_connId, who.local8Bit(), msg.local8Bit() );
}

void YahooSession::ignoreBuddy( const QString &who, int unignore)
{
	yahoo_ignore_buddy( m_connId, who.local8Bit(), unignore );

}

void YahooSession::changeBuddyGroup( const QString &who, const QString &old_group, const QString &new_group)
{
	yahoo_change_buddy_group( m_connId, who.local8Bit(), old_group.local8Bit(), new_group.local8Bit() );
}

void YahooSession::conferenceInvite( const QString & from, const QStringList &who,
		const QString &room, const QString &msg )
{
	YList *tmplist;
	tmplist = (YList *) malloc(sizeof(YList));

	for ( QStringList::ConstIterator it = who.begin(); it != who.end(); ++it )
	{
		char *member;
		member = strdup( (*it).local8Bit() );
		y_list_append( tmplist, member );
	}

	yahoo_conference_invite( m_connId, from.local8Bit(), tmplist, room.local8Bit(), msg.local8Bit() );

	y_list_free_1( tmplist );
	y_list_free( tmplist );
}

void YahooSession::conferenceAddinvite( const QString & from, const QString &who, const QString &room,
		const QStringList &members, const QString &msg )
{

	YList *tmplist;
	tmplist = (YList *) malloc( sizeof( YList ) );

	for ( QStringList::ConstIterator it = members.begin(); it != members.end(); ++it )
	{

		char *member;
		member = strdup( (*it).local8Bit() );
		y_list_append( tmplist, member );
	}

	yahoo_conference_addinvite( m_connId, from.local8Bit(), who.local8Bit(), room.local8Bit(), tmplist, msg.local8Bit() );

	y_list_free_1( tmplist );
	y_list_free( tmplist );
}

void YahooSession::conferenceDecline( const QString & from, const QStringList &who,
		const QString &room, const QString &msg )
{
	YList *tmplist;
	tmplist = (YList *) malloc( sizeof( YList ) );

	for ( QStringList::ConstIterator it = who.begin(); it != who.end(); ++it )
	{
		char *member;
		member = strdup( (*it).local8Bit() );
		y_list_append( tmplist, member );
	}

	yahoo_conference_decline( m_connId, from.local8Bit(), tmplist, room.local8Bit(), msg.local8Bit() );

	y_list_free_1( tmplist );
	y_list_free( tmplist );
}

void YahooSession::conferenceMessage( const QString & from, const QStringList &who,
		const QString &room, const QString &msg )
{
	YList *tmplist;
	tmplist = (YList *) malloc( sizeof( YList ) );

	for ( QStringList::ConstIterator it = who.begin(); it != who.end(); ++it )
	{
		char *member;
		member = strdup( (*it).local8Bit() );
		y_list_append( tmplist, member );
	}

	yahoo_conference_message( m_connId, from.local8Bit(), tmplist,
		room.local8Bit(), (const char *) msg.utf8(), 1 );

	y_list_free_1( tmplist );
	y_list_free( tmplist );
}

void YahooSession::YahooSession::conferenceLogon( const QString & from, const QStringList &who,
		const QString &room)
{
	YList *tmplist;
	tmplist = (YList *) malloc( sizeof( YList ) );

	for ( QStringList::ConstIterator it = who.begin(); it != who.end(); ++it )
	{
		char *member;
		member = strdup( (*it).local8Bit() );
		y_list_append( tmplist, member );
	}

	yahoo_conference_logon( m_connId, from.local8Bit(), tmplist, room.local8Bit() );

	y_list_free_1( tmplist );
	y_list_free( tmplist );
}

void YahooSession::conferenceLogoff( const QString &from, const QStringList &who,
		const QString &room )
{
	YList *tmplist;
	tmplist = (YList *) malloc(sizeof(YList));

	for ( QStringList::ConstIterator it = who.begin(); it != who.end(); ++it )
	{
		char *member;
		member = strdup( (*it).local8Bit() );
		y_list_append( tmplist, member );
	}

	yahoo_conference_logoff( m_connId, from.local8Bit(), tmplist, room.local8Bit() );

	y_list_free_1( tmplist );
	y_list_free( tmplist );
}

int YahooSession::sendFile( const QString& /*who*/, const QString& /*msg*/,
		const QString& /*name*/, long /*size*/ )
{
	// FIXME 0,0 is the callback and void *data
	// void (*yahoo_get_fd_callback)(int id, int fd, int error, void *data);

	//return yahoo_send_file(m_connId, who.local8Bit(), msg.local8Bit(),
	//				 name.local8Bit(), size, file_send_callback ,0);
	return 0;

}

int YahooSession::getUrlHandle( const QString& /*url*/, const QString& /*filename*/,
		unsigned long* /*filesize*/ )
{
	/*
	FIXME! API CHANGED! add callback and data to the call
	FIXME why is filesize an unsigned long pointer?
	char *_url;
	char *_filename;
	int result;

	_url = strdup(url.local8Bit());
	_filename = strdup(QFile::encodeName(filename));

	result = yahoo_get_url_handle(m_connId, _url, _filename, filesize);

	free(_url);
	free(_filename);

	return result;
	*/

	return 0;
}

enum yahoo_status YahooSession::currentStatus()
{
	return yahoo_current_status( m_connId );
}

const YList *YahooSession::getLegacyBuddyList()
{
	return yahoo_get_buddylist( m_connId );
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
	return QString( yahoo_get_cookie( m_connId, which.latin1() ) );
}

QString YahooSession::getProfile_url( void )
{
	return QString( yahoo_get_profile_url() );
}

void YahooSession::requestWebcam( const QString& from )
{
	yahoo_webcam_get_feed( m_connId, from.latin1() );
}

void YahooSession::closeWebcam( const QString& from )
{
	yahoo_webcam_close_feed( m_connId, from.latin1() );
}

void YahooSession::slotLoginResponseReceiver( int /* succ */, char * /* url */ )
{
	kdDebug(14181)<< k_funcinfo << endl;
}

void YahooSession::getUserInfo( const QString &who )
{
	m_targetID = who;
	m_UserInfo = QString::null;
	//QString url = QString::fromLatin1("http://insider.msg.yahoo.com/ycontent/?filter=timef&ab2=0&intl=us&os=win&%1&%2").arg(getCookie("y")).arg(getCookie("t"));
	QString url = QString::fromLatin1("http://insider.msg.yahoo.com/ycontent/?filter=timef&ab2=0&intl=us&os=win");
	
	mTransferJob = KIO::get( url , false, false );
	mTransferJob->addMetaData("cookies", "manual");
	mTransferJob->addMetaData("setcookies", QString::fromLatin1("Cookie: Y=%1; T=%2").arg(getCookie("y")).arg(getCookie("t")) );
	connect( mTransferJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ), this, SLOT( slotUserInfoData( KIO::Job*, const QByteArray & ) ) );
	connect( mTransferJob, SIGNAL( result( KIO::Job *) ), this, SLOT( slotUserInfoResult( KIO::Job* ) ) );
}

void YahooSession::slotUserInfoResult( KIO::Job* job )
{
	kdDebug(14180) << k_funcinfo << endl;
	if ( job->error () || mTransferJob->isErrorPage () )
		kdDebug(14180) << k_funcinfo << "Could not retrieve server side addressbook for user info." << endl;
	else {
		QDomDocument doc;
		QDomNodeList list;
		QDomElement e;
		QString msg;
		uint it = 0;
		
		kdDebug(14180) << k_funcinfo << "Server side Adressbook successfully retrieved." << endl;
		//kdDebug(14180) << "Adressbook:" << endl << m_UserInfo << endl;
		
		doc.setContent( m_UserInfo );
		list = doc.elementsByTagName( "record" );			// Get records
		
		for( it = 0; it < list.count(); it++ )	{
			if( !list.item( it ).isElement() )
				continue;
			e = list.item( it ).toElement();
			if( e.attribute("userid") != m_targetID )
				continue;
			
			YahooUserInfo info;
			kdDebug(14180) << k_funcinfo << "dbID: " << e.attribute("dbid")  << endl;
			info.userID = e.attribute("userid");
			info.abID = e.attribute("dbid");
			info.firstName = e.attribute( "fname" );
			info.lastName = e.attribute( "lname" );
			info.nickName = e.attribute( "nname" );
			info.email =  e.attribute( "email" );
			info.phoneHome = e.attribute( "hphone" );
			info.phoneWork = e.attribute( "wphone" );
			info.phoneMobile = e.attribute( "mphone" );
			
			YahooUserInfoDialog* theDialog = new YahooUserInfoDialog( Kopete::UI::Global::mainWidget(), "User Information" );
			theDialog->setUserInfo( info );
			theDialog->setSession( this );
			theDialog->show();
			
			return;
		}
		
		// If we get here, no entry was found --> ask to create a new one
		if( KMessageBox::Yes == KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget(), i18n( "Do you want to create a new entry?" ), i18n( "No Yahoo Addressbook Entry Found" )) ){
			YahooUserInfo info;
			info.userID = m_targetID;
			info.abID = "-1";
			
			YahooUserInfoDialog* theDialog = new YahooUserInfoDialog( Kopete::UI::Global::mainWidget(), "User Information" );
			theDialog->setUserInfo( info );
			theDialog->setSession( this );
			theDialog->show();
		} else {
			viewUserProfile( m_targetID );
		}
	}	
}

void YahooSession::slotUserInfoData( KIO::Job* /*job*/, const QByteArray &info  )
{
	kdDebug(14180) << k_funcinfo << endl;	
	m_UserInfo += info;	
}

void YahooSession::saveAdressBookEntry( const YahooUserInfo &entry)
{
	kdDebug(14180) << k_funcinfo << endl;
	QString url; 
	
	if( entry.abID.toInt() > 0 )	{		// This is an Update --> append entry-ID
		url = QString::fromLatin1("http://insider.msg.yahoo.com/ycontent/?addab2=0&ee=1&ow=1&id=%0&fn=%1&ln=%2&yid=%3&nn=%4&e=%5&hp=%6&wp=%7").arg(entry.abID).arg(entry.firstName).arg(entry.lastName).arg(entry.userID).arg(entry.nickName).arg(entry.email).arg(entry.phoneHome).arg(entry.phoneWork);
		//url += QString::fromLatin1("&%1&%2&%3&%4").arg(getCookie("y")).arg(getCookie("t")).arg(getCookie("b")).arg(getCookie("q"));
	} else {
	url = QString::fromLatin1("http://address.yahoo.com/yab/us?A=m&v=PG&ver=2&fn=%0&ln=%1&yid=%2&nn=%3&e=%4&hp=%5&wp=%6").arg(entry.firstName).arg(entry.lastName).arg(entry.userID).arg(entry.nickName).arg(entry.email).arg(entry.phoneHome).arg(entry.phoneWork);
	/*url = QString::fromLatin1("http://insider.msg.yahoo.com/ycontent/?addab2=0&fn=%0&ln=%1&yid=%2&nn=%3&e=%4&hp=%5&wp=%6").arg(entry.firstName).arg(entry.lastName).arg(entry.userID).arg(entry.nickName).arg(entry.email).arg(entry.phoneHome).arg(entry.phoneWork);*/
		//url += QString::fromLatin1("&%1&%2&%3&%4").arg(getCookie("y")).arg(getCookie("t")).arg(getCookie("b")).arg(getCookie("q"));

	}
	kdDebug(14180) << url << endl;
	
	m_UserInfo = QString::null;
	mTransferJob = KIO::get( url , false, false );
	mTransferJob->addMetaData("cookies", "manual");
	mTransferJob->addMetaData("setcookies", QString::fromLatin1("Cookie: Y=%1; T=%2").arg(getCookie("y")).arg(getCookie("t")) );
	connect( mTransferJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ), this, SLOT( slotUserInfoData( KIO::Job*, const QByteArray & ) ) );
	connect( mTransferJob, SIGNAL( result( KIO::Job *) ), this, SLOT( slotUserInfoSaved( KIO::Job* ) ) );
}
void YahooSession::slotUserInfoSaved( KIO::Job* job )
{
	kdDebug(14180) << k_funcinfo << endl << "Return data:" << m_UserInfo << endl;
	
	if ( job->error() || mTransferJob->isErrorPage() || m_UserInfo.find(m_targetID) < 0 )	{
		kdDebug(14180) << "Could not save the adressbook entry." << endl;
		KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n( "An unknown error occured. A possible reason is a invalid email address." ), i18n("Error") );
	} else {
		kdDebug(14180) << "Adressbook entry succesfully saved." << endl;		
	}
}

void YahooSession::viewUserProfile( const QString &who )
{
	kdDebug(14180) << k_funcinfo << endl;
	
	QString profileSiteString = QString::fromLatin1("http://profiles.yahoo.com/") + who;
	KRun::runURL( KURL( profileSiteString ) , "text/html" );	
}

 /*
    *************************************************************************
    * Callback implementation                                               *
    *************************************************************************
*/

extern "C"
{

void YAHOO_CALLBACK_TYPE( ext_yahoo_login_response ) ( int id, int succ, const char *url )
{
	YahooSession *session = YahooSessionManager::manager()->session(id);
	session->_loginResponseReceiver( succ, url );
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_got_buddies ) ( int id, YList * buds )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_gotBuddiesReceiver( buds );
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_got_ignore ) ( int id, YList * igns )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_gotIgnoreReceiver( igns );
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_got_identities ) ( int id, YList * ids )
{
	YahooSession *session = YahooSessionManager::manager()->session(id);
	session->_gotIdentitiesReceiver(ids);
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_got_cookies )( int /*id*/ )
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_status_changed )( int id, char *who, int stat,
		char *msg, int away )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_statusChangedReceiver( who, stat, msg, away );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_got_im )( int id, char *who, char *msg,
		long tm, int stat, int utf8 )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_gotImReceiver( who, msg, tm, stat, utf8 );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_got_conf_invite )( int id, char *who, char *room,
		char *msg, YList *members )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_gotConfInviteReceiver( who, room, msg, members );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_conf_userdecline )( int id, char *who,
		char *room, char *msg )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_confUserDeclineReceiver( who, room, msg );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_conf_userjoin )( int id, char *who, char *room )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_confUserJoinReceiver( who, room );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_conf_userleave )( int id, char *who, char *room )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_confUserLeaveReceiver( who, room );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_conf_message )( int id, char *who, char *room,
		 char *msg, int utf8 )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_confMessageReceiver( who, room, msg, utf8 );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_chat_cat_xml )( int /*id*/, char* /*xml*/ )
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_chat_join )( int /*id*/, char* /*room*/,
		char* /*topic*/, YList* /*members*/ )
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_chat_userjoin )( int /*id*/, char* /*room*/,
		 struct yahoo_chat_member* /*who*/ )
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_chat_userleave )( int /*id*/, char* /*room*/, char* /*who*/ )
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_chat_message )( int /*id*/, char* /*who*/, char* /*room*/,
		char* /*msg*/, int /*msgtype*/, int /*utf8*/ )
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_got_file )( int id, char *who, char *url, long expires,
		char *msg, char *fname, unsigned long fesize )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_gotFileReceiver( who, url, expires, msg, fname, fesize );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_contact_added )( int id, char *myid, char *who, char *msg )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_contactAddedReceiver( myid, who, msg );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_rejected )( int id, char *who, char *msg )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_rejectedReceiver( who, msg );
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_typing_notify )( int id, char *who, int stat )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_typingNotifyReceiver( who, stat );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_game_notify )( int id, char *who, int stat )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_gameNotifyReceiver( who, stat );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_mail_notify )( int id, char *from, char *subj, int cnt )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_mailNotifyReceiver( from, subj, cnt );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_system_message )( int id, char *msg )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_systemMessageReceiver( msg );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_error )( int id, char *err, int fatal )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_errorReceiver( err, fatal );

}

int YAHOO_CALLBACK_TYPE( ext_yahoo_log )( const char* /*fmt*/, ... )
{
	/* Do nothing? */
	return 0;
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_add_handler )( int id, int fd, yahoo_input_condition cond, void * data )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_addHandlerReceiver( fd, cond, data );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_remove_handler )( int id, int fd )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_removeHandlerReceiver( fd );

}

int YAHOO_CALLBACK_TYPE( ext_yahoo_connect )( char *host, int port )
{
	return YahooSessionManager::manager()->_hostConnectReceiver( host, port );
}

int YAHOO_CALLBACK_TYPE( ext_yahoo_connect_async )( int id, char *host, int port,
		yahoo_connect_callback callback, void *callback_data )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	return session->_hostAsyncConnectReceiver( host, port, callback, callback_data );
}


void YAHOO_CALLBACK_TYPE( ext_yahoo_got_webcam_image )( int id, const char* who,
		const unsigned char* image, unsigned int image_size, unsigned int real_size,
		unsigned int timestamp )
{
	YahooSession* session = YahooSessionManager::manager()->session( id );
	return session->_gotWebcamImage( who, image, image_size, real_size, timestamp );
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_webcam_invite )( int /*id*/, char* /*from*/ )
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_webcam_invite_reply )( int /*id*/, char* /*from*/, int /*accept*/ )
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_webcam_closed )( int id, char* who, int reason )
{
	YahooSession* session = YahooSessionManager::manager()->session( id );
	session->_webcamDisconnected( who, reason );
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_webcam_viewer )( int /*id*/, char* /*who*/, int /*connect*/ )
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_webcam_data_request )( int /*id*/, int /*send*/ )
{
	/* Not implemented , No receiver yet */
}

/* End of extern C */
}

/*
    *************************************************************************
    * Private Session Callback Receiver, don't use them                     *
    *************************************************************************
*/

void YahooSession::_loginResponseReceiver( int succ, const char *url )
{

	kdDebug(14181) << k_funcinfo << endl;

	emit loginResponse( succ, QString( url ) );
}

void YahooSession::_gotIgnoreReceiver( YList * igns )
{
	kdDebug(14181) << k_funcinfo << endl;

	YList *l;
	QStringList ign_list;

	for ( l = igns; l; l = l->next )
	{
		struct yahoo_buddy *bud = ( yahoo_buddy* ) l->data;

		if( !bud )
		{
			kdDebug(14181) << k_funcinfo << " Null Id" << endl;
			continue;
		}
		else
		{
			kdDebug(14181) << k_funcinfo << "Got buddy: " << bud->id << endl;
			ign_list.append( QString( bud->id ) );
		}
	}

	emit gotIgnore(ign_list);
}

void YahooSession::_gotBuddiesReceiver( YList * buds )
{
	kdDebug(14181) << k_funcinfo << endl;

	int buddyListCount = 0;
	YList *l;

	for ( l = buds; l; l = l->next )
	{
		struct yahoo_buddy *bud = ( yahoo_buddy* )l->data;

		if( !bud )
		{
			kdDebug(14181) << k_funcinfo << " Null Buddy" << endl;
			continue;
		}
		else
		{
			kdDebug(14181) << k_funcinfo << " " << bud->id << endl;
			emit gotBuddy( QString( bud->id ) , QString::fromLocal8Bit( bud->real_name ),
					QString::fromLocal8Bit( bud->group ) );
			buddyListCount++;
		}
	}

	emit buddyListFetched( buddyListCount );
}

void YahooSession::_gotIdentitiesReceiver( YList *ids )
{
	kdDebug(14181) << k_funcinfo << endl;


	YList *l;
	QStringList idslist;

	for ( l = ids; l; l = l->next )
	{
		char *userid = ( char* ) l->data;

		if ( !userid )
		{
			kdDebug(14181) << k_funcinfo << endl;
			continue;
		}
		else
		{
			kdDebug(14181) << k_funcinfo << userid << endl;
			idslist.append( QString( userid ) );
		}
	}

	emit gotIdentities( idslist );
}

void YahooSession::_statusChangedReceiver( char *who, int stat, char *msg, int away )
{
//	kdDebug(14181) << k_funcinfo << endl;

	emit statusChanged( QString::fromLocal8Bit( who ), stat, QString::fromLocal8Bit( msg ), away );
}

void YahooSession::_gotImReceiver( char *who, char *msg, long tm, int stat, int utf8 )
{
	kdDebug(14181) << k_funcinfo << endl;

	QString convertedMessage;

	if ( utf8 )
		convertedMessage = QString::fromUtf8( msg );
	else
		convertedMessage = QString::fromLocal8Bit( msg );

	kdDebug(14181)<<"got IM"<<endl;
	emit gotIm( QString::fromLocal8Bit( who ), convertedMessage, tm, stat );
}

void YahooSession::_gotConfInviteReceiver( char *who, char *room, char *msg, YList *members )
{
	kdDebug(14181) << k_funcinfo << endl;

	YList *l;
	QStringList member_list;

	for ( l = members; l; l = l->next )
	{
		char *buddy = ( char* ) l->data;
		if ( !buddy )
		{
			kdDebug(14181) << k_funcinfo << " Null Id" << endl;
			continue;
		}
		else
		{
			member_list.append( QString::fromLocal8Bit( buddy ) );
		}
	}

	emit gotConfInvite( QString::fromLocal8Bit( who ), QString::fromLocal8Bit( room ),
		 QString::fromLocal8Bit( msg ), member_list) ;
}

void YahooSession::_confUserDeclineReceiver( char *who, char *room, char *msg )
{
	kdDebug(14181) <<  k_funcinfo << endl;
	emit confUserDecline( QString::fromLocal8Bit(who), QString::fromLocal8Bit( room ),
			QString::fromLocal8Bit(msg));
}

void YahooSession::_confUserJoinReceiver( char *who, char *room )
{

	emit confUserJoin( QString::fromLocal8Bit( who ), QString::fromLocal8Bit( room ) );
}

void YahooSession::_confUserLeaveReceiver( char *who, char *room )
{

	emit confUserLeave( QString::fromLocal8Bit( who ), QString::fromLocal8Bit( room ) );
}

void YahooSession::_confMessageReceiver( char *who, char *room, char *msg, int utf8 )
{
	QString convertedMessage;

	if ( utf8 )
		convertedMessage = QString::fromUtf8( msg );
	else
		convertedMessage = QString::fromLocal8Bit( msg );

	emit confMessage( QString::fromLocal8Bit( who ), QString::fromLocal8Bit( room ), convertedMessage);
}

void YahooSession::_gotFileReceiver( char *who, char *url, long expires, char *msg,
		char *fname, unsigned long fesize )
{

	emit gotFile( QString::fromLocal8Bit( who ), QString::fromLocal8Bit( url ),
			expires, QString::fromLocal8Bit( msg ), QString::fromLocal8Bit( fname ), fesize );
}

void YahooSession::_contactAddedReceiver( char *myid, char *who, char *msg )
{

	emit contactAdded( QString::fromLocal8Bit( myid ), QString::fromLocal8Bit( who ),
			QString::fromLocal8Bit( msg ) );
}

void YahooSession::_rejectedReceiver( char *who, char *msg )
{

	emit rejected( QString::fromLocal8Bit( who ), QString::fromLocal8Bit( msg ) );
}

void YahooSession::_typingNotifyReceiver( char *who, int stat )
{

	emit typingNotify( QString::fromLocal8Bit( who ), stat );
}

void YahooSession::_gameNotifyReceiver( char *who, int stat )
{
	emit gameNotify( QString::fromLocal8Bit( who ), stat );
}

void YahooSession::_mailNotifyReceiver( char *from, char *subj, int cnt )
{
	//kdDebug(14181) << k_funcinfo << " session: " <<  endl;

	emit mailNotify( QString::fromLocal8Bit( from ), QString::fromLocal8Bit( subj ), cnt);
}

void YahooSession::_systemMessageReceiver( char *msg )
{
	kdDebug(14181) << k_funcinfo << " session: " << endl;

	emit systemMessage( QString::fromLocal8Bit( msg ) );
}

void YahooSession::_errorReceiver( char *err, int fatal )
{
	kdDebug(14181) << k_funcinfo << " session: " << m_connId <<  endl;

	emit error( err, fatal );
}

int YahooSession::_logReceiver( char */*fmt*/, ... )
{
	kdDebug(14181) << k_funcinfo << endl;
	return 0;
}

void YahooSession::_addHandlerReceiver( int fd, yahoo_input_condition cond, void *data )
{
	kdDebug(14181) << k_funcinfo << " " << m_connId << " Socket: " << fd << endl;

	m_data = data;
	if ( fd == -1 )
		return;
	
	KExtendedSocket* socket = m_connManager.connectionForFD( fd );
	if ( !socket )
	{
		kdDebug(14181) << k_funcinfo << "No existing socket for connection found. We're screwed"
			<< endl;
		return;
	}
	
	if ( cond == YAHOO_INPUT_READ )
	{
		kdDebug(14181) << k_funcinfo << " add handler read" << endl;
		socket->enableRead( true );
		connect ( socket, SIGNAL( readyRead() ), this, SLOT( slotReadReady() ) );
	}
	else if ( cond == YAHOO_INPUT_WRITE )
	{
		kdDebug(14181) << k_funcinfo << " add handler write" << endl;
		socket->enableWrite( true );
		connect ( socket, SIGNAL( readyWrite() ), this, SLOT( slotWriteReady() ) );
	}

}

void YahooSession::addHandler( int /*fd*/, yahoo_input_condition /*cond*/ )
{
}

void YahooSession::_removeHandlerReceiver( int fd )
{
	if ( fd == -1 )
		return;
	
	KExtendedSocket* socket = m_connManager.connectionForFD( fd );
	if ( !socket )
	{
		kdDebug(14181) << k_funcinfo << "No existing socket for connection found. We're screwed"
			<< endl;
		return;
	}
	
	kdDebug(14181) << k_funcinfo << " read off" << endl;
	socket->enableRead( false );
	disconnect ( socket, SIGNAL( readyRead() ), this, SLOT( slotReadReady() ) );
	
	kdDebug(14181) << k_funcinfo << " write off" << endl;
	socket->enableRead( false );
	disconnect ( socket, SIGNAL( readyWrite() ), this, SLOT( slotWriteReady() ) );
}

void YahooSession::removeHandler( int /*fd*/ )
{
}

int YahooSessionManager::_hostConnectReceiver( char* /*host*/, int /*port*/ )
{
	kdDebug(14181) << k_funcinfo << endl;
	return 0;
}

int YahooSession::_hostAsyncConnectReceiver( char *host, int port,
		yahoo_connect_callback callback, void *callback_data )
{
	struct connect_callback_data *ccd;
	int error;
	KExtendedSocket* yahooSocket = new KExtendedSocket( host, port );

	// TODO Do an async connect in the future
	error = yahooSocket->connect();

	if ( !error )
	{
		kdDebug(14181) << k_funcinfo << " Connected! fd "<< yahooSocket->fd() << endl;
		m_connManager.addConnection( yahooSocket );
		callback( yahooSocket->fd(), 0, callback_data );
		return 0;
	}
	else if( error == -1 && errno == EINPROGRESS )
	{
		kdDebug(14181) << k_funcinfo << " In progress?" << endl;
		m_connManager.addConnection( yahooSocket );
		ccd = ( struct connect_callback_data* ) calloc( 1, sizeof( struct connect_callback_data ) );
		ccd->callback = callback;
		ccd->callback_data = callback_data;
		ccd->id = m_connId;
		ext_yahoo_add_handler( -1, yahooSocket->fd(), YAHOO_INPUT_WRITE, ccd );
		return 1;
	}
	else
	{
		kdDebug(14181) << k_funcinfo << " Failed!" << endl;
		yahooSocket->close();
		delete yahooSocket;
		yahooSocket = 0L;
		_errorReceiver(0, 1);
		return -1;
	}
}

void YahooSession::_gotWebcamImage( const char* who, const unsigned char* image,
                                    unsigned int image_size, unsigned int real_size,
                                    unsigned int timestamp )
{
	if ( real_size != image_size )
		return; //not enough to make a full image
	
	if ( timestamp > m_lastWebcamTimestamp )
	{
		kdDebug(14181) << k_funcinfo << "Processing webcam image from " << who << endl;
		//this is the next image in the sequence. If we get images out of
		//sequence, they're just dropped.
		m_lastWebcamTimestamp = timestamp;
		QPixmap webcamImage;
		webcamImage.loadFromData( image, real_size );
		emit webcamImageReceived( webcamImage, QString::fromLatin1( who ) );
	}
}

void YahooSession::_webcamDisconnected( const char* who, int reason )
{
	kdDebug(14181) << k_funcinfo << "Webcam closed remotely, reason: " << reason << endl;
	emit webcamClosed( QString::fromLocal8Bit( who ), reason );
}


void YahooSession::slotReadReady()
{
	int ret = 1;
	
	//using sender is the only way to reliably get the socket 
	const KExtendedSocket* socket = dynamic_cast<const KExtendedSocket*>( sender() );
	if ( !socket )
	{
		kdDebug(14181) << k_funcinfo << "sender() was not a KExtendedSocket!" << endl;
		return;
	}
	
	int fd = socket->fd();
	//kdDebug(14181) << k_funcinfo << "Socket FD: " << fd << endl;
	ret = yahoo_read_ready( m_connId , fd, m_data );

	if ( ret == -1 )
		kdDebug(14181) << k_funcinfo << "Read Error (" << errno << ": " << strerror(errno) << endl;
	else if ( ret == 0 )
		kdDebug(14181) << k_funcinfo << "Server closed socket" << endl;

}

void YahooSession::slotWriteReady()
{
	int ret = 1;
	
	//using sender is the only way to reliably get the socket 
	const KExtendedSocket* socket = dynamic_cast<const KExtendedSocket*>( sender() );
	if ( !socket )
	{
		kdDebug(14181) << k_funcinfo << "sender() was not a KExtendedSocket!" << endl;
		return;
	}
	
	int fd = socket->fd();
	kdDebug(14181) << k_funcinfo << "Socket FD: " << fd << endl;

	ret = yahoo_write_ready( m_connId , fd, m_data );

	if ( ret == -1 )
		kdDebug(14181) << k_funcinfo << "Read Error (" << errno << ": " << strerror(errno) << endl;
	else if ( ret == 0 )
		kdDebug(14181) << k_funcinfo << "Server closed socket" << endl;
}

#include "kyahoo.moc"

// vim: set noet ts=4 sts=4 sw=4:
// kate: indent-mode csands; tab-width 4;

