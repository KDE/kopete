/*
    kopetemessage.h  -  Base class for Kopete messages

    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart       <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

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

class QDateTime;
struct KopeteMessagePrivate;

namespace Kopete
{

class Contact;
class MessageManager;
typedef QPtrList<Contact> ContactPtrList;

/**
 * @author Martijn Klingens <klingens@kde.org>
 * @author Olivier Goffart <ogoffart@tiscalinet.be>
 *
 * Kopete::Message represents any kind of messages shown on a chat view.
 *
 * The message may be a simple plaintext string, or a Richtext HTML like message,
 * this is indicated by the @ref format() flag.
 * PlainText message can however have a color, or specific fonts with the flag
 * @ref bg(), @ref fg(), @ref font()
 * It is recommended to use these flags, even for RichText messages, so the user can disable
 * custom colors in the chat window style.
 */
class Message
{
public:
	/**
	 * Direction of a message.
	 * - Inbound: Message is from the chat partner
	 * - Outbound: Message sent by the user.
	 * - Internal: Messages which are not sent via the network. This is just a notification a plugin can show in a chat view
	 * - Action: For the /me command , like on irc
	 */
	enum MessageDirection { Inbound = 0, Outbound = 1, Internal= 2, Action = 3 };

	/**
	 * Format of body
	 * - PlainText: Just a simple text, without any formatting. If it contains HTML tags then they will be simply shown in the chatview.
	 * - RichText: Text already HTML escaped and which can contains some tags. the string
	 *   should be a valid (X)HTML string.
	 *   Any HTML specific characters (\<, \>, \&, ...) are escaped to the equivalent HTML
	 *   entity (\&gt;, \&lt;, ...) newlines are \<br /\> and any other HTML tags will be interpreted.
	 * - ParsedHTML: only used by the chatview, this text is parsed and ready to
	 *  show into the chatview, with Emoticons, and URLs
	 * - Crypted is used only by Jabber and the Cryptography plugin
	 */
	enum MessageFormat{ PlainText = 0x01 , RichText =0x02 , ParsedHTML = 0x04|RichText , Crypted = 0x08|PlainText};

	/**
	 * Specifies the type of the view.
	 * May currently be of type
	 * - Chat: Two way chat
	 * - Email: Single shot messaging
	 */
	enum ViewType { Undefined, Chat, Email };

	/**
	 * Specifies the type of notification that will be sent with this message
	 * - Low: almost no notifications. automatically used in groupChat
	 * - Normal: Default notification, for normal message
	 * - Highlight: Highlight notification, for most important messages, which require particular attentions.
	 */
	enum MessageImportance { Low = 0, Normal = 1, Highlight = 2 };

	/**
	 * Constructs a new empty message
	 */
	Message();

	/**
	 * Deref and clean private object if refcount == 0
	 */
	~Message();

	/**
	 * Constructs a new message. See @ref setBody() to more information about the format
	 * @param fromKC The Kopete::Contact that the message is coming from
	 * @param toKC List of KopeteContacts the message is going to
	 * @param body Message body
	 * @param direction The direction of the message, Kopete::Message::Inbound, Kopete::Message::Outbound, Kopete::Message::Internal
	 * @param format Format of the message
	 * @param type Type of the message, see @ref ViewType
	 */
	Message( const Contact *fromKC, const ContactPtrList &toKC, const QString &body,
		MessageDirection direction, MessageFormat format = PlainText, ViewType type = Undefined );

	/**
	 * Constructs a new message. See @ref setBody() to more information about the format
	 * @param fromKC The Kopete::Contact that the message is coming from
	 * @param toKC List of KopeteContacts the message is going to
	 * @param body Message body
	 * @param direction The direction of the message, Kopete::Message::Inbound, Kopete::Message::Outbound, Kopete::Message::Internal
	 * @param format Format of the message
	 * @param type Type of the message, see @ref ViewType
	 */
	Message( const Contact *fromKC, const Contact *toKC, const QString &body,
		MessageDirection direction, MessageFormat format = PlainText, ViewType type = Undefined );


	/**
	 * Constructs a new message. See @ref setBody() to more information about the format
	 * @param fromKC The Kopete::Contact that the message is coming from
	 * @param toKC List of KopeteContacts the message is going to
	 * @param body Message body
	 * @param subject The subject of the message
	 * @param direction The direction of the message, Kopete::Message::Inbound, Kopete::Message::Outbound, Kopete::Message::Internal
	 * @param format Format of the message
	 * @param type Type of the message, see @ref ViewType
	 */
	Message( const Contact *fromKC, const ContactPtrList &toKC, const QString &body,
		const QString &subject, MessageDirection direction, MessageFormat format = PlainText, ViewType type = Undefined );

