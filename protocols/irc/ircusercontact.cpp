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
#include "ircidentity.h"
#include "kopetemessagemanager.h"
#include "kopeteviewmanager.h"

#include "kirc.h"
#include "ksparser.h"

#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <qtimer.h>

IRCUserContact::IRCUserContact(IRCIdentity *identity, const QString &nickname, KopeteMetaContact *m)
	: IRCContact( identity, nickname, m )
{
	mNickName = nickname;

	mCustomActions = new KActionCollection(this);

	actionCtcpMenu = new KActionMenu(i18n("C&TCP"), 0, this );
	actionCtcpMenu->insert( new KAction(i18n("&Version"), 0, this, SLOT(slotCtcpVersion()), this) );
	actionCtcpMenu->insert(  new KAction(i18n("&Ping"), 0, this, SLOT(slotCtcpPing()), this) );
	mCustomActions->insert( actionCtcpMenu );

	actionModeMenu = new KActionMenu(i18n("&Modes"), 0, this, "actionModeMenu");
	actionModeMenu->insert( new KAction(i18n("&Op"), 0, this, SLOT(slotOp()), this, "actionOp") );
	actionModeMenu->insert( new KAction(i18n("&Deop"), 0, this, SLOT(slotDeop()), this, "actionDeop") );
	actionModeMenu->insert( new KAction(i18n("&Voice"), 0, this, SLOT(slotVoice()), this, "actionVoice") );
	actionModeMenu->insert( new KAction(i18n("Devoice"), 0, this, SLOT(slotDevoice()), this, "actionDevoice") );
	actionModeMenu->setEnabled( false );
	mCustomActions->insert( actionModeMenu );

	actionKick = new KAction(i18n("&Kick"), 0, this, SLOT(slotKick()), this);
	mCustomActions->insert( actionKick );

	actionBanMenu = new KActionMenu(i18n("&Ban"), 0, this, "actionBanMenu");
	actionBanMenu->insert( new KAction(i18n("Ban *!*@*.host"), 0, this, SLOT(slotBanHost()), this ) );
	actionBanMenu->insert( new KAction(i18n("Ban *!*@domain"), 0, this, SLOT(slotBanDomain()), this ) );
	actionBanMenu->insert( new KAction(i18n("Ban *!*user@*.host"), 0, this, SLOT(slotBanUserHost()), this ) );
	actionBanMenu->insert( new KAction(i18n("Ban *!*user@domain"), 0, this, SLOT(slotBanUserDomain()), this ) );
	mCustomActions->insert( actionBanMenu );

	mOnlineTimer = new QTimer( this );
	connect( mOnlineTimer, SIGNAL(timeout()), this, SLOT( slotUserOffline() ) );

	QObject::connect(identity->engine(), SIGNAL(incomingModeChange(const QString&, const QString&, const QString&)), this, SLOT(slotIncomingModeChange(const QString&,const QString&, const QString&)));
	QObject::connect(identity->engine(), SIGNAL(incomingPrivMessage(const QString &, const QString &, const QString &)), this, SLOT(slotNewPrivMessage(const QString &, const QString &, const QString &)));
	QObject::connect(identity->engine(), SIGNAL(userOnline( const QString & )), this, SLOT(slotUserOnline(const QString &)));

	isConnected = false;
}

IRCUserContact::~IRCUserContact()
{
	delete mOnlineTimer;
}

KopeteMessageManager* IRCUserContact::manager(bool)
{
	if (!mMsgManager)
	{
		kdDebug(14120) << k_funcinfo << "Creating new KMM for " << mNickName << endl;

		mMsgManager = KopeteMessageManagerFactory::factory()->create( (KopeteContact *)mIdentity->mySelf(), mContact, (KopeteProtocol *)mIdentity->protocol());
		mMsgManager->setDisplayName( caption() );
		QObject::connect( mMsgManager, SIGNAL(messageSent(KopeteMessage&, KopeteMessageManager *)), this, SLOT(slotSendMsg(KopeteMessage&, KopeteMessageManager *)));
		QObject::connect( mMsgManager, SIGNAL(destroyed()), this, SLOT(slotMessageManagerDestroyed()));
		isConnected = true;
	}
	return mMsgManager;
}

void IRCUserContact::slotMessageManagerDestroyed()
{
	mIdentity->unregisterUser( mNickName );
	mMsgManager = 0L;
	isConnected = false;
}

void IRCUserContact::slotUserOnline( const QString &nick )
{
	if( nick.lower() == mNickName.lower() )
	{
		setOnlineStatus( KopeteContact::Online );
		mOnlineTimer->start( 90000, true );
	}
}

void IRCUserContact::slotUserOffline()
{
	setOnlineStatus( KopeteContact::Offline );
}

QString IRCUserContact::statusIcon() const
{
	/*if (mUserclass == KIRC::Operator)
		return "irc_op";

	if (mUserclass == KIRC::Voiced)
		return "irc_voice";*/

	return "irc_normal";
}

void IRCUserContact::slotNewPrivMessage(const QString &originating, const QString &, const QString &message)
{
	//kdDebug(14120) << k_funcinfo << "o:" << originating << "; t:" << target << endl;
	if ( originating.section('!',0,0).lower() == mNickName.lower() )
	{
		KopeteMessage msg( (KopeteContact*)this, mMyself, message, KopeteMessage::Inbound, KopeteMessage::PlainText, KopeteMessage::Chat );
		msg.setBody( mParser->parse( msg.escapedBody() ), KopeteMessage::RichText );
		manager()->appendMessage(msg);
	}
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
	KopeteView *activeView = KopeteViewManager::viewManager()->activeView();
	if( activeView && activeView->msgManager()->user()->inherits("IRCUserContact") )
	{
		QString channelName = activeView->msgManager()->displayName().section(' ', 0, 0);
		mEngine->kickUser(mNickName, channelName, QString::null);
	}
}

void IRCUserContact::contactMode( const QString &mode )
{
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
	if( mChannels.contains( channel.lower() ) )
	{
		QString user = mode.section(' ', 1, 1);
		if( user == mNickName )
		{
			QString modeChange = mode.section(' ', 0, 0);
			if(modeChange == QString::fromLatin1("+o"))
				mUserClassMap[channel.lower()] = KIRC::Operator;
			else if(modeChange == QString::fromLatin1("-o"))
				mUserClassMap[channel.lower()] = KIRC::Normal;
			else if(modeChange == QString::fromLatin1("+v"))
				mUserClassMap[channel.lower()] = KIRC::Voiced;
			else if(modeChange == QString::fromLatin1("-v"))
				mUserClassMap[channel.lower()] = KIRC::Normal;
		}

		bool isOperator = mIdentity->mySelf()->userclass(channel) == KIRC::Operator;
		actionModeMenu->setEnabled(isOperator);
		actionBanMenu->setEnabled(isOperator);
		actionKick->setEnabled(isOperator);
	}
}

#include "ircusercontact.moc"
