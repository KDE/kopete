/*
    kopetemessage.h  -  Base class for Kopete messages

    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart       <ogoffart@kde.org>
	Copyright (c) 2006-2007 by Charles Connell       <charles@connells.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

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

#include "kopetecontact.h"

#include <qstring.h>
#include <qdom.h>
#include <qcolor.h>
#include <qfont.h>
#include <qdatetime.h>
#include <QByteArray>
#include <QSharedDataPointer>
#include <QList>
#include <QByteArray>

#include "kopete_export.h"


class QDateTime;
class QTextDocument;

namespace Kopete {


class ChatSession;
class Contact;


/**
 * @author Martijn Klingens <klingens@kde.org>
 * @author Olivier Goffart <ogoffart@kde.org>
 * @author Charles Connell <charles@connells.org>
 *
 * Message represents any kind of messages shown on a chat view.
 *
 * The message may be a simple plaintext string, or a Richtext HTML like message,
 * this is indicated by the @ref format() flag.
 * PlainText message can however have a color, or specific fonts with the flag
 * @ref bg(), @ref fg(), @ref font()
 * It is recommended to use these flags, even for RichText messages, so the user can disable
 * custom colors in the chat window style.
 */
class KOPETE_EXPORT Message
{
public:
	/**
	 * Direction of a message.
	 * - Inbound: Message is from the chat partner
	 * - Outbound: Message sent by the user.
	 * - Internal: Messages which are not sent via the network. This is just a notification a plugin can show in a chat view
	 * - Action: For the /me command , like on irc
	 */
	enum MessageDirection { Inbound = 0, Outbound = 1, Internal= 2 };

	/**
	 * Specifies the type of the message.
	 * Currently supported types are:
	 * - Normal: a message
	 * - Action: an IRC-style DESCRIBE action.
	 */
	enum MessageType { TypeNormal, TypeAction };

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
	 * @param fromKC The Contact that the message is coming from
	 * @param toKC List of Contacts the message is going to
	 * @param body Message body
	 * @param direction The direction of the message, Message::Inbound, Message::Outbound, Message::Internal
	 * @param format Format of the message
	 * @param requestedPlugin Requested view plugin for the message
	 * @param type Type of the message, see @ref MessageType
	 */
	Message( const Contact *fromKC, const QList<Contact*> &toKC, const QString &body,
		 MessageDirection direction, Qt::TextFormat format = Qt::PlainText,
		 const QString &requestedPlugin = QString(), MessageType type = TypeNormal );

	/**
	 * Constructs a new message. See @ref setBody() to more information about the format
	 * @param fromKC The Contact that the message is coming from
	 * @param toKC List of Contacts the message is going to
	 * @param body Message body
	 * @param direction The direction of the message, Message::Inbound, Message::Outbound, Message::Internal
	 * @param format Format of the message
	 * @param requestedPlugin Requested view plugin for the message
	 * @param type Type of the message, see @ref MessageType
	 */
	Message( const Contact *fromKC, const Contact *toKC, const QString &body,
		 MessageDirection direction, Qt::TextFormat format = Qt::PlainText,
		 const QString &requestedPlugin = QString(), MessageType type = TypeNormal );

	/**
	 * Constructs a new message. See @ref setBody() to more information about the format
	 * @param fromKC The Contact that the message is coming from
	 * @param toKC List of Contacts the message is going to
	 * @param body Message body
	 * @param subject The subject of the message
	 * @param direction The direction of the message, Message::Inbound, Message::Outbound, Message::Internal
	 * @param format Format of the message
	 * @param requestedPlugin Requested view plugin for the message
	 * @param type Type of the message, see @ref MessageType
	 */
	Message( const Contact *fromKC, const QList<Contact*> &toKC, const QString &body,
		 const QString &subject, MessageDirection direction, Qt::TextFormat format = Qt::PlainText,
		 const QString &requestedPlugin = QString(), MessageType type = TypeNormal );

