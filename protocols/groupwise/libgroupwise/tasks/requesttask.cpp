//
// C++ Implementation: requesttask
//
// Description: 
//
//
// Author: SUSE AG  (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "gwfield.h"
#include "response.h"

#include "requesttask.h"

RequestTask::RequestTask( Task * parent )
: Task( parent )
{
}

bool RequestTask::forMe( Transfer * transfer ) const
{
	// see if we can down-cast transfer to a Response
	Response * theResponse = dynamic_cast<Response *>(transfer);
	if ( theResponse )
	{
		// if we can locate a transaction id field in its fields
		Field::FieldList fields = theResponse->fields();
		Field::FieldBase * field;
		int index;
		if ( ( index = fields.locate( NM_A_SZ_TRANSACTION_ID ) ) != -1 )
			field = theResponse->fields().at( index );
		if ( field )
		{
			// if the transaction id matches ours, return true.
			Field::SingleField * tIdField = dynamic_cast<Field::SingleField *>( field );
			if ( tIdField )
				return ( tIdField->value().toInt() == m_transactionId );
		}
	}
	return false;
}

void RequestTask::setTransactionId( const int transactionId )
{
	m_transactionId = transactionId;
}

#include "requesttask.moc"
