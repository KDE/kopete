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
#include "ircchannelcontact.h"
#include "ircaccount.h"
#include "ircprotocol.h"
#include "kcodecaction.h"
#include "kopetemetacontact.h"

IRCUserContact::IRCUserContact(IRCContactManager *contactManager, const QString &nickname, KopeteMetaContact *m )
	: IRCContact(contactManager, nickname, m ),
	  m_isAway(false)
{
	mOnlineTimer = new QTimer( this );
	m_isOnline = m_metaContact->isTemporary();

	QObject::connect(mOnlineTimer, SIGNAL(timeout()), this, SLOT( slotUserOffline() ) );

	QObject::connect(m_engine, SIGNAL(incomingModeChange(const QString&, const QString&, const QString&)),
		this, SLOT(slotIncomingModeChange(const QString&,const QString&, const QString&)));
	QObject::connect(m_engine, SIGNAL(userOnline( const QString & )),
		this, SLOT(slotUserOnline(const QString &)));
	QObject::connect(m_engine, SIGNAL(incomingUserIsAway( const QString &, const QString & )),
		this, SLOT(slotIncomingUserIsAway(const QString &, const QString &)));

	actionCtcpMenu = 0L;

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
	updateStatus();
}

void IRCUserContact::setAway(bool isAway)
{
	m_isAway = isAway;
	updateStatus();
}

void IRCUserContact::slotIncomingUserIsAway( const QString &nick, const QString &reason )
{
	if( nick.lower() == m_nickName.lower() )
	{
		/*
		Uncomment after string freeze
		if( manager(false ) )
		{
			KopeteMessage msg( , to->manager()->members(), i18n("%1 is away (%2)")
				.arg( m_nickName ).arg( reason ), KopeteMessage::Internal, KopeteMessage::PlainText, KopeteMessage::Chat );
			manager()->appendMessage(msg);
		}*/
	}
}

void IRCUserContact::slotUserOnline( const QString &nick )
{
	if( nick.lower() == m_nickName.lower() )
	{
		m_isOnline = true;
		mOnlineTimer->start( 40000, true );
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
	m_engine->sendCtcpPing(m_nickName);
}

void IRCUserContact::slotCtcpVersion()
{
	m_engine->sendCtcpVersion(m_nickName);
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

void IRCUserContact::slotIncomingModeChange( const QString &, const QString &channel, const QString &mode )
{
	IRCChannelContact *chan = m_account->findChannel( channel );
	if( chan->locateUser( m_nickName ) )
	{
		QString user = mode.section(' ', 1, 1);
		kdDebug(14120) << k_funcinfo << mode << ", " << user << ", " << m_nickName << endl;
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
