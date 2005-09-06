/*
    Kopete Yahoo Protocol
    
    Copyright (c) 2005 Andre Duffeck <andre.duffeck@kdemail.net>
    Copyright (c) 2004 Duncan Mac-Vicar P. <duncan@kde.org>
    Copyright (c) 2004 Matt Rogers <matt.rogers@kdemail.net>
    Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
    Copyright (C) 2003  Justin Karneges
    
    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <kdebug.h>

#include "yahooclientstream.h"
#include "yahooconnector.h"
#include "task.h"
#include "logintask.h"
#include "listtask.h"
#include "statusnotifiertask.h"
#include "mailnotifiertask.h"
#include "messagereceivertask.h"
#include "sendnotifytask.h"
#include "sendmessagetask.h"
#include "logofftask.h"
#include "client.h"
#include "yahootypes.h"

class Client::ClientPrivate
{
public:
	ClientPrivate() {}

	ClientStream *stream;
	int id_seed;
	Task *root;
	QString host, user, pass;
	uint port;
	bool active;

	// tasks
	LoginTask * loginTask;
	ListTask *listTask;
	StatusNotifierTask *statusTask;
	MailNotifierTask *mailTask;
	MessageReceiverTask *messageReceiverTask;

	// Connection data
	uint sessionID;
	QString yCookie;
	QString tCookie;
	QString cCookie;
	QString loginCookie;
	Yahoo::Status status;
	Yahoo::Status statusOnConnect;

};

Client::Client(QObject *par) :QObject(par, "yahooclient" )
{
	d = new ClientPrivate;
/*	d->tzoffset = 0;*/
	d->active = false;

	d->root = new Task(this, true);
	d->statusOnConnect = Yahoo::StatusAvailable;
	d->status = Yahoo::StatusDisconnected;
	d->stream = 0;
}

Client::~Client()
{
	close();
	delete d->root;
	delete d;
}

void Client::connect( const QString &host, const uint port, const QString &userId, const QString &pass )
{
	kdDebug(14180) << k_funcinfo << endl;
	d->host = host;
	d->port = port;
	d->user = userId;
	d->pass = pass;

	m_connector = new KNetworkConnector;
	m_connector->setOptHostPort( host, port );
	d->stream = new ClientStream( m_connector, this );
	QObject::connect( d->stream, SIGNAL( connected() ), this, SLOT( cs_connected() ) );
	QObject::connect( d->stream, SIGNAL( readyRead() ), this, SLOT( streamReadyRead() ) );
	
	
	d->stream->connectToServer( host, false );
}

void Client::cs_connected()
{
	kdDebug(14180) << k_funcinfo << endl;
	emit connected();
	kdDebug(14180) << k_funcinfo << " starting login task ... "<<  endl;

	d->loginTask = new LoginTask( d->root );
	d->listTask = new ListTask( d->root );
	//QObject::connect( d->loginTask, SIGNAL( finished() ), SLOT( lt_loginFinished() ) );
	QObject::connect( d->loginTask, SIGNAL( haveSessionID( uint ) ), SLOT( lt_gotSessionID( uint ) ) );
	QObject::connect( d->loginTask, SIGNAL( loginResponse( int, const QString& ) ), 
				SLOT( slotLoginResponse( int, const QString& ) ) );
	QObject::connect( d->loginTask, SIGNAL( haveCookies() ), SLOT( slotGotCookies() ) );
	QObject::connect( d->listTask, SIGNAL( gotBuddy(const QString &, const QString &, const QString &) ), 
					SIGNAL( gotBuddy(const QString &, const QString &, const QString &) ) );

	d->loginTask->setStateOnConnect( d->statusOnConnect );
	d->loginTask->go(true);
	d->active = true;
}

void Client::close()
{
	kdDebug(14180) << k_funcinfo << endl;
	LogoffTask *lt = new LogoffTask( d->root );
	lt->go( true );
	deleteTasks();
}

// SLOTS //
void Client::streamError( int error )
{
	kdDebug(14180) << k_funcinfo << "CLIENT ERROR (Error " <<  error<< ")" << endl;
}

void Client::streamReadyRead()
{
	// take the incoming transfer and distribute it to the task tree
	Transfer * transfer = d->stream->read();
	distribute( transfer );
}

void Client::lt_loginFinished()
{
	kdDebug(14180) << k_funcinfo << endl;

	if( d->loginTask->statusCode() == Yahoo::YAHOO_LOGIN_OK )
		initTasks();
	kdDebug(14180) << k_funcinfo << "Emitting loggedIn" << endl;
	emit loggedIn( d->loginTask->statusCode(), d->loginTask->statusString() );
}

void Client::slotLoginResponse( int response, const QString &msg )
{
	if( response == Yahoo::YAHOO_LOGIN_OK )
		initTasks();

	emit loggedIn( response, msg );
}

