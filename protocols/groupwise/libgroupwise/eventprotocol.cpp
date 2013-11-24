/*
    Kopete Groupwise Protocol
    eventprotocol.cpp - reads the protocol used by GroupWise for signalling Events

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
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

#include "eventprotocol.h"

#include <qbuffer.h>

#include "gwerror.h"

#include "eventtransfer.h"

using namespace GroupWise;

EventProtocol::EventProtocol(QObject *parent)
 : InputProtocolBase(parent)
{
}

EventProtocol::~EventProtocol()
{
}

Transfer * EventProtocol::parse( QByteArray & wire, uint& bytes )
{
	m_bytes = 0;
	//m_din = new QDataStream( wire, QIODevice::ReadOnly );
	QBuffer inBuf( &wire );
	inBuf.open( QIODevice::ReadOnly); 
	m_din.setDevice( &inBuf );
	m_din.setByteOrder( QDataStream::LittleEndian );
	quint32 type;

	if ( !okToProceed() )
	{
		m_din.unsetDevice();
		return 0;
	}
	// read the event type
	m_din >> type;
	m_bytes += sizeof( quint32 );
	
	debug( QString( "EventProtocol::parse() Reading event of type %1" ).arg( type ) );
	if ( type > Stop )
	{
		debug( QString ( "EventProtocol::parse() - found unexpected event type %1 - assuming out of sync" ).arg( type ) );
		m_state = OutOfSync;
		return 0;
	}

	// read the event source
	QString source;
	if ( !readString( source ) )
	{
		m_din.unsetDevice();
		return 0;
	}
	
	// now create an event object
	//HACK: lowercased DN
	EventTransfer * tentative = new EventTransfer( type, source.toLower(), QDateTime::currentDateTime() );

	// add any additional data depending on the type of event
	// Note: if there are any errors in the way the data is read below, we will soon be OutOfSync
	QString statusText;
	QString guid;
	quint16 status;
	quint32 flags;
	QString message;
	
	switch ( type )
	{
		case StatusChange: //103 - STATUS + STATUSTEXT
			if ( !okToProceed() )
			{
				m_din.unsetDevice();
				return 0;
			}
			m_din >> status;
			m_bytes += sizeof( quint16 );
			if ( !readString( statusText ) )
			{
				m_din.unsetDevice();
				return 0;
			}
			debug( QString( "got status: %1").arg( status ) );
			tentative->setStatus( status );
			debug( QString( "tentative status: %1").arg( tentative->status() ) );
			tentative->setStatusText( statusText );
			break;
		case ConferenceJoined:		// 106 - GUID + FLAGS
		case ConferenceLeft:		// 107
			if ( !readString( guid ) )
			{
				m_din.unsetDevice();
				return 0;
			}
			tentative->setGuid( guid );
			if ( !readFlags( flags ) )
			{
				m_din.unsetDevice();
				return 0;
			}
			tentative->setFlags( flags );
			break;
		case UndeliverableStatus:	//102 - GUID
		case ConferenceClosed:		//105
		case ConferenceInviteNotify://118
		case ConferenceReject:		//119
		case UserTyping:			//112
		case UserNotTyping:			//113
			if ( !readString( guid ) )
			{
				m_din.unsetDevice();
				return 0;
			}
			tentative->setGuid( guid );
			break;
		case ReceiveAutoReply:		//121 - GUID + FLAGS + MESSAGE
		case ReceiveMessage:		//108
			// guid
			if ( !readString( guid ) )
			{
				m_din.unsetDevice();
				return 0;
			}
			tentative->setGuid( guid );
			// flags
			if ( !readFlags( flags ) )
			{
				m_din.unsetDevice();
				return 0;
			}
			tentative->setFlags( flags );
			// message
			if ( !readString( message ) )
			{
				m_din.unsetDevice();
				return 0;
			}
			tentative->setMessage( message );
			break;
		case ConferenceInvite:		//117 GUID + MESSAGE
			// guid
			if ( !readString( guid ) )
			{
				m_din.unsetDevice();
				return 0;
			}
			tentative->setGuid( guid );
			// message
			if ( !readString( message ) )
			{
				m_din.unsetDevice();
				return 0;
			}
			tentative->setMessage( message );
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
		/* GW7 */
		case ReceivedBroadcast:		//122
		case ReceivedSystemBroadcast: //123
			// message
			if ( !readString( message ) )
			{
				m_din.unsetDevice();
				return 0;
			}
			tentative->setMessage( message );
			break;
		default:
			debug( QString( "EventProtocol::parse() - found unexpected event type %1" ).arg( type ) );
			break;
	}
	// if we got this far, the parse succeeded, return the Transfer
	m_state = Success;
	//delete m_din;
	bytes = m_bytes;
	m_din.unsetDevice();
	return tentative;
}

bool EventProtocol::readFlags( quint32 &flags)
{
	if ( okToProceed() )
	{
		m_din >> flags;
		m_bytes += sizeof( quint32 );
		return true;
	}
	return false;
}

#include "eventprotocol.moc"
