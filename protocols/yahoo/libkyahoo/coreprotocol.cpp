/*
    Kopete Yahoo Protocol
    
    Copyright (c) 2004 Duncan Mac-Vicar P. <duncan@kde.org>
    
    Based on code 
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
	int oldsize = m_in.size();
	kdDebug(14180) << k_funcinfo << incomingBytes.size() << " bytes. already had " << oldsize << " bytes" << endl;
	
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
		kdDebug(14180) << k_funcinfo << " parsed transfer " <<  transferCount << " in chunk of "<< parsedBytes << " bytes" << endl; 
		int size =  m_in.size();
		if ( parsedBytes < size )
		{
			kdDebug(14180) << k_funcinfo << " more data in chunk! ( I have parsed " << parsedBytes << " and total data of " << size << ")" << endl;
			// copy the unparsed bytes into a new qbytearray and replace m_in with that
			QByteArray remainder( size - parsedBytes );
			memcpy( remainder.data(), m_in.data() + parsedBytes, remainder.size() );
			m_in = remainder;
		}
		else
			m_in.truncate( 0 );
	}
	if ( m_state == NeedMore )
		kdDebug(14180) << k_funcinfo << " message was incomplete, waiting for more..." << endl;
	/*
	if ( m_eventProtocol->state() == EventProtocol::OutOfSync )
	{	
		qDebug( " - protocol thinks it's out of sync, discarding the rest of the buffer and hoping the server regains sync soon..." );
		m_in.truncate( 0 );
	}
	*/
	kdDebug(14180) << k_funcinfo << " done processing chunk" << endl;
	
}

Transfer* CoreProtocol::incomingTransfer()
{
	kdDebug(14180) << k_funcinfo << endl;	
	if ( m_state == Available )
	{
		kdDebug(14180) << k_funcinfo << " - got a transfer" << endl;
		m_state = NoData;
		return m_inTransfer;
		m_inTransfer = 0;
	}
	else
	{
		kdDebug(14180) << k_funcinfo << " no milk today" << endl;
		return 0;
	}
}

void cp_dump( const QByteArray &bytes )
{
#ifdef YAHOO_COREPROTOCOL_DEBUG
	kdDebug(14180) << k_funcinfo << " contains " << bytes.count() << " bytes" << endl;
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
		//kdDebug(14181) << k_funcinfo << " " << bytesOut << endl;
		emit outgoingData( bytesOut );
		// now convert 
		//fieldsToWire( fields );
		return;
	}
}



int CoreProtocol::wireToTransfer( const QByteArray& wire )
{
	kdDebug(14180) << k_funcinfo << endl;
	// processing incoming data and reassembling it into transfers
	// may be an event or a response
	
	uint bytesParsed = 0;
			
	if ( wire.size() < 20 ) // minimal value of a YMSG header
	{
		m_state = NeedMore;
		return bytesParsed;
	}
	
	m_din = new QDataStream( wire, IO_ReadOnly );
	
	// look at first four bytes and decide what to do with the chunk
	if ( okToProceed() )
	{
		if ( (wire[0] == 'Y') && (wire[1] == 'M') && (wire[2] == 'S') && (wire[3] == 'G'))
		{
			kdDebug(14180) << k_funcinfo << " - looks like a valid YMSG packet" << endl;
			Transfer *t = m_YMSGProtocol->parse( wire, bytesParsed );
			kdDebug(14180) << k_funcinfo << " - YMSG Protocol parsed " << bytesParsed << " bytes" << endl;
			if ( t )
			{
				m_inTransfer = t;
				kdDebug(14180) << k_funcinfo << " - got a valid packet " << endl;
				
				m_state = Available;
				emit incomingData();
			}
			else
				bytesParsed = 0;
		}
		else 
		{ 
			kdDebug(14180) << k_funcinfo << " - not a valid YMSG packet" << endl;
			kdDebug(14180) << k_funcinfo << wire[0] << wire[1] << wire[2] << wire[3] << endl;
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
			kdDebug(14180) << k_funcinfo << " saved message prematurely" << endl;
		}
		else
			return true;
	}
	return false;
}

#include "coreprotocol.moc"
