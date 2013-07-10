/*
    kopetemessage.h  -  Base class for Kopete messages

    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart       <ogoffart@kde.org>
    Copyright (c) 2006-2007 by Charles Connell       <charles@connells.org>
    Copyright (c) 2007      by Michaël Larouche      <larouche@kde.org>
    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>

    Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEMESSAGE_H
#define KOPETEMESSAGE_H

#include <QtCore/QSharedData>
#include <QtCore/QList>
#include <QtCore/Qt>

#include "kopete_export.h"

class QByteArray;
class QColor;
class QDateTime;
class QFont;
class QTextCodec;
class QTextDocument;
class QStringList;
class QPixmap;

namespace Kopete {


class ChatSession;
class Contact;


/**
 * @brief Representation of a message in Kopete
 *
 * @author Martijn Klingens <klingens@kde.org>
 * @author Olivier Goffart <ogoffart@kde.org>
 * @author Charles Connell <charles@connells.org>
 * @author Michaël Larouche <larouche@kde.org>
 *
 * Message represents any kind of messages shown on a chat view.
 * The message may contain a simple plain text string, or a rich text HTML
 * message. Also, Message can use a QTextDocument to structure the message.
 *
 * Message in plain text can however have a color or a specific font. You can
 * set color with setForegroundColor() and setBackgroundColor() and change the font
 * with setFont()
 *
 * Message have a direction from where the message come from. By default, the direction
 * is Internal but it is strongly advised to set the direction explicitly.
 *
 * @section plainMessage Creating a plain text message
 * @code
Kopete::Message newMessage(sourceContact, destionationContact);
newMessage.setPlainBody( QString("A plain text message") );
newMessage.setDirection( Kopete::Message::Inbound );
 * @endcode
 *
 * @section richTextMessage Creating a complete rich text message
 * @code
Kopete::Message richTextMessage(sourceContact, destinationContactList);
richTextMessage.setTimestamp( QDateTime::currentDateTime() );
richTextMessage.setDirection( Kopete::Message::Outbound );
richTextMessage.setSubject( QString("Kopete API documentation thread") );
richTextMessage.setHtmlBody( QString("<b>A bold text</b>") );
 * @endcode
 */
class KOPETE_EXPORT Message
{
public:
	/**
	 * Direction of a message.
	 */
	enum MessageDirection
	{
		Inbound = 0, ///< Message is from the chat partner
		Outbound = 1, ///< Message sent by the user
		Internal= 2 ///< (Default) Message which are not sent via the network. This is just a notification a plugin can show in a chat view
	};

	/**
	 * Specifies the type of the message.
	 */
	enum MessageType
	{
		TypeNormal, ///< A typical message
		TypeAction, ///< An IRC-style action.
		TypeFileTransferRequest, ///< A incoming file transfer request message
		TypeVoiceClipRequest ///< A incoming voice clip message
	};

	/**
	 * Specifies the type of notification that will be sent with this message
	 */
	enum MessageImportance
	{
		Low = 0, ///< almost no notifications. automatically used in groupChat
		Normal = 1, ///< Default notification, for normal message
		Highlight = 2 ///< Highlight notification, for most important messages, which require particular attentions.
	};

	enum MessageState
	{
		StateUnknown = 0, ///< state of message isn't known (e.g. protocol doesn't support message acknowledgment)
		StateSending = 1, ///< message was sent but not yet delivered.
		StateSent = 2, ///< message was delivered
		StateError = 3 ///< message has not been delivered
	};

	/**
	 * Constructs a new empty message
	 */
	explicit Message();

	/**
	 * Deref and clean private object if refcount == 0
	 */
	~Message();

	/**
	 * @brief Constructs a new message with a from and to contact
	 *
	 * This constructor is a convience of the constructor who
	 * take a list of contacts for destination
	 * @param fromKC Contact who send the message
	 * @param toKC Contact which the message is destined.
	 */
	explicit Message( const Contact *fromKC, const Contact *toKC );
	/**
	 * @brief Constructs a new message with many contacts as the destination.
	 * @param fromKC Contact who send the message
	 * @param contacts List of Contact to send the message
	 */
	explicit Message( const Contact *fromKC, const QList<Contact*> &contacts);

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
	 * @brief Get unique message id.
	 * @return message id
	 */
	uint id() const;

	/**
	 * @brief Get next unique message id.
	 * @return next id
	 */
	static uint nextId();

	/**
	 * @brief Accessor method for the timestamp of the message
	 * @return The message's timestamp
	 */
	QDateTime timestamp() const;

	/**
	 * @brief Set the message timestamp
	 * @param timestamp timestamp as QDateTime. By default the current date and time.
	 */
	void setTimestamp(const QDateTime &timestamp);

