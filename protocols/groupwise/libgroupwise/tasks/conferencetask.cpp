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
#include "conferencetask.h"

ConferenceTask::ConferenceTask( Task* parent )
 : EventTask( parent )
{
	// register all the events that this task monitors
	registerEvent( EventTransfer::ConferenceClosed );
	registerEvent( EventTransfer::ConferenceJoined );
	registerEvent( EventTransfer::ConferenceLeft );
	registerEvent( EventTransfer::ReceiveMessage );
	registerEvent( EventTransfer::UserTyping );
	registerEvent( EventTransfer::UserNotTyping );
	registerEvent( EventTransfer::ConferenceInvite );
	registerEvent( EventTransfer::ConferenceInviteNotify );
	registerEvent( EventTransfer::ConferenceReject );
	registerEvent( EventTransfer::ReceiveAutoReply );
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
		qDebug( "Got a status change!" );
		QDataStream din( incomingEvent->payload(), IO_ReadOnly);
		din.setByteOrder( QDataStream::LittleEndian );
		
		ConferenceEvent event;
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

		switch ( incomingEvent->event() )
		{
			case EventTransfer::ConferenceClosed:
				qDebug( "ConferenceClosed" );
				dumpConferenceEvent( event );
				emit closed( event );
				break;
			case EventTransfer::ConferenceJoined:
				event.flags = readFlags( din );
				qDebug( "ConferenceJoined" );
				dumpConferenceEvent( event );
				emit joined( event );
				break;
			case EventTransfer::ConferenceLeft:
				event.flags = readFlags( din );
				qDebug( "ConferenceLeft" );
				dumpConferenceEvent( event );
				emit left( event );
				break;
			case EventTransfer::ReceiveMessage:
				event.flags = readFlags( din );
				event.message = readMessage( din );
				qDebug( "ReceiveMessage" );
				dumpConferenceEvent( event );
				qDebug( "message: %s\n", event.message.ascii() );
				emit message( event );
				break;
			case EventTransfer::UserTyping:
				qDebug( "UserTyping" );
				dumpConferenceEvent( event );
				emit typing( event );
				break;
			case EventTransfer::UserNotTyping:
				qDebug( "UserNotTyping" );
				dumpConferenceEvent( event );
				emit notTyping( event );
				break;
			case EventTransfer::ConferenceInvite:
				event.message = readMessage( din );
				qDebug( "ConferenceInvite" );
				dumpConferenceEvent( event );
				qDebug( "message: %s\n", event.message.ascii() );
				emit invited( event );
				break;
			case EventTransfer::ConferenceInviteNotify:
				qDebug( "ConferenceInviteNotify" );
				dumpConferenceEvent( event );
				emit otherInvited( event );
				break;
			case EventTransfer::ConferenceReject:
				qDebug( "ConferenceReject" );
				dumpConferenceEvent( event );
				emit invitationRejected( event );
				break;
			case EventTransfer::ReceiveAutoReply:
				event.flags = readFlags( din );
				event.message = readMessage( din );
				qDebug( "ReceiveAutoReply" );
				dumpConferenceEvent( event );
				qDebug( "message: %s\n", event.message.ascii() );
				emit autoReply( event );
				break;
			default:
				qDebug( "WARNING: didn't handle registered event %i, on conference %s\n", incomingEvent->event(), event.guid.ascii() );
		}
		return true;
	}
	return false;
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

#include "conferencetask.moc"
