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
#include "kopetetransfermanager.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "yahoouserinfo.h"
#include "yahoobuddyiconloader.h"

// QT Includes
#include <qfile.h>
#include <qtimer.h>
#include <qdom.h>
#include <qtextstream.h>

// KDE Includes
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <krun.h>
#include <kurl.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
//#include <kimageio.h>
#include <kprocess.h>
#include <ktempfile.h>
#include <kstandarddirs.h>

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

extern "C" {
	void receive_file_callback( int id, int fd, int error,
	                            const char *filename, unsigned long size, void *data );
	void upload_file_callback( int id, int fd, int error, void *data );
}

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

void YahooConnectionManager::addConnection( KStreamSocket* socket )
{
	kdDebug(14181) << k_funcinfo << "Adding socket with fd " << socket->socketDevice()->socket() << endl;
	
	m_connectionList.append( socket );
}

KStreamSocket* YahooConnectionManager::connectionForFD( int fd )
{
	//kdDebug(14181) << k_funcinfo << "Looking for socket with fd " << fd << endl;
	QValueList<KStreamSocket*>::const_iterator it, ycEnd = m_connectionList.constEnd();
	KSocketDevice *dev;
	
	for ( it = m_connectionList.begin(); it != ycEnd; ++it )
	{
		dev = ( *it )->socketDevice();
		if ( dev->socket() == fd )
		{
			kdDebug(14181) << k_funcinfo << "Found socket" << endl;
			KStreamSocket* socket = ( *it );
			return socket;
		}
	}
	
	return 0L;
}

void YahooConnectionManager::remove( KStreamSocket* socket )
{
	QValueList<KStreamSocket*>::iterator it, ycEnd = m_connectionList.end();
	
	for ( it = m_connectionList.begin(); it != ycEnd; it++ )
	{
		if ( ( *it ) == socket )
		{
			socket->reset();
			m_connectionList.remove( it );
			delete socket;
			return;
		}
	}
}

