/*
	kopetemessagemanager.cpp - Manages all chats

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

#include "kopetemessagemanager.h"
#include "kopetechatwindow.h"
#include "kopeteevent.h"
#include "kopete.h"

#include "messagelog.h"

#include <kcolorbutton.h>
#include <kdebug.h>
#include <klocale.h>
#include <knotifyclient.h>

#include <qstylesheet.h>

KopeteMessageManager::KopeteMessageManager( const KopeteContact *user, KopeteContactList others,
		KopeteProtocol *protocol, QString logFile, enum WidgetType widget,
		QObject *parent, const char *name) : QObject( parent, name)
{
    mSendEnabled = true;
	mContactList = others;
	mUser = user;
	mChatWindow = 0L;
	mUnreadMessageEvent = 0L;
	mProtocol = protocol;
	mWidget = widget;
	
	readModeChanged();
	connect(kopeteapp->appearance(), SIGNAL(queueChanged()), this, SLOT(readModeChanged()));
	
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

void KopeteMessageManager::slotSendEnabled( bool e )
{
	mSendEnabled = e;
	if ( mChatWindow )
	{
		mChatWindow->setSendEnabled(e);
	}	
}

void KopeteMessageManager::newChatWindow() {
	mChatWindow = new KopeteChatWindow();
	mChatWindow->setSendEnabled( mSendEnabled);	

	if (mContactList.first() != 0L)
		mChatWindow->setCaption(mContactList.first()->name()); //TODO: add multi-user support
	/* When the window is shown, we have to delete this contact event */
	kdDebug() << "[KopeteMessageManager] Connecting message box shown() to event killer" << endl;
	connect (mChatWindow, SIGNAL(shown()), this, SLOT(cancelUnreadMessageEvent()));
	connect (mChatWindow, SIGNAL(sendMessage(const QString &)), this, SLOT(messageSentFromWindow(const QString &)));
	connect (mChatWindow, SIGNAL(closeClicked()), this, SLOT(chatWindowClosing()));
}

void KopeteMessageManager::setReadMode(int mode)
{
	if ((mode == Queued) || (mode == Popup))
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
	if (mWidget == ChatWindow) {
		if (mChatWindow == 0L) {
			kdDebug() << "[KopeteMessageManager] mChatWindow just doesn't exist" << endl;
			newChatWindow();
		}
		if (mChatWindow->isMinimized())
			kdDebug() << "[KopeteMessageManager] mChatWindow is minimized" << endl;
		if (mChatWindow->isHidden())
			kdDebug() << "[KopeteMessageManager] mChatWindow is hidden" << endl;
		mChatWindow->raise();	// make it top window
		for (KopeteMessageList::Iterator it = mMessageQueue.begin(); it != mMessageQueue.end(); it++)
		{
			kdDebug() << "[KopeteMessageManager] Inserting message from " << (*it).from()->name() << endl;
			mChatWindow->messageReceived((*it));
		}
		mChatWindow->show();	// show message window again
	}
	else {
		kdDebug() << "[KopeteMessageManager] Widget is non-oldschool: " << mWidget << endl;
	}
	
	mMessageQueue.clear();
}

void KopeteMessageManager::slotReadMessages()
{
	readMessages();
}

void KopeteMessageManager::messageSentFromWindow(const QString &message)
{
	QString body = message;
	KopeteMessage tmpmessage(mUser, mContactList, body, KopeteMessage::Outbound);
	if (mChatWindow->fg()->color().isValid()) {
		tmpmessage.setFg(mChatWindow->fg()->color());
	}
	if (mChatWindow->bg()->color().isValid()) {
		tmpmessage.setBg(mChatWindow->bg()->color());
	}
	emit messageSent(tmpmessage);
	if ( kopeteapp->appearance()->soundNotify() )
	    KNotifyClient::event("kopete_outgoing");
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
		kdDebug() << "[KopeteMessageManager] cancelUnreadMessageEvent Event Deleted" << endl;
	}
}

void KopeteMessageManager::slotEventDeleted(KopeteEvent *e)
{
	kdDebug() << "[KopeteMessageManager] Event done(), now 0L" << endl;	
	if ( e == mUnreadMessageEvent)
		mUnreadMessageEvent = 0L;
}



void KopeteMessageManager::appendMessage( const KopeteMessage &msg )
{
	mMessageQueue.append(msg);

	if( mLogger )
	{
		mLogger->append( msg );
	}

	/* First stage, see what to do */
	bool isvisible = false;
	if ( mChatWindow == 0L )
	{
		isvisible = false;
	}
	else if ( !mChatWindow->isVisible() )
    {
		isvisible = false;
	}
	else
	{
		isvisible = true;
	}

	if (mReadMode == Popup)
	{
    	readMessages();
	}
	else if ( mReadMode == Queued )
	{
		/* Second stage, do it */
		if ( isvisible )
		{
			readMessages();	
		}
		else
		{
			/* Create an event if a prevoius one not exist */
			if ( (mUnreadMessageEvent == 0L) && (msg.direction() == KopeteMessage::Inbound) )
			{
		 		mUnreadMessageEvent = new KopeteEvent( i18n("Message from %1").arg(msg.from()->name()), "kopete/pics/newmsg.png", this, SLOT(slotReadMessages()));
				connect(mUnreadMessageEvent, SIGNAL(done(KopeteEvent *)), this, SLOT(slotEventDeleted(KopeteEvent *)));
				kopeteapp->notifyEvent( mUnreadMessageEvent );
			}
		}
	}
	if ( kopeteapp->appearance()->soundNotify() && isvisible && (msg.direction() != KopeteMessage::Outbound) )
	    KNotifyClient::event("kopete_incoming");
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

void KopeteMessageManager::readModeChanged()
{
	if ( kopeteapp->appearance()->useQueue() )
	{
		mReadMode = Queued;
	}
	else
	{
		mReadMode = Popup;
	}
}

// vim: set noet ts=4 sts=4 sw=4:
#include "kopetemessagemanager.moc"
