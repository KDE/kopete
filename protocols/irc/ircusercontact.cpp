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
#include "ircidentity.h"
#include "kopetemessagemanager.h"
#include "kopeteviewmanager.h"

#include "kirc.h"
#include "ksparser.h"

#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>

IRCUserContact::IRCUserContact(IRCIdentity *identity, const QString &nickname, KIRC::UserClass userclass, KopeteMetaContact *m)
	: IRCContact( identity, nickname, m )
{
	mConversations = 0;
	mUserclass = userclass;
	mNickName = nickname;
	setDisplayName(mNickName);

	mCustomActions = new KActionCollection(this);

	actionCtcpMenu = new KActionMenu(i18n("C&TCP"), 0, this );
	actionCtcpPing = new KAction(i18n("&Ping"), 0, this, SLOT(slotCtcpPing()), this);
	actionCtcpVersion = new KAction(i18n("&Version"), 0, this, SLOT(slotCtcpVersion()), this);
	actionCtcpMenu->insert( actionCtcpVersion );
	actionCtcpMenu->insert( actionCtcpPing );
	mCustomActions->insert( actionCtcpMenu );

	actionModeMenu = new KActionMenu(i18n("&Modes"), 0, this, "actionModeMenu");
	actionOp = new KAction(i18n("&Op"), 0, this, SLOT(slotOp()), this, "actionOp");
	actionDeop = new KAction(i18n("&Deop"), 0, this, SLOT(slotDeop()), this, "actionDeop");
	actionVoice = new KAction(i18n("&Voice"), 0, this, SLOT(slotVoice()), this, "actionVoice");
	actionDevoice = new KAction(i18n("Devoice"), 0, this, SLOT(slotDevoice()), this, "actionDevoice");
	actionModeMenu->insert( actionOp );
	actionModeMenu->insert( actionDeop );
	actionModeMenu->insert( actionVoice );
	actionModeMenu->insert( actionDevoice );

	actionModeMenu->setEnabled( false );
	mCustomActions->insert( actionModeMenu );

	QObject::connect(identity->engine(), SIGNAL(incomingModeChange(const QString&, const QString&, const QString&)), this, SLOT(slotIncomingModeChange(const QString&,const QString&, const QString&)));
	QObject::connect(identity->engine(), SIGNAL(incomingPrivMessage(const QString &, const QString &, const QString &)), this, SLOT(slotNewPrivMessage(const QString &, const QString &, const QString &)));
}

QString IRCUserContact::statusIcon() const
{
	if (mUserclass == KIRC::Operator)
		return "irc_op";

	if (mUserclass == KIRC::Voiced)
		return "irc_voice";

	return "irc_normal";
}

void IRCUserContact::slotNewPrivMessage(const QString &originating, const QString &target, const QString &message)
{
	//kdDebug(14120) << k_funcinfo << "o:" << originating << "; t:" << target << endl;
	if (originating.section('!',0,0).lower() == mNickName.lower())
	{
		KopeteMessage msg( (KopeteContact*)this, mMyself, message, KopeteMessage::Inbound, KopeteMessage::PlainText, KopeteMessage::Chat );
		msg.setBody( mParser->parse( msg.escapedBody() ), KopeteMessage::RichText );
		manager()->appendMessage(msg);
	}
}

void IRCUserContact::slotUserInfo()
{
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

void IRCUserContact::contactMode( const QString &mode )
{
	KopeteView *activeView = KopeteViewManager::viewManager()->activeView();
	if( activeView && activeView->msgManager()->user()->inherits("IRCContact") )
	{
		if( activeView->msgManager()->displayName().startsWith( QString::fromLatin1("#") ) )
		{
			QString channelName = activeView->msgManager()->displayName().section(' ', 0, 0);
			mEngine->changeMode( channelName, QString::fromLatin1("%1 %2").arg(mode).arg(mNickName) );
		}
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

void IRCUserContact::slotIncomingModeChange( const QString &, const QString &, const QString &mode )
{
	QString user = mode.section(' ', 1, 1);
	if( user == mNickName )
	{
		QString modeChange = mode.section(' ', 0, 0);
		if(modeChange == QString::fromLatin1("+o"))
			mUserclass = KIRC::Operator;
		else if(modeChange == QString::fromLatin1("-o"))
			mUserclass = KIRC::Normal;
		else if(modeChange == QString::fromLatin1("+v"))
			mUserclass = KIRC::Voiced;
		else if(modeChange == QString::fromLatin1("-v"))
			mUserclass = KIRC::Normal;

		emit( onlineStatusChanged( static_cast<KopeteContact*>(this), onlineStatus() ) );
	}
	else if( user == mIdentity->mySelf()->nickName() )
	{
		actionModeMenu->setEnabled( mIdentity->mySelf()->userclass() == KIRC::Operator );
	}
}

#include "ircusercontact.moc"