	/**
	 * Constructs a new message. See @ref setBody() to more information about the format
	 * @param timeStamp Timestamp for the message
	 * @param fromKC The Kopete::Contact that the message is coming from
	 * @param toKC List of KopeteContacts the message is going to
	 * @param body Message body
	 * @param direction The direction of the message, Kopete::Message::Inbound, Kopete::Message::Outbound, Kopete::Message::Internal
	 * @param format Format of the message
	 * @param type Type of the message, see @ref ViewType
	 */
	Message( const QDateTime &timeStamp, const Contact *fromKC, const ContactPtrList &toKC,
		const QString &body, MessageDirection direction, MessageFormat format = PlainText, ViewType type = Undefined );

	/**
	 * Constructs a new message. See @ref setBody() to more information about the format
	 * @param timeStamp Timestamp for the message
	 * @param fromKC The Kopete::Contact that the message is coming from
	 * @param toKC List of KopeteContacts the message is going to
	 * @param body Message body
	 * @param subject The subject of the message
	 * @param direction The direction of the message, Kopete::Message::Inbound, Kopete::Message::Outbound, Kopete::Message::Internal
	 * @param format Format of the message
	 * @param type Type of the message, see @ref ViewType
	 */
	Message( const QDateTime &timeStamp, const Contact *fromKC, const ContactPtrList &toKC,
		const QString &body, const QString &subject, MessageDirection direction,
		MessageFormat format = PlainText, ViewType type = Undefined );

	/**
	 * Copy constructor.
	 * Just adds a reference, doesn't actually copy.
	 */
	Message( const Message &other );

	/**
	 * Assignment operator
	 * Just like the copy constructor it just refs and doesn't copy.
	 */
	Message & operator=( const Message &other );

	/**
	 * Accessor method for the timestamp of the message
	 * @return The message's timestamp
	 */
	QDateTime timestamp() const;

	/**
	 * Accessor method for the Kopete::Contact that sent this message
	 * @return The Kopete::Contact who sent this message
	 */
	const Contact * from() const;

	/**
	 * Accessor method for the KopeteContacts that this message was sent to
	 * @return Pointer list of the KopeteContacts this message was sent to
	 */
	Kopete::ContactPtrList to() const;

	/**
	 * @return the @ref ViewType of this message
	 */
	ViewType type() const;

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
	 * @see importance
	 * @param importance The message importance to set
	 */
	void setImportance(MessageImportance importance);

	/**
	 * Sets the foreground color for the message
	 * @param color The color
	 */
	void setFg( const QColor &color );

	/**
	 * Sets the background color for the message
	 * @param color The color
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
	 * @param body The body
	 * @param format The format of the message, @see MessageFormat
	 */
	void setBody( const QString &body, MessageFormat format = PlainText );

	/**
	 * Get the message body back as plain text
	 * @return The message body as plain text
	 */
	QString plainBody() const;

	/**
	 * Get the message body as escaped (X)HTML format.
	 * That means every HTML special char (\>, \<, \&, ...) is escaped to the HTML entity (\&lt;, \&gt;, ...)
	 * and newlines (\\n) are converted to \<br /\>
	 * @return The message body as escaped text
	 */
	QString escapedBody() const;

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
	 * The @ref Kopete::MessageManager is only set if the message is already passed by the manager.
	 * We should trust this only in aboutToSend/aboutToReceive signals
	 */
	 MessageManager *manager() const ;

	 /**
	  * set the kopetemessagemanager for this message.
	  * should be only used by the manager itself
	  */
	 void setManager(MessageManager *);

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
	 * Enables the use of a foreground for a message
	 * @param enable A flag to indicate if the foreground should be enabled or disabled.
	 */
	void setFgOverride( bool enable );

	/**
	 * Enables the use of a RTF formatting for a message
	 * @param enable A flag to indicate if the RTF formatting should be enabled or disabled.
	 */
	void setRtfOverride( bool enable );

	/**
	* Unescapes a string, removing XML entity references
	*
	* @param xml The string you want to unescape
	*/
	static QString unescape( const QString &xml );

	/**
	 * @brief Transform a pleintext message to an html.
	 * it escape main entity like &gt; &lt; add some &lt;br /&gt; or &amp;nbsp
	 */
	static QString escape( const QString & );

	
	/**
	 * Helper function to decode a string. Whatever returned here is *nearly guarenteed* to
	 * be parseable by the XML engine.
	 *
	 * @param message The string you are trying to decode
	 * @param providedCodec A codec you want to try to decode with
	 * @param success Optional pointer to a bool you want updated on success. "Success"
	 *	is defined as a successfull decoding using either UTF8 or the codec you
	 *	provided. If a guess has to be taken, success will be false.
	 */
	static QString decodeString( const QCString &message,
		const QTextCodec *providedCodec = 0L, bool *success = 0L );

private:
	/**
	 * Helper for constructors
	 */
	void init( const QDateTime &timeStamp, const Contact *from, const ContactPtrList &to,
		const QString &body, const QString &subject, MessageDirection direction, MessageFormat f, ViewType type );

	/**
	 * Kopete::Message is implicitly shared.
	 * Detach the instance when modifying data.
	 */
	void detach();

	KopeteMessagePrivate *d;

	static QString parseLinks( const QString &message, MessageFormat format );
};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

