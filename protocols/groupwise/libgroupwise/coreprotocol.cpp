/*
    Kopete Groupwise Protocol
    coreprotocol.h- the core GroupWise protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
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

#include <string.h>
#include <iostream>
#include <stdlib.h>

#include <qdatastream.h>
#include <qdatetime.h>
#include <q3textstream.h>
#include <QByteArray>


#include <kdebug.h>
#include <kurl.h>

#include "eventprotocol.h"
#include "eventtransfer.h"
#include "gwerror.h"
#include "gwfield.h"
#include "request.h"
#include "response.h"
#include "responseprotocol.h"

#define NO_ESCAPE(ch) ((ch == 0x20) || (ch >= 0x30 && ch <= 0x39) || (ch >= 0x41 && ch <= 0x5a) || (ch >= 0x61 && ch <= 0x7a))
#define GW_URLVAR_TAG "&tag="
#define GW_URLVAR_METHOD "&cmd="
#define GW_URLVAR_VAL "&val="
#define GW_URLVAR_TYPE "&type="

//#define GW_COREPROTOCOL_DEBUG

QByteArray
url_escape_string( const char *src)
{
	uint escape = 0;
	const char *p;
	uint q;
	//char *encoded = NULL;
	int ch;

	static const char hex_table[17] = "0123456789abcdef";

	if (src == NULL) {
		return QByteArray();
	}

	/* Find number of chars to escape */
	for (p = src; *p != '\0'; p++) {
		ch = (uchar) *p;
		if (!NO_ESCAPE(ch)) {
			escape++;
		}
	}

	QByteArray encoded((p - src) + (escape * 2) + 1, 0);

	/* Escape the string */
	for (p = src, q = 0; *p != '\0'; p++) {
		ch = (uchar) * p;
		if (NO_ESCAPE(ch)) {
			if (ch != 0x20) {
				encoded.insert( q, (char)ch );
				q++;
			} else {
				encoded.insert( q, '+' );
				q++;
			}
		} else {
			encoded.insert( q, '%' );
			q++;

			encoded.insert( q, hex_table[ch >> 4] );
			q++;

			encoded.insert( q, hex_table[ch & 15] );
			q++;
		}
	}
	encoded.insert( q, '\0' );

	return encoded;
}

CoreProtocol::CoreProtocol() : QObject()
{
	m_eventProtocol = new EventProtocol( this );
	m_eventProtocol->setObjectName( "eventprotocol" );
	m_responseProtocol = new ResponseProtocol( this );
	m_responseProtocol->setObjectName( "responseprotocol" );
}

CoreProtocol::~CoreProtocol() 
{
}

int CoreProtocol::state()
{
	return m_state;
}

void CoreProtocol::debug( const QString &str )
{
#ifdef LIBGW_USE_KDEBUG
	kDebug() << str;
#else
	qDebug() << "GW RAW PROTO: " << str.toAscii();
#endif
}

void CoreProtocol::addIncomingData( const QByteArray & incomingBytes )
{
// store locally
	debug( QString() );
	int oldsize = m_in.size();
	m_in.resize( oldsize + incomingBytes.size() );
	memcpy( m_in.data() + oldsize, incomingBytes.data(), incomingBytes.size() );
	m_state = Available;
	// convert every event in the chunk to a Transfer, signalling it back to the clientstream
	
	int parsedBytes = 0;
	int transferCount = 0;
	// while there is data left in the input buffer, and we are able to parse something out of it
	while ( m_in.size() && ( parsedBytes = wireToTransfer( m_in ) ) )
	{
		transferCount++;
		debug( QString( "parsed transfer #%1 in chunk" ).arg( transferCount ) );
		int size =  m_in.size();
		if ( parsedBytes < size )
		{
			debug( " - more data in chunk!" );
			// copy the unparsed bytes into a new qbytearray and replace m_in with that
			QByteArray remainder( size - parsedBytes, 0 );
			memcpy( remainder.data(), m_in.data() + parsedBytes, remainder.size() );
			m_in = remainder;
		}
		else
			m_in.truncate( 0 );
	}
	if ( m_state == NeedMore )
		debug( " - message was incomplete, waiting for more..." );
	if ( m_eventProtocol->state() == EventProtocol::OutOfSync )
	{	
		debug( " - protocol thinks it is out of sync, discarding the rest of the buffer and hoping the server regains sync soon..." );
		m_in.truncate( 0 );
	}
	debug( " - done processing chunk" );
}

