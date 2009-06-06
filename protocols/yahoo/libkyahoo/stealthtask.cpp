/*
    Kopete Yahoo Protocol
    Stealth/Unstealth a buddy

    Copyright (c) 2005 André Duffeck <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "stealthtask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>

StealthTask::StealthTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
}

StealthTask::~StealthTask()
{
}

void StealthTask::onGo()
{
	YMSGTransfer *t = new YMSGTransfer();
	if( m_mode == Yahoo::StealthOnline )
	{
		t->setService( Yahoo::ServiceStealthOnline );
		t->setParam( 13, "1" );
		t->setParam( 31, m_state );	
	}
	else if( m_mode == Yahoo::StealthOffline )
	{
		t->setService( Yahoo::ServiceStealthOffline );
		t->setParam( 13, "1" );
		t->setParam( 31, m_state );
	}
	else if( m_mode == Yahoo::StealthPermOffline )
	{
		t->setService( Yahoo::ServiceStealthOffline );
		t->setParam( 13, "2" );
		t->setParam( 31, m_state );
	}
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit());
	if( !m_target.isEmpty() ) {
		t->setParam( 302, 319 );
		t->setParam( 300, 319 );
		t->setParam( 7, m_target.toLocal8Bit() );
		t->setParam( 301, 319 );
		t->setParam( 303, 319 );
	}
	send( t );
	
	setSuccess();
}

void StealthTask::setTarget( const QString &to )
{
	m_target = to;
}

void StealthTask::setState( Yahoo::StealthStatus state)
{
	m_state = state;
}

void StealthTask::setMode( Yahoo::StealthMode mode )
{
	m_mode = mode;
}
