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

#include "ircusercontact.h"
#include "ircchannelcontact.h"
#include "ircaccount.h"
#include "ircprotocol.h"
#include "kopetemessagemanager.h"
#include "kopeteviewmanager.h"
#include "kopeteview.h"

#include "kirc.h"

#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <qtimer.h>

IRCUserContact::IRCUserContact(IRCAccount *account, const QString &nickname, KopeteMetaContact *m)
	: IRCContact( account, nickname, m )
{
	mNickName = nickname;

	mCustomActions = new KActionCollection(this);

	actionCtcpMenu = new KActionMenu(i18n("C&TCP"), 0, mCustomActions );
	actionCtcpMenu->insert( new KAction(i18n("&Version"), 0, this, SLOT(slotCtcpVersion()), this) );
	actionCtcpMenu->insert(  new KAction(i18n("&Ping"), 0, this, SLOT(slotCtcpPing()), this) );

	actionModeMenu = new KActionMenu(i18n("&Modes"), 0, mCustomActions, "actionModeMenu");
	actionModeMenu->insert( new KAction(i18n("&Op"), 0, this, SLOT(slotOp()), this, "actionOp") );
	actionModeMenu->insert( new KAction(i18n("&Deop"), 0, this, SLOT(slotDeop()), this, "actionDeop") );
	actionModeMenu->insert( new KAction(i18n("&Voice"), 0, this, SLOT(slotVoice()), this, "actionVoice") );
	actionModeMenu->insert( new KAction(i18n("Devoice"), 0, this, SLOT(slotDevoice()), this, "actionDevoice") );
	actionModeMenu->setEnabled( false );

	actionKick = new KAction(i18n("&Kick"), 0, this, SLOT(slotKick()), mCustomActions);

	actionBanMenu = new KActionMenu(i18n("&Ban"), 0, mCustomActions, "actionBanMenu");
	actionBanMenu->insert( new KAction(i18n("Ban *!*@*.host"), 0, this, SLOT(slotBanHost()), this ) );
	actionBanMenu->insert( new KAction(i18n("Ban *!*@domain"), 0, this, SLOT(slotBanDomain()), this ) );
	actionBanMenu->insert( new KAction(i18n("Ban *!*user@*.host"), 0, this, SLOT(slotBanUserHost()), this ) );
	actionBanMenu->insert( new KAction(i18n("Ban *!*user@domain"), 0, this, SLOT(slotBanUserDomain()), this ) );

	mOnlineTimer = new QTimer( this );
	connect( mOnlineTimer, SIGNAL(timeout()), this, SLOT( slotUserOffline() ) );

	QObject::connect(account->engine(), SIGNAL(incomingModeChange(const QString&, const QString&, const QString&)), this, SLOT(slotIncomingModeChange(const QString&,const QString&, const QString&)));
	QObject::connect(account->engine(), SIGNAL(userOnline( const QString & )), this, SLOT(slotUserOnline(const QString &)));

	isConnected = false;

	setOnlineStatus( IRCProtocol::IRCUserOffline() );
}

KopeteMessageManager* IRCUserContact::manager(bool)
{
	if (!mMsgManager)
	{
		kdDebug(14120) << k_funcinfo << "Creating new KMM for " << mNickName << endl;

		mMsgManager = KopeteMessageManagerFactory::factory()->create( mAccount->myself(), mMyself, (KopeteProtocol *)mAccount->protocol());
		mMsgManager->setDisplayName( caption() );
		QObject::connect( mMsgManager, SIGNAL(messageSent(KopeteMessage&, KopeteMessageManager *)), this, SLOT(slotSendMsg(KopeteMessage&, KopeteMessageManager *)));
		QObject::connect( mMsgManager, SIGNAL(destroyed()), this, SLOT(slotMessageManagerDestroyed()));
		isConnected = true;
	}
	return mMsgManager;
}

void IRCUserContact::slotMessageManagerDestroyed()
{
	mAccount->unregisterUser( mNickName );
	mMsgManager = 0L;
	isConnected = false;
}

void IRCUserContact::slotUserOnline( const QString &nick )
{
	if( nick.lower() == mNickName.lower() )
	{
		setOnlineStatus( IRCProtocol::IRCUserOnline() );
		mOnlineTimer->start( 60000, true );
	}
}

void IRCUserContact::slotUserOffline()
{
	setOnlineStatus( IRCProtocol::IRCUserOffline() );
}

void IRCUserContact::slotUserInfo()
{
	if( conversations() > 0 )
		mEngine->whoisUser( mNickName );
}

const QString IRCUserContact::caption() const
{
	return i18n("%1 @ %2").arg(mNickName).arg(mEngine->host());
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
	//FIXME: Eh! what is this UGLY thing!
	KopeteView *activeView = KopeteViewManager::viewManager()->activeView();
	if( activeView && activeView->msgManager()->user()->inherits("IRCUserContact") )
	{
		QString channelName = activeView->msgManager()->displayName().section(' ', 0, 0);
		mEngine->kickUser(mNickName, channelName, QString::null);
	}
}

void IRCUserContact::contactMode( const QString &mode )
{
	//FIXME: Eh! what is this UGLY thing!
	KopeteView *activeView = KopeteViewManager::viewManager()->activeView();
	if( activeView && activeView->msgManager()->user()->inherits("IRCUserContact") )
	{
		QString channelName = activeView->msgManager()->displayName().section(' ', 0, 0);
		mEngine->changeMode( channelName, QString::fromLatin1("%1 %2").arg(mode).arg(mNickName) );
	}
}

void IRCUserContact::slotCtcpPing()
{
	mEngine->sendCtcpPing(mNickName);
}

void IRCUserContact::slotCtcpVersion()
{
	mEngine->sendCtcpVersion(mNickName);
}

void IRCUserContact::slotIncomingModeChange( const QString &, const QString &channel, const QString &mode )
{
	IRCChannelContact *chan = mAccount->findChannel( channel );
	if( chan->locateUser( mNickName ) )
	{
		QString user = mode.section(' ', 1, 1);
		if( user == mNickName )
		{
			QString modeChange = mode.section(' ', 0, 0);
			if(modeChange == QString::fromLatin1("+o"))
				manager()->setContactOnlineStatus( static_cast<const KopeteContact*>(this), IRCProtocol::IRCUserOp() );
			else if(modeChange == QString::fromLatin1("-o"))
				manager()->setContactOnlineStatus( static_cast<const KopeteContact*>(this), IRCProtocol::IRCUserOnline() );
			else if(modeChange == QString::fromLatin1("+v"))
				manager()->setContactOnlineStatus( static_cast<const KopeteContact*>(this), IRCProtocol::IRCUserVoice() );
			else if(modeChange == QString::fromLatin1("-v"))
				manager()->setContactOnlineStatus( static_cast<const KopeteContact*>(this), IRCProtocol::IRCUserOnline() );
		}

		bool isOperator = ( chan->manager()->contactOnlineStatus( mAccount->myself() ) == IRCProtocol::IRCUserOp() );
		actionModeMenu->setEnabled(isOperator);
		actionBanMenu->setEnabled(isOperator);
		actionKick->setEnabled(isOperator);
	}
}

#include "ircusercontact.moc"
