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
#include <klocale.h>

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

	connect ( this, SIGNAL( messageSent ( KopeteMessage &, KopeteMessageManager * ) ),
			  SLOT( slotMessageSent ( KopeteMessage &, KopeteMessageManager * ) ) );
	connect( this, SIGNAL( typingMsg ( bool ) ), SLOT( slotSendTypingNotification ( bool ) ) );
	connect( account(), SIGNAL( contactTyping( const ConferenceEvent & ) ), SLOT( slotGotTypingNotification( const ConferenceEvent & ) ) );
	connect( account(), SIGNAL( contactNotTyping( const ConferenceEvent & ) ), SLOT( slotGotNotTypingNotification( const ConferenceEvent & ) ) );
	
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

bool GroupWiseMessageManager::closed()
{
	return m_flags & GroupWise::Closed;
}

bool GroupWiseMessageManager::logging()
{
	return m_flags & GroupWise::Logging;
}

bool GroupWiseMessageManager::secure()
{
	return m_flags & GroupWise::Secure;
}

void GroupWiseMessageManager::setClosed()
{
	m_flags = m_flags | GroupWise::Closed;
}

void GroupWiseMessageManager::setLogging( bool logging )
{
	if ( logging )
		m_flags = m_flags | GroupWise::Logging;
	else
		m_flags = m_flags & !GroupWise::Logging;
}

void GroupWiseMessageManager::setSecure( bool secure )
{
	if ( secure )
		m_flags = m_flags | GroupWise::Secure;
	else
		m_flags = m_flags & !GroupWise::Secure;
}

void GroupWiseMessageManager::updateDisplayName()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "NOT IMPLEMENTED" << endl;
}

GroupWiseAccount *  GroupWiseMessageManager::account()
{
	return static_cast<GroupWiseAccount *>( KopeteMessageManager::account() );
}

void GroupWiseMessageManager::createConference()
{
	if ( m_guid.isEmpty() )
	{
		kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
		// form a list of invitees
		QStringList invitees;
		KopeteContactPtrList chatMembers = members();
		for ( KopeteContact * contact = chatMembers.first(); contact; contact = chatMembers.next() )
		{
			invitees.append( contact->contactId() );
		}
		// this is where we will set the GUID and send any pending messages
		connect( account(), SIGNAL( conferenceCreated( const int, const QString & ) ), SLOT( receiveGuid( const int, const QString & ) ) );
		connect( account(), SIGNAL( conferenceCreationFailed( const int, const int ) ), SLOT( slotCreationFailed( const int, const int ) ) );
		
		// create the conference
		account()->createConference( mmId(), invitees );
	}
	else
		kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " tried to create conference on the server when it was already instantiated" << endl;
}

void GroupWiseMessageManager::receiveGuid( const int newMmId, const QString & guid )
{
	if ( newMmId == mmId() )
	{
		kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " got GUID from server" << endl;
		setGuid( guid );
		// notify the contact(s) using this message manager that it's been instantiated on the server
		emit conferenceCreated();
		// TODO: send invitations if we're not inviting in the conf create...
		dequeMessages();
	}
}

void GroupWiseMessageManager::slotCreationFailed( const int failedId, const int statusCode )
{
	if ( failedId == mmId() )
	{
		kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " couldn't start a chat, no GUID.\n" << endl;
		//emit creationFailed();
		KopeteMessage failureNotify = KopeteMessage(/*m_manager->user(),  m_manager->members()*/ user(), members(), i18n("An error occurred when trying to start a chat: %1").arg( statusCode ), KopeteMessage::Internal, KopeteMessage::PlainText);
		appendMessage( failureNotify );
		setClosed();
	}
}

void GroupWiseMessageManager::slotSendTypingNotification( bool typing )
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	if ( !m_guid.isEmpty() )
		account()->client()->sendTyping( guid(), typing );
}

void GroupWiseMessageManager::slotMessageSent( KopeteMessage & message, KopeteMessageManager * )
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	if( account()->isConnected() )
	{
		if ( closed() )
		{
			KopeteMessage failureNotify = KopeteMessage(/*m_manager->user(),  m_manager->members()*/ user(), members(), i18n("Your message could not be sent. This conversation has been closed by the server, because all the other participants left or declined invitations. "), KopeteMessage::Internal, KopeteMessage::PlainText);
			appendMessage( failureNotify );
			messageSucceeded();
		}
		else
		{
			if ( m_guid.isEmpty() )
			{
				kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << "waiting for server to create a conference, queuing message" << endl;
				// the conference hasn't been instantiated on the server yet, so queue the message
				m_pendingOutgoingMessages.append( message );
			}
			else 
			{
				kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << "sending message" << endl;
				account()->sendMessage( guid(), message );
				// we could wait until the server acks our send, 
				// but we'd need a UID for outgoing messages and a list to track them
				appendMessage( message );
				messageSucceeded();
			}
		}
	}
}

void GroupWiseMessageManager::slotGotTypingNotification( const ConferenceEvent& event )
{
	if ( event.guid == guid() )
		receivedTypingMsg( event.user, true );
}

void GroupWiseMessageManager::slotGotNotTypingNotification( const ConferenceEvent& event )
{
	if ( event.guid == guid() )
		receivedTypingMsg( event.user, false );
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
