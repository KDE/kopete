/*
    Kopete Oscar Protocol
    oftprotocol.cpp - reads the protocol used for oscar filetransfers

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

#include "oftprotocol.h"

#include <qtextcodec.h>
#include <qdatastream.h>
#include <qobject.h>
#include <kdebug.h>

#include "ofttransfer.h"

using namespace Oscar;

OftProtocol::OftProtocol(QObject *parent)
 : InputProtocolBase(parent)
{
}

OftProtocol::~OftProtocol()
{
}

Transfer* OftProtocol::parse( const QByteArray & packet, uint& bytes )
{
	QDataStream m_din( const_cast<QByteArray*>( &packet ), QIODevice::ReadOnly );

	Oscar::BYTE b;
	Oscar::WORD w;
	Oscar::DWORD d;

	OFT data;
	//first 4 bytes should be "OFT2"
	//TODO: find a nicer way of checking this
	m_din >> b;
	if( b != 'O' )
		return 0;
	m_din >> b;
	if( b != 'F' )
		return 0;
	m_din >> b;
	if( b != 'T' )
		return 0;
	m_din >> b;
	if( b != '2' )
		return 0;

	m_din >> w;
	int length = w;
	m_din >> w;
	data.type = w;
	//next 8 bytes are cookie
	char cookie[8];
	m_din.readRawData( cookie, 8 );
	data.cookie = QByteArray( cookie, 8 );
	m_din >> w;
	if( w != 0 )
		kWarning(OSCAR_RAW_DEBUG) << "other side wants encryption";
	m_din >> w;
	if( w != 0 )
		kWarning(OSCAR_RAW_DEBUG) << "other side wants compression";
	m_din >> w;
	if( w > 1 )
		kWarning(OSCAR_RAW_DEBUG) << "more than one file to send";
	data.fileCount = w;

	m_din >> w;
	data.filesLeft = w;
	m_din >> w;
	data.partCount = w;
	m_din >> w;
	data.partsLeft = w;

	m_din >> d;
	data.totalSize = d;
	m_din >> d;
	data.fileSize = d;
	m_din >> d;
	data.modTime = d;
	m_din >> d;
	data.checksum = d;
	m_din.skipRawData( 16 ); //resource recv'd checksum, resource size, creation time, resource checksum
	m_din >> d;
	data.bytesSent = d;
	m_din >> d;
	data.sentChecksum = d;
	m_din.skipRawData( 32 ); //idstring
	m_din >> b;
	data.flags = b;
	m_din.skipRawData( 87 ); //name offset, size offset, dummy block, mac info
	m_din >> d;
	int namelen = length - 256 +  64;
	QByteArray name;
	name.resize( namelen );
	m_din.readRawData( name.data(), namelen );

	QTextCodec *c=0;
	switch ( d )
	{
		case 0:
			c=QTextCodec::codecForName( "UTF8" );
			break;
		case 0x00020000:
			c=QTextCodec::codecForName( "UTF-16BE" );
			break;
		case 0x00030000:
			c=QTextCodec::codecForName( "ISO-8859-1" );
			break;
		default:
			kWarning(OSCAR_RAW_DEBUG) << "unknown codec: " << d;
	}
	if ( c )
		data.fileName = c->toUnicode( name );
	else
	{
		kWarning(OSCAR_RAW_DEBUG) << "couldn't find codec!!!!!! " << d;
		data.fileName = name; //pretend it's just ascii
	}

	const int firstNull = data.fileName.indexOf( QChar() );
	if ( firstNull != -1 )
		data.fileName.truncate( firstNull );

	kDebug(OSCAR_RAW_DEBUG) << "got OFT";

	//if there's more data, skip the oft header
	//so we don't have to do double parsing in the tasks
	Buffer *fileBuffer = 0;
	if ( packet.length() > length )
		fileBuffer = new Buffer( packet.mid( length ) );

	OftTransfer* ft = new OftTransfer( data, fileBuffer );
	bytes = packet.length();
	return ft;
}


#include "oftprotocol.moc"
