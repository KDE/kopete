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

#include <kdebug.h>
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
	Kopete::ChatSessionManager::self()->registerChatSession( this );
	setComponentData(protocol->componentData());

	connect ( this, SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)),
			  SLOT(slotMessageSent(Kopete::Message&,Kopete::ChatSession*)) );

	setDisplayName( i18n("Yahoo Chat: " ));

	setXMLFile("yahoochatui.rc");
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
	kDebug ( YAHOO_GEN_DEBUG ) ;

	YahooAccount *acc = dynamic_cast< YahooAccount *>( account() );
	if( acc )
		acc->sendChatMessage( message, m_handle );
	appendMessage( message );
	messageSucceeded();
}

#include "yahoochatchatsession.moc"

// vim: set noet ts=4 sts=4 sw=4:

