//
// C++ Implementation: getstatustask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "client.h"
#include "request.h"
#include "requestfactory.h"
#include "response.h"

#include "getstatustask.h"

GetStatusTask::GetStatusTask(Task* parent): RequestTask(parent)
{
}

GetStatusTask::~GetStatusTask()
{
}

void GetStatusTask::userDN( const QString & dn )
{
	m_userDN = dn;
	// set up Transfer
	QCString command = "getstatus";
	Request * getDetailsRequest = client()->requestFactory()->request( command );
	Field::FieldList lst;
	
	lst.append( new Field::SingleField( NM_A_SZ_USERID, 0, NMFIELD_TYPE_UTF8, m_userDN ) );
	
	getDetailsRequest->setFields( lst );
	setTransfer( getDetailsRequest );
}

void GetStatusTask::onGo()
{	
	qDebug( "GetStatusTask::onGo() - sending getstatus field for user: %s", m_userDN.ascii() );
	send( static_cast<Request *>( transfer() ) );
}

bool GetStatusTask::take( Transfer * transfer )
{
	if ( !forMe( transfer ) )
		return false;
	Response * response = dynamic_cast<Response *>( transfer );
	if ( !response )
		return false;
	
	Field::FieldList responseFields = response->fields();
	responseFields.dump( true );
	// parse received details and signal like billio
	Field::SingleField * sf = 0;
	Q_UINT16 status;
	sf = responseFields.findSingleField( NM_A_SZ_STATUS );
	if ( sf )
	{
		// As of Sept 2004 the server always responds with 2 (Available) here, even if the sender is not
		// This must be because the sender is not on our contact list but has sent us a message.
		status = sf->value().toInt();
		// unfortunately getstatus doesn't give us an away message so we pass QString::null here
		emit gotStatus( m_userDN, status, QString::null );
		setSuccess();
	}	
	else
		setError();	
	return true;
}

#include "getstatustask.moc"
