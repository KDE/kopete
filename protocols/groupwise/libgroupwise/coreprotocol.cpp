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

#include "coreprotocol.h"

#define NO_ESCAPE(ch) ((ch == 0x20) || (ch >= 0x30 && ch <= 0x39) || \
					(ch >= 0x41 && ch <= 0x5a) || (ch >= 0x61 && ch <= 0x7a))

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

void CoreProtocol::addIncomingData( const QByteArray & incomingBytes )
{
	int oldsize = m_in.size();
	m_in.resize( oldsize + incomingBytes.size() );
	memcpy( m_in.data() + oldsize, incomingBytes.data(), incomingBytes.size() );
}

Transfer * CoreProtocol::incomingTransfer()
{
	// Convert the incoming data into either an event or a response, stored in an IncomingData
	return 0;
}

void CoreProtocol::outgoingTransfer( Request* outgoing )
{
	// Convert the outgoing data into wire format
	Request * request = dynamic_cast<Request *>( outgoing );
	Field::FieldList fields = request->fields();
	if ( fields.isEmpty() )
	{
		cout << " CoreProtocol::outgoingTransfer: Transfer contained no fields!" << endl;
		m_error = NMERR_BAD_PARM;
		return;
	}
	// Append field containing transaction id
	fields.append( new Field::SingleField( NM_A_SZ_TRANSACTION_ID, NMFIELD_METHOD_VALID, 
					0, NMFIELD_TYPE_UTF8, request->transactionId() ) );
	
	// convert to QByteArray
	QCString bout;
	
	// add the POST
	bout += "POST /";
	bout += request->command();
	bout += " HTTP/1.0\r\n";
	
	// if a login, add Host arg 
	if ( request->command() == "login" )
	{
		bout += "Host: ";
		bout += "reiser.suse.de"; //FIXME: Get this from somewhere else!!
		bout += ":8300\r\n\r\n";
	}
	else
		bout += "\r\n";
	
	emit outgoingData( bout );
	// now convert 
	//fieldsToWire( fields );
	return;
}

void CoreProtocol::wireToTransfer( const QByteArray& wire )
{
	// processing incoming data and reassembling it into transfers
	// may be an event or a response
	QDataStream din( wire, IO_ReadOnly );
	din.setByteOrder( QDataStream::LittleEndian );
	
	// does protocol state indicate we are partially through reading a response?
	if ( m_state == NeedMore )
		readResponse( din );
	else
	{
		// otherwise, examine the data to see what it is
		// look at first four bytes
		Q_UINT32 val;
		din >> val;
		// is 'HTTP' -> response
		if ( qstrncmp( (const char *)&val, "HTTP", strlen( "HTTP" ) ) == 0 )
		{
			if ( readResponse( din ) )
			{
				m_state = Available;
				emit incomingData();
			}
			else
				m_state = NeedMore;
		}
		else	
		// otherwise -> event
		{
			readEvent( val, din );
			emit incomingData();
		}
	}
}

void CoreProtocol::readEvent( const Q_UINT32 eventType, QDataStream& wireEvent )
{
	// discover the length of the event's source, then read it
	Q_UINT32 len;
	wireEvent >> len;
	QCString source;
	if ( len > 0 )
	{
		char* rawSource;
		wireEvent.readBytes( rawSource, len );
		source = rawSource; // shallow copy, QCString's destructor will delete the allocated space
	}
	// now create an event object
	EventTransfer * evt = new EventTransfer( eventType, source, QTime::currentTime() );
	m_inQueue.append( evt );
	
}

