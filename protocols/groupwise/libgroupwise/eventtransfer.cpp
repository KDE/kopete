/*
    eventtransfer.cpp - Kopete Groupwise Protocol
    
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

#include "eventtransfer.h"

EventTransfer::EventTransfer( const quint32 eventType, const QString & source, QDateTime timeStamp )
 : Transfer(), m_eventType( eventType ), m_source( source ), m_timeStamp( timeStamp )
{
	m_contentFlags |= ( EventType | Source | TimeStamp );
}


EventTransfer::~EventTransfer()
{
}

// query contents

bool EventTransfer::hasEventType()
{
	return ( m_contentFlags & EventType );
}

bool EventTransfer::hasSource()
{
	return ( m_contentFlags & Source );
}

bool EventTransfer::hasTimeStamp()
{
	return ( m_contentFlags & TimeStamp );
}

bool EventTransfer::hasGuid()
{
	return ( m_contentFlags & Guid );
}

bool EventTransfer::hasFlags()
{
	return ( m_contentFlags & Flags );
}

bool EventTransfer::hasMessage()
{
	return ( m_contentFlags & Message );
}

bool EventTransfer::hasStatus()
{
	return ( m_contentFlags & Status );
}

bool EventTransfer::hasStatusText()
{
	return ( m_contentFlags & StatusText );
}

// accessors
	
int EventTransfer::eventType() const
{ 
	return m_eventType;
}

QString EventTransfer::source()
{
	return m_source;
}

QDateTime EventTransfer::timeStamp()
{
	return m_timeStamp;
}

GroupWise::ConferenceGuid EventTransfer::guid()
{
	return m_guid;
}

quint32 EventTransfer::flags()
{
	return m_flags;
}

QString EventTransfer::message()
{
	return m_message;
}

quint16 EventTransfer::status()
{
	return m_status;
}

QString EventTransfer::statusText()
{
	return m_statusText;
}

// mutators
void EventTransfer::setGuid( const GroupWise::ConferenceGuid & guid )
{
	m_contentFlags |= Guid;
	m_guid = guid;
}

void EventTransfer::setFlags( const quint32 flags )
{
	m_contentFlags |= Flags;
	m_flags = flags;
}

void EventTransfer::setMessage( const QString & message )
{
	m_contentFlags |= Message;
	m_message = message;
}

void EventTransfer::setStatus( const quint16 inStatus )
{
	m_contentFlags |= Status;
	m_status = inStatus;
}

void EventTransfer::setStatusText( const QString & statusText )
{
	m_contentFlags |= StatusText;
	m_statusText = statusText;
}

