/*
    kopetemessagemanager.h - Manages all chats

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Daniel Stone           <dstone@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Copyright (c) 2003      by Jason Keirstead        <jason@keirstead.org>

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

#ifndef __KOPETEMESSAGEMANAGER_H__
#define __KOPETEMESSAGEMANAGER_H__

#include <qobject.h>
#include <qptrlist.h>
#include <qvaluelist.h>

#include <kxmlguiclient.h>

// FIXME: get rid of this include in header
#include "kopetemessage.h"

class KopeteContact;
class KopeteMessage;
class KopeteProtocol;
class KopeteView;
class KopeteOnlineStatus;
class KopeteAccount;

typedef QPtrList<KopeteContact>   KopeteContactPtrList;
typedef QValueList<KopeteMessage> KopeteMessageList;

class KMMPrivate;

/**
 * @author Duncan Mac-Vicar Prett <duncan@kde.org>
 * @author Daniel Stone           <dstone@kde.org>
 * @author Martijn Klingens       <klingens@kde.org>
 * @author Olivier Goffart        <ogoffart@tiscalinet.be>
 * @author Jason Keirstead        <jason@keirstead.org>
 *
 * The KopeteMessageManager (also called KMM for simplicity) manages a single chat.
 * It is an interface between the protocol, and the chatwindow.
 * The protocol can connect to @ref messageSent() signals to send the message, and can
 * append received message with @ref messageReceived()
 *
 * The KMM inherits from KXMLGUIClient, this client is merged with the chatwindow's ui
 * so plugins can add childClients of this client to add their own actions in the
 * chatwindow.
 */
class KopeteMessageManager : public QObject , public KXMLGUIClient
{
	// friend class so the object factory can access the protected constructor
	friend class KopeteMessageManagerFactory;

	Q_OBJECT

public:
	/**
	 * Delete a chat manager instance
	 * You shouldn't delete the KMM yourself. it will be deleted when the chatwindow is closed
	 * see also @ref setCanBeDeleted()
	 */
	~KopeteMessageManager();

	/**
	 * @brief Get a list of all contacts in the session
	 */
	const KopeteContactPtrList& members() const;

	/**
	 * @brief Get the local user in the session
	 * @return the local user in the session, same as account()->myself()
	 */
	const KopeteContact* user() const;

	/**
	 * @brief Get the protocol being used.
	 * @return the protocol
	 */
	KopeteProtocol* protocol() const;

	/**
	 * @brief get the account
	 * @return the account
	 */
	KopeteAccount *account() const ;

	/**
	 * @brief the KMM unique id
	 * @return a unique identifier associated with this manager
	 */
	int mmId() const;

	/**
	 * @brief The caption of the chat
	 *
	 * Used for named chats
	 */
	const QString displayName();

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
	void setContactOnlineStatus( const KopeteContact *contact, const KopeteOnlineStatus &newStatus );

	/**
	 * @brief get the status of a contact.
	 *
	 * see @ref setContactOnlineStatus()
	 */
	const KopeteOnlineStatus contactOnlineStatus( const KopeteContact *contact ) const;

	/**
	 * @brief the manager's view
	 *
	 * Return the view for the supplied KopeteMessageManager.  If it already
	 * exists, it will be returned, otherwise, 0L will be returned or a new one
	 * if canCreate=true
	 * @param canCreate create a new one if it does not exist
	 * @param type Specifies the type of view if we have to create one.
	 */
	// FIXME: canCreate should definitely be an enum and not a bool - Martijn
	KopeteView* view( bool canCreate = false, KopeteMessage::MessageType type = KopeteMessage::Undefined );
	
	/**
	 * says if you may invite contact from the same account to this chat with @ref inviteContact
	 * @see setMayInvite
	 * @return true if it is possible to invite contact to this chat.
	 */
	bool mayInvite() const ;
	
	/**
	 * this method is called when a contact is dragged to the contactlist.
	 * @p contactId is the id of the contact. the contact is supposed to be of the same account as
	 * the @ref account() but we can't be sure the KopeteContact is realy on the contactlist 
	 *
	 * It is possible to drag contact only if @ref mayInvite return true
	 *
	 * the default implementaiton do nothing
	 */
	virtual void inviteContact(const QString &contactId);
	

signals:
	/**
	 * @brief the KMM will be deleted
	 * Used by a KopeteMessageManager to signal that it is closing.
	 */
	void closing( KopeteMessageManager *kmm );

	/**
	 * a message will be soon shown in the chatwindow.
	 * See @ref KopeteMessageManagerFactory::aboutToShow() signal
	 */
	void messageAppended( KopeteMessage &msg, KopeteMessageManager *kmm = 0L );

	/**
	 * a message will be soon received
	 * See @ref KopeteMessageManagerFactory::aboutToReceive() signal
	 */
	void messageReceived( KopeteMessage &msg, KopeteMessageManager *kmm = 0L );

	/**
	 * @brief a message is going to be sent
	 *
	 * The message is going to be sent.
	 * protocols can connect to this signal to send the message ro the network.
	 * the protocol have also to call @ref appendMessage() and @ref messageSucceeded()
	 * See also @ref KopeteMessageManagerFactory::aboutToSend() signal
	 */
	void messageSent( KopeteMessage &msg, KopeteMessageManager *kmm = 0L );

	/**
	 * The last message has finaly successfully been sent
	 */
	void messageSuccess();

	/**
	 * @brief a new contact is now in the chat
	 */
	// FIXME: What's 'suppress'? Shouldn't this be an enum? - Martijn
	void contactAdded( const KopeteContact *contact, bool suppress );

	/**
	 * @brief a contact is no longer in this chat
	 */
	void contactRemoved( const KopeteContact *contact, const QString &reason, KopeteMessage::MessageFormat format = KopeteMessage::PlainText );

	/**
	 * @brief a contact in this chat has changed his status
	 */
	void onlineStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus & );

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
	void typingMsg( bool isTyping );

	/**
	 * Signals that a remote user is typing a message.
	 * the chatwindow connects to this signal to update the statusbar
	 */
	void remoteTyping( const KopeteContact *contact, bool isTyping );

public slots:
	/**
	 * @brief Got a typing notification from a user
	 */
	void receivedTypingMsg( const KopeteContact *contact , bool isTyping = true );

	/**
	 * Got a typing notification from a user. This is a convenience version
	 * of the above method that takes a QString contactId instead of a full
	 * KopeteContact
	 */
	void receivedTypingMsg( const QString &contactId, bool isTyping = true );

	/**
	 * Show a message to the chatwindow, or append it to the queue.
	 * This is the function protocols HAVE TO call for both incoming and outgoing messages
	 * if the message must be showed in the chatwindow
	 */
	void appendMessage( KopeteMessage &msg );

	/**
	 * Add a contact to the session
	 * @param c is the contact
	 * @param suppress mean the there will be no automatic notifications in the chatwindow.
	 *  (note that i don't like the param suppress at all. it is used in irc to show a different notification (with an info text)
	 *   a QStringinfo would be more interesting, but it is also used to don't show the notification when entering in a channel)
	 */
	void addContact( const KopeteContact *c, bool suppress = false );

	/**
	 * Remove a contact from the session
	 * @param c is the contact
	 * @param reason is the optional raison message showed in the chatwindow
	 * @param format The format of the message
	 */
	void removeContact( const KopeteContact *contact, const QString& reason = QString::null, KopeteMessage::MessageFormat format = KopeteMessage::PlainText );

	/**
	 * Set if the KMM will be deleted when the chatwindow is deleted. It is useful if you want
	 * to keep the KMM alive even if the chatwindow is closed.
	 * Warning: if you set it to false, please keep in mind that you have to reset it to true
	 *  later to delete it. In many case, you should never delete yourself the KMM, just call this
	 *  this method.
	 * default is true.
	 * If there are no chatwindow when setting it to true, the kmm will be deleted.
	 */
	void setCanBeDeleted ( bool canBeDeleted );

	/**
	 * Send a message to the user
	 */
	void sendMessage( KopeteMessage &message );

	/**
	 * Tell the KMM that the user is typing
	 * This method should be called only by a chatwindow. It emits @ref typingMsg signal
	 */
	void typing( bool t );

	/**
	 * Protocols have to call this method when the last message sent has been correctly sent
	 * This will emit @ref messageSuccess signal. and allow the email window to get closed
	 */
	void messageSucceeded();

private slots:
	void slotUpdateDisplayName();
	void slotViewDestroyed();
	void slotOnlineStatusChanged( KopeteContact *c, const KopeteOnlineStatus &status, const KopeteOnlineStatus &oldStatus );
	void slotContactDestroyed( KopeteContact *contact );

protected:
	/**
	 * Create a message manager. This constructor is private, because the
	 * static factory method createSession() creates the object. You may
	 * not create instances yourself directly!
	 */
	KopeteMessageManager( const KopeteContact *user, KopeteContactPtrList others,
		KopeteProtocol *protocol, int id = 0, const char *name = 0 );

	void setMMId( int );
	
	/**
	 * Set wether or not contact from this account may be invited in this chat.
	 * By default, it is set to false
	 * @see inviteContact()
	 * @see mayInvite()
	 */
	void setMayInvite(bool);
	

private:
	KMMPrivate *d;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

