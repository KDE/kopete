//
// C++ Implementation: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "client.h"
#include "gwerror.h"
#include "request.h"
#include "requestfactory.h"

#include "setstatustask.h"

using namespace GroupWise; 

SetStatusTask::SetStatusTask(Task* parent): RequestTask(parent)
{
}

SetStatusTask::~SetStatusTask()
{
}

void SetStatusTask::status( Status newStatus, const QString &awayMessage, const QString &autoReply )
{
	if ( newStatus > GroupWise::Invalid )
	{
		setError( 1, "Invalid Status" );
		return;
	}
	
	m_status = newStatus;
	m_awayMessage = awayMessage;
	m_autoReply = autoReply;
	
	QCString command("setstatus");
	Request * setStatus = client()->requestFactory()->request( command );
	Field::FieldList lst;
	lst.append( new Field::SingleField( NM_A_SZ_STATUS, 0, NMFIELD_TYPE_UTF8, QString::number( newStatus ) ) );
	if ( !awayMessage.isNull() )
		lst.append( new Field::SingleField( NM_A_SZ_STATUS_TEXT, 0, NMFIELD_TYPE_UTF8, awayMessage ) );
	if ( !autoReply.isNull() )
		lst.append( new Field::SingleField( NM_A_SZ_MESSAGE_BODY, 0, NMFIELD_TYPE_UTF8, autoReply ) );
	setStatus->setFields( lst );
	setTransfer( setStatus );
}

void SetStatusTask::onGo()
{
	//cout << "SetStatusTask::onGo() - sending status fields" << endl;
	send( static_cast<Request *>( transfer() ) );
}

Status SetStatusTask::requestedStatus() const
{
	return m_status;
}

QString SetStatusTask::awayMessage() const
{
	return m_awayMessage;
}

QString SetStatusTask::autoReply() const 
{
	return m_autoReply;
}

#include "setstatustask.moc"
