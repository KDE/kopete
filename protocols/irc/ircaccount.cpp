/*
    ircaccount.cpp - IRC Account

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
#include <kaction.h>
#include <kpopupmenu.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <klineeditdlg.h>
#include <kmessagebox.h>

#include "ircaccount.h"
#include "ircprotocol.h"
#include "ircusercontact.h"
#include "ircchannelcontact.h"

#include "kopeteaway.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kdebug.h"
#include "ksparser.h"

IRCAccount::IRCAccount(const QString &accountId, const IRCProtocol *protocol) : KopeteAccount( (KopeteProtocol*)protocol, accountId )
{
	mManager = 0L;
	mProtocol = protocol;

	mNickName = accountId.section('@',0,0);
	QString serverInfo = accountId.section('@',1);
	mServer = serverInfo.section(':',0,0);
	mPort = serverInfo.section(':',1).toUInt();

	mEngine = new KIRC( mServer, mPort );
	QString version=i18n("Kopete IRC Plugin %1 [http://kopete.kde.org]").arg(kapp->aboutData()->version());
	mEngine->setVersionString( version  );
	if( rememberPassword() )
		mEngine->setPassword( getPassword() );

	QObject::connect(mEngine, SIGNAL(successfullyChangedNick(const QString &, const QString &)), this, SLOT(successfullyChangedNick(const QString &, const QString &)));
	QObject::connect(mEngine, SIGNAL(incomingPrivMessage(const QString &, const QString &, const QString &)), this, SLOT(slotNewPrivMessage(const QString &, const QString &, const QString &)));
	QObject::connect(mEngine, SIGNAL(connectedToServer()), this, SLOT(slotConnectedToServer()));
	QObject::connect(mEngine, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));

	mMySelf = findUser( mNickName );
}

IRCAccount::~IRCAccount()
{
	kdDebug(14120) << k_funcinfo << endl;
	if ( engine()->state() != QSocket::Idle )
		engine()->quitIRC( i18n("Plugin Unloaded") );

	delete engine();
}

KActionMenu *IRCAccount::actionMenu()
{
	QString menuTitle = QString::fromLatin1( " %1 <%2> " ).arg( accountId() ).arg( mMySelf->onlineStatus().description() );

	KActionMenu *mActionMenu = new KActionMenu( accountId(), this );
	mActionMenu->popupMenu()->insertTitle( mMySelf->onlineStatus().iconFor( mMySelf ), menuTitle, 1 );
	mActionMenu->setIconSet( QIconSet ( mMySelf->onlineStatus().iconFor( mMySelf ) ) );

	mActionMenu->insert( new KAction ( i18n("Online"), IRCProtocol::IRCUserOnline().iconFor( mMySelf ), 0, this, SLOT(connect()), mActionMenu ) );
	mActionMenu->insert( new KAction ( i18n("Offline"), IRCProtocol::IRCUserOffline().iconFor( mMySelf ), 0, this, SLOT(disconnect()), mActionMenu ) );
	mActionMenu->insert( new KAction ( i18n("Away"), IRCProtocol::IRCUserAway().iconFor( mMySelf ), 0, this, SLOT(slotGoAway()), mActionMenu ) );
	mActionMenu->popupMenu()->insertSeparator();
	mActionMenu->insert( new KAction ( i18n("Join Channel..."), "", 0, this, SLOT(slotJoinChannel()), mActionMenu ) );

	return mActionMenu;
}

void IRCAccount::slotNewPrivMessage(const QString &originating, const QString &, const QString &message)
{
	//kdDebug(14120) << k_funcinfo << "o:" << originating << "; t:" << target << endl;
	KopeteContactPtrList others;
	others.append( myself() );
	IRCUserContact *c = findUser(  originating.section('!',0,0) );
	KopeteMessage msg( (KopeteContact*)c, others, message, KopeteMessage::Inbound, KopeteMessage::PlainText, KopeteMessage::Chat );
	msg.setBody( mProtocol->parser()->parse( msg.escapedBody() ), KopeteMessage::RichText );
	c->manager()->appendMessage(msg);
}

void IRCAccount::connect()
{
	if( engine()->isLoggedIn() )
	{
		if( isAway() )
			setAway( false );
	}
	else
	{
		mMySelf->setOnlineStatus( IRCProtocol::IRCUserConnecting() );
		engine()->connectToServer( mMySelf->nickName() );
	}
}

void IRCAccount::disconnect()
{
	engine()->quitIRC("Kopete IRC [http://kopete.kde.org]");
}

void IRCAccount::setAway( bool isAway, const QString &awayMessage )
{
	if( engine()->isLoggedIn() )
	{
		if( isAway )
			mySelf()->setOnlineStatus( IRCProtocol::IRCUserAway() );
		else
			mySelf()->setOnlineStatus( IRCProtocol::IRCUserOnline() );
		engine()->setAway( isAway, awayMessage );
	}
}

void IRCAccount::slotGoAway()
{
	if( engine()->isLoggedIn() )
	{
		mySelf()->setOnlineStatus( IRCProtocol::IRCUserAway() );
		engine()->setAway( true, KopeteAway::message() );
	}
}

bool IRCAccount::isConnected()
{
	return (mMySelf->onlineStatus().status() == KopeteOnlineStatus::Online);
}

IRCChannelContact *IRCAccount::findChannel(const QString &name, KopeteMetaContact *m  )
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
		if( mEngine->isLoggedIn() )
			channel->setOnlineStatus( IRCProtocol::IRCChannelOnline() );
		QObject::connect(channel, SIGNAL(contactDestroyed(KopeteContact *)), this,
			SLOT(slotContactDestroyed(KopeteContact *)));
	}
	else
	{
		channel = mChannels[ lowerName ];
		kdDebug(14120) << k_funcinfo << lowerName << " conversations:" << channel->conversations() << endl;
	}

	return channel;
}

void IRCAccount::unregisterChannel( const QString &name )
{
	QString lowerName = name.lower();
	if( mChannels.contains( lowerName ) )
	{
		IRCChannelContact *channel = mChannels[lowerName];
		if( channel->conversations() == 0 && channel->metaContact()->isTemporary() )
		{
			kdDebug(14120) << k_funcinfo << name << endl;
			delete channel->metaContact();
		}
	}
}

IRCUserContact *IRCAccount::findUser(const QString &name, KopeteMetaContact *m)
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
		QObject::connect(user, SIGNAL(contactDestroyed(KopeteContact *)), this,
			SLOT(slotContactDestroyed(KopeteContact *)));
	}
	else
	{
		user = mUsers[ lowerName ];
		kdDebug(14120) << k_funcinfo << lowerName << " conversations:" << user->conversations() << endl;
	}

	return user;
}

void IRCAccount::unregisterUser( const QString &name )
{
	QString lowerName = name.lower();
	if( lowerName != mNickName.lower() && mUsers.contains( lowerName ) )
	{
		IRCUserContact *user = mUsers[lowerName];
		if( user->conversations() == 0 && user->metaContact()->isTemporary() )
		{
			kdDebug(14120) << k_funcinfo << name << endl;
			delete user->metaContact();
		}
	}
}

void IRCAccount::slotContactDestroyed(KopeteContact *contact)
{
	kdDebug(14120) << k_funcinfo << endl;
	const QString nickname = static_cast<IRCContact*>( contact )->nickName().lower();

	if ( nickname.startsWith( QString::fromLatin1("#") ) )
		mChannels.remove( nickname );
	else
	{
		mUsers.remove(nickname);
		engine()->removeFromNotifyList( nickname );
	}
}

void IRCAccount::slotConnectedToServer()
{
	kdDebug(14120) << k_funcinfo << endl;
	mMySelf->setOnlineStatus( IRCProtocol::IRCUserOnline() );
}

void IRCAccount::slotConnectionClosed()
{
	kdDebug(14120) << k_funcinfo << endl;
	mMySelf->setOnlineStatus( IRCProtocol::IRCUserOffline() );
}

void IRCAccount::successfullyChangedNick(const QString &/*oldnick*/, const QString &newnick)
{
	kdDebug(14120) << k_funcinfo << "Changing nick to " << newnick << endl;
	mMySelf->manager()->setDisplayName( mMySelf->caption() );

	if( isConnected() )
		engine()->changeNickname( newnick );
}

bool IRCAccount::addContactToMetaContact( const QString &contactId, const QString &displayName,
	 KopeteMetaContact *m )
{
	IRCContact *c;

	if( !m )
	{
		m = new KopeteMetaContact();
		KopeteContactList::contactList()->addMetaContact(m);
		m->setDisplayName( displayName );
	}

	if ( contactId.startsWith( QString::fromLatin1("#") ) )
		c = static_cast<IRCContact*>( findChannel(contactId, m) );
	else
	{
		engine()->addToNotifyList( contactId );
		c = static_cast<IRCContact*>( findUser(contactId, m) );
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

	return true;
}

void IRCAccount::slotJoinChannel()
{
	if(!isConnected())
		return;

	QString chan = KLineEditDlg::getText( i18n( "Kopete IRC Plugin" ),
		i18n( "Please enter name of the channel you want to join:" ), QString::null);
	if( !chan.isNull() )
	{
		if( chan.startsWith( QString::fromLatin1("#") ) )
			findChannel( chan )->startChat();
		else
			KMessageBox::error(0l, i18n("<qt>\"%1\" is an invalid channel. Channels must start with '#'.</qt>").arg(chan), i18n("Kopete IRC Plugin"));
	}
}

#include "ircaccount.moc"