void YahooConnectionManager::reset()
{
	QValueList<KStreamSocket*>::iterator it, ycEnd = m_connectionList.end();
	
	for ( it = m_connectionList.begin(); it != ycEnd; it++ )
	{
		KStreamSocket *socket = ( *it );
		socket->reset();
		it = m_connectionList.remove( it );
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
	id = yahoo_init_with_attributes( username.local8Bit(), password.local8Bit(), "pager_host", pager_host, "pager_port", QString(pager_port).toInt(), 0L );

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
	currentImage = 0L;
	m_iconLoader = new YahooBuddyIconLoader();

	connect( m_iconLoader, SIGNAL(fetchedBuddyIcon(const QString&, KTempFile*, int )), this, SLOT(slotBuddyIconFetched(const QString&, KTempFile*,  int ) ) );
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
	delete m_iconLoader;
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

void YahooSession::sendIm( const QString &from, const QString &who, const QString &msg, int picture )
{
	kdDebug(14181) << k_funcinfo << " Picture: " << picture <<  endl;
	yahoo_send_im( m_connId, from.local8Bit(), who.local8Bit(), (const char *)msg.utf8(), 1, picture );
}

void YahooSession::sendTyping( const QString &from, const QString &who, int typ)
{
	kdDebug(14181) << k_funcinfo << endl;
	yahoo_send_typing( m_connId, from.local8Bit(), who.local8Bit(), typ );
}

void YahooSession::buzzContact( const QString &from, const QString &who, int pictureFlag )
{
	kdDebug(14181) << k_funcinfo << endl;
	yahoo_send_im( m_connId, from.local8Bit(), who.local8Bit(), "<ding>", 1, pictureFlag );
}

void YahooSession::setAway( enum yahoo_status state, const QString &msg, int away)
{
	kdDebug(14181)<< k_funcinfo << state << ", " << msg << ", " << away << "]" << m_connId << endl;

	yahoo_set_away( m_connId, state, msg.isNull() ? QCString() : msg.local8Bit(), away );
}

void YahooSession::addBuddy( const QString &who, const QString &group)
{
	kdDebug(14181) << k_funcinfo << endl;
	yahoo_add_buddy( m_connId, who.local8Bit(), group.local8Bit(), "Please add me" );
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

void YahooSession::requestBuddyIcon( const QString &who )
{
	kdDebug(14181) << k_funcinfo << "Requesting avatar for: " << who << endl;
	yahoo_buddyicon_request( m_connId, who.local8Bit() );
}

void YahooSession::downloadBuddyIcon( const QString &who, KURL url, int checksum )
{
	m_iconLoader->fetchBuddyIcon( QString(who), KURL(url), checksum );
}

void YahooSession::sendBuddyIconChecksum( int checksum, const QString &who )
{
	kdDebug(14181) << k_funcinfo << checksum << " sent to " << who << endl;
	if ( who.isEmpty() )
		yahoo_send_picture_checksum( m_connId, 0, checksum );
	else
		yahoo_send_picture_checksum( m_connId, who.local8Bit(), checksum );
}

void YahooSession::sendBuddyIconInfo( const QString &who, const QString &url, int checksum )
{
	kdDebug(14180) << k_funcinfo << "Url: " << url << " checksum: " << checksum << endl;
	yahoo_send_picture_info( m_connId, who.local8Bit(), url.local8Bit(), checksum );
}

void YahooSession::sendBuddyIconUpdate( const QString &who, int type )
{
	kdDebug(14181) << k_funcinfo << endl;
	yahoo_send_picture_update( m_connId, who.local8Bit(), type );
}

void YahooSession::uploadBuddyIcon( const QString &url, int size )
{
	kdDebug(14181) << k_funcinfo << endl;
	YahooUploadData *uploadData = new YahooUploadData();
	uploadData->size = size;
	uploadData->transmitted = 0;
	uploadData->file.setName( url );
	
	yahoo_send_picture( m_connId, url.local8Bit(), size, upload_file_callback, reinterpret_cast< void*>( uploadData ) );
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

int YahooSession::sendFile( const QString& who, const QString& msg,
               const QString& name, long size )
{	
	kdDebug(14181) << k_funcinfo << "Offering file " << name << " (" << size << ") to " << who << endl;
	YahooUploadData *upload = new YahooUploadData();
	upload->size = size;
	upload->transmitted = 0;
	upload->file.setName( name );
	
	yahoo_send_file( m_connId, who.local8Bit(), msg.local8Bit(), name.local8Bit(), size, upload_file_callback, upload );
	
	return 0;
}


int YahooSession::getUrlHandle( Kopete::Transfer *trans )
{
	char *_url;

	m_kopeteTransfer = trans;
	_url = strdup( trans->info().internalId().local8Bit() );
	m_Filename = strdup( QFile::encodeName(trans->destinationURL().path()) );

	yahoo_get_url_handle(m_connId, _url, receive_file_callback, 0);

	free(_url);
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

void YahooSession::stealthContact( const QString &who, int unstealth )
{
	kdDebug(14181)<< k_funcinfo << "Unstealth: " << unstealth << endl;
	yahoo_stealth_buddy( m_connId, who.local8Bit(), unstealth );
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
    	if( KMessageBox::Yes == KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget(), i18n( "Do you want to create a new entry?" ), i18n( "No Yahoo Addressbook Entry Found" ), i18n("Create Entry"), i18n("Do Not Create")) ){
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
		url = QString("http://insider.msg.yahoo.com/ycontent/?addab2=0&ee=1&ow=1&id=%0&fn=%1&ln=%2&yid=%3&nn=%4&e=%5&hp=%6&wp=%7")
			.arg(entry.abID).arg(entry.firstName).arg(entry.lastName).arg(entry.userID).
			arg(entry.nickName).arg(entry.email).arg(entry.phoneHome).arg(entry.phoneWork);
		//url += QString::fromLatin1("&%1&%2&%3&%4").arg(getCookie("y")).arg(getCookie("t")).arg(getCookie("b")).arg(getCookie("q"));
	} else {
		url = QString("http://address.yahoo.com/yab/us?A=m&v=PG&ver=2&fn=%0&ln=%1&yid=%2&nn=%3&e=%4&hp=%5&wp=%6")
			.arg(entry.firstName).arg(entry.lastName).arg(entry.userID).arg(entry.nickName)
			.arg(entry.email).arg(entry.phoneHome).arg(entry.phoneWork);
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
		KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n( "An unknown error occurred. A possible reason is a invalid email address." ), i18n("Error") );
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

void YAHOO_CALLBACK_TYPE(ext_yahoo_got_ping)(int /*id*/, const char */*errormsg*/)
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_status_changed )( int id, const char *who, int stat,
	const char *msg, int away, int /*idle*/, int /*mobile*/ )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_statusChangedReceiver( (char*)who, stat, (char*)msg, away );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_got_im )( int id, const char */*me*/, const char *who, 		const char *msg, long tm, int stat, int utf8 )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_gotImReceiver( (char*)who, (char*)msg, tm, stat, utf8 );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_got_conf_invite )(int id, const char */*me*/, const char *who, const char *room, const char *msg, YList *members)
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_gotConfInviteReceiver( (char*)who, (char*)room, (char*)msg, members );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_conf_userdecline )(int id, const char */*me*/, const char *who, const char *room, const char *msg)
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_confUserDeclineReceiver( (char*)who, (char*)room, (char*)msg );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_conf_userjoin )(int id, const char */*me*/, const char *who, const char *room)
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_confUserJoinReceiver( (char*)who, (char*)room );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_conf_userleave )(int id, const char */*me*/, const char *who, const char *room)
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_confUserLeaveReceiver( (char*)who, (char*)room );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_conf_message )(int id, const char */*me*/, const char *who, const char *room, const char *msg, int utf8)
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_confMessageReceiver( (char*)who, (char*)room, (char*)msg, utf8 );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_chat_cat_xml )( int /*id*/, const char* /*xml*/ )
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_chat_join )( int /*id*/, const char */*me*/, const char */*room*/, const char */*topic*/, YList */*members*/, int /*fd*/ )
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_chat_userjoin )( int /*id*/, const char */*me*/, const char */*room*/, struct yahoo_chat_member */*who*/ )
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_chat_userleave )( int /*id*/, const char */*me*/, const char */*room*/, const char */*who*/ )
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_chat_message )( int /*id*/, const char */*me*/, const char */*who*/, const char */*room*/, const char */*msg*/, int /*msgtype*/, int /*utf8*/)
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_chat_yahoologout)(int /*id*/, const char */*me*/)
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_chat_yahooerror)(int /*id*/, const char */*me*/)
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_got_file )( int id, const char */*me*/, const char *who, const char *url, long expires, const char *msg, const char *fname, unsigned long fesize)
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_gotFileReceiver( (char*)who, (char*)url, expires, (char*)msg, (char*)fname, fesize );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_contact_added )( int id, const char *myid, const char *who, const char *msg )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_contactAddedReceiver( (char*)myid, (char*)who, (char*)msg );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_rejected )( int id, const char *who, const char *msg )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_rejectedReceiver( (char*)who, (char*)msg );
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_typing_notify )( int id, const char */*me*/, const char *who, int stat )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_typingNotifyReceiver( (char*)who, stat );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_game_notify )(int id, const char */*me*/, const char *who, int stat)
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_gameNotifyReceiver( (char*)who, stat );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_mail_notify )( int id, const char *from, const char *subj, int cnt )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_mailNotifyReceiver( (char*)from, (char*)subj, cnt );

}

