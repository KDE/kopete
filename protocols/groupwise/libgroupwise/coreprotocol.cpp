// url_escape_string taken directly from gaim

#include <string.h>
#include <iostream.h>

#include <qdatastream.h>
#include <qtextstream.h>

#include <qdatetime.h>

#include <kdebug.h>
#include <kurl.h>

#include "eventtransfer.h"
#include "gwerror.h"
#include "gwfield.h"
#include "request.h"
#include "response.h"

#include "coreprotocol.h"

#define NO_ESCAPE(ch) ((ch == 0x20) || (ch >= 0x30 && ch <= 0x39) || \
					(ch >= 0x41 && ch <= 0x5a) || (ch >= 0x61 && ch <= 0x7a))
#define GW_URLVAR_TAG "&tag="
#define GW_URLVAR_METHOD "&cmd="
#define GW_URLVAR_VAL "&val="
#define GW_URLVAR_TYPE "&type="

static char *
url_escape_string( const char *src)
{
	uint escape = 0;
	const char *p;
	char *q;
	char *encoded = NULL;
	int ch;

	static const char hex_table[17] = "0123456789abcdef";

	if (src == NULL) {
		return NULL;
	}

	/* Find number of chars to escape */
	for (p = src; *p != '\0'; p++) {
		ch = (uchar) *p;
		if (!NO_ESCAPE(ch)) {
			escape++;
		}
	}

	encoded = (char*)malloc((p - src) + (escape * 2) + 1);

	/* Escape the string */
	for (p = src, q = encoded; *p != '\0'; p++) {
		ch = (uchar) * p;
		if (NO_ESCAPE(ch)) {
			if (ch != 0x20) {
				*q = ch;
				q++;
			} else {
				*q = '+';
				q++;
			}
		} else {
			*q = '%';
			q++;

			*q = hex_table[ch >> 4];
			q++;

			*q = hex_table[ch & 15];
			q++;
		}
	}
	*q = '\0';

	return encoded;
}

CoreProtocol::CoreProtocol() : QObject()
{
	// debug
	// connect( this, SIGNAL( outgoingData( const QCString & )), SLOT( slotOutgoingData( const QCString & ) ) );
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
	m_in.resize( oldsize + incomingBytes.size() );
	memcpy( m_in.data() + oldsize, incomingBytes.data(), incomingBytes.size() );
	m_state = Available;
	// convert to a Transfer
	if ( wireToTransfer( m_in ) )  // if we've received or assembled a complete message, we can discard this now
		m_in = QByteArray();
}

Transfer* CoreProtocol::incomingTransfer()
{	
	qDebug( "CoreProtocol::incomingTransfer()" );
	if ( m_state == Available )
	{
		qDebug( " - got a transfer" );
		m_state = NoData;
		return m_inTransfer;
		m_inTransfer = 0;
	}
	else
	{
		qDebug( " - no milk today." );
		return 0;
	}
}

void cp_dump( const QByteArray &bytes )
{
	qDebug( "contains: %i bytes", bytes.count() );
	for ( uint i = 0; i < bytes.count(); ++i )
	{
		printf( "%02x ", bytes[ i ] );
	}
	printf( "\n" );
}

bool CoreProtocol::okToProceed()
{
	if ( m_din )
	{
		if ( m_din->atEnd() )
		{
			m_state = NeedMore;
			qDebug( "CoreProtocol::okToProceed() - Server message ended prematurely!" );
		}
		else
			return true;
	}
	return false;
}

bool CoreProtocol::readGroupWiseLine( QCString & line )
{
	line = QCString();
	while ( true )
	{
		Q_UINT8 c;
		
		if (! okToProceed() )
			return false;
		*m_din >> c;
		line += QChar(c);
		if ( c == '\n' )
			break;
	}
	return true;	
}

bool CoreProtocol::safeReadBytes( QCString & data, uint & len )
{
	// read the length of the bytes
	Q_UINT32 val;
	if ( !okToProceed() )
		return false;
	*m_din >> val;
	if ( !okToProceed() )
		return false;
	QCString temp( val );
	// if the server splits packets here we are in trouble,
	// as there is no way to see how much data was actually read
	m_din->readRawBytes( temp.data(), val );
	// the rest of the string will be filled with FF,
	// so look for that in the last position instead of \0
	if ( (Q_UINT8)( * ( temp.data() + ( temp.length() - 1 ) ) ) == 0xFF )
	{
		cout << "CoreProtocol::safeReadBytes() - string broke, giving up" << endl;
		m_state = NeedMore;
		return false;
	}
	data = temp;
	len = val;
	return true;
}

