/*
 * messengeraccount.cpp - Windows Live Messenger Kopete Account.
 *
 * Copyright (c) 2007 by Zhang Panyong <pyzhang@gmail.com>
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 * 
 * Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#include "messengeraccount.h"

// KDE includes
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>

// Kopete includes
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "kopetecontactlist.h"

// Messenger includes
#include "messengerprotocol.h"
#include "messengercontact.h"

class MessengerAccountPrivate
{
	public:
		MessengerAccountPrivate()
		{

		}
		~MessengerAccountPrivate()
		{
			delete client;
		}

		/*client*/
		Client *client;
}

MessengerAccount::MessengerAccount(MessengerProtocol *protocol, const QString &accountId)
 : Kopete::PasswordedAccount(protocol, accountId.toLower(), false)
{
	setMyself( new MessengerContact(this, accountId, Kopete::ContactList::self()->myself()) );

	m_openInboxAction = new KAction( KIcon("mail"), i18n( "Open Inbo&x..." ), this );
        //, "m_openInboxAction" );
	QObject::connect( m_openInboxAction, SIGNAL(triggered(bool)), this, SLOT(slotOpenInbox()) );

	m_changeDNAction = new KAction( i18n( "&Change Display Name..." ), this );
        //, "renameAction" );
	QObject::connect( m_changeDNAction, SIGNAL(triggered(bool)), this, SLOT(slotChangePublicName()) );

	m_startChatAction = new KAction( KIcon("mail"), i18n( "&Start Chat..." ), this );
        //, "startChatAction" );
	QObject::connect( m_startChatAction, SIGNAL(triggered(bool)), this, SLOT(slotStartChat()) );

	d = new MessengerAccountPrivate();
	d->client = new Client(new QtConnector(this),this);

}

MessengerAccount::~MessengerAccount()
{
	delete d;
}

KActionMenu *MessengerAccount::actionMenu()
{
	KActionMenu *m_actionMenu = Kopete::Account::actionMenu();

	if(isConnected())
	{
		m_openInboxAction->setEnabled( true );
		m_startChatAction->setEnabled( true );
		m_changeDNAction->setEnabled( true );

	}
	else
	{
		m_openInboxAction->setEnabled( false );
		m_startChatAction->setEnabled( false );
		m_changeDNAction->setEnabled( false );
	}
	m_actionMenu->addSeperator();

	m_actionMenu->addAction( m_changeDNAction );
	m_actionMenu->addAction( m_startChatAction );
	m_actionMenu->addAction( m_openInboxAction );

#if !defined NDEBUG
	KActionMenu *debugMenu = new KActionMenu( "Debug", this );

	KAction *rawCmd = new KAction( i18n( "Send Raw C&ommand..." ), this );
        //, "m_debugRawCommand" );
	QObject::connect( rawCmd, SIGNAL(triggered()), this, SLOT(slotDebugRawCommand()) );
	debugMenu->addAction(rawCmd);

	m_actionMenu->addSeparator();
	m_actionMenu->addAction( debugMenu );
#endif

	return m_actionMenu;
}

void MessengerAccount::connectWithPassword(const QString &password)
{
	kDebug(MESSENGER_DEBUG) << k_funcinfo << "connect with password"<<endl;
	if(password.isNull())
	{
		kDebug(MESSENGER_DEBUG ) << k_funcinfo <<"Abort connection (null password)"  << endl;
		return;
	}

	if(isConnected())
	{
		kDebug(MESSENGER_DEBUG) << k_funcinfo << "Ignore connect request(already connected)"<<endl;
		return;
	}

	if ( contacts().count() <= 1 )
	{
		// Maybe the contact list.xml has been removed, and the serial number not updated
		// ( the 1 is for the myself contact )
		configGroup()->writeEntry( "serial", 0 );
	}

	m_openInboxAction->setEnabled( false );

	QString server = configGroup()->readEntry( "serverName", MESSENGER_DEFAULT_SERVER );
	int port = configGroup()->readEntry( "serverPort", MESSENGER_DEFAULT_PORT );
	d->client->setServer(&server,port);
	d->client->userContact()->setLoginInformation( accountId(), password );
    d->client->connectToServer();
}

void MessengerAccount::disconnect()
{
	kDebug(MESSENGER_DEBUG) << k_funcinfo << "attempt to set status offline"<<endl;
	d->client->disconnectFromServer();
}

void MessengerAccount::setOnlineStatus(const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason)
{
	
}

void MessengerAccount::setStatusMessage(const Kopete::StatusMessage &statusMessage)
{
	
}

bool MessengerAccount::createContact(const QString &contactId, Kopete::MetaContact *parentMetaContact)
{
	return false;
}

void MessengerContact::slotUserInfo()
{

}

Client * MessengerAccount::client()
{
	return d->client;
}

#include "messengeraccount.moc"
