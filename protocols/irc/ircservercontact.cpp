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
#include "irccontact.h"
#include "ircservercontact.h"
#include "ircaccount.h"
#include "ircprotocol.h"
#include "ksparser.h"

IRCServerContact::IRCServerContact(IRCContactManager *contactManager, const QString &servername, KopeteMetaContact *m)
	: IRCContact(contactManager, servername, m, "irc_server")
{
	QObject::connect(m_engine, SIGNAL(internalError(KIRC::EngineError, const KIRCMessage &)),
			this, SLOT(engineInternalError(KIRC::EngineError, const KIRCMessage &)));
	
	//FIXME: Have some kind of a debug option for raw input/ouput display??
	/*QObject::connect(m_engine, SIGNAL(sentMessage(const KIRCMessage &)),
			this, SLOT(engineSentMessage(const KIRCMessage &)));
	QObject::connect(m_engine, SIGNAL(receivedMessage(const KIRCMessage &)),
			this, SLOT(engineReceivedMessage(const KIRCMessage &)));*/
	
	QObject::connect( m_engine, SIGNAL(incomingNotice( const QString &, const QString &)),
			this, SLOT(slotIncomingNotice(const QString &, const QString &)) );
			
	QObject::connect( m_engine, SIGNAL(incomingUnknown( const QString &)),
			this, SLOT(slotAppendMessage(const QString &)) );
			
	QObject::connect( m_engine, SIGNAL(incomingConnectString( const QString &)),
			this, SLOT(slotAppendMessage(const QString &)) );
		
	//FIXME:: This shouldn't add MOTD stuffs when someone uses /motd	
	QObject::connect( m_engine, SIGNAL(incomingMotd( const QStringList &)),
			this, SLOT(slotIncomingMotd(const QStringList &)) );
			
	QObject::connect(KopeteMessageManagerFactory::factory(), SIGNAL(viewCreated(KopeteView*)),
			this, SLOT(slotViewCreated(KopeteView*)) );
			
	updateStatus();
}

void IRCServerContact::updateStatus()
{
	KIRC::EngineStatus status = m_engine->status();
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
	return i18n("%1 @ %2").arg( m_engine->nickName() ).arg( m_engine->host() );
}

void IRCServerContact::engineInternalError( KIRC::EngineError engineError, const KIRCMessage &ircmsg )
{
	QString error;
	switch( engineError )
	{
		case KIRC::ParsingFailed:
			error = i18n("KIRC Error - parse error: ");
			break;
		case KIRC::UnknownCommand:
			error = i18n("KIRC Error - Unknown command: ");
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

	KopeteContactPtrList members;
	members.append( this );
	KopeteMessage msg(this, members, error + QString( ircmsg.raw() ), KopeteMessage::Internal,
		KopeteMessage::PlainText, KopeteMessage::Chat);
	appendMessage(msg);
}

void IRCServerContact::slotSendMsg(KopeteMessage &, KopeteMessageManager *manager )
{
	KopeteMessage msg( manager->user(), manager->members(), 
		i18n("You can not talk to the server, you can only issue commands here. Type /help for supported commands."), KopeteMessage::Internal, KopeteMessage::PlainText, KopeteMessage::Chat);
	manager->appendMessage(msg);
}

void IRCServerContact::slotAppendMessage( const QString &message )
{
	KopeteContactPtrList members;
	members.append( this );
	KopeteMessage msg( this, members, message, KopeteMessage::Internal, 
		KopeteMessage::PlainText, KopeteMessage::Chat );
	msg.setBody( KSParser::parse( msg.escapedBody().stripWhiteSpace() ), KopeteMessage::RichText );
	appendMessage(msg);
}

void IRCServerContact::slotIncomingNotice( const QString &orig, const QString &notice )
{
	slotAppendMessage( i18n("NOTICE %1: %2").arg( orig ).arg( notice ) );
}

void IRCServerContact::slotIncomingMotd( const QStringList &motd )
{
	for( QStringList::ConstIterator it = motd.begin(); it != motd.end(); ++it )
		slotAppendMessage( *it );
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
	for( QValueList<KopeteMessage>::Iterator it = mMsgBuffer.begin(); it != mMsgBuffer.end(); ++it )
		manager()->appendMessage(*it);	
	mMsgBuffer.clear();
}

void IRCServerContact::slotViewCreated( KopeteView *v )
{
	kdDebug() << k_funcinfo << "Created: " << v->msgManager() << ", Mine: " << m_msgManager << endl;
	if( m_msgManager && v->msgManager() == m_msgManager )
		QTimer::singleShot( 500, this, SLOT( slotDumpMessages() ) );
}

#include "ircservercontact.moc"