	/**
	 * @brief Accessor method for the "delayed" attribute of the message
	 * @return true if the message was delayed (for example because it has
	 * been stored on a server while the intended recipient was offline or
	 * because the message is contained in the history of a group chat room).
	 */
	bool delayed() const;

	/**
	 * @brief Set the "delayed" attribute of the message
	 * @param delay whether the message was delayed, see delayed()
	 */
	void setDelayed(bool delay);

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
	 * @brief Set message type
	 * @param type The type of the message
	 * @see MessageType
	 */
	void setType(MessageType type);

	/**
	 * @brief Accessor method for the preferred plugin
	 * If null, Kopete will use the user's preferred plugin.
	 * @return The preferred plugin
	 */
	QString requestedPlugin() const;

	/**
	 * @brief Set a view plugin which will display the message
	 *
	 * This is used mostly by Jabber plugin to select between
	 * the email window or the chat window depending of the
	 * type of message.
	 * @param requesedPlugin View plugin name
	 */
	void setRequestedPlugin(const QString &requestedPlugin);

	/**
	 * @brief Accessor method for the foreground color
	 * @return The message's foreground color
	 */
	QColor foregroundColor() const;

	/**
	 * @brief Accessor method for the background color of the message
	 * @return The message's background color
	 */
	QColor backgroundColor() const;

	/**
	 * @brief Accesssor method for the direction of the text based on what language it is in
	 * @return The message text's direction
	 */
	bool isRightToLeft() const;

	/**
	* returns QStringList with regexp patterns
	* will be used to look for links in the message
	*/
	const QStringList regexpPatterns();

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
	 * @brief Set message subject
	 * @param subject Message's subject
	 */
	void setSubject(const QString &subject);

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
	 * @brief Set the message direction
	 * @param direction The message direction
	 * @see MessageDirection
	 */
	void setDirection(MessageDirection direction);

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
	 * @brief Accessor method for the state
	 * @return The message state (unknown/sending/sent/error)
	 */
	MessageState state() const;

	/**
	 * @brief Set the state of message.
	 * @see MessageState
	 * @param state The message state to set
	 */
	void setState(MessageState state);

	/**
	 * @brief Sets the foreground color for the message
	 * @see foregroundColor
	 * @param color The color
	 */
	void setForegroundColor( const QColor &color );

	/**
	 * @brief Sets the background color for the message
	 * @see backgroundColor
	 * @param color The color
	 */
	void setBackgroundColor( const QColor &color );

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
	 * @brief Sets the body of the message, which is used even if formatting is overridden
	 *
	 * @param body The body, interpreted as HTML
	 */
	void setForcedHtmlBody( const QString &body);

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
	 * @brief Does nothing
	 */
	void KDE_DEPRECATED setBackgroundOverride( bool enable );

	/**
	 * @brief Does nothing
	 */
	void KDE_DEPRECATED setForegroundOverride( bool enable );

	/**
	 * @brief Does nothing
	 */
	void KDE_DEPRECATED setRichTextOverride( bool enable );
	
	/**
	 * @brief Ignores peer's formatting
	 */
	void setFormattingOverride( bool enable );

	/**
	 * @brief Return HTML style attribute for this message.
	 * @return A string formatted like this: "style=attr"
	 */
	QString getHtmlStyleAttribute() const;

	/**
	 * @brief Set the state of incoming file transfer
	 * @param disabled flag to indicate if the file transfer request should be enabled or disabled.
	 */
	void setFileTransferDisabled( bool disabled );

	/**
	 * @brief Accessor method for the file transfer state
	 * @return if file transfer request should be enable or disable
	 */
	bool fileTransferDisabled() const;

	/**
	 * @brief Set file name of incoming file transfer
	 * @param fileName file name
	 */
	void setFileName( const QString &fileName );

	/**
	 * @brief Accessor method for the file name of incoming file transfer
	 * @return file name of incoming file transfer
	 */
	QString fileName() const;

	/**
	 * @brief Set file transfer size
	 * @param size file transfer size
	 */
	void setFileSize( unsigned long size );

	/**
	 * @brief Accessor method for the file transfer size
	 * @return file transfer size
	 */
	unsigned long fileSize() const;

	/**
	 * @brief Set file preview icon for file transfer
	 * @param preview file preview icon
	 */
	void setFilePreview( const QPixmap &preview );

	/**
	 * @brief Accessor method for the file preview icon
	 * @return file preview icon
	 */
	QPixmap filePreview() const;

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

#if 0
	//candidate for removal!
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
#endif

private:
	class Private;
	QSharedDataPointer<Private> d;

	/**
	 * Called internally by @ref setBody() and the constructor
	 * Basically @ref setBody() without detach
	 * @internal
	 */
	void doSetBody( QString body, Qt::TextFormat format = Qt::PlainText );

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

