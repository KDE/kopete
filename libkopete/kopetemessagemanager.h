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

class KopeteContact;
class KopeteMessage;
class KopeteMessageManager;
class QObject;
class KopeteChatWindow;
class KopeteEvent;
class KopeteMessageLog;
class KopeteProtocol;


typedef QPtrList<KopeteContact>        KopeteContactPtrList;
typedef QValueList<KopeteMessage>        KopeteMessageList;
typedef QPtrList<KopeteMessageManager> KopeteMessageManagerList;
struct  KMMPrivate;

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
	 * Widget type to display.
	 */
	enum WidgetType { ChatWindow, Email, MDI, FileSend };
	/**
	 * Delete a chat manager instance
	 */
	~KopeteMessageManager();

	/**
	 * Fire up a new widget.
	 */
	void newChatWindow();
	void newReplyWindow();

	/**
	 * Set Reading mode
	 */
	void setReadMode( int mode );

	/**
	 * Get Current Reading mode
	 */
	int readMode() const;

	WidgetType widget() const;

	/**
	 * Read Messages
	 */
	void readMessages();

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
	const KopeteProtocol* protocol() const;

	/**
	 * @return Returns a unique identifier associated with this
	 *         manager
	 */
	int id() const;

signals:
	/**
	 * A message has been sent by the user or a plugin. The protocol should
	 * connect to this signal to actually send the message over the wire.
	 */
	void messageSent(const KopeteMessage& msg, KopeteMessageManager *);
	void dying(KopeteMessageManager *);
	
	/**
	 * The following signals are used internally by Kopete.
	 * They allow plugins to change message before
	 * they are being displayed or sent.
	 * If I'll see them used anywhere in plugins I will
	 * strangle the author - Zack
	 */
	void messageReceived( KopeteMessage& msg );
	void messageQueued( KopeteMessage& msg );

	void contactAdded(const KopeteContact *);
	void contactRemoved(const KopeteContact *);


public slots:
	void readModeChanged();
	void slotSendEnabled(bool);

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


protected slots:
	void slotCancelUnreadMessageEvent();
	void slotEventDeleted(KopeteEvent *);
	void slotChatWindowClosing();
	void slotReplyWindowClosing();
	void slotMessageSent(KopeteMessage &message);
	void slotReadMessages();
	void slotReply();

private:
	/**
	 * Create a message manager. This constructor is private, because the
	 * static factory method createSession() creates the object. You may
	 * not create instances yourself directly!
	 */
	KopeteMessageManager( const KopeteContact *user, KopeteContactPtrList others,
			      KopeteProtocol *protocol, int id = 0, QString logFile = QString::null,
			      enum WidgetType widget = ChatWindow,
			      QObject *parent = 0, const char *name = 0 );
	void setID( int );
	KMMPrivate *d;
};

#endif



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */

// vim: set noet ts=4 sts=4 sw=4:

