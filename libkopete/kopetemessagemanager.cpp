/*
	kopetemessagemanager.cpp - Manages all chats

	Copyright   : (c) 2002 by Martijn Klingens <klingens@kde.org>
                  (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

	Thanks to Daniel Stone for heavy bugfixing and testing.	

	*************************************************************************
	*                                                                       *
	* This program is free software; you can redistribute it and/or modify  *
	* it under the terms of the GNU General Public License as published by  *
	* the Free Software Foundation; either version 2 of the License, or     *
	* (at your option) any later version.                                   *
	*                                                                       *
	*************************************************************************
*/

#include "kopetemessagemanager.h"
#include "kopetechatwindow.h"
#include "kopeteevent.h"
#include "kopete.h"

#include "messagelog.h"
#include <kdebug.h>
#include <klocale.h>

KopeteMessageManager::KopeteMessageManager( const KopeteContact *user, KopeteContactList others,
		QString logFile, QObject *parent, const char *name) : QObject( parent, name)
{

	mContactList = others;
	mUser = user;
	mChatWindow = 0L;
	mUnreadMessageEvent = 0L;
	if ( kopeteapp->appearance()->useQueue() )
	{
		mReadMode = Queued;
	}
	else
	{
		mReadMode = Popup;
	}
	
	if (!logFile.isEmpty())
	{
		QString logFileName = "kopete/" + logFile;
		mLogger = new KopeteMessageLog(logFileName, this);
	}
	else
	{
		mLogger = 0L;
	}

}

KopeteMessageManager::~KopeteMessageManager()
{
	emit dying(this);
}

void KopeteMessageManager::setReadMode( int mode )
{

	if ( (mode == Queued) || (mode == Popup) )
	{
		mReadMode = mode;
	}
	else
	{
		kdDebug() << "[KopeteMessageManager] ERROR: unknown reading method, setting to default" << endl;
		mReadMode = Queued;
	}
}

void KopeteMessageManager::readMessages()
{
	if ( mChatWindow != 0L )	// We still have our messagebox
	{
		kdDebug() << "[KopeteMessageManager] mChatWindow has already been created" << endl;
		if (mChatWindow->isMinimized() )
			kdDebug() << "[KopeteMessageManager] mChatWindow is minimized" << endl;
		if (mChatWindow->isHidden() )
			kdDebug() << "[KopeteMessageManager] mChatWindow is hidden" << endl;
		mChatWindow->raise();	// make it top window
	}
	else
	{
		/* We create the chat window */
		mChatWindow = new KopeteChatWindow ();
		/* When the window is shown, we have to delete tjis contact event */
		kdDebug() << "[KopeteMessageManager] Connecting message box shown() to event killer" << endl;
		connect ( mChatWindow, SIGNAL(shown()), this, SLOT(cancelUnreadMessageEvent()) );
		connect ( mChatWindow, SIGNAL(sendMessage(const QString &)), this, SLOT(messageSentFromWindow(const QString &)) );
		connect ( mChatWindow, SIGNAL(closeClicked()), this, SLOT(chatWindowClosing()) );
	}
	
	for (KopeteMessageList::Iterator it = mMessageQueue.begin(); it != mMessageQueue.end(); it++)
	{
		kdDebug() << "[KopeteMessageManager] Inserting message from " << (*it).from() << endl;
		mChatWindow->messageReceived((*it));
	}
	mMessageQueue.clear();
	mChatWindow->show();	// show message window again
}

void KopeteMessageManager::slotReadMessages()
{
	readMessages();
}

void KopeteMessageManager::messageSentFromWindow(const QString &message)
{
	QString body = message;
	KopeteMessage tmpmessage(mUser->userID(), (mContactList.first())->userID(), body, KopeteMessage::Outbound);
	emit messageSent ( tmpmessage );
}

void KopeteMessageManager::chatWindowClosing()
{
	kdDebug() << "[KopeteMessageManager] Chat Window closed, now 0L" << endl;	
	mChatWindow = 0L;
}

void KopeteMessageManager::cancelUnreadMessageEvent()
{
	if (mUnreadMessageEvent == 0L)
	{
		kdDebug() << "[KopeteMessageManager] No event to delete" << endl;
	}
	else
	{
		kdDebug() << "[KopeteMessageManager] cancelUnreadMessageEvent Deleting Event" << endl;
		delete mUnreadMessageEvent;
		mUnreadMessageEvent = 0L;
	}		
}


void KopeteMessageManager::appendMessage( const KopeteMessage &msg )
{
	//mMessageQueue.append( KopeteMessage( msg.timestamp(), msg.from(), msg.to(), msg.body(), msg.direction(),msg.fg(), msg.bg(), msg.font()));
	mMessageQueue.append(msg);
	if( mLogger )
	{
		mLogger->append( msg );
	}

	/* We dont need an event if it already exits or if we are in popup mode */
	if ( (mUnreadMessageEvent == 0L) && ( mReadMode != Popup) && (msg.direction() == KopeteMessage::Inbound) )
	{
		mUnreadMessageEvent = new KopeteEvent( i18n("Message from %1").arg(msg.from()), "kopete/pics/newmsg.png", this, SLOT(slotReadMessages()));
		kopeteapp->notifyEvent( mUnreadMessageEvent );
	}

	if (mReadMode == Popup)
	{
		readMessages();
	}
}
void KopeteMessageManager::addContact( const KopeteContact *c )
{
	KopeteContact *tmp;

	for ( tmp = mContactList.first(); tmp; tmp = mContactList.next() )
	{
		if ( tmp == c )
		{
			kdDebug() << "[KopeteMessageManager] Contact already exists" <<endl;
			return;
		}
	}

	kdDebug() << "[KopeteMessageManager] Contact Joined session" <<endl;
	mContactList.append(c);
}

void KopeteMessageManager::removeContact( const KopeteContact *c )
{
	mContactList.take( mContactList.find(c) );
}

