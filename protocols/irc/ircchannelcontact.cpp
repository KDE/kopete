/***************************************************************************
                          ircchannelcontact.cpp  -  description
                             -------------------
    begin                : Thu Feb 20 2003
    copyright            : (C) 2003 by nbetcher
    email                : nbetcher@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ircchannelcontact.h"
#include "ircidentity.h"

#include "kirc.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopetestdaction.h"

#include <klocale.h>
#include <qsocket.h>
#include <kdebug.h>

IRCChannelContact::IRCChannelContact(IRCIdentity *identity, const QString &channel, KopeteMetaContact *metac) :
		IRCContact( identity, metac )
{
	// Variable assignments
	mChannelName = channel;

	// Registers this IRCChannelContact with the identity
	identity->registerChannel(mChannelName, this);

	// Contact list display name
	setDisplayName(channel);

	// KAction stuff
	mCustomActions = new KActionCollection(this);
	actionJoin = new KAction(i18n("&Join"), 0, this, SLOT(slotJoin()), this, "actionJoin");
	actionPart = new KAction(i18n("&Part"), 0, this, SLOT(slotPart()), this, "actionPart");

	// KopeteMessageManagerFactory stuff
	mContact.append((KopeteContact *)this);
	mMyself.append((KopeteContact *)identity->mySelf());

	// KIRC Engine stuff
	QObject::connect(identity->engine(), SIGNAL(connectedToServer()), this, SLOT(slotConnectedToServer()));
	QObject::connect(identity->engine(), SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
	QObject::connect(identity->engine(), SIGNAL(userJoinedChannel(const QString &, const QString &)), this, SLOT(slotUserJoinedChannel(const QString &, const QString &)));
	QObject::connect(identity->engine(), SIGNAL(incomingPartedChannel(const QString &, const QString &, const QString &)), this, SLOT(slotUserPartedChannel(const QString &, const QString &, const QString &)));
	QObject::connect(identity->engine(), SIGNAL(incomingMessage(const QString &, const QString &, const QString &)), this, SLOT(slotNewMessage(const QString &, const QString &, const QString &)));
	QObject::connect(identity->engine(), SIGNAL(incomingNamesList(const QString &, const QString &, const int)), this, SLOT(slotNamesList(const QString &, const QString &, const int)));

	// TODO: make this configurable: (on connect, join)
	if (mEngine->state() == QSocket::Idle)
		identity->engine()->connectToServer(identity->mySelf()->nickName());

}

IRCChannelContact::~IRCChannelContact()
{
	mIdentity->unregisterChannel(mChannelName);
}

KopeteMessageManager* IRCChannelContact::manager(bool)
{
	if (!mMsgManager)
	{
		kdDebug(14120) << k_funcinfo << "Creating new KMM" << endl;

		KopeteContactPtrList initialContact;
		initialContact.append((KopeteContact *)mIdentity->mySelf());
		mMsgManager = KopeteMessageManagerFactory::factory()->create( (KopeteContact *)mIdentity->mySelf(), initialContact, (KopeteProtocol *)mIdentity->protocol());
		QObject::connect( mMsgManager, SIGNAL(messageSent(KopeteMessage&, KopeteMessageManager *)), this, SLOT(slotSendMsg(KopeteMessage&, KopeteMessageManager *)));
		QObject::connect( mMsgManager, SIGNAL(destroyed()), this, SLOT(slotMessageManagerDestroyed()));
		if( mEngine->isLoggedIn() )
			mEngine->joinChannel(mChannelName);
	}
	return mMsgManager;
}

void IRCChannelContact::slotMessageManagerDestroyed()
{
	mMsgManager = 0L;
}

void IRCChannelContact::slotConnectedToServer()
{
	// We've connected to the server, join this channel
	mEngine->joinChannel(mChannelName);
}

void IRCChannelContact::slotNewMessage(const QString &originating, const QString &target, const QString &message)
{
	kdDebug(14120) << "[IRCProtocol]: slotNewMessage: originating is " << originating << " target is " << target << endl;
	if (target.lower() == mChannelName.lower())
	{
		QString nickname = originating.section('!', 0, 0);
		kdDebug(14120) << "[IRCProtocol]: slotNewMessage: originating is mChannelName, finding user " << nickname << endl;
		IRCChanPrivUser *user = mMembers[nickname];
		if (user)
		{
			KopeteMessage msg((KopeteContact *)user, mContact, message, KopeteMessage::Inbound);
			manager()->appendMessage(msg);
		}
	}
}

void IRCChannelContact::slotNamesList(const QString &channel, const QString &nickname, const int mode)
{
	QString newNick = nickname;
	if (channel.lower() == mChannelName.lower())
	{
		if (newNick.startsWith("@") || newNick.startsWith("+"))
			newNick.remove(0, 1);

		IRCChanPrivUser *user = new IRCChanPrivUser(mIdentity, newNick, (KIRC::UserClass)mode);
		mMembers.insert(nickname, user);
		manager()->addContact((KopeteContact *)user);
	}
}

void IRCChannelContact::slotJoin()
{
	if ( onlineStatus() == KopeteContact::Offline )
		mEngine->joinChannel(mChannelName);
}

void IRCChannelContact::slotPart()
{
	if ( onlineStatus() == KopeteContact::Online || onlineStatus() == KopeteContact::Away)
		mEngine->partChannel(mChannelName, QString("Kopete 2.0: http://kopete.kde.org"));
}

void IRCChannelContact::slotUserJoinedChannel(const QString &user, const QString &channel)
{
	QString nickname = user.section('!', 0, 0);
	if (nickname.lower() == mEngine->nickName().lower() && channel.lower() == mChannelName.lower())
	{
		setOnlineStatus( KopeteContact::Online ); // We joined the channel, change status

		KopeteMessage msg((KopeteContact *)this, mContact,
		i18n(QString("You have joined channel %1").arg(mChannelName)), KopeteMessage::Internal);
		manager()->appendMessage(msg);
	}
	else {
		IRCChanPrivUser *contact = new IRCChanPrivUser(mIdentity, nickname, KIRC::Normal);
		mMembers.insert(nickname, contact);
		manager()->addContact((KopeteContact *)contact);

		KopeteMessage msg((KopeteContact *)this, mContact,
		i18n(QString("User %1[%2] joined channel %3").arg(nickname).arg(user.section('!', 1))
		.arg(mChannelName)), KopeteMessage::Internal);
		manager()->appendMessage(msg);
	}
}

void IRCChannelContact::slotUserPartedChannel(const QString &user, const QString &channel, const QString &reason)
{
	QString nickname = user.section('!', 0, 0);
	if (nickname.lower() == mEngine->nickName().lower() && channel.lower() == mChannelName.lower())
		setOnlineStatus( KopeteContact::Offline ); // We parted the channel, change status
	else {
		IRCChanPrivUser *user = mMembers[nickname];
		mMembers.remove(nickname);
		if (user)
		{
			manager()->removeContact((KopeteContact *)user);
			delete user;
		}
		KopeteMessage msg((KopeteContact *)this, mContact,
		i18n(QString("User %1 parted channel %2 (%3)").arg(nickname).arg(mChannelName).arg(reason)),
		KopeteMessage::Internal);
		manager()->appendMessage(msg);
	}
}

void IRCChannelContact::slotConnectionClosed()
{
	setOnlineStatus( KopeteContact::Offline );
}

void IRCChannelContact::slotSendMsg(KopeteMessage &message, KopeteMessageManager *)
{
	if( onlineStatus() != KopeteContact::Online )
		mEngine->joinChannel(mChannelName);

	if( processMessage( message ) )
	{
		// If the above was false, there was a server command
		mEngine->messageContact(mChannelName, message.plainBody());
		manager()->appendMessage(message);
	}

	manager()->messageSucceeded();
}

KActionCollection *IRCChannelContact::customContextMenuActions()
{
	if ( onlineStatus() == KopeteContact::Offline || onlineStatus() == KopeteContact::Unknown )
		mCustomActions->insert(actionJoin);
	else if ( onlineStatus() == KopeteContact::Online || onlineStatus() == KopeteContact::Away )
		mCustomActions->insert(actionPart);

	return mCustomActions;
}

bool IRCChannelContact::isReachable()
{
	if ( onlineStatus() != KopeteContact::Offline && onlineStatus() != KopeteContact::Unknown )
		return true;

	return false;
}

QString IRCChannelContact::statusIcon() const
{
	if ( onlineStatus() == KopeteContact::Online || onlineStatus() == KopeteContact::Away )
		return "irc_protocol_small";
	return "irc_protocol_offline";
}

IRCChanPrivUser::IRCChanPrivUser(IRCIdentity *identity, const QString &nickname, KIRC::UserClass userclass)
	: IRCContact( identity, 0L )
{
	mUserclass = userclass;
	mNickname = nickname;
	setDisplayName(mNickname);
}

QString IRCChanPrivUser::statusIcon() const
{
	if (mUserclass == KIRC::Operator)
		return "irc_op";
	else if (mUserclass == KIRC::Voiced)
		return "irc_voice";

	return "irc_normal";
}

#include "ircchannelcontact.moc"
