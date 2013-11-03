/*
    testbedaccount.cpp - Kopete Testbed Protocol

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


#include "testbedaccount.h"

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kactionmenu.h>
#include <kmenu.h>
#include <kicon.h>

#include "kopetemetacontact.h"
#include "kopetecontactlist.h"

#include "testbedcontact.h"
#include "testbedfakeserver.h"
#include "testbedprotocol.h"


TestbedAccount::TestbedAccount( TestbedProtocol *parent, const QString& accountID )
: Kopete::Account ( parent, accountID )
{
	// Init the myself contact
	setMyself( new TestbedContact( this, accountId(), Kopete::ContactList::self()->myself() ) );
	myself()->setOnlineStatus( TestbedProtocol::protocol()->testbedOffline );
	m_server = new TestbedFakeServer();;
}

TestbedAccount::~TestbedAccount()
{
	delete m_server;
}

void TestbedAccount::fillActionMenu( KActionMenu *actionMenu )
{
	Kopete::Account::fillActionMenu( actionMenu );

	actionMenu->addSeparator();

	KAction *action;

	action = new KAction (KIcon("testbed_showvideo"), i18n ("Show my own video..."), actionMenu );
        //, "actionShowVideo");
	QObject::connect( action, SIGNAL(triggered(bool)), this, SLOT(slotShowVideo()) );
	actionMenu->addAction(action);
	action->setEnabled( isConnected() );
}

bool TestbedAccount::createContact(const QString& contactId, Kopete::MetaContact* parentContact)
{
	TestbedContact* newContact = new TestbedContact( this, contactId, parentContact );
	return newContact != 0L;
}

void TestbedAccount::setAway( bool away, const QString & /* reason */ )
{
	if ( away )
		slotGoAway();
	else
		slotGoOnline();
}

void TestbedAccount::setOnlineStatus(const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason, const OnlineStatusOptions& /*options*/)
{
	if ( status.status() == Kopete::OnlineStatus::Online &&
			myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline )
		slotGoOnline();
	else if (status.status() == Kopete::OnlineStatus::Online &&
			(myself()->onlineStatus().status() == Kopete::OnlineStatus::Away || 
				myself()->onlineStatus().status() == Kopete::OnlineStatus::Busy) )
		setAway( false, reason.message() );
	else if ( status.status() == Kopete::OnlineStatus::Offline )
		slotGoOffline();
	else if ( status.status() == Kopete::OnlineStatus::Away )
		slotGoAway( /* reason */ );
	else if ( status.status() == Kopete::OnlineStatus::Busy )
		slotGoBusy( /* reason */ );
}

void TestbedAccount::setStatusMessage(const Kopete::StatusMessage& statusMessage)
{
	Q_UNUSED(statusMessage);
	/* Not used in testbed */
}

void TestbedAccount::connect( const Kopete::OnlineStatus& /* initialStatus */ )
{
	kDebug ( 14210 ) ;
	myself()->setOnlineStatus( TestbedProtocol::protocol()->testbedOnline );
	QObject::connect ( m_server, SIGNAL (messageReceived(QString)),
			this, SLOT (receivedMessage(QString)) );
}

void TestbedAccount::disconnect()
{
	kDebug ( 14210 ) ;
	myself()->setOnlineStatus( TestbedProtocol::protocol()->testbedOffline );
	QObject::disconnect ( m_server, 0, 0, 0 );
}

TestbedFakeServer * TestbedAccount::server()
{
	return m_server;
}

void TestbedAccount::slotGoOnline ()
{
	kDebug ( 14210 ) ;

	if (!isConnected ())
		connect ();
	else
		myself()->setOnlineStatus( TestbedProtocol::protocol()->testbedOnline );
	updateContactStatus();
}

void TestbedAccount::slotGoAway ()
{
	kDebug ( 14210 ) ;

	if (!isConnected ())
		connect();

	myself()->setOnlineStatus( TestbedProtocol::protocol()->testbedAway );
	updateContactStatus();
}

void TestbedAccount::slotGoBusy ()
{
	kDebug ( 14210 ) ;

	if (!isConnected ())
		connect();

	myself()->setOnlineStatus( TestbedProtocol::protocol()->testbedBusy );
	updateContactStatus();
}


void TestbedAccount::slotGoOffline ()
{
	kDebug ( 14210 ) ;

	if (isConnected ())
		disconnect ();
	updateContactStatus();
}

void TestbedAccount::slotShowVideo ()
{
	kDebug ( 14210 ) ;

	if (isConnected ())
	{
		TestbedWebcamDialog *testbedWebcamDialog = new TestbedWebcamDialog(0, 0);
		Q_UNUSED(testbedWebcamDialog);
	}
	updateContactStatus();
}

void TestbedAccount::receivedMessage( const QString &message )
{
	// Look up the contact the message is from
	QString from;
	TestbedContact* messageSender;

	from = message.section( ':', 0, 0 );
	Kopete::Contact* contact = contacts().value(from);
	messageSender = dynamic_cast<TestbedContact *>( contact );

	kDebug( 14210 ) << " got a message from " << from << ", " << messageSender << ", is: " << message;
	// Pass it on to the contact to process and display via a KMM
	if ( messageSender )
		messageSender->receivedMessage( message );
	else
		kWarning(14210) << "unable to look up contact for delivery";
}

void TestbedAccount::updateContactStatus()
{
	QHashIterator<QString, Kopete::Contact*>itr( contacts() );
	for ( ; itr.hasNext(); ) {
		itr.next();
		itr.value()->setOnlineStatus( myself()->onlineStatus() );
	}
}


#include "testbedaccount.moc"
