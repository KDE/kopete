/*
    Kopete Oscar Protocol
    eventprotocol.cpp - reads the protocol used by Oscar for BOS connections

    Copyright (c) 2004 Matt Rogers <matt.rogers@kdemail.net>
    Based on code Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>

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

#include <qcstring.h>
#include <qdatastream.h>
#include <qobject.h>

#include "ymsgprotocol.h"
#include "ymsgtransfer.h"


YMSGProtocol::YMSGProtocol(QObject *parent, const char *name)
 : InputProtocolBase(parent, name)
{
}

YMSGProtocol::~YMSGProtocol()
{
}

Transfer* YMSGProtocol::parse( const QByteArray & packet, uint& bytes )
{
	QDataStream* m_din = new QDataStream( packet, IO_ReadOnly );
	
	//BYTE b;
	//WORD w;
	//DWORD dw;
	
	YMSGTransfer *t = new YMSGTransfer();
	//*m_din >> b; //this should be the start byte
	//qDebug( "read: %u", b );
	//*m_din >> b;
	/*
	t->setFlapChannel( b );
	*m_din >> w;
	t->setFlapSequence( w );
	*m_din >> w;
	t->setFlapLength( w );
	*m_din >> w;
	t->setSnacService( w );
	*m_din >> w;
	t->setSnacSubtype( w );
	*m_din >> w;
	t->setSnacFlags( w );
	*m_din >> dw;
	t->setSnacRequest( dw );
	*/

	//use pointer arithmatic to skip the flap and snac headers
	//so we don't have to do double parsing in the tasks
	//char* charPacket = packet.data();
	//char* snacData = charPacket + 16;
	/* TODO: Write the buffer class
	Buffer snacBuffer( snacData, ( t->flapLength() - 10 ) );
	t->setBuffer( Buffer );
	*/
	
	return t;
}


#include "ymsgprotocol.moc"
