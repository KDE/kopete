/*
	kopetemessagemanager.h - Manages all chats

	Copyright   : (c) 2002 by Martijn Klingens
	Email       : klingens@kde.org

	*************************************************************************
	*                                                                       *
	* This program is free software; you can redistribute it and/or modify  *
	* it under the terms of the GNU General Public License as published by  *
	* the Free Software Foundation; either version 2 of the License, or     *
	* (at your option) any later version.                                   *
	*                                                                       *
	*************************************************************************
*/

#ifndef __KOPETEMESSAGEMANAGER_H__
#define __KOPETEMESSAGEMANAGER_H__

#include <qptrlist.h>

class KopeteContact;
class KopeteMessage;
class KopeteMessageManager;

typedef QPtrList<KopeteContact>        KopeteContactList;
typedef QPtrList<KopeteMessage>        KopeteMessageList;
typedef QPtrList<KopeteMessageManager> KopeteMessageManagerList;

class KopeteMessageManager : public QObject
{
public:
	/**
	 * Create a new chat session. Provided is the initial list of contacts in
	 * the session. If a session with exactly these contacts already exists,
	 * it will be reused. Otherwise a new session is created.
	 */
	static KopeteMessageManager* createSession( const KopeteContactList &contacts );

	/**
	 * Get a list of all open sessions
	 */
	static const KopeteMessageManagerList& sessions();

	/**
	 * Delete a chat manager instance
	 */
	~KopeteChatManager();

	/**
	 * Append a message to the queue
	 */
	void appendMessage( const KopeteMessage *msg );

	/**
	 * Add a contact to the session
	 */
	void addContact( const KopeteContact *c );

	/**
	 * Remove a contact from the session
	 */
	void removeContact( const KopeteContact *c );

	/**
	 * Get a list of all contacts in the session
	 */
	const KopeteContactList& contacts() const;

signals:
	/**
	 * A message has been sent by the user or a plugin. The protocol should
	 * connect to this signal to actually send the message over the wire.
	 */
	void messageSent( const KopeteMessage &msg );

private:
	/**
	 * Create a message manager. This constructor is private, because the
	 * static factory method createSession() creates the object. You may
	 * not create instances yourself directly!
	 */
	KopeteMessageManager( const KopeteContactList &contacts,
		QWidget *parent = 0, const char *name = 0 );

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