void CoreProtocol::outgoingTransfer( Request* outgoing )
{
	cout << "CoreProtocol::outgoingTransfer()" << endl;
	// Convert the outgoing data into wire format
	Request * request = dynamic_cast<Request *>( outgoing );
	Field::FieldList fields = request->fields();
	if ( fields.isEmpty() )
	{
		cout << " CoreProtocol::outgoingTransfer: Transfer contained no fields, it must be a ping." << endl;
/*		m_error = NMERR_BAD_PARM;
		return;*/
	}
	// Append field containing transaction id
	fields.append( new Field::SingleField( NM_A_SZ_TRANSACTION_ID, NMFIELD_METHOD_VALID, 
					0, NMFIELD_TYPE_UTF8, request->transactionId() ) );
	
	// convert to QByteArray
	QByteArray bytesOut;
	QTextStream dout( bytesOut, IO_WriteOnly );
	dout.setEncoding( QTextStream::Latin1 );
	//dout.setByteOrder( QDataStream::LittleEndian );

	// strip out any embedded host and port in the command string 
	QCString command, host, port;
	if ( request->command().section( ':', 0, 0 ) == "login" )
	{
		command = "login";
		host = request->command().section( ':', 1, 1 ).ascii();
		port = request->command().section( ':', 2, 2 ).ascii();
		qDebug( "Host: %s Port: %s", host.data(), port.data() );
	}
	else
		command = request->command().ascii();
	
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
	
		
	printf( "data out: %s", bytesOut.data() );
	
	emit outgoingData( bytesOut );
	// now convert 
	fieldsToWire( fields );
	return;
}

void CoreProtocol::fieldsToWire( Field::FieldList fields, int depth )
{
	cout << "CoreProtocol::fieldsToWire" << endl;
	int subFieldCount = 0;
	
	// TODO: consider constructing this as a QStringList and then join()ing it.
	Field::FieldListIterator it;
	Field::FieldListIterator end = fields.end();
	Field::FieldBase* field;
	for ( it = fields.begin(); it != end ; ++it )
	{
		field = *it;
		//cout << " - writing a field" << endl;
		QByteArray bytesOut;
		QDataStream dout( bytesOut, IO_WriteOnly );
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
		//dout << (Q_UINT8)methodChar;
		
		// value
		//dout.writeRawBytes( GW_URLVAR_VAL, sizeof( GW_URLVAR_VAL ) );
		
		char valString[ NMFIELD_MAX_STR_LENGTH ];
		switch ( field->type() )
		{
			case NMFIELD_TYPE_UTF8:		// Field contains UTF-8
			case NMFIELD_TYPE_DN:		// Field contains a Distinguished Name
			{
				//cout << " - it's a single string" << endl;
				const Field::SingleField *sField = static_cast<const Field::SingleField*>( field );
// 				QString encoded = KURL::encode_string( sField->value().toString(), 106 /* UTF-8 */);
// 				encoded.replace( "%20", "+" );
// 				dout <<  encoded.ascii();

				snprintf( valString, NMFIELD_MAX_STR_LENGTH, "%s", url_escape_string( sField->value().toString().utf8() ) );
				//dout <<  sField->value().toString().ascii();
				break;
			}
			case NMFIELD_TYPE_ARRAY:	// Field contains a field array
			case NMFIELD_TYPE_MV:		// Field contains a multivalue
			{
				//cout << " - it's a multi" << endl;
				const Field::MultiField *mField = static_cast<const Field::MultiField*>( field );
				subFieldCount = mField->fields().count();	// determines if we have a subarray to send after this field
				//dout <<  QString::number( subFieldCount ).ascii();
				snprintf( valString, NMFIELD_MAX_STR_LENGTH, "%u", subFieldCount );
				break;
			}
			default:					// Field contains a numeric value
			{
				//cout << " - it's a number" << endl;
				const Field::SingleField *sField = static_cast<const Field::SingleField*>( field );
				//dout <<  QString::number( sField->value().toInt() ).ascii();
				snprintf( valString, NMFIELD_MAX_STR_LENGTH, "%u", sField->value().toInt() );
			}
		}
				
		// type
		//dout.writeRawBytes( GW_URLVAR_TYPE, sizeof( GW_URLVAR_TYPE ) );

		//dout << QString::number( field->type() ).ascii();
		QCString typeString;
		typeString.setNum( field->type() );
		QCString outgoing = GW_URLVAR_TAG + field->tag() 
								+ GW_URLVAR_METHOD + (char)encode_method( field->method() ) 
								+ GW_URLVAR_VAL + (const char *)valString 
								+ GW_URLVAR_TYPE + typeString;
								
		cout << "CoreProtocol::fieldsToWire - outgoing data: " << outgoing.data() << endl;
		dout.writeRawBytes( outgoing.data(), outgoing.length() );
		// write what we have so far, we may be calling this function recursively
		//kdDebug( 14999 ) << k_funcinfo << "writing \'" << bout << "\'" << endl;
		//cout << " - signalling data" << endl;
		emit outgoingData( bytesOut );

		// write fields of subarray, if that's what the current field is
		if ( subFieldCount > 0 && 
				( field->type() == NMFIELD_TYPE_ARRAY || field->type() == NMFIELD_TYPE_MV ) )
		{
			const Field::MultiField *mField = static_cast<const Field::MultiField*>( field );
			fieldsToWire( mField->fields(), depth + 1 );
		}
		//cout << " - field done" << endl;
	}
	if ( depth == 0 ) // this call to the function was not recursive, so the entire request has been sent at this point
	{
		// very important, don't send put the \r\n on the wire as a string or it will be preceded with the string length and 0 terminated, which the server reads as a request to disconnect.
		QByteArray bytesOut;
		QDataStream dout( bytesOut, IO_WriteOnly );
		dout.setByteOrder( QDataStream::LittleEndian );
		dout.writeRawBytes( "\r\n", 2 );
		emit outgoingData( bytesOut );
		cout << "CoreProtocol::fieldsToWire - request completed" << endl;
	}
	//cout << " - method done" << endl;
}