Transfer* CoreProtocol::incomingTransfer()
{	
	debug( "" );
	if ( m_state == Available )
	{
		debug( " - got a transfer" );
		m_state = NoData;
		return m_inTransfer;
		m_inTransfer = 0;
	}
	else
	{
		debug( " - no milk today." );
		return 0;
	}
}

void cp_dump( const QByteArray &bytes )
{
#ifdef LIBGW_DEBUG
	CoreProtocol::debug( QString( "contains: %1 bytes" ).arg( bytes.count() ) );
	for ( int i = 0; i < bytes.count(); ++i )
	{
		printf( "%02x ", bytes[ i ] );
	}
	printf( "\n" );
#else
	Q_UNUSED(bytes);
#endif
}

void CoreProtocol::outgoingTransfer( Request* outgoing )
{
	debug( "" );
	// Convert the outgoing data into wire format
	Request * request = dynamic_cast<Request *>( outgoing );
	Field::FieldList fields = request->fields();
	if ( fields.isEmpty() )
	{
		debug( "Transfer contained no fields, it must be a ping.");
/*		m_error = NMERR_BAD_PARM;
		return;*/
	}
	// Append field containing transaction id
	Field::SingleField * fld = new Field::SingleField( Field::NM_A_SZ_TRANSACTION_ID, NMFIELD_METHOD_VALID, 
					0, NMFIELD_TYPE_UTF8, request->transactionId() ); 
	fields.append( fld );
	
	// convert to QByteArray
	QByteArray bytesOut;
	QTextStream dout( &bytesOut, QIODevice::WriteOnly );
	dout.setCodec( "ISO 8859-1" );
	//dout.setByteOrder( QDataStream::LittleEndian );

	// strip out any embedded host and port in the command string 
	QByteArray command, host, port;
	if ( request->command().section( ':', 0, 0 ) == "login" )
	{
		command = "login";
		host = request->command().section( ':', 1, 1 ).toAscii();
		port = request->command().section( ':', 2, 2 ).toAscii();
		debug( QString( "Host: %1 Port: %2" ).arg( host.data() ).arg( port.data() ) );
	}
	else
		command = request->command().toAscii();
	
	// add the POST
	dout << "POST /";
	dout << command;
	dout << " HTTP/1.0\r\n";
	
	// if a login, add Host arg
	if ( command == "login" )
	{
		dout << "Host: ";
		dout << host; //FIXME: Get this from somewhere else!!
		dout << ":" << port << "\r\n\r\n";
	}
	else
		dout <<  "\r\n";

	dout.flush();
	debug( QString( "data out: %1" ).arg( bytesOut.data() ) );
	emit outgoingData( bytesOut );
	// now convert 
	fieldsToWire( fields );
	delete request;
	delete fld;
	return;
}

void CoreProtocol::fieldsToWire( Field::FieldList fields, int depth )
{
	debug( "");
	int subFieldCount = 0;
	
	// TODO: consider constructing this as a QStringList and then join()ing it.
	Field::FieldListIterator it;
	Field::FieldListIterator end = fields.end();
	Field::FieldBase* field;
	for ( it = fields.begin(); it != end ; ++it )
	{
		field = *it;
		//debug( " - writing a field" );
		QByteArray bytesOut;
		QDataStream dout( &bytesOut,QIODevice::WriteOnly );
		dout.setVersion(QDataStream::Qt_3_1);
		dout.setByteOrder( QDataStream::LittleEndian );
		
		// these fields are ignored by Gaim's novell
		if ( field->type() == NMFIELD_TYPE_BINARY  || field->method() == NMFIELD_METHOD_IGNORE )
			continue;
			
		// GAIM writes these tags to the secure socket separately - if we can't connect, check here
		// NM Protocol 1 writes them in an apparently arbitrary order
		// tag
		//dout.writeRawBytes( GW_URLVAR_TAG, sizeof( GW_URLVAR_TAG ) );
		//dout <<  field->tag();
		
		// method
		//dout.writeRawBytes( GW_URLVAR_METHOD, sizeof( GW_URLVAR_METHOD ) );
		//		char methodChar = encode_method( field->method() );
		//dout << (quint8)methodChar;
		
		// value
		//dout.writeRawBytes( GW_URLVAR_VAL, sizeof( GW_URLVAR_VAL ) );
		
		char valString[ NMFIELD_MAX_STR_LENGTH ];
		switch ( field->type() )
		{
			case NMFIELD_TYPE_UTF8:		// Field contains UTF-8
			case NMFIELD_TYPE_DN:		// Field contains a Distinguished Name
			{
				//debug( " - it's a single string" );
				const Field::SingleField *sField = static_cast<const Field::SingleField*>( field );
// 				QString encoded = KUrl::encode_string( sField->value().toString(), 106 /* UTF-8 */);
// 				encoded.replace( "%20", "+" );
// 				dout <<  encoded.ascii();

				snprintf( valString, NMFIELD_MAX_STR_LENGTH, "%s", url_escape_string( sField->value().toString().toUtf8() ).data() );
				//dout <<  sField->value().toString().ascii();
				break;
			}
			case NMFIELD_TYPE_ARRAY:	// Field contains a field array
			case NMFIELD_TYPE_MV:		// Field contains a multivalue
			{
				//debug( " - it's a multi" );
				const Field::MultiField *mField = static_cast<const Field::MultiField*>( field );
				subFieldCount = mField->fields().count();	// determines if we have a subarray to send after this field
				//dout <<  QString::number( subFieldCount ).ascii();
				snprintf( valString, NMFIELD_MAX_STR_LENGTH, "%u", subFieldCount );
				break;
			}
			default:					// Field contains a numeric value
			{
				//debug( " - it's a number" );
				const Field::SingleField *sField = static_cast<const Field::SingleField*>( field );
				//dout <<  QString::number( sField->value().toInt() ).ascii();
				snprintf( valString, NMFIELD_MAX_STR_LENGTH, "%u", sField->value().toInt() );
			}
		}
				
		// type
		//dout.writeRawBytes( GW_URLVAR_TYPE, sizeof( GW_URLVAR_TYPE ) );

		//dout << QString::number( field->type() ).ascii();
		QByteArray typeString;
		typeString.setNum( field->type() );
		QByteArray outgoing;
		outgoing.append( GW_URLVAR_TAG );
		outgoing.append( field->tag() );
		outgoing.append( GW_URLVAR_METHOD );
		outgoing.append( encode_method( field->method() ).toLatin1() );
		outgoing.append( GW_URLVAR_VAL );
		outgoing.append( valString );
		outgoing.append( GW_URLVAR_TYPE );
		outgoing.append( typeString );
								
		debug( QString( "outgoing data: %1" ).arg( outgoing.data() ) );
		dout.writeRawData( outgoing.data(), outgoing.length() );
		// write what we have so far, we may be calling this function recursively
		//kDebug( 14999 ) << "writing \'" << bout << "\'";
		//debug( " - signalling data" );
		emit outgoingData( bytesOut );

		// write fields of subarray, if that's what the current field is
		if ( subFieldCount > 0 && 
				( field->type() == NMFIELD_TYPE_ARRAY || field->type() == NMFIELD_TYPE_MV ) )
		{
			const Field::MultiField *mField = static_cast<const Field::MultiField*>( field );
			fieldsToWire( mField->fields(), depth + 1 );
		}
		//debug( " - field done" );
	}
	if ( depth == 0 ) // this call to the function was not recursive, so the entire request has been sent at this point
	{
		// very important, don't send put the \r\n on the wire as a string or it will be preceded with the string length and 0 terminated, which the server reads as a request to disconnect.
		QByteArray bytesOut;
		QDataStream dout( &bytesOut,QIODevice::WriteOnly );
		dout.setVersion(QDataStream::Qt_3_1);
		dout.setByteOrder( QDataStream::LittleEndian );
		dout.writeRawData( "\r\n", 2 );
		emit outgoingData( bytesOut );
		debug( "- request completed" );
	}
	//debug( " - method done" );
}

int CoreProtocol::wireToTransfer( QByteArray& wire )
{
	// processing incoming data and reassembling it into transfers
	// may be an event or a response
	uint bytesParsed = 0;
	m_din = new QDataStream( &wire,QIODevice::ReadOnly );
	m_din->setVersion(QDataStream::Qt_3_1);
	m_din->setByteOrder( QDataStream::LittleEndian );
	
	// look at first four bytes and decide what to do with the chunk
	quint32 val;
	if ( okToProceed() )
	{
		*m_din >> val;

		// if is 'HTTP', it's a Response. PTTH it is after endian conversion
		if ( !qstrncmp( (const char *)&val, "HTTP", strlen( "HTTP" ) )  ||
		     !qstrncmp( (const char *)&val, "PTTH", strlen( "PTTH" ) )
		) {
			Transfer * t = m_responseProtocol->parse( wire, bytesParsed );
			if ( t )
			{
				m_inTransfer = t;
				debug( "- got a RESPONSE " );
				
				m_state = Available;
				emit incomingData();
			}
			else
				bytesParsed = 0;
		}
		else // otherwise -> Event code
		{	
			debug( QString( "- looks like an EVENT: %1, length %2" ).arg( val ).arg( wire.size() ) );
			Transfer * t = m_eventProtocol->parse( wire, bytesParsed );
			if ( t )
			{
				m_inTransfer = t;
				debug( QString( "- got an EVENT: %1, parsed: %2" ).arg( val ).arg( bytesParsed ) );
				m_state = Available;
				emit incomingData();
			}
			else
			{
				debug( "- EventProtocol was unable to parse it" );
				bytesParsed = 0;
			}
		}
	}
	delete m_din;
	return bytesParsed;
}

void CoreProtocol::reset()
{
	m_in.resize( 0 );
}

QLatin1Char CoreProtocol::encode_method( quint8 method )
{
	switch (method)
	{
		case NMFIELD_METHOD_EQUAL:
			return QLatin1Char('G');
			break;
		case NMFIELD_METHOD_UPDATE:
			return QLatin1Char('F');
			break;
		case NMFIELD_METHOD_GTE:
			return QLatin1Char('E');
			break;
		case NMFIELD_METHOD_LTE:
			return QLatin1Char('D');
			break;
		case NMFIELD_METHOD_NE:
			return QLatin1Char('C');
			break;			
		case NMFIELD_METHOD_EXIST:
			return QLatin1Char('B');
			break;
		case NMFIELD_METHOD_NOTEXIST:
			return QLatin1Char('A');
			break;
		case NMFIELD_METHOD_SEARCH:
			return QLatin1Char('9');
			break;
		case NMFIELD_METHOD_MATCHBEGIN:
			return QLatin1Char('8');
			break;
		case NMFIELD_METHOD_MATCHEND:
			return QLatin1Char('7');
			break;
		case NMFIELD_METHOD_NOT_ARRAY:
			return QLatin1Char('6');
			break;
		case NMFIELD_METHOD_OR_ARRAY:
			return QLatin1Char('5');
			break;
		case NMFIELD_METHOD_AND_ARRAY:
			return QLatin1Char('4');
			break;
		case NMFIELD_METHOD_DELETE_ALL:
			return QLatin1Char('3');
			break;
		case NMFIELD_METHOD_DELETE:
			return QLatin1Char('2');
			break;
		case NMFIELD_METHOD_ADD:
			return QLatin1Char('1');
			break;
		default:					/* NMFIEL D_METHOD_VALID */
			return QLatin1Char('0');
			break;
	}
}

void CoreProtocol::slotOutgoingData( const QByteArray &out )
{
	debug( QString( "%1" ).arg( QString::fromAscii( out ) ) );
}

bool CoreProtocol::okToProceed()
{
	if ( m_din )
	{
		if ( m_din->atEnd() )
		{
			m_state = NeedMore;
			debug( "- Server message ended prematurely!" );
		}
		else
			return true;
	}
	return false;
}

#include "coreprotocol.moc"
