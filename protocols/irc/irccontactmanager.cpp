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

#include "kopetemetacontact.h"
#include "kopeteview.h"

#include "kirc.h"

#include "ircaccount.h"
#include "ircprotocol.h"

#include "ircservercontact.h"
#include "ircchannelcontact.h"
#include "ircusercontact.h"

#include "irccontactmanager.h"

IRCContactManager::IRCContactManager(const QString &nickName, const QString &serverName, IRCAccount *account, const char *name)
	: QObject(account, name),
	  m_account(account),
	  m_engine(account->engine())
{
	m_mySelf = findUser(nickName);
	m_myServer = findServer(serverName);

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

	m_NotifyTimer = new QTimer(this);
	QObject::connect(m_NotifyTimer, SIGNAL(timeout()),
			this, SLOT(checkOnlineNotifyList()));
	m_NotifyTimer->start(30000); // check online every 60sec
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
	//kdDebug(14120) << k_funcinfo << "o:" << originating << "; t:" << target << endl;
	IRCContact *from = findUser(originating.section('!', 0, 0));
	IRCChannelContact *to = findChannel(channel);
	emit privateMessage(from, to, message);
}

void IRCContactManager::slotNewPrivMessage(const QString &originating, const QString &user, const QString &message)
{
	//kdDebug(14120) << k_funcinfo << "o:" << originating << "; t:" << target << endl;
	IRCContact *from = findUser(originating.section('!', 0, 0));
	IRCUserContact *to = findUser(user);
	emit privateMessage(from, to, message);
}

void IRCContactManager::slotNewAction(const QString &originating, const QString &channel, const QString &message)
{
	kdDebug(14120) << k_funcinfo << "o:" << originating << "; t:" << channel << endl;
	IRCContact *from = findUser(originating.section('!', 0, 0));
	IRCChannelContact *to = findChannel(channel);

	emit action(from, to, message);
}

void IRCContactManager::slotNewPrivAction(const QString &originating, const QString &user, const QString &message)
{
	kdDebug(14120) << k_funcinfo << "o:" << originating << "; u:" << user << endl;
	IRCContact *from = findUser( originating.section('!', 0, 0) );
	IRCUserContact *to = findUser( user );

	emit action(from, to, message);
}

void IRCContactManager::unregister(KopeteContact *contact)
{
	unregisterServer(contact);
	unregisterChannel(contact, true);
	unregisterUser(contact, true);
}

IRCServerContact *IRCContactManager::findServer(const QString &serverName, KopeteMetaContact *m)
{
	if( !m )
	{
		m = new KopeteMetaContact();
		m->setTemporary( true );
	}

	QString lowerServerName = serverName.lower();
	IRCServerContact *server = 0;
	if ( !m_servers.contains( lowerServerName ) )
	{
		server = new IRCServerContact(this, serverName, m);
		m_servers.insert( lowerServerName, server );
		QObject::connect(server, SIGNAL(contactDestroyed(KopeteContact *)),
				this, SLOT(unregisterServer(KopeteContact *)));
	}
	else
	{
		server = m_servers[ lowerServerName ];
//		kdDebug(14120) << k_funcinfo << lowerServerName << " conversations:" << server->conversations() << endl;
	}

	return server;
}

void IRCContactManager::unregisterServer(const QString &name)
{
	QString lowerName = name.lower();
	if( m_servers.contains( lowerName ) )
	{
		IRCServerContact *server = m_servers[lowerName];
		if(!server->isChatting() && server->metaContact()->isTemporary())
		{
			kdDebug(14120) << k_funcinfo << name << endl;
//			delete server->metaContact();
		}
	}
}

void IRCContactManager::unregisterServer(KopeteContact *contact)
{
	const IRCServerContact *server = (const IRCServerContact *)contact;
	if(	server != 0 &&
		server != myServer() &&
		!server->isChatting())
	{
		m_servers.remove( server->nickName().lower() );
	}
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
			this, SLOT(unregister(KopeteContact *)));
	}
	else
	{
		channel = m_channels[ lowerName ];
//		kdDebug(14120) << k_funcinfo << lowerName << " conversations:" << channel->conversations() << endl;
	}

	return channel;
}

void IRCContactManager::unregisterChannel(const QString &name)
{
	QString lowerName = name.lower();
	if( m_channels.contains( lowerName ) )
	{
		IRCChannelContact *channel = m_channels[lowerName];
		if(!channel->isChatting() && channel->metaContact()->isTemporary())
		{
			kdDebug(14120) << k_funcinfo << name << endl;
//			delete channel->metaContact();
		}
	}
}

void IRCContactManager::unregisterChannel(KopeteContact *contact, bool force)
{
	const IRCChannelContact *channel = (const IRCChannelContact *)contact;
	if( force || (
		channel!=0 &&
		!channel->isChatting() &&
		channel->metaContact() ) )
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
				this, SLOT(unregister(KopeteContact *)));
	}
	else
	{
		user = m_users[ lowerName ];
//		kdDebug(14120) << k_funcinfo << lowerName << " conversations:" << user->conversations() << endl;
	}

	return user;
}

void IRCContactManager::unregisterUser( const QString & /* name */ )
{
/*	QString lowerName = name.lower();
	if( lowerName != mNickName.lower() && m_users.contains( lowerName ) )
	{
		IRCUserContact *user = m_users[lowerName];
		if(!user->isChatting() && user->metaContact()->isTemporary())
		{
			kdDebug(14120) << k_funcinfo << name << endl;
			delete user->metaContact();
		}
	}*/
}

void IRCContactManager::unregisterUser(KopeteContact *contact, bool force)
{
	const IRCUserContact *user = (const IRCUserContact *)contact;
	if( force || (
		user!=0 &&
		user!=mySelf() &&
		!user->isChatting() ) )
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
	if(m_engine->isConnected())
		m_engine->isOn( m_NotifyList );
}

#include "irccontactmanager.moc"
