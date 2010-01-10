/*
    Kopete Oscar Protocol
    oscarmessage.cpp - Oscar Message Object

    Copyright (c) 2005 Matt Rogers <mattr@kde.org>
    Copyright (c) 2005 Conrad Hoffmann <conrausch@gmx.de>
    Copyright (c) 2006-2007 Roman Jarosz <kedgedev@centrum.cz>

    Kopete (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

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

#include <QTextCodec>
#include <QByteArray>
#include <QSharedData>

#include "oscarmessageplugin.h"

namespace Oscar
{

class Message::MessagePrivate : public QSharedData
{
public:
	MessagePrivate()
		: QSharedData()
	{
		channel = -1;
		properties = 0;
		messageType = 0;
		requestType = 0;
		port = 0;
		requestNumber = 1;
		protocolVersion = 0;
		channel2Counter = 0;
		encoding = UserDefined;
		fileSize = 0;
		fileCount = 0;
		plugin = 0;
		id = 0;
	}
	MessagePrivate( const MessagePrivate &other )
		: QSharedData( other )
	{
		sender = other.sender;
		receiver = other.receiver;
		channel = other.channel;
		properties = other.properties;
		messageType = other.messageType;
		requestType = other.requestType;
		port = other.port;
		requestNumber = other.requestNumber;
		protocolVersion = other.protocolVersion;
		channel2Counter = other.channel2Counter;
		icbmCookie = other.icbmCookie;
		proxy = other.proxy;
		textArray = other.textArray;
		timestamp = other.timestamp;
		exchange = other.exchange;
		chatRoom = other.chatRoom;
		encoding = other.encoding;
		fileName = other.fileName;
		fileSize = other.fileSize;
		fileCount = other.fileCount;
		id = other.id;

		if ( other.plugin )
			plugin = new MessagePlugin(*other.plugin);
		else
			plugin = 0;
	}

	~MessagePrivate()
	{
		delete plugin;
	}

	QString sender;
	QString receiver;
	int channel;
	int properties;
	int messageType;
	int requestType;
	int port;
	int requestNumber;
	int protocolVersion;
	int channel2Counter;
	QByteArray icbmCookie;
	QByteArray proxy;
	QByteArray textArray;
	QDateTime timestamp;
	WORD exchange;
	QString chatRoom;
	Encoding encoding;
	QString fileName;
	DWORD fileSize;
	WORD fileCount;
	MessagePlugin* plugin;
	uint id;
};

Message::Message()
	: d( new MessagePrivate )
{
}
/*
Message::Message( Encoding messageEncoding, const QByteArray& messageText, int channel, int properties, QDateTime timestamp )
	: d( new MessagePrivate )
{
	d->channel = channel;
	d->properties = properties;
	d->textArray = messageText;
	d->timestamp = timestamp;
	d->encoding = messageEncoding;
}

Message::Message( Encoding messageEncoding, const QString& messageText, int channel, int properties, QDateTime timestamp, QTextCodec* codec )
	: d( new MessagePrivate )
{
	d->channel = channel;
	d->properties = properties;
	d->timestamp = timestamp;

	setText( messageEncoding, messageText, codec );
}
*/
Message::Message( const Message& m )
	: d( m.d )
{
}

Message& Message::operator=( const Message& m )
{
	d = m.d;
	return *this;
}

Message::~Message()
{
}

QString Message::sender() const
{
	return d->sender;
}

void Message::setSender( const QString& sender  )
{
	d->sender = sender;
}

QString Message::receiver() const
{
	return d->receiver;
}

void Message::setReceiver( const QString& receiver )
{
	d->receiver = receiver;
}

QByteArray Message::textArray() const
{
    return d->textArray;
}

QString Message::text( QTextCodec *codec ) const
{
	switch ( d->encoding )
	{
	case Message::UserDefined:
		return codec->toUnicode( d->textArray );
	case Message::ASCII:
		return QString::fromAscii( d->textArray.data(), d->textArray.size() );
	case Message::LATIN1:
		return QString::fromLatin1( d->textArray.data(), d->textArray.size() );
	case Message::UTF8:
		return QString::fromUtf8( d->textArray.data(), d->textArray.size() );
	case Message::UCS2:
	{
		uint len = d->textArray.size() / 2;
		QString result;
		result.resize( len );
		QByteArray::ConstIterator p = d->textArray.begin();
		for ( uint i = 0; i < len; i++)
		{
			char row = *p++;
			char cell = *p++;
			result[i] = QChar( cell, row );
		}
		//check if last character isn't null
		if ( result.at(len-1).isNull() )
			result.resize( len - 1 );

		return result;
	}
	default:
		break; // Should never happen.
	}
	return QString();
	//FIXME: warn at least with kdWarning if an unrecognised encoding style was seen.
}

