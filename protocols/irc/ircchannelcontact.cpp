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
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopetestdaction.h"

#include <klocale.h>
#include <qsocket.h>
#include <kdebug.h>
#include <klineeditdlg.h>

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
	actionTopic = new KAction(i18n("Change &Topic"), 0, this, SLOT(setTopic()), this, "actionTopic");
	actionModeMenu = new KActionMenu(i18n("Channel Modes"), 0, this, "actionModeMenu");

	mCustomActions->insert( actionJoin );
	mCustomActions->insert( actionPart );
	mCustomActions->insert( actionTopic );
	mCustomActions->insert( actionModeMenu );

	actionModeT = new KToggleAction(i18n("Only Operators Can Change &Topic"), 0, this, SLOT(slotModeChanged()), actionModeMenu );
	actionModeN = new KToggleAction(i18n("&No Outside Messages"), 0, this, SLOT(slotModeChanged()), actionModeMenu );
	actionModeS = new KToggleAction(i18n("&Secret"), 0, this, SLOT(slotModeChanged()), actionModeMenu );
	actionModeM = new KToggleAction(i18n("&Moderated"), 0, this, SLOT(slotModeChanged()), actionModeMenu );
	actionModeI = new KToggleAction(i18n("&Invite Only"), 0, this, SLOT(slotModeChanged()), actionModeMenu );

	actionModeMenu->insert( actionModeT );
	actionModeMenu->insert( actionModeN );
	actionModeMenu->insert( actionModeS );
	actionModeMenu->insert( actionModeM );
	actionModeMenu->insert( actionModeI );

	// KIRC Engine stuff
	QObject::connect(identity->engine(), SIGNAL(connectedToServer()), this, SLOT(slotConnectedToServer()));
	QObject::connect(identity->engine(), SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
	QObject::connect(identity->engine(), SIGNAL(userJoinedChannel(const QString &, const QString &)), this, SLOT(slotUserJoinedChannel(const QString &, const QString &)));
	QObject::connect(identity->engine(), SIGNAL(incomingPartedChannel(const QString &, const QString &, const QString &)), this, SLOT(slotUserPartedChannel(const QString &, const QString &, const QString &)));
	QObject::connect(identity->engine(), SIGNAL(incomingNamesList(const QString &, const QString &, const int)), this, SLOT(slotNamesList(const QString &, const QString &, const int)));
	QObject::connect(identity->engine(), SIGNAL(incomingExistingTopic(const QString &, const QString &)), this, SLOT( slotChannelTopic(const QString&, const QString &)));
	QObject::connect(identity->engine(), SIGNAL(incomingTopicChange(const QString &, const QString &, const QString &)), this, SLOT( slotTopicChanged(const QString&,const QString&,const QString&)));
	QObject::connect(identity->engine(), SIGNAL(incomingModeChange(const QString&, const QString&, const QString&)), this, SLOT(slotIncomingModeChange(const QString&,const QString&, const QString&)));

	QObject::connect( this, SIGNAL( endSession() ), this, SLOT( slotPart() ) );

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
	// TODO: make this configurable: (if this channel is join on connetct, run this)
	//mEngine->joinChannel(mNickName);
	setOnlineStatus( KopeteContact::Online );
}

void IRCChannelContact::slotNamesList(const QString &channel, const QString &nickname, const int mode)
{
	QString newNick = nickname;
	if (channel.lower() == mNickName.lower())
	{
		if (newNick.startsWith("@") || newNick.startsWith("+"))
			newNick.remove(0, 1);

		IRCUserContact *user = new IRCUserContact(mIdentity, newNick, (KIRC::UserClass)mode);
		user->setOnlineStatus( KopeteContact::Online );
		manager()->addContact((KopeteContact *)user, true);
	}
}

void IRCChannelContact::slotChannelTopic(const QString &channel, const QString &topic)
{
	if( mNickName.lower() == channel.lower() )
	{
		mTopic = topic;
		manager()->setDisplayName( caption() );
		KopeteMessage msg((KopeteContact*)this, mContact, i18n("Topic for %1 is %2").arg(mNickName).arg(mTopic), KopeteMessage::Internal);
		manager()->appendMessage(msg);
	}
}

void IRCChannelContact::slotJoin()
{
	if ( onlineStatus() == KopeteContact::Online )
		mEngine->joinChannel(mNickName);
}

void IRCChannelContact::slotPart()
{
	mEngine->partChannel(mNickName, QString("Kopete 2.0: http://kopete.kde.org"));
}