void YAHOO_CALLBACK_TYPE( ext_yahoo_system_message )( int id, const char *msg )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_systemMessageReceiver( (char*)msg );

}

	void YAHOO_CALLBACK_TYPE( ext_yahoo_error )( int id, const char *err, int fatal, int /*num*/ )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_errorReceiver( (char*)err, fatal );

}

int YAHOO_CALLBACK_TYPE( ext_yahoo_log )( const char* /*fmt*/, ... )
{
	/* Do nothing? */
	return 0;
}

int YAHOO_CALLBACK_TYPE( ext_yahoo_add_handler )( int id, int fd, yahoo_input_condition cond, void * data )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	return session->_addHandlerReceiver( fd, cond, data );
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_remove_handler )( int id, int fd )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_removeHandlerReceiver( fd );

}

int YAHOO_CALLBACK_TYPE( ext_yahoo_connect )( const char *host, int port )
{
	return YahooSessionManager::manager()->_hostConnectReceiver( (char*)host, port );
}

int YAHOO_CALLBACK_TYPE( ext_yahoo_connect_async )( int id, const char *host, int port,
		yahoo_connect_callback callback, void *callback_data )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	return session->_hostAsyncConnectReceiver( (char*)host, port, callback, callback_data );
}


void YAHOO_CALLBACK_TYPE( ext_yahoo_got_webcam_image )( int id, const char* who,
		const unsigned char* image, unsigned int image_size, unsigned int real_size,
		unsigned int timestamp )
{
	YahooSession* session = YahooSessionManager::manager()->session( id );
	return session->_gotWebcamImage( (char*)who, (unsigned char*)image, image_size, real_size, timestamp );
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_webcam_invite )( int id, const char */*me*/, const char* from )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	session->_gotWebcamInvite( (char*)from );
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_webcam_invite_reply )( int /*id*/, const char */*me*/, const char* /*from*/, int /*accept*/ )
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_webcam_closed )( int id, const char* who, int reason )
{
	YahooSession* session = YahooSessionManager::manager()->session( id );
	session->_webcamDisconnected( (char*)who, reason );
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_got_search_result)(int /*id*/, int /*found*/, int /*start*/, int /*total*/, YList */*contacts*/)
{
	
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_webcam_viewer )( int /*id*/, const char* /*who*/, int /*connect*/ )
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE( ext_yahoo_webcam_data_request )( int /*id*/, int /*send*/ )
{
	/* Not implemented , No receiver yet */
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_got_buddyicon)(int id, const char */*me*/, const char *who, const char *url, int checksum)
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	if ( session )
		session->_gotBuddyIconReceiver( id, (char*)who, (char*)url, checksum );
}

