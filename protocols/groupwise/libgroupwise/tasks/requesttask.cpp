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
		Field::FieldListIterator it;
		if ( ( it = fields.find( NM_A_SZ_TRANSACTION_ID ) ) != fields.end() )
		{
			if ( *it )
			{
				// if the transaction id matches ours, return true.
				Field::SingleField * tIdField = dynamic_cast<Field::SingleField *>( *it );
				if ( tIdField )
					return ( tIdField->value().toInt() == m_transactionId );
			}
		}
	}
	return false;
}

void RequestTask::setTransactionId( const int transactionId )
{
	m_transactionId = transactionId;
}

#include "requesttask.moc"
