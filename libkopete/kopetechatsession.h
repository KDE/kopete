/*
    kopetechatsession.h - Manages all chats

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Daniel Stone           <dstone@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart @tiscalinet.be>
    Copyright (c) 2003      by Jason Keirstead        <jason@keirstead.org>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

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
#include <qvaluelist.h>

#include <kxmlguiclient.h>
#include <kopetemessage.h>



namespace Kopete
{


class Contact;
class Message;
class Protocol;
class OnlineStatus;
class Account;

namespace UI
{
	class ChatView;
}


/**
 * @author Duncan Mac-Vicar Prett <duncan@kde.org>
 * @author Daniel Stone           <dstone@kde.org>
 * @author Martijn Klingens       <klingens@kde.org>
 * @author Olivier Goffart        <ogoffart @tiscalinet.be>
 * @author Jason Keirstead        <jason@keirstead.org>
 *
 * The ChatSession manages a single chat.
 * It is an interface between the protocol, and the chatwindow.
 * The protocol can connect to @ref messageSent() signals to send the message, and can
 * append received message with @ref messageReceived()
 *
 * The KMM inherits from KXMLGUIClient, this client is merged with the chatwindow's ui
 * so plugins can add childClients of this client to add their own actions in the
 * chatwindow.
 */
class ChatSession : public QObject , public KXMLGUIClient
{
	Q_OBJECT

public:

	/**
	 * the notification messages that are passed with @ref addContact,  @ref contactAdded , @ref removeContact, @ref contactRemoved
	 */
	enum NotificationType{ Normal , /** The notification string is (if not empty) in plain text format, and html tags will be escaped */
						   RichText , /** The notificaiton string is contains HTML tags which will not be escaped */
						   Silent }; /** The notification will not be shown */


   	/**
	 * @brief Create a message manager.
	 * If you subclass the ChatSession, don't foget to call  @ref ChatSessionManager::registerChatSession
	 *
	 * You may not create dirrect instances yourself directly, uses @ref ChatSessionManager::createChatSession
	 */
	ChatSession( Contact *user, QValueList<Contact*> others,  const char *name = 0 );



	/**
	 * Delete a chat manager instance
	 * You shouldn't delete the KMM yourself. it will be deleted when the chatwindow is closed
	 * see also @ref setCanBeDeleted()
	 */
	~ChatSession();

	/**
	 * @brief Get a list of all contacts in the session
	 */
	QValueList<Contact*> members() const;

	/**
	 * @brief Get the local user in the session
	 * myself is generaly the same as @code account()->myself() @endcode  But may differ for instance in Jabber group chats.
	 * @return the local user in the session
	 */
	Contact* myself() const;

	/**
	 * @brief Get the protocol being used.
	 * @return the protocol
	 */
	Protocol* protocol() const;

	/**
	 * @brief get the account
	 * @return the account
	 */
	Account *account() const ;

	/**
	 * @brief the unique id
	 * @return a unique identifier associated with this manager
	 */
	unsigned int chatSessionId() const;

	/**
	 * @brief The caption of the chat
	 *
	 * Used for named chats.   see @ref setDisplayName
	 */
	QString displayName() const;

	/**
	 * @brief change the displayname
	 *
	 * change the display name of the chat
	 * If the displayName is QString::null  (the default)  the displayName is automaticaly
	 * genered by libkopete using contacts names and status
	 */
	void setDisplayName( const QString &displayName );

	/**
	 * @brief set a specified OnlineStatus for a specified Contact.
	 *
	 * Sometimes you may want to set a different status to a contact while it is in a chat.
	 * The could be used for channel operator for instance.
	 *
	 * Set a empty OnlineStatus to fall back to the default.
	 *
	 * By default, all contact have their own status
	 */
	void setContactOnlineStatus( Contact *contact, const OnlineStatus &newStatus );

	/**
	 * @brief get the status of a contact.
	 *
	 * see @ref setContactOnlineStatus()
	 */
	OnlineStatus contactOnlineStatus( const Contact *contact ) const;

	enum CanCreateFlags {  CannotCreate=false , CanCreate=true  };

	/**
	 * @brief the manager's view
	 *
	 * Return the view for the supplied ChatSession.  If it already
	 * exists, it will be returned, otherwise, 0L will be returned or a new one
	 * if canCreate=true
	 * @param canCreate create a new one if it does not exist
	 * @param type Specifies the type of view if we have to create one.
	 */
	UI::ChatView* view( CanCreateFlags canCreate=CannotCreate, Message::MessageType type = Message::Undefined );


	/**
	 * says if you may invite contact from the same account to this chat with @ref inviteContact
	 * @see setMayInvite
	 * @return true if it is possible to invite contact to this chat.
	 */
	bool mayInvite() const ;

