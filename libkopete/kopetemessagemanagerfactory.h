/*
    kopetemessagemanagerfactory.h - Creates chat sessions

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

#ifndef KOPETEMESSAGEMANAGERFACTORY_H
#define KOPETEMESSAGEMANAGERFACTORY_H

#include <qobject.h>
#include <qptrlist.h>
#include <qintdict.h>
#include <qvaluelist.h>

#include "kopetemessagemanager.h"
#include "kopetemessage.h"

class KopeteView;

namespace Kopete
{

class Contact;
class Protocol;
class MessageEvent;

typedef QPtrList<Contact>        ContactPtrList;
typedef QValueList<Message>      MessageList;
typedef QIntDict<MessageManager> MessageManagerDict;

/**
 * @author Duncan Mac-Vicar Prett <duncan@kde.org>
 *
 * Kopete::MessageManagerFactory is responsible for creating and tracking Kopete::MessageManager
 * instances for each chat.
 */
class MessageManagerFactory : public QObject
{
	Q_OBJECT

public:
	static MessageManagerFactory* factory();

	~MessageManagerFactory();

	/**
	 * Create a new chat session. Provided is the initial list of contacts in
	 * the session. If a session with exactly these contacts already exists,
	 * it will be reused. Otherwise a new session is created.
	 * @param user The local user in the session.
	 * @param chatContacts The list of contacts taking part in the chat.
	 * @param protocol The protocol that the chat is using.
	 * @return A pointer to a new or reused Kopete::MessageManager.
	 */
	Kopete::MessageManager* create( const Kopete::Contact *user,
		Kopete::ContactPtrList chatContacts, Kopete::Protocol *protocol);

	/**
	 * Find a chat session, if one exists, that matches the given list of contacts.
	 * @param user The local user in the session.
	 * @param chatContacts The list of contacts taking part in the chat.
	 * @param protocol The protocol that the chat is using.
	 * @return A pointer to an existing Kopete::MessageManager, or 0L if none was found.
	 */
	Kopete::MessageManager* findMessageManager( const Kopete::Contact *user,
		Kopete::ContactPtrList chatContacts, Kopete::Protocol *protocol);

	/**
	 * Registers a Kopete::MessageManager (or subclass thereof) with the Kopete::MessageManagerFactory
	 */
	void addMessageManager(Kopete::MessageManager *);

	/**
	 * Find the idth Kopete::MessageManager that the factory knows of.
	 * @param id The number of the desired Kopete::MessageManager.
	 * @return A pointer to the Kopete::MessageManager, or 0 if it was not found.
	 */
	Kopete::MessageManager *findMessageManager( int id );

	/**
	 * Get a list of all open sessions.
	 */
	const Kopete::MessageManagerDict& sessions();

	/**
	 * @internal
	 * called by the kmm itself when it gets deleted
	 */
	void removeSession( Kopete::MessageManager *session );

	/**
	 * create a new view for the manager.
	 * only the manager should call this function
	 */
	KopeteView *createView( Kopete::MessageManager * , Kopete::Message::ViewType type );

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
	void messageManagerCreated( Kopete::MessageManager *);

	/*
	 * Request the creation of a new view
	 */
	void requestView(KopeteView*& , Kopete::MessageManager * , Kopete::Message::ViewType type );

	/**
	 * the message is ready to be displayed
	 */
	void display( Kopete::Message& message, Kopete::MessageManager * );

	/**
	 * A new event has been posted.
	 */
	void newEvent(Kopete::MessageEvent *);

	/**
	 * The global shortcut for sending message has been used
	 */
	void readMessage();

	/**
	 * Emit this signal to obtain the avtive view from the
	 * KopeteViewManager
	 *
	 * FIXME: This is realy dumb API
	 */
	 void getActiveView( KopeteView *& );

private:
	MessageManagerFactory( QObject* parent = 0, const char* name = 0 );

	int mId;
	Kopete::MessageManagerDict mSessionDict;

	static MessageManagerFactory *s_factory;

};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

