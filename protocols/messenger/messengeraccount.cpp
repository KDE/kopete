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

	m_changeDNAction = new KAction( KIcon("help-contents"),i18n( "&Change Display Name..." ), this );
        //, "renameAction" );
	QObject::connect( m_changeDNAction, SIGNAL(triggered(bool)), this, SLOT(slotChangePublicName()) );

	m_editUserInfoAction = new KAction( KIcon("help-contents"),i18n( "&Edit User Info" ), this );
        //, "editUserInfoAction" );
	QObject::connect( m_editUserInfoAction, SIGNAL(triggered(bool)), this, SLOT(slotUserInfo()) );

	m_startChatAction = new KAction( KIcon("mail"), i18n( "&Start Chat..." ), this );
        //, "startChatAction" );
	QObject::connect( m_startChatAction, SIGNAL(triggered(bool)), this, SLOT(slotStartChat()) );

	d = new MessengerAccountPrivate();

	d->client = new Client(new QtConnector(this),this);
	// Set the client Id for the myself contact.  It sets what Messenger feature we support.
	m_clientId = MessengerProtocol::MessengerC4 | MessengerProtocol::InkFormatGIF | MessengerProtocol::SupportMultiPacketMessaging;

}

MessengerAccount::~MessengerAccount()
{
	delete d;
}

QString MessengerAccount::serverName()
{
	return configGroup()->readEntry(  "serverName" , MESSENGER_DEFAULT_SERVER );
}

uint MessengerAccount::serverPort()
{
	return configGroup()->readEntry(  "serverPort" , MESSENGER_DEFAULT_PORT );
}

bool MessengerAccount::useHttpMethod() const
{
	return configGroup()->readEntry(  "useHttpMethod" , false );
}

QString MessengerAccount::myselfClientId() const
{
	return QString::number(m_clientId, 10);
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

#if defined MESSENGER_DEBUG
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
	d->client->setServer(&server, port);
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
	myself()->setStatusMessage(statusMessage);
	d->client->setStatusMessage(statusMessage);
}

bool MessengerAccount::createContact(const QString &contactId, Kopete::MetaContact *parentMetaContact)
{
	if ( !metaContact->isTemporary() && m_notifySocket)
	{
		m_addWizard_metaContact = metaContact;

		// TODO: finish contact add
		//addContactServerside(contactId, metaContact->groups());
		// FIXME: Find out if this contact was really added or not!
		return true;
	}
	else
	{
		// This is a temporary contact.		
		MesengerContact *newContact = new MessengerContact( this, contactId, metaContact );
		newContact->setDeleted(true);
		return true;
	}
}

void MessengerContact::slotUserInfo()
{
	myself()->slotUserInfo();
}
 
void MessengerAccount::setPublicName( const QString &publicName )
{
	if ( d->client )
	{
		d->client->changePublicName( publicName, QString() );
	}
}

Client * MessengerAccount::client()
{
	return d->client;
}

void MessengerAccount::slotStartChat()
{
	bool ok;
	QString handle ;

	handle = KInputDialog::getText( i18n( "Start Chat - Messenger Plugin" ),
		i18n( "Please enter the email address of the person with whom you want to chat:" ), QString(), &ok ).toLower();
	if ( ok )
	{
		if ( MessengerProtocol::validContactId( handle ) )
		{
			if ( !contacts()[ handle ] )
				addContact( handle, handle, 0L, Kopete::Account::Temporary );

			contacts()[ handle ]->execute();
		}
		else
		{
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
				i18n( "<qt>You must enter a valid email address.</qt>" ), i18n( "Messenger Plugin" ) );
		}
	}
}

void MessengerAccount::slotChangePublicName()
{
	if ( !isConnected() )
	{
		return;
	}

	bool ok;
	QString name = KInputDialog::getText( i18n( "Change Display Name - Messenger Plugin" ),
			i18n( "Enter the new display name by which you want to be visible to your friends on Messenger:" ),
			myself()->property( Kopete::Global::Properties::self()->nickName()).value().toString(), &ok );

	if ( ok )
	{
		if ( name.length() > 387 )
		{
			KMessageBox::error( Kopete::UI::Global::mainWidget(),
					i18n( "<qt>The display name you entered is too long. Please use a shorter name.\n"
						"Your display name has <b>not</b> been changed.</qt>" ),
					i18n( "Change Display Name - Messenger Plugin" ) );
			return;
		}
		setPublicName( name );
	}
	return d->client;
}

#include "messengeraccount.moc"
