/*
    kopeteChatSessionmanager.h - Creates chat sessions

    Copyright (c) 2002-2003 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart <ogoffart @ tiscalinet.be>

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

#ifndef KOPETEMESSAGEMANAGERFACTORY_H
#define KOPETEMESSAGEMANAGERFACTORY_H

#include <qobject.h>
#include <qvaluelist.h>

namespace Kopete
{

class ChatSession;
class Message;
class Contact;
namespace UI
{
	class ChatView;
}

/**
 * @author Duncan Mac-Vicar Prett <duncan@kde.org>
 *
 * ChatSessionManager is responsible for tracking ChatSession
 * instances for each chat.
 */
class ChatSessionManager : public QObject
{
	Q_OBJECT

public:
	static ChatSessionManager* self();

	~ChatSessionManager();

	/**
	 * Registers a ChatSession (or subclass thereof) with the ChatSessionManager
	 */
	void registerChatSession(ChatSession *);
	
	
	/**
	 * Get a list of all open sessions.
	 */
	QValueList<ChatSession*> sessions() const;
	
	
	
	
	/**
	 * Create a new chat session. Provided is the initial list of contacts in
	 * the session. If a session with exactly these contacts already exists,
	 * it will be reused. Otherwise a new session is created.
	 * @param myself The local user in the session.   see @ref ChatSession::myself()
	 * @param chatContacts The list of contacts taking part in the chat.
	 * @param protocol The protocol that the chat is using.
	 * @return A pointer to a new or reused ChatSession.
	 */
	ChatSession* create( Contact *myself, QValueList<Contact*> chatContacts );

	/**
	 * Find a chat session, if one exists, that matches the given list of contacts.
	 * @param user The local user in the session.
	 * @param chatContacts The list of contacts taking part in the chat.
	 * @param protocol The protocol that the chat is using.
	 * @return A pointer to an existing ChatSession, or 0L if none was found.
	 */
	ChatSession* findChatSession( const Contact *myself, QValueList<Contact*> chatContacts) const;

	/**
	 * Returns the current active chat view
	 */
	UI::ChatView* activeView();
	
	/**
	 * Set the active chat view
	 */
	void setActiveView(UI::ChatView* );

	

#if 0 //TODO
	/**
	 * create a new view for the manager.
	 * only the manager should call this function
	 */
	KopeteView *createView( KopeteChatSession * , KopeteMessage::MessageType type );

	/**
	 * Post a new event. this will emit the @ref newEvent signal
	 */
	void postNewEvent(KopeteEvent*);


 //signals:
	/**
	 * This signal is emitted whenever a message
	 * is about to be displayed by the KopeteChatWindow.
	 * Please remember that both messages sent and
	 * messages received will emit this signal!
	 * Plugins may connect to this signal to change
	 * the message contents before it's going to be displayed.
	 */
	void aboutToDisplay( KopeteMessage& message );

	/**
	 * Plugins may connect to this signal
	 * to manipulate the contents of the
	 * message that is being sent.
	 */
	void aboutToSend( KopeteMessage& message );

	/**
	 * Plugins may connect to this signal
	 * to manipulate the contents of the
	 * message that is being received.
	 *
	 * This signal is emitted before @ref aboutToDisplay()
	 */
	void aboutToReceive( KopeteMessage& message );

	/**
	 * A new view has been created
	 */
	void viewCreated( KopeteView * );

	/**
	 * A view as been activated(manually only?).
	 */
	void viewActivated( KopeteView *view );

	/*
	 * A view is about to close.
	 */
	void viewClosing( KopeteView *view );


	/*
	 * Request the creation of a new view
	 */
	void requestView(KopeteView*& , KopeteChatSession * , KopeteMessage::MessageType type );

	/**
	 * the message is ready to be displayed
	 */
	void display( KopeteMessage& message, KopeteChatSession * );

	/**
	 * A new event has been posted.
	 */
	void newEvent(KopeteEvent *);

	/**
	 * The global shortcut for sending message has been used
	 */
	void readMessage();

#endif 

signals:
	/**
	 * a new chatSession has been created
	 */
	void chatSessionCreated( ChatSession *);


private slots:
	/**
	 * @internal
	 * called by the kmm itself when it gets deleted
	 */
	void removeSession( ChatSession *session );


private:
	ChatSessionManager( QObject* parent = 0, const char* name = 0 );

	class Private;
	Private *d;

	static ChatSessionManager *s_self;

};


} //END namespace Kopete

#endif



