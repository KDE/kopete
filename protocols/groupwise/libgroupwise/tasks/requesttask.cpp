/*
    Kopete Groupwise Protocol
    requesttask.cpp - Ancestor of all tasks that carry out a user request

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

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

#include "requesttask.h"

#include "gwfield.h"
#include "client.h"
#include "request.h"
#include "response.h"
#include "requestfactory.h"

RequestTask::RequestTask( Task * parent )
: Task( parent )
{
}

bool RequestTask::forMe( const Transfer * transfer ) const
{
	// see if we can down-cast transfer to a Response
	const Response * theResponse = dynamic_cast<const Response *>(transfer);
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
		client()->debug( QString( "%1::onGo() - sending %2 fields" ).arg( metaObject()->className() ).arg( static_cast<Request *>( transfer() )->command() ) );
		send( static_cast<Request *>( transfer() ) );
	}
	else
		client()->debug( "RequestTask::onGo() - called prematurely, no transfer set." );
}

bool RequestTask::take( Transfer * transfer )
{
	if ( forMe( transfer ) )
	{
		client()->debug( "RequestTask::take() - Default take() Accepting transaction ack, taking no further action" );
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