bool CoreProtocol::wireToTransfer( const QByteArray& wire )
{
	// processing incoming data and reassembling it into transfers
	// may be an event or a response
	bool ok = true;
	m_din = new QDataStream( wire, IO_ReadOnly );
	m_din->setByteOrder( QDataStream::LittleEndian );
	
	// does protocol state indicate we are partially through reading an Event?
	// if so, call readEvent which will create an EventTransfer out of the event code already received 
	// and the event data that is on the wire
	// otherwise, examine the data to see what it is
	// look at first four bytes
	Q_UINT32 val;

	ok = okToProceed();
	if ( ok )
	{
		*m_din >> val;

		// is 'HTTP' -> response
		if ( qstrncmp( (const char *)&val, "HTTP", strlen( "HTTP" ) ) == 0 )
		{
			cout << "CoreProtocol::wireToTransfer() - looks like a RESPONSE " << endl;
			if ( ( ok = readResponse() ) )
			{
				cout << "CoreProtocol::wireToTransfer() - got a RESPONSE " << endl;
				m_state = Available;
				emit incomingData();
			}
		}
		else	
		{	// otherwise -> event code, store it and await the rest of the event
			qDebug( "CoreProtocol::wireToTransfer() - looks like an EVENT: %i\n", val );
			m_collatingEvent = val;
			if ( ( ok = readEvent( wire, sizeof( Q_UINT32 ) ) ) )
			{
				m_state = Available;
				emit incomingData();
			}
		}
	}
	delete m_din;
	return ok;
}

bool CoreProtocol::readEvent( const QByteArray& wire, int bytesRead )
{
	// wire == m_din at this point
	qDebug( "Reading event of type %i", m_collatingEvent );
	QString source;
	Q_UINT32 len;
	QCString rawData;
	if ( !safeReadBytes( rawData, len ) )
		return false;
	source = QString::fromUtf8( rawData.data(), len );
	bytesRead = bytesRead + sizeof( Q_UINT32 ) + len;
	// now create an event object, passing it the wire data minus the source we just read
	QByteArray remainder( wire.size() - bytesRead );
	memcpy( remainder.data(), wire.data() + bytesRead, wire.size() - bytesRead );
	//HACK: lowercased DN
	m_inTransfer = new EventTransfer( m_collatingEvent, source.lower(), QDateTime::currentDateTime(), remainder );
	m_collatingEvent = 0;
	return true;
}

