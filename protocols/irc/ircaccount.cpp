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
#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klineeditdlg.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>

#include "kopeteaway.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"

#include "ircaccount.h"
#include "ircprotocol.h"
#include "irccontactmanager.h"
#include "ircservercontact.h"
#include "ircchannelcontact.h"
#include "ircusercontact.h"
#include "ksparser.h"

IRCAccount::IRCAccount(IRCProtocol *protocol, const QString &accountId)
	: KopeteAccount(protocol, accountId)
{
	m_manager = 0L;
	m_protocol = protocol;

	mNickName = accountId.section('@',0,0);
	QString serverInfo = accountId.section('@',1);
	m_server = serverInfo.section(':',0,0);
	m_port = serverInfo.section(':',1).toUInt();

	m_engine = new KIRC( m_server, m_port );
	QString version=i18n("Kopete IRC Plugin %1 [http://kopete.kde.org]").arg(kapp->aboutData()->version());
	m_engine->setVersionString( version  );
	if( rememberPassword() )
		m_engine->setPassword( password() );

	QObject::connect(m_engine, SIGNAL(successfullyChangedNick(const QString &, const QString &)),
			this, SLOT(successfullyChangedNick(const QString &, const QString &)));

	m_contactManager = new IRCContactManager(mNickName, m_server, this);
	m_mySelf = m_contactManager->mySelf();
	m_myServer = m_contactManager->myServer();
}

IRCAccount::~IRCAccount()
{
//	kdDebug(14120) << k_funcinfo << mServer << " " << engine() << endl;
	if ( engine()->isConnected() )
		engine()->quitIRC(i18n("Plugin Unloaded"), true);

	delete m_contactManager;
	delete m_engine;
}

void IRCAccount::loaded()
{
	m_engine->setUserName(userName());
}

QString IRCAccount::userName()
{
	return pluginData(protocol(), QString::fromLatin1("userName"));
}

void IRCAccount::setUserName(QString userName)
{
	m_engine->setUserName(userName);
	setPluginData(protocol(), QString::fromLatin1( "userName" ), userName);
}

KActionMenu *IRCAccount::actionMenu()
{
	QString menuTitle = QString::fromLatin1( " %1 <%2> " ).arg( accountId() ).arg( m_mySelf->onlineStatus().description() );

	KActionMenu *mActionMenu = new KActionMenu( accountId(),myself()->onlineStatus().iconFor(this), this, "IRCAccount::mActionMenu" );
	mActionMenu->popupMenu()->insertTitle( m_mySelf->onlineStatus().iconFor( m_mySelf ), menuTitle );

	mActionMenu->insert( new KAction ( i18n("Go Online"), m_protocol->m_UserStatusOnline.iconFor( this ), 0, this, SLOT(connect()), mActionMenu ) );
	mActionMenu->insert( new KAction ( i18n("Set Away"), m_protocol->m_UserStatusAway.iconFor( this ), 0, this, SLOT(slotGoAway()), mActionMenu ) );
	mActionMenu->insert( new KAction ( i18n("Go Offline"), m_protocol->m_UserStatusOffline.iconFor( this ), 0, this, SLOT(disconnect()), mActionMenu ) );
	mActionMenu->popupMenu()->insertSeparator();
	mActionMenu->insert( new KAction ( i18n("Join Channel..."), "", 0, this, SLOT(slotJoinChannel()), mActionMenu ) );
	mActionMenu->insert( new KAction ( i18n("Show Server Window"), "", 0, this, SLOT(slotShowServerWindow()), mActionMenu ) );

	return mActionMenu;
}

void IRCAccount::connect()
{
	if( m_engine->isConnected() )
	{
		if( isAway() )
			setAway( false );
	}
	else if( m_engine->isDisconnected() )
	{
		m_engine->connectToServer( m_mySelf->nickName() );
	}
}

void IRCAccount::disconnect()
{
	m_engine->quitIRC("Kopete IRC [http://kopete.kde.org]");
}

