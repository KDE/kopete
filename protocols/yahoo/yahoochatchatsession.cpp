/*
    yahoochatchatsession.cpp - Yahoo Chat Chatsession

    Copyright (c) 2003 by Duncan Mac-Vicar <duncan@kde.org>
    Copyright (c) 2006 by André Duffeck        <duffeck@kde.org>

    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "yahoochatchatsession.h"

#include "yahoo_protocol_debug.h"
#include <klocale.h>
#include <kcomponentdata.h>

#include <kopetecontactlist.h>
#include <kopetecontact.h>
#include <kopetechatsessionmanager.h>
#include <kopeteuiglobal.h>
#include <kopetemessage.h>

#include "yahoocontact.h"
#include "yahooaccount.h"

YahooChatChatSession::YahooChatChatSession( Kopete::Protocol *protocol, const Kopete::Contact *user,
	Kopete::ContactPtrList others )
: Kopete::ChatSession( user, others, protocol )
{
    setComponentName(QStringLiteral("yahoo_protocol"), i18n("Kopete"));
	Kopete::ChatSessionManager::self()->registerChatSession( this );

	connect ( this, SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)),
			  SLOT(slotMessageSent(Kopete::Message&,Kopete::ChatSession*)) );

	setDisplayName( i18n("Yahoo Chat: " ));

	setXMLFile(QStringLiteral("yahoochatui.rc"));
}

YahooChatChatSession::~YahooChatChatSession()
{
	emit leavingChat( this );
}

void YahooChatChatSession::removeAllContacts()
{
	Kopete::ContactPtrList m = members();
	foreach( Kopete::Contact *c, m )
	{
		removeContact( c );
	}
}

void YahooChatChatSession::setTopic( const QString &topic )
{
	setDisplayName( i18n("Yahoo Chat: %1", topic) );
}

YahooAccount *YahooChatChatSession::account()
{
	return static_cast< YahooAccount *>( Kopete::ChatSession::account() );
}

void YahooChatChatSession::joined( YahooContact *c, bool suppressNotification )
{
	addContact( c, suppressNotification );
}

void YahooChatChatSession::left( YahooContact *c )
{
	removeContact( c );
}

void YahooChatChatSession::slotMessageSent( Kopete::Message & message, Kopete::ChatSession * )
{
    qCDebug (YAHOO_PROTOCOL_LOG) ;

	YahooAccount *acc = dynamic_cast< YahooAccount *>( account() );
	if( acc )
		acc->sendChatMessage( message, m_handle );
	appendMessage( message );
	messageSucceeded();
}

