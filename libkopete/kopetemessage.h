/*
    kopetemessage.h  -  Base class for Kopete messages

    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef __KOPETE_MESSAGE_H__
#define __KOPETE_MESSAGE_H__

#include <qptrlist.h>
#include <qstring.h>
#include <qdom.h>

#include "kopetecontact.h"

typedef QPtrList<KopeteContact> KopeteContactPtrList;

class QDateTime;

struct KopeteMessagePrivate;

/**
 * @author Martijn Klingens <klingens@kde.org>
 */
class KopeteMessage
{
public:
	/**
	 * Direction of a message. Inbound is from the chat partner, Outbound is
	 * from the user.
	 */
	enum MessageDirection { Inbound, Outbound, Internal, Action };

	/**
	 * Format of the body
	 * -PlainText: Just a simple text
	 * -RichText: Text already HTML escaped and which can contains some tags
	 * -ParsedHTML: only used by the chatwindow, this text is parsed and ready to
	 *  show into the chatwindow
	 * -Crypted is used only by Jabber and the cryptography plugin
	 */
	enum MessageFormat{ PlainText = 0x01 , RichText =0x02 , ParsedHTML = 0x06 , Crypted = 0x09};

	/**
	* Specifies the type of the view.
	* May currently be of type
	* - Chat: Two way chat
	* - Email: Single shot messaging
	*/
	enum MessageType { Undefined, Chat, Email };

	/**
	* Specifies the type of notification will be send with this message
	* - Low: almost no notifications used in groupChat
	* - Normal: default notification
	* - Highlight: Highlight notification
	*/
	enum MessageImportance { Low = 0, Normal = 1, Highlight = 2 };

	/**
	 * Constructs a new empty message
	 */
	KopeteMessage();

	/**
	 * Deref and clean private object if refcount == 0
	 */
	~KopeteMessage();

	/**
	 * Constructs a new message
	 * @param fromKC The KopeteContact that the message is coming from
	 * @param toKC List of KopeteContacts the message is going to
	 * @param body Message body
	 * @param direction The direction of the message, KopeteMessage::Inbound, KopeteMessage::Outbound, KopeteMessage::Internal
	 * @param format Format of the message
	 */
	KopeteMessage( const KopeteContact *fromKC, const KopeteContactPtrList &toKC, const QString &body,
		MessageDirection direction, MessageFormat format = PlainText, MessageType type = Undefined );

	/**
	 * Constructs a new message
	 * @param fromKC The KopeteContact that the message is coming from
	 * @param toKC List of KopeteContacts the message is going to
	 * @param body Message body
	 * @param subject The subject of the message
	 * @param direction The direction of the message, KopeteMessage::Inbound, KopeteMessage::Outbound, KopeteMessage::Internal
	 * @param format Format of the message
	 */
	KopeteMessage( const KopeteContact *fromKC, const KopeteContactPtrList &toKC, const QString &body,
		const QString &subject, MessageDirection direction, MessageFormat format = PlainText, MessageType type = Undefined );

	/**
	 * Constructs a new message
	 * @param timeStamp Timestamp for the message
	 * @param fromKC The KopeteContact that the message is coming from
	 * @param toKC List of KopeteContacts the message is going to
	 * @param body Message body
	 * @param direction The direction of the message, KopeteMessage::Inbound, KopeteMessage::Outbound, KopeteMessage::Internal
	 * @param format Format of the message
	 */
	KopeteMessage( const QDateTime &timeStamp, const KopeteContact *fromKC, const KopeteContactPtrList &toKC,
		const QString &body, MessageDirection direction, MessageFormat format = PlainText, MessageType type = Undefined );

	/**
	 * Constructs a new message
	 * @param timeStamp Timestamp for the message
	 * @param fromKC The KopeteContact that the message is coming from
	 * @param toKC List of KopeteContacts the message is going to
	 * @param body Message body
	 * @param subject The subject of the message
	 * @param direction The direction of the message, KopeteMessage::Inbound, KopeteMessage::Outbound, KopeteMessage::Internal
	 * @param format Format of the message
	 */
	KopeteMessage( const QDateTime &timeStamp, const KopeteContact *fromKC, const KopeteContactPtrList &toKC,
		const QString &body, const QString &subject, MessageDirection direction,
		MessageFormat format = PlainText, MessageType type = Undefined );

