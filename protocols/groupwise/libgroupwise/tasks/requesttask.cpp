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
	if ( transfer() )
	{
		qDebug( "%s::onGo() - sending %s fields", className(), ( static_cast<Request *>( transfer() )->command().ascii() ) );
		send( static_cast<Request *>( transfer() ) );
	}
	else
		qDebug( "RequestTask::onGo() - called prematurely, no transfer set." );
}

bool RequestTask::take( Transfer * transfer )
{
	if ( forMe( transfer ) )
	{
		qDebug( "RequestTask::take() - Default take() Accepting transaction ack, taking no further action" );
		Response * response = dynamic_cast<Response *>( transfer );
		if ( response->resultCode() == GroupWise::None )
			setSuccess();
		else
			setError( response->resultCode() );
		return true;
	}
	else
		return false;
}

#include "requesttask.moc"
