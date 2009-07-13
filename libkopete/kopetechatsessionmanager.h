/*
    kopetechatsessionmanager.h - Creates chat sessions

    Copyright (c) 2002-2003 by Duncan Mac-Vicar Prett <duncan@kde.org>

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

#ifndef KOPETECHATSESSIONMANAGER_H
#define KOPETECHATSESSIONMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QList>

#include "kopetechatsession.h"
#include "kopetemessage.h"

#include "kopete_export.h"

class KopeteView;

namespace Kopete
{

class Contact;
class Protocol;
class MessageEvent;

typedef QList<Contact*>     ContactPtrList;
typedef QList<Message>      MessageList;

/**
 * @author Duncan Mac-Vicar Prett <duncan@kde.org>
 *
 * Kopete::ChatSessionManager is responsible for creating and tracking Kopete::ChatSession
 * instances for each chat.
 */
class KOPETE_EXPORT ChatSessionManager : public QObject
{
	Q_OBJECT

public:
	static ChatSessionManager* self();

	~ChatSessionManager();

	/**
	 * Create a new chat session. Provided is the initial list of contacts in
	 * the session. If a session with exactly these contacts already exists,
	 * it will be reused. Otherwise a new session is created.
	 * @param user The local user in the session.
	 * @param chatContacts The list of contacts taking part in the chat.
	 * @param protocol The protocol that the chat is using.
	 * @return A pointer to a new or reused Kopete::ChatSession.
	 */
	Kopete::ChatSession* create( const Kopete::Contact *user,
		Kopete::ContactPtrList chatContacts, Kopete::Protocol *protocol, Kopete::ChatSession::Form form = Kopete::ChatSession::Small );

	/**
	 * Find a chat session, if one exists, that matches the given list of contacts.
	 * @param user The local user in the session.
	 * @param chatContacts The list of contacts taking part in the chat.
	 * @param protocol The protocol that the chat is using.
	 * @return A pointer to an existing Kopete::ChatSession, or 0L if none was found.
	 */
	Kopete::ChatSession* findChatSession( const Kopete::Contact *user,
		Kopete::ContactPtrList chatContacts, Kopete::Protocol *protocol);

	/**
	 * Registers a Kopete::ChatSession (or subclass thereof) with the Kopete::ChatSessionManager
	 */
	void registerChatSession(Kopete::ChatSession *);

	/**
	 * Get a list of all open sessions.
	 */
	QList<ChatSession*> sessions();

	/**
	 * @internal
	 * called by the kmm itself when it gets deleted
	 */
	void removeSession( Kopete::ChatSession *session );

	/**
	 * create a new view for the manager.
	 * only the manager should call this function
	 */
	KopeteView *createView( Kopete::ChatSession * , const QString &requestedPlugin = QString() );

	/**
	 * Post a new event. this will emit the @ref newEvent signal
	 */
	void postNewEvent(Kopete::MessageEvent*);

	/**
	 * Returns the current active Kopete view
	 */
	KopeteView *activeView();

signals:
	/**
	 * This signal is emitted whenever a message
	 * is about to be displayed by the KopeteChatWindow.
	 * Please remember that both messages sent and
	 * messages received will emit this signal!
	 * Plugins may connect to this signal to change
	 * the message contents before it's going to be displayed.
	 */
	void aboutToDisplay( Kopete::Message& message );

	/**
	 * Plugins may connect to this signal
	 * to manipulate the contents of the
	 * message that is being sent.
	 */
	void aboutToSend( Kopete::Message& message );

	/**
	 * Plugins may connect to this signal
	 * to manipulate the contents of the
	 * message that is being received.
	 *
	 * This signal is emitted before @ref aboutToDisplay()
	 */
	void aboutToReceive( Kopete::Message& message );

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

	/**
	 * a new KMM has been created
	 */
	void chatSessionCreated( Kopete::ChatSession *);

	/**
	 * the message is ready to be displayed
	 */
	void display( Kopete::Message& message, Kopete::ChatSession * );

	/**
	 * A new event has been posted.
	 */
	void newEvent(Kopete::MessageEvent *);

	/**
	 * The global shortcut for sending message has been used
	 */
	void readMessage();

public slots:
	void slotReadMessage();

private:
	ChatSessionManager( QObject* parent = 0 );

	class Private;
	Private * const d;

	static ChatSessionManager *s_self;

};

}

#endif // KOPETECHATSESSIONMANAGER_H

// vim: set noet ts=4 sts=4 sw=4:
