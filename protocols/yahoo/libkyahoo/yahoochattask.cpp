/*
    Kopete Yahoo Protocol
    yahoochattask.cpp - Handle Yahoo Chat

    Copyright (c) 2006 André Duffeck <andre.duffeck@kdemail.net>
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

YahooChatTask::YahooChatTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	m_loggedIn = false;
}

YahooChatTask::~YahooChatTask()
{
}

bool YahooChatTask::take( Transfer* transfer )
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
	if ( !forMe( transfer ) )
		return false;

	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return false;
	
	if( t->service() == Yahoo::ServiceChatOnline )
		parseLoginResponse( t );

	return true;
}

bool YahooChatTask::forMe( const Transfer* transfer ) const
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
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
		t->service() == Yahoo::ServiceChatleave )	
		return true;
	else
		return false;
}

void YahooChatTask::onGo()
{
}

void YahooChatTask::getYahooChatCategories()
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	KIO::TransferJob *transfer;

	transfer = KIO::get( KUrl("http://insider.msg.yahoo.com/ycontent/?chatcat=0"), false, false );
	transfer->addMetaData( "UserAgent", "Mozilla/4.0 (compatible; MSIE 5.5)");
	transfer->addMetaData( "no-cache", "true" );
	transfer->addMetaData( "cookies", "manual" );
	transfer->addMetaData("setcookies", QString("Cookie: %1; %2; %3").arg(client()->tCookie(), client()->yCookie()) );


	connect( transfer, SIGNAL( result( KJob* ) ), this, SLOT( slotCategoriesComplete( KJob* ) ) );
	connect( transfer, SIGNAL( data( KIO::Job*, const QByteArray& ) ), this, SLOT( slotData( KIO::Job*, const QByteArray& ) ) );
}

void YahooChatTask::getYahooChatRooms( const Yahoo::ChatCategory &category )
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Category Id: " << category.id << endl;
	KIO::TransferJob *transfer;

	transfer = KIO::get( KUrl(QString("http://insider.msg.yahoo.com/ycontent/?chatroom_%1=0").arg( category.id )), false, false );
	transfer->addMetaData( "UserAgent", "Mozilla/4.0 (compatible; MSIE 5.5)");
	transfer->addMetaData( "no-cache", "true" );
	transfer->addMetaData( "cookies", "manual" );
	transfer->addMetaData("setcookies", QString("Cookie: %1; %2; %3").arg(client()->tCookie(), client()->yCookie()) );


	connect( transfer, SIGNAL( result( KJob* ) ), this, SLOT( slotChatRoomsComplete( KJob* ) ) );
	connect( transfer, SIGNAL( data( KIO::Job*, const QByteArray& ) ), this, SLOT( slotData( KIO::Job*, const QByteArray& ) ) );

	m_jobs[ transfer ].category = category;
}

void YahooChatTask::slotData( KIO::Job *job, const QByteArray& data)
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	m_jobs[job].data.append( data );
}

void YahooChatTask::slotCategoriesComplete( KJob *job )
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	KIO::TransferJob *transfer = static_cast< KIO::TransferJob * >(job);

	if ( job->error () || transfer->isErrorPage () )
	{
		kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "An error occurred while downloading the chat categories list." << endl;
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
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	KIO::TransferJob *transfer = static_cast< KIO::TransferJob * >(job);

	if ( job->error () || transfer->isErrorPage () )
	{
		kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "An error occurred while downloading the chat categories list." << endl;
	}
	else
	{
		QDomDocument doc;
		doc.setContent( m_jobs[ transfer ].data );
		kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << doc.toString() << endl;
		emit gotYahooChatRooms( m_jobs[ transfer ].category, doc );
	}

	m_jobs.remove( transfer );
}

void YahooChatTask::joinRoom( const Yahoo::ChatRoom &room )
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
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

void YahooChatTask::login()
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceChatOnline);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit() );
	t->setParam( 135, QString("ym%1").arg(YMSG_PROGRAM_VERSION_STRING).toLocal8Bit() );

	send( t );
}

void YahooChatTask::logout()
{	
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceChatLogout);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit() );

	send( t );
}

void YahooChatTask::parseLoginResponse( YMSGTransfer * )
{
	for( int i = 0; i < m_pendingJoins.size(); ++i )
	{
		Yahoo::ChatRoom entry = m_pendingJoins.at( i );
		joinRoom( entry );
		m_pendingJoins.removeAt( i );
	}
}

#include "yahoochattask.moc"