	/**
	 * Constructs a new message. See @ref setBody() to more information about the format
	 * @param timeStamp Timestamp for the message
	 * @param fromKC The Contact that the message is coming from
	 * @param toKC List of Contacts the message is going to
	 * @param body Message body
	 * @param direction The direction of the message, Message::Inbound, Message::Outbound, Message::Internal
	 * @param format Format of the message
	 * @param requestedPlugin Requested view plugin for the message
	 * @param type Type of the message, see @ref MessageType
	 */
	Message( const QDateTime &timeStamp, const Contact *fromKC, const QList<Contact*> &toKC,
		 const QString &body, MessageDirection direction, Qt::TextFormat format = Qt::PlainText,
		 const QString &requestedPlugin = QString(), MessageType type = TypeNormal );

	/**
	 * Constructs a new message. See @ref setBody() to more information about the format
	 * @param timeStamp Timestamp for the message
	 * @param fromKC The Contact that the message is coming from
	 * @param toKC List of Contacts the message is going to
	 * @param body Message body
	 * @param subject The subject of the message
	 * @param direction The direction of the message, Message::Inbound, Message::Outbound, Message::Internal
	 * @param format Format of the message
	 * @param requestedPlugin Requested view plugin for the message
	 * @param type Type of the message, see @ref MessageType
	 */
	Message( const QDateTime &timeStamp, const Contact *fromKC, const QList<Contact*> &toKC,
		const QString &body, const QString &subject, MessageDirection direction,
		Qt::TextFormat format = Qt::PlainText, const QString &requestedPlugin = QString(),
		MessageType type = TypeNormal );

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
	 * @brief Accessor method for the timestamp of the message
	 * @return The message's timestamp
	 */
	QDateTime timestamp() const;

	/**
	 * @brief Accessor method for the Contact that sent this message
	 * @return The Contact who sent this message
	 */
	const Contact * from() const;

	/**
	 * @brief Accessor method for the Contacts that this message was sent to
	 * @return Pointer list of the Contacts this message was sent to
	 */
	QList<Contact*> to() const;

	/**
	 * @brief Accessor method for the message type
	 * @return The type of the message
	 * @see MessageType
	 */
	MessageType type() const;

	/**
	 * @brief Accessor method for the preferred plugin
	 * If null, Kopete will use the user's preferred plugin.
	 * @return The preferred plugin
	 */
	QString requestedPlugin() const;

	/**
	 * @brief Accessor method for the foreground color
	 * @return The message's foreground color
	 */
	QColor fg() const;

	/**
	 * @brief Accessor method for the background color of the message
	 * @return The message's background color
	 */
	QColor bg() const;

	/**
	 * @brief Accesssor method for the direction of the text based on what language it is in
	 * @return The message text's direction
	 */
	bool isRightToLeft() const;

	/**
	 * @brief Accessor method for the font of the message
	 * @return The message's font
	 */
	QFont font() const;

	/**
	 * @brief Accessor method for the subject of the message
	 * @return The message subject
	 */
	QString subject() const;

	/**
	 * @brief Accessor method for the body of the message
	 * This is used internaly, to modify it make a copy of it with QTextDocument::clone()
	 * @return The message body
	 */
	const QTextDocument *body() const;

	/**
	 * @brief Accessor method for the format of the message
	 * @return The message format
	 */
	Qt::TextFormat format() const;

	/**
	 * @brief Accessor method for the direction of the message
	 * @return The message direction
	 */
	MessageDirection direction() const;

	/**
	 * @brief Accessor method for the importance
	 * @see setImportance
	 * @return The message importance (low/normal/highlight)
	 */
	MessageImportance importance() const;

	/**
	 * @brief Set the importance.
	 * @see importance and @see MessageImportance
	 * @param importance The message importance to set
	 */
	void setImportance(MessageImportance importance);

	/**
	 * @brief Sets the foreground color for the message
	 * @see fg
	 * @param color The color
	 */
	void setFg( const QColor &color );

	/**
	 * @brief Sets the background color for the message
	 * @see bg
	 * @param color The color
	 */
	void setBg( const QColor &color );

	/**
	 * @brief Sets the font for the message
	 * @see font
	 * @param font The font
	 */
	void setFont( const QFont &font );

	/**
	 * @brief Sets the body of the message
	 *
	 * @param body The body, intpreted as plain text
	 */
	void setPlainBody( const QString &body);

	/**
	 * @brief Sets the body of the message
	 *
	 * @param body The body, interpreted as HTML
	 */
	void setHtmlBody( const QString &body);

	/**
	 * @brief Sets the body of the message
	 * The format is changed to RichText automatically
	 * @param body The body
	 */
	void setBody( const QTextDocument *body);