	/**
	 * Copy constructor
	 * Just adds a reference, doesn't actually copy.
	 */
	KopeteMessage( const KopeteMessage &other );

	/**
	 * Assignment operator
	 * Just like the copy constructor it just refs and doesn't copy.
	 */
	KopeteMessage & operator=( const KopeteMessage &other );

	/**
	 * Accessor method for the timestamp of the message
	 * @return The message's timestamp
	 */
	QDateTime timestamp() const;

	/**
	 * Accessor method for the KopeteContact that sent this message
	 * @return The KopeteContact who sent this message
	 */
	const KopeteContact * from() const;

	/**
	 * Accessor method for the KopeteContacts that this message was sent to
	 * @return Pointer list of the KopeteContacts this message was sent to
	 */
	KopeteContactPtrList to() const;

	MessageType type() const;

	/**
	 * Acessor method for the foreground color
	 * @return The message's foreground color
	 */
	QColor fg() const;

	/**
	 * Accessor method for the background color of the message
	 * @return The message's background color
	 */
	QColor bg() const;

	/**
	 * Accessor method for the font used in the message
	 * @return The message's font
	 */
	QFont font() const;

	/**
	 * Accessor method for the body of the message in the current format
	 * @return The message body
	 */
	QString body() const;

	/**
	 * Accessor method for the subject of the message
	 * @return The message subject
	 */
	QString subject() const;

	/**
	 * Accessor method for the format of the message
	 * @return The message format
	 */
	MessageFormat format() const;

	/**
	 * Accessor method for the direction of the message
	 * @return The message direction
	 */
	MessageDirection direction() const;

	/**
	 * Accessor method for the importance
	 * @return The message importance (low/normal/highlight)
	 */
	MessageImportance importance() const;

	/**
	 * set the importance
	 */
	void setImportance(MessageImportance );

	/**
	 * Sets the foreground color for the message
	 * @param color The color
	 */
	void setFg( const QColor &color );

	/**
	 * Sets the background color for the message
	 * @param The color
	 */
	void setBg( const QColor &color );

	/**
	 * Sets the font for the message
	 * @param font The font
	 */
	void setFont( const QFont &font );

	/**
	 * Sets the body of the message
	 * @param body The body
	 * @param format The format of the message
	 */
	void setBody( const QString &body, MessageFormat format = PlainText );

	/**
	 * Get the message body back as plain text
	 * @return The message body as plain text
	 */
	QString plainBody() const ;

	/**
	 * Get the message body as escaped text
	 * @return The message body as escaped text
	 */
	QString escapedBody() const ;

	/**
	 * Get the message body as parsed HTML with Emoticons,
	 * this should be ready to show in the chatwindow
	 * @internal
	 * @return The HTML and Emoticon parsed message body
	 */
	QString parsedBody() const ;

	/**
	 * internal Functions
	 *
	 */
	QString asHTML() const;

	QDomDocument asXML();

	QString transformMessage( const QString &model ) const;

	void setBgOverride( bool enable );

	/**
	 * Use it to parse HTML in text.
	 * You dont need to use this for chat windows,
	 * There is a special class that abstract a chat view
	 * and uses HTML parser.
	 */
	static QString parseHTML( const QString &message, bool parseURLs = true );

	/**
	 * Highlight the message with the default highlight options
	 */
	void highlight();

private:
	/**
	 * Helper for constructors
	 */
	void init( const QDateTime &timeStamp, const KopeteContact *from, const KopeteContactPtrList &to,
		const QString &body, const QString &subject, MessageDirection direction, MessageFormat f, MessageType type );

	/**
	 * KopeteMessage is implicitly shared.
	 * Detach the instance when modifying data.
	 */
	void detach();

	KopeteMessagePrivate *d;

	/**
	 * Compares colors. If the colors are too simmilar (they are deemed
	 * not be readable on top of eachother), then colorFg is adjusted to become
	 * readable.
	 */
	void compareColors( QColor &colorFg, QColor &colorBg );

	/**
	 * Helper method for transformMessage for formatting names for display
	 */
	QString formatDisplayName( const QString &name ) const;

	/**
	 * Helper method for transformMessage for finding closing % tag
	 */
	int findClosingTag( const QString &model, int openTag ) const;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

