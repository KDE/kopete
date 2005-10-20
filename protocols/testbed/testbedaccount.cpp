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
#include <kmenu.h>

#include "kopetemetacontact.h"

#include "testbedcontact.h"
#include "testbedfakeserver.h"
#include "testbedprotocol.h"


TestbedAccount::TestbedAccount( TestbedProtocol *parent, const QString& accountID, const char *name )
: Kopete::Account ( parent, accountID , name )
{
	// Init the myself contact
	// FIXME: I think we should add a global self metaContact (Olivier)
	setMyself( new TestbedContact( this, accountId(), TestbedContact::Null, accountId(), 0L ) );
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

void TestbedAccount::receivedMessage( const QString &message )
{
	// Look up the contact the message is from
	QString from;
	TestbedContact* messageSender;

	from = message.section( ':', 0, 0 );
	//from = QString::fromLatin1("echo");
	messageSender = static_cast<TestbedContact *>( contacts ()[ from ] );

	kdDebug( 14210 ) << k_funcinfo << " got a message from " << from << ", " << messageSender << ", is: " << message << endl;
	// Pass it on to the contact to process and display via a KMM
	messageSender->receivedMessage( message );
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
