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
	if ( m_state = NeedMore )
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
			m_state = ( readResponse( din ) ? Available : NeedMore );
			emit incomingData();
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

bool CoreProtocol::readResponse( const QDataStream& wireResponse )
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
		m_state = ServerRedirect;
		return false;
	}
	// other header processing ( 500! )
	if ( ok && rtnCode == 500 )
	{	
		m_state = ServerError;
		return false;
	}
	// find transaction id field and create Response object if nonzero
	// add fields to Response object
	
	// find field type 0 that indicates end of response
		// append to inQueue
		// set state to Available
	// didn't find an 0 - Response is in multiple packets...
		// Gaim doesn't handle this case yet
		// Reconstruction - if we are just reading top level fields, read next packet until 0
		// if we were reading a subarray, we know how long it should be, 
		// store the partial request, partial MultiField and count of total number of fields in the protocol to await next packet
	return true;
}

void CoreProtocol::readFields( QTextStream &tin )
{
	// WHILE
	
	// read fields 
		// read uint8 type
		// if type is 0 SOMETHING_INVALID
			// break
		// read uint8 method
		// read length of tag
		// read tag 
		// if multivalue or array
			// read length uint32
			// call self
			// append result to fieldlist
			// create multifield
		// if utf8 or dn
			// read length uint32
			// read string
			// convert to unicode
			// create singlefield
		// otherwise ( numeric )
			// read value uint32
		// append to fieldList
			
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
