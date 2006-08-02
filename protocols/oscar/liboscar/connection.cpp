/*
	Kopete Oscar Protocol
	connection.cpp - independent protocol encapsulation

	Copyright (c) 2004-2005 by Matt Rogers <mattr@kde.org>

	Based on code Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
	Based on Iris, Copyright (C) 2003  Justin Karneges

	Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

	*************************************************************************
	*                                                                       *
	* This library is free software; you can redistribute it and/or         *
	* modify it under the terms of the GNU Lesser General Public            *
	* License as published by the Free Software Foundation; either          *
	* version 2 of the License, or (at your option) any later version.      *
	*                                                                       *
	*************************************************************************
*/

#include "connection.h"
#include "client.h"
#include "connector.h"
#include "oscarclientstream.h"
#include "rateclassmanager.h"
#include "task.h"
#include "transfer.h"
#include <kapplication.h>
#include <kdebug.h>

#include "oscartypeclasses.h"


class ConnectionPrivate
{
public:
	DWORD snacSequence;
	WORD flapSequence;

	QValueList<int> familyList;
	RateClassManager* rateClassManager;

	ClientStream* clientStream;
	Connector* connector;
	Client* client;

	Task* root;
};



Connection::Connection( Connector* connector, ClientStream* cs, const char* name )
: QObject( 0, name )
{
	d = new ConnectionPrivate();
	d->clientStream = cs;
	d->client = 0;
	d->connector = connector;
	d->rateClassManager = new RateClassManager( this );
	d->root = new Task( this, true /* isRoot */ );
	m_loggedIn = false;
	initSequence();

}

Connection::~Connection()
{

	delete d->rateClassManager;
	delete d->clientStream;
	delete d->connector;
	delete d->root;
	delete d;
}

void Connection::setClient( Client* c )
{
	d->client = c;
	connect( c, SIGNAL( loggedIn() ), this, SLOT( loggedIn() ) );
}

void Connection::connectToServer( const QString& host, bool auth )
{
	connect( d->clientStream, SIGNAL( error( int ) ), this, SLOT( streamSocketError( int ) ) );
	connect( d->clientStream, SIGNAL( readyRead() ), this, SLOT( streamReadyRead() ) );
	connect( d->clientStream, SIGNAL( connected() ), this, SIGNAL( connected() ) );
	d->clientStream->connectToServer( host, auth );
}

void Connection::close()
{
	d->clientStream->close();
	reset();
}

bool Connection::isSupported( int family ) const
{
	return ( d->familyList.findIndex( family ) != -1 );
}

QValueList<int> Connection::supportedFamilies() const
{
	return d->familyList;
}

void Connection::addToSupportedFamilies( const QValueList<int>& familyList )
{
	d->familyList += familyList;
}

void Connection::addToSupportedFamilies( int family )
{
	d->familyList.append( family );
}

void Connection::taskError( const Oscar::SNAC& s, int errCode )
{
	d->client->notifyTaskError( s, errCode, false /*fatal*/ );
}

void Connection::fatalTaskError( const Oscar::SNAC& s, int errCode )
{
	d->client->notifyTaskError( s, errCode, true /* fatal */ );
}

Oscar::Settings* Connection::settings() const
{
	return d->client->clientSettings();
}

Q_UINT16 Connection::flapSequence()
{
	d->flapSequence++;
	if ( d->flapSequence >= 0x8000 ) //the max flap sequence is 0x8000 ( HEX )
		d->flapSequence = 1;

	return d->flapSequence;
}

Q_UINT32 Connection::snacSequence()
{
	d->snacSequence++;
	return d->snacSequence;
}

QString Connection::userId() const
{
	return d->client->userId();
}

QString Connection::password() const
{
	return d->client->password();
}

bool Connection::isIcq() const
{
	return d->client->isIcq();
}

Task* Connection::rootTask() const
{
	return d->root;
}

SSIManager* Connection::ssiManager() const
{
	return d->client->ssiManager();
}

const Oscar::ClientVersion* Connection::version() const
{
	return d->client->version();
}

bool Connection::isLoggedIn() const
{
	return m_loggedIn;
}

RateClassManager* Connection::rateManager() const
{
	return d->rateClassManager;
}

void Connection::send( Transfer* request ) const
{
	if( !d->clientStream )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "No stream to write on!" << endl;
		return;
	}
	d->rateClassManager->queue( request );

}

void Connection::forcedSend( Transfer* request ) const
{
	if ( !d->clientStream )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "No stream to write on" << endl;
		return;
	}
	d->clientStream->write( request );
}

void Connection::initSequence()
{
	d->snacSequence = ( KApplication::random() & 0xFFFF );
	d->flapSequence = ( KApplication::random() & 0xFFFF );
}

void Connection::distribute( Transfer * transfer ) const
{
	//d->rateClassManager->recalcRateLevels();
	if( !rootTask()->take( transfer ) )
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "root task refused transfer" << endl;

	delete transfer;
}

void Connection::reset()
{
	//clear the family list
	d->familyList.clear();
	d->rateClassManager->reset();
}

void Connection::streamReadyRead()
{
	// take the incoming transfer and distribute it to the task tree
	Transfer * transfer = d->clientStream->read();
	distribute( transfer );
}

void Connection::loggedIn()
{
	m_loggedIn = true;
}

void Connection::streamSocketError( int code )
{
	emit socketError( code, d->clientStream->errorText() );
}

#include "connection.moc"
//kate: tab-width 4; indent-mode csands;
