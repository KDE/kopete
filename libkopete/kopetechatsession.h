/*
    kopetechatsession.h - Manages all chats

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Daniel Stone           <dstone@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2003      by Jason Keirstead        <jason@keirstead.org>
    Copyright (c) 2005      by Michaël Larouche      <larouche@kde.org>
    Copyright (c) 2009      by Fabian Rami          <fabian.rami@wowcompany.com>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETECHATSESSION_H
#define KOPETECHATSESSION_H

#include <QtCore/QObject>

#include <kxmlguiclient.h>

#include "kopete_export.h"

// FIXME: get rid of these includes
#include "kopetemessage.h"
#include "kopetemessagehandlerchain.h"

class KMMPrivate;

class KopeteView;

namespace Kopete
{

class Contact;
class Message;
class Protocol;
class OnlineStatus;
class Account;
class ChatSessionManager;
class PropertyContainer;
class MessageHandlerChain;
class TemporaryKMMCallbackAppendMessageHandler;

typedef QList<Contact*>   ContactPtrList;
typedef QList<Message> MessageList;


/**
 * @author Duncan Mac-Vicar Prett <duncan@kde.org>
 * @author Daniel Stone           <dstone@kde.org>
 * @author Martijn Klingens       <klingens@kde.org>
 * @author Olivier Goffart        <ogoffart@kde.org>
 * @author Jason Keirstead        <jason@keirstead.org>
 *
 * The Kopete::ChatSession manages a single chat.
 * It is an interface between the protocol, and the chatwindow.
 * The protocol can connect to @ref messageSent() signals to send the message, and can
 * append received message with @ref messageReceived()
 *
 * The KMM inherits from KXMLGUIClient, this client is merged with the chatwindow's ui
 * so plugins can add childClients of this client to add their own actions in the
 * chatwindow.
 */
class KOPETE_EXPORT ChatSession : public QObject , public KXMLGUIClient
{
	// friend class so the object factory can access the protected constructor
	friend class ChatSessionManager;

	Q_OBJECT

public:
	/**
	 * Describes the form of this chat session
	 */ 
	enum Form { Small,/**< The chat is a small group or 1:1 chat */
		Chatroom/** Chat with many members and high traffic */ };
	/**
	 * Delete a chat manager instance
	 * You shouldn't delete the KMM yourself. it will be deleted when the chatwindow is closed
	 * see also @ref setCanBeDeleted() , @ref deref() 
	 */
	~ChatSession();

	/**
	 * @brief Get a list of all contacts in the session
	 */
	const ContactPtrList& members() const;

	/**
	 * @brief Get the local user in the session
	 * @return the local user in the session, same as account()->myself()
	 * @note Can be 0 if local user was already deleted during account destruction
	 */
	const Contact* myself() const;

	/**
	 * @brief Get the protocol being used.
	 * @return the protocol
	 */
	Protocol* protocol() const;

	/**
	 * @brief get the account
	 * @return the account
	 * @note Can be 0 if account was already deleted
	 */
	Account *account() const ;

	/**
	 * @brief The caption of the chat
	 *
	 * Used for named chats
	 */
	const QString displayName();

	/**
	* sets lastUrl for current ChatSession
	*/
	void setLastUrl( const QString &verylastUrl );

	/**
	* returns lastUrl for current ChatSession
	* can be empty
	*/
	const QString lastUrl();

	/**
	 * @brief change the displayname
	 *
	 * change the display name of the chat
	 */
	void setDisplayName( const QString &displayName );

	/**
	 * @brief set a specified KOS for specified contact in this KMM
	 *
	 * Set a special icon for a contact in this kmm only.
	 * by default, all contact have their own status
	 */
	void setContactOnlineStatus( const Contact *contact, const OnlineStatus &newStatus );

	/**
	 * @brief get the status of a contact.
	 *
	 * see @ref setContactOnlineStatus()
	 */
	const OnlineStatus contactOnlineStatus( const Contact *contact ) const;

	/**
	 * @brief the manager's view
	 *
	 * Return the view for the supplied Kopete::ChatSession.  If it already
	 * exists, it will be returned, otherwise, 0L will be returned or a new one
	 * if canCreate=true
	 * @param canCreate create a new one if it does not exist
	 * @param requestedPlugin Specifies the view plugin to use if we have to create one.
	 */
	// FIXME: canCreate should definitely be an enum and not a bool - Martijn
	KopeteView* view( bool canCreate = false, const QString &requestedPlugin = QString() );

	/**
	 * says if you may invite contact from the same account to this chat with @ref inviteContact
	 * @see setMayInvite
	 * @return true if it is possible to invite contact to this chat.
	 */
	bool mayInvite() const ;

	/**
	 * this method is called when a contact is dragged to the contact list.
	 * @p contactId is the id of the contact. the contact is supposed to be of the same account as
	 * the @ref account() but we can't be sure the Kopete::Contact is really on the contact list
	 *
	 * It is possible to drag contact only if @ref mayInvite return true
	 *
	 * the default implementaiton do nothing
	 */
	virtual void inviteContact(const QString &contactId);

	/**
	 * Returns the message handler chain for the message direction @p dir.
	 */
	MessageHandlerChain::Ptr chainForDirection( Message::MessageDirection dir );

	/**
	 * Get the form of this chatsession.  This is a hint to the UI so it can present the chat
	 * appropriately
	 */
	Form form() const;

	/**
	 * say if kopete show warning message, when you closing window of group chat
	 * @see setWarnGroupChat()
	 * @return true if kopete show warning message
	 */
	bool warnGroupChat() const;

	/**
	 * finds proper file with lasturls for current
	 * contact ChatSession->members().first()
	 * then sets lasturl for current ChatSession
	 * if there is no file, lasturl will be set to empty string
	 */
	QString initLastUrl( const Kopete::Contact* c );

	/**
	* returns file name where urls for this contact supposed to be
	*/
	static QString getUrlsFileName(const Kopete::Contact*);

	/**
	* prosesses every sent/appended message
	* looks for urls, if found: sets current lastUrl and save it to proper file
	*/
	void urlSearch( const Kopete::Message &msg );

	/**
	 * finds all urls in the current message (if there are many)
	 * sorts them as they are in the meassage QStringList[0] - is the earliest
	 */
	QStringList findUrls(const Kopete::Message &msg );

signals:
	/**
	 * @brief the KMM will be deleted
	 * Used by a Kopete::ChatSession to signal that it is closing.
	 */
	void closing( Kopete::ChatSession *kmm );

	/**
	 * a message will be soon shown in the chatwindow.
	 * See @ref Kopete::ChatSessionManager::aboutToDisplay() signal
	 */
	void messageAppended( Kopete::Message &msg, Kopete::ChatSession *kmm = 0L );

	/**
	 * a message will be soon received
	 * See @ref Kopete::ChatSessionManager::aboutToReceive() signal
	 */
	void messageReceived( Kopete::Message &msg, Kopete::ChatSession *kmm = 0L );

	/**
	 * @brief a message is going to be sent
	 *
	 * The message is going to be sent.
	 * protocols can connect to this signal to send the message ro the network.
	 * the protocol have also to call @ref appendMessage() and @ref messageSucceeded()
	 * See also @ref Kopete::ChatSessionManager::aboutToSend() signal
	 */
	void messageSent( Kopete::Message &msg, Kopete::ChatSession *kmm = 0L );

	/**
	 * The last message has finaly successfully been sent
	 */
	void messageSuccess();

	/**
	 * @brief a new contact is now in the chat
	 */
	// FIXME: What's 'suppress'? Shouldn't this be an enum? - Martijn
	void contactAdded( const Kopete::Contact *contact, bool suppress );

	/**
	 * @brief a contact is no longer in this chat
	 */
	void contactRemoved( const Kopete::Contact *contact, const QString &reason, Qt::TextFormat format = Qt::PlainText, bool suppressNotification = false );

	/**
	 * @brief a contact in this chat has changed his status
	 */
	void onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & );

	/**
	 * @brief a contact in this chat has changed his status message
	 */
	void statusMessageChanged( Kopete::Contact* );

	/**
	 * @brief a contact in this chat has changed his display name (previously nickname)
	 */
	void nickNameChanged( Kopete::Contact *, const QString & );

	/**
	 * @brief The name of the chat is changed
	 */
	void displayNameChanged();

	/**
	 * @brief emitting a typing notification
	 *
	 * The user is typing a message, or just stopped typing
	 * the protocol should connect to this signal to signal to others
	 * that the user is typing if the protocol supports this
	 * @param isTyping say if the user is typing or not
	 */
	void myselfTyping( bool isTyping );

	/**
	 * Signals that a remote user is typing a message.
	 * the chatwindow connects to this signal to update the statusbar
	 */
	void remoteTyping( const Kopete::Contact *contact, bool isTyping );

	/**
	 * Signals that a an event has to be displayed in the statusbar.
	 * The chatwindow connects to this signal to update the statusbar.
	 */
	void eventNotification( const QString& notificationText);

	/**
	 * Signals that view for that chat session was activated
	 */
	void viewActivated( KopeteView* view );

	/**
	 * Signals that a message has changed its state.
	 * The chat window connects to this signal to update the message in chat view.
	 */
	void messageStateChanged( uint messageId, Kopete::Message::MessageState state );

	/**
	 * @brief A contact within the chat session changed his photo.
	 * Used to update the contacts photo in chat window.
	 */
	void photoChanged();

	/**
	 * does nothing
	 */
	void toggleGraphicOverride(bool enable);

