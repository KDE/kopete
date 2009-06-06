/*
   Kopete Oscar Protocol
   ssiauthtask.cpp - SSI Authentication Task

   Copyright (c) 2004 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

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

#include "ssiauthtask.h"
#include "contactmanager.h"
#include "transfer.h"
#include "buffer.h"
#include "connection.h"
#include "oscarutils.h"

#include <kdebug.h>

#include <QTextCodec>

SSIAuthTask::SSIAuthTask( Task* parent )
	: Task( parent )
{
	m_manager = parent->client()->ssiManager();
}

SSIAuthTask::~SSIAuthTask()
{
}

bool SSIAuthTask::forMe( const Transfer* t ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*> ( t );
	
	if ( !st )
		return false;
		
	if ( st->snacService() != 0x0013 )
		return false;
		
	switch ( st->snacSubtype() )
	{
		case 0x0015: // Future authorization granted
		case 0x0019: // Authorization request
		case 0x001b: // Authorization reply
		case 0x001c: // "You were added" message
			return true;
			break;
		default:
			return false;
	}
}

bool SSIAuthTask::take( Transfer* t )
{
	if ( forMe( t ) )
	{
		setTransfer( t );
		SnacTransfer* st = static_cast<SnacTransfer*> ( t );
		
		switch ( st->snacSubtype() )
		{
			case 0x0015: // Future authorization granted
				handleFutureAuthGranted();
				break;
			case 0x0019: // Authorization request
				handleAuthRequested();
				break;
			case 0x001b: // Authorization reply
				handleAuthReplied();
				break;
			case 0x001c: // "You were added" message
				handleAddedMessage();
				break;
		}
		setTransfer( 0 );
		return true;
	}
	return false;
}

void SSIAuthTask::grantFutureAuth( const QString& uin, const QString& reason )
{
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0013, 0x0014, 0x0000, client()->snacSequence() };
	
	Buffer* buf = new Buffer();
	buf->addBUIN( uin.toLatin1() );
	buf->addBSTR( reason.toUtf8() );
	buf->addWord( 0x0000 ); // Unknown
	
	Transfer* t = createTransfer( f, s, buf );
	send( t );
}

void SSIAuthTask::sendAuthRequest( const QString& uin, const QString& reason )
{
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0013, 0x0018, 0x0000, client()->snacSequence() };
	
	Buffer* buf = new Buffer();
	buf->addBUIN( uin.toLatin1() );
	buf->addBSTR( reason.toUtf8() );
	buf->addWord( 0x0000 ); // Unknown
	
	Transfer* t = createTransfer( f, s, buf );
	send( t );
}

void SSIAuthTask::sendAuthReply( const QString& uin, const QString& reason, bool auth )
{
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0013, 0x001A, 0x0000, client()->snacSequence() };
	
	Buffer* buf = new Buffer();
	buf->addBUIN( uin.toLatin1() );
	buf->addByte( auth ? 0x01 : 0x00 ); // accepted / declined
	buf->addBSTR( reason.toUtf8() );
	
	Transfer* t = createTransfer( f, s, buf );
	send( t );
}

void SSIAuthTask::handleFutureAuthGranted()
{
	Buffer* buf = transfer()->buffer();
	
	QString uin = Oscar::normalize( buf->getBUIN() );
	QString reason = parseReason( buf );
	
	kDebug( OSCAR_RAW_DEBUG ) << "Future authorization granted from " << uin;
	kDebug( OSCAR_RAW_DEBUG ) << "Reason: " << reason;
	emit futureAuthGranted( uin, reason );
}

void SSIAuthTask::handleAuthRequested()
{
	Buffer* buf = transfer()->buffer();
	
	QString uin = Oscar::normalize( buf->getBUIN() );
	QString reason = parseReason( buf );
	
	kDebug( OSCAR_RAW_DEBUG ) << "Authorization requested from " << uin;
	kDebug( OSCAR_RAW_DEBUG ) << "Reason: " << reason;
	
	emit authRequested( uin, reason );
}

void SSIAuthTask::handleAuthReplied()
{
	Buffer* buf = transfer()->buffer();
	
	QString uin = Oscar::normalize( buf->getBUIN() );
	bool accepted = buf->getByte();
	QString reason = parseReason( buf );
	
	if ( accepted )
		kDebug( OSCAR_RAW_DEBUG ) << "Authorization request accepted by " << uin;
	else
		kDebug( OSCAR_RAW_DEBUG ) << "Authorization request declined by " << uin;
		
	kDebug( OSCAR_RAW_DEBUG ) << "Reason: " << reason;
	emit authReplied( uin, reason, accepted );
}

void SSIAuthTask::handleAddedMessage()
{
	Buffer* buf = transfer()->buffer();
	
	QString uin = Oscar::normalize( buf->getBUIN() );
	
	kDebug( OSCAR_RAW_DEBUG ) << "User " << uin << " added you to the contact list";
	emit contactAddedYou( uin );
}

QString SSIAuthTask::parseReason( Buffer* buffer )
{
	QTextCodec* codec = 0;
	
	QByteArray reasonData = buffer->getBSTR();
	Oscar::WORD tlvCount = buffer->getWord();
	
	if ( tlvCount > 0 )
	{
		QList<Oscar::TLV> tlvList = buffer->getTLVList();
		
		Oscar::TLV encodingTlv = findTLV( tlvList, 0x0001 );
		if ( encodingTlv )
			codec = Oscar::codecForName( encodingTlv.data );
	}
	
	if ( codec )
		return codec->toUnicode( reasonData );
	else
		return QString::fromUtf8( reasonData );
}

#include "ssiauthtask.moc"
//kate: tab-width 4; indent-mode csands;
