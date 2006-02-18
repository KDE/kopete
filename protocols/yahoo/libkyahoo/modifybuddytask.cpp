/*
    Kopete Yahoo Protocol
    Add a buddy to the Contactlist

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

#include "modifybuddytask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <kdebug.h>

ModifyBuddyTask::ModifyBuddyTask(Task* parent) : Task(parent)
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
}

ModifyBuddyTask::~ModifyBuddyTask()
{
}

void ModifyBuddyTask::onGo()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	switch( m_type )
	{
		case AddBuddy:
			addBuddy();
		break;
		case RemoveBuddy:
			removeBuddy();
		break;
		case MoveBuddy:
			moveBuddy();
		break;
	}


	
	setSuccess( true );
}

void ModifyBuddyTask::addBuddy()
{
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceAddBuddy);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().local8Bit() );
	t->setParam( 7, m_target.local8Bit() );
	t->setParam( 14, m_message.utf8() );
	t->setParam( 65, m_group.local8Bit() );	
	t->setParam( 97, 1 );	// UTF-8
	send( t );
}

void ModifyBuddyTask::removeBuddy()
{
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceRemBuddy);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().local8Bit() );
	t->setParam( 7, m_target.local8Bit() );
	t->setParam( 65, m_group.local8Bit() );	
	send( t );
}

void ModifyBuddyTask::moveBuddy()
{
	YMSGTransfer *mov = new YMSGTransfer( Yahoo::ServiceBuddyChangeGroup );
	mov->setId( client()->sessionID() );
	mov->setParam( 1, client()->userId().local8Bit() );
	mov->setParam( 302, 240 );
	mov->setParam( 300, 240 );
	mov->setParam( 7, m_target.local8Bit() );
	mov->setParam( 224, m_oldGroup.local8Bit() );
	mov->setParam( 264, m_group.local8Bit() );
	mov->setParam( 301, 240 );
	mov->setParam( 303, 240 );
	send( mov );
}

void ModifyBuddyTask::setTarget( const QString &target )
{
	m_target = target;
}

void ModifyBuddyTask::setMessage( const QString &text )
{
	m_message = text;
}

void ModifyBuddyTask::setGroup( const QString &group )
{
	m_group = group;
}

void ModifyBuddyTask::setOldGroup( const QString &old )
{
	m_oldGroup = old;
}

void ModifyBuddyTask::setType( Type type )
{
	m_type = type;
}