void YAHOO_CALLBACK_TYPE(ext_yahoo_got_buddyicon_checksum)(int id, const char */*me*/, const char *who, int checksum)
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	if( session )
		session->_gotBuddyIconChecksumReceiver( id, (char*)who, checksum );
}
	
void YAHOO_CALLBACK_TYPE(ext_yahoo_got_buddyiconrequest)(int id, const char */*me*/, const char *who)
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	if ( session )
		session->_gotBuddyIconRequestReceiver( id, (char*)who );
}
	
void YAHOO_CALLBACK_TYPE(ext_yahoo_buddyicon_uploaded)(int id, const char *url)
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	if ( session )
		session->_gotBuddyIconUploadResponseReceiver( id, (char*)url );
}
	
void receive_file_callback( int id, int fd, int error,
	                            const char *filename, unsigned long size, void *data )
{
	YahooSession *session = YahooSessionManager::manager()->session( id );
	if ( session )
		session->_receiveFileProceed( id, fd, error, (char*)filename, size, (char*)data );
}

void upload_file_callback( int id, int fd, int error, void *data )
{
	
	YahooSession *session = YahooSessionManager::manager()->session( id );
	if ( session )
		session->_uploadFileReceiver( id, fd, error, data );
}
/* End of extern C */
}


/*
    *************************************************************************
    * Private Session Callback Receiver, don't use them                     *
    *************************************************************************
*/

void YahooSession::_uploadFileReceiver( int /*id*/, int fd, int error, void *data )
{
	YahooUploadData *uploadData = reinterpret_cast< YahooUploadData *>( data );
	kdDebug(14181) << k_funcinfo << "Url: " << uploadData->file.name() << " Size: " << uploadData->size << endl;

	if ( error )
	{
		kdDebug(14180) << "Could not upload file " << uploadData->file.name() << ". Error: " << error << endl;
		KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n( "An unknown error occurred when trying to upload the file."
			" The file was not transferred." ), i18n("Error") );
		return;
	}
	
	if ( !uploadData->file.open(IO_ReadOnly) )
	{
		kdDebug(14180) << "Could not open local file." << endl;
		KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n( "Could not open local file!" ), i18n("Error") );
		return;
	}
	
	slotTransmitFile( fd, uploadData );
}