void IRCChannelContact::slotUserJoinedChannel(const QString &user, const QString &channel)
{
	QString nickname = user.section('!', 0, 0);
	if (nickname.lower() == mEngine->nickName().lower() && channel.lower() == mNickName.lower())
	{
		KopeteMessage msg((KopeteContact *)this, mContact,
		i18n("You have joined channel %1").arg(mNickName), KopeteMessage::Internal);
		manager()->appendMessage(msg);
	}
	else {
		IRCUserContact *contact = new IRCUserContact(mIdentity, nickname, KIRC::Normal);
		contact->setOnlineStatus( KopeteContact::Online );
		manager()->addContact((KopeteContact *)contact, true);

		KopeteMessage msg((KopeteContact *)this, mContact,
		i18n("User %1 [%2] joined channel %3").arg(nickname).arg(user.section('!', 1)).arg(mNickName),
		KopeteMessage::Internal);
		manager()->appendMessage(msg);
	}
}

void IRCChannelContact::slotUserPartedChannel(const QString &user, const QString &channel, const QString &reason)
{
	QString nickname = user.section('!', 0, 0);
	if (! (nickname.lower() == mEngine->nickName().lower() && channel.lower() == mNickName.lower()) )
	{
		KopeteContact *user = locateUser( nickname );
		if ( user )
		{
			manager()->removeContact( user, true );
			delete user->metaContact();
		}
		KopeteMessage msg((KopeteContact *)this, mContact,
		i18n("User %1 parted channel %2 (%3)").arg(nickname).arg(mNickName).arg(reason),
		KopeteMessage::Internal);
		manager()->appendMessage(msg);
	}
}

void IRCChannelContact::setTopic( const QString &topic )
{
	bool okPressed = true;
	QString newTopic = topic;
	if( newTopic.isNull() )
		newTopic = KLineEditDlg::getText( i18n("New Topic"), i18n("Enter the new topic:"), mTopic, &okPressed, 0L );

	if( okPressed )
	{
		mTopic = newTopic;
		mEngine->setTopic( mNickName, newTopic );
	}
}

void IRCChannelContact::slotTopicChanged( const QString &channel, const QString &nick, const QString &newtopic )
{
	if( mNickName.lower() == channel.lower() )
	{
		mTopic = newtopic;
		mMsgManager->setDisplayName( caption() );
		KopeteMessage msg((KopeteContact *)this, mContact, i18n("%1 has changed the topic to %2").arg(nick).arg(newtopic), KopeteMessage::Internal);
		manager()->appendMessage(msg);
	}
}

void IRCChannelContact::slotIncomingModeChange( const QString &nick, const QString &channel, const QString &mode )
{
	if( mNickName.lower() == channel.lower() )
	{
		KopeteMessage msg((KopeteContact *)this, mContact, i18n("%1 sets mode %2 %3").arg(nick).arg(mode).arg(mNickName), KopeteMessage::Internal);
		manager()->appendMessage(msg);
	}
}

void IRCChannelContact::setMode( const QString &mode )
{
	mEngine->changeMode( mNickName, mode );
}

void IRCChannelContact::slotModeChanged()
{
	QString modeString;
	if( actionModeT->isChecked() )
		modeString.append( QString::fromLatin1("+t") );
	else
		modeString.append( QString::fromLatin1("-t") );

	if( actionModeN->isChecked() )
		modeString.append( QString::fromLatin1("+n") );
	else
		modeString.append( QString::fromLatin1("-n") );

	if( actionModeS->isChecked() )
		modeString.append( QString::fromLatin1("+s") );
	else
		modeString.append( QString::fromLatin1("-s") );

	if( actionModeM->isChecked() )
		modeString.append( QString::fromLatin1("+m") );
	else
		modeString.append( QString::fromLatin1("-m") );

	if( actionModeI->isChecked() )
		modeString.append( QString::fromLatin1("+i") );
	else
		modeString.append( QString::fromLatin1("-i") );

	setMode( modeString );
}

void IRCChannelContact::slotConnectionClosed()
{
	setOnlineStatus( KopeteContact::Offline );
}

KActionCollection *IRCChannelContact::customContextMenuActions()
{
	if ( onlineStatus() == KopeteContact::Offline || onlineStatus() == KopeteContact::Unknown )
	{
		actionJoin->setEnabled( true );
		actionPart->setEnabled( false );
		actionTopic->setEnabled( false );
		actionModeMenu->setEnabled( false );
	}
	else if ( onlineStatus() == KopeteContact::Online || onlineStatus() == KopeteContact::Away )
	{
		actionJoin->setEnabled( false );
		actionPart->setEnabled( true );
		actionTopic->setEnabled( true );
		actionModeMenu->setEnabled( true );
	}

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

const QString IRCChannelContact::caption() const
{
	return QString::fromLatin1("%1 @ %2 - %4").arg(mNickName).arg(mEngine->host()).arg(mTopic);
}

#include "ircchannelcontact.moc"
