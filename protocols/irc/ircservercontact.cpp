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
#include <qtimer.h>

#undef KDE_NO_COMPAT
#include <kaction.h>

#include "kopetemessagemanagerfactory.h"
#include "kopeteview.h"
#include "ircusercontact.h"
#include "ircservercontact.h"
#include "ircaccount.h"
#include "ircprotocol.h"

IRCServerContact::IRCServerContact(IRCContactManager *contactManager, const QString &servername, KopeteMetaContact *m)
	: IRCContact(contactManager, servername, m, "irc_server")
{
	QObject::connect(MYACCOUNT->engine(), SIGNAL(internalError(KIRC::EngineError, const KIRCMessage &)),
			this, SLOT(engineInternalError(KIRC::EngineError, const KIRCMessage &)));

	//FIXME: Have some kind of a debug option for raw input/ouput display??
	/*QObject::connect(m_engine, SIGNAL(sentMessage(const KIRCMessage &)),
			this, SLOT(engineSentMessage(const KIRCMessage &)));
	QObject::connect(m_engine, SIGNAL(receivedMessage(const KIRCMessage &)),
			this, SLOT(engineReceivedMessage(const KIRCMessage &)));*/

	QObject::connect( MYACCOUNT->engine(), SIGNAL(incomingNotice( const QString &, const QString &)),
			this, SLOT(slotIncomingNotice(const QString &, const QString &)) );

	QObject::connect( MYACCOUNT->engine(), SIGNAL(incomingCannotSendToChannel( const QString &, const QString &)),
			this, SLOT(slotCannotSendToChannel(const QString &, const QString &)) );

	QObject::connect( MYACCOUNT->engine(), SIGNAL(incomingUnknown( const QString &)),
			this, SLOT(slotIncomingUnknown(const QString &)) );

	QObject::connect( MYACCOUNT->engine(), SIGNAL(incomingConnectString( const QString &)),
			this, SLOT(slotIncomingConnect(const QString &)) );

	QObject::connect( MYACCOUNT->engine(), SIGNAL(incomingMotd( const QString &)),
			this, SLOT(slotIncomingMotd(const QString &)) );

	QObject::connect(KopeteMessageManagerFactory::factory(), SIGNAL(viewCreated(KopeteView*)),
			this, SLOT(slotViewCreated(KopeteView*)) );

	updateStatus();
}

void IRCServerContact::updateStatus()
{
	KIRC::EngineStatus status = MYACCOUNT->engine()->status();
	switch( status )
	{
		case KIRC::Disconnected:
		case KIRC::Connecting:
			setOnlineStatus( m_protocol->m_ServerStatusOffline );
			break;

		case KIRC::Authentifying:
		case KIRC::Connected:
		case KIRC::Closing:
			// should make some extra check here
			setOnlineStatus( m_protocol->m_ServerStatusOnline );
			break;

		default:
			setOnlineStatus( m_protocol->m_StatusUnknown );
	}
}

const QString IRCServerContact::caption() const
{
	return i18n("%1 @ %2").arg( MYACCOUNT->mySelf()->nickName() ).arg( MYACCOUNT->engine()->currentHost() );
}

void IRCServerContact::engineInternalError( KIRC::EngineError engineError, const KIRCMessage &ircmsg )
{
	QString error;
	switch( engineError )
	{
		case KIRC::ParsingFailed:
			error = i18n("KIRC Error - Parse error: ");
			break;
		case KIRC::UnknownCommand:
			error = i18n("KIRC Error - Unknown command: ");
			break;
		case KIRC::UnknownNumericReply:
			error = i18n("KIRC Error - Unknown numeric reply: ");
			break;
		case KIRC::InvalidNumberOfArguments:
			error = i18n("KIRC Error - Invalid number of arguments: ");
			break;
		case KIRC::MethodFailed:
			error = i18n("KIRC Error - Method failed: ");
			break;
		default:
			error = i18n("KIRC Error - Unknown error: ");
	}

	MYACCOUNT->appendMessage( error + QString( ircmsg.raw() ), IRCAccount::ErrorReply );
}

void IRCServerContact::slotSendMsg(KopeteMessage &, KopeteMessageManager *manager )
{
	manager->messageSucceeded();
	KopeteMessage msg( manager->user(), manager->members(),
		i18n("You can not talk to the server, you can only issue commands here. Type /help for supported commands."), KopeteMessage::Internal, KopeteMessage::PlainText, KopeteMessage::Chat);
	manager->appendMessage(msg);
}

void IRCServerContact::appendMessage( const QString &message )
{
	KopeteContactPtrList members;
	members.append( this );
	KopeteMessage msg( this, members, message, KopeteMessage::Internal,
		KopeteMessage::RichText, KopeteMessage::Chat );
	appendMessage(msg);
}

void IRCServerContact::slotIncomingNotice( const QString &orig, const QString &notice )
{
	MYACCOUNT->appendMessage( i18n("NOTICE from %1: %2").arg( orig.section('!',0,0) ).arg( notice ), IRCAccount::NoticeReply );
}

void IRCServerContact::slotIncomingUnknown( const QString &message )
{
	MYACCOUNT->appendMessage( message , IRCAccount::UnknownReply );
}

void IRCServerContact::slotIncomingConnect( const QString &message )
{
	MYACCOUNT->appendMessage( message , IRCAccount::ConnectReply );
}

void IRCServerContact::slotIncomingMotd( const QString &message )
{
	MYACCOUNT->appendMessage( message , IRCAccount::InfoReply );
}

void IRCServerContact::slotCannotSendToChannel( const QString &channel, const QString &message )
{
	MYACCOUNT->appendMessage( QString::fromLatin1("%1: %2").arg( channel ).arg( message ), IRCAccount::ErrorReply );
}

void IRCServerContact::appendMessage( KopeteMessage &msg )
{
	msg.setImportance( KopeteMessage::Low ); //to don't distrub the user

	if( m_msgManager && m_msgManager->view(false) )
		m_msgManager->appendMessage(msg);
	else
		mMsgBuffer.append( msg );
}

void IRCServerContact::slotDumpMessages()
{
	if( !mMsgBuffer.isEmpty() )
	{
		manager()->appendMessage( mMsgBuffer.front() );
		mMsgBuffer.pop_front();
		QTimer::singleShot( 0, this, SLOT( slotDumpMessages() ) );
	}
}

void IRCServerContact::slotViewCreated( KopeteView *v )
{
	kdDebug(14121) << k_funcinfo << "Created: " << v << ", mgr: " << v->msgManager() << ", Mine: " << m_msgManager << endl;
	if( m_msgManager && v->msgManager() == m_msgManager )
		QTimer::singleShot( 500, this, SLOT( slotDumpMessages() ) );
}

#include "ircservercontact.moc"
