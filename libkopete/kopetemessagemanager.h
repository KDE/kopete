/*
    kopetemessagemanager.h - Manages all chats

    Copyright   : (c) 2002 by Martijn Klingens <klingens@kde.org>
                  (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
                  (c) 2002 by Daniel Stone <dstone@kde.org>

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
#include <qvaluelist.h>
#include <qmap.h>

#include "kopetemessage.h"
#include "kopetecontact.h"

class KopeteContact;
class KopeteMessageManager;
class KopeteEvent;
class KopeteMessageLog;
class KopeteProtocol;

typedef QPtrList<KopeteContact>        KopeteContactPtrList;
typedef QValueList<KopeteMessage>        KopeteMessageList;
typedef QPtrList<KopeteMessageManager> KopeteMessageManagerList;

struct  KMMPrivate;

/**
 * Each KopeteMessageManager manages a single chat.  More illuminating docs here!
 */
class KopeteMessageManager : public QObject
{
	friend class KopeteMessageManagerFactory;

	Q_OBJECT

public:
	/**
	 * Delete a chat manager instance
	 */
	~KopeteMessageManager();

	/**
	 * true if logging is turned on
	 */
	bool logging() const;

	/**
	 * Get a list of all contacts in the session
	 * Sorry, had to change this to members(), it was conflicting with
	 * kxContact
	 */
	const KopeteContactPtrList& members() const;

	/**
	 * Get the local user in the session
	 */
	const KopeteContact* user() const;

	/**
	 * Get the protocol being used.
	 */
	KopeteProtocol* protocol() const;

	/**
	 * @return Returns a unique identifier associated with this
	 *         manager
	 */
	int mmId() const;

	KopeteMessage currentMessage();

	/**
	 * Used for named chats
	 */
	virtual const QString chatName();


	void setCurrentMessage( const KopeteMessage &message );

signals:
	/**
	 * Used by a KopeteMessageManager to signal that it is closing.
	 */
	 void closing(KopeteMessageManager *);

	/**
	 * The following signals are used internally by Kopete.
	 * They allow plugins to change message before
	 * they are being displayed or sent.
	 * If I'll see them used anywhere in plugins I will
	 * strangle the author - Zack
	 */
	void messageAppended( KopeteMessage& msg, KopeteMessageManager * = 0L );
	void messageReceived( KopeteMessage& msg, KopeteMessageManager * = 0L );
 	void messageSent( KopeteMessage& msg, KopeteMessageManager * = 0L );
	void messageSuccess();

	void contactAdded(const KopeteContact *);
	void contactRemoved(const KopeteContact *);

	/**
	 * The name of the chat is changed
	 */
	void chatNameChanged();

	/**
	 * The user is typing a message
	 */
	void typingMsg( bool isTyping );
	/**
	 * Signals that a remote user is typing a message.
	 */
	void remoteTyping( const KopeteContact *, bool );

	void dying( QWidget* );
	
public slots:
	/**
	 * Got a typing notification from a user
	 */
	void receivedTypingMsg( const KopeteContact *c , bool isTyping = true );

	/**
	 * Got a typing notification from a user. This is a convenience version
	 * of the above method that takes a QString contactId instead of a full
	 * KopeteContact
	 */
	void receivedTypingMsg( const QString &contactId, bool isTyping = true );

	/**
	 * Enables/disables logging
	 */
	void setLogging( bool on );

	/**
	 * Append a message to the queue
	 */
	void appendMessage( KopeteMessage &msg );

	/**
	 * Add a contact to the session
	 */
	void addContact( const KopeteContact *c );

	/**
	 * Remove a contact from the session
	 */
	void removeContact( const KopeteContact *c );

	/**
	 * Set if the KMM will be deleted when the chatwindow is deleted
	 */
	void setCanBeDeleted ( bool ) ;

	/**
	 * Send a message to the user
	 */
	void sendMessage(KopeteMessage &message);
	/**
	 * Tell the KMM that someone is typing
	 */
	void typing(bool t);

	/**
	 * Called after a message was sent successfully
	 */
	void messageSucceeded();

protected:
	/**
	 * Create a message manager. This constructor is private, because the
	 * static factory method createSession() creates the object. You may
	 * not create instances yourself directly!
	 */
	KopeteMessageManager( const KopeteContact *user,
		KopeteContactPtrList others, KopeteProtocol *protocol, int id = 0,
		QObject *parent = 0, const char *name = 0 );
	
	void setMMId( int );


private:
	KMMPrivate *d;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

