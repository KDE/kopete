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
#include "kopeteemailwindow.h"
#include "kopeteprotocol.h"

class KopeteContact;
class KopeteMessage;
class KopeteMessageManager;
class QObject;
class KopeteChatWindow;
class KopeteEvent;
class KopeteMessageLog;


typedef QPtrList<KopeteContact>        KopeteContactPtrList;
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

	WidgetType widget() { return mWidget; };

	/**
	 * Read Messages
	 */
	void readMessages();

	/**
	 * Get a list of all contacts in the session
	 * Sorry, had to change this to members(), it was conflicting with
	 * kxContact
	 */
	const KopeteContactPtrList& members() const { return mContactList; }

	/**
	 * Get the local user in the session
	 */
	const KopeteContact* user() const { return mUser; };
    const KopeteProtocol* protocol() const { return mProtocol; };

signals:
	/**
	 * A message has been sent by the user or a plugin. The protocol should
	 * connect to this signal to actually send the message over the wire.
	 */
	void messageSent(const KopeteMessage msg);
	void dying(KopeteMessageManager *);

public slots:
	void readModeChanged();
	void slotSendEnabled(bool);
protected slots:
	void slotCancelUnreadMessageEvent();
	void slotEventDeleted(KopeteEvent *);
    void slotChatWindowClosing();
	void slotReplyWindowClosing();
	void slotMessageSent(const KopeteMessage &message);
	void slotReadMessages();
	void slotReply();
	
private:
	/**
	 * Create a message manager. This constructor is private, because the
	 * static factory method createSession() creates the object. You may
	 * not create instances yourself directly!
	 */
	KopeteMessageManager( const KopeteContact *user, KopeteContactPtrList others,
		KopeteProtocol *protocol, QString logFile = QString::null, enum WidgetType widget = ChatWindow,
		QObject *parent = 0, const char *name = 0 );

	KopeteContactPtrList mContactList;
	const KopeteContact *mUser;
	KopeteChatWindow *mChatWindow;
	KopeteEmailWindow *mEmailWindow, *mEmailReplyWindow;
	KopeteEvent *mUnreadMessageEvent;
	KopeteMessageList mMessageQueue;
	KopeteMessageLog *mLogger;
	int mReadMode;
	enum WidgetType mWidget;
	QMap<const KopeteContact *, QStringList> resources;
	KopeteProtocol *mProtocol;
	bool mSendEnabled;
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

