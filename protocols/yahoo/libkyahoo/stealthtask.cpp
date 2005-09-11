/*
    Kopete Yahoo Protocol
    Stealth/Unstealth a buddy

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

#include "stealthtask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>

StealthTask::StealthTask(Task* parent) : Task(parent)
{
	kdDebug(14180) << k_funcinfo << endl;
}

StealthTask::~StealthTask()
{
}

void StealthTask::onGo()
{
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceStealth);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId());
	t->setParam( 7, m_target );
	t->setParam( 13, QString::fromLatin1("2") );
	t->setParam( 31, m_state );	
	send( t );
	
	setSuccess( true );
}

void StealthTask::setTarget( const QString &to )
{
	m_target = to;
}

void StealthTask::setState( Yahoo::StealthStatus state)
{
	m_state = state;
}
