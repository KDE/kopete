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
#include "kopeteprefs.h"

#include "kopetemessagelog.h"

#include <kcolorbutton.h>
#include <kdebug.h>
#include <klocale.h>
#include <knotifyclient.h>

#include <qstylesheet.h>

KopeteMessageManager::KopeteMessageManager( const KopeteContact *user, KopeteContactPtrList others,
		KopeteProtocol *protocol, QString logFile, enum WidgetType widget,
		QObject *parent, const char *name) : QObject( parent, name)
{
    mSendEnabled = true;
	mContactList = others;
	mUser = user;
	mChatWindow = 0L;
	mEmailWindow = 0L;
	mEmailReplyWindow = 0L;
	mUnreadMessageEvent = 0L;
	mProtocol = protocol;
	mWidget = widget;
	
	readModeChanged();
	connect( KopetePrefs::prefs(), SIGNAL(queueChanged()), this, SLOT(readModeChanged()));
	
	if (!logFile.isEmpty())	{
		QString logFileName = "kopete/" + logFile;
		mLogger = new KopeteMessageLog(logFileName, this);
	}
	else
		mLogger = 0L;
}

KopeteMessageManager::~KopeteMessageManager()
{
	emit dying(this);
}

void KopeteMessageManager::slotSendEnabled(bool e)
{
	mSendEnabled = e;
	if (mWidget == ChatWindow) {
		if (mChatWindow)
			mChatWindow->setSendEnabled(e);
	}
	if (mWidget == Email) {
		if (mEmailWindow)
			mEmailWindow->setSendEnabled(e);
	}
}

void KopeteMessageManager::newChatWindow() {
	if (mWidget == ChatWindow) {
		mChatWindow = new KopeteChatWindow(mUser, mContactList);
		mChatWindow->setSendEnabled(mSendEnabled);	

		/* When the window is shown, we have to delete this contact event */
		kdDebug() << "[KopeteMessageManager] Connecting message box shown() to event killer" << endl;
		connect (mChatWindow, SIGNAL(shown()), this, SLOT(slotCancelUnreadMessageEvent()));
		connect (mChatWindow, SIGNAL(sendMessage(const KopeteMessage &)), this, SLOT(slotMessageSent(const KopeteMessage &)));
		connect (mChatWindow, SIGNAL(closeClicked()), this, SLOT(slotChatWindowClosing()));
	}
	if (mWidget == Email) {
		mEmailWindow = new KopeteEmailWindow(mUser, mContactList);
		mEmailWindow->setSendEnabled(mSendEnabled);
		connect (mEmailWindow, SIGNAL(shown()), this, SLOT(slotCancelUnreadMessageEvent()));
		connect (mEmailWindow, SIGNAL(sendMessage(const KopeteMessage &)), this, SLOT(slotMessageSent(const KopeteMessage &)));
		connect (mEmailWindow, SIGNAL(closeClicked()), this, SLOT(slotChatWindowClosing()));
		connect (mEmailWindow, SIGNAL(replyClicked()), this, SLOT(slotReply()));
	}
}

void KopeteMessageManager::newReplyWindow() {
	if (mWidget == Email) {
		kdDebug() << "[KopeteMessageManager] newReplyWindow() called for email-type window" << endl;
		mEmailReplyWindow = new KopeteEmailWindow(mUser, mContactList);
		mEmailReplyWindow->setSendEnabled(true);
		mEmailReplyWindow->setReplyMode(true);
		mEmailReplyWindow->show();
		mEmailReplyWindow->raise();
		connect (mEmailReplyWindow, SIGNAL(sendMessage(const KopeteMessage &)), this, SLOT(slotMessageSent(const KopeteMessage &)));
		connect (mEmailReplyWindow, SIGNAL(closeClicked()), this, SLOT(slotReplyWindowClosing()));
	}
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
		if(KopetePrefs::prefs()->raiseMsgWindow())
			mChatWindow->raise();	// make it top window
		for (KopeteMessageList::Iterator it = mMessageQueue.begin(); it != mMessageQueue.end(); it++)
		{
			kdDebug() << "[KopeteMessageManager] Inserting message from " << (*it).from()->displayName() << endl;
			mChatWindow->messageReceived(*it);
		}
		mChatWindow->show();	// show message window again
	}
	else if (mWidget == Email) {
		if (mEmailWindow == 0L) {
			kdDebug() << "[KopeteMessageManager] mEmailWindow just doesn't exist" << endl;
			newChatWindow();
		}
		if (mEmailWindow->isMinimized())
			kdDebug() << "[KopeteMessageManager] mEmailWindow is minimized" << endl;
		if (mEmailWindow->isHidden())
			kdDebug() << "[KopeteMessageManager] mEmailWindow is hidden" << endl;
		if(KopetePrefs::prefs()->raiseMsgWindow())
			mEmailWindow->raise();
		for (KopeteMessageList::Iterator it = mMessageQueue.begin(); it != mMessageQueue.end(); it++) {
			kdDebug() << "[KopeteMessageManager] Inserting message from " << (*it).from()->displayName() << endl;
			mEmailWindow->messageReceived(*it);
		}
		mEmailWindow->show();
	}
	else {
		kdDebug() << "[KopeteMessageManager] Widget is non-oldschool: " << mWidget << endl;
	}
	
	mMessageQueue.clear();
}

