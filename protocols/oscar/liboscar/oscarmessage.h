/*
    Kopete Oscar Protocol
    oscarmessage.h - Oscar Message Object

    Copyright (c) 2005 Matt Rogers <mattr@kde.org>
    Copyright (c) 2005 Conrad Hoffmann <conrausch@gmx.de>
    Copyright (c) 2005 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
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

#ifndef _OSCARMESSAGE_H_
#define _OSCARMESSAGE_H_


#include <QDateTime>
#include <QSharedDataPointer>

#include "liboscar_export.h"
#include "oscartypes.h"

class QTextCodec;

namespace Oscar
{

	class MessagePlugin;

/**
 * This class is responsible for holding all the details
 * of a message and includes the following:
 * \li channel ( type )
 * \li encoding
 */

namespace MessageType
{
	enum {
		Unknown      = 0x00, // Unknown
		Plain        = 0x01, // Plain text (simple) message
		Chat         = 0x02, // Chat request message
		File         = 0x03, // File request / file ok message
		Url          = 0x04, // URL message (0xFE formatted)
		AuthRequest  = 0x06, // Authorization request message (0xFE formatted)
		AuthDeny     = 0x07, // Authorization denied message (0xFE formatted)
		AuthGranted  = 0x08, // Authorization given message (empty)
		Server       = 0x09, // Message from OSCAR server (0xFE formatted)
		Added        = 0x0C, // "You-were-added" message (0xFE formatted)
		WebPager     = 0x0D, // Web pager message (0xFE formatted)
		EmailExpress = 0x0E, // Email express message (0xFE formatted)
		ContactList  = 0x13, // Contact list message
		Plugin       = 0x1A, // Plugin message described by text string
		AutoAway     = 0xE8, // Auto away message
		AutoBusy     = 0xE9, // Auto occupied message
		AutoNA       = 0xEA, // Auto not available message
		AutoDND      = 0xEB, // Auto do not disturb message
		AutoFFC      = 0xEC  // Auto free for chat message
	};
}

class LIBOSCAR_EXPORT Message
{
public:

	enum {
		Normal = 0x0000,
		AutoResponse = 0x0001,
		WWP = 0x0002,
		EMail = 0x0004,
		ChatRoom = 0x0008,
		Request = 0x0100,
		StatusMessageRequest = 0x0200
	};

	enum Encoding {
		UserDefined,
		ASCII,
		LATIN1,
		UTF8,
		UCS2
	};

	Message();

// 	Message( Encoding messageEncoding, const QByteArray& messageText, int channel, int properties, QDateTime timestamp );
// 	Message( Encoding messageEncoding, const QString& messageText, int channel, int properties, QDateTime timestamp, QTextCodec* codec = 0 );

	Message( const Message& m );
	Message& operator=( const Message& m );
	~Message();

	/** Get the sender of the message */
	QString sender() const;

	/** Set the sender of the message */
	void setSender( const QString& sender );

	/** Get the receiver of the message */
	QString receiver() const;

	/** Set the receiver of the message */
	void setReceiver( const QString& receiver);

	/** get the message text */
	QString text( QTextCodec* codec ) const;

	/** get best encoding for text */
	static Message::Encoding encodingForText( const QString& newText, bool allowUCS2 = false );

	/** set the message text */
	void setText( Encoding newEncoding, const QString& newText, QTextCodec* codec  = 0);

	/** get the message text as a bytearray for decoding */
	QByteArray textArray() const;

	/** set the message text as a bytearray for decoding */
	void setTextArray( const QByteArray& newTextArray );

	/** get the message properties */
	int properties() const;

	/** ask about a specific property */
	bool hasProperty( int prop ) const;

	/** add a property to the message */
	void addProperty( int prop );

	/** get the channel ( NOT type ) of the message */
	int channel() const;

	/** set the channel ( NOT type ) of the message */
	void setChannel( int newChannel );

	/** get the timestamp of the message */
	QDateTime timestamp() const;

	/** set the timestamp of the message */
	void setTimestamp( QDateTime ts );

	/** get the ICBM cookie of the message */
	QByteArray icbmCookie() const;

	/** set the ICBM cookie of the message */
	void setIcbmCookie( const QByteArray& cookie );

	/** get the protocol version (channel 2 messages only) */
	int protocolVersion() const;

	/** prepare for handling of different protocol versions */
	void setProtocolVersion( int version );

	/** get the channel 2 counter value of the message */
	int channel2Counter() const; // channel 2 message have an additional counter whose value needs be kept in a request response

	/** set the channel 2 counter value */
	void setChannel2Counter( int value );

	/** get the message (content) type */
	int messageType() const;

	/** set the message (content) type */
	void setMessageType( int type );

	/** get the request type (req/cancel/accept) */
	int requestType() const;

	/** set the request type (req/cancel/accept) */
	void setRequestType( int type );

	/** get the port */
	int port() const;

	/** set the port (for transfer requests) */
	void setPort( int port );

	/** get the proxy ip*/
	QByteArray proxy() const;

	/** set the proxy ip (for transfer requests) */
	void setProxy( QByteArray proxy );

	/** get the request # */
	int requestNumber() const;

	/** set the request # (for transfer requests) */
	void setRequestNumber( int n );

	/** get the file name */
	QString fileName() const;

	/** get file(s) size */
	DWORD filesSize() const;

	/** get file count */
	WORD fileCount() const;

	/** set the file name (for transfer requests) */
	void setFileName( const QString &name );

	/** set file(s) size (for transfer requests)*/
	void setFilesSize( DWORD size );
	
	/** set file count (for transfer requests)*/
	void setFileCount( WORD count );

    /** get the exchange for the chat room this message is for */
    Oscar::WORD exchange() const;

    /** set the exchange for the chat room this message is for */
    void setExchange( Oscar::WORD );

    /** get the chat room that this message is for */
    QString chatRoom() const;

    /** set the chat room that this message is for */
    void setChatRoom( const QString& );

	/** get the message encoding */
	Encoding encoding() const;

	/** set the message encoding */
	void setEncoding( Encoding newEncoding );

	/** get the message plugin */
	MessagePlugin* plugin() const;

	/** set the message plugin
	 *  The message deletes old plugin when a new plugin is set.
	 */
	void setPlugin( MessagePlugin* plugin );

	/** Get the id of the message */
	uint id() const;

	/** Set the id of the message */
	void setId( uint id );

	operator bool() const;

private:
	class MessagePrivate;
	QSharedDataPointer<MessagePrivate> d;

};

}

//kate: indent-mode csands; auto-insert-doxygen on; tab-width 4;

#endif
