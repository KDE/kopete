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

#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>

#include "kopeteview.h"

#include "irccontact.h"
#include "ircservercontact.h"
#include "ircaccount.h"
#include "ircprotocol.h"
#include "ksparser.h"

IRCServerContact::IRCServerContact(IRCContactManager *contactManager, const QString &servername, KopeteMetaContact *m)
	: IRCContact( contactManager, servername, m, QString::fromLatin1("irc_contact_server_")+servername )
{
	QObject::connect(m_engine, SIGNAL(internalError(KIRC::EngineError, const KIRCMessage &)),
			this, SLOT(engineInternalError(KIRC::EngineError, const KIRCMessage &)));
	QObject::connect(m_engine, SIGNAL(sentMessage(const KIRCMessage &)),
			this, SLOT(engineSentMessage(const KIRCMessage &)));
	QObject::connect(m_engine, SIGNAL(receivedMessage(const KIRCMessage &)),
			this, SLOT(engineReceivedMessage(const KIRCMessage &)));

	updateStatus();
}

void IRCServerContact::updateStatus()
{
	KIRC::EngineStatus status = m_engine->status();
	KopeteOnlineStatus kopeteStatus;
	switch( status )
	{
	case KIRC::Disconnected:
	case KIRC::Connecting:
		kopeteStatus = IRCProtocol::IRCServerOffline();
		break;
	case KIRC::Authentifying:
	case KIRC::Connected:
	case KIRC::Closing:
		// should make some extra check here
		kopeteStatus = IRCProtocol::IRCServerOnline();
		break;
	default:
		kopeteStatus = IRCProtocol::IRCUnknown();
	}
	setOnlineStatus( kopeteStatus );
}

void IRCServerContact::slotServerOnline(const QString &server)
{
	if( server.lower() == m_nickName.lower() )
	{
		setOnlineStatus( IRCProtocol::IRCServerOnline() );
	}
}

void IRCServerContact::slotServerOffline()
{
	setOnlineStatus( IRCProtocol::IRCServerOffline() );
}

const QString IRCServerContact::caption() const
{
	return m_engine->host();
}

KActionCollection *IRCServerContact::customContextMenuActions()
{

	m_customActions = new KActionCollection(this);
/*
	actionCtcpMenu = new KActionMenu(i18n("C&TCP"), 0, mCustomActions );
	actionCtcpMenu->insert( new KAction(i18n("&Version"), 0, this, SLOT(slotCtcpVersion()), actionCtcpMenu) );
	actionCtcpMenu->insert(  new KAction(i18n("&Ping"), 0, this, SLOT(slotCtcpPing()), actionCtcpMenu) );

	actionModeMenu = new KActionMenu(i18n("&Modes"), 0, mCustomActions, "actionModeMenu");
	actionModeMenu->insert( new KAction(i18n("&Op"), 0, this, SLOT(slotOp()), actionModeMenu, "actionOp") );
	actionModeMenu->insert( new KAction(i18n("&Deop"), 0, this, SLOT(slotDeop()), actionModeMenu, "actionDeop") );
	actionModeMenu->insert( new KAction(i18n("&Voice"), 0, this, SLOT(slotVoice()), actionModeMenu, "actionVoice") );
	actionModeMenu->insert( new KAction(i18n("Devoice"), 0, this, SLOT(slotDevoice()), actionModeMenu, "actionDevoice") );
	actionModeMenu->setEnabled( false );

	actionKick = new KAction(i18n("&Kick"), 0, this, SLOT(slotKick()), mCustomActions);
	actionKick->setEnabled( false );

	actionBanMenu = new KActionMenu(i18n("&Ban"), 0, mCustomActions, "actionBanMenu");
	actionBanMenu->insert( new KAction(i18n("Ban *!*@*.host"), 0, this, SLOT(slotBanHost()), actionBanMenu ) );
	actionBanMenu->insert( new KAction(i18n("Ban *!*@domain"), 0, this, SLOT(slotBanDomain()), actionBanMenu ) );
	actionBanMenu->insert( new KAction(i18n("Ban *!*user@*.host"), 0, this, SLOT(slotBanUserHost()), actionBanMenu ) );
	actionBanMenu->insert( new KAction(i18n("Ban *!*user@domain"), 0, this, SLOT(slotBanUserDomain()), actionBanMenu ) );
	actionBanMenu->setEnabled( false );

	//bool isOperator = ( chan->manager()->contactOnlineStatus( mAccount->myself() ) == IRCProtocol::IRCUserOp() );
	//actionModeMenu->setEnabled(isOperator);
	//actionBanMenu->setEnabled(isOperator);
	//actionKick->setEnabled(isOperator);
*/
	return m_customActions;
}

