#include <string.h>
#include <iostream.h>

#include <kdebug.h>
#include <kurl.h>

#include "gwerror.h"
#include "gwfield.h"
#include "request.h"

#include "coreprotocol.h"

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
	int oldsize = in.size();
	in.resize(oldsize + incomingBytes.size());
	memcpy(in.data() + oldsize, incomingBytes.data(), incomingBytes.size());
}

Transfer * CoreProtocol::incomingTransfer()
{
	// Convert the incoming data into either an event or a response, stored in an IncomingData
	return 0;
}

void CoreProtocol::outgoingTransfer( Transfer * outgoing )
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
	// Prepend field containing transaction id
	fields.prepend( new Field::SingleField( NM_A_SZ_TRANSACTION_ID, NMFIELD_METHOD_VALID, 
					0, NMFIELD_TYPE_UTF8, request->transactionId() ) );
	
	// convert to QByteArray
	QCString bout;
	
	// add the POST
	bout += "POST /";
	bout += request->command();
	bout += " HTTP/1.0\r\n";
	
	// if a login, add Host arg : public FieldBase
	if ( request->command() == "login" )
	{
		bout += "Host: ";
		bout += "reiser.suse.de"; //FIXME: Get this from somewhere else!!
		bout += ":8300\r\n";
	}
	else
		bout += "\r\n";
	
	emit outgoingData( bout );
	// now convert 
	fieldsToWire( fields );
}

void CoreProtocol::fieldsToWire( Field::FieldList fields )
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
//				QString encoded = KURL::encode_string( sField->value().toString(), 106 /* UTF-8 */);
//				encoded.replace( "%20", "+" );
//				bout += encoded.ascii();
				bout += sField->value().toString().ascii();
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
			fieldsToWire( mField->fields() );
		}
		//cout << " - field done" << endl;
	}
	cout << " - method done" << endl;
}

void CoreProtocol::reset()
{
	in.resize( 0 );
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
