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

#undef KDE_NO_COMPAT
#include <kaction.h>
#include <qtimer.h>

#include "ircusercontact.h"
#include "ircservercontact.h"
#include "ircchannelcontact.h"
#include "irccontactmanager.h"
#include "ircaccount.h"
#include "ircprotocol.h"
#include "kcodecaction.h"
#include "kopetemetacontact.h"
#include "kopeteview.h"

IRCUserContact::IRCUserContact(IRCContactManager *contactManager, const QString &nickname, KopeteMetaContact *m )
	: IRCContact(contactManager, nickname, m ),
	  m_isAway(false)
{
	mOnlineTimer = new QTimer( this );
	m_isOnline = m_metaContact->isTemporary();
	mOnlineTimer->start( 45000, true );

	QObject::connect(mOnlineTimer, SIGNAL(timeout()), this, SLOT( slotUserOffline() ) );

	QObject::connect(m_engine, SIGNAL(incomingChannelModeChange(const QString&, const QString&, const QString&)),
		this, SLOT(slotIncomingModeChange(const QString&,const QString&, const QString&)));

	actionCtcpMenu = 0L;

	mInfo.isOperator = false;
	mInfo.idle = 0;
	mInfo.hops = 0;
	mInfo.away = false;

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
			if(this == m_account->mySelf())
				setOnlineStatus(m_protocol->m_UserStatusConnecting);
			else
				setOnlineStatus(m_protocol->m_UserStatusOffline);
			break;

		case KIRC::Connected:
		case KIRC::Closing:
			if( m_isAway )
				setOnlineStatus(m_protocol->m_UserStatusAway);
			else if( m_isOnline )
				setOnlineStatus(m_protocol->m_UserStatusOnline);
			else
				setOnlineStatus(m_protocol->m_UserStatusOffline);
			break;

		default:
			setOnlineStatus(m_protocol->m_StatusUnknown);
	}
}

void IRCUserContact::slotUserOffline()
{
	m_isOnline = false;
	m_isAway = false;
	m_engine->writeMessage( QString::fromLatin1("WHOWAS %1").arg(m_nickName) );

	removeProperty( QString::fromLatin1("userInfo") );
	removeProperty( QString::fromLatin1("server") );
	removeProperty( QString::fromLatin1("channels") );
}

void IRCUserContact::setAway(bool isAway)
{
	m_isAway = isAway;
	updateStatus();
}

void IRCUserContact::incomingUserIsAway(const QString &reason )
{
	if( manager(false ) )
	{
		KopeteMessage msg( (KopeteContact*)m_account->myServer(), mMyself,
			i18n("%1 is away (%2)").arg( m_nickName ).arg( reason ),
			KopeteMessage::Internal, KopeteMessage::PlainText, KopeteMessage::Chat );
		manager()->appendMessage(msg);
	}
}

void IRCUserContact::userOnline()
{
	m_isOnline = true;
	mOnlineTimer->start( 45000, true );
	updateStatus();
	if( this != m_account->mySelf() && !metaContact()->isTemporary() )
	{
		m_engine->writeMessage( QString::fromLatin1("WHOIS %1").arg(m_nickName) );
	}

	removeProperty( QString::fromLatin1("lastOnline") );
}

void IRCUserContact::slotUserInfo()
{
	if( isChatting() )
		m_engine->whoisUser( m_nickName );
}

const QString IRCUserContact::caption() const
{
	return i18n("%1 @ %2").arg(m_nickName).arg( m_engine->currentHost() );
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
	KopeteContactPtrList members = mActiveManager->members();
	QString channelName = static_cast<IRCContact*>( members.first() )->nickName();
	m_engine->kickUser(m_nickName, channelName, QString::null);
}

void IRCUserContact::contactMode( const QString &mode )
{
	KopeteContactPtrList members = mActiveManager->members();
	QString channelName = static_cast<IRCContact*>( members.first() )->nickName();
	m_engine->changeMode( channelName, QString::fromLatin1("%1 %2").arg(mode).arg(m_nickName) );
}

void IRCUserContact::slotCtcpPing()
{
	m_engine->CtcpRequest_pingPong(m_nickName);
}

void IRCUserContact::slotCtcpVersion()
{
	m_engine->CtcpRequest_version(m_nickName);
}

