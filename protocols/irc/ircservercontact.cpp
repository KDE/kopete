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

#include "ircusercontact.h"
#include "ircservercontact.h"
#include "ircaccount.h"
#include "ircprotocol.h"

#include "kopetechatsessionmanager.h"
#include "kopeteview.h"

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include <qtimer.h>

IRCServerContact::IRCServerContact(IRCContactManager *contactManager, const QString &servername, Kopete::MetaContact *m)
	: IRCContact(contactManager, servername, m, "irc_server")
{
	KIRC::Engine *engine = kircEngine();

	QObject::connect(engine, SIGNAL(internalError(KIRC::Engine::Error, KIRC::Message &)),
			this, SLOT(engineInternalError(KIRC::Engine::Error, KIRC::Message &)));
/*
	//FIXME: Have some kind of a debug option for raw input/ouput display??
	QObject::connect(engine, SIGNAL(sentMessage(KIRC::Message &)),
			this, SLOT(engineSentMessage(KIRC::Message &)));
	QObject::connect(engine, SIGNAL(receivedMessage(KIRC::Message &)),
			this, SLOT(engineReceivedMessage(KIRC::Message &)));
*/

	QObject::connect(engine, SIGNAL(incomingNotice(const QString &, const QString &)),
			this, SLOT(slotIncomingNotice(const QString &, const QString &)));

	QObject::connect(engine, SIGNAL(incomingCannotSendToChannel(const QString &, const QString &)),
			this, SLOT(slotCannotSendToChannel(const QString &, const QString &)));

	QObject::connect(engine, SIGNAL(incomingUnknown(const QString &)),
			this, SLOT(slotIncomingUnknown(const QString &)));

	QObject::connect(engine, SIGNAL(incomingConnectString(const QString &)),
			this, SLOT(slotIncomingConnect(const QString &)));

	QObject::connect(engine, SIGNAL(incomingMotd(const QString &)),
			this, SLOT(slotIncomingMotd(const QString &)));

	QObject::connect(Kopete::ChatSessionManager::self(), SIGNAL(viewCreated(KopeteView*)),
			this, SLOT(slotViewCreated(KopeteView*)) );

	updateStatus();
}

void IRCServerContact::updateStatus()
{
	KIRC::Engine::Status status = kircEngine()->status();
	switch( status )
	{
		case KIRC::Engine::Idle:
		case KIRC::Engine::Connecting:
			if( m_chatSession )
				m_chatSession->setDisplayName( caption() );
			setOnlineStatus( m_protocol->m_ServerStatusOffline );
			break;

		case KIRC::Engine::Authentifying:
		case KIRC::Engine::Connected:
		case KIRC::Engine::Closing:
			// should make some extra check here
			setOnlineStatus( m_protocol->m_ServerStatusOnline );
			break;

		default:
			setOnlineStatus( m_protocol->m_StatusUnknown );
	}
}

const QString IRCServerContact::caption() const
{
	return i18n("%1 @ %2").arg(ircAccount()->mySelf()->nickName() ).arg(
		kircEngine()->currentHost().isEmpty() ? ircAccount()->networkName() : kircEngine()->currentHost()
	);
}

void IRCServerContact::engineInternalError(KIRC::Engine::Error engineError, KIRC::Message &ircmsg)
{
	QString error;
	switch (engineError)
	{
		case KIRC::Engine::ParsingFailed:
			error = i18n("KIRC Error - Parse error: ");
			break;
		case KIRC::Engine::UnknownCommand:
			error = i18n("KIRC Error - Unknown command: ");
			break;
		case KIRC::Engine::UnknownNumericReply:
			error = i18n("KIRC Error - Unknown numeric reply: ");
			break;
		case KIRC::Engine::InvalidNumberOfArguments:
			error = i18n("KIRC Error - Invalid number of arguments: ");
			break;
		case KIRC::Engine::MethodFailed:
			error = i18n("KIRC Error - Method failed: ");
			break;
		default:
			error = i18n("KIRC Error - Unknown error: ");
	}

	ircAccount()->appendMessage(error + QString(ircmsg.raw()), IRCAccount::ErrorReply);
}

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
	if (orig.isEmpty()) {
		// Prefix missing.
		// NOTICE AUTH :*** Checking Ident

		ircAccount()->appendMessage(i18n("NOTICE from %1: %2").arg(kircEngine()->currentHost(), notice),
				IRCAccount::NoticeReply);

	} else {
		// :Global!service@rizon.net NOTICE foobar :[Logon News - Oct 12 2005] Due to growing problems ...
		// :somenick!~fooobar@somehostname.fi NOTICE foobar :hello

		if (orig.contains('!')) {
			ircAccount()->appendMessage(i18n("NOTICE from %1 (%2): %3").arg(
						orig.section('!', 0, 0),
						orig.section('!', 1, 1),
						notice),
					IRCAccount::NoticeReply);
		} else {
			ircAccount()->appendMessage(i18n("NOTICE from %1: %2").arg(
						orig, notice), IRCAccount::NoticeReply);
		}
	}
}

void IRCServerContact::slotIncomingUnknown(const QString &message)
{
	ircAccount()->appendMessage(message, IRCAccount::UnknownReply);
}

void IRCServerContact::slotIncomingConnect(const QString &message)
{
	ircAccount()->appendMessage(message, IRCAccount::ConnectReply);
}

void IRCServerContact::slotIncomingMotd(const QString &message)
{
	ircAccount()->appendMessage(message, IRCAccount::InfoReply);
}

void IRCServerContact::slotCannotSendToChannel(const QString &channel, const QString &message)
{
	ircAccount()->appendMessage(QString::fromLatin1("%1: %2").arg(channel).arg(message), IRCAccount::ErrorReply);
}

void IRCServerContact::appendMessage(Kopete::Message &msg)
{
	msg.setImportance( Kopete::Message::Low ); //to don't distrub the user

	if (m_chatSession && m_chatSession->view(false))
		m_chatSession->appendMessage(msg);
/*
//	disable the buffering for now: cause a memleak since we don't made it a *fixed size fifo*
	else
		mMsgBuffer.append( msg );
*/
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

#include "ircservercontact.moc"
