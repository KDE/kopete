/*
    Kopete Yahoo Protocol
    yahoochattask.cpp - Handle Yahoo Chat

    Copyright (c) 2006 André Duffeck <duffeck@kde.org>
    Kopete (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "yahoochattask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <kdebug.h>
#include <klocale.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <QDomDocument>

using namespace KYahoo;

YahooChatTask::YahooChatTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
	m_loggedIn = false;
}

YahooChatTask::~YahooChatTask()
{
}

bool YahooChatTask::take( Transfer* transfer )
{
	if ( !forMe( transfer ) )
		return false;

	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return false;
	
	if( t->service() == Yahoo::ServiceChatOnline )
		parseLoginResponse( t );
	else if( t->service() == Yahoo::ServiceComment )
		parseChatMessage( t );
	else if( t->service() == Yahoo::ServiceChatJoin )
		parseJoin( t );
	else if( t->service() == Yahoo::ServiceChatExit )
		parseChatExit( t );
	else if( t->service() == Yahoo::ServiceChatLogout )
		parseLogout( t );
	return true;
}

bool YahooChatTask::forMe( const Transfer* transfer ) const
{
	const YMSGTransfer *t = 0L;
	t = dynamic_cast<const YMSGTransfer*>(transfer);
	if (!t)
		return false;

	if ( t->service() == Yahoo::ServiceChatOnline ||
		t->service() == Yahoo::ServiceChatGoto ||
		t->service() == Yahoo::ServiceChatJoin ||
		t->service() == Yahoo::ServiceChatleave ||
		t->service() == Yahoo::ServiceChatExit ||
		t->service() == Yahoo::ServiceChatLogout ||
		t->service() == Yahoo::ServiceChatPing ||
		t->service() == Yahoo::ServiceChatLogon ||
		t->service() == Yahoo::ServiceChatLogoff ||
		t->service() == Yahoo::ServiceComment )	
		return true;
	else
		return false;
}

void YahooChatTask::onGo()
{
}

void YahooChatTask::getYahooChatCategories()
{
	kDebug(YAHOO_RAW_DEBUG) ;
	KIO::TransferJob *transfer;

	transfer = KIO::get( KUrl("http://insider.msg.yahoo.com/ycontent/?chatcat=0"), KIO::NoReload, KIO::HideProgressInfo );
	transfer->addMetaData( "UserAgent", "Mozilla/4.0 (compatible; MSIE 5.5)");
	transfer->addMetaData( "no-cache", "true" );
	transfer->addMetaData( "cookies", "manual" );
	transfer->addMetaData("setcookies", QString("Cookie: %1; %2; %3").arg(client()->tCookie(), client()->yCookie()) );


	connect( transfer, SIGNAL(result(KJob*)), this, SLOT(slotCategoriesComplete(KJob*)) );
	connect( transfer, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(slotData(KIO::Job*,QByteArray)) );
}

void YahooChatTask::getYahooChatRooms( const Yahoo::ChatCategory &category )
{
	kDebug(YAHOO_RAW_DEBUG) << "Category Id: " << category.id;
	KIO::TransferJob *transfer;

	transfer = KIO::get( KUrl(QString("http://insider.msg.yahoo.com/ycontent/?chatroom_%1=0").arg( category.id )), KIO::NoReload, KIO::HideProgressInfo );
	transfer->addMetaData( "UserAgent", "Mozilla/4.0 (compatible; MSIE 5.5)");
	transfer->addMetaData( "no-cache", "true" );
	transfer->addMetaData( "cookies", "manual" );
	transfer->addMetaData("setcookies", QString("Cookie: %1; %2; %3").arg(client()->tCookie(), client()->yCookie()) );


	connect( transfer, SIGNAL(result(KJob*)), this, SLOT(slotChatRoomsComplete(KJob*)) );
	connect( transfer, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(slotData(KIO::Job*,QByteArray)) );

	m_jobs[ transfer ].category = category;
}

void YahooChatTask::slotData( KIO::Job *job, const QByteArray& data)
{
	kDebug(YAHOO_RAW_DEBUG) ;
	m_jobs[job].data.append( data );
}

void YahooChatTask::slotCategoriesComplete( KJob *job )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	KIO::TransferJob *transfer = static_cast< KIO::TransferJob * >(job);

	if ( job->error () || transfer->isErrorPage () )
	{
		kDebug(YAHOO_RAW_DEBUG) << "An error occurred while downloading the chat categories list.";
	}
	else
	{
		QDomDocument doc;
		doc.setContent( m_jobs[ transfer ].data );
		emit gotYahooChatCategories( doc );
	}

	m_jobs.remove( transfer );
}

void YahooChatTask::slotChatRoomsComplete( KJob *job )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	KIO::TransferJob *transfer = static_cast< KIO::TransferJob * >(job);

	if ( job->error () || transfer->isErrorPage () )
	{
		kDebug(YAHOO_RAW_DEBUG) << "An error occurred while downloading the chat categories list.";
	}
	else
	{
		QDomDocument doc;
		doc.setContent( m_jobs[ transfer ].data );
// 		kDebug(YAHOO_RAW_DEBUG) << doc.toString();
		emit gotYahooChatRooms( m_jobs[ transfer ].category, doc );
	}

	m_jobs.remove( transfer );
}

void YahooChatTask::joinRoom( const Yahoo::ChatRoom &room )
{
	kDebug(YAHOO_RAW_DEBUG) << "Joining room " << room.name << " (" << room.id << ")...";
	if( !m_loggedIn )
	{
		m_pendingJoins.append( room );
		login();
		return;
	}

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceChatJoin);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit() );
	t->setParam( 104, room.name.toLocal8Bit() );
	t->setParam( 129, room.id );
	t->setParam( 62, 2 );

	send( t );
}

void YahooChatTask::sendYahooChatMessage( const QString &msg, const QString &handle )
{
	kDebug(YAHOO_RAW_DEBUG) ;
	if( !m_loggedIn )
	{
		kDebug(YAHOO_RAW_DEBUG) << "Error: trying to send, but not logged in.";
		return;
	}

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceComment);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit() );
	t->setParam( 104, handle.toLocal8Bit() );
	t->setParam( 117, msg.toLocal8Bit() );
	t->setParam( 124, 1 );

	send( t );
}

void YahooChatTask::login()
{
	kDebug(YAHOO_RAW_DEBUG) ;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceChatOnline);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit() );
	t->setParam( 135, QString("ym%1").arg(YMSG_PROGRAM_VERSION_STRING).toLocal8Bit() );

	send( t );
}

void YahooChatTask::logout()
{	
	kDebug(YAHOO_RAW_DEBUG) ;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceChatLogout);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit() );

	send( t );
}

void YahooChatTask::parseLoginResponse( YMSGTransfer *t )
{
	if( !t->firstParam( 1 ).startsWith( client()->userId().toLocal8Bit() ) )
		return;
	m_loggedIn = true;
	for( int i = 0; i < m_pendingJoins.size(); ++i )
	{
		Yahoo::ChatRoom entry = m_pendingJoins.at( i );
		joinRoom( entry );
		m_pendingJoins.removeAt( i );
	}
}

void YahooChatTask::parseJoin( YMSGTransfer *t )
{
	int room;
	int category;
	QString handle;
	QString comment;
	bool suppressJoinNotification = false;
	QString error;

	room = t->firstParam( 129 ).toInt();
	category = t->firstParam( 128 ).toInt();
	handle = t->firstParam( 104 );
	comment = t->firstParam( 105 );
	error = t->firstParam( 114 );

	if( error.startsWith( "-35" ) ) {
		client()->notifyError( i18n("Could not join chat"), 
				i18n("The room is full. Please choose another one."), Client::Error );
		return;
	} else if( error.startsWith( "-15" ) ) {
		client()->notifyError( i18n("Could not join chat"), 
				i18n("Invalid user."), Client::Error );
		return;
	} else if( !error.isEmpty() ) {
		client()->notifyError( i18n("Could not join chat"), 
				i18n("An unknown error occurred while joining the chat room."), Client::Error );
		return;
	}

        // Yahoo sends a captcha requests before we can join the room
        if( room == 0 && category == 0 && !comment.isEmpty() ) 
        {
        	kDebug(YAHOO_RAW_DEBUG) << "Showing captcha request";
		emit chatRoomJoined( room, category, "", handle );
		emit chatMessageReceived( "Yahoo", comment, handle );
        }

	if( room > 0 && category > 0 )
	{
		// We have just joined this room. Suppress join notifications for the people
		// that are already in here.
		suppressJoinNotification = true;
		emit chatRoomJoined( room, category, comment, handle );
	}

	QString nick;
	for( int i = 0; i < t->paramCount( 109 ); ++i )
	{
		nick = t->nthParam( 109 , i );
		emit chatBuddyHasJoined( nick, handle, suppressJoinNotification );
	}
}

void YahooChatTask::parseChatMessage( YMSGTransfer *t )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	QString handle;
	QString msg;
	QString nick;

	handle = t->firstParam( 104 );
	for( int i = 0; i < t->paramCount( 109 ); ++i )
	{
		nick = t->nthParam( 109, i );
		msg = t->nthParamSeparated( 117, i, 109 );
		emit chatMessageReceived( nick, msg, handle );
	}
}

void YahooChatTask::parseChatExit( YMSGTransfer *t )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	QString handle;
	QString nick;

	handle = t->firstParam( 104 );
	for( int i = 0; i < t->paramCount( 109 ); ++i )
	{
		nick = t->nthParam( 109, i );
		emit chatBuddyHasLeft( nick, handle );
	}
}

void YahooChatTask::parseLogout( YMSGTransfer *t )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	QString nick = t->firstParam( 1 );
	if( nick == client()->userId() )
		m_loggedIn = false;
}

#include "yahoochattask.moc"
