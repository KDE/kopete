/*
	kopetemessage.h  -  Base class for Kopete messages

    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef _KOPETE_MESSAGE_H
#define _KOPETE_MESSAGE_H

#include <qdatetime.h>
#include <qstring.h>
#include <qfont.h>
#include <qcolor.h>

#include "kopetecontact.h"
#include "qptrlist.h"

typedef QPtrList<KopeteContact> KopeteContactPtrList;

/**
 * @author Martijn Klingens <klingens@kde.org>
 *
 */

class KopeteMessage
{
public:
	/**
		Direction of a message. Inbound is from the chat partner, Outbound is
		from the user.
	*/
	enum MessageDirection { Inbound, Outbound, Internal};

	/**
		Format of the body
		-PlainText: Just a simple text
		-RichText: Text already HTML escaped and which can contains some tags
		-ParsedHTML: only used by the chatwindow, this text is parsed and ready to
			show into the chatwindow
	*/
	enum MessageFormat  { PlainText, RichText, ParsedHTML };

	/**
	 * Constructs a new empty message
	 */
	KopeteMessage();

	/**
	 * Constructs a new message
	 * @param fromKC The KopeteContact that the message is coming from
	 * @param toKC List of KopeteContacts the message is going to
	 * @param body Message body
	 * @param direction The direction of the message, KopeteMessage::Inbound, KopeteMessage::Outbound, KopeteMessage::Internal
	 * @param format Format of the message
	 */
	KopeteMessage(const KopeteContact *fromKC, KopeteContactPtrList toKC,
									QString body, MessageDirection direction, MessageFormat format=PlainText );

	/**
	 * Constructs a new message
	 * @param fromKC The KopeteContact that the message is coming from
	 * @param toKC List of KopeteContacts the message is going to
	 * @param body Message body
	 * @param subject The subject of the message
	 * @param direction The direction of the message, KopeteMessage::Inbound, KopeteMessage::Outbound, KopeteMessage::Internal
	 * @param format Format of the message
	 */
	KopeteMessage(const KopeteContact* fromKC, KopeteContactPtrList toKC, QString body,
									QString subject, MessageDirection direction, MessageFormat format=PlainText );

	/**
	 * Constructs a new message
	 * @param timeStamp Timestamp for the message
	 * @param fromKC The KopeteContact that the message is coming from
	 * @param toKC List of KopeteContacts the message is going to
	 * @param body Message body
	 * @param direction The direction of the message, KopeteMessage::Inbound, KopeteMessage::Outbound, KopeteMessage::Internal
	 * @param format Format of the message
	 */
	KopeteMessage(QDateTime timeStamp, const KopeteContact *fromKC, KopeteContactPtrList toKC,
									QString body, MessageDirection direction, MessageFormat format=PlainText);

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
	KopeteMessage(QDateTime timeStamp, const KopeteContact *fromKC, KopeteContactPtrList toKC,
									QString body, QString subject, MessageDirection direction, MessageFormat format=PlainText);


	// Accessors

	/**
	 * Accessor method for the timestamp of the message
	 * @return The message's timestamp
	 */
	QDateTime timestamp() const { return mTimestamp; }

	/**
	 * Accessor method for the KopeteContact that sent this message
	 * @return The KopeteContact who sent this message
	 */
	const KopeteContact *from() const { return mFrom; }

	/**
	 * Accessor method for the KopeteContacts that this message was sent to
	 * @return Pointer list of the KopeteContacts this message was sent to
	 */
	KopeteContactPtrList to() const { return mTo; }

	/**
	 * Acessor method for the foreground color
	 * @return The message's foreground color
	 */
	QColor fg() const { return mFg; }

	/**
	 * Accessor method for the background color of the message
	 * @return The message's background color
	 */
	QColor bg() const { return mBg; }

	/**
	 * Accessor method for the font used in the message
	 * @return The message's font
	 */
	QFont font() const { return mFont; }

	/**
	 * Accessor method for the body of the message in the current format
	 * @return The message body
	 */
	QString body() const { return mBody; }

	/**
	 * Accessor method for the subject of the message
	 * @return The message subject
	 */
	QString subject() const { return mSubject; }

	/**
	 * Accessor method for the format of the message
	 * @return The message format
	 */
	MessageFormat format() const { return mFormat; }

	/**
	 * Accessor method for the direction of the message
	 * @return The message direction
	 */
	MessageDirection direction() const { return mDirection; }

	/**
	 * Sets the foreground color for the message
	 * @param color The color
	 */
	void setFg(QColor color);

	/**
	 * Sets the background color for the message
	 * @param The color
	 */
	void setBg(QColor color);

	/**
	 * Sets the font for the message
	 * @param font The font
	 */
	void setFont(QFont font);

	/**
	 * Sets the body of the message
	 * @param body The body
	 * @param format The format of the message
	 */
	void setBody( const QString& body , MessageFormat format=PlainText );

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
	 * @return The HTML and Emoticon parsed message body
	 */
	QString parsedBody() const ;

	/**
	 * Functions
	 *
	 */
	QString asHTML() const;
	
	QString transformMessage( const QString &model ) const;

protected:
	// Helper for constructors
	void init(QDateTime timeStamp, const KopeteContact * from, KopeteContactPtrList to,
			  QString body, QString subject, MessageDirection direction, MessageFormat f);

	/** Timestamp */
	QDateTime mTimestamp;
	/** Contact the message came from */
	const KopeteContact *mFrom;
	/** Pointer list of contacts the message is going to */
	KopeteContactPtrList mTo;
	/** Message body */
	QString mBody;
	/** Message subject */
	QString mSubject;
	/** Message font */
	QFont mFont;
	/** The foreground color */
	QColor mFg;
	/** The background color */
	QColor mBg;
	/** The message direction */
	MessageDirection mDirection;
	/** The message format */
	MessageFormat mFormat;

public:
	/**
	 * Use it to parse HTML in text.
	 * You dont need to use this for chat windows,
	 * There is a special class that abstract a chat view
	 * and uses HTML parser.
	 **/
	static QString parseHTML( QString message, bool parseURLs = true );
};

#endif





/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

