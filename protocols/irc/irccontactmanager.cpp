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

#include "ircusercontact.h"
#include "ircaccount.h"
#include "irccontactmanager.h"
#include "ircprotocol.h"
#include "ircsignalhandler.h"

#include "ircservercontact.h"
#include "ircchannelcontact.h"

#include "kircengine.h"

#include <kopeteaccountmanager.h>
#include <kopetemetacontact.h>
#include <kopetecontactlist.h>
#include <kopeteview.h>

#include <kconfig.h>
#include <kstandarddirs.h>

#include <qtimer.h>

IRCContactManager::IRCContactManager(const QString &nickName, IRCAccount *account, const char *name)
	: QObject(account, name),
	  m_channels( QDict<IRCChannelContact>( 17, false ) ),
	  m_users( QDict<IRCUserContact>( 577, false ) ),
	  m_account( account )
{
	m_mySelf = findUser(nickName);

	Kopete::MetaContact *m = new Kopete::MetaContact();
//	m->setTemporary( true );
	m_myServer = new IRCServerContact(this, account->networkName(), m);

	QObject::connect(account->engine(), SIGNAL(incomingMessage(const QString &, const QString &, const QString &)),
			this, SLOT(slotNewMessage(const QString &, const QString &, const QString &)));

	QObject::connect(account->engine(), SIGNAL(incomingPrivMessage(const QString &, const QString &, const QString &)),
			this, SLOT(slotNewPrivMessage(const QString &, const QString &, const QString &)));

	QObject::connect(account->engine(), SIGNAL(incomingNickChange(const QString &, const QString &)),
			this, SLOT( slotNewNickChange(const QString&, const QString&)));

	QObject::connect(account->engine(), SIGNAL(successfullyChangedNick(const QString &, const QString &)),
			this, SLOT( slotNewNickChange(const QString &, const QString &)));

	QObject::connect(account->engine(), SIGNAL(incomingUserOnline(const QString &)),
			this, SLOT( slotIsonRecieved()));

	QObject::connect(Kopete::ContactList::self(), SIGNAL(metaContactAdded( Kopete::MetaContact * )),
			this, SLOT( slotContactAdded( Kopete::MetaContact* )));

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

	new IRCSignalHandler(this);
}

void IRCContactManager::slotNewNickChange(const QString &oldnick, const QString &newnick)
{
	IRCUserContact *c =  m_users[ oldnick ];
	if( c )
	{
		m_users.insert(newnick, c);
		m_users.remove(oldnick);
	}
}

void IRCContactManager::slotNewMessage(const QString &originating, const QString &channel, const QString &message)
{
	IRCContact *from = findUser(originating);
	IRCChannelContact *to = findChannel(channel);
	emit privateMessage(from, to, message);
}

void IRCContactManager::slotNewPrivMessage(const QString &originating, const QString &user, const QString &message)
{
	IRCContact *from = findUser(originating);
	IRCUserContact *to = findUser(user);
	emit privateMessage(from, to, message);
}

void IRCContactManager::unregister(Kopete::Contact *contact)
{
	unregisterChannel(contact, true);
	unregisterUser(contact, true);
}

QValueList<IRCChannelContact*> IRCContactManager::findChannelsByMember( IRCUserContact *contact )
{
	QValueList<IRCChannelContact*> retVal;
	for( QDictIterator<IRCChannelContact> it(m_channels); it.current(); ++it )
	{
		if( it.current()->manager(Kopete::Contact::CannotCreate) )
		{
			if( contact == m_mySelf )
				retVal.push_back( it.current() );
			else
			{
				bool c = true;

				Kopete::ContactPtrList members = it.current()->manager()->members();
				for( QPtrListIterator<Kopete::Contact> it2( members ); c && it2.current(); ++it2 )
				{
					if( it2.current() == contact )
					{
						retVal.push_back( it.current() );
						c = false;
					}
				}
			}
		}
	}

	return retVal;
}

IRCChannelContact *IRCContactManager::findChannel(const QString &name, Kopete::MetaContact *m)
{
	IRCChannelContact *channel = m_channels[ name ];

	if ( !channel )
	{
		if( !m )
		{
			m = new Kopete::MetaContact();
			m->setTemporary( true );
		}

		channel = new IRCChannelContact(this, name, m);
		m_channels.insert( name, channel );
		QObject::connect(channel, SIGNAL(contactDestroyed(Kopete::Contact *)),
			this, SLOT(unregister(Kopete::Contact *)));
	}

	return channel;
}

IRCChannelContact *IRCContactManager::existChannel( const QString &channel ) const
{
	return m_channels[ channel ];
}

void IRCContactManager::unregisterChannel(Kopete::Contact *contact, bool force )
{
	IRCChannelContact *channel = (IRCChannelContact*)contact;
	if( force || (
		channel!=0 &&
		!channel->isChatting() &&
		channel->metaContact()->isTemporary() ) )
	{
		m_channels.remove( channel->nickName() );
	}
}

IRCUserContact *IRCContactManager::findUser(const QString &name, Kopete::MetaContact *m)
{
	IRCUserContact *user = m_users[name.section('!', 0, 0)];

	if ( !user )
	{
		if( !m )
		{
			m = new Kopete::MetaContact();
			m->setTemporary( true );
		}

		user = new IRCUserContact(this, name, m);
		m_users.insert( name, user );
		QObject::connect(user, SIGNAL(contactDestroyed(Kopete::Contact *)),
				this, SLOT(unregister(Kopete::Contact *)));
	}

	return user;
}

IRCUserContact *IRCContactManager::existUser( const QString &user ) const
{
	return m_users[user];
}

IRCContact *IRCContactManager::findContact( const QString &id, Kopete::MetaContact *m )
{
	if( KIRC::Entity::isChannel(id) )
		return findChannel( id, m );
	else
		return findUser( id, m );
}

IRCContact *IRCContactManager::existContact( const KIRC::Engine *engine, const QString &id )
{
	QDict<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts( IRCProtocol::protocol() );
	QDictIterator<Kopete::Account> it(accounts);
	for( ; it.current(); ++it )
	{
		IRCAccount *account = (IRCAccount *)it.current();
		if( account && account->engine() == engine )
			return account->contactManager()->existContact(id);
	}
	return 0L;
}

IRCContact *IRCContactManager::existContact( const QString &id ) const
{
	if( KIRC::Entity::isChannel(id) )
		return existChannel( id );
	else
		return existUser( id );
}

void IRCContactManager::unregisterUser(Kopete::Contact *contact, bool force )
{
	IRCUserContact *user = (IRCUserContact *)contact;
	if( force || (
		user!=0 &&
		user!=mySelf() &&
		!user->isChatting() &&
		user->metaContact()->isTemporary() ) )
	{
		m_users.remove( user->nickName() );
	}
}

void IRCContactManager::slotContactAdded( Kopete::MetaContact *contact )
{
	for( QPtrListIterator<Kopete::Contact> it( contact->contacts() ); it.current(); ++it )
	{
		if( it.current()->account() == m_account )
		{
			addToNotifyList( static_cast<IRCContact*>( it.current() )->nickName() );
		}
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
	if( m_account->engine()->isConnected() )
	{
		isonRecieved = false;
		m_account->engine()->ison( m_NotifyList );
		//QTimer::singleShot( socketTimeout, this, SLOT( slotIsonTimeout() ) );
	}
}

void IRCContactManager::slotIsonRecieved()
{
	isonRecieved = true;
}

void IRCContactManager::slotIsonTimeout()
{
	if( !isonRecieved )
		m_account->engine()->quit("", true);
}

#include "irccontactmanager.moc"
