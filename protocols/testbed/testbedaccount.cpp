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
#include <kpopupmenu.h>

#include "kopetemetacontact.h"
#include "kopetecontactlist.h"

#include "testbedcontact.h"
#include "testbedfakeserver.h"
#include "testbedprotocol.h"


TestbedAccount::TestbedAccount( TestbedProtocol *parent, const QString& accountID, const char *name )
: Kopete::Account ( parent, accountID , name )
{
	// Init the myself contact
	setMyself( new TestbedContact( this, accountId(), TestbedContact::Null, accountId(), Kopete::ContactList::self()->myself() ) );
	myself()->setOnlineStatus( TestbedProtocol::protocol()->testbedOffline );
	m_server = new TestbedFakeServer();;
}

TestbedAccount::~TestbedAccount()
{
	delete m_server;
}

KActionMenu* TestbedAccount::actionMenu()
{
	KActionMenu *mActionMenu = Kopete::Account::actionMenu();

	mActionMenu->popupMenu()->insertSeparator();

	KAction *action;
	
	action = new KAction (i18n ("Show my own video..."), "testbed_showvideo", 0, this, SLOT (slotShowVideo ()), this, "actionShowVideo");
	mActionMenu->insert(action);
	action->setEnabled( isConnected() );

	return mActionMenu;
}

bool TestbedAccount::createContact(const QString& contactId, Kopete::MetaContact* parentContact)
{
	TestbedContact* newContact = new TestbedContact( this, contactId, TestbedContact::Echo, parentContact->displayName(), parentContact );
	return newContact != 0L;
}

void TestbedAccount::setAway( bool away, const QString & /* reason */ )
{
	if ( away )
		slotGoAway();
	else
		slotGoOnline();
}

void TestbedAccount::setOnlineStatus(const Kopete::OnlineStatus& status, const QString &reason )
{
	if ( status.status() == Kopete::OnlineStatus::Online &&
			myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline )
		slotGoOnline();
	else if (status.status() == Kopete::OnlineStatus::Online &&
			myself()->onlineStatus().status() == Kopete::OnlineStatus::Away )
		setAway( false, reason );
	else if ( status.status() == Kopete::OnlineStatus::Offline )
		slotGoOffline();
	else if ( status.status() == Kopete::OnlineStatus::Away )
		slotGoAway( /* reason */ );
}

void TestbedAccount::connect( const Kopete::OnlineStatus& /* initialStatus */ )
{
	kdDebug ( 14210 ) << k_funcinfo << endl;
	myself()->setOnlineStatus( TestbedProtocol::protocol()->testbedOnline );
	QObject::connect ( m_server, SIGNAL ( messageReceived( const QString & ) ),
			this, SLOT ( receivedMessage( const QString & ) ) );
}

void TestbedAccount::disconnect()
{
	kdDebug ( 14210 ) << k_funcinfo << endl;
	myself()->setOnlineStatus( TestbedProtocol::protocol()->testbedOffline );
	QObject::disconnect ( m_server, 0, 0, 0 );
}

TestbedFakeServer * TestbedAccount::server()
{
	return m_server;
}

void TestbedAccount::slotGoOnline ()
{
	kdDebug ( 14210 ) << k_funcinfo << endl;

	if (!isConnected ())
		connect ();
	else
		myself()->setOnlineStatus( TestbedProtocol::protocol()->testbedOnline );
	updateContactStatus();
}

void TestbedAccount::slotGoAway ()
{
	kdDebug ( 14210 ) << k_funcinfo << endl;

	if (!isConnected ())
		connect();

	myself()->setOnlineStatus( TestbedProtocol::protocol()->testbedAway );
	updateContactStatus();
}


void TestbedAccount::slotGoOffline ()
{
	kdDebug ( 14210 ) << k_funcinfo << endl;

	if (isConnected ())
		disconnect ();
	updateContactStatus();
}

void TestbedAccount::slotShowVideo ()
{
	kdDebug ( 14210 ) << k_funcinfo << endl;

	if (isConnected ())
		TestbedWebcamDialog *testbedWebcamDialog = new TestbedWebcamDialog(0, 0, "Testbed video window");
	updateContactStatus();
}

void TestbedAccount::receivedMessage( const QString &message )
{
	// Look up the contact the message is from
	QString from;
	TestbedContact* messageSender;

	from = message.section( ':', 0, 0 );
	Kopete::Contact* contact = contacts()[from];
	messageSender = dynamic_cast<TestbedContact *>( contact );

	kdDebug( 14210 ) << k_funcinfo << " got a message from " << from << ", " << messageSender << ", is: " << message << endl;
	// Pass it on to the contact to process and display via a KMM
	if ( messageSender )
		messageSender->receivedMessage( message );
	else
		kdWarning(14210) << k_funcinfo << "unable to look up contact for delivery" << endl;
}

void TestbedAccount::updateContactStatus()
{
	QDictIterator<Kopete::Contact> itr( contacts() );
	for ( ; itr.current(); ++itr )
		itr.current()->setOnlineStatus( myself()->onlineStatus() );
}


#include "testbedaccount.moc"
