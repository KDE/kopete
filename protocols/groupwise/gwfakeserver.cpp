/*
    gwfakeserver.cpp - Kopete GroupWise Protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed   
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

#include "gwfakeserver.h"
#include <qtimer.h>
#include <kdebug.h>
#include "gwincomingmessage.h"


GroupWiseFakeServer::GroupWiseFakeServer()
{
	m_incomingMessages.setAutoDelete( true );
}

GroupWiseFakeServer::~GroupWiseFakeServer()
{
}

void GroupWiseFakeServer::sendMessage( QString contactId, QString message )
{
	// see what contact the message is for
	// if it's for Echo, respond immediately
	kdDebug( 14220 ) << k_funcinfo << "Message for: " << contactId << ", is: " << message << endl;
	if ( contactId == QString::fromLatin1( "echo" ) )
	{
		kdDebug( 14220 ) << "recipient is echo, coming back at you." << endl;
		// put the message in a map and start a timer to tell it to deliver itself.
		//emit messageReceived( QString::fromLatin1( "echo: " ) + message );
		GroupWiseIncomingMessage* msg = new GroupWiseIncomingMessage( this, QString::fromLatin1( "echo: " ) + message );
		m_incomingMessages.append( msg );
		QTimer::singleShot( 3000, msg, SLOT( deliver() ) );
	}
	else
		kdWarning( 14220 ) << "message recipient: " << contactId << " is unknown." << endl;
	
	// This removes any delivered messages 
	purgeMessages();
}

void GroupWiseFakeServer::incomingMessage( QString message )
{
	emit messageReceived( message );
}

void GroupWiseFakeServer::purgeMessages()
{
	GroupWiseIncomingMessage* msg;
	for ( msg = m_incomingMessages.first(); msg; msg = m_incomingMessages.next() )
	{
		if ( msg->delivered() )
			m_incomingMessages.remove();
	}
}

#include "gwfakeserver.moc"
