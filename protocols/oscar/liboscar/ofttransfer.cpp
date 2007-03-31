/*

    Kopete (c) 2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "ofttransfer.h"
//#include <ctype.h>
#include <qtextcodec.h>
#include <kdebug.h>

/*
void Transfer::populateWireBuffer( int offset, const QByteArray& buffer )
{
	int j;
	for ( int i = 0; i < buffer.size(); ++i )
	{
		j = i + offset;
		m_wireFormat[j] = buffer[i];
	} //why? for the love of god why??
}
*/

OftTransfer::OftTransfer()
: Transfer(), m_isOftValid( false )
{
}

OftTransfer::OftTransfer( struct OFT data, Buffer* buffer )
: Transfer( buffer ), m_data( data )
{
	m_isOftValid = true; //FIXME: don't assume
}

OftTransfer::~OftTransfer()
{
}

QByteArray OftTransfer::toWire()
{
	//kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Buffer length is " << m_buffer.length() << endl;

	//get filename length - the only variable length in the OFT
	int fileNameEncoding = 0;
	QByteArray fileName = encodeFileName( m_data.fileName, fileNameEncoding );
	const int fileNameLen = fileName.length();

	Buffer b;
	b.addString( "OFT2" ); //protocol version
	b.addWord( fileNameLen > 63 ? fileNameLen - 63 + 256 : 256 );
	b.addWord( m_data.type );
	b.addString( m_data.cookie );
	b.addDWord( 0 ); //no encryption, no compression
	b.addWord( 1 ); //total files
	b.addWord( 1 ); //files left
	b.addWord( 1 ); //total parts (macs might have 2)
	b.addWord( 1 ); //parts left
	b.addDWord( m_data.fileSize ); //total bytes
	b.addDWord( m_data.fileSize ); // size or 'bytes sent' XXX - documentation must be wrong. I'm guessing this is the size of the current file, usually same as total bytes
	b.addDWord( m_data.modTime );
	b.addDWord( m_data.checksum );
	b.addDWord( 0xFFFF0000 ); //recv'd resource fork checksum (mac thing)
	b.addDWord( 0 ); //resource fork size
	b.addDWord( 0 ); //creation time ( or 0 )
	b.addDWord( 0xFFFF0000 ); //resource fork checksum (mac thing)
	b.addDWord( m_data.bytesSent );
	b.addDWord( m_data.sentChecksum ); //checksum of transmitted bytes
	//'idstring'
	b.addString( "Cool FileXfer" );
	QByteArray zeros;
	zeros.fill( 0, 19 ); //32 - 13 = 19
	b.addString( zeros ); //pad to 32 bytes

	b.addByte( m_data.flags ); //flags; 0x20=not done, 1=done
	b.addByte( 0x1c ); //'name offset'
	b.addByte( 0x11 ); //'size offset'
	zeros.fill( 0, 69 );
	b.addString( zeros ); //dummy block
	zeros.resize( 16 );
	b.addString( zeros ); //mac file info
	b.addWord( fileNameEncoding ); //encoding 0=ascii, 2=UTF-16BE without BOM, 3= ISO-8859-1
	b.addWord( 0 ); //encoding subcode
	b.addString( fileName );
	if ( fileNameLen < 63 )
	{ //minimum length 64
		zeros.fill( 0, 64 - fileNameLen );
		b.addString( zeros );
	}
	else
		b.addByte( 0 ); //always null-terminated string

	//yay! the big bloated header is done.
	m_wireFormat = b.buffer();

	//deepcopy the high-level buffer to the wire format buffer
	//populateWireBuffer( 6, useBuf );
	return m_wireFormat;
}

void OftTransfer::setData( OFT data )
{
	m_data = data;
	m_isOftValid = true;
}

OFT OftTransfer::data() const
{
	return m_data;
}

bool OftTransfer::oftValid() const
{
	return m_isOftValid;
}

Transfer::TransferType OftTransfer::type() const
{
	return Transfer::FileTransfer;
}

QByteArray OftTransfer::encodeFileName( const QString &fileName, int &encodingType ) const
{
	QTextCodec *codec = QTextCodec::codecForName( "ISO 8859-1" );
	if ( codec->canEncode( fileName ) )
	{
		QByteArray data = codec->fromUnicode( fileName ); // write as ISO 8859-1
		for ( int i = 0; i < data.size(); ++i )
		{
			if ( (unsigned char)data.at(i) >= 128 )
			{
				encodingType = 3; // is ISO 8859-1
				return data;
			}
		}
		encodingType = 0; // is US-ASCII
		return data;
	}

	codec = QTextCodec::codecForName( "UTF-16BE" );
	QTextCodec::ConverterState state( QTextCodec::IgnoreHeader );
	encodingType = 2; // is UTF-16BE
	return codec->fromUnicode( fileName.constData(), fileName.size(), &state );
}