bool CoreProtocol::readResponse( QDataStream& wireResponse )
{
	// read rest of HTTP header and look for a 301 redirect. 
	// we can treat the rest of the data as a textstream
	QTextStream tin( wireResponse.device() );
	QString headerFirst = tin.readLine();
	// pull out the HTTP return code
	int firstSpace = headerFirst.find( ' ' );
	QString rtnField = headerFirst.mid( firstSpace, headerFirst.find( ' ', firstSpace + 1 ) );
	bool ok = true;
	int rtnCode;
	int packetState = -1;
	rtnCode = rtnField.toInt( &ok );
	
	// read rest of header
	QStringList headerRest;
	QString line;
	while ( line != "\r" )
	{
		line = tin.readLine();
		headerRest.append( line );
	}
	// if it's a redirect, set flag
	if ( ok && rtnCode == 301 )
	{	
		packetState = ServerRedirect;
		return false;
	}
	// other header processing ( 500! )
	if ( ok && rtnCode == 500 )
	{	
		packetState = ServerError;
		return false;
	}
	
	// read fields
	readFields( wireResponse, -1 );
	// find transaction id field and create Response object if nonzero
	int tId;
	Field::FieldBase* field = 0;
	for ( field = m_collatingFields.first(); field != m_collatingFields.end(); field = m_collatingFields.next() )
	{
		if ( field->tag() == "NM_A_SZ_TRANSACTION_ID" )
		{
			Field::SingleField * sf = dynamic_cast<Field::SingleField*>( field );
			if ( sf )
			{
				tId = sf->value().toInt();
				m_collatedFields.remove( f );
				break;
			}
		}
	}
	// append to inQueue
	if ( tId )
	{
		UserTransfer * response = new UserTransfer( tId );
		response->setFields( m_collatedFields );
		m_inQueue.append( response );
		packetState = Available;
	}
	// TODO - handle broken responses..
	// didn't find an 0 - Response is in multiple packets...
		// Gaim doesn't handle this case yet
		// Reconstruction - if we are just reading top level fields, read next packet until 0
		// if we were reading a subarray, we know how long it should be, 
		// store the partial request, partial MultiField and count of total number of fields in the protocol to await next packet
		
	return ( packetState == Available );
}

