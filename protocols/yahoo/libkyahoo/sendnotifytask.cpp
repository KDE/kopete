/*
    Kopete Yahoo Protocol
    Send a notification

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

#include "sendnotifytask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>

SendNotifyTask::SendNotifyTask(Task* parent) : Task(parent)
{
	kdDebug(14180) << k_funcinfo << endl;
}

SendNotifyTask::~SendNotifyTask()
{
}

void SendNotifyTask::onGo()
{
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceNotify);
	t->setId( client()->sessionID() );
	t->setStatus( Yahoo::StatusNotify );
	t->setParam( 4, client()->userId());
	t->setParam( 5, m_target );
	t->setParam( 13, m_state );
	t->setParam( 14, QString::fromLatin1(" ") );	
	switch( m_type )
	{
	case NotifyTyping:
		t->setParam( 49, QString::fromLatin1("TYPING") );
	break;
	case NotifyWebcamInvite:
	case NotifyGame:
	default:
		setSuccess( false );
		return;
	break;
	}
	send( t );
	
	setSuccess( true );
}

void SendNotifyTask::setType( Type type )
{
	m_type = type;
}

void SendNotifyTask::setTarget( const QString &to )
{
	m_target = to;
}

void SendNotifyTask::setState( State state)
{
	m_state = state;
}


#include "sendnotifytask.moc"
