//
// C++ Implementation: gwmessagemanager
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <kdebug.h>
#include <kopetecontact.h>
#include <kopetemessagemanagerfactory.h>
#include <kopeteprotocol.h>

#include "client.h"
#include "gwaccount.h"
#include "gwerror.h"

#include "gwmessagemanager.h"

GroupWiseMessageManager::GroupWiseMessageManager(const KopeteContact* user, KopeteContactPtrList others, KopeteProtocol* protocol, const QString & guid, int id, const char* name): KopeteMessageManager(user, others, protocol, 0, name), m_guid( guid )
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "New message manager for " << user->contactId() << endl;

	// make sure Kopete knows about this instance
	KopeteMessageManagerFactory::factory()->addKopeteMessageManager ( this );

	connect ( this, SIGNAL ( messageSent ( KopeteMessage &, KopeteMessageManager * ) ),
			  this, SLOT ( slotMessageSent ( KopeteMessage &, KopeteMessageManager * ) ) );

	connect ( this, SIGNAL ( typingMsg ( bool ) ), this, SLOT ( slotSendTypingNotification ( bool ) ) );
	
	if ( m_guid.isEmpty() )
		createConference();

	updateDisplayName();
}

GroupWiseMessageManager::~GroupWiseMessageManager()
{
}

void GroupWiseMessageManager::setGuid( const QString & guid )
{
	if ( m_guid.isEmpty() )
	{
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "setting GUID to: " << guid << endl;
		m_guid = guid;
	}
	else
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "attempted to change the conference's GUID when already set!" << endl;
}

void GroupWiseMessageManager::updateDisplayName()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "NOT IMPLEMENTED" << endl;
}

void GroupWiseMessageManager::createConference()
{
	if ( m_guid.isEmpty() )
	{
		kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
		GroupWiseAccount * acct = static_cast<GroupWiseAccount *>( account() );
		// form a list of invitees
		QStringList invitees;
		KopeteContactPtrList chatMembers = members();
		for ( KopeteContact * contact = chatMembers.first(); contact; contact = chatMembers.next() )
		{
			invitees.append( contact->contactId() );
		}
		// this is where we will set the GUID and send any pending messages
		connect( acct, SIGNAL( conferenceCreated( const int, const QString & ) ), SLOT( receiveGuid( const int, const QString & ) ) );
		// create the conference
		acct->createConference( mmId(), invitees );
	}
	else
		kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " tried to create conference on the server when it was already instantiated" << endl;
}

void GroupWiseMessageManager::receiveGuid( const int newMmId, const QString & guid )
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " got GUID from server" << endl;
	if ( newMmId == mmId() )
	{
		setGuid( guid );
		// notify the contact(s) using this message manager that it's been instantiated on the server
		emit conferenceCreated();
		// TODO: send invitations if we're not inviting in the conf create...
		dequeMessages();
	}
}

void GroupWiseMessageManager::slotSendTypingNotification( bool typing )
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "NOT IMPLEMENTED" << endl;
}

void GroupWiseMessageManager::slotMessageSent( KopeteMessage & message, KopeteMessageManager * )
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	if( account()->isConnected() )
	{
	
		kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
		if ( m_guid.isEmpty() )
		{
			// the conference hasn't been instantiated on the server yet, so queue the message
			m_pendingOutgoingMessages.append( message );
		}
		else 
		{
			GroupWiseAccount * acct = static_cast<GroupWiseAccount *>( account() );
			acct->sendMessage( guid(), message );
			// we could wait until the server acks our send, 
			// but we'd need a UID for outgoing messages and a list to track them
			messageSucceeded();
		}
	}
}

void GroupWiseMessageManager::dequeMessages()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	for ( QValueListIterator< KopeteMessage > it = m_pendingOutgoingMessages.begin();
		  it != m_pendingOutgoingMessages.end();
		  ++it )
	{
		slotMessageSent( *it, this );
	}
}
#include "gwmessagemanager.moc"
