/*
	Kopete Oscar Protocol
	connection.cpp - independent protocol encapsulation

	Copyright (c) 2004-2005 by Matt Rogers <mattr@kde.org>

	Based on code Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
	Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

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
#include "oscarclientstream.h"
#include "rateclassmanager.h"
#include "task.h"
#include "transfer.h"
#include <kdebug.h>

#include "oscartypeclasses.h"
#include <QList>
#include <krandom.h>


class ConnectionPrivate
{
public:
	// The Oscar::WORD isn't typo see snacSequence function
	Oscar::WORD snacSequence;
	Oscar::WORD flapSequence;

	QList<int> familyList;
	RateClassManager* rateClassManager;

	ClientStream* clientStream;
	Client* client;

	Task* root;
	QHash<Oscar::DWORD, Oscar::MessageInfo> snacSequenceToMessageInfo;
};

QList<Oscar::WORD> Connection::m_startFlapSequenceList;

Connection::Connection( ClientStream* cs, const char* name )
: QObject( 0 )
{
	setObjectName( QLatin1String(name) );
	d = new ConnectionPrivate();
	d->clientStream = cs;
	d->client = 0;
	d->rateClassManager = new RateClassManager( this );
	d->root = new Task( this, true /* isRoot */ );
	m_loggedIn = false;
	initSequence();

}

Connection::~Connection()
{
	// During clientStream deletion it can emit connected signal so we disconnect signals.
	disconnect( d->clientStream, 0, this, 0 );

	delete d->rateClassManager;
	delete d->clientStream;
	delete d;
}

void Connection::setStartFlapSequenceList( const QList<Oscar::WORD>& seqList )
{
	m_startFlapSequenceList = seqList;
}

void Connection::setClient( Client* c )
{
	d->client = c;
	connect( c, SIGNAL(loggedIn()), this, SLOT(loggedIn()) );
}

void Connection::connectToServer( const QString& host, quint16 port, bool encrypted, const QString &name )
{
	connect( d->clientStream, SIGNAL(error(int)), this, SLOT(streamSocketError(int)) );
	connect( d->clientStream, SIGNAL(readyRead()), this, SLOT(streamReadyRead()) );
	connect( d->clientStream, SIGNAL(connected()), this, SIGNAL(connected()) );
	d->clientStream->connectToServer( host, port, encrypted, name );
}

void Connection::close()
{
	d->clientStream->close();
	reset();
}

bool Connection::isSupported( int family ) const
{
	return ( d->familyList.indexOf( family ) != -1 );
}

QList<int> Connection::supportedFamilies() const
{
	return d->familyList;
}

void Connection::addToSupportedFamilies( const QList<int>& familyList )
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

Oscar::WORD Connection::flapSequence()
{
	d->flapSequence++;
	if ( d->flapSequence >= 0x8000 ) //the max flap sequence is 0x8000 ( HEX )
		d->flapSequence = 1;

	return d->flapSequence;
}

Oscar::DWORD Connection::snacSequence()
{
	d->snacSequence++;

	if ( d->snacSequence == 0x0000 )
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

ContactManager* Connection::ssiManager() const
{
	return d->client->ssiManager();
}

const Oscar::ClientVersion* Connection::version() const
{
	return d->client->version();
}

Oscar::Guid Connection::versionCap() const
{
	return d->client->versionCap();
}

bool Connection::isLoggedIn() const
{
	return m_loggedIn;
}

QHostAddress Connection::localAddress() const
{
	return d->clientStream->localAddress();
}

void Connection::addMessageInfo( Oscar::DWORD snacSequence, const Oscar::MessageInfo& messageInfo )
{
	d->snacSequenceToMessageInfo.insert( snacSequence, messageInfo );
}

Oscar::MessageInfo Connection::takeMessageInfo( Oscar::DWORD snacSequence )
{
	return d->snacSequenceToMessageInfo.take( snacSequence );
}

QList<Oscar::MessageInfo> Connection::messageInfoList() const
{
	return d->snacSequenceToMessageInfo.values();
}

RateClassManager* Connection::rateManager() const
{
	return d->rateClassManager;
}

void Connection::send( Transfer* request ) const
{
	if( !d->clientStream )
	{
		kDebug(OSCAR_RAW_DEBUG) << "No stream to write on!";
		return;
	}
	d->rateClassManager->queue( request );

}

void Connection::forcedSend( Transfer* request ) const
{
	if ( !d->clientStream )
	{
		kDebug(OSCAR_RAW_DEBUG) << "No stream to write on";
		return;
	}
	d->clientStream->write( request );
}

void Connection::initSequence()
{
	d->snacSequence = ( KRandom::random() & 0xFFFF );

	if ( m_startFlapSequenceList.isEmpty() )
		d->flapSequence = generateInitialFlapSequence();
	else
		d->flapSequence = m_startFlapSequenceList.value( qrand() % m_startFlapSequenceList.size() ) - 1;

	kDebug(OSCAR_RAW_DEBUG) << "d->flapSequence:" << hex << d->flapSequence;
}

Oscar::WORD Connection::generateInitialFlapSequence() const
{
	// Taken from Miranda (icq_packet.cpp)
	Oscar::DWORD n = qrand() % 0x8000;
	Oscar::DWORD s = 0;
	
	for ( Oscar::DWORD i = n; i >>= 3; s += i ) {}
	return ((((0 - s) ^ (Oscar::BYTE)n) & 7) ^ n) + 2;
}

void Connection::distribute( Transfer * transfer ) const
{
	//d->rateClassManager->recalcRateLevels();
	if( !rootTask()->take( transfer ) )
		kDebug(OSCAR_RAW_DEBUG) << "root task refused transfer";

	delete transfer;
}

void Connection::reset()
{
	//clear the family list
	d->familyList.clear();
	d->rateClassManager->reset();
	d->snacSequenceToMessageInfo.clear();
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
	emit socketError( code, d->clientStream->errorString() );
}

#include "connection.moc"
//kate: tab-width 4; indent-mode csands;
