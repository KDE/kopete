/*
    Kopete Groupwise Protocol
    setstatustask.cpp - Sets our status on the server

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
	lst.append( new Field::SingleField( Field::NM_A_SZ_STATUS, 0, NMFIELD_TYPE_UTF8, QString::number( newStatus ) ) );
	if ( !awayMessage.isNull() )
		lst.append( new Field::SingleField( Field::NM_A_SZ_STATUS_TEXT, 0, NMFIELD_TYPE_UTF8, awayMessage ) );
	if ( !autoReply.isNull() )
		lst.append( new Field::SingleField( Field::NM_A_SZ_MESSAGE_BODY, 0, NMFIELD_TYPE_UTF8, autoReply ) );
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
