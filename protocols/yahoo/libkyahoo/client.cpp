/*
	client.cpp - Kopete Oscar Protocol
	
	Copyright (c) 2004 Matt Rogers <matt.rogers@kdemail.net>
	
	Based on code Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
	Based on Iris, Copyright (C) 2003  Justin Karneges
	
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

#include <qapplication.h> //for qDebug

#include "yahooclientstream.h"
#include "task.h"
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

};

Client::Client(QObject *par)
:QObject(par, "yahooclient" )
{
	d = new ClientPrivate;
/*	d->tzoffset = 0;*/
	d->active = false;

	d->root = new Task(this, true);
	d->stream = 0;
}

Client::~Client()
{
	close();
	delete d->root;
	delete d;
}

void Client::connectToServer( ClientStream *s, const QString& server, bool auth )
{
	d->stream = s;

	connect(d->stream, SIGNAL(error(int)), SLOT(streamError(int)));
	connect(d->stream, SIGNAL(readyRead()), SLOT(streamReadyRead()));

	d->stream->connectToServer(server, auth);
}

void Client::start( const QString &host, const uint port, const QString &userId, const QString &pass )
{
	d->host = host;
	d->port = port;
	d->user = userId;
	d->pass = pass;

	//start login task here
	d->active = true;
}

void Client::close()
{
	qDebug( "TODO: close()" );
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
	qDebug( "CLIENT ERROR (Error %i)", error );
}

void Client::streamReadyRead()
{
	qDebug( "CLIENT STREAM READY READ" );
	// take the incoming transfer and distribute it to the task tree
	Transfer * transfer = d->stream->read();
	distribute( transfer );
}

void Client::lt_loginFinished()
{
	qDebug( "Client::lt_loginFinished() got login finished" );
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
	if( !rootTask()->take( transfer ) )
		qDebug( "CLIENT: root task refused transfer" );
}

void Client::send( Transfer* request )
{
	qDebug( "CLIENT::send()" );
	if( !d->stream )
	{	
		qDebug( "CLIENT - NO STREAM TO SEND ON!");
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
