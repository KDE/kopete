/*
    Kopete Oscar Protocol
    oscarmessage.h - Oscar Message Object

    Copyright (c) 2005 Matt Rogers <mattr@kde.org>
    Copyright (c) 2005 Conrad Hoffmann <conrausch@gmx.de>
    Copyright (c) 2005 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

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

#ifndef _OSCARMESSAGE_H_
#define _OSCARMESSAGE_H_

#include <qglobal.h>
#include <qstring.h>
#include <qcstring.h>
#include <qdatetime.h>
#include "kopete_export.h"
#include "oscartypes.h"

class QTextCodec;

namespace Oscar
{

/**
 * This class is responsible for holding all the details
 * of a message and includes the following:
 * \li channel ( type )
 * \li encoding
 */

class KOPETE_EXPORT Message
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
		UTF8,
		UCS2
	};

	Message();

	Message( Encoding messageEncoding, const QByteArray& messageText, int channel, int properties, QDateTime timestamp );
	Message( Encoding messageEncoding, const QCString& messageText, int channel, int properties, QDateTime timestamp );
	Message( Encoding messageEncoding, const QString& messageText, int channel, int properties, QDateTime timestamp, QTextCodec* codec = 0 );

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

	/** set the message text */
	void setText( Encoding newEncoding, const QString& newText, QTextCodec* codec  = 0);

	/** get the message text as a bytearray for decoding */
	QByteArray textArray() const;

	/** set the message text as a bytearray for decoding */
	void setTextArray( const QByteArray& newTextArray );

	/** set the mesasge text as a bytearray for decoding */
	void setTextArray( const QCString& newTextArray );

	/** get the message properties */
	int properties() const;

	/** ask about a specific property */
	bool hasProperty( int prop ) const;

	/** add a property to the message */
	void addProperty( int prop );

	/** get the channel ( type ) of the message */
	int type() const;

	/** set the channel ( type ) of the message */
	void setType( int newType );

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

	operator bool() const;

private:
    //TODO d-pointer
	QString m_sender;
	QString m_receiver;
	int m_channel;
	int m_properties;
	int m_messageType;
	int m_protocolVersion;
	int m_channel2Counter;
	QByteArray m_icbmCookie;
	QByteArray m_textArray;
	QDateTime m_timestamp;
	Oscar::WORD m_exchange;
	QString m_chatRoom;
	Encoding m_encoding;
};

}

//kate: indent-mode csands; auto-insert-doxygen on; tab-width 4;

#endif
