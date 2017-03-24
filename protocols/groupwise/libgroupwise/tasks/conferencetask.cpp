/*
    Kopete Groupwise Protocol
    conferencetask.cpp - Event Handling task responsible for all conference related events

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "conferencetask.h"

#include "client.h"
#include "userdetailsmanager.h"

ConferenceTask::ConferenceTask( Task* parent )
 : EventTask( parent )
{
	// register all the events that this task monitors
	registerEvent( GroupWise::ConferenceClosed );
	registerEvent( GroupWise::ConferenceJoined );
	registerEvent( GroupWise::ConferenceLeft );
	registerEvent( GroupWise::ReceiveMessage );
	registerEvent( GroupWise::UserTyping );
	registerEvent( GroupWise::UserNotTyping );
	registerEvent( GroupWise::ConferenceInvite );
	registerEvent( GroupWise::ConferenceInviteNotify );
	registerEvent( GroupWise::ConferenceReject );
	registerEvent( GroupWise::ReceiveAutoReply );
	// GW7
	registerEvent( GroupWise::ReceivedBroadcast );
	registerEvent( GroupWise::ReceivedSystemBroadcast );

	// listen to the UserDetailsManager telling us that user details are available
	connect( client()->userDetailsManager(), SIGNAL(gotContactDetails(GroupWise::ContactDetails)), 
		SLOT(slotReceiveUserDetails(GroupWise::ContactDetails)) );
}

ConferenceTask::~ConferenceTask()
{
}

void ConferenceTask::dumpConferenceEvent( ConferenceEvent & evt )
{
	client()->debug( QStringLiteral( "Conference Event - guid: %1 user: %2 timestamp: %3:%4:%5" ).arg
			( evt.guid ).arg( evt.user ).arg( evt.timeStamp.time().hour() ).arg
			( evt.timeStamp.time().minute() ).arg( evt.timeStamp.time().second() ) ); 
 	client()->debug( QStringLiteral( "                  flags: %1" ).arg( evt.flags, 8 ) );
}

bool ConferenceTask::take( Transfer * transfer )
{
	if ( forMe( transfer ) )
	{
		EventTransfer * incomingEvent = static_cast<EventTransfer *>(transfer);
		client()->debug( QStringLiteral("Got a conference event:") );
		ConferenceEvent event;
		event.type = (GroupWise::Event)( incomingEvent->eventType() );
		event.timeStamp = incomingEvent->timeStamp();
		event.user = incomingEvent->source();
		event.flags = 0;
		Q_ASSERT( incomingEvent->hasGuid() );
		event.guid = incomingEvent->guid();
		
		switch ( event.type )
		{
			case GroupWise::ConferenceClosed:
				// extra debug - we never see these events, against spec.
				client()->debug( QStringLiteral("********************") );
				client()->debug( QStringLiteral("* ConferenceClosed *") );
				client()->debug( QStringLiteral("* ConferenceClosed *") );
				client()->debug( QStringLiteral("* ConferenceClosed *") );
				client()->debug( QStringLiteral("********************") );
				emit closed( event );
				break;
			case GroupWise::ConferenceJoined:
				Q_ASSERT( incomingEvent->hasFlags() );
				event.flags = incomingEvent->flags();
				client()->debug( QStringLiteral("ConferenceJoined") );
				if ( !queueWhileAwaitingData( event ) )
					emit joined( event );
				break;
			case GroupWise::ConferenceLeft:
				Q_ASSERT( incomingEvent->hasFlags() );
				event.flags = incomingEvent->flags();
				client()->debug( QStringLiteral("ConferenceLeft") );
				emit left( event );
				break;
			case GroupWise::ReceiveMessage:
				Q_ASSERT( incomingEvent->hasFlags() );
				event.flags = incomingEvent->flags();
				Q_ASSERT( incomingEvent->hasMessage() );
				event.message = incomingEvent->message();
				client()->debug( QStringLiteral("ReceiveMessage") );
				client()->debug( QStringLiteral( "message: %1" ).arg( event.message ) );
				if ( !queueWhileAwaitingData( event ) )
					emit message( event );
				break;
			case GroupWise::UserTyping:
				client()->debug( QStringLiteral("UserTyping") );
				emit typing( event );
				break;
			case GroupWise::UserNotTyping:
				client()->debug( QStringLiteral("UserNotTyping") );
				emit notTyping( event );
				break;
			case GroupWise::ConferenceInvite:
				Q_ASSERT( incomingEvent->hasMessage() );
				event.message = incomingEvent->message();
				client()->debug( QStringLiteral("ConferenceInvite") );
				client()->debug( QStringLiteral( "message: %1" ).arg( event.message ) );
				if ( !queueWhileAwaitingData( event ) )
					emit invited( event );
				break;
			case GroupWise::ConferenceInviteNotify:
				client()->debug( QStringLiteral("ConferenceInviteNotify") );
				if ( !queueWhileAwaitingData( event ) )
					emit otherInvited( event );
				break;
			case GroupWise::ConferenceReject:
				client()->debug( QStringLiteral("ConferenceReject") );
				if ( !queueWhileAwaitingData( event ) )
					emit invitationDeclined( event );
				break;
			case GroupWise::ReceiveAutoReply:
				Q_ASSERT( incomingEvent->hasFlags() );
				event.flags = incomingEvent->flags();
				Q_ASSERT( incomingEvent->hasMessage() );
				event.message = incomingEvent->message();
				client()->debug( QStringLiteral("ReceiveAutoReply") );
				client()->debug( QStringLiteral( "message: %1" ).arg( event.message ) );
				emit autoReply( event );
				break;
			case GroupWise::ReceivedBroadcast:
				Q_ASSERT( incomingEvent->hasMessage() );
				event.message = incomingEvent->message();
				client()->debug( QStringLiteral("ReceivedBroadCast") );
				client()->debug( QStringLiteral( "message: %1" ).arg( event.message ) );
				if ( !queueWhileAwaitingData( event ) )
					emit broadcast( event );
				break;
			case GroupWise::ReceivedSystemBroadcast:
				Q_ASSERT( incomingEvent->hasMessage() );
				event.message = incomingEvent->message();
				client()->debug( QStringLiteral("ReceivedSystemBroadCast") );
				client()->debug( QStringLiteral( "message: %1" ).arg( event.message ) );
				emit systemBroadcast( event );
				break;
			default:
				client()->debug( QStringLiteral( "WARNING: did not handle registered event %1, on conference %2" ).arg( incomingEvent->eventType() ).arg( event.guid ) );
		}
		dumpConferenceEvent( event );

		return true;
	}
	return false;
}

void ConferenceTask::slotReceiveUserDetails( const GroupWise::ContactDetails & details )
{
	client()->debug( QStringLiteral("ConferenceTask::slotReceiveUserDetails()") );
	
	// dequeue any events which are deliverable now we have these details 
	QList< ConferenceEvent >::Iterator end = m_pendingEvents.end();
	QList< ConferenceEvent >::Iterator it = m_pendingEvents.begin();
	while ( it != end )
	{
		// if the details relate to event, try again to handle it
		if ( details.dn == (*it).user )
		{
			client()->debug( QStringLiteral( " - got details for event involving %1" ).arg( (*it).user ) );
			switch ( (*it).type )
			{
				case GroupWise::ConferenceJoined:
					client()->debug( QStringLiteral("ConferenceJoined") );
					emit joined( *it );
					break;
				case GroupWise::ReceiveMessage:
					client()->debug( QStringLiteral("ReceiveMessage") );
					emit message( *it );
					break;
				case GroupWise::ConferenceInvite:
					client()->debug( QStringLiteral("ConferenceInvite") );
					emit invited( *it );
					break;
				case GroupWise::ConferenceInviteNotify:
					client()->debug( QStringLiteral("ConferenceInviteNotify") );
					emit otherInvited( *it );
					break;
				default:
					client()->debug( QStringLiteral("Queued an event while waiting for more data, but did not write a handler for the dequeue!") );
			}
			it = m_pendingEvents.erase( it );
			client()->debug( QStringLiteral( "Event handled - now %1 pending events" ).arg
			( (uint)m_pendingEvents.count() ) );
		} else {
			++it;
        }
	}
}

bool ConferenceTask::queueWhileAwaitingData( const ConferenceEvent & event )
{
	if ( client()->userDetailsManager()->known( event.user ) )
	{
		client()->debug( QStringLiteral("ConferenceTask::queueWhileAwaitingData() - source is known!") );
		return false;
	}
	else
	{
		client()->debug( QStringLiteral( "ConferenceTask::queueWhileAwaitingData() - queueing event involving %1" ).arg( event.user ) );
		client()->userDetailsManager()->requestDetails( event.user );
		m_pendingEvents.append( event );
		return true;
	}
}

