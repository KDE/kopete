/*
    Kopete Yahoo Protocol
    Send a authorization request response

    Copyright (c) 2006 André Duffeck <duffeck@kde.org>
    Kopete    (c) 2003-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "sendauthresptask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <kdebug.h>

SendAuthRespTask::SendAuthRespTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
}

SendAuthRespTask::~SendAuthRespTask()
{
}

void SendAuthRespTask::onGo()
{
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceAuthorization);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit() );
	t->setParam( 5, m_target.toLocal8Bit() );
	if( m_granted )
	{
		t->setParam( 13, 1 );
	}
	else
	{
		t->setParam( 13, 2 );
		t->setParam( 97, 1 );	// UTF
		t->setParam( 14, m_msg.toUtf8() );
		
	}
	send( t );
	
	setSuccess();
}

void SendAuthRespTask::setGranted( bool granted )
{
	m_granted = granted;
}

void SendAuthRespTask::setTarget( const QString &to )
{
	m_target = to;
}

void SendAuthRespTask::setMessage( const QString &msg )
{
	m_msg = msg;
}


#include "sendauthresptask.moc"