void KopeteMessageManager::slotReadMessages() {
	readMessages();
}

void KopeteMessageManager::slotReply() {
	kdDebug() << "[KopeteMessageManager] slotReply() called." << endl;
	if (mEmailReplyWindow == 0L) {
		/* PLTHARG! */
		kdDebug() << "[KopeteMessageManager] mEmailReplyWindow == 0L, calling nRW()" << endl;
		newReplyWindow();
	}
	else {
		kdDebug() << "[KopeteMessageManager] mEmailWindow != 0L, not starting a new one (duh)." << endl;
	}
}

void KopeteMessageManager::slotMessageSent(const KopeteMessage &message) {
	emit messageSent(message);
	if ( KopetePrefs::prefs()->soundNotify() )
	    KNotifyClient::event("kopete_outgoing");
}

void KopeteMessageManager::slotChatWindowClosing() {
	if (mWidget == ChatWindow) {
		kdDebug() << "[KopeteMessageManager] Chat Window closed, now 0L" << endl;	
		mChatWindow = 0L;
	}
	else if (mWidget == Email) {
		kdDebug() << "[KopeteMessageManager] Email Window closed, now 0L." << endl;
		delete mEmailWindow;
		mEmailWindow = 0L;
	}
}

void KopeteMessageManager::slotReplyWindowClosing() {
	if (mWidget == Email) {
		delete mEmailReplyWindow;
		mEmailReplyWindow = 0L;
	}
}

void KopeteMessageManager::slotCancelUnreadMessageEvent() {
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

void KopeteMessageManager::slotEventDeleted(KopeteEvent *e) {
	kdDebug() << "[KopeteMessageManager] Event done(), now 0L" << endl;	
	if ( e == mUnreadMessageEvent)
		mUnreadMessageEvent = 0L;
}



void KopeteMessageManager::appendMessage( const KopeteMessage &msg ) {
	mMessageQueue.append(msg);

	if( mLogger )
	{
		mLogger->append( msg );
	}

	/* First stage, see what to do */
	bool isvisible = false;

	if (mWidget == ChatWindow) {
		if (mChatWindow == 0L)
			newChatWindow();
		else if (!mChatWindow->isVisible())
			isvisible = false;
		else
			isvisible = true;
	}
	else if (mWidget == Email) {
		if (mEmailWindow == 0L)
			newChatWindow();
		else if (!mEmailWindow->isVisible())
			isvisible = false;
		else
			isvisible = true;
	}

	if (mReadMode == Popup)
    	readMessages();

	else if (mReadMode == Queued) {
		/* Second stage, do it */
		if (isvisible) {
			readMessages();	
		}
		else { /* Bug, WHOOHOO! If a window's on another desktop, we queue regardless. Grrr. */
			/* Create an event if a prevoius one not exist */
			if ((mUnreadMessageEvent == 0L) && (msg.direction() == KopeteMessage::Inbound)) {
		 		mUnreadMessageEvent = new KopeteEvent( i18n("Message from %1").arg(msg.from()->displayName()), "kopete/pics/newmsg.png", this, SLOT(slotReadMessages()));
				connect(mUnreadMessageEvent, SIGNAL(done(KopeteEvent *)), this, SLOT(slotEventDeleted(KopeteEvent *)));
				kopeteapp->notifyEvent( mUnreadMessageEvent );
			}
		}
	}
	if ( KopetePrefs::prefs()->soundNotify() && isvisible && (msg.direction() != KopeteMessage::Outbound) )
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
	if ( KopetePrefs::prefs()->useQueue() )
	{
		mReadMode = Queued;
	}
	else
	{
		mReadMode = Popup;
	}
}


#include "kopetemessagemanager.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

