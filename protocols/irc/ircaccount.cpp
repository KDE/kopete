/*
    ircidentity.cpp - IRC Identity

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
#include <klocale.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kiconloader.h>

#include "ircaccount.h"
#include "ircprotocol.h"
#include "ircusercontact.h"
#include "ircchannelcontact.h"

#include "kopetecontactlist.h"
#include "kopetemessagemanager.h"
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "kopetemessage.h"
#include "kdebug.h"
#include "ksparser.h"
#include "kirc.h"

IRCIdentity::IRCIdentity(const QString &identityId, const IRCProtocol *protocol) : KopeteIdentity( (KopeteProtocol*)protocol, identityId )
{
	mManager = 0L;
	mMySelf = 0L;
	mEngine = 0L;
	mProtocol = protocol;

	actionOnline = new KAction ( i18n("Online"), "", 0, this, SLOT(connect()), this );
	actionOffline =  new KAction ( i18n("Offline"), "", 0, this, SLOT(disconnect()), this);
	QObject::connect( this, SIGNAL(identityIdChanged()), this, SLOT(slotIdentityIdChanged()) );
	QObject::connect( this, SIGNAL(passwordChangeded()), this, SLOT(slotPasswordChanged()) );

	slotIdentityIdChanged();
}

IRCIdentity::~IRCIdentity()
{
	kdDebug(14120) << k_funcinfo << endl;
	if ( mEngine && mEngine->state() != QSocket::Idle )
		mEngine->quitIRC( i18n("Plugin Unloaded") );

	delete mEngine;
}

void IRCIdentity::slotIdentityIdChanged()
{
	mNickName = identityId().section('@',0,0);
	QString serverInfo = identityId().section('@',1);
	mServer = serverInfo.section(':',0,0);
	mPort = serverInfo.section(':',1).toUInt();
	if( !mMySelf )
		mMySelf = findUser( mNickName );

	bool reConnect = mEngine->isLoggedIn();

	if( mServer == mEngine->host() && mPort == mEngine->port() )
	{
		if( mNickName != mMySelf->nickName() )
			successfullyChangedNick( mMySelf->nickName(), mNickName );
	}
	else
	{
		if( reConnect )
			disconnect();

		if( mNickName != mMySelf->nickName() )
		{
			unregisterUser( mMySelf->nickName() );
			mMySelf = findUser( mNickName );
		}

		delete mEngine;
	}

	if( reConnect )
		connect();
}

void IRCIdentity::slotPasswordChanged()
{
	if( !isConnected() )
		delete mEngine;
}

KIRC *IRCIdentity::engine()
{
	if( !mEngine )
	{
		mEngine = new KIRC( mServer, mPort );
		if( rememberPassword() )
			mEngine->setPassword( getPassword() );

		QObject::connect(mEngine, SIGNAL(successfullyChangedNick(const QString &, const QString &)), this, SLOT(successfullyChangedNick(const QString &, const QString &)));
		QObject::connect(mEngine, SIGNAL(incomingPrivMessage(const QString &, const QString &, const QString &)), this, SLOT(slotNewPrivMessage(const QString &, const QString &, const QString &)));
		QObject::connect(mEngine, SIGNAL(connectedToServer()), this, SLOT(slotConnectedToServer()));
		QObject::connect(mEngine, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
	}
	return mEngine;
}

KActionMenu *IRCIdentity::actionMenu()
{
	QString menuTitle = QString::fromLatin1( " %1 <%2> " ).arg( identityId() ).arg( mMySelf->onlineStatus().description() );

	KActionMenu *mActionMenu = new KActionMenu( identityId(), this );
	mActionMenu->popupMenu()->insertTitle( SmallIcon( mMySelf->onlineStatus().icon() ), menuTitle, 1 );
	mActionMenu->setIcon( mMySelf->onlineStatus().icon() );

	mActionMenu->insert( actionOnline );
	mActionMenu->insert( actionOffline );

	return mActionMenu;
}

void IRCIdentity::slotNewPrivMessage(const QString &originating, const QString &, const QString &message)
{
	//kdDebug(14120) << k_funcinfo << "o:" << originating << "; t:" << target << endl;
	KopeteContactPtrList others;
	others.append( myself() );
	IRCUserContact *c = findUser(  originating.section('!',0,0) );
	KopeteMessage msg( (KopeteContact*)c, others, message, KopeteMessage::Inbound, KopeteMessage::PlainText, KopeteMessage::Chat );
	msg.setBody( mProtocol->parser()->parse( msg.escapedBody() ), KopeteMessage::RichText );
	c->manager()->appendMessage(msg);
}

void IRCIdentity::connect()
{
	engine()->connectToServer( mMySelf->nickName() );
}

void IRCIdentity::disconnect()
{
 	engine()->quitIRC("Kopete IRC 2.0. http://kopete.kde.org");
	delete mEngine;
}

void IRCIdentity::setAway(bool)
{

}

void IRCIdentity::addContact( const QString &contact, const QString &displayName, KopeteMetaContact *m )
{
	IRCContact *c;

	if( !m )
	{
		m = new KopeteMetaContact();
		KopeteContactList::contactList()->addMetaContact(m);
		m->setDisplayName( displayName );
	}

	if ( contact.startsWith( QString::fromLatin1("#") ) )
		c = static_cast<IRCContact*>( findChannel(contact, m) );
	else
	{
		mEngine->addToNotifyList( contact );
		c = static_cast<IRCContact*>( findUser(contact, m) );
	}

	if( c->metaContact() != m )
	{
		KopeteMetaContact *old = c->metaContact();
		c->setMetaContact( m );
		KopeteContactPtrList children = old->contacts();
		if( children.isEmpty() )
			KopeteContactList::contactList()->removeMetaContact( old );
	}
	else if( c->metaContact()->isTemporary() )
		m->setTemporary(false);
}

IRCChannelContact *IRCIdentity::findChannel(const QString &name, KopeteMetaContact *m  )
{
	if( !m )
	{
		m = new KopeteMetaContact();
		m->setTemporary( true );
	}

	QString lowerName = name.lower();
	IRCChannelContact *channel = 0L;
	if ( !mChannels.contains( lowerName ) )
	{
		channel = new IRCChannelContact(this, name, m);
		mChannels.insert( lowerName, channel );
		if( mEngine->state() == QSocket::Connected)
			channel->setOnlineStatus( IRCProtocol::IRCChannelOnline() );
	}
	else
	{
		channel = mChannels[ lowerName ];
		kdDebug(14120) << k_funcinfo << lowerName << " conversations:" << channel->conversations() << endl;
	}

	return channel;
}

void IRCIdentity::unregisterChannel( const QString &name )
{
	QString lowerName = name.lower();
	if( mChannels.contains( lowerName ) )
	{
		IRCChannelContact *channel = mChannels[lowerName];
		if( channel->conversations() == 0 && !channel->metaContact() )
		{
			delete channel->metaContact();
			mChannels.remove(lowerName);
		}
	}
}

IRCUserContact *IRCIdentity::findUser(const QString &name, KopeteMetaContact *m)
{
	if( !m )
	{
		m = new KopeteMetaContact();
		m->setTemporary( true );
	}

	QString lowerName = name.lower();
	IRCUserContact *user = 0L;
	if ( !mUsers.contains( lowerName ) )
	{
		user = new IRCUserContact(this, name, m);
		mUsers.insert( lowerName, user );
	}
	else
	{
		user = mUsers[ lowerName ];
		kdDebug(14120) << k_funcinfo << lowerName << " conversations:" << user->conversations() << endl;
	}

	return user;
}

void IRCIdentity::unregisterUser( const QString &name )
{
	QString lowerName = name.lower();
	if( mUsers.contains( lowerName ) )
	{
		IRCUserContact *user = mUsers[lowerName];
		if( user->conversations() == 0 && !user->metaContact() )
		{
			delete user->metaContact();
			mUsers.remove(lowerName);
			engine()->removeFromNotifyList( lowerName );
		}
	}
}

void IRCIdentity::slotConnectedToServer()
{
	mMySelf->setOnlineStatus( IRCProtocol::IRCUserOnline() );
}

void IRCIdentity::slotConnectionClosed()
{
	mMySelf->setOnlineStatus( IRCProtocol::IRCUserOffline() );
}

void IRCIdentity::successfullyChangedNick(const QString &/*oldnick*/, const QString &newnick)
{
	kdDebug(14120) << k_funcinfo << "Changing nick to " << newnick << endl;

	mMySelf->setNickName(newnick);
	mMySelf->manager()->setDisplayName( mMySelf->caption() );

	if( isConnected() )
		engine()->changeNickname( newnick );
}

#include "ircaccount.moc"
