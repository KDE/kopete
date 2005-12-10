/*
    Kopete Yahoo Protocol
    Send a message

    Copyright (c) 2005 André Duffeck <andre.duffeck@kdemail.net>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "sendmessagetask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>

SendMessageTask::SendMessageTask(Task* parent) : Task(parent)
{
	kdDebug(14180) << k_funcinfo << endl;
}

SendMessageTask::~SendMessageTask()
{
}

void SendMessageTask::onGo()
{
	kdDebug(14180) << k_funcinfo << endl;

	if( m_text.isEmpty() )
	{
		kdDebug(14180) << k_funcinfo << "Text to send is empty." << endl;
		return;
	}	

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceMessage);
	t->setId( client()->sessionID() );
	t->setStatus( Yahoo::StatusNotify );
	t->setParam( 1, client()->userId().local8Bit() );
	t->setParam( 5, m_target.local8Bit() );
	t->setParam( 14, m_text.utf8() );
	t->setParam( 63, ";0" );
	t->setParam( 64, "0"  );	
	t->setParam( 97, 1 );	// UTF-8
	t->setParam( 206, client()->pictureFlag() );	
	send( t );
	
	setSuccess( true );
}

void SendMessageTask::setTarget( const QString &to )
{
	m_target = to;
}

void SendMessageTask::setText( const QString &text )
{
	m_text = text;
}

void SendMessageTask::setPicureFlag( int flag )
{
	m_pictureFlag = flag;
}
