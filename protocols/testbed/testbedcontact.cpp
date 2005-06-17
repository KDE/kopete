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
#include "kopetechatsessionmanager.h"
#include "kopetemetacontact.h"

#include "testbedaccount.h"
#include "testbedfakeserver.h"
#include "testbedprotocol.h"

TestbedContact::TestbedContact( Kopete::Account* _account, const QString &uniqueName,
		const TestbedContactType type, const QString &displayName, Kopete::MetaContact *parent )
: Kopete::Contact( _account, uniqueName, parent )
{
	kdDebug( 14210 ) << k_funcinfo << " uniqueName: " << uniqueName << ", displayName: " << displayName << endl;
	m_type = type;
	// FIXME: ? setDisplayName( displayName );
	m_msgManager = 0L;

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
	}
	serializedData[ "contactType" ] = value;
}

Kopete::ChatSession* TestbedContact::manager( CanCreateFlags )
{
	kdDebug( 14210 ) << k_funcinfo << endl;
	if ( m_msgManager )
	{
		return m_msgManager;
	}
	else
	{
		QPtrList<Kopete::Contact> contacts;
		contacts.append(this);
		m_msgManager = Kopete::ChatSessionManager::self()->create(account()->myself(), contacts, protocol());
		connect(m_msgManager, SIGNAL(messageSent(Kopete::Message&, Kopete::ChatSession*)),
				this, SLOT( sendMessage( Kopete::Message& ) ) );
		connect(m_msgManager, SIGNAL(destroyed()), this, SLOT(slotChatSessionDestroyed()));
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

void TestbedContact::sendMessage( Kopete::Message &message )
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
	// Create a Kopete::Message
	Kopete::Message *newMessage;
	Kopete::ContactPtrList contactList;
	account();
	contactList.append( account()->myself() );
	newMessage = new Kopete::Message( this, contactList, message, Kopete::Message::Inbound );

	// Add it to the manager
	manager()->appendMessage (*newMessage);

	delete newMessage;
}

void TestbedContact::slotChatSessionDestroyed()
{
	//FIXME: the chat window was closed?  Take appropriate steps.
	m_msgManager = 0L;
}

#include "testbedcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

