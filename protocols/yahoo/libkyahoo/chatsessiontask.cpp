/*
    Kopete Yahoo Protocol
    chatsessiontask.cpp - Register / Unregister a chatsession

    Copyright (c) 2006 André Duffeck <andre.duffeck@kdemail.net>

    Kopete (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "chatsessiontask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <kdebug.h>

ChatSessionTask::ChatSessionTask(Task* parent) : Task(parent)
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
}

ChatSessionTask::~ChatSessionTask()
{
}

void ChatSessionTask::onGo()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	YMSGTransfer *t = new YMSGTransfer( Yahoo::ServiceChatSession );
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().local8Bit() );
	t->setParam( 5, m_target.local8Bit() );
	if( m_type == RegisterSession )
	{
		t->setParam( 13, 1 );
	}
	else
	{
		t->setParam( 13, 2 );
		t->setParam( 34, 1 );
	}
	send( t );
	
	setSuccess( true );
}

void ChatSessionTask::setTarget( const QString &to )
{
	m_target = to;
}

void ChatSessionTask::setType( Type type )
{
	m_type = type;
}
