/*
    yahooconferencemessagemanager.h - Yahoo Conference Message Manager

    Copyright (c) 2003 by Duncan Mac-Vicar <duncan@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#include <kdebug.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kconfig.h>

#include "kopetecontactaction.h"
#include "kopetecontactlist.h"
#include "kopetecontact.h"
#include "kopetechatsessionmanager.h"

#include "yahooconferencemessagemanager.h"
#include "yahoocontact.h"
#include "yahooaccount.h"

YahooConferenceChatSession::YahooConferenceChatSession( const QString & yahooRoom, Kopete::Protocol *protocol, const Kopete::Contact *user,
	Kopete::ContactPtrList others, const char *name )
: Kopete::ChatSession( user, others, protocol,  name )
{
	Kopete::ChatSessionManager::self()->registerChatSession( this );

	connect ( this, SIGNAL( messageSent ( Kopete::Message &, Kopete::ChatSession * ) ),
			  SLOT( slotMessageSent ( Kopete::Message &, Kopete::ChatSession * ) ) );

	m_yahooRoom = yahooRoom;
}

YahooConferenceChatSession::~YahooConferenceChatSession()
{
	emit leavingConference( this );
}

const QString &YahooConferenceChatSession::room()
{
	return m_yahooRoom;
}

void YahooConferenceChatSession::joined( YahooContact *c )
{
	addContact( c );
}

void YahooConferenceChatSession::left( YahooContact *c )
{
	removeContact( c, i18n("%1 has left the conference.").arg(c->userId()) );
}

void YahooConferenceChatSession::slotMessageSent( Kopete::Message & message, Kopete::ChatSession * )
{
	kdDebug ( 14180 ) << k_funcinfo << endl;

	YahooAccount *acc = dynamic_cast< YahooAccount *>( account() );
	if( acc )
		acc->sendConfMessage( this, message );
	appendMessage( message );
	messageSucceeded();
}

#include "yahooconferencemessagemanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

