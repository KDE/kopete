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
#include "ircusercontact.h"
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
		IRCContact( identity, channel, metac )
{
	// Variable assignments
	mNickName = channel;

	// Registers this IRCChannelContact with the identity
	identity->registerChannel(mNickName, this);

	// Contact list display name
	setDisplayName(channel);

	// KAction stuff
	mCustomActions = new KActionCollection(this);
	actionJoin = new KAction(i18n("&Join"), 0, this, SLOT(slotJoin()), this, "actionJoin");
	actionPart = new KAction(i18n("&Part"), 0, this, SLOT(slotPart()), this, "actionPart");

	// KIRC Engine stuff
	QObject::connect(identity->engine(), SIGNAL(connectedToServer()), this, SLOT(slotConnectedToServer()));
	QObject::connect(identity->engine(), SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
	QObject::connect(identity->engine(), SIGNAL(userJoinedChannel(const QString &, const QString &)), this, SLOT(slotUserJoinedChannel(const QString &, const QString &)));
	QObject::connect(identity->engine(), SIGNAL(incomingPartedChannel(const QString &, const QString &, const QString &)), this, SLOT(slotUserPartedChannel(const QString &, const QString &, const QString &)));
	QObject::connect(identity->engine(), SIGNAL(incomingNamesList(const QString &, const QString &, const int)), this, SLOT(slotNamesList(const QString &, const QString &, const int)));

	connect( this, SIGNAL( endSession() ), this, SLOT( slotPart() ) );
	
	// TODO: make this configurable: (join on load)
	if (mEngine->state() == QSocket::Idle)
		identity->engine()->connectToServer(identity->mySelf()->nickName());
}

IRCChannelContact::~IRCChannelContact()
{
	mIdentity->unregisterChannel(mNickName);
}

void IRCChannelContact::slotConnectedToServer()
{
	// TODO: make this configurable: (on connect, join)
	mEngine->joinChannel(mNickName);
}

void IRCChannelContact::slotNamesList(const QString &channel, const QString &nickname, const int mode)
{
	QString newNick = nickname;
	if (channel.lower() == mNickName.lower())
	{
		if (newNick.startsWith("@") || newNick.startsWith("+"))
			newNick.remove(0, 1);

		IRCUserContact *user = new IRCUserContact(mIdentity, newNick, (KIRC::UserClass)mode);
		manager()->addContact((KopeteContact *)user);
	}
}

void IRCChannelContact::slotJoin()
{
	if ( onlineStatus() == KopeteContact::Offline )
		mEngine->joinChannel(mNickName);
}

void IRCChannelContact::slotPart()
{
	if ( onlineStatus() == KopeteContact::Online || onlineStatus() == KopeteContact::Away)
	{
		mEngine->partChannel(mNickName, QString("Kopete 2.0: http://kopete.kde.org"));
		manager()->deleteLater();
	}
}

void IRCChannelContact::slotUserJoinedChannel(const QString &user, const QString &channel)
{
	QString nickname = user.section('!', 0, 0);
	if (nickname.lower() == mEngine->nickName().lower() && channel.lower() == mNickName.lower())
	{
		setOnlineStatus( KopeteContact::Online ); // We joined the channel, change status

		KopeteMessage msg((KopeteContact *)this, mContact,
		i18n(QString("You have joined channel %1").arg(mNickName)), KopeteMessage::Internal);
		manager()->appendMessage(msg);
	}
	else {
		IRCUserContact *contact = new IRCUserContact(mIdentity, nickname, KIRC::Normal);
		manager()->addContact((KopeteContact *)contact);

		KopeteMessage msg((KopeteContact *)this, mContact,
		i18n(QString("User %1[%2] joined channel %3").arg(nickname).arg(user.section('!', 1))
		.arg(mNickName)), KopeteMessage::Internal);
		manager()->appendMessage(msg);
	}
}

void IRCChannelContact::slotUserPartedChannel(const QString &user, const QString &channel, const QString &reason)
{
	QString nickname = user.section('!', 0, 0);
	if (nickname.lower() == mEngine->nickName().lower() && channel.lower() == mNickName.lower())
		setOnlineStatus( KopeteContact::Offline ); // We parted the channel, change status
	else {
		KopeteContact *user = locateUser( nickname );
		if ( user )
		{
			manager()->removeContact( user );
			delete user;
		}
		KopeteMessage msg((KopeteContact *)this, mContact,
		i18n(QString("User %1 parted channel %2 (%3)").arg(nickname).arg(mNickName).arg(reason)),
		KopeteMessage::Internal);
		manager()->appendMessage(msg);
	}
}

void IRCChannelContact::slotConnectionClosed()
{
	setOnlineStatus( KopeteContact::Offline );
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

#include "ircchannelcontact.moc"
