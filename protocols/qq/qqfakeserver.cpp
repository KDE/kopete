/*
    qqfakeserver.cpp - Kopete QQ Protocol

    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "qqfakeserver.h"
#include <qtimer.h>
#include <kdebug.h>
#include "qqincomingmessage.h"


QQFakeServer::QQFakeServer()
{
	m_incomingMessages.setAutoDelete( true );
}

QQFakeServer::~QQFakeServer()
{
}

void QQFakeServer::sendMessage( QString contactId, QString message )
{
	// see what contact the message is for
	// if it's for Echo, respond immediately
	kDebug( 14210 ) << k_funcinfo << "Message for: " << contactId << ", is: " << message << endl;
	kDebug( 14210 ) << "recipient is echo, coming back at you." << endl;
	// put the message in a map and start a timer to tell it to deliver itself.
	//emit messageReceived( QString::fromLatin1( "echo: " ) + message );
	QString messageId = contactId + QString::fromLatin1(": ");
	QQIncomingMessage* msg = new QQIncomingMessage( this, messageId + message );
	m_incomingMessages.append( msg );
	QTimer::singleShot( 1000, msg, SLOT( deliver() ) );
	
	// This removes any delivered messages 
	purgeMessages();
}

void QQFakeServer::incomingMessage( QString message )
{
	emit messageReceived( message );
}

void QQFakeServer::purgeMessages()
{
	QQIncomingMessage* msg;
	for ( msg = m_incomingMessages.first(); msg; msg = m_incomingMessages.next() )
	{
		if ( msg->delivered() )
			m_incomingMessages.remove();
	}
}

#include "qqfakeserver.moc"