	/**
	 * this method is called when a contact is dragged to the contactlist.
	 * @p contactId is the id of the contact. the contact is supposed to be of the same account as
	 * the @ref account() but we can't be sure the Contact is realy on the contactlist
	 *
	 * It is possible to drag contact only if @ref mayInvite return true
	 *
	 * the default implementaiton do nothing
	 */
	virtual void inviteContact(const QString &contactId);


signals:
	/**
	 * Called in the destructor.
	 */
	void chatSessionDestroyed( ChatSession * );


	/**
	 * @brief a new contact is now in the chat
	 * @param contact is the contact which has been added
	 * @param flags  if set to Supress, the notificaiton may not be shown on the chat window
	 */
	void contactAdded( const Contact *contact, NotificationType flags );

	/**
	 * @brief a contact is no longer in this chat
	 * @param contact is the contact which has been added
	 * @param reason Is a reason message which will be shown with
	 * @param flags  if set to Supress, the notificaiton may not be shown on the chat window. if set to RichText, the reason is in RichText
	 */
	void contactRemoved( const Contact *contact, const QString &reason, NotificationType flags );

	/**
	 * @brief a contact in this chat has changed his status
	 */
	void onlineStatusChanged( Contact *, const OnlineStatus &, const OnlineStatus & );

	/**
	 * @brief The name of the chat is changed
	 * @see setDisplayName()
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
	void userTyping( bool isTyping );

	/**
	 * Signals that a remote user is typing a message.
	 * the chatwindow connects to this signal to update the statusbar
	 */
	void remoteTyping( const Contact *contact, bool isTyping );



public slots:
	/**
	 * @brief Got a typing notification from a user
	 */
	void receivedTypingMsg( const Contact *contact , bool isTyping = true );

	/**
	 * Got a typing notification from a user. This is a convenience version
	 * of the above method that takes a QString contactId instead of a full
	 * Contact
	 */
	void receivedTypingMsg( const QString &contactId, bool isTyping = true );

	/**
	 * Show a message to the chatwindow, or append it to the queue.
	 * This is the function protocols HAVE TO call for both incoming and outgoing messages
	 * if the message must be showed in the chatwindow
	 */
	void appendMessage( Message &msg );

	/**
	 * Add a contact to the session
	 * @param c is the contact
	 * @param flag if set to Silent, there will be no automatic notifications in the chatwindow.
	 */
	void addContact( Contact *c, NotificationType flags=Normal );

	/**
	 * Remove a contact from the session
	 * @param c is the contact
	 * @param reason is the optional raison message showed in the chatwindow
	 * @param flags You may specify if the format is EichText, or if you don't want to show any notifications
	 */
	void removeContact( Contact *contact, const QString& reason = QString::null, NotificationType flags=Normal );

	/**
	 * Set if this ChetSesison will be deleted when the chatwindow is deleted. It is useful if you want
	 * to keep it alive even if the chatwindow is closed (if you are handling file transfer or others)
	 * @warning If you set it to false, please keep in mind that you have to reset it to true
	 *  later to delete it. In many case, you should never delete it yourself just call this method.
	 *
	 * default is true.
	 *
	 * If there are no chatwindow when setting it to true, the kmm will be deleted.
	 */
	void setCanBeDeleted ( bool canBeDeleted );

	/**
	 * Send a message to the user
	 */
	void sendMessage( Message &message );

	/**
	 * Tell the KMM that the user is typing
	 * This method should be called only by a chatwindow. It emits @ref userTyping signal
	 */
	void typing( bool t );



private slots:
	void slotUpdateDisplayName();
	void slotViewDestroyed();
	void slotOnlineStatusChanged( Contact *c, const OnlineStatus &status, const OnlineStatus &oldStatus );
	void slotContactDestroyed( Contact *contact );

protected:

	/**
	 * Set wether or not contact from this account may be invited in this chat.
	 * By default, it is set to false
	 * @see inviteContact()
	 * @see mayInvite()
	 */
	void setMayInvite(bool);


	virtual void virtual_hook(uint id, void *data);

private:
	class Private;
	Private *d;




	#if 0 //TODO message processing
	/**
	 * a message will be soon shown in the chatwindow.
	 * See @ref ChatSessionFactory::aboutToShow() signal
	 */
	void messageAppended( Message &msg, ChatSession *kmm = 0L );

	/**
	 * a message will be soon received
	 * See @ref ChatSessionFactory::aboutToReceive() signal
	 */
	void messageReceived( Message &msg, ChatSession *kmm = 0L );

	/**
	 * @brief a message is going to be sent
	 *
	 * The message is going to be sent.
	 * protocols can connect to this signal to send the message ro the network.
	 * the protocol have also to call @ref appendMessage() and @ref messageSucceeded()
	 * See also @ref ChatSessionFactory::aboutToSend() signal
	 */
	void messageSent( Message &msg, ChatSession *kmm = 0L );

	/**
	 * The last message has finaly successfully been sent
	 */
	void messageSuccess();


	/**
	 * Protocols have to call this method when the last message sent has been correctly sent
	 * This will emit @ref messageSuccess signal. and allow the email window to get closed
	 */
	void messageSucceeded();

#endif
};

} //END namespace Kopete

#endif



