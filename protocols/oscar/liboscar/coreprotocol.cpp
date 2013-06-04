/*
    Kopete Oscar Protocol
    coreprotocol.h- the core Oscar protocol

    Copyright (c) 2004 by Matt Rogers <mattr@kde.org>

    Based on code Copyright (c) 2004 SuSE Linux AG http://www.suse.com

    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>
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

#include "coreprotocol.h"

#include <qdatastream.h>
#include <qdatetime.h>
#include <qtextstream.h>
#include <QByteArray>

#include <kdebug.h>
#include <ctype.h>

#include "oscartypes.h"
#include "transfer.h"
#include "flapprotocol.h"
#include "snacprotocol.h"

static QString toString( const QByteArray& buffer )
{
	// line format:
	//00 03 00 0b 00 00 90 b8 f5 9f 09 31 31 33 37 38   |;tJ�..........|

	int i = 0;
	QString output = "\n";
	QString hex, ascii;

	QByteArray::ConstIterator it;
	for ( it = buffer.begin(); it != buffer.end(); ++it )
	{
		i++;

		unsigned char c = static_cast<unsigned char>(*it);

		if ( c < 0x10 )
			hex.append("0");
		hex.append(QString("%1 ").arg(c, 0, 16));

		ascii.append(isprint(c) ? c : '.');

		if (i == 16)
		{
			output += hex + "   |" + ascii + "|\n";
			i=0;
			hex.clear();
			ascii.clear();
		}
	}

	if(!hex.isEmpty())
		output += hex.leftJustified(48, ' ') + "   |" + ascii.leftJustified(16, ' ') + '|';
	output.append('\n');

	return output;
}


using namespace Oscar;

CoreProtocol::CoreProtocol() : QObject()
{
	m_snacProtocol = new SnacProtocol( this );
	m_flapProtocol = new FlapProtocol( this );
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
	kDebug(OSCAR_RAW_DEBUG) << "Received " << incomingBytes.count() << " bytes. ";
	// store locally
	m_in.append( incomingBytes );
	m_state = Available;

	// convert every event in the chunk to a Transfer, signalling it back to the clientstream
	int parsedBytes = 0;
	int transferCount = 0;
	// while there is data left in the input buffer, and we are able to parse something out of it
	while ( m_in.size() && ( parsedBytes = wireToTransfer( m_in ) ) )
	{
		transferCount++;
		m_in.remove(0, parsedBytes);
	}

	if ( m_state == NeedMore )
		kDebug(OSCAR_RAW_DEBUG) << "message was incomplete, waiting for more...";

	if ( m_snacProtocol->state() == OutOfSync || m_flapProtocol->state() == OutOfSync )
	{
		kDebug(OSCAR_RAW_DEBUG) << "protocol thinks it's out of sync. "
			<< "discarding the rest of the buffer and hoping the server regains sync soon..." << endl;
		m_in.truncate( 0 );
	}
}

Transfer* CoreProtocol::incomingTransfer()
{
	if ( m_state == Available )
	{
		m_state = NoData;
		return m_inTransfer;
		m_inTransfer = 0;
	}
	else
	{
		kDebug(OSCAR_RAW_DEBUG) << "we shouldn't be here!" << kBacktrace();
		return 0;
	}
}

void cp_dump( const QByteArray &bytes )
{
#ifdef OSCAR_COREPROTOCOL_DEBUG
	kDebug(OSCAR_RAW_DEBUG) << "contains: " << bytes.count() << " bytes";
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
	//kDebug(OSCAR_RAW_DEBUG) << "CoreProtocol::outgoingTransfer()";
	// Convert the outgoing data into wire format
	// pretty leet, eh?
	emit outgoingData( outgoing->toWire() );
	delete outgoing;

	return;
}

int CoreProtocol::wireToTransfer( const QByteArray& wire )
{
	// processing incoming data and reassembling it into transfers
	// may be an event or a response

	Oscar::BYTE flapStart, flapChannel = 0;
	Oscar::WORD flapLength = 0;
	Oscar::WORD s1, s2 = 0;
	uint bytesParsed = 0;

	//kDebug(OSCAR_RAW_DEBUG) << "Current packet" << toString(wire);
	if ( wire.size() < 6 ) //check for valid flap length
	{
		kDebug(OSCAR_RAW_DEBUG) 
				<< "packet not long enough! couldn't parse FLAP!" << endl;
		kDebug(OSCAR_RAW_DEBUG) << "packet size is " << wire.size();
		m_state = NeedMore;
		return bytesParsed;
	}

	QByteArray tempWire = wire;
	QDataStream din( &tempWire, QIODevice::ReadOnly );

	// look at first four bytes and decide what to do with the chunk
	if ( okToProceed( din ) )
	{
		din >> flapStart;
		QByteArray packet( wire );
		if ( flapStart == 0x2A )
		{
			din >> flapChannel;
			din >> flapLength; //discard the first one it's not really the flap length
			din >> flapLength;
			if ( wire.size() < flapLength )
			{
				kDebug(OSCAR_RAW_DEBUG) 
					<< "Not enough bytes to make a correct transfer. Have " << wire.size()
					<< " bytes. need " << flapLength + 6 << " bytes" << endl;
				m_state = NeedMore;
				return bytesParsed;
			}

			Transfer *t;
			if ( flapChannel == 2 )
			{
				din >> s1;
				din >> s2;
				t = m_snacProtocol->parse( packet, bytesParsed );
			}
			else
				t = m_flapProtocol->parse( packet, bytesParsed );

			if ( t )
			{
				m_inTransfer = t;
				m_state = Available;
				emit incomingData();
			}
			else
			{
				bytesParsed = 0;
				m_state = NeedMore;
			}
		}
		else
		{ //unknown wire format
			kDebug(OSCAR_RAW_DEBUG) << "unknown wire format detected!";
			kDebug(OSCAR_RAW_DEBUG) << "start byte is " << flapStart;
			kDebug(OSCAR_RAW_DEBUG) << "Packet is " << endl << toString( wire );
		}

	}
	return bytesParsed;
}

void CoreProtocol::reset()
{
	m_in.clear();
}

void CoreProtocol::slotOutgoingData( const QByteArray &out )
{
	kDebug(OSCAR_RAW_DEBUG) << out.data();
}

bool CoreProtocol::okToProceed( const QDataStream &din )
{
	if ( din.atEnd() )
	{
		m_state = NeedMore;
		kDebug(OSCAR_RAW_DEBUG) << "Server message ended prematurely!";
		return false;
	}
	else
		return true;
}

#include "coreprotocol.moc"
//kate: indent-mode csands; tab-width 4;