void YahooSession::slotTransmitFile( int fd, YahooUploadData *uploadData )
{
	KStreamSocket* socket = m_connManager.connectionForFD( fd );
	if( !socket )
		return;
	
	if( uploadData->transmitted >= uploadData->file.size() )
	{
		kdDebug(14181) << k_funcinfo << "File successfully uploaded." << endl;
		uploadData->file.close();
		delete uploadData;
		m_connManager.remove( socket );
		return;
	}
	
	
	uint written;
	uint read;
	char buf[2048];
	
	socket->setBlocking( true );
	
	read = uploadData->file.readBlock( buf, 2048 );
	
	written = socket->writeBlock( buf, read );
	
	uploadData->transmitted += written;
	
	if( written != read )
	{
		kdDebug(14181) << k_funcinfo << "An error occured while sending the file: " << socket->error() << " transmitted: " << uploadData->transmitted << endl;
		uploadData->file.close();
		delete uploadData;
		m_connManager.remove( socket );
	}
	else
		slotTransmitFile( fd, uploadData );
}

void YahooSession::_gotBuddyIconUploadResponseReceiver( int /*id*/, const char *url)
{
	kdDebug(14181) << k_funcinfo << endl;
	emit buddyIconUploaded( QString( url ) );
}

void YahooSession::_gotBuddyIconReceiver( int /*id*/, char *who, char *url, int checksum )
{
	kdDebug(14181) << k_funcinfo << "BuddyIcon reveived from: " << who << " checksum: " << checksum <<endl;
	kdDebug(14181) << k_funcinfo << "BuddyIcon-Url: " << url <<endl;

	emit gotBuddyIconInfo( QString( who ), KURL( url ), checksum );
}

void YahooSession::_gotBuddyIconChecksumReceiver( int /*id*/, char *who, int checksum )
{
	kdDebug(14181) << k_funcinfo << "checksum: " << checksum << " received from: " << who << endl;

	emit gotBuddyIconChecksum( QString( who ), checksum );
	
}

void YahooSession::_gotBuddyIconRequestReceiver( int /*id*/, char *who )
{
	kdDebug(14181) << k_funcinfo << "Got Buddy Icon Request from: " << who << endl;

	emit gotBuddyIconRequest ( QString( who ) );
}

void YahooSession::slotBuddyIconFetched(const QString &who, KTempFile *file, int checksum)
{
	emit gotBuddyIcon( who, file, checksum );
}

void YahooSession::_receiveFileProceed( int id, int fd, int error,
                                        const char */*filename*/, unsigned long /*size*/, void */*data*/ )
{
	kdDebug(14181) << k_funcinfo << "FD:" << fd << " Filename:" << m_Filename <<endl;
	int read = 0, received = 0;
	char buf[1024];

	if ( error )
	{
		kdDebug(14180) << "Could not download file. Error: " << error << endl;
		KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n( "An error occurred when trying to download the file." ), i18n("Error") );
		return;
	}
	
	KStreamSocket* socket = m_connManager.connectionForFD( fd );
	if ( !socket )
	{
		kdDebug(14181) << k_funcinfo << "No existing socket for connection found. We're screwed" << endl;
		return;
	}
	
	QFile file( m_Filename );
	if ( file.open(IO_WriteOnly ) )
	{
		QTextStream stream( &file );
		while( (read = socket->readBlock( buf, 1024 )) > 0 )
		{
			stream << buf;
			received += read;
			m_kopeteTransfer->slotProcessed( received );
		}
		m_kopeteTransfer->slotComplete();
		file.close();
	} else	
		m_kopeteTransfer->slotError( KIO::ERR_CANNOT_OPEN_FOR_WRITING, i18n("Cannot open file %1 for writing.\n%2")
									.arg( m_Filename, file.errorString() ) );
	
	ext_yahoo_remove_handler( id, fd );
	return;
}

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

	if ( convertedMessage == "<ding>" ) {
		kdDebug(14181)<<"got BUZZ"<<endl;
		emit gotBuzz( QString::fromLocal8Bit( who ), tm );
	}
	else {
		kdDebug(14181)<<"got IM"<<endl;
		emit gotIm( QString::fromLocal8Bit( who ), convertedMessage, tm, stat );
	}
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

