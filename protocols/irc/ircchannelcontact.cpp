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

IRCChannelContact::IRCChannelContact(IRCIdentity *identity, const QString &channel, KopeteMetaContact *metac)
	: KopeteContact((KopeteProtocol *)identity->protocol(), QString("%1:%2@%3/%4")
	.arg(identity->mySelf()->nickName()).arg(identity->engine()->password())
	.arg(identity->engine()->host()).arg(channel), metac)
{
	// Variable assignments
	mChannelName = channel;
	mIdentity = identity;
	mEngine = mIdentity->engine();
	mMetaContact = metac;
	mMsgManager = 0L;

	// Registers this IRCChannelContact with the identity
	mIdentity->registerChannel(mChannelName, this);

	// Contact list display name
	setDisplayName(channel);

	// KAction stuff
	mCustomActions = new KActionCollection(this);
	actionJoin = new KAction(i18n("&Join"), 0, this, SLOT(slotJoin()), this, "actionJoin");
	actionPart = new KAction(i18n("&Part"), 0, this, SLOT(slotPart()), this, "actionPart");

	// KopeteMessageManagerFactory stuff
	mContact.append((KopeteContact *)this);
	mMyself.append((KopeteContact *)mIdentity->mySelf());

	// KIRC Engine stuff
	QObject::connect(mEngine, SIGNAL(connectedToServer()), this, SLOT(slotConnectedToServer()));
	QObject::connect(mEngine, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
	QObject::connect(mEngine, SIGNAL(userJoinedChannel(const QString &, const QString &)), this, SLOT(slotUserJoinedChannel(const QString &, const QString &)));
	QObject::connect(mEngine, SIGNAL(incomingPartedChannel(const QString &, const QString &, const QString &)), this, SLOT(slotUserPartedChannel(const QString &, const QString &, const QString &)));
	QObject::connect(mEngine, SIGNAL(incomingMessage(const QString &, const QString &, const QString &)), this, SLOT(slotNewMessage(const QString &, const QString &, const QString &)));
	QObject::connect(mEngine, SIGNAL(incomingNamesList(const QString &, const QString &, const int)), this, SLOT(slotNamesList(const QString &, const QString &, const int)));

	// TODO: make this configurable: (on connect, join)
	if (mEngine->state() == QSocket::Idle)
		mEngine->connectToServer(identity->mySelf()->nickName());

}

IRCChannelContact::~IRCChannelContact()
{
	mIdentity->unregisterChannel(mChannelName);
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
	if (mStatus == KopeteContact::Offline || mStatus == KopeteContact::Unknown)
		mEngine->joinChannel(mChannelName);
}

void IRCChannelContact::slotPart()
{
	if (mStatus == KopeteContact::Online || mStatus == KopeteContact::Away)
		mEngine->partChannel(mChannelName, QString("Kopete 2.0: http://kopete.kde.org"));
}

void IRCChannelContact::slotUserJoinedChannel(const QString &user, const QString &channel)
{
	QString nickname = user.section('!', 0, 0);
	if (nickname.lower() == mEngine->nickName().lower() && channel.lower() == mChannelName.lower())
	{
		mStatus = KopeteContact::Online; // We joined the channel, change status

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
		mStatus = KopeteContact::Offline; // We parted the channel, change status
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
	mStatus = KopeteContact::Offline;
}

KopeteMessageManager* IRCChannelContact::manager(bool)
{
	if (!mMsgManager)
	{
		KopeteContactPtrList initialContact;
		initialContact.append((KopeteContact *)mIdentity->mySelf());
		mMsgManager = KopeteMessageManagerFactory::factory()->create( (KopeteContact *)mIdentity->mySelf(), initialContact, (KopeteProtocol *)mIdentity->protocol());
		QObject::connect( mMsgManager, SIGNAL(messageSent(KopeteMessage&, KopeteMessageManager *)), this, SLOT(slotSendMsg(KopeteMessage&, KopeteMessageManager *)));
		QObject::connect( mMsgManager, SIGNAL(destroyed()), this, SLOT(slotMessageManagerDestroyed()));
	}
	return mMsgManager;
}

void IRCChannelContact::slotMessageManagerDestroyed()
{
	mMsgManager = 0L;
}

void IRCChannelContact::slotSendMsg(KopeteMessage &message, KopeteMessageManager *)
{
	mEngine->messageContact(mChannelName, message.plainBody());
	manager()->appendMessage(message);
	manager()->messageSucceeded();
}

KActionCollection *IRCChannelContact::customContextMenuActions()
{
	if (mStatus == KopeteContact::Offline || mStatus == KopeteContact::Unknown)
		mCustomActions->insert(actionJoin);
	else if (mStatus == KopeteContact::Online || mStatus == KopeteContact::Away)
		mCustomActions->insert(actionPart);

	return mCustomActions;
}

bool IRCChannelContact::isReachable()
{
	if (mStatus != KopeteContact::Offline && mStatus != KopeteContact::Unknown)
		return true;

	return false;
}

QString IRCChannelContact::statusIcon() const
{
	if (mStatus == KopeteContact::Online || mStatus == KopeteContact::Away)
		return "irc_protocol_small";
	return "irc_protocol_offline";
}

IRCChanPrivUser::IRCChanPrivUser(IRCIdentity *identity, const QString &nickname, KIRC::UserClass userclass)
	: KopeteContact((KopeteProtocol *)identity->protocol(), QString("%1@%3/%4")
	.arg(identity->mySelf()->nickName()).arg(identity->engine()->host()).arg(nickname), 0L)
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
