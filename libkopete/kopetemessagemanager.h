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
#include <qobject.h>

#include "kopetemessage.h"
#include "kopetecontact.h"
#include "kopetechatwindow.h"
#include "kopeteprotocol.h"

class KopeteContact;
class KopeteMessage;
class KopeteMessageManager;
class QObject;
class KopeteChatWindow;
class KopeteEvent;
class KopeteMessageLog;


typedef QPtrList<KopeteContact>        KopeteContactList;
typedef QValueList<KopeteMessage>        KopeteMessageList;
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
	 * Fire up a new KopeteChatWindow
	 */
	void newChatWindow();
	
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
	 * Add a resource to the session
	 */
	void addResource(const KopeteContact *c, QString resource);

	/**
	 * Remove a resource from the session
	 */
	void removeResource(const KopeteContact *c, QString resource);
	
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
	const KopeteContactList& members() const { return mContactList; }; /* Sorry, had to change this to members(), it was conflicting with kxContact */
    /**
	 * Get the local user in the session
	 */
	const KopeteContact* user() const { return mUser; };
    const KopeteProtocol* protocol() const { return mProtocol; };

	bool serverChecked(); //return if the send through server checkbox for icq is checked
	void checkServer(bool state); //Set the send through server checkbox for icq

signals:
	/**
	 * A message has been sent by the user or a plugin. The protocol should
	 * connect to this signal to actually send the message over the wire.
	 */
	void messageSent( const KopeteMessage msg );
	void dying( KopeteMessageManager *);

public slots:
	void readModeChanged();
	
protected slots:
	void cancelUnreadMessageEvent();
	void slotEventDeleted(KopeteEvent *);
    void chatWindowClosing();
	void messageSentFromWindow( const QString &message);
	void slotReadMessages();
private:
	/**
	 * Create a message manager. This constructor is private, because the
	 * static factory method createSession() creates the object. You may
	 * not create instances yourself directly!
	 */
	KopeteMessageManager( const KopeteContact *user, KopeteContactList others,
		KopeteProtocol *protocol, QString logFile = QString::null, int widget = 0, int capabilities = 0,
		QObject *parent = 0, const char *name = 0 );

	KopeteContactList mContactList;
	const KopeteContact *mUser;
	KopeteChatWindow *mChatWindow;
	KopeteEvent *mUnreadMessageEvent;
	KopeteMessageList mMessageQueue;
	KopeteMessageLog *mLogger;
	int mReadMode, mWidget, mCapabilities;
	QMap<const KopeteContact *, QStringList> resources;
	KopeteProtocol *mProtocol;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

