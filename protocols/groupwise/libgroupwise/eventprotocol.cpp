//
// C++ Implementation: eventprotocol
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "gwerror.h"

#include "eventtransfer.h"
#include "eventprotocol.h"

using namespace GroupWise;

EventProtocol::EventProtocol(QObject *parent, const char *name)
 : QObject(parent, name)
{
}

EventProtocol::~EventProtocol()
{
}

uint EventProtocol::parse( const QByteArray & wire, EventTransfer *& transfer )
{
	m_bytes = 0;
	transfer = 0;
	m_din = new QDataStream( wire, IO_ReadOnly );
	m_din->setByteOrder( QDataStream::LittleEndian );
	Q_UINT32 type;

	if ( !okToProceed() )
		return m_bytes;
	// read the event type
	*m_din >> type;
	m_bytes += sizeof( Q_UINT32 );	
	
	qDebug( "EventProtocol::parse() Reading event of type %i", type );
	if ( type > Stop )
	{
		qDebug( "EventProtocol::parse() - found unexpected event type %i - assuming out of sync", type );
		m_state = OutOfSync;
		return m_bytes;
	}

	// read the event source
	QString source;
	if ( !readString( source ) )
		return m_bytes;
	
	// now create an event object
	//HACK: lowercased DN
	transfer = new EventTransfer( type, source.lower(), QDateTime::currentDateTime() );

	// add any additional data depending on the type of event
	// Note: if there are any errors in the way the data is read below, we will soon be OutOfSync
	QString statusText;
	QString guid;
	Q_UINT16 status;
	Q_UINT32 flags;
	QString message;
	
	switch ( type )
	{
		case StatusChange: //103 - STATUS + STATUSTEXT
			if ( !okToProceed() )
				return m_bytes;
			*m_din >> status;
			m_bytes += sizeof( Q_UINT16 );
			if ( !readString( statusText ) )
				return m_bytes;
			qDebug( "got status: %i", status );
			transfer->setStatus( status );
			qDebug( "transfer status: %i", transfer->status() );
			transfer->setStatusText( statusText );
			break;
		case ConferenceJoined:		// 106 - GUID + FLAGS
		case ConferenceLeft:		// 107
			if ( !readString( guid ) )
				return m_bytes;
			transfer->setGuid( guid );
			if ( !readFlags( flags ) )
				return m_bytes;
			transfer->setFlags( flags );
			break;
		case UndeliverableStatus:	//102 - GUID
		case ConferenceClosed:		//105
		case ConferenceInviteNotify://118
		case ConferenceReject:		//119
		case UserTyping:			//112
		case UserNotTyping:			//113
			if ( !readString( guid ) )
				return m_bytes;
			transfer->setGuid( guid );
			break;
		case ReceiveAutoReply:		//121 - GUID + FLAGS + MESSAGE
		case ReceiveMessage:		//108
			// guid
			if ( !readString( guid ) )
				return m_bytes;
			transfer->setGuid( guid );
			// flags
			if ( !readFlags( flags ) )
				return m_bytes;
			transfer->setFlags( flags );
			// message
			if ( !readString( message ) )
				return m_bytes;
			transfer->setMessage( message );
			break;
		case ConferenceInvite:		//117 GUID + MESSAGE
			// guid
			if ( !readString( guid ) )
				return m_bytes;
			transfer->setGuid( guid );
			// message
			if ( !readString( message ) )
				return m_bytes;
			transfer->setMessage( message );
			break;
		case UserDisconnect:		//114 (NOTHING)
		case ServerDisconnect:		//115
			// nothing else to read
			break;
		case InvalidRecipient:		//101
		case ContactAdd:			//104
		case ReceiveFile:			//109
		case ConferenceRename:		//116
			// unhandled because unhandled in Gaim
			break;
		default:
			qDebug( "EventProtocol::parse() - found unexpected event type %i", type );
			break;
	}
	m_state = Success;
	delete m_din;
	return m_bytes;
}

uint EventProtocol::state() const
{
	return m_state;
}

bool EventProtocol::readFlags( Q_UINT32 &flags)
{
	if ( okToProceed() )
	{
		*m_din >> flags;
		m_bytes += sizeof( Q_UINT32 );
		return true;
	}
	return false;
}


bool EventProtocol::readString( QString &message )
{
	uint len;
	QCString rawData;
	if ( !safeReadBytes( rawData, len ) )
		return false;
	message = QString::fromUtf8( rawData.data(), len );
	return true;
}


bool EventProtocol::okToProceed()
{
	if ( m_din )
	{
		if ( m_din->atEnd() )
		{
			m_state = NeedMore;
			qDebug( "EventProtocol::okToProceed() - Server message ended prematurely!" );
		}
		else
			return true;
	}
	return false;
}

bool EventProtocol::safeReadBytes( QCString & data, uint & len )
{
	// read the length of the bytes
	Q_UINT32 val;
	if ( !okToProceed() )
		return false;
	*m_din >> val;
	m_bytes += sizeof( Q_UINT32 );
	
	QCString temp( val );
	if ( val != 0 )
	{
		if ( !okToProceed() )
			return false;
		// if the server splits packets here we are in trouble,
		// as there is no way to see how much data was actually read
		m_din->readRawBytes( temp.data(), val );
		// the rest of the string will be filled with FF,
		// so look for that in the last position instead of \0
		if ( (Q_UINT8)( * ( temp.data() + ( temp.length() - 1 ) ) ) == 0xFF )
		{
			qDebug( "EventProtocol::safeReadBytes() - string broke, giving up" );
			m_state = NeedMore;
			return false;
		}
	}
	data = temp;
	len = val;
	m_bytes += val;
	return true;
}

#include "eventprotocol.moc"
