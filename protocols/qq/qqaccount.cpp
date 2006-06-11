/*
    qqaccount.cpp - Kopete QQ Protocol

    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/


#include "qqaccount.h"

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kactionmenu.h>
#include <kmenu.h>

#include "kopetemetacontact.h"
#include "kopetecontactlist.h"

#include "qqcontact.h"
#include "qqnotifysocket.h"
#include "qqfakeserver.h"
#include "qqprotocol.h"


QQAccount::QQAccount( QQProtocol *parent, const QString& accountID )
: Kopete::PasswordedAccount ( parent, accountID )
{
	m_notifySocket = 0L;
	// Init the myself contact
	setMyself( new QQContact( this, accountId(), QQContact::Null, accountId(), Kopete::ContactList::self()->myself() ) );
	myself()->setOnlineStatus( QQProtocol::protocol()->qqOffline );
	m_server = new QQFakeServer();;
}

QQAccount::~QQAccount()
{
	delete m_server;
}

KActionMenu* QQAccount::actionMenu()
{
	KActionMenu *mActionMenu = Kopete::Account::actionMenu();

	mActionMenu->kMenu()->insertSeparator();

	KAction *action;
	
	action = new KAction (KIcon("qq_showvideo"), i18n ("Show my own video..."), 0, "actionShowVideo");
	QObject::connect( action, SIGNAL(triggered(bool)), this, SLOT(slotShowVideo()) );
	mActionMenu->addAction(action);
	action->setEnabled( isConnected() );

	return mActionMenu;
}

bool QQAccount::createContact(const QString& contactId, Kopete::MetaContact* parentContact)
{
	QQContact* newContact = new QQContact( this, contactId, QQContact::Echo, parentContact->displayName(), parentContact );
	return newContact != 0L;
}


void QQAccount::setOnlineStatus(const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason )
{
	if(status.status()== Kopete::OnlineStatus::Offline)
		disconnect();
	else if ( m_notifySocket )
	{
		// m_notifySocket->setStatus( status );
		//setPersonalMessage( reason );
	}
	else
	{
		m_connectstatus = status;
		/* TODO: use connect() later */
		connect( status );
	}
}

void QQAccount::setStatusMessage(const Kopete::StatusMessage& statusMessage)
{
	/* Not used in qq */
}

void QQAccount::connectWithPassword( const QString &password )
{
	kDebug ( 14210 ) << k_funcinfo << "connect with pass" << endl;
	myself()->setOnlineStatus( QQProtocol::protocol()->qqOnline );
	QObject::connect ( m_server, SIGNAL ( messageReceived( const QString & ) ),
			this, SLOT ( receivedMessage( const QString & ) ) );
}

void QQAccount::disconnect()
{
	kDebug ( 14210 ) << k_funcinfo << endl;
	myself()->setOnlineStatus( QQProtocol::protocol()->qqOffline );
	QObject::disconnect ( m_server, 0, 0, 0 );
}

QQFakeServer * QQAccount::server()
{
	return m_server;
}


void QQAccount::slotShowVideo ()
{
	kDebug ( 14210 ) << k_funcinfo << endl;

	if (isConnected ())
		QQWebcamDialog *qqWebcamDialog = new QQWebcamDialog(0, 0, "QQ video window");
	updateContactStatus();
}

void QQAccount::receivedMessage( const QString &message )
{
	// Look up the contact the message is from
	QString from;
	QQContact* messageSender;

	from = message.section( ':', 0, 0 );
	Kopete::Contact* contact = contacts()[from];
	messageSender = dynamic_cast<QQContact *>( contact );

	kDebug( 14210 ) << k_funcinfo << " got a message from " << from << ", " << messageSender << ", is: " << message << endl;
	// Pass it on to the contact to process and display via a KMM
	if ( messageSender )
		messageSender->receivedMessage( message );
	else
		kWarning(14210) << k_funcinfo << "unable to look up contact for delivery" << endl;
}

void QQAccount::updateContactStatus()
{
	QHashIterator<QString, Kopete::Contact*>itr( contacts() );
	for ( ; itr.hasNext(); ) {
		itr.next();
		itr.value()->setOnlineStatus( myself()->onlineStatus() );
	}
}

