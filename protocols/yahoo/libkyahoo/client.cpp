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
#include "client.h"

class Client::ClientPrivate
{
public:
	ClientPrivate() {}

	ClientStream *stream;
	int id_seed;
	Task *root;
	QString host, user, pass;
	uint port;
/*	int tzoffset;*/
	bool active;

	// tasks
	LoginTask * loginTask;
	ListTask *listTask;

	// Connection data
	uint sessionID;
	QString yCookie;
	QString tCookie;
	QString cCookie;
	QString loginCookie;

};

Client::Client(QObject *par) :QObject(par, "yahooclient" )
{
	d = new ClientPrivate;
/*	d->tzoffset = 0;*/
	d->active = false;

	d->root = new Task(this, true);
	d->loginTask = new LoginTask( d->root );
	d->listTask = new ListTask( d->root );
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
	QObject::connect( d->loginTask, SIGNAL( finished() ), SLOT( lt_loginFinished() ) );
	QObject::connect( d->listTask, SIGNAL( gotCookies() ), SLOT( slotGotCookies() ) );
	QObject::connect( d->listTask, SIGNAL( gotBuddy(const QString &, const QString &, const QString &) ), 
					SIGNAL( gotBuddy(const QString &, const QString &, const QString &) ) );
	d->loginTask->go(true);
	d->active = true;
}

void Client::close()
{
	kdDebug(14180) << k_funcinfo << endl;
}

QString Client::host()
{
	return d->host;
}

int Client::port()
{
	return d->port;
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

	kdDebug(14180) << k_funcinfo << "Emitting loggedIn" << endl;
	emit loggedIn( d->loginTask->statusCode(), d->loginTask->statusString() );
}

void Client::slotGotCookies()
{
	kdDebug(14180) << k_funcinfo << "Y: " << d->listTask->yCookie()
					<< " T: " << d->listTask->tCookie()
					<< " C: " << d->listTask->cCookie()
					<< " login: " << d->listTask->loginCookie() << endl;
	d->yCookie = d->listTask->yCookie();
	d->tCookie = d->listTask->tCookie();
	d->cCookie = d->listTask->cCookie();
	d->loginCookie = d->listTask->loginCookie();
}

// INTERNALS //

QString Client::userId()
{
	return d->user;
}

void Client::setUserId( const QString & userId )
{
	d->user = userId;
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

#include "client.moc"
