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
	
	Field::FieldList lst;
	lst.append( new Field::SingleField( NM_A_SZ_STATUS, 0, NMFIELD_TYPE_UTF8, QString::number( newStatus ) ) );
	if ( !awayMessage.isNull() )
		lst.append( new Field::SingleField( NM_A_SZ_STATUS_TEXT, 0, NMFIELD_TYPE_UTF8, awayMessage ) );
	if ( !autoReply.isNull() )
		lst.append( new Field::SingleField( NM_A_SZ_MESSAGE_BODY, 0, NMFIELD_TYPE_UTF8, autoReply ) );
	createTransfer( "setstatus", lst );
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