bool CoreProtocol::readResponse()
{
	// read rest of HTTP header and look for a 301 redirect. 
	QCString headerFirst;
	if ( !readGroupWiseLine( headerFirst ) )
		return false;
	// pull out the HTTP return code
	int firstSpace = headerFirst.find( ' ' );
	QString rtnField = headerFirst.mid( firstSpace, headerFirst.find( ' ', firstSpace + 1 ) );
	bool ok = true;
	int rtnCode;
	int packetState = -1;
	rtnCode = rtnField.toInt( &ok );
	cout << "CoreProtocol::readResponse() got HTTP return code " << rtnCode << endl;
	// read rest of header
	QStringList headerRest;
	QCString line;
	int safetyCheck = 0; // sanity check in case the server is a babbling idiot
	while ( line != "\r\n" && safetyCheck < 1000000 ) // FIXME REMOVE SANITY CHECK AS WE ARE CHECKING ATEND ON EVERY READ
	{
		if ( !readGroupWiseLine( line ) )
			return false;
		headerRest.append( line );
		cout << "- read header line " << safetyCheck << " (" << line.length() <<"):" << line.data();
		safetyCheck++;
	}
	cout << "CoreProtocol::readResponse() header finished" << endl;
	// if it's a redirect, set flag
	if ( ok && rtnCode == 301 )
	{	
		cout << "- server redirect " << rtnCode << endl;
		packetState = ServerRedirect;
		return false;
	}
	// other header processing ( 500! )
	if ( ok && rtnCode == 500 )
	{
		cout << "- server error" << rtnCode << endl;
		packetState = ServerError;
		return false;
	}
	if ( m_din->atEnd() )
	{
		cout << "- no fields" << endl;
		packetState = ProtocolError;
		return false;
	}
	
	// read fields
	if ( !readFields( -1 ) )
		return false;
	// find transaction id field and create Response object if nonzero
	int tId = 0;
	Field::FieldListIterator it;
	Field::FieldListIterator end = m_collatingFields.end();
	Field::FieldBase* field;
	it = m_collatingFields.find( NM_A_SZ_TRANSACTION_ID );
	if ( it != end )
	{
		Field::SingleField * sf = dynamic_cast<Field::SingleField*>( *it );
		if ( sf )
		{
			tId = sf->value().toInt();
			qDebug( "CoreProtocol::readResponse() - transaction ID is %i", tId );
			m_collatingFields.remove( it );
		}
	}
	// append to inQueue
	if ( tId )
	{
		qDebug( "CoreProtocol::readResponse() - setting state Available" );
		m_inTransfer = new Response( tId, m_collatingFields );
		m_collatingFields.clear();
		packetState = Available;
	}
	else
		qDebug( "- WARNING - NO TRANSACTION ID FOUND!" );
	// TODO - handle broken responses..
	// didn't find an 0 - Response is in multiple packets...
		// Gaim doesn't handle this case yet
		// Reconstruction - if we are just reading top level fields, read next packet until 0
		// if we were reading a subarray, we know how long it should be, 
		// store the partial request, partial MultiField and count of total number of fields in the protocol to await next packet
		
	return ( packetState == Available );
}

bool CoreProtocol::readFields( int fieldCount, Field::FieldList * list )
{
	// build a list of fields.  
	// If there is already a list of fields stored in m_collatingFields, 
	// the list we're reading on this iteration must be a nested list
	// so when we're done reading it, add it to the MultiList element
	// that is the last element in the top list in m_collatingFields.
	// if we find the beginning of a new nested list, push the current list onto m_collatingFields
	cout << "CoreProtocol::readFields()" << endl;
	if ( fieldCount > 0 )
		cout << " reading " << fieldCount << "fields" << endl;
	Field::FieldList currentList;
	int safetyCheck = 0;
	while ( fieldCount != 0 && safetyCheck++ < 1000 )  // prevents bad input data from ruining our day
	{
		cout << fieldCount << " fields left to read" << endl;
		// the field being read
		// read field
		Q_UINT8 type, method;
		Q_UINT32 val;
		QCString tag;
		// read uint8 type
		if ( !okToProceed() )
			return false;
		*m_din >> type;
		// if type is 0 SOMETHING_INVALID, we're at the end of the fields
		if ( type == 0 ) /*&& m_din->atEnd() )*/
		{
			cout << "- end of field list" << endl;
/*			*m_din >> type;
			printf( "next byte is %02x", type );
			*m_din >> type;
			printf( "next byte is %02x", type );*/
			m_packetState = FieldsRead;
			// do something to indicate we're done
			break;
		}
		// read uint8 method
		if ( !okToProceed() )
			return false;
		*m_din >> method;
		// read tag and length
		if ( !safeReadBytes( tag, val ) )
			return false;
			
		qDebug( "- type: %i, method: %i, tag: %s,", type, method, tag.data() );
		// if multivalue or array
		if ( type == NMFIELD_TYPE_MV || type == NMFIELD_TYPE_ARRAY )
		{
			// read length uint32
			if ( !okToProceed() )
				return false;
			*m_din >> val;
			// create multifield
			qDebug( " multi field containing: %i\n", val );
			Field::MultiField* m = new Field::MultiField( tag, method, 0, type );
			currentList.append( m );
			if ( !readFields( val, &currentList) )
				return false;
		}
		else 
		{
		
			if ( type == NMFIELD_TYPE_UTF8 || type == NMFIELD_TYPE_DN )
			{
				QCString rawData;
				if( !safeReadBytes( rawData, val ) )
					return false;
				if ( val > NMFIELD_MAX_STR_LENGTH )
				{
					m_packetState = ProtocolError;
					break;
				}
				// convert to unicode
				QString fieldValue = QString::fromUtf8( rawData.data(), val );
				qDebug("- utf/dn single field: %s", fieldValue.ascii() );
				// create singlefield
				Field::SingleField* s = new Field::SingleField( tag, method, 0, type, fieldValue );
				currentList.append( s );
			}
			else
			{
				// otherwise ( numeric )
				// read value uint32
				if ( !okToProceed() )
					return false;
				*m_din >> val;
				qDebug( "- numeric field: %i\n", val );
				Field::SingleField* s = new Field::SingleField( tag, method, 0, type, val );
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
		cout << "- finished reading nested list" << endl;
		Field::MultiField * m = dynamic_cast<Field::MultiField*>( list->last() );
		m->setFields( currentList );
	}

	// if fieldCount == -1; we're done reading the top level fieldlist, so store it.
	if ( fieldCount == -1 )
	{
		cout << "- finished reading ALL FIELDS!" << endl;
		m_collatingFields = currentList;
	}
	return true;
}

void CoreProtocol::reset()
{
	m_in.resize( 0 );
}

QChar CoreProtocol::encode_method( Q_UINT8 method )
{
	QChar str;

	switch (method) {
		case NMFIELD_METHOD_EQUAL:
			str = 'G';
			break;
		case NMFIELD_METHOD_UPDATE:
			str = 'F';
			break;
		case NMFIELD_METHOD_GTE:
			str = 'E';
			break;
		case NMFIELD_METHOD_LTE:
			str = 'D';
			break;
		case NMFIELD_METHOD_NE:
			str = 'C';
			break;			
		case NMFIELD_METHOD_EXIST:
			str = 'B';
			break;
		case NMFIELD_METHOD_NOTEXIST:
			str = 'A';
			break;
		case NMFIELD_METHOD_SEARCH:
			str = '9';
			break;
		case NMFIELD_METHOD_MATCHBEGIN:
			str = '8';
			break;
		case NMFIELD_METHOD_MATCHEND:
			str = '7';
			break;
		case NMFIELD_METHOD_NOT_ARRAY:
			str = '6';
			break;
		case NMFIELD_METHOD_OR_ARRAY:
			str = '5';
			break;
		case NMFIELD_METHOD_AND_ARRAY:
			str = '4';
			break;
		case NMFIELD_METHOD_DELETE_ALL:
			str = '3';
			break;
		case NMFIELD_METHOD_DELETE:
			str = '2';
			break;
		case NMFIELD_METHOD_ADD:
			str = '1';
			break;
		default:					/* NMFIEL D_METHOD_VALID */
			str = '0';
			break;
	}

	return str;
}

void CoreProtocol::slotOutgoingData( const QCString &out )
{
	cout << out.data() << endl;
}

#include "coreprotocol.moc"
