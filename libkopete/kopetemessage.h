/*
    kopetemessage.h  -  Base class for Kopete messages

    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef __KOPETE_MESSAGE_H__
#define __KOPETE_MESSAGE_H__

#include <qptrlist.h>
#include <qstring.h>
#include <qdom.h>
#include <qcolor.h>
#include <qfont.h>
#include <qdatetime.h>

class KopeteContact;
class QDateTime;
class KopeteMessageManager;
struct KopeteMessagePrivate;

typedef QPtrList<KopeteContact> KopeteContactPtrList;

/**
 * @author Martijn Klingens <klingens@kde.org>
 * @author Olivier Goffart <ogoffart@tiscalinet.be>
 *
 * The KopeteMessage represent any kind of messages showed on the chatwindow.
 *
 * The message may be a simple plain text string, or a Rich text HTML like message,
 * this flag is indicated by the @ref format() flag.
 * PlainText message can however have a color, or speccifics fonts with the flag
 * @ref bg() @ref fg() @ref font()
 * It is recemanded to use theses flag even for RichText message, so the user can disable
 * custom colors in the chat window style.
 */
class KopeteMessage
{
public:
	/**
	 * Direction of a message.
	 * - Inbound is from the chat partner
	 * - Outbound is from the user.
	 * - Internal messages are messages which are not send via the network. this is just a notification than plugin can show on the chat window
	 * - Action is for the /me command , like on irc
	 */
	enum MessageDirection { Inbound = 0, Outbound = 1, Internal= 2, Action = 3 };

	/**
	 * Format of the body
	 * - PlainText: Just a simple text, without any formating. If there are html tag on it, they simply be shown in the chatwindow.
     * - RichText: Text already HTML escaped and which can contains some tags. the string should be a valid (X)HTML string. Any html special
	 *   charactere (<,>,&,...) are escaped to the equivalent html entity (&gt;,...) newlines are <br \> and any other html tags will be interpreted.
	 * - ParsedHTML: only used by the chatwindow, this text is parsed and ready to
	 *  show into the chatwindow, with Emoticons, and urls
	 * - Crypted is used only by Jabber and the Cryptography plugin
	 */
	enum MessageFormat{ PlainText = 0x01 , RichText =0x02 , ParsedHTML = 0x04|RichText , Crypted = 0x08|PlainText};

	/**
	* Specifies the type of the view.
	* May currently be of type
	* - Chat: Two way chat
	* - Email: Single shot messaging
	*/
	enum MessageType { Undefined, Chat, Email };

	/**
	* Specifies the type of notification that will be sent with this message
	* - Low: almost no notifications. automaticaly used in groupChat
	* - Normal: Default notification, for normal message
	* - Highlight: Highlight notification, for most important messages, which require particular attentions.
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
	 * Constructs a new message. See @ref setBody() to more information about the format
	 * @param fromKC The KopeteContact that the message is coming from
	 * @param toKC List of KopeteContacts the message is going to
	 * @param body Message body
	 * @param direction The direction of the message, KopeteMessage::Inbound, KopeteMessage::Outbound, KopeteMessage::Internal
	 * @param format Format of the message
	 */
	KopeteMessage( const KopeteContact *fromKC, const KopeteContactPtrList &toKC, const QString &body,
		MessageDirection direction, MessageFormat format = PlainText, MessageType type = Undefined );

	/**
	 * Constructs a new message. See @ref setBody() to more information about the format
	 * @param fromKC The KopeteContact that the message is coming from
	 * @param toKC List of KopeteContacts the message is going to
	 * @param body Message body
	 * @param direction The direction of the message, KopeteMessage::Inbound, KopeteMessage::Outbound, KopeteMessage::Internal
	 * @param format Format of the message
	 */
	KopeteMessage( const KopeteContact *fromKC, const KopeteContact *toKC, const QString &body,
		MessageDirection direction, MessageFormat format = PlainText, MessageType type = Undefined );


	/**
	 * Constructs a new message. See @ref setBody() to more information about the format
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
	 * Constructs a new message. See @ref setBody() to more information about the format
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
	 * Constructs a new message. See @ref setBody() to more information about the format
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
	 * Copy constructor.
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

	/**
	 * @return the @ref MessageType of this message
	 */
	MessageType type() const;

	/**
	 * Accessor method for the foreground color
	 * @return The message's foreground color
	 */
	QColor fg() const;

	/**
	 * Accessor method for the background color of the message
	 * @return The message's background color
	 */
	QColor bg() const;

	/**
	 * Accessor method for the font of the message
	 * @return The message's font
	 */
	QFont font() const;

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
	 * @brief Accessor method for the importance
	 * @return The message importance (low/normal/highlight)
	 */
	MessageImportance importance() const;

	/**
	 * @brief Set the importance.
	 *
	 * see @importance
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
	 * @brief Sets the body of the message
	 *
	 * if format is PlainText the text is a simple brute QString, without any formating
	 * if there are html tag on it, they simply be shown in the chatwindow.
	 *
	 * if format is RichText, then the format must be a valid (X)HTML string. any html special
	 * charactere (<,>,&,...) must be escaped to the equivalent html etity (&gt;,...) newlines are <br \> and
	 * any other html tags will be interpreted.
	 *
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
	 * Get the message body as escaped (x)html format.
	 * That mean every HTML special char (>,<,&,...) are escaped to the html entity (&lt; , ...)
	 * and \n are converted to <br />
	 * @return The message body as escaped text
	 */
	QString escapedBody() const ;

	/**
	 * Get the message body as parsed HTML with Emoticons, and URL parsed
	 * this should be ready to be shown in the chatwindow.
	 * @return The HTML and Emoticon parsed message body
	 */
	QString parsedBody() const;

	/**
	 * Get the related kopete message manager.
	 * If it is not set, returns 0L.
	 *
	 * The kopeteMessagemanager is only set if the message is already passed by the manager.
	 * We should trust this only in aboutToSend/aboutToReceive signals
	 */
	 KopeteMessageManager *manager()const ;

	 /**
	  * set the kopetemessagemanager for this message.
	  * should be only used by the manager itself
	  */
	 void setManager(KopeteMessageManager *);

	/**
	 * get a XML version of this message
	 */
	const QDomDocument asXML() const;

	/**
	 * Enables the use of a background for a message
	 * @param enable A flag to indicate if the background should be enabled or disabled.
	 */
	void setBgOverride( bool enable );

	/**
	* Unescapes a string, removing XML entitiy references
	*
	* @param xml The string you want to unescape
	*/
	static QString unescape( const QString &xml );

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

	QString parseLinks( const QString &message ) const;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

