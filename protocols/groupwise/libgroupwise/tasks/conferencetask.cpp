//
// C++ Implementation: conferencetask
//
// Description: 
//
//
// Author: SUSE AG (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "client.h"
#include "userdetailsmanager.h"

#include "conferencetask.h"

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
	
	// listen to the UserDetailsManager telling us that user details are available
	connect( client()->userDetailsManager(), SIGNAL( gotContactDetails( const GroupWise::ContactDetails & ) ), 
		SLOT( slotReceiveUserDetails( const GroupWise::ContactDetails & ) ) );
}


ConferenceTask::~ConferenceTask()
{
}

void dumpConferenceEvent( ConferenceEvent & evt )
{
	qDebug( "Conference Event - guid: %s user: %s timestamp: %i:%i:%i flags: %08x\n",
			evt.guid.ascii(), evt.user.ascii(), evt.timeStamp.time().hour(), evt.timeStamp.time().minute(), evt.timeStamp.time().second(), evt.flags );
}

bool ConferenceTask::take( Transfer * transfer )
{
	EventTransfer * incomingEvent;
	if ( forMe( transfer, incomingEvent ) )
	{
		qDebug( "Got a conference event:" );
		QDataStream din( incomingEvent->payload(), IO_ReadOnly);
		din.setByteOrder( QDataStream::LittleEndian );
		
		ConferenceEvent event;
		event.type = (GroupWise::Event)incomingEvent->event();
		event.timeStamp = incomingEvent->timeStamp();
		event.user = incomingEvent->source();
		event.flags = 0;
		// read the conference guid
		uint val;
		char* rawData;
		din.readBytes( rawData, val );
		if ( val ) // val is 0 if there is no status text
			event.guid = QString::fromUtf8( rawData, val );
		//else 
			// set error protocolError

		switch ( event.type )
		{
			case GroupWise::ConferenceClosed:
				qDebug( "ConferenceClosed" );
				emit closed( event );
				break;
			case GroupWise::ConferenceJoined:
				event.flags = readFlags( din );
				qDebug( "ConferenceJoined" );
				if ( !queueWhileAwaitingData( event ) )
					emit joined( event );
				break;
			case GroupWise::ConferenceLeft:
				event.flags = readFlags( din );
				qDebug( "ConferenceLeft" );
				emit left( event );
				break;
			case GroupWise::ReceiveMessage:
				event.flags = readFlags( din );
				event.message = readMessage( din );
				qDebug( "ReceiveMessage" );
				qDebug( "message: %s\n", event.message.ascii() );
				if ( !queueWhileAwaitingData( event ) )
					emit message( event );
				break;
			case GroupWise::UserTyping:
				qDebug( "UserTyping" );
				emit typing( event );
				break;
			case GroupWise::UserNotTyping:
				qDebug( "UserNotTyping" );
				emit notTyping( event );
				break;
			case GroupWise::ConferenceInvite:
				event.message = readMessage( din );
				qDebug( "ConferenceInvite" );
				qDebug( "message: %s\n", event.message.ascii() );
				if ( !queueWhileAwaitingData( event ) )
					emit invited( event );
				break;
			case GroupWise::ConferenceInviteNotify:
				qDebug( "ConferenceInviteNotify" );
				if ( !queueWhileAwaitingData( event ) )
					emit otherInvited( event );
				break;
			case GroupWise::ConferenceReject:
				qDebug( "ConferenceReject" );
				emit invitationRejected( event );
				break;
			case GroupWise::ReceiveAutoReply:
				event.flags = readFlags( din );
				event.message = readMessage( din );
				qDebug( "ReceiveAutoReply" );
				qDebug( "message: %s\n", event.message.ascii() );
				emit autoReply( event );
				break;
			default:
				qDebug( "WARNING: didn't handle registered event %i, on conference %s\n", incomingEvent->event(), event.guid.ascii() );
		}
		dumpConferenceEvent( event );

		return true;
	}
	return false;
}

void ConferenceTask::handleEvent( GroupWise::ConferenceEvent & event )
{
}

Q_UINT32 ConferenceTask::readFlags( QDataStream & din )
{
	Q_UINT32 flags;
	if ( din.atEnd() )
	{
		setError( GroupWise::Protocol );
		return 0;
	}
	else
	{
		din >> flags;
		return flags;
	}
}

QString ConferenceTask::readMessage( QDataStream & din )
{
	QString message;
	uint val;
	char* rawData;
	din.readBytes( rawData, val );
	if ( val ) // val is 0 if there is no status text
		message = QString::fromUtf8( rawData, val );
	 else
		setError( GroupWise::Protocol );
	return message;
}

void ConferenceTask::slotReceiveUserDetails( const GroupWise::ContactDetails & details )
{
	qDebug( "ConferenceTask::slotReceiveUserDetails()" );
	
	// emit the details, so anything waiting for them will update itself
	//client()-›contactUserDetailsReceived(details);
	// dequeue any events which are deliverable now we have these details 
	QValueListIterator< ConferenceEvent > end = m_pendingEvents.end();
	QValueListIterator< ConferenceEvent > it = m_pendingEvents.begin();
	while ( it != end )
	{
		QValueListIterator< ConferenceEvent > current = it;
		++it;
		// if the details relate to event, try again to handle it
		if ( details.dn == (*current).user )
		{
			qDebug( " - got details for event involving %s", (*current).user.ascii() );
			emit temporaryContact( details );
			switch ( (*current).type )
			{
				case GroupWise::ConferenceJoined:
					qDebug( "ConferenceJoined" );
					emit joined( *current );
					break;
				case GroupWise::ReceiveMessage:
					qDebug( "ReceiveMessage" );
					emit message( *current );
					break;
				case GroupWise::ConferenceInvite:
					qDebug( "ConferenceInvite" );
					emit invited( *current );
					break;
				case GroupWise::ConferenceInviteNotify:
					qDebug( "ConferenceInviteNotify" );
					emit otherInvited( *current );
					break;
				default:
					qDebug( "Queued an event while waiting for more data, but didn't write a handler for the dequeue!" );
			}
			m_pendingEvents.remove( current );
			qDebug( "Event handled - now %i pending events", m_pendingEvents.count() );
		}
	}
}


bool ConferenceTask::queueWhileAwaitingData( const ConferenceEvent & event )
{
	if ( client()->userDetailsManager()->known( event.user ) )
	{
		qDebug( "ConferenceTask::queueWhileAwaitingData() -	source is known!" );
		return false;
	}
	else
	{
		qDebug( "ConferenceTask::queueWhileAwaitingData() - queueing event involving %s", event.user.ascii() );
		client()->userDetailsManager()->requestDetails( event.user );
		m_pendingEvents.append( event );
		return true;
	}
}

#include "conferencetask.moc"
