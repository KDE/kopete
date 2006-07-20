/*
    qqincomingmessage.cpp - Kopete QQ Protocol

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

#include "qqincomingmessage.h"

QQIncomingMessage::QQIncomingMessage( QQFakeServer* const server , QString message )
{
	m_server = server; 
	m_message = message; 
	m_delivered = false; 
}

QQIncomingMessage::~QQIncomingMessage()
{
}

void QQIncomingMessage::deliver() 
{ 
	m_server->incomingMessage( m_message ); 
	m_delivered = true; 
}

#include "qqincomingmessage.moc"