	/**
	 * @brief Get the message body back as plain text
	 * @return The message body as plain text
	 */
	QString plainBody() const;

	/**
	 * @brief Get the message body as escaped (X)HTML format.
	 * That means every HTML special char (\>, \<, \&, ...) is escaped to the HTML entity (\&lt;, \&gt;, ...)
	 * and newlines (\\n) are converted to \<br /\>
	 * Just because you set an HTML body doesn't mean you'll get the same string back here, but it will
	 * be equivalent in meaning
	 * @return The message body as escaped text
	 */
	QString escapedBody() const;

	/**
	 * @brief Get the message body as parsed HTML with Emoticons, and URL parsed
	 * This should be ready to be shown in the chatwindow.
	 * @return The HTML and Emoticon parsed message body
	 */
	QString parsedBody() const;

	/**
	 * Get the related message manager.
	 * If it is not set, returns 0L.
	 *
	 * The @ref ChatSession is only set if the message is already passed by the manager.
	 * We should trust this only in aboutToSend/aboutToReceive signals
	 */
	 ChatSession *manager() const ;

	 /**
	  * @brief Set the messagemanager for this message.
	  * Should be only used by the manager itself. @see manager
	  * @param manager The chat session
	  */
	 void setManager(ChatSession * manager);

	/**
	 * @brief Enables the use of a background for a message
	 * @see bgOverride
	 * @param enable A flag to indicate if the background should be enabled or disabled.
	 */
	void setBgOverride( bool enable );

	/**
	 * @brief Enables the use of a foreground for a message
	 * @see fgOverride
	 * @param enable A flag to indicate if the foreground should be enabled or disabled.
	 */
	void setFgOverride( bool enable );

	/**
	 * @brief Enables the use of a RTF formatting for a message
	 * @see rtfOverride
	 * @param enable A flag to indicate if the RTF formatting should be enabled or disabled.
	 */
	void setRtfOverride( bool enable );

	/**
	 * @brief Return HTML style attribute for this message.
	 * @return A string formatted like this: "style=attr"
	 */
	QString getHtmlStyleAttribute() const;
	
	/**
	 * @return The list of classes
	 * Class are used to give different notification on a message. They are also used in the chatwindow as an HTML class 
	 */
	QStringList classes() const;
	
	/**
	 * @brief Add a class
	 * @see classes
	 * @param class class to add
	 */
	void addClass(const QString& classe);
	
	/**
	 * @brief Set the classes
	 * @see classes
	 * @param classes The new classes
	 */
	void setClasses(const QStringList &classes);

public:  /* static helpers */

	/**
	* Unescapes a string, removing XML entity references and returns a plain text.
	*
	* Note that this method is *VERY* expensive when called on rich text bodies,
	* use with care!
	*
	*/
	static QString unescape( const QString &xml );

	/**
	 * @brief Transform a plaintext message into HTML.
	 * it escape main entity like &gt; &lt; add some &lt;br /&gt; or &amp;nbsp;
	 */
	static QString escape( const QString & );


	/**
	 * Helper function to decode a string. Whatever returned here is *nearly guaranteed* to
	 * be parseable by the XML engine.
	 *
	 * @param message The string you are trying to decode
	 * @param providedCodec A codec you want to try to decode with
	 * @param success Optional pointer to a bool you want updated on success. "Success"
	 *	is defined as a successful decoding using either UTF8 or the codec you
	 *	provided. If a guess has to be taken, success will be false.
	 * @return The decoded string

	 */
	static QString decodeString( const QByteArray &message,
 		const QTextCodec *providedCodec = 0L, bool *success = 0L );

private:
	class Private;
	QSharedDataPointer<Private> d;

	/**
	 * Called internally by @ref setBody() and the constructor
	 * Basically @ref setBody() without detach
	 * @internal
	 */
	void doSetBody( const QString &body, Qt::TextFormat format = Qt::PlainText );

	/**
	 * Called internally by @ref setBody() and the constructor
	 * Basically @ref setBody() without detach
	 * @internal
	 */
	void doSetBody (const QTextDocument *body, Qt::TextFormat format = Qt::PlainText);

	/**
	 * Called internally in rich text handling
	 * @internal
	 */
	static QString parseLinks( const QString &message, Qt::TextFormat format );
};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

