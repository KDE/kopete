/*
    ircservercontact.cpp - IRC Server Contact

    Copyright (c) 2003      by Michel Hermier <mhermier@kde.org>
    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "ircaccount.h"
#include "irccontact.h"
#include "ircprotocol.h"

#include "kopetechatsessionmanager.h"
#include "kopeteview.h"

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include <qtimer.h>
/*
void IRCServerContact::slotSendMsg(Kopete::Message &, Kopete::ChatSession *manager)
{
	manager->messageSucceeded();
	Kopete::Message msg( manager->myself(), manager->members(),
		i18n("You can not talk to the server, you can only issue commands here. Type /help for supported commands."), Kopete::Message::Internal, Kopete::Message::PlainText, CHAT_VIEW);
	manager->appendMessage(msg);
}

void IRCServerContact::appendMessage( const QString &message )
{
	Kopete::ContactPtrList members;
	members.append( this );
	Kopete::Message msg( this, members, message, Kopete::Message::Internal,
		Kopete::Message::RichText, CHAT_VIEW );
	appendMessage(msg);
}

void IRCServerContact::slotIncomingNotice( const QString &orig, const QString &notice )
{
	QString originator = orig.contains('!') ? orig.section('!',0,1) : orig;
	ircAccount()->appendMessage(
		i18n("NOTICE from %1: %2").arg(
			originator == ircAccount()->mySelf()->nickName() ? kircEngine()->currentHost() : originator, notice
		),
		IRCAccount::NoticeReply
	);
}

void IRCServerContact::slotDumpMessages()
{
	if (!mMsgBuffer.isEmpty())
	{
		manager()->appendMessage( mMsgBuffer.front() );
		mMsgBuffer.pop_front();
		QTimer::singleShot( 0, this, SLOT( slotDumpMessages() ) );
	}
}

void IRCServerContact::slotViewCreated( KopeteView *v )
{
	kdDebug(14121) << k_funcinfo << "Created: " << v << ", mgr: " << v->msgManager() << ", Mine: " << m_chatSession << endl;
	if (m_chatSession && v->msgManager() == m_chatSession)
		QTimer::singleShot(500, this, SLOT(slotDumpMessages()));
}
*/
