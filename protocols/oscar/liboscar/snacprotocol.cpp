/*
    Kopete Oscar Protocol
    snacprotocol.cpp - reads the protocol used by Oscar for signalling stuff

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

#include "snacprotocol.h"

#include <qdatastream.h>
#include <qobject.h>
#include <kdebug.h>
#include <stdlib.h>
#include "transfer.h"


using namespace Oscar;

SnacProtocol::SnacProtocol(QObject *parent)
 : InputProtocolBase(parent)
{
}

SnacProtocol::~SnacProtocol()
{
}

Transfer* SnacProtocol::parse( const QByteArray & packet, uint& bytes )
{
	Oscar::BYTE b;
	Oscar::WORD w;
	Oscar::DWORD dw;

	FLAP f;
	SNAC s;
	SnacTransfer *st;
	QDataStream din( const_cast<QByteArray*>( &packet ), QIODevice::ReadOnly );

	//flap parsing
	din >> b; //this should be the start byte
	//kDebug(OSCAR_RAW_DEBUG) << "start byte is " << b;
	din >> b;
	f.channel = b;
	din >> w;
	f.sequence = w;
	din >> w;
	f.length = w;

	if ( ( f.length + 6 ) > packet.size() )
	{
		kDebug(OSCAR_RAW_DEBUG) << "Packet not big enough to parse!";
		kDebug(OSCAR_RAW_DEBUG) << "packet size is " << packet.size()
			<< " we need " << f.length + 6 << endl;
		return 0;
	}
	//snac parsing
	din >> w;
	s.family = w;
	din >> w;
	s.subtype = w;
	din >> w;
	s.flags = w;
	din >> dw;
	s.id = dw;

	kDebug(OSCAR_RAW_DEBUG) << "family: " << s.family
			<< " subtype: " << s.subtype << " flags: " << s.flags
			<< " id: " << s.id << endl;

	//use pointer arithmatic to skip the flap and snac headers
	//so we don't have to do double parsing in the tasks
	char* charPacket = const_cast<char*>( packet.data() );
	char* snacData;
	int snacOffset = 10; //default
	if ( s.flags >= 0x8000  ) //skip the next 8 bytes, we don't care about the snac version ATM
	{
		//kDebug(OSCAR_RAW_DEBUG) << "skipping snac version";
		snacOffset = 18;
		snacData = charPacket + 24;
	}
	else
	{
		snacOffset = 10;
		snacData = charPacket + 16;
	}

	Buffer *snacBuffer = new Buffer( snacData, f.length - snacOffset  );
	st = new SnacTransfer( f, s, snacBuffer );
	bytes = f.length + 6;
	return st;
}


#include "snacprotocol.moc"
