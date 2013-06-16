/*
    Kopete Yahoo Protocol
    Send a notification

    Copyright (c) 2005-2006 André Duffeck <duffeck@kde.org>

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

#include <kdebug.h>

SendNotifyTask::SendNotifyTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
}

SendNotifyTask::~SendNotifyTask()
{
}

void SendNotifyTask::onGo()
{
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceNotify);
	t->setId( client()->sessionID() );
	t->setStatus( Yahoo::StatusNotify );

	switch( m_type )
	{
	case NotifyTyping:
		/**
		* TODO michaelacole
		* Add here later a check for if stealth, only want to send if not stealted
		* so if you start to answer and decide not to at least they do not know you are there
		*/
		t->setParam( 1, client()->userId().toLocal8Bit() );
		t->setParam( 5, m_target.toLocal8Bit() );
		t->setParam( 13, m_state );
		t->setParam( 14, " " );	
		t->setParam( 49, "TYPING" );
	break;
	case NotifyWebcamInvite:
		
		kDebug(YAHOO_RAW_DEBUG) << "send invitation set Param";
		t->setParam( 1, client()->userId().toLocal8Bit() );
		t->setParam( 5, m_target.toLocal8Bit() );
		t->setParam( 13, 0 );
		t->setParam( 14, " " );	
		t->setParam( 49, "WEBCAMINVITE" );		
	break;
	case NotifyGame:
	default:
		setError();
		delete t;
		return;
	break;
	}
	send( t );
	
	setSuccess();
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
