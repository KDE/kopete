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

#include <qdeepcopy.h>
#include <qtextcodec.h>


Oscar::Message::Message()
: m_channel( -1 ),
  m_properties( -1 ),
  m_messageType( 0 ),
  m_protocolVersion( 0 ),
  m_channel2Counter( 0 ),
  m_encoding( UserDefined )
{
}

Oscar::Message::Message( Encoding messageEncoding, const QByteArray& messageText, int channel, int properties, QDateTime timestamp )
: m_channel( channel ),
  m_properties( properties ),
  m_messageType( 0 ),
  m_protocolVersion( 0 ),
  m_channel2Counter( 0 ),
  m_textArray( messageText ),
  m_timestamp( timestamp ),
  m_encoding( messageEncoding )
{
}

Oscar::Message::Message( Encoding messageEncoding, const QCString& messageText, int channel, int properties, QDateTime timestamp )
: m_channel( channel ),
  m_properties( properties ),
  m_messageType( 0 ),
  m_protocolVersion( 0 ),
  m_channel2Counter( 0 ),
  m_timestamp( timestamp ),
  m_encoding( messageEncoding )
{
	setTextArray( messageText );
}

Oscar::Message::Message( Encoding messageEncoding, const QString& messageText, int channel, int properties, QDateTime timestamp, QTextCodec* codec )
: m_channel( channel ),
  m_properties( properties ),
  m_messageType( 0 ),
  m_protocolVersion( 0 ),
  m_channel2Counter( 0 ),
  m_timestamp( timestamp )
{
	setText( messageEncoding, messageText, codec );
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

QByteArray Oscar::Message::textArray() const
{
    return m_textArray;
}

QString Oscar::Message::text( QTextCodec *codec ) const
{
	switch ( m_encoding )
	{
	case Oscar::Message::UserDefined:
		return codec->toUnicode( m_textArray );
	case Oscar::Message::UTF8:
		return QString::fromUtf8( m_textArray.data(), m_textArray.size() );
	case Oscar::Message::UCS2:
	{
		uint len = m_textArray.size() / 2;
		QString result;
		result.setLength( len );
		QByteArray::ConstIterator p = m_textArray.begin();
		for ( uint i = 0; i < len; i++)
		{
			char row = *p++;
			char cell = *p++;
			result[i] = QChar( cell, row );
		}
		//check if last character isn't null
		if ( result[len-1].isNull() )
			result.setLength( len - 1 );

		return result;
	}
	default:
		break; // Should never happen.
	}
	return QString::null;
	//FIXME: warn at least with kdWarning if an unrecognised encoding style was seen.
}

void Oscar::Message::setText( Oscar::Message::Encoding newEncoding, const QString& newText, QTextCodec* codec )
{
	uint len;
	switch ( newEncoding )
	{
	case Oscar::Message::UserDefined:
		// Oscar::Message::setTextArray( const QCString& )
		// strips trailing null byte automatically.
		setTextArray( codec->fromUnicode( newText ) );
		break;
	case Oscar::Message::UTF8:
		// Oscar::Message::setTextArray( const QCString& )
		// strips trailing null byte automatically.
		setTextArray( newText.utf8() );
		break;
	case Oscar::Message::UCS2:
	{
		len = newText.length();
		m_textArray.resize( len * 2 );
		QByteArray::Iterator p = m_textArray.begin();
		for ( uint i = 0; i < len; i++)
		{
			*p++ = newText[i].row();
			*p++ = newText[i].cell();
		}
		break;
	}
	default:
		break; // Should never happen.
	}
	m_encoding = newEncoding;
}

void Oscar::Message::setTextArray( const QByteArray& newTextArray )
{
	m_textArray.duplicate( newTextArray );
}

void Oscar::Message::setTextArray( const QCString& newTextArray )
{
	m_textArray.duplicate( newTextArray );
	uint len = m_textArray.size();
	if ( len > 0 )
	{
		--len;
		if ( m_textArray[len] == '\0' )
		{
			// Strip trailing null byte.
			m_textArray.resize( len );
		}
	}
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

Oscar::Message::Encoding Oscar::Message::encoding() const
{
	return m_encoding;
}

void Oscar::Message::setEncoding( Oscar::Message::Encoding newEncoding )
{
	m_encoding = newEncoding;
}

Oscar::Message::operator bool() const
{
	return m_channel != -1 && m_properties != -1;
}

//kate: indent-mode csands; auto-insert-doxygen on; tab-width 4;