int YahooSession::_addHandlerReceiver( int fd, yahoo_input_condition cond, void *data )
{
	kdDebug(14181) << k_funcinfo << " " << m_connId << " Socket: " << fd << endl;

	m_data = data;
	if ( fd == -1 )
	{
		kdDebug(14181) << k_funcinfo << "why is fd -1?" << endl;
		return -1;
	}
	
	KStreamSocket* socket = m_connManager.connectionForFD( fd );
	if ( !socket )
	{
		kdDebug(14181) << k_funcinfo << "No existing socket for connection found. We're screwed"
			<< endl;
		return -1;
	}
	
	/* This works ONLY IF (YAHOO_INPUT_READ==1 && YAHOO_INPUT_WRITE==2) */
	int tag = 0;
	if ( cond == YAHOO_INPUT_READ )
	{
		kdDebug(14181) << k_funcinfo << " add handler read" << endl;
		socket->enableRead( true );
		connect ( socket, SIGNAL( readyRead() ), this, SLOT( slotReadReady() ) );
		tag = 2*fd + YAHOO_INPUT_READ;
	}
	else if ( cond == YAHOO_INPUT_WRITE )
	{
		kdDebug(14181) << k_funcinfo << " add handler write" << endl;
		socket->enableWrite( true );
		connect ( socket, SIGNAL( readyWrite() ), this, SLOT( slotWriteReady() ) );
		tag = 2*fd + YAHOO_INPUT_WRITE;
	}
	return tag;
}

void YahooSession::addHandler( int /*fd*/, yahoo_input_condition /*cond*/ )
{
}

void YahooSession::_removeHandlerReceiver( int tag )
{
	kdDebug(14181) << k_funcinfo << " " << m_connId << " tag: " << tag << endl;

	if ( tag == 0 )
		return;

	KStreamSocket* socket = m_connManager.connectionForFD( (tag-1)/2 );
	if ( !socket )
	{
		kdDebug(14181) << k_funcinfo << "No existing socket for connection found. We're screwed"
			<< endl;
		return;
	}
	/* This works ONLY IF (YAHOO_INPUT_READ==1 && YAHOO_INPUT_WRITE==2) */
	if( tag % 2 == YAHOO_INPUT_READ ) {
		kdDebug(14181) << k_funcinfo << " read off" << endl;
		socket->enableRead( false );
		disconnect ( socket, SIGNAL( readyRead() ), this, SLOT( slotReadReady() ) );
	}
	else 
	{
		kdDebug(14181) << k_funcinfo << " write off" << endl;
		socket->enableWrite( false );
		disconnect ( socket, SIGNAL( readyWrite() ), this, SLOT( slotWriteReady() ) );
	}
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
	kdDebug(14181) << k_funcinfo << "Establishing connection to " << host << " on port " << port << endl;
	KStreamSocket* yahooSocket = new KStreamSocket( host, QString::number( port ) );

	m_ccd = ( struct connect_callback_data* ) calloc( 1, sizeof( struct connect_callback_data ) );
	m_ccd->callback = callback;
	m_ccd->callback_data = callback_data;
	m_ccd->id = m_connId;
	
	connect( yahooSocket, SIGNAL( connected( const KResolverEntry& ) ), this, SLOT( slotAsyncConnectSucceeded() ) );
	connect( yahooSocket, SIGNAL( gotError(int) ), this, SLOT( slotAsyncConnectFailed(int) ) );
	
	yahooSocket->connect();
	
	return 0;
}

void YahooSession::slotAsyncConnectSucceeded()
{
	KStreamSocket* socket = const_cast<KStreamSocket*>( dynamic_cast<const KStreamSocket*>( sender() ) );
	kdDebug(14181) << k_funcinfo << " Connected! fd "<< socket->socketDevice()->socket() << endl;
	m_connManager.addConnection( socket );
	
	disconnect( socket, SIGNAL( connected( const KResolverEntry& ) ), this, SLOT( slotAsyncConnectSucceeded() ) );
	disconnect( socket, SIGNAL( gotError(int) ), this, SLOT( slotAsyncConnectFailed(int) ) );
	
	m_ccd->callback( socket->socketDevice()->socket(), 0, m_ccd->callback_data );
}

