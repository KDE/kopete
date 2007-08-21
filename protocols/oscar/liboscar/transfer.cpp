/*
    transfer.cpp - Kopete Groupwise Protocol

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>

    Based on code copyright (c) 2004 SUSE Linux AG <http://www.suse.com>

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

#include "transfer.h"
#include <ctype.h>
#include <kdebug.h>

Transfer::Transfer()
{
	m_isBufferValid = false;
}

Transfer::Transfer( Buffer* buf )
{
	m_buffer = buf;
	m_isBufferValid = true;
}

Transfer::TransferType Transfer::type() const
{
	return Transfer::RawTransfer;
}

QByteArray Transfer::toWire()
{
	m_wireFormat = m_buffer->buffer();
	return m_wireFormat;
}

Transfer::~Transfer()
{
	delete m_buffer;
	m_buffer = 0;
}

void Transfer::setBuffer( Buffer* buffer )
{
	m_buffer = buffer;
}

Buffer* Transfer::buffer()
{
	return m_buffer;
}

const Buffer* Transfer::buffer() const
{
	return m_buffer;
}

bool Transfer::dataValid() const
{
	return m_isBufferValid;
}

QString Transfer::toString() const
{
	// line format:
	//00 03 00 0b 00 00 90 b8 f5 9f 09 31 31 33 37 38   |;tJ�..........|

	int i = 0;
	QString output = "\n";
	QString hex, ascii;

	QByteArray::ConstIterator it;
	QByteArray::ConstIterator end = m_wireFormat.end();
	for ( it = m_wireFormat.begin(); it != end; ++it )
	{
		i++;

		unsigned char c = static_cast<unsigned char>(*it);

		if(c < 0x10)
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

void Transfer::populateWireBuffer( int offset, const QByteArray& buffer )
{
	int j;
	for ( int i = 0; i < buffer.size(); ++i )
	{
		j = i + offset;
		m_wireFormat[j] = buffer[i];
	}
}


FlapTransfer::FlapTransfer()
	: Transfer()
{
	m_isFlapValid = false;
}

FlapTransfer::FlapTransfer( struct FLAP f, Buffer* buffer )
	: Transfer( buffer )
{
	m_flapChannel = f.channel;
	m_flapSequence = f.sequence;
	m_flapLength = f.length;

	if ( m_flapChannel == 0 || m_flapLength < 6 )
		m_isFlapValid = false;
	else
		m_isFlapValid = true;

}

FlapTransfer::FlapTransfer( Buffer* buffer, Oscar::BYTE chan, Oscar::WORD seq, Oscar::WORD len )
	: Transfer( buffer )
{
	m_flapChannel = chan;
	m_flapSequence = seq;
	m_flapLength = len;

	if ( m_flapChannel == 0 || m_flapLength < 6 )
		m_isFlapValid = false;
	else
		m_isFlapValid = true;
}

FlapTransfer::~FlapTransfer()
{

}

QByteArray FlapTransfer::toWire()
{
	//kDebug(OSCAR_RAW_DEBUG) << "Buffer length is " << m_buffer.length();
	//kDebug(OSCAR_RAW_DEBUG) << "Buffer is " << m_buffer.toString();

	m_wireFormat.truncate( 0 );
	QByteArray useBuf( m_buffer->buffer() );
	m_flapLength = useBuf.size();
	m_wireFormat.resize( 6 + m_flapLength );
	m_wireFormat[0] = 0x2A;
	m_wireFormat[1] = m_flapChannel;
	m_wireFormat[2] = (m_flapSequence & 0xFF00) >> 8;
	m_wireFormat[3] = (m_flapSequence & 0x00FF);
	m_wireFormat[4] = (m_flapLength & 0xFF00) >> 8;
	m_wireFormat[5] = (m_flapLength & 0x00FF);

	//deepcopy the high-level buffer to the wire format buffer
	populateWireBuffer( 6, useBuf );
	QByteArray wire = m_wireFormat;
	return wire;
}

void FlapTransfer::setFlapChannel( Oscar::BYTE channel )
{
	if ( channel != 0 )
	{
		m_flapChannel = channel;
		m_isFlapValid = true;
	}
}


Oscar::BYTE FlapTransfer::flapChannel() const
{
	return m_flapChannel;
}


void FlapTransfer::setFlapSequence( Oscar::WORD seq )
{
	m_flapSequence = seq;
}


Oscar::WORD FlapTransfer::flapSequence() const
{
	return m_flapSequence;
}

void FlapTransfer::setFlapLength( Oscar::WORD len )
{
	m_flapLength = len;
}

Oscar::WORD FlapTransfer::flapLength() const
{
	return m_flapLength;
}

bool FlapTransfer::flapValid() const
{
	return m_isFlapValid;
}

Transfer::TransferType FlapTransfer::type() const
{
	return Transfer::FlapTransfer;
}



SnacTransfer::SnacTransfer()
	: FlapTransfer()
{
	m_isSnacValid = false;
}


SnacTransfer::SnacTransfer( Buffer* buffer, Oscar::BYTE chan, Oscar::WORD seq, Oscar::WORD len, Oscar::WORD service,
				 Oscar::WORD subtype, Oscar::WORD flags, Oscar::DWORD reqId )
	: FlapTransfer( buffer, chan, seq, len )
{
	m_snacService = service;
	m_snacSubtype = subtype;
	m_snacFlags = flags;
	m_snacReqId = reqId;

	if ( m_snacService == 0 || m_snacSubtype == 0 )
		m_isSnacValid = false;
	else
		m_isSnacValid = true;

}

SnacTransfer::SnacTransfer( struct FLAP f, struct SNAC s, Buffer* buffer )
	: FlapTransfer( f, buffer )
{
	m_snacService = s.family;
	m_snacSubtype = s.subtype;
	m_snacFlags = s.flags;
	m_snacReqId = s.id;

	if ( m_snacService == 0 || m_snacSubtype == 0 )
		m_isSnacValid = false;
	else
		m_isSnacValid = true;
}

SnacTransfer::~SnacTransfer()
{

}

QByteArray SnacTransfer::toWire()
{

	m_wireFormat.truncate( 0 );
	QByteArray useBuf( m_buffer->buffer() );
	setFlapLength( useBuf.size() + 10 );
	m_wireFormat.resize( 16 + useBuf.size() );

	//Transfer the flap - 6 bytes
	m_wireFormat[0] = 0x2A;
	m_wireFormat[1] = flapChannel();
	m_wireFormat[2] = (flapSequence() & 0xFF00) >> 8;
	m_wireFormat[3] = (flapSequence() & 0x00FF);
	m_wireFormat[4] = (flapLength() & 0xFF00) >> 8;
	m_wireFormat[5] = (flapLength() & 0x00FF);

	//Transfer the Snac - 10 bytes
	m_wireFormat[6] = (m_snacService & 0xFF00) >> 8;
	m_wireFormat[7] = (m_snacService & 0x00FF);
	m_wireFormat[8] = (m_snacSubtype & 0xFF00) >> 8;
	m_wireFormat[9] = (m_snacSubtype & 0x00FF);
	m_wireFormat[10] = (m_snacFlags & 0xFF00) >> 8;
	m_wireFormat[11] = (m_snacFlags & 0x00FF);
	m_wireFormat[12] = (m_snacReqId & 0xFF000000) >> 24;
	m_wireFormat[13] = (m_snacReqId & 0x00FF0000) >> 16;
	m_wireFormat[14] = (m_snacReqId & 0x0000FF00) >> 8;
	m_wireFormat[15] = (m_snacReqId & 0x000000FF);

	//deepcopy the high-level buffer to the wire format buffer
	populateWireBuffer( 16, useBuf );
	QByteArray wire = m_wireFormat;
	return wire;
}

Transfer::TransferType SnacTransfer::type() const
{
	return Transfer::SnacTransfer;
}

bool SnacTransfer::snacValid() const
{
	return m_isSnacValid;
}

void SnacTransfer::setSnacService( Oscar::WORD service )
{
	m_snacService = service;
}

Oscar::WORD SnacTransfer::snacService() const
{
	return m_snacService;
}

void SnacTransfer::setSnacSubtype( Oscar::WORD subtype )
{
	m_snacSubtype = subtype;
}

Oscar::WORD SnacTransfer::snacSubtype() const
{
	return m_snacSubtype;
}

void SnacTransfer::setSnacFlags( Oscar::WORD flags )
{
	m_snacFlags = flags;
}

Oscar::WORD SnacTransfer::snacFlags() const
{
	return m_snacFlags;
}

void SnacTransfer::setSnacRequest( Oscar::DWORD id )
{
	m_snacReqId = id;
}

Oscar::DWORD SnacTransfer::snacRequest() const
{
	return m_snacReqId;
}

SNAC SnacTransfer::snac() const
{
	SNAC s = { m_snacService, m_snacSubtype, m_snacFlags, m_snacReqId };
	return s;
}


