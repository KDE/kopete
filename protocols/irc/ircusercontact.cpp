/*
    ircusercontact.cpp - IRC User Contact

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002      by the Kopete developers <kopete-devel@kde.org>

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
#include <klocale.h>
#include <kaction.h>
#include <qtimer.h>

#include "ircusercontact.h"
#include "ircchannelcontact.h"
#include "ircaccount.h"
#include "ircprotocol.h"
#include "ksparser.h"

IRCUserContact::IRCUserContact(IRCContactManager *contactManager, const QString &nickname, KopeteMetaContact *m)
	: IRCContact(contactManager, nickname, m),
	  m_isAway(false)
{
	mOnlineTimer = new QTimer( this );

	QObject::connect(m_engine, SIGNAL(incomingModeChange(const QString&, const QString&, const QString&)),
		this, SLOT(slotIncomingModeChange(const QString&,const QString&, const QString&)));
	QObject::connect(m_engine, SIGNAL(userOnline( const QString & )),
		this, SLOT(slotUserOnline(const QString &)));

	updateStatus();
}

void IRCUserContact::updateStatus()
{
	KIRC::EngineStatus status = m_engine->status();
	switch( status )
	{
	case KIRC::Disconnected:
		setOnlineStatus(m_protocol->m_UserStatusOffline);
		break;
	case KIRC::Connecting:
	case KIRC::Authentifying:
		setOnlineStatus(m_protocol->m_UserStatusConnecting);
		break;
	case KIRC::Connected:
	case KIRC::Closing:
		// FIXME: should make some extra check here
//		if(m_isOnline)
//			setOnlineStatus(m_protocol->m_UserStatusOnline);
		if(m_isAway)
			setOnlineStatus(m_protocol->m_UserStatusAway);
		else
//			setOnlineStatus(m_protocol->m_UserStatusOffline);
			setOnlineStatus(m_protocol->m_UserStatusOnline);
		break;
	default:
		setOnlineStatus(m_protocol->m_StatusUnknown);
	}
}

void IRCUserContact::setAway(bool isAway)
{
	m_isAway = isAway;
	updateStatus();
}

void IRCUserContact::slotUserOnline( const QString &nick )
{
	if( nick.lower() == m_nickName.lower() )
	{
		mOnlineTimer->start( 60000, true );
		updateStatus();
	}
}

void IRCUserContact::slotUserInfo()
{
	if( isChatting() )
		m_engine->whoisUser( m_nickName );
}

const QString IRCUserContact::caption() const
{
	return i18n("%1 @ %2").arg(m_nickName).arg(m_engine->host());
}

void IRCUserContact::slotOp()
{
	contactMode( QString::fromLatin1("+o") );
}

void IRCUserContact::slotDeop()
{
	contactMode( QString::fromLatin1("-o") );
}

void IRCUserContact::slotVoice()
{
	contactMode( QString::fromLatin1("+v") );
}

void IRCUserContact::slotDevoice()
{
	contactMode( QString::fromLatin1("-v") );
}

void IRCUserContact::slotBanHost()
{
	slotKick();
}

void IRCUserContact::slotBanUserHost()
{
	slotKick();
}

void IRCUserContact::slotBanDomain()
{
	slotKick();
}

void IRCUserContact::slotBanUserDomain()
{
	slotKick();
}

void IRCUserContact::slotKick()
{
	/* TODO: rewrite this code
	KopeteView *activeView = KopeteViewManager::viewManager()->activeView();
	if( activeView && activeView->msgManager()->user()->inherits("IRCUserContact") )
	{
		QString channelName = activeView->msgManager()->displayName().section(' ', 0, 0);
		mEngine->kickUser(mNickName, channelName, QString::null);
	}*/
}

void IRCUserContact::contactMode( const QString & /* mode */ )
{
	/* TODO: rewrite this code
	KopeteView *activeView = KopeteViewManager::viewManager()->activeView();
	if( activeView && activeView->msgManager()->user()->inherits("IRCUserContact") )
	{
		QString channelName = activeView->msgManager()->displayName().section(' ', 0, 0);
		mEngine->changeMode( channelName, QString::fromLatin1("%1 %2").arg(mode).arg(mNickName) );
	}*/
}

void IRCUserContact::slotCtcpPing()
{
	m_engine->sendCtcpPing(m_nickName);
}

void IRCUserContact::slotCtcpVersion()
{
	m_engine->sendCtcpVersion(m_nickName);
}

KActionCollection *IRCUserContact::customContextMenuActions()
{
	mCustomActions = new KActionCollection(this);

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

	return mCustomActions;
}

void IRCUserContact::slotIncomingModeChange( const QString &, const QString &channel, const QString &mode )
{
	IRCChannelContact *chan = m_account->findChannel( channel );
	if( chan->locateUser( m_nickName ) )
	{
		QString user = mode.section(' ', 1, 1);
		kdDebug(14120) << k_funcinfo << user << ", " << m_nickName << endl;
		if( user == m_nickName )
		{
			QString modeChange = mode.section(' ', 0, 0);
			if(modeChange == QString::fromLatin1("+o"))
				chan->manager()->setContactOnlineStatus( static_cast<const KopeteContact*>(this), m_protocol->m_UserStatusOp );
			else if(modeChange == QString::fromLatin1("-o"))
				chan->manager()->setContactOnlineStatus( static_cast<const KopeteContact*>(this), m_protocol->m_UserStatusOnline );
			else if(modeChange == QString::fromLatin1("+v"))
				chan->manager()->setContactOnlineStatus( static_cast<const KopeteContact*>(this), m_protocol->m_UserStatusVoice );
			else if(modeChange == QString::fromLatin1("-v"))
				chan->manager()->setContactOnlineStatus( static_cast<const KopeteContact*>(this), m_protocol->m_UserStatusOnline );
		}
	}
}

void IRCUserContact::privateMessage(IRCContact *from, IRCContact *to, const QString &message)
{
	if(to == this)
	{
		if(to==account()->myself())
		{
			KopeteMessage msg(from, from->manager()->members(), message, KopeteMessage::Inbound, KopeteMessage::PlainText, KopeteMessage::Chat);
			msg.setBody( KSParser::parse( msg.escapedBody() ), KopeteMessage::RichText );
			from->appendMessage(msg);
		}
		else
		{
			kdDebug(14120) << "IRC Server error: Received a private message for " << to->nickName() << ":" << message << endl;
			// emit/call something on main ircservercontact
		}
	}
}

void IRCUserContact::action(IRCContact *from, IRCContact *to, const QString &action)
{
	if(to == this)
	{
		if(to==account()->myself())
		{
			KopeteMessage msg(from, from->manager()->members(), action, KopeteMessage::Action, KopeteMessage::PlainText, KopeteMessage::Chat);
			msg.setBody( KSParser::parse( msg.escapedBody() ), KopeteMessage::RichText );
			from->appendMessage(msg);
		}
		else
		{
			kdDebug(14120) << "IRC Server error: Received an action message for " << to->nickName() << ":" << action << endl;
			// emit/call something on main ircservercontact
		}
	}
}

#include "ircusercontact.moc"