void IRCAccount::setAway( bool isAway, const QString &awayMessage )
{
	kdDebug(14120) << k_funcinfo << isAway << " " << awayMessage << endl;
	if(m_engine->isConnected())
	{
		m_mySelf->setAway( isAway );
		engine()->setAway( isAway, awayMessage );
	}
}

void IRCAccount::slotGoAway()
{
	setAway( true, KopeteAway::message() );
}

void IRCAccount::slotShowServerWindow()
{
	m_myServer->startServerChat();
}

bool IRCAccount::isConnected()
{
	return (m_mySelf->onlineStatus().status() == KopeteOnlineStatus::Online);
}

void IRCAccount::unregister(KopeteContact *contact)
{
	m_contactManager->unregister(contact);
}

IRCServerContact *IRCAccount::findServer( const QString &name, KopeteMetaContact *m )
{
	return m_contactManager->findServer(name, m);
}

void IRCAccount::unregisterServer( const QString &name )
{
	m_contactManager->unregisterServer(name);
}

IRCChannelContact *IRCAccount::findChannel( const QString &name, KopeteMetaContact *m )
{
	return m_contactManager->findChannel(name, m);
}

void IRCAccount::unregisterChannel( const QString &name )
{
	m_contactManager->unregisterChannel(name);
}

IRCUserContact *IRCAccount::findUser(const QString &name, KopeteMetaContact *m)
{
	return m_contactManager->findUser(name, m);
}

void IRCAccount::unregisterUser( const QString &name )
{
	m_contactManager->unregisterUser(name);
}

void IRCAccount::successfullyChangedNick(const QString &/*oldnick*/, const QString &newnick)
{
	kdDebug(14120) << k_funcinfo << "Changing nick to " << newnick << endl;
	m_mySelf->manager()->setDisplayName( m_mySelf->caption() );

	if( isConnected() )
		m_engine->changeNickname( newnick );
}

bool IRCAccount::addContactToMetaContact( const QString &contactId, const QString &displayName,
	 KopeteMetaContact *m )
{
//	kdDebug(14120) << k_funcinfo << contactId << "|" << displayName << endl;
	//FIXME: I think there are too many tests in this functions.  This function should be called ONLY by
	// KopeteAccount::addContact, where all test are already done. Can a irc developer look at this?   -Olivier
	IRCContact *c;

	if( !m )
	{//This should NEVER happends
		m = new KopeteMetaContact();
		KopeteContactList::contactList()->addMetaContact(m);
		m->setDisplayName( displayName );
	}

	if ( contactId.startsWith( QString::fromLatin1("#") ) )
		c = static_cast<IRCContact*>( findChannel(contactId, m) );
	else
	{
		m_contactManager->addToNotifyList( contactId );
		c = static_cast<IRCContact*>( findUser(contactId, m) );
	}

	if( c->metaContact() != m )
	{//This should NEVER happends
		KopeteMetaContact *old = c->metaContact();
		c->setMetaContact( m );
		KopeteContactPtrList children = old->contacts();
		if( children.isEmpty() )
			KopeteContactList::contactList()->removeMetaContact( old );
	}
	else if( c->metaContact()->isTemporary() ) //FIXME: if the metacontact is temporary, that mean this is a temporary contact
		m->setTemporary(false);

	return true;
}

void IRCAccount::slotJoinChannel()
{
	if(!isConnected())
		return;

	QString chan = KLineEditDlg::getText( i18n( "IRC Plugin" ),
		i18n( "Please enter name of the channel you want to join:" ), QString::null);
	if( !chan.isNull() )
	{
		if( chan.startsWith( QString::fromLatin1("#") ) )
			findChannel( chan )->startChat();
		else
			KMessageBox::error(0l, i18n("<qt>\"%1\" is an invalid channel. Channels must start with '#'.</qt>").arg(chan), i18n("IRC Plugin"));
	}
}

KopeteContact *IRCAccount::myself() const
{
	return m_mySelf;
}

IRCUserContact *IRCAccount::mySelf() const
{
	return m_mySelf;
}

IRCServerContact *IRCAccount::myServer() const
{
	return m_myServer;
}

#include "ircaccount.moc"
