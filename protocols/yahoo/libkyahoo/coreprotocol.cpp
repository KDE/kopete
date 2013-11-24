/*
    Kopete Yahoo Protocol
    
    Copyright (c) 2004 Duncan Mac-Vicar P. <duncan@kde.org>
    
    Based on code 
    Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
    Copyright (C) 2003  Justin Karneges <justin@affinix.com>
    
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

#include "coreprotocol.h"

#include <string.h>
#include <iostream>

#include <QDataStream>
#include <QDateTime>
#include <QTextStream>
#include <QByteArray>


#include <kdebug.h>
#include <kurl.h>

#include "ymsgprotocol.h"
#include "ymsgtransfer.h"

CoreProtocol::CoreProtocol() : QObject()
{
	m_YMSGProtocol = new YMSGProtocol( this );
	m_YMSGProtocol->setObjectName( QLatin1String("ymsgprotocol") );
}

CoreProtocol::~CoreProtocol() 
{
}

int CoreProtocol::state()
{
	return m_state;
}

void CoreProtocol::addIncomingData( const QByteArray & incomingBytes )
{
	// store locally
	int oldsize = m_in.size();
	kDebug(YAHOO_RAW_DEBUG) << incomingBytes.size() << " bytes. already had " << oldsize << " bytes";
	
	m_in.resize( oldsize + incomingBytes.size() );
	memcpy( m_in.data() + oldsize, incomingBytes.data(), incomingBytes.size() );
	
	m_state = Available;
	// convert every event in the chunk to a Transfer, signalling it back to the clientstream
	
	int parsedBytes = 0;
	int transferCount = 0;
	// while there is data left in the input buffer, and we are able to parse something out of it
	
	while ( m_in.size() && ( parsedBytes = wireToTransfer(m_in) ) )
	{
		transferCount++;
		kDebug(YAHOO_RAW_DEBUG) << " parsed transfer " <<  transferCount << " in chunk of "<< parsedBytes << " bytes"; 
		int size =  m_in.size();
		if ( parsedBytes < size )
		{
			kDebug(YAHOO_RAW_DEBUG) << " more data in chunk! ( I have parsed " << parsedBytes << " and total data of " << size << ")";
			// remove parsed bytes from the buffer
			m_in.remove( 0, parsedBytes );
		}
		else
			m_in.truncate( 0 );
	}
	if ( m_state == NeedMore )
		kDebug(YAHOO_RAW_DEBUG) << " message was incomplete, waiting for more...";
	/*
	if ( m_eventProtocol->state() == EventProtocol::OutOfSync )
	{	
		qDebug( " - protocol thinks it's out of sync, discarding the rest of the buffer and hoping the server regains sync soon..." );
		m_in.truncate( 0 );
	}
	*/
	kDebug(YAHOO_RAW_DEBUG) << " done processing chunk";
	
}

Transfer* CoreProtocol::incomingTransfer()
{
	kDebug(YAHOO_RAW_DEBUG) ;	
	if ( m_state == Available )
	{
// 		kDebug(YAHOO_RAW_DEBUG) << " - got a transfer";
		m_state = NoData;
		return m_inTransfer;
		m_inTransfer = 0;
	}
	else
	{
		kDebug(YAHOO_RAW_DEBUG) << " no milk today";
		return 0;
	}
}

void cp_dump( const QByteArray &bytes )
{
#ifdef YAHOO_COREPROTOCOL_DEBUG
	kDebug(YAHOO_RAW_DEBUG) << " contains " << bytes.count() << " bytes";
	for ( uint i = 0; i < bytes.count(); ++i )
	{
		printf( "%02x ", bytes[ i ] );
	}
	printf( "\n" );
#else
	Q_UNUSED( bytes );
#endif
}

void CoreProtocol::outgoingTransfer( Transfer* outgoing )
{
	kDebug(YAHOO_RAW_DEBUG) ;
	if ( outgoing->type() == Transfer::YMSGTransfer )
	{
		kDebug(YAHOO_RAW_DEBUG) << " got YMSGTransfer";
		YMSGTransfer *yt = (YMSGTransfer *) outgoing;
		QByteArray bytesOut = yt->serialize();
		
		//QTextStream dout( bytesOut, QIODevice::WriteOnly );
		//dout.setEncoding( QTextStream::Latin1 );
		//dout.setByteOrder( QDataStream::LittleEndian );
		//dout << bytesOut;
		//kDebug(YAHOO_RAW_DEBUG) << " " << bytesOut;
		emit outgoingData( bytesOut );
		// now convert 
		//fieldsToWire( fields );
	}
	delete outgoing;
}



int CoreProtocol::wireToTransfer( const QByteArray& wire )
{
	kDebug(YAHOO_RAW_DEBUG) ;
	// processing incoming data and reassembling it into transfers
	// may be an event or a response
	
	uint bytesParsed = 0;
			
	if ( wire.size() < 20 ) // minimal value of a YMSG header
	{
		m_state = NeedMore;
		return bytesParsed;
	}
	
	QByteArray tempWire = wire;
	QDataStream din( &tempWire, QIODevice::ReadOnly );
	
	// look at first four bytes and decide what to do with the chunk
	if ( okToProceed( din ) )
	{
		if ( (wire[0] == 'Y') && (wire[1] == 'M') && (wire[2] == 'S') && (wire[3] == 'G'))
		{
// 			kDebug(YAHOO_RAW_DEBUG) << " - looks like a valid YMSG packet";
			YMSGTransfer *t = static_cast<YMSGTransfer *>(m_YMSGProtocol->parse( wire, bytesParsed ));
// 			kDebug(YAHOO_RAW_DEBUG) << " - YMSG Protocol parsed " << bytesParsed << " bytes";
			if ( t )
			{
				if( wire.size() < t->packetLength() )
				{
					m_state = NeedMore;
					delete t;
					return 0;
				}
				m_inTransfer = t;
// 				kDebug(YAHOO_RAW_DEBUG) << " - got a valid packet ";
				
				m_state = Available;
				emit incomingData();
			}
			else
				bytesParsed = 0;
		}
		else 
		{ 
			kDebug(YAHOO_RAW_DEBUG) << " - not a valid YMSG packet. Trying to recover.";
			QTextStream s( wire, QIODevice::ReadOnly );
			QString remaining = s.readAll();
			int pos = remaining.indexOf( "YMSG", bytesParsed );
			if( pos >= 0 )
			{
				kDebug(YAHOO_RAW_DEBUG) << "Recover successful.";
				bytesParsed += pos;
			}
			else
			{
				kDebug(YAHOO_RAW_DEBUG) << "Recover failed. Dump it!";
				bytesParsed = wire.size();
			}
		}
	}
	return bytesParsed;
}

void CoreProtocol::reset()
{
	m_in.resize( 0 );
}

void CoreProtocol::slotOutgoingData( const QByteArray &out )
{
	qDebug( "%s", out.data() );
}

bool CoreProtocol::okToProceed( QDataStream &din)
{
	if ( din.atEnd() )
	{
		m_state = NeedMore;
		kDebug(YAHOO_RAW_DEBUG) << " saved message prematurely";
		return false;
	}
	else
		return true;
}

#include "coreprotocol.moc"
