/*
    testbedcontact.cpp - Kopete Testbed Protocol

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

#include "testbedcontact.h"

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kopeteaccount.h"
#include "kopeteglobal.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"

#include "testbedaccount.h"
#include "testbedfakeserver.h"
#include "testbedprotocol.h"

TestbedContact::TestbedContact( KopeteAccount* _account, const QString &uniqueName,
		const TestbedContactType type, const QString &displayName, KopeteMetaContact *parent )
: KopeteContact( _account, uniqueName, parent )
{
	kdDebug( 14210 ) << k_funcinfo << " uniqueName: " << uniqueName << ", displayName: " << displayName << endl;
	m_type = type;
	setProperty( Kopete::Global::Properties::self()->nickName(), displayName );
	m_msgManager = 0L;
	
	connect( &m_actionTimer, SIGNAL( timeout() ), SLOT ( slotNextAction() ) );

	setOnlineStatus( TestbedProtocol::protocol()->testbedOffline );
}

TestbedContact::~TestbedContact()
{
}

bool TestbedContact::isReachable()
{
    return true;
}

void TestbedContact::serialize( QMap< QString, QString > &serializedData, QMap< QString, QString > & /* addressBookData */ )
{
    QString value;
	switch ( m_type )
	{
	case Null:
		value = "null";
	case Echo:
		value = "echo";
	case StatusChanger:
		value = "statuschanger";
	}
	serializedData[ "contactType" ] = value;
}

KopeteMessageManager* TestbedContact::manager( bool )
{
	kdDebug( 14210 ) << k_funcinfo << endl;
	if ( m_msgManager )
	{
		return m_msgManager;
	}
	else
	{
		QPtrList<KopeteContact> contacts;
		contacts.append(this);
		m_msgManager = KopeteMessageManagerFactory::factory()->create(account()->myself(), contacts, protocol());
		connect(m_msgManager, SIGNAL(messageSent(KopeteMessage&, KopeteMessageManager*)),
				this, SLOT( sendMessage( KopeteMessage& ) ) );
		connect(m_msgManager, SIGNAL(destroyed()), this, SLOT(slotMessageManagerDestroyed()));
		return m_msgManager;
	}
}


QPtrList<KAction> *TestbedContact::customContextMenuActions() //OBSOLETE
{
	//FIXME!!!  this function is obsolete, we should use XMLGUI instead
	/*m_actionCollection = new KActionCollection( this, "userColl" );
	m_actionPrefs = new KAction(i18n( "&Contact Settings" ), 0, this,
			SLOT( showContactSettings( )), m_actionCollection, "contactSettings" );

	return m_actionCollection;*/
	return 0L;
}

void TestbedContact::showContactSettings()
{
	//TestbedContactSettings* p = new TestbedContactSettings( this );
	//p->show();
}

void TestbedContact::sendMessage( KopeteMessage &message )
{
	kdDebug( 14210 ) << k_funcinfo << endl;
	// convert to the what the server wants
	// For this 'protocol', there's nothing to do
	// send it
	static_cast<TestbedAccount *>( account() )->server()->sendMessage(
			message.to().first()->contactId(),
			message.plainBody() );
	// give it back to the manager to display
	manager()->appendMessage( message );
	// tell the manager it was sent successfully
	manager()->messageSucceeded();
}

void TestbedContact::receivedMessage( const QString &message )
{
	// Create a KopeteMessage as a reply
	KopeteMessage *newMessage;
	KopeteContactPtrList contactList;
	contactList.append( account()->myself() );
	QString reply;
	// perform contact specific actions
	if ( contactId() == "echo" )
	{	
		reply = message;
		kdDebug( 14210 ) << k_funcinfo << " message for echo is: " << message << endl;
	
	}
	else if ( contactId() == "statuschanger" )
	{
		kdDebug( 14210 ) << k_funcinfo << " message for statuschanger is: " << message << endl;
		if ( m_actionTimer.isActive() )
		{
			reply = "stopping.";
			m_actionTimer.stop();
		}
		else
		{
			reply = "starting.";
			m_actionTimer.start( 5000 ); // todo: read interval from message
		}
	}
	// Add it to the manager
	newMessage = new KopeteMessage( this, contactList, reply, KopeteMessage::Inbound );
	manager()->appendMessage (*newMessage);
	delete newMessage;
}

void TestbedContact::slotMessageManagerDestroyed()
{
	//FIXME: the chat window was closed?  Take appropriate steps.
	m_msgManager = 0L;
}

void TestbedContact::slotNextAction()
{
	if ( onlineStatus() ==  TestbedProtocol::protocol()->testbedOffline )
	{
		kdDebug( 14210 ) << "offline, going online" << endl;
		setOnlineStatus( TestbedProtocol::protocol()->testbedOnline );
	}
	else if ( onlineStatus() == TestbedProtocol::protocol()->testbedOnline )
	{
		kdDebug( 14210 ) << "online, going away" << endl;
		setOnlineStatus( TestbedProtocol::protocol()->testbedAway );
	}
	else
	{
		kdDebug( 14210 ) << "away, going offline" << endl;
		setOnlineStatus( TestbedProtocol::protocol()->testbedOffline );
	}
}

#include "testbedcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

