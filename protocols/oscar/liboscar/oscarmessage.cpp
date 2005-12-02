/*
    Kopete Oscar Protocol
    oscarmessage.cpp - Oscar Message Object

    Copyright (c) 2005 Matt Rogers <mattr@kde.org>
    Copyright (c) 2005 Conrad Hoffmann <conrausch@gmx.de>

    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "oscarmessage.h"

#include <q3deepcopy.h>


Oscar::Message::Message()
{
	m_channel = -1;
	m_properties = -1;
}

Oscar::Message::Message( const QString& text, int channel, int properties, QDateTime timestamp )
{
	m_text = text;
	m_channel = channel;
	m_properties = properties;
	m_timestamp = timestamp;
}

Oscar::Message::Message( const Oscar::Message& m )
{
	m_text = m.m_text;
	m_channel = m.m_channel;
	m_properties = m.m_properties;
	m_timestamp = m.m_timestamp;
	m_icbmCookie.truncate( 0 );
	m_icbmCookie.duplicate( m.m_icbmCookie );
	m_protocolVersion = m.m_protocolVersion;
	m_channel2Counter = m.m_channel2Counter;
	m_messageType = m.m_messageType;
}

QString Oscar::Message::sender() const
{
	return m_sender;
}

void Oscar::Message::setSender( const QString& sender  )
{
	m_sender = sender;
}

QString Oscar::Message::receiver() const
{
	return m_receiver;
}

void Oscar::Message::setReceiver( const QString& receiver )
{
	m_receiver = receiver;
}

QString Oscar::Message::text() const
{
	return m_text;
}

void Oscar::Message::setText( const QString& newText )
{
	m_text = newText;
}

QByteArray Oscar::Message::textArray() const
{
    return m_textArray;
}

void Oscar::Message::setTextArray( const QByteArray& newTextArray )
{
    m_textArray.duplicate( newTextArray );
}

int Oscar::Message::properties() const
{
	return m_properties;
}

void Oscar::Message::addProperty( int prop )
{
	if ( m_properties == -1  )
		m_properties = 0;

	m_properties = m_properties | prop;
}

bool Oscar::Message::hasProperty( int prop ) const
{
	if ( m_properties == -1 )
		return false;
	if ( ( m_properties & prop ) == 0 )
		return false;
	else
		return true;
}

int Oscar::Message::type() const
{
	return m_channel;
}

void Oscar::Message::setType( int newType )
{
	m_channel = newType;
}

QDateTime Oscar::Message::timestamp() const
{
	return m_timestamp;
}

void Oscar::Message::setTimestamp( QDateTime ts )
{
	m_timestamp = ts;
}

QByteArray Oscar::Message::icbmCookie() const
{
	return m_icbmCookie;
}

void Oscar::Message::setIcbmCookie( const QByteArray& cookie )
{
	m_icbmCookie.duplicate( cookie );
}

int Oscar::Message::protocolVersion() const
{
	return m_protocolVersion;
}

void Oscar::Message::setProtocolVersion( int version )
{
	m_protocolVersion = version;
}

int Oscar::Message::channel2Counter() const
{
	return m_channel2Counter;
}

void Oscar::Message::setChannel2Counter( int value )
{
	m_channel2Counter = value;
}

int Oscar::Message::messageType() const
{
	return m_messageType;
}

void Oscar::Message::setMessageType( int type )
{
	m_messageType = type;
}

Oscar::WORD Oscar::Message::exchange() const
{
    return m_exchange;
}

void Oscar::Message::setExchange( Oscar::WORD exchange )
{
    m_exchange = exchange;
}

QString Oscar::Message::chatRoom() const
{
    return m_chatRoom;
}

void Oscar::Message::setChatRoom( const QString& room )
{
    m_chatRoom = room;
}

Oscar::Message::operator bool() const
{
	return m_channel != -1 && m_properties != -1;
}

//kate: indent-mode csands; auto-insert-doxygen on; tab-width 4;

