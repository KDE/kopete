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
#include "client.h"
#include "request.h"
#include "response.h"
#include "requestfactory.h"

#include "requesttask.h"

RequestTask::RequestTask( Task * parent )
: Task( parent )
{
}

bool RequestTask::forMe( Transfer * transfer ) const
{
	// see if we can down-cast transfer to a Response
	Response * theResponse = dynamic_cast<Response *>(transfer);
	return (theResponse && theResponse->transactionId() == m_transactionId );
}

void RequestTask::createTransfer( const QString & command, const Field::FieldList & fields )
{
	Request * request = client()->requestFactory()->request( command );
	m_transactionId = request->transactionId();
	request->setFields( fields );
	Task::setTransfer( request );
}

void RequestTask::onGo()
{
	qDebug( "%s::onGo() - sending %s fields", className(), ( static_cast<Request *>( transfer() )->command().ascii() ) );
	send( static_cast<Request *>( transfer() ) );
}

bool RequestTask::take( Transfer * transfer )
{
	if ( forMe( transfer ) )
	{
		qDebug( "RequestTask::take() - Default take() Accepting transaction ack, taking no further action" );
		Response * response = dynamic_cast<Response *>( transfer );
		Field::FieldList responseFields = response->fields();
		Field::SingleField * resultCodeField = responseFields.findSingleField( NM_A_SZ_RESULT_CODE );
		int resultCode = resultCodeField->value().toInt();
		if ( resultCode == GroupWise::None )
			setSuccess();
		else
			setError( resultCode );
		return true;
	}
	else
		return false;
}

#include "requesttask.moc"