Message::Encoding Message::encodingForText( const QString& newText, bool allowUCS2 )
{
	Message::Encoding encoding = Message::ASCII;
	const QChar *ch = newText.constData();
	const int len = newText.length();

	for ( int i = 0; i < len; ++i )
	{
		if ( ch[i] > 0xff )
		{
			encoding = ( allowUCS2 ) ? Message::UCS2 : Message::UserDefined;
			break;
		}
		else if ( encoding == Message::ASCII && ch[i] > 0x7f )
		{
			encoding = Message::LATIN1;
		}
	}

	return encoding;
}

void Message::setText( Message::Encoding newEncoding, const QString& newText, QTextCodec* codec )
{
	uint len;
	switch ( newEncoding )
	{
	case Message::UserDefined:
		// Message::setTextArray( const QCString& )
		// strips trailing null byte automatically.
		setTextArray( codec->fromUnicode( newText ) );
		break;
	case Message::ASCII:
		setTextArray( newText.toAscii() );
		break;
	case Message::LATIN1:
		setTextArray( newText.toLatin1() );
		break;
	case Message::UTF8:
		// Message::setTextArray( const QCString& )
		// strips trailing null byte automatically.
		setTextArray( newText.toUtf8() );
		break;
	case Message::UCS2:
	{
		len = newText.length();
		d->textArray.resize( len * 2 );
		QByteArray::Iterator p = d->textArray.begin();
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
	d->encoding = newEncoding;
}

void Message::setTextArray( const QByteArray& newTextArray )
{
	d->textArray = newTextArray;
}

int Message::properties() const
{
	return d->properties;
}

void Message::addProperty( int prop )
{
	d->properties = d->properties | prop;
}

bool Message::hasProperty( int prop ) const
{
	if ( ( d->properties & prop ) == 0 )
		return false;
	else
		return true;
}

int Message::channel() const
{
	return d->channel;
}

void Message::setChannel( int newChannel )
{
	d->channel = newChannel;
}

QDateTime Message::timestamp() const
{
	return d->timestamp;
}

void Message::setTimestamp( QDateTime ts )
{
	d->timestamp = ts;
}

QByteArray Message::icbmCookie() const
{
	return d->icbmCookie;
}

void Message::setIcbmCookie( const QByteArray& cookie )
{
	d->icbmCookie = cookie;
}

int Message::protocolVersion() const
{
	return d->protocolVersion;
}

void Message::setProtocolVersion( int version )
{
	d->protocolVersion = version;
}

int Message::channel2Counter() const
{
	return d->channel2Counter;
}

void Message::setChannel2Counter( int value )
{
	d->channel2Counter = value;
}

int Message::messageType() const
{
	return d->messageType;
}

void Message::setMessageType( int type )
{
	d->messageType = type;
}

int Message::requestType() const
{
	return d->requestType;
}

void Message::setRequestType( int type )
{
	d->requestType = type;
}

int Message::port() const
{
	return d->port;
}

void Message::setPort( int port )
{
	d->port = port;
}

QByteArray Message::proxy() const
{
	return d->proxy;
}

void Message::setProxy( QByteArray proxy )
{
	d->proxy = proxy;
}

int Message::requestNumber() const
{
	return d->requestNumber;
}

void Message::setRequestNumber( int n )
{
	d->requestNumber = n;
}

QString Message::fileName() const
{
	return d->fileName;
}

DWORD Message::filesSize() const
{
	return d->fileSize;
}

WORD Message::fileCount() const
{
	return d->fileCount;
}

void Message::setFileName( const QString &name )
{
	d->fileName = name;
}

void Message::setFilesSize( DWORD size )
{
	d->fileSize = size;
}

void Message::setFileCount( WORD count )
{
	d->fileCount = count;
}

WORD Message::exchange() const
{
    return d->exchange;
}

void Message::setExchange( WORD exchange )
{
    d->exchange = exchange;
}

QString Message::chatRoom() const
{
    return d->chatRoom;
}

void Message::setChatRoom( const QString& room )
{
    d->chatRoom = room;
}

Message::Encoding Message::encoding() const
{
	return d->encoding;
}

void Message::setEncoding( Message::Encoding newEncoding )
{
	d->encoding = newEncoding;
}

MessagePlugin* Message::plugin() const
{
	return d->plugin;
}

void Message::setPlugin( MessagePlugin* plugin )
{
	if ( d->plugin )
		delete d->plugin;

	d->plugin = plugin;
}

uint Message::id() const
{
	return d->id;
}

void Message::setId( uint id )
{
	d->id = id;
}

Message::operator bool() const
{
	return d->channel != -1;
}

}

//kate: indent-mode csands; auto-insert-doxygen on; tab-width 4;

