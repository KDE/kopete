/*
    Kopete Oscar Protocol
    flapprotocol.cpp - reads the protocol used by Oscar for signaling stuff

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

#include "flapprotocol.h"

#include <qdatastream.h>
#include <qobject.h>
#include <kdebug.h>

#include "transfer.h"

using namespace Oscar;

FlapProtocol::FlapProtocol(QObject *parent)
 : InputProtocolBase(parent)
{
}

FlapProtocol::~FlapProtocol()
{
}

Transfer* FlapProtocol::parse( const QByteArray & packet, uint& bytes )
{
	QDataStream* m_din = new QDataStream( const_cast<QByteArray*>( &packet ), QIODevice::ReadOnly );

	Oscar::BYTE b;
	Oscar::WORD w;

	FLAP f;
	*m_din >> b; //this should be the start byte
	*m_din >> b;
	f.channel = b;
	*m_din >> w;
	f.sequence = w;
	*m_din >> w;
	f.length = w;

	kDebug(OSCAR_RAW_DEBUG) << "channel: " << f.channel
			<< " sequence: " << f.sequence << " length: " << f.length << endl;
	//use pointer arithmatic to skip the flap and snac headers
	//so we don't have to do double parsing in the tasks
	char* charPacket = const_cast<char*>( packet.data() );
	char* snacData = charPacket + 6;
	Buffer *snacBuffer = new Buffer( snacData, f.length );

	FlapTransfer* ft = new FlapTransfer( f, snacBuffer );
	bytes = snacBuffer->length() + 6;
	delete m_din;
	m_din = 0;
	return ft;
}


#include "flapprotocol.moc"
