/*
    Kopete Groupwise Protocol
    responseprotocol.cpp - Protocol used for reading incoming GroupWise Responses

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
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

#include "responseprotocol.h"

#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QStringList>

#include "response.h"

ResponseProtocol::ResponseProtocol(QObject* parent)
: InputProtocolBase(parent)
{
}


ResponseProtocol::~ResponseProtocol()
{
}

Transfer * ResponseProtocol::parse( QByteArray & wire, uint & bytes )
{
	m_bytes = 0;
	m_collatingFields.clear();
	//m_din = new QDataStream( wire, QIODevice::ReadOnly );
	QBuffer inBuf( &wire );
	inBuf.open( QIODevice::ReadOnly); 
	m_din.setDevice( &inBuf );
	m_din.setByteOrder( QDataStream::LittleEndian );
	
	// check that this begins with a HTTP (is a response)
	quint32 val;
	m_din >> val;
	m_bytes += sizeof( quint32 );
	
	Q_ASSERT( qstrncmp( (const char *)&val, "HTTP", strlen( "HTTP" ) ) == 0 );
	
	// read rest of HTTP header and look for a 301 redirect. 
	QByteArray headerFirst;
	if ( !readGroupWiseLine( headerFirst ) )
		return 0;
	// pull out the HTTP return code
	int firstSpace = headerFirst.indexOf( ' ' );
	QByteArray rtnField = headerFirst.mid( firstSpace + 1, 3 );
	bool ok = true;
	int rtnCode;
#ifdef __GNUC__
	#warning what to do with packetState?
#endif
	int packetState = -1;
	rtnCode = rtnField.toInt( &ok );
	debug( QString("CoreProtocol::readResponse() got HTTP return code '%1'").arg( rtnCode) );
	// read rest of header
	QStringList headerRest;
	QByteArray line;
	while ( line != "\r\n" )
	{
		if ( !readGroupWiseLine( line ) )
		{
			m_din.unsetDevice();
			return 0;
		}
		headerRest.append( line );
		debug( QString( "- read header line - (%1) : %2" ).arg( line.length() ).arg( line.data() ) );
	}
	debug( "ResponseProtocol::readResponse() header finished" );
	// if it's a redirect, set flag
	if ( ok && rtnCode == 301 )
	{	
		debug( "- server redirect " );
		packetState = ServerRedirect;
		m_din.unsetDevice();
		return 0;
	}
	// other header processing ( 500! )
	if ( ok && rtnCode == 500 )
	{
		debug( QString( "- server error %1" ).arg( rtnCode ) );
		packetState = ServerError;
		m_din.unsetDevice();
		return 0;
	}
	if ( ok && rtnCode == 404 )
	{
		debug( QString( "- server error %1" ).arg( rtnCode ) );
		packetState = ServerError;
		m_din.unsetDevice();
		return 0;
	}
	if ( m_din.atEnd() )
	{
		debug( "- no fields" );
		packetState = ProtocolError;
		m_din.unsetDevice();
		return 0;
	}
	
	// read fields
	if ( !readFields( -1 ) )
	{
		m_din.unsetDevice();
		return 0;
	}
	// find transaction id field and create Response object if nonzero
	int tId = 0;
	int resultCode = -1;
	Field::FieldListIterator it;
	Field::FieldListIterator end = m_collatingFields.end();
	it = m_collatingFields.find( Field::NM_A_SZ_TRANSACTION_ID );
	if ( it != end )
	{
		Field::SingleField * sf = dynamic_cast<Field::SingleField*>( *it );
		if ( sf )
		{
			tId = sf->value().toInt();
			debug( QString( "ResponseProtocol::readResponse() - transaction ID is %1" ).arg( tId ) );
			m_collatingFields.erase( it );
			delete sf;
		}
	}
	it = m_collatingFields.find( Field::NM_A_SZ_RESULT_CODE );
	if ( it != end )
	{
		Field::SingleField * sf = dynamic_cast<Field::SingleField*>( *it );
		if ( sf )
		{
			resultCode = sf->value().toInt();
			debug( QString( "ResponseProtocol::readResponse() - result code is %1" ).arg( resultCode ) );
			m_collatingFields.erase( it );
			delete sf;
		}
	}
	// append to inQueue
	if ( tId )
	{
		debug( QString( "ResponseProtocol::readResponse() - setting state Available, got %1 fields in base array" ).arg(m_collatingFields.count() ) );
		packetState = Available;
		bytes = m_bytes;
		m_din.unsetDevice();
		return new Response( tId, resultCode, m_collatingFields );
	}
	else
	{
		debug( "- WARNING - NO TRANSACTION ID FOUND!" );
		if ( resultCode == -1 ) {
			debug( "- WARNING - NO RESULT CODE FOUND!" );
		}
		m_state = ProtocolError;
		m_din.unsetDevice();
		m_collatingFields.purge();
		return 0;
	}
}

bool ResponseProtocol::readFields( int fieldCount, Field::FieldList * list )
{
	// build a list of fields.  
	// If there is already a list of fields stored in m_collatingFields, 
	// the list we're reading on this iteration must be a nested list
	// so when we're done reading it, add it to the MultiList element
	// that is the last element in the top list in m_collatingFields.
	// if we find the beginning of a new nested list, push the current list onto m_collatingFields
	debug("");
	if ( fieldCount > 0 )
		debug( QString( "reading %1 fields" ).arg( fieldCount ) );
	Field::FieldList currentList;
	while ( fieldCount != 0 )  // prevents bad input data from ruining our day
	{
		// the field being read
		// read field
		quint8 type, method;
		quint32 val;
		QByteArray tag;
		// read uint8 type
		if ( !okToProceed() )
		{
			currentList.purge();
			return false;
		}
		m_din >> type;
		m_bytes += sizeof( quint8 );
		// if type is 0 SOMETHING_INVALID, we're at the end of the fields
		if ( type == 0 ) /*&& m_din->atEnd() )*/
		{
			debug( "- end of field list" );
			m_packetState = FieldsRead;
			// do something to indicate we're done
			break;
		}
		// read uint8 method
		if ( !okToProceed() )
		{
			currentList.purge();
			return false;
		}
		m_din >> method;
		m_bytes += sizeof( quint8 );
		// read tag and length
		if ( !safeReadBytes( tag, val ) )
		{
			currentList.purge();
			return false;
		}

		debug( QString( "- type: %1, method: %2, tag: %3," ).arg( type ).arg( method ).arg( tag.data() ) );
		// if multivalue or array
		if ( type == NMFIELD_TYPE_MV || type == NMFIELD_TYPE_ARRAY )
		{
			// read length uint32
			if ( !okToProceed() )
			{
				currentList.purge();
				return false;
			}
			m_din >> val;
			m_bytes += sizeof( quint32 );

			// create multifield
			debug( QString( " multi field containing: %1" ).arg( val ) );
			Field::MultiField* m = new Field::MultiField( tag.constData(), method, 0, type );
			currentList.append( m );
			if ( !readFields( val, &currentList) )
			{
				currentList.purge();
				return false;
			}
		}
		else
		{
		
			if ( type == NMFIELD_TYPE_UTF8 || type == NMFIELD_TYPE_DN )
			{
				QByteArray rawData;
				if( !safeReadBytes( rawData, val ) )
				{
					currentList.purge();
					return false;
				}
				if ( val > NMFIELD_MAX_STR_LENGTH )
				{
					m_packetState = ProtocolError;
					break;
				}
				// convert to unicode - ignore the terminating NUL, because Qt<3.3.2 doesn't sanity check val.
				QString fieldValue = QString::fromUtf8( rawData.data(), val - 1 );
				debug( QString( "- utf/dn single field: %1" ).arg( fieldValue ) );
				// create singlefield
				Field::SingleField* s = new Field::SingleField( tag.constData(), method, 0, type, fieldValue );
				currentList.append( s );
			}
			else
			{
				// otherwise ( numeric )
				// read value uint32
				if ( !okToProceed() )
				{
					currentList.purge();
					return false;
				}
				m_din >> val;
				m_bytes += sizeof( quint32 );
				debug( QString( "- numeric field: %1" ).arg( val ) );
				Field::SingleField* s = new Field::SingleField( tag.constData(), method, 0, type, val );
				currentList.append( s );
			}
		}
		// decrease the fieldCount if we're using it
		if ( fieldCount > 0 )
			fieldCount--;
	}
	// got a whole list!
	// if fieldCount == 0, we've just read a whole nested list, so add this list to the last element in 'list'
	if ( fieldCount == 0 && list )
	{
		debug( "- finished reading nested list" );
		Field::MultiField * m = dynamic_cast<Field::MultiField*>( list->last() );
		m->setFields( currentList );
	}

	// if fieldCount == -1; we're done reading the top level fieldlist, so store it.
	if ( fieldCount == -1 )
	{
		debug( "- finished reading ALL FIELDS!" );
		m_collatingFields = currentList;
	}
	return true;
}

bool ResponseProtocol::readGroupWiseLine( QByteArray & line )
{
	line = QByteArray();
	while ( true )
	{
		quint8 c = 0;
		
		if (! okToProceed() )
			return false;
		m_din >> c;
		m_bytes++;
		line.append( c );
		if ( c == '\n' )
			break;
	}
	return true;
}

#include "responseprotocol.moc"
