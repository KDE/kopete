/*
    Kopete Groupwise Protocol
    coreprotocol.h- the core GroupWise protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges
    url_escape_string from Gaim src/protocols/novell/nmconn.c
    Copyright (c) 2004 Novell, Inc. All Rights Reserved

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

#include <string.h>
#include <iostream>

#include <qdatastream.h>
#include <qdatetime.h>
#include <qtextstream.h>


#include <kdebug.h>
#include <kurl.h>

#include "coreprotocol.h"
#include "ymsgprotocol.h"
#include "ymsgtransfer.h"

CoreProtocol::CoreProtocol() : QObject()
{
	m_YMSGProtocol = new YMSGProtocol( this, "ymsgprotocol" );
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
	qDebug( "CoreProtocol::addIncomingData()");
	int oldsize = m_in.size();
	m_in.resize( oldsize + incomingBytes.size() );
	memcpy( m_in.data() + oldsize, incomingBytes.data(), incomingBytes.size() );
	m_state = Available;
	// convert every event in the chunk to a Transfer, signalling it back to the clientstream
	
	int parsedBytes = 0;
	int transferCount = 0;
	// while there is data left in the input buffer, and we are able to parse something out of it
	/*
	while ( m_in.size() && ( parsedBytes = wireToTransfer( m_in ) ) )
	{
		transferCount++;
		qDebug( "CoreProtocol::addIncomingData() - parsed transfer #%i in chunk", transferCount);
		int size =  m_in.size();
		if ( parsedBytes < size )
		{
			qDebug( " - more data in chunk!" );
			// copy the unparsed bytes into a new qbytearray and replace m_in with that
			QByteArray remainder( size - parsedBytes );
			memcpy( remainder.data(), m_in.data() + parsedBytes, remainder.size() );
			m_in = remainder;
		}
		else
			m_in.truncate( 0 );
	}
	if ( m_state == NeedMore )
		qDebug( " - message was incomplete, waiting for more..." );
	if ( m_eventProtocol->state() == EventProtocol::OutOfSync )
	{	
		qDebug( " - protocol thinks it's out of sync, discarding the rest of the buffer and hoping the server regains sync soon..." );
		m_in.truncate( 0 );
	}
	qDebug( " - done processing chunk" );
	*/
}

Transfer* CoreProtocol::incomingTransfer()
{	
	qDebug( "CoreProtocol::incomingTransfer()" );
	if ( m_state == Available )
	{
		qDebug( " - got a transfer" );
		m_state = NoData;
		return m_inTransfer;
		m_inTransfer = 0;
	}
	else
	{
		qDebug( " - no milk today." );
		return 0;
	}
}

void cp_dump( const QByteArray &bytes )
{
#ifdef OSCAR_COREPROTOCOL_DEBUG
	qDebug( "contains: %i bytes", bytes.count() );
	for ( uint i = 0; i < bytes.count(); ++i )
	{
		printf( "%02x ", bytes[ i ] );
	}
	printf( "\n" );
#endif
}

void CoreProtocol::outgoingTransfer( Transfer* outgoing )
{
	kdDebug(14180) << k_funcinfo << endl;
	if ( outgoing->type() == Transfer::YMSGTransfer )
	{
		kdDebug(14180) << k_funcinfo << " got YMSGTransfer" << endl;
		YMSGTransfer *yt = (YMSGTransfer *) outgoing;
		QByteArray bytesOut = yt->serialize();
		
		//QTextStream dout( bytesOut, IO_WriteOnly );
		//dout.setEncoding( QTextStream::Latin1 );
		//dout.setByteOrder( QDataStream::LittleEndian );
		//dout << bytesOut;
		kdDebug(14181) << k_funcinfo << " " << bytesOut << endl;
		emit outgoingData( bytesOut );
		// now convert 
		//fieldsToWire( fields );
		return;
	}
}



int CoreProtocol::wireToTransfer( const QByteArray& wire )
{
	// processing incoming data and reassembling it into transfers
	// may be an event or a response
	
	uint bytesParsed = 0;
			
	if ( wire.size() < 6 ) //check for valid flap length
	{
		m_state = NeedMore;
		return bytesParsed;
	}	
	
	m_din = new QDataStream( wire, IO_ReadOnly );
	
	// look at first four bytes and decide what to do with the chunk
	QByteArray ymsgStart = QByteArray(4);
	if ( okToProceed() )
	{
		*m_din >> ymsgStart;
		
		if ( (char * ) ymsgStart.data() == "YMSG" )
		{
			qDebug( "CoreProtocol::wireToTransfer() - looks like a valid YMSG packet " );
			/*
			QByteArray packet = wire.duplicate( wire.data(), flapLength + 6 )
			Transfer * t = m_YMSGProtocol->parse( packet, bytesParsed );
			
			if ( t )
			{
				m_inTransfer = t;
				qDebug( "CoreProtocol::wireToTransfer() - got a valid packet " );
				
				m_state = Available;
				emit incomingData();
			}
			else
				bytesParsed = 0;
			*/	
		}
		else 
		{ //unknown wire format
		}
	}
	delete m_din;
	return bytesParsed;
}

void CoreProtocol::reset()
{
	m_in.resize( 0 );
}

void CoreProtocol::slotOutgoingData( const QCString &out )
{
	qDebug( "%s", out.data() );
}

bool CoreProtocol::okToProceed()
{
	if ( m_din )
	{
		if ( m_din->atEnd() )
		{
			m_state = NeedMore;
			qDebug( "EventProtocol::okToProceed() - Server message ended prematurely!" );
		}
		else
			return true;
	}
	return false;
}

#include "coreprotocol.moc"