void IRCUserContact::newWhoIsUser(const QString &username, const QString &hostname, const QString &realname)
{
	mInfo.channels.clear();
	mInfo.userName = username;
	mInfo.hostName = hostname;
	mInfo.realName = realname;

	if( onlineStatus().status() == KopeteOnlineStatus::Offline )
	{
		setProperty( QString::fromLatin1("userInfo"), i18n("User"), QString::fromLatin1("%1@%2")
			.arg(mInfo.userName).arg(mInfo.hostName) );
		setProperty( QString::fromLatin1("server"), i18n("Server"), mInfo.serverName );
	}
}

void IRCUserContact::newWhoIsServer(const QString &servername, const QString &serverinfo)
{
	mInfo.serverName = servername;
	if( metaContact()->isTemporary() || onlineStatus().status() == KopeteOnlineStatus::Online )
		mInfo.serverInfo = serverinfo;
	else
	{
		kdDebug(14120)<< "Setting last online: " << serverinfo << endl;
		setProperty( QString::fromLatin1("lastOnline"), i18n("Last Online"), QDateTime::fromString( serverinfo ) );
	}
}

void IRCUserContact::newWhoIsIdle(unsigned long idle)
{
	mInfo.idle = idle;
}

void IRCUserContact::newWhoIsOperator()
{
	mInfo.isOperator = true;
}

void IRCUserContact::newWhoIsChannels(const QString &channel)
{
	mInfo.channels.append( channel );
}

void IRCUserContact::whoIsComplete()
{
	updateInfo();

	if( m_protocol->commandInProgress() )
	{
		//User info
		QString msg = i18n("%1 is (%2@%3): %4\n")
			.arg(m_nickName)
			.arg(mInfo.userName)
			.arg(mInfo.hostName)
			.arg(mInfo.realName);

		if( mInfo.isOperator )
			msg += i18n("%1 is an IRC operator\n").arg(m_nickName);

		//Channels
		msg += i18n("on channels %1\n").arg(mInfo.channels.join(" ; "));

		//Server
		msg += i18n("on IRC via server %1 ( %2 )\n").arg(mInfo.serverName).arg(mInfo.serverInfo);

		//Idle
		QString idleTime = formattedIdleTime();
		msg += i18n("idle: %2\n").arg( idleTime.isEmpty() ? QString::number(0) : idleTime );

		//End
		m_account->appendMessage(msg, IRCAccount::InfoReply );
		m_protocol->setCommandInProgress(false);
	}
}

QString IRCUserContact::formattedName() const
{
	return mInfo.realName;
}

void IRCUserContact::updateInfo()
{
	setProperty( QString::fromLatin1("userInfo"), i18n("User"), QString::fromLatin1("%1@%2")
		.arg(mInfo.userName).arg(mInfo.hostName) );
	setProperty( QString::fromLatin1("server"), i18n("Server"), mInfo.serverName );
	setProperty( QString::fromLatin1("channels"), i18n("Channels"), mInfo.channels.join(" ") );
	setProperty( QString::fromLatin1("hops"), i18n("Hops"), QString::number(mInfo.hops) );

	setIdleTime( mInfo.idle );

	mInfo.lastUpdate = QTime::currentTime();
}

void IRCUserContact::newWhoReply( const QString &channel, const QString &user, const QString &host,
	const QString &server, bool away, const QString &flags, uint hops, const QString &realName )
{
	if( !mInfo.channels.contains( channel ) )
		mInfo.channels.append( channel );

	mInfo.userName = user;
	mInfo.hostName = host;
	mInfo.serverName = server;
	mInfo.flags = flags;
	mInfo.hops = hops;
	mInfo.realName = realName;

	setAway(away);

	updateInfo();

	if( m_protocol->commandInProgress() )
	{
		m_protocol->setCommandInProgress(false);
	}
}


