//
// C++ Implementation: logintask
//
// Description: 
//
//
// Author: SUSE AG (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "client.h"
#include "request.h"
#include "requestfactory.h"
#include "response.h"

#include "logintask.h"

LoginTask::LoginTask( Task * parent )
 : RequestTask( parent )
{
}

LoginTask::~LoginTask()
{
}

void LoginTask::initialise()
{
	QCString command("login");
	Request * loginRequest = client()->requestFactory()->request( command );
	Field::FieldList lst;
	lst.append( new Field::SingleField( NM_A_SZ_USERID, 0, NMFIELD_TYPE_UTF8, client()->userId() ) );
	lst.append( new Field::SingleField( NM_A_SZ_CREDENTIALS, 0, NMFIELD_TYPE_UTF8, client()->password() ) );
	lst.append( new Field::SingleField( NM_A_SZ_USER_AGENT, 0, NMFIELD_TYPE_UTF8, client()->userAgent() ) );
	lst.append( new Field::SingleField( NM_A_UD_BUILD, 0, NMFIELD_TYPE_UDWORD, 2 ) );
	lst.append( new Field::SingleField( NM_A_IP_ADDRESS, 0, NMFIELD_TYPE_UTF8, client()->ipAddress() ) );
	loginRequest->setFields( lst );
	setTransactionId( loginRequest->transactionId() );
	setTransfer( loginRequest );
}

void LoginTask::onGo()
{
	send( static_cast<Request *>( transfer() ) );
}

bool LoginTask::take( Transfer * transfer )
{
	if ( !forMe( transfer ) )
		return false;
	Response * response = dynamic_cast<Response *>( transfer );
	if ( !response )
		return false;
	
	// read in myself()'s metadata fields and emit signal
/*	
	Field::FieldList loginResponseFields = response->fields();
	emit gotMyself( loginResponseFields );
	
	// create contact list
	// locate contact list
	 
	// extract folder fields 
	while ( findFolder() )
	{
		emit gotFolder( fields );
	}
	// extract contact fields 
	while ( findContact() )
	{
		emit gotContact( fields );
		if ( findUserDetails( fields );
			emit gotContactUserRecord( fields );
	}
	
	// create privacy list
	*/
	return true;
}

#include "logintask.moc"
