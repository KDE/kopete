/*
    testbedfakeserver.cpp - Kopete Testbed Protocol

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

#include "testbedfakeserver.h"
#include <qtimer.h>
#include <kdebug.h>
#include "testbedincomingmessage.h"


TestbedFakeServer::TestbedFakeServer()
{
	m_incomingMessages.setAutoDelete( true );
}

TestbedFakeServer::~TestbedFakeServer()
{
}

void TestbedFakeServer::sendMessage( QString contactId, QString message )
{
	// see what contact the message is for
	// if it's for Echo, respond immediately
	kdDebug( 14210 ) << k_funcinfo << "Message for: " << contactId << ", is: " << message << endl;
	kdDebug( 14210 ) << "recipient is echo, coming back at you." << endl;
	// put the message in a map and start a timer to tell it to deliver itself.
	//emit messageReceived( QString::fromLatin1( "echo: " ) + message );
	QString messageId = contactId + QString::fromLatin1(": ");
	TestbedIncomingMessage* msg = new TestbedIncomingMessage( this, messageId + message );
	m_incomingMessages.append( msg );
	QTimer::singleShot( 1000, msg, SLOT( deliver() ) );
	
	// This removes any delivered messages 
	purgeMessages();
}

void TestbedFakeServer::incomingMessage( QString message )
{
	emit messageReceived( message );
}

void TestbedFakeServer::purgeMessages()
{
	TestbedIncomingMessage* msg;
	for ( msg = m_incomingMessages.first(); msg; msg = m_incomingMessages.next() )
	{
		if ( msg->delivered() )
			m_incomingMessages.remove();
	}
}

#include "testbedfakeserver.moc"