void YahooSession::slotAsyncConnectFailed( int error)
{
	KStreamSocket* socket = const_cast<KStreamSocket*>( dynamic_cast<const KStreamSocket*>( sender() ) );
	kdDebug(14181) << k_funcinfo << " Failed with error " << error << endl;
	socket->close();
	delete socket;
	//_errorReceiver(0, 1);
	_errorReceiver(0, 0);
}

void YahooSession::_gotWebcamInvite( const char* who )
{
	emit gotWebcamInvite( QString::fromLocal8Bit( who ) );
}

void YahooSession::_gotWebcamImage( const char* who, const unsigned char* image,
                                    unsigned int image_size, unsigned int real_size,
                                    unsigned int timestamp )
{
	m_lastWebcamTimestamp = timestamp;
	if ( image_size == 0 || real_size == 0)
	{
		// use timestamp to handle syncronization here???
		return;
	}
	
	if ( currentImage == NULL )
	{
		currentImage = new QBuffer();
		currentImage->open(IO_ReadWrite);
	}
	currentImage->writeBlock( (char *) image, real_size );
	//kdDebug(14181) << " real_size " << real_size << " image_size " << image_size << " timestamp " << timestamp << " " << who << endl;
	
	if ( currentImage->size() == image_size ) {
		QPixmap webcamImage;
		currentImage->close();
		// uncomment the following line when jpc is supported by kdelibs
		//webcamImage.loadFromData( currentImage->buffer() );
		
		/****** DELETE below once kdelibs has jpc support ******/
		KTempFile jpcTmpImageFile;
		KTempFile bmpTmpImageFile;
		QFile *file = jpcTmpImageFile.file();;
		file->writeBlock((currentImage->buffer()).data(), currentImage->size());
		file->close();
		
		KProcess p;
		p << "jasper";
		p << "--input" << jpcTmpImageFile.name() << "--output" << bmpTmpImageFile.name() << "--output-format" << "bmp";
		
		p.start( KProcess::Block );
		if( p.exitStatus() != 0 )
		{
			kdDebug(14181) << " jasper exited with status " << p.exitStatus() << " " << who << endl;
		}
		else
		{
			webcamImage.load( bmpTmpImageFile.name() );
			/******* UPTO THIS POINT ******/
			kdDebug(14181) << " emitting image " << currentImage->size() << endl;
			emit webcamImageReceived( QString::fromLatin1( who ), webcamImage );
		}
		QFile::remove(jpcTmpImageFile.name());
		QFile::remove(bmpTmpImageFile.name());
		
		delete currentImage;
		currentImage = NULL;
	}
}

void YahooSession::_webcamDisconnected( const char* who, int reason )
{
	kdDebug(14181) << k_funcinfo << "Webcam closed remotely, reason: " << reason << endl;
	emit remoteWebcamClosed( QString::fromLocal8Bit( who ), reason );
}


void YahooSession::slotReadReady()
{
	int ret = 1;
	
	//using sender is the only way to reliably get the socket 
	const KStreamSocket* socket = dynamic_cast<const KStreamSocket*>( sender() );
	if ( !socket )
	{
		kdDebug(14181) << k_funcinfo << "sender() was not a KStreamSocket!" << endl;
		return;
	}
	
	int fd = socket->socketDevice()->socket();
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
	const KStreamSocket* socket = dynamic_cast<const KStreamSocket*>( sender() );
	if ( !socket )
	{
		kdDebug(14181) << k_funcinfo << "sender() was not a KStreamSocket!" << endl;
		return;
	}
	
	int fd = socket->socketDevice()->socket();
	//kdDebug(14181) << k_funcinfo << "Socket FD: " << fd << endl;

	ret = yahoo_write_ready( m_connId , fd, m_data );

	if ( ret == -1 )
		kdDebug(14181) << k_funcinfo << "Read Error (" << errno << ": " << strerror(errno) << endl;
	else if ( ret == 0 )
		kdDebug(14181) << k_funcinfo << "Server closed socket" << endl;
}

#include "kyahoo.moc"

// vim: set noet ts=4 sts=4 sw=4:
// kate: indent-mode csands; tab-width 4;