public slots:
	/**
	 * @brief Got a typing notification from a user
	 */
	void receivedTypingMsg( const Kopete::Contact *contact , bool isTyping = true );

	/**
	 * Got a typing notification from a user. This is a convenience version
	 * of the above method that takes a QString contactId instead of a full
	 * Kopete::Contact
	 */
	void receivedTypingMsg( const QString &contactId, bool isTyping = true );

	/**
	 * @brief Got an event notification from a user.
	 * It will emit the signal eventNotification(). Use this slot in your protocols
	 * and plugins to change chatwindow statusBar text.
	 */
	void receivedEventNotification(  const QString& notificationText );

	/**
	 * @brief Change state of message.
	 * It will emit the signal messageStateChanged(). Use this slot in your protocols
	 * and plugins to change message state.
	 */
	void receivedMessageState( uint messageId, Kopete::Message::MessageState state );
	
	/**
	 * Show a message to the chatwindow, or append it to the queue.
	 * This is the function protocols HAVE TO call for both incoming and outgoing messages
	 * if the message must be showed in the chatwindow
	 */
	void appendMessage( Kopete::Message &msg );

	/**
	 * Add a contact to the session
	 * @param c is the contact
	 * @param suppress mean the there will be no automatic notifications in the chatwindow.
	 *  (note that i don't like the param suppress at all. it is used in irc to show a different notification (with an info text)
	 *   a QStringinfo would be more interesting, but it is also used to don't show the notification when entering in a channel)
	 */
	void addContact( const Kopete::Contact *c, bool suppress = false );

	/**
	 * Add a contact to the session with a pre-set initial status
	 * @param c is the contact
	 * @param initialStatus The initial contactOnlineStatus of the contact
	 * @param suppress mean the there will be no automatic notifications in the chatwindow.
	 *  (note that i don't like the param suppress at all. it is used in irc to show a different notification (with an info text)
	 *   a QStringinfo would be more interesting, but it is also used to don't show the notification when entering in a channel)
	 * @see contactOnlineStatus
	 */
	void addContact( const Kopete::Contact *c, const Kopete::OnlineStatus &initialStatus, bool suppress = false );

	/**
	 * Remove a contact from the session
	 * @param contact is the contact
	 * @param reason is the optional raison message showed in the chatwindow
	 * @param format The format of the message
	 * @param suppressNotification prevents a notification of the removal in the chat view.  See note in @ref addContact
	 */
	void removeContact( const Kopete::Contact *contact, const QString& reason = QString(), Qt::TextFormat format = Qt::PlainText, bool suppressNotification = false );

	/**
	 * Set if the KMM will be deleted when the chatwindow is deleted. It is useful if you want
	 * to keep the KMM alive even if the chatwindow is closed.
	 * Warning: if you set it to false, please keep in mind that you have to reset it to true
	 *  later to delete it. In many case, you should never delete yourself the KMM, just call this
	 *  this method.
	 * default is true.
	 * If there are no chatwindow when setting it to true, the kmm will be deleted.
	 *
	 * @deprecated  use ref and deref
	 */
	void setCanBeDeleted ( bool canBeDeleted );

	/**
	 * reference count the chat session.
	 * the chat session may be deleted only if the count reach 0
	 * if you ref, don't forget to deref
	 * @see deref()
	 */
	void ref();
	/**
	 * dereference count the chat session
	 * if the reference counter reach 0 and there is no chat window open, the chat session will be deleted.
	 */
	void deref();
		

	/**
	 * Send a message to the user
	 */
	void sendMessage( Kopete::Message &message );

	/**
	 * Tell the KMM that the user is typing
	 * This method should be called only by a chatwindow. It emits @ref myselfTyping signal
	 */
	void typing( bool t );

	/**
	 * Protocols have to call this method when the last message sent has been correctly sent
	 * This will emit @ref messageSuccess signal. and allow the email window to get closed
	 */
	void messageSucceeded();

	/**
	 * Protocols have to call this method if they want to emit a notification when a nudge/buzz is received.
	 */
	void emitNudgeNotification();
	
	/**
	 * Raise the chat window and give him the focus
	 * It's used when the user wanted to activated  (by clicking on the "view" button of a popup)
	 */
	void raiseView();

private slots:
	void clearChains();
	void slotUpdateDisplayName();
	void slotViewDestroyed();
	void slotOnlineStatusChanged( Kopete::Contact *c, const Kopete::OnlineStatus &status, const Kopete::OnlineStatus &oldStatus );
	void slotContactDestroyed( Kopete::Contact *contact );
	void slotMyselfDestroyed( Kopete::Contact *contact );
	void slotDisplayNameChanged( const QString &oldName, const QString &newName );

protected:
	/**
	 * Create a message manager. This constructor is private, because the
	 * static factory method createSession() creates the object. You may
	 * not create instances yourself directly!
	 */
	ChatSession( const Contact *user, ContactPtrList others, Protocol *protocol, Form form = Small );

	/**
	 * Set whether or not contact from this account may be invited in this chat.
	 * By default, it is set to false
	 * @see inviteContact()
	 * @see mayInvite()
	 */
	void setMayInvite(bool);

	/**
	 * set if kopete show warning message, when you closing window of group chat
	 * By default, it is set to true
	 * @see warnGroupChat()
	 */
	void setWarnGroupChat(bool);

private:
	KMMPrivate *d;

	// FIXME: remove
	friend class TemporaryKMMCallbackAppendMessageHandler;
};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

