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
#include <qevent.h>

#include "kopetemessage.h"
#include "kopetecontact.h"
#include "kopeteonlinestatus.h"

class KopeteContact;
class KopeteMessageManager;
class KopeteEvent;
class KopeteMessageLog;
class KopeteProtocol;
class KopeteView;

typedef QPtrList<KopeteContact>        KopeteContactPtrList;
typedef QValueList<KopeteMessage>        KopeteMessageList;
typedef QPtrList<KopeteMessageManager> KopeteMessageManagerList;

class ContactAddedEvent : public QCustomEvent
{
	public:
		ContactAddedEvent( KopeteContact *c ) : QCustomEvent( QEvent::User + 1 ) { setData( c ); };
};

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

	KopeteAccount *account() const { return user()->account(); };

	/**
	 * @return Returns a unique identifier associated with this
	 *         manager
	 */
	int mmId() const;

	KopeteMessage currentMessage();

	/**
	 * Used for named chats
	 */
	const QString displayName();

	void setDisplayName( const QString & );

	void setCurrentMessage( const KopeteMessage &message );

	void setContactOnlineStatus( const KopeteContact*, const KopeteOnlineStatus & );

	const KopeteOnlineStatus contactOnlineStatus( const KopeteContact* ) const;

	/**
	 * Return the view for the supplied KopeteMessageManager.  If it already
	 * exists, it will be returned, otherwise, 0L will be returned or a new one
	 * if canCreate=true
	 * @param canCreate create a new one if it does not exist
	 * @param type Specifies the type of view if we have to create one.
	 */
	KopeteView* view(bool canCreate=false  , KopeteMessage::MessageType type = KopeteMessage::Undefined);


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

	void contactAdded(const KopeteContact *, bool surpress);
	void contactRemoved(const KopeteContact *, bool surpress);

	void contactDisplayNameChanged(const QString &, const QString &);

	/**
	 * The name of the chat is changed
	 */
	void displayNameChanged();

	/**
	 * The user is typing a message
	 */
	void typingMsg( bool isTyping );
	/**
	 * Signals that a remote user is typing a message.
	 */
	void remoteTyping( const KopeteContact *, bool );

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
	 * Append a message to the queue
	 */
	void appendMessage( KopeteMessage &msg );

	/**
	 * Add a contact to the session
	 */
	void addContact( const KopeteContact *c, bool surpress = false );

	/**
	 * Remove a contact from the session
	 */
	void removeContact( const KopeteContact *c, bool surpress = false );

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

private slots:
	void slotUpdateDisplayName();
	void slotViewDestroyed();

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

	void customEvent( QCustomEvent * e );


private:
	KMMPrivate *d;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