void Client::lt_gotSessionID( uint id )
{
	kdDebug(14180) << k_funcinfo << "Got SessionID: " << id << endl;	
	d->sessionID = id;
}

void Client::slotGotCookies()
{
	kdDebug(14180) << k_funcinfo << "Y: " << d->loginTask->yCookie()
					<< " T: " << d->loginTask->tCookie()
					<< " C: " << d->loginTask->cCookie()
					<< " login: " << d->loginTask->loginCookie() << endl;
	d->yCookie = d->loginTask->yCookie();
	d->tCookie = d->loginTask->tCookie();
	d->cCookie = d->loginTask->cCookie();
	d->loginCookie = d->loginTask->loginCookie();
}

// INTERNALS //

void Client::sendTyping( const QString &who, int typ)
{
	SendNotifyTask *snt = new SendNotifyTask( d->root );
	snt->setTarget( who );
	snt->setState( SendNotifyTask::Active );
	snt->setType( SendNotifyTask::NotifyTyping );
	snt->go( true );
}

void Client::sendMessage( const QString &to, const QString &msg )
{
	SendMessageTask *smt = new SendMessageTask( d->root );
	smt->setTarget( to );
	smt->setText( msg );
	smt->setPicureFlag( pictureFlag() );
	smt->go( true );
}

void Client::sendBuzz( const QString &to )
{
	SendMessageTask *smt = new SendMessageTask( d->root );
	smt->setTarget( to );
	smt->setText( QString::fromLatin1( "<ding>" ) );
	smt->setPicureFlag( pictureFlag() );
	smt->go( true );
}

QString Client::userId()
{
	return d->user;
}

void Client::setUserId( const QString & userId )
{
	d->user = userId;
}

Yahoo::Status Client::status()
{
	return d->status;
}

void Client::setStatus( Yahoo::Status status )
{
	d->status = status;
}

void Client::setStatusOnConnect( Yahoo::Status status )
{
	d->statusOnConnect = status;
}

QString Client::password()
{
	return d->pass;
}

QCString Client::ipAddress()
{
	//TODO determine ip address
	return "127.0.0.1";
}

QString Client::host()
{
	return d->host;
}

int Client::port()
{
	return d->port;
}

uint Client::sessionID()
{
	return d->sessionID;
}

int Client::pictureFlag()
{
	return 0;
}

void Client::distribute( Transfer * transfer )
{
	kdDebug(14180) << k_funcinfo << endl;
	if( !rootTask()->take( transfer ) )
		kdDebug(14180) << "CLIENT: root task refused transfer" << endl;
}

void Client::send( Transfer* request )
{
	kdDebug(14180) << "CLIENT::send()"<< endl;
	if( !d->stream )
	{	
		kdDebug(14180) << "CLIENT - NO STREAM TO SEND ON!" << endl;
		return;
	}

	d->stream->write( request );
}

void Client::debug(const QString &str)
{
	qDebug( "CLIENT: %s", str.ascii() );
}

Task * Client::rootTask()
{
	return d->root;
}

void Client::initTasks()
{
	d->statusTask = new StatusNotifierTask( d->root );
	QObject::connect( d->statusTask, SIGNAL( statusChanged( const QString&, int, const QString&, int ) ), 
				SIGNAL( statusChanged( const QString&, int, const QString&, int ) ) );
	QObject::connect( d->statusTask, SIGNAL( loginResponse( int, const QString& ) ), 
				SIGNAL( loggedIn( int, const QString& ) ) );

	d->mailTask = new MailNotifierTask( d->root );
	QObject::connect( d->mailTask, SIGNAL( mailNotify(const QString&, const QString&, int) ), 
				SIGNAL( mailNotify(const QString&, const QString&, int) ) );

	d->messageReceiverTask = new MessageReceiverTask( d->root );
	QObject::connect( d->messageReceiverTask, SIGNAL( gotIm(const QString&, const QString&, long, int) ),
				SIGNAL( gotIm(const QString&, const QString&, long, int) ) );
	QObject::connect( d->messageReceiverTask, SIGNAL( systemMessage(const QString&) ),
				SIGNAL( systemMessage(const QString&) ) );
	QObject::connect( d->messageReceiverTask, SIGNAL( typingNotify(const QString &, int) ),
				SIGNAL( typingNotify(const QString &, int) ) );
	QObject::connect( d->messageReceiverTask, SIGNAL( gotBuzz( const QString &, long ) ),
				SIGNAL( gotBuzz( const QString &, long ) ) );
}

void Client::deleteTasks()
{
	d->statusTask->deleteLater();
	d->statusTask = 0L;
	d->mailTask->deleteLater();
	d->mailTask = 0L;
	d->messageReceiverTask->deleteLater();
	d->messageReceiverTask = 0L;
}

#include "client.moc"