void QQAccount::connect( const Kopete::OnlineStatus& /* initialStatus */ )
{
	kDebug ( 14210 ) << k_funcinfo << endl;

	if ( isConnected() )
	{
		kDebug( 14210 ) << k_funcinfo <<"Ignoring Connect request "
			<< "(Already Connected)" << endl;
		return;
	}

	if ( m_notifySocket )
	{
		kDebug( 14210 ) << k_funcinfo <<"Ignoring Connect request (Already connecting)"  << endl;
		return;
	}
	/* Hard-coded password for debug only */
	m_password = "hello";

	createNotificationServer(serverName(), serverPort());
}


void QQAccount::createNotificationServer( const QString &host, uint port )
{
	if(m_notifySocket) //we are switching from one to another notifysocket.
	{
		//remove every slots to that socket, so we won't delete receive signals
		// from the old socket thinking they are from the new one
		QObject::disconnect( m_notifySocket , 0, this, 0 );
		m_notifySocket->deleteLater(); //be sure it will be deleted
		m_notifySocket=0L;
	}


	myself()->setOnlineStatus( QQProtocol::protocol()->CNT );


	m_notifySocket = new QQNotifySocket( this, accountId() , m_password);
/*
	QObject::connect( m_notifySocket, SIGNAL( groupAdded( const QString&, const QString& ) ),
		SLOT( slotGroupAdded( const QString&, const QString& ) ) );
	QObject::connect( m_notifySocket, SIGNAL( groupRenamed( const QString&, const QString& ) ),
		SLOT( slotGroupRenamed( const QString&, const QString& ) ) );
	QObject::connect( m_notifySocket, SIGNAL( groupListed( const QString&, const QString& ) ),
		SLOT( slotGroupAdded( const QString&, const QString& ) ) );
	QObject::connect( m_notifySocket, SIGNAL( groupRemoved( const QString& ) ),
		SLOT( slotGroupRemoved( const QString& ) ) );
	QObject::connect( m_notifySocket, SIGNAL( contactList(const QString&, const QString&, const QString&, uint, const QString& ) ),
		SLOT( slotContactListed(const QString&, const QString&, const QString&, uint, const QString& ) ) );
	QObject::connect( m_notifySocket, SIGNAL(contactAdded(const QString&, const QString&, const QString&, const QString&, const QString& ) ),
		SLOT( slotContactAdded(const QString&, const QString&, const QString&, const QString&, const QString& ) ) );
	QObject::connect( m_notifySocket, SIGNAL( contactRemoved(const QString&, const QString&, const QString&, const QString& ) ),
		SLOT( slotContactRemoved(const QString&, const QString&, const QString&, const QString& ) ) );
	QObject::connect( m_notifySocket, SIGNAL( statusChanged( const Kopete::OnlineStatus & ) ),
		SLOT( slotStatusChanged( const Kopete::OnlineStatus & ) ) );
	QObject::connect( m_notifySocket, SIGNAL( invitedToChat( const QString&, const QString&, const QString&, const QString&, const QString& ) ),
		SLOT( slotCreateChat( const QString&, const QString&, const QString&, const QString&, const QString& ) ) );
	QObject::connect( m_notifySocket, SIGNAL( startChat( const QString&, const QString& ) ),
		SLOT( slotCreateChat( const QString&, const QString& ) ) );
	QObject::connect( m_notifySocket, SIGNAL( socketClosed() ),
		SLOT( slotNotifySocketClosed() ) );
	QObject::connect( m_notifySocket, SIGNAL( newContactList() ),
		SLOT( slotNewContactList() ) );
	QObject::connect( m_notifySocket, SIGNAL( receivedNotificationServer(const QString&, uint )  ),
		SLOT(createNotificationServer(const QString&, uint ) ) );
	QObject::connect( m_notifySocket, SIGNAL( hotmailSeted( bool ) ),
		m_openInboxAction, SLOT( setEnabled( bool ) ) );
	QObject::connect( m_notifySocket, SIGNAL( errorMessage(int, const QString& ) ), 
		SLOT( slotErrorMessageReceived(int, const QString& ) ) );
*/
	m_notifySocket->setStatus( m_connectstatus );
	m_notifySocket->connect(host, port);
}

QString QQAccount::serverName()
{
	return configGroup()->readEntry(  "serverName" , "messenger.hotmail.com" );
}

uint QQAccount::serverPort()
{
	return configGroup()->readEntry(  "serverPort" , 1863 );
}



#include "qqaccount.moc"
