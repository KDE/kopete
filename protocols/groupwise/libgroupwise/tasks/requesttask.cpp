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
#include "request.h"
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
	return (theResponse && theResponse->transactionId() == m_transactionId );
}


void RequestTask::setTransfer( Transfer * transfer )
{
	m_transactionId = static_cast<Request *>(transfer)->transactionId();
	Task::setTransfer( transfer );
}

#include "requesttask.moc"
