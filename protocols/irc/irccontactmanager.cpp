/*
    irccontactmanager.cpp - Manager of IRC Contacts

    Copyright (c) 2003      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2003      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qtimer.h>
#include <kstandarddirs.h>

#include "kopetemetacontact.h"
#include "kopeteview.h"

#include "kirc.h"

#include "ircaccount.h"
#include "ircprotocol.h"

#include "ircservercontact.h"
#include "ircchannelcontact.h"
#include "ircusercontact.h"

#include "irccontactmanager.h"

IRCContactManager::IRCContactManager(const QString &nickName, IRCAccount *account, const char *name)
	: QObject(account, name),
	  m_account(account),
	  m_engine(account->engine())
{
	m_mySelf = findUser(nickName);

	KopeteMetaContact *m = new KopeteMetaContact();
	m->setTemporary( true );
	m_myServer = new IRCServerContact(this, account->engine()->currentHost(), m);

	QObject::connect(m_engine, SIGNAL(incomingMessage(const QString &, const QString &, const QString &)),
			this, SLOT(slotNewMessage(const QString &, const QString &, const QString &)));

	QObject::connect(m_engine, SIGNAL(incomingPrivMessage(const QString &, const QString &, const QString &)),
			this, SLOT(slotNewPrivMessage(const QString &, const QString &, const QString &)));

	QObject::connect(m_engine, SIGNAL(incomingAction(const QString &, const QString &, const QString &)),
			this, SLOT(slotNewAction(const QString &, const QString &, const QString &)));

	QObject::connect(m_engine, SIGNAL(incomingPrivAction(const QString &, const QString &, const QString &)),
			this, SLOT(slotNewPrivAction(const QString &, const QString &, const QString &)));

	QObject::connect(m_engine, SIGNAL(incomingNickChange(const QString &, const QString &)),
			this, SLOT( slotNewNickChange(const QString&, const QString&)));

	QObject::connect(m_engine, SIGNAL(successfullyChangedNick(const QString &, const QString &)),
			this, SLOT( slotNewNickChange(const QString &, const QString &)));

	QObject::connect(m_engine, SIGNAL(userOnline(const QString &)),
			this, SLOT( slotIsonRecieved()));

	socketTimeout = 15000;
	QString timeoutPath = locate( "config", "kioslaverc" );
	if( !timeoutPath.isEmpty() )
	{
		KConfig config( timeoutPath );
		socketTimeout = config.readNumEntry( "ReadTimeout", 15 ) * 1000;
	}

	m_NotifyTimer = new QTimer(this);
	QObject::connect(m_NotifyTimer, SIGNAL(timeout()),
			this, SLOT(checkOnlineNotifyList()));
	m_NotifyTimer->start(30000); // check online every 30sec
}

void IRCContactManager::slotNewNickChange(const QString &oldnick, const QString &newnick)
{
	IRCUserContact *c =  m_users[ oldnick ];
	if( c )
	{
		m_users[ newnick ] = c;
		m_users.remove(oldnick);
	}
}

void IRCContactManager::slotNewMessage(const QString &originating, const QString &channel, const QString &message)
{
	IRCContact *from = findUser(originating.section('!', 0, 0));
	IRCChannelContact *to = findChannel(channel);
	emit privateMessage(from, to, message);
}

void IRCContactManager::slotNewPrivMessage(const QString &originating, const QString &user, const QString &message)
{
	IRCContact *from = findUser(originating.section('!', 0, 0));
	IRCUserContact *to = findUser(user);
	emit privateMessage(from, to, message);
}

void IRCContactManager::slotNewAction(const QString &originating, const QString &channel, const QString &message)
{
	IRCContact *from = findUser(originating.section('!', 0, 0));
	IRCChannelContact *to = findChannel(channel);

	emit action(from, to, message);
}

void IRCContactManager::slotNewPrivAction(const QString &originating, const QString &user, const QString &message)
{
	IRCContact *from = findUser( originating.section('!', 0, 0) );
	IRCUserContact *to = findUser( user );

	emit action(from, to, message);
}

void IRCContactManager::unregister(KopeteContact *contact)
{
	unregisterChannel(contact);
	unregisterUser(contact);
}

IRCChannelContact *IRCContactManager::findChannel(const QString &name, KopeteMetaContact *m)
{
	QString lowerName = name.lower();
	IRCChannelContact *channel = 0;

	if ( !m_channels.contains( lowerName ) )
	{
		if( !m )
		{
			m = new KopeteMetaContact();
			m->setTemporary( true );
		}

		channel = new IRCChannelContact(this, name, m);
		m_channels.insert( lowerName, channel );
		QObject::connect(channel, SIGNAL(contactDestroyed(KopeteContact *)),
			this, SLOT(unregisterChannel(KopeteContact *)));
	}
	else
	{
		channel = m_channels[ lowerName ];
	}

	return channel;
}

void IRCContactManager::unregisterChannel(KopeteContact *contact)
{
	const IRCChannelContact *channel = (const IRCChannelContact *)contact;
	if(	channel!=0 &&
		!channel->isChatting() &&
		channel->metaContact())
	{
		m_channels.remove( channel->nickName().lower() );
	}
}

IRCUserContact *IRCContactManager::findUser(const QString &name, KopeteMetaContact *m)
{
	QString lowerName = name.lower();
	IRCUserContact *user = 0;

	if ( !m_users.contains( lowerName ) )
	{
		if( !m )
		{
			m = new KopeteMetaContact();
			m->setTemporary( true );
		}

		user = new IRCUserContact(this, name, m);
		m_users.insert( lowerName, user );
		QObject::connect(user, SIGNAL(contactDestroyed(KopeteContact *)),
				this, SLOT(unregisterUser(KopeteContact *)));
	}
	else
	{
		user = m_users[ lowerName ];
	}

	return user;
}

void IRCContactManager::unregisterUser(KopeteContact *contact)
{
	const IRCUserContact *user = (const IRCUserContact *)contact;
	if(	user!=0 &&
		user!=mySelf() &&
		!user->isChatting())
	{
		kdDebug(14120) << k_funcinfo << user->nickName() << endl;
		m_users.remove( user->nickName().lower() );
	}
}

void IRCContactManager::addToNotifyList(const QString &nick)
{
 	if (!m_NotifyList.contains(nick.lower()))
	{
		m_NotifyList.append(nick);
		checkOnlineNotifyList();
	}
}

void IRCContactManager::removeFromNotifyList(const QString &nick)
{
	if (m_NotifyList.contains(nick.lower()))
		m_NotifyList.remove(nick.lower());
}

void IRCContactManager::checkOnlineNotifyList()
{
	if( m_engine->isConnected() )
	{
		isonRecieved = false;
		m_engine->isOn( m_NotifyList );
		QTimer::singleShot( socketTimeout, this, SLOT( slotIsonTimeout() ) );
	}
}

void IRCContactManager::slotIsonRecieved()
{
	isonRecieved = true;
}

void IRCContactManager::slotIsonTimeout()
{
	if( !isonRecieved )
		m_engine->quitIRC("", true);
}

#include "irccontactmanager.moc"