QPtrList<KAction> *IRCUserContact::customContextMenuActions( KopeteMessageManager *manager )
{
	if( manager )
	{
		QPtrList<KAction> *mCustomActions = new QPtrList<KAction> ();
		mActiveManager = manager;
		KopeteContactPtrList members = mActiveManager->members();
		IRCChannelContact *isChannel = dynamic_cast<IRCChannelContact*>( members.first() );

		if( !actionCtcpMenu )
		{
			actionCtcpMenu = new KActionMenu(i18n("C&TCP"), 0, this );
			actionCtcpMenu->insert( new KAction(i18n("&Version"), 0, this,
				SLOT(slotCtcpVersion()), actionCtcpMenu) );
			actionCtcpMenu->insert(  new KAction(i18n("&Ping"), 0, this,
				SLOT(slotCtcpPing()), actionCtcpMenu) );

			actionModeMenu = new KActionMenu(i18n("&Modes"), 0, this, "actionModeMenu");
			actionModeMenu->insert( new KAction(i18n("&Op"), 0, this,
				SLOT(slotOp()), actionModeMenu, "actionOp") );
			actionModeMenu->insert( new KAction(i18n("&Deop"), 0, this,
				SLOT(slotDeop()), actionModeMenu, "actionDeop") );
			actionModeMenu->insert( new KAction(i18n("&Voice"), 0, this,
				SLOT(slotVoice()), actionModeMenu, "actionVoice") );
			actionModeMenu->insert( new KAction(i18n("Devoice"), 0, this,
				SLOT(slotDevoice()), actionModeMenu, "actionDevoice") );
			actionModeMenu->setEnabled( false );

			actionKick = new KAction(i18n("&Kick"), 0, this, SLOT(slotKick()), this);
			actionKick->setEnabled( false );

			actionBanMenu = new KActionMenu(i18n("&Ban"), 0, this, "actionBanMenu");
			actionBanMenu->insert( new KAction(i18n("Ban *!*@*.host"), 0, this,
				SLOT(slotBanHost()), actionBanMenu ) );
			actionBanMenu->insert( new KAction(i18n("Ban *!*@domain"), 0, this,
				SLOT(slotBanDomain()), actionBanMenu ) );
			actionBanMenu->insert( new KAction(i18n("Ban *!*user@*.host"), 0, this,
				 SLOT(slotBanUserHost()), actionBanMenu ) );
			actionBanMenu->insert( new KAction(i18n("Ban *!*user@domain"), 0, this,
				 SLOT(slotBanUserDomain()), actionBanMenu ) );
			actionBanMenu->setEnabled( false );

			codecAction = new KCodecAction( i18n("&Encoding"), 0, this, "selectcharset" );
			connect( codecAction, SIGNAL( activated( const QTextCodec * ) ),
				this, SLOT( setCodec( const QTextCodec *) ) );
			codecAction->setCodec( codec() );
		}

		mCustomActions->append( actionCtcpMenu );
		mCustomActions->append( actionModeMenu );
		mCustomActions->append( actionBanMenu );
		mCustomActions->append( codecAction );

		if( isChannel )
		{
			bool isOperator = ( manager->contactOnlineStatus( account()->myself() ) == m_protocol->m_UserStatusOp );
			actionModeMenu->setEnabled(isOperator);
			actionBanMenu->setEnabled(isOperator);
			actionKick->setEnabled(isOperator);
		}

		return mCustomActions;
	}

	mActiveManager = 0L;

	return 0L;
}

void IRCUserContact::slotIncomingModeChange( const QString &channel, const QString &nick, const QString &mode )
{
	IRCChannelContact *chan = m_account->contactManager()->findChannel( channel );
	if( chan->locateUser( m_nickName ) )
	{
		QString user = mode.section(' ', 1, 1);
		kdDebug(14120) << k_funcinfo << mode << ", " << user << ", " << m_nickName << endl;
		if( user == m_nickName )
		{
			QString modeChange = mode.section(' ', 0, 0);
			if(modeChange == QString::fromLatin1("+o"))
				chan->manager()->setContactOnlineStatus( this, m_protocol->m_UserStatusOp );
			else if(modeChange == QString::fromLatin1("-o"))
				chan->manager()->setContactOnlineStatus( this, m_protocol->m_UserStatusOnline );
			else if(modeChange == QString::fromLatin1("+v"))
				chan->manager()->setContactOnlineStatus( this, m_protocol->m_UserStatusVoice );
			else if(modeChange == QString::fromLatin1("-v"))
				chan->manager()->setContactOnlineStatus( this, m_protocol->m_UserStatusOnline );
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
	//Either this is from me to a guy, or from a guy to me. Either way its a PM
	if( to == this && from == m_account->mySelf() )
	{
		KopeteMessage msg(from, to->manager()->members(), action, KopeteMessage::Action, KopeteMessage::PlainText, KopeteMessage::Chat);
		to->appendMessage(msg);
	}
	else if( from == this && to == m_account->mySelf() )
	{
		KopeteMessage msg(from, from->manager()->members(), action, KopeteMessage::Action, KopeteMessage::PlainText, KopeteMessage::Chat);
		from->appendMessage(msg);
	}
}

#include "ircusercontact.moc"
