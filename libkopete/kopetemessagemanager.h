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
#include <qobject.h>

#include "kopetemessage.h"
#include "kopetecontact.h"

class KopeteContact;
class KopeteMessage;
class KopeteMessageManager;
class QObject;
class KopeteChatWindow;
class KopeteEvent;
class KopeteMessageLog;


typedef QPtrList<KopeteContact>        KopeteContactList;
typedef QPtrList<KopeteMessage>        KopeteMessageList;
typedef QPtrList<KopeteMessageManager> KopeteMessageManagerList;

class KopeteMessageManager : public QObject
{
friend class KopeteMessageManagerFactory;
Q_OBJECT
public:
	/**
	 *	Reading mode
	 */
	enum ReadingMode { Queued, Popup };
	/**
	 * Delete a chat manager instance
	 */
	~KopeteMessageManager();

	/**
	 * Append a message to the queue
	 */
	void appendMessage( const KopeteMessage &msg );

	/**
	 * Add a contact to the session
	 */
	void addContact( const KopeteContact *c );

	/**
	 * Remove a contact from the session
	 */
	void removeContact( const KopeteContact *c );

	/**
	 * Set Reading mode
	 */
	void setReadMode( int mode );

	/**
	 * Get Current Reading mode
	 */
	int readMode() { return mReadMode; };


	/**
	 *	Read Messages
	 */
	void readMessages();

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

protected slots:
	void cancelUnreadMessageEvent();
    void chatWindowClosing();
private:
	/**
	 * Create a message manager. This constructor is private, because the
	 * static factory method createSession() creates the object. You may
	 * not create instances yourself directly!
	 */
	KopeteMessageManager( const KopeteContact *user, KopeteContactList &others,
		QString logFile = QString::null, QObject *parent = 0, const char *name = 0 );

	KopeteContactList mContactList;
	KopeteChatWindow *mChatWindow;
	KopeteEvent *mUnreadMessageEvent;
	KopeteMessageList mMessageQueue;
	KopeteMessageLog *mLogger;
	int mReadMode;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