void CoreProtocol::readFields( QDataStream &din, int fieldCount, Field::FieldList * list )
{
	// build a list of fields.  
	// If there is already a list of fields stored in m_collatingFields, 
	// the list we're reading on this iteration must be a nested list
	// so when we're done reading it, add it to the MultiList element
	// that is the last element in the top list in m_collatingFields.
	// if we find the beginning of a new nested list, push the current list onto m_collatingFields
	
	Field::FieldList currentList;
	while ( fieldCount != 0 )
	{
		// the field being read
		// read field
		Q_UINT8 type, method;
		Q_UINT32 val;
		QCString tag;
		// read uint8 type
		din >> type;
		// if type is 0 SOMETHING_INVALID, we're at the end of the fields
		if ( type == 0 )
		{
			state = FieldsRead;
			// do something to indicate we're done
			break;
		}
		// read uint8 method
		din >> method;
		// read tag and length
		char* rawData;
		din.readBytes( rawData, val );
		// set tag from the raw data.  QDataStream::readBytes() allocates the 
		// space for rawData and expects us to manage it, and we let 
		// QCString take care of it
		tag.setRawData( rawData, val );
		
		// if multivalue or array
		if ( type == NMFIELD_TYPE_MV || type == NMFIELD_TYPE_ARRAY )
		{
			// read length uint32
			din >> val;
			if ( val > 0 )
			{
			// create multifield
				Field::MultiField* m = new Field::MultiField( tag, method, 0, type );
				currentList.append( m );
				readFields( din, val, &currentList);
			}
			else 
				state = ProtocolError;
				break;
		}
		else 
		{
		
			if ( type == NMFIELD_TYPE_UTF8 || type == NMFIELD_TYPE_DN )
			{
				din.readBytes( rawData, val );
				if ( val > NMFIELD_MAX_STR_LENGTH )
				{
					state = ProtocolError;
					break;
				}
				// convert to unicode
				QString fieldValue = QString::fromUtf8( rawData, val );
				// create singlefield
				Field::SingleField* s = new Field::SingleField( tag, method, 0, type, fieldValue );
				currentList.append( s );
			}
			else
			{
				// otherwise ( numeric )
				// read value uint32
				din >> val;
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
	if ( fieldCount == 0 )
	{
		Field::MultiField * m = dynamic_cast<Field::MultiField*>( list->last() );
		m->setFields( currentList );
	}

	// if fieldCount == -1; we're done reading the top level fieldlist, so store it.
	if ( fieldCount == -1 )
	{
		m_collatingFields = currentList;
	}
}

void CoreProtocol::fieldsToWire( Field::FieldList fields, int depth )
{
	cout << "CoreProtocol::fieldsToWire" << endl;
	int subFieldCount = 0;
	
	// TODO: consider constructing this as a QStringList and then join()ing it.
	QPtrListIterator<Field::FieldBase> it( fields );
	Field::FieldList::const_iterator end = fields.end();
	Field::FieldBase* field;
	while ( ( field = it.current()) != 0 )
	{
		++it;
		//cout << " - writing a field" << endl;
		QCString bout;
		// these fields are ignored by Gaim's novell
		if ( field->type() == NMFIELD_TYPE_BINARY  || field->method() == NMFIELD_METHOD_IGNORE )
			continue;
			
		// GAIM writes these tags to the socket separately - if we can't connect, check here
		// NM Protocol 1 writes them in an apparently arbitrary order
		// tag
		bout += "&tag=";
		bout += field->tag();
		
		// method
		bout += "&cmd=";
		bout += encode_method( field->method() );
		
		// value
		bout += "&val=";
		
		switch ( field->type() )
		{
			case NMFIELD_TYPE_UTF8:		// Field contains UTF-8
			case NMFIELD_TYPE_DN:		// Field contains a Distinguished Name
			{
				//cout << " - it's a single string" << endl;
				const Field::SingleField *sField = static_cast<const Field::SingleField*>( field );
// 				QString encoded = KURL::encode_string( sField->value().toString(), 106 /* UTF-8 */);
// 				encoded.replace( "%20", "+" );
// 				bout += encoded.ascii();

				bout += url_escape_string( sField->value().toString().utf8() );
				//bout += sField->value().toString().ascii();
				break;
			}
			case NMFIELD_TYPE_ARRAY:	// Field contains a field array
			case NMFIELD_TYPE_MV:		// Field contains a multivalue
			{
				//cout << " - it's a multi" << endl;
				const Field::MultiField *mField = static_cast<const Field::MultiField*>( field );
				subFieldCount = mField->fields().count();	// determines if we have a subarray to send after this field
				bout += QString::number( subFieldCount ).ascii();
				break;
			}
			default:					// Field contains a numeric value
			{
				//cout << " - it's a number" << endl;
				const Field::SingleField *sField = static_cast<const Field::SingleField*>( field );
				bout += QString::number( sField->value().toInt() ).ascii();
			}
		}
				
		// type
		bout += "&type=";
		bout += QString::number( field->type() ).ascii();
		
		// write what we have so far, we may be calling this function recursively
		//kdDebug( 14999 ) << k_funcinfo << "writing \'" << bout << "\'" << endl;
		//cout << " - signalling data" << endl;
		emit outgoingData( bout );

		// write fields of subarray, if that's what the current field is
		if ( subFieldCount > 0 && 
				( field->type() == NMFIELD_TYPE_ARRAY || field->type() == NMFIELD_TYPE_MV ) )
		{
			const Field::MultiField *mField = static_cast<const Field::MultiField*>( field );
			fieldsToWire( mField->fields(), ++depth );
		}
		//cout << " - field done" << endl;
		
	}
	if ( depth == 0 ) // this call to the function was not recursive, so the entire request has been sent at this point
	{
		emit outgoingData( QCString( "\r\n" ) );
		cout << " - request sent." << endl;
	}
	cout << " - method done" << endl;
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