void IRCServerContact::engineInternalError(KIRC::EngineError engineError, const KIRCMessage &ircmsg)
{
	QString error;
	switch( engineError )
	{
	case KIRC::ParsingFailed:
		error = i18n("Parse error: ");
		break;
	case KIRC::UnknownCommand:
		error = i18n("Unknown command: ");
		break;
	case KIRC::InvalidNumberOfArguments:
		error = i18n("Invalid number of arguments: ");
		break;
	case KIRC::MethodFailed:
		error = i18n("Method failed: ");
		break;
	default:
		error = i18n("Unknown error: ");
	}

	KopeteMessage msg(this, manager()->members(), error+QString(ircmsg.raw()), KopeteMessage::Internal, KopeteMessage::PlainText, KopeteMessage::Chat);
	msg.setBody(m_account->protocol()->parser()->parse(msg.escapedBody().stripWhiteSpace()), KopeteMessage::RichText);
	appendMessage(msg);
}

void IRCServerContact::engineSentMessage(const KIRCMessage &ircmsg)
{
	KopeteMessage msg(m_account->myself(), manager()->members(), QString(ircmsg.raw()), KopeteMessage::Inbound, KopeteMessage::PlainText, KopeteMessage::Chat);
	msg.setBody(m_account->protocol()->parser()->parse(msg.escapedBody().stripWhiteSpace()), KopeteMessage::RichText);
	appendMessage(msg);
}

void IRCServerContact::engineReceivedMessage(const KIRCMessage &ircmsg)
{
	KopeteMessage msg(this, manager()->members(), QString(ircmsg.raw()), KopeteMessage::Inbound, KopeteMessage::PlainText, KopeteMessage::Chat);
	msg.setBody(m_account->protocol()->parser()->parse(msg.escapedBody().stripWhiteSpace()), KopeteMessage::RichText);
	appendMessage(msg);
}

void IRCServerContact::privateMessage(IRCContact *from, IRCContact *to, const QString &message)
{
/*	KopeteMessage msg( from, manager()->members(), i18n("to %1:").arg(to.accountId())+message, KopeteMessage::Inbound, KopeteMessage::PlainText, KopeteMessage::Chat );
	msg.setBody( m_account->protocol()->parser()->parse( msg.escapedBody() ), KopeteMessage::RichText );
	appendMessage(msg);*/
}

void IRCServerContact::action(IRCContact *from, IRCContact *to, const QString &message)
{
/*	KopeteMessage msg( from, manager()->members(), i18n("to %1:").arg(to.accountId())+message, KopeteMessage::Inbound, KopeteMessage::PlainText, KopeteMessage::Chat );
	msg.setBody( m_account->protocol()->parser()->parse( msg.escapedBody() ), KopeteMessage::RichText );
	appendMessage(msg);*/
}


void IRCServerContact::appendMessage( KopeteMessage &msg )
{
	msg.setImportance(KopeteMessage::Low); //to don't distrub the user
//	m_messageQueue.append(msg);
	if(manager(false) && manager()->view(false))
	{
		manager()->appendMessage(msg);
	}
}

void IRCServerContact::startServerChat()
{
	kdDebug(14120) << k_funcinfo << manager() << endl;

	if(manager()->view(false))
	{
		kdDebug(14120) << k_funcinfo << "Starting filled view." << endl;
		startChat();
//		manager()->view()->appendMessages(m_messageQueue);
	}
	else
	{
		kdDebug(14120) << k_funcinfo << "Raising view." << endl;
		startChat();
	}
}

#include "ircservercontact.moc"
