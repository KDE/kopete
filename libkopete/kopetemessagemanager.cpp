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

struct KMMPrivate {
	KopeteContactPtrList  				 mContactList;
	const KopeteContact  				*mUser;
	KopeteChatWindow 				*mChatWindow;
	KopeteEmailWindow 				*mEmailWindow, *mEmailReplyWindow;
	KopeteEvent 					*mUnreadMessageEvent;
	KopeteMessageList 				 mMessageQueue;
	KopeteMessageLog 				*mLogger;
	int 						 mReadMode;
	KopeteMessageManager::WidgetType		 mWidget;
	QMap<const KopeteContact *, QStringList> 	 resources;
	KopeteProtocol 					*mProtocol;
	bool 						 mSendEnabled;
	int                                              mId;
};

KopeteMessageManager::KopeteMessageManager( const KopeteContact *user, KopeteContactPtrList others,
		KopeteProtocol *protocol, int id, QString logFile, enum WidgetType widget,
		QObject *parent, const char *name) : QObject( parent, name)
{
	d = new KMMPrivate;
	d->mSendEnabled = true;
	d->mContactList = others;
	d->mUser = user;
	d->mChatWindow = 0L;
	d->mEmailWindow = 0L;
	d->mEmailReplyWindow = 0L;
	d->mUnreadMessageEvent = 0L;
	d->mProtocol = protocol;
	d->mWidget = widget;
	d->mId = id;

	readModeChanged();
	connect( KopetePrefs::prefs(), SIGNAL(queueChanged()), this, SLOT(readModeChanged()));

	if (!logFile.isEmpty())	{
		QString logFileName = "kopete/" + logFile;
		d->mLogger = new KopeteMessageLog(logFileName, this);
	}
	else
		d->mLogger = 0L;
}

KopeteMessageManager::~KopeteMessageManager()
{
	emit dying(this);
	delete d;
	d = 0;
}

void KopeteMessageManager::slotSendEnabled(bool e)
{
	d->mSendEnabled = e;
	if (d->mWidget == ChatWindow) {
		if (d->mChatWindow)
			d->mChatWindow->setSendEnabled(e);
	}
	if (d->mWidget == Email) {
		if (d->mEmailWindow)
			d->mEmailWindow->setSendEnabled(e);
	}
}

void KopeteMessageManager::newChatWindow() {
	if (d->mWidget == ChatWindow) {
		d->mChatWindow = new KopeteChatWindow(d->mUser, d->mContactList);
		d->mChatWindow->setSendEnabled(d->mSendEnabled);

		/* When the window is shown, we have to delete this contact event */
		kdDebug() << "[KopeteMessageManager] Connecting message box shown() to event killer" << endl;
		connect (d->mChatWindow, SIGNAL(shown()), this, SLOT(slotCancelUnreadMessageEvent()));
		connect (d->mChatWindow, SIGNAL(sendMessage(KopeteMessage &)), this, SLOT(slotMessageSent(KopeteMessage &)));
		connect (d->mChatWindow, SIGNAL(closeClicked()), this, SLOT(slotChatWindowClosing()));

		connect (this, SIGNAL(contactAdded(const KopeteContact *)), d->mChatWindow, SLOT(slotContactAdded(const KopeteContact *)));
		connect (this, SIGNAL(contactRemoved(const KopeteContact *)), d->mChatWindow, SLOT(slotContactRemoved(const KopeteContact *)));
	}
	if (d->mWidget == Email) {
		d->mEmailWindow = new KopeteEmailWindow(d->mUser, d->mContactList);
		d->mEmailWindow->setSendEnabled(d->mSendEnabled);
		connect (d->mEmailWindow, SIGNAL(shown()), this, SLOT(slotCancelUnreadMessageEvent()));
		connect (d->mEmailWindow, SIGNAL(sendMessage(KopeteMessage &)),
			 this, SLOT(slotMessageSent(KopeteMessage &)));
		connect (d->mEmailWindow, SIGNAL(closeClicked()), this, SLOT(slotChatWindowClosing()));
		connect (d->mEmailWindow, SIGNAL(replyClicked()), this, SLOT(slotReply()));
	}
}

void KopeteMessageManager::newReplyWindow() {
	if (d->mWidget == Email) {
		kdDebug() << "[KopeteMessageManager] newReplyWindow() called for email-type window" << endl;
		d->mEmailReplyWindow = new KopeteEmailWindow(d->mUser, d->mContactList);
		d->mEmailReplyWindow->setSendEnabled(true);
		d->mEmailReplyWindow->setReplyMode(true);
		d->mEmailReplyWindow->show();
		d->mEmailReplyWindow->raise();
		connect (d->mEmailReplyWindow, SIGNAL(sendMessage(KopeteMessage &)),
			 this, SLOT(slotMessageSent(KopeteMessage &)));
		connect (d->mEmailReplyWindow, SIGNAL(closeClicked()),
			 this, SLOT(slotReplyWindowClosing()));
	}
}

void KopeteMessageManager::setReadMode(int mode)
{
	if ((mode == Queued) || (mode == Popup))
	{
		d->mReadMode = mode;
	}
	else
	{
		kdDebug() << "[KopeteMessageManager] ERROR: unknown reading method, setting to default" << endl;
		d->mReadMode = Queued;
	}
}

int KopeteMessageManager::readMode() const
{
	return d->mReadMode;
}

KopeteMessageManager::WidgetType KopeteMessageManager::widget() const
{
	return d->mWidget;
}

void KopeteMessageManager::readMessages()
{
	if (d->mWidget == ChatWindow) {
		if (d->mChatWindow == 0L) {
			kdDebug() << "[KopeteMessageManager] mChatWindow just doesn't exist" << endl;
			newChatWindow();
		}
		if (d->mChatWindow->isMinimized())
			kdDebug() << "[KopeteMessageManager] mChatWindow is minimized" << endl;
		if (d->mChatWindow->isHidden())
			kdDebug() << "[KopeteMessageManager] mChatWindow is hidden" << endl;
		if(KopetePrefs::prefs()->raiseMsgWindow())
			d->mChatWindow->raise();	// make it top window
		for (KopeteMessageList::Iterator it = d->mMessageQueue.begin(); it != d->mMessageQueue.end(); it++)
		{
			kdDebug() << "[KopeteMessageManager] Inserting message from " << (*it).from()->displayName() << endl;
			emit messageReceived( *it );
			d->mChatWindow->messageReceived(*it);
		}
		d->mChatWindow->show();	// show message window again
	}
	else if (d->mWidget == Email) {
		if (d->mEmailWindow == 0L) {
			kdDebug() << "[KopeteMessageManager] mEmailWindow just doesn't exist" << endl;
			newChatWindow();
		}
		if (d->mEmailWindow->isMinimized())
			kdDebug() << "[KopeteMessageManager] mEmailWindow is minimized" << endl;
		if (d->mEmailWindow->isHidden())
			kdDebug() << "[KopeteMessageManager] mEmailWindow is hidden" << endl;
		if(KopetePrefs::prefs()->raiseMsgWindow())
			d->mEmailWindow->raise();
		for (KopeteMessageList::Iterator it = d->mMessageQueue.begin(); it != d->mMessageQueue.end(); it++) {
			kdDebug() << "[KopeteMessageManager] Inserting message from " << (*it).from()->displayName() << endl;
			emit messageReceived( *it );
			d->mEmailWindow->messageReceived(*it);
		}
		d->mEmailWindow->show();
	}
	else {
		kdDebug() << "[KopeteMessageManager] Widget is non-oldschool: " << d->mWidget << endl;
	}

	d->mMessageQueue.clear();
}

const KopeteContactPtrList& KopeteMessageManager::members() const
{
	return d->mContactList;
}

const KopeteContact* KopeteMessageManager::user() const
{
	return d->mUser;
}

const KopeteProtocol* KopeteMessageManager::protocol() const
{
	return d->mProtocol;
}

int KopeteMessageManager::id() const
{
	return d->mId;
}

void KopeteMessageManager::setID( int id )
{
	d->mId = id;
}

void KopeteMessageManager::slotReadMessages() {
	readMessages();
}

void KopeteMessageManager::slotReply() {
	kdDebug() << "[KopeteMessageManager] slotReply() called." << endl;
	if (d->mEmailReplyWindow == 0L) {
		/* PLTHARG! */
		kdDebug() << "[KopeteMessageManager] mEmailReplyWindow == 0L, calling nRW()" << endl;
		newReplyWindow();
	}
	else {
		kdDebug() << "[KopeteMessageManager] mEmailWindow != 0L, not starting a new one (duh)." << endl;
	}
}

void KopeteMessageManager::slotMessageSent(KopeteMessage &message) {

	emit messageQueued( message );
	emit messageSent(message, this);
	if ( KopetePrefs::prefs()->soundNotify() )
	    KNotifyClient::event("kopete_outgoing");
}

void KopeteMessageManager::slotChatWindowClosing() {
	if (d->mWidget == ChatWindow) {
		kdDebug() << "[KopeteMessageManager] Chat Window closed, now 0L" << endl;
		d->mChatWindow = 0L;
	}
	else if (d->mWidget == Email) {
		kdDebug() << "[KopeteMessageManager] Email Window closed, now 0L." << endl;
		delete d->mEmailWindow;
		d->mEmailWindow = 0L;
	}
}

void KopeteMessageManager::slotReplyWindowClosing() {
	if (d->mWidget == Email) {
		delete d->mEmailReplyWindow;
		d->mEmailReplyWindow = 0L;
	}
}

void KopeteMessageManager::slotCancelUnreadMessageEvent() {
	if (d->mUnreadMessageEvent == 0L)
	{
		kdDebug() << "[KopeteMessageManager] No event to delete" << endl;
	}
	else
	{
		kdDebug() << "[KopeteMessageManager] cancelUnreadMessageEvent Deleting Event" << endl;
		delete d->mUnreadMessageEvent;
		d->mUnreadMessageEvent = 0L;
		kdDebug() << "[KopeteMessageManager] cancelUnreadMessageEvent Event Deleted" << endl;
	}
}

void KopeteMessageManager::slotEventDeleted(KopeteEvent *e) {
	kdDebug() << "[KopeteMessageManager] Event done(), now 0L" << endl;
	if ( e == d->mUnreadMessageEvent)
		d->mUnreadMessageEvent = 0L;
}



void KopeteMessageManager::appendMessage( const KopeteMessage &msg ) {
	d->mMessageQueue.append(msg);

	if( d->mLogger )
	{
		d->mLogger->append( msg );
	}

	/* First stage, see what to do */
	bool isvisible = false;

	if (d->mWidget == ChatWindow) {
		if (d->mChatWindow == 0L)
			newChatWindow();
		else if (!d->mChatWindow->isVisible())
			isvisible = false;
		else
			isvisible = true;
	}
	else if (d->mWidget == Email) {
		if (d->mEmailWindow == 0L)
			newChatWindow();
		else if (!d->mEmailWindow->isVisible())
			isvisible = false;
		else
			isvisible = true;
	}

	if (d->mReadMode == Popup)
		readMessages();
	else if (d->mReadMode == Queued) {
		/* Second stage, do it */
		if (isvisible) {
			readMessages();
		}
		else { /* Bug, WHOOHOO! If a window's on another desktop, we queue regardless. Grrr. */
			/* Create an event if a prevoius one not exist */
			if ((d->mUnreadMessageEvent == 0L) && (msg.direction() == KopeteMessage::Inbound)) {
		 		d->mUnreadMessageEvent = new KopeteEvent( i18n("Message from %1").arg(msg.from()->displayName()),
									       "kopete/pics/newmsg.png", this, SLOT(slotReadMessages()));
				connect(d->mUnreadMessageEvent, SIGNAL(done(KopeteEvent *)),
					this, SLOT(slotEventDeleted(KopeteEvent *)));
				kopeteapp->notifyEvent( d->mUnreadMessageEvent );
			}
		}
	}
	if ( KopetePrefs::prefs()->soundNotify() && isvisible && (msg.direction() != KopeteMessage::Outbound) )
	    KNotifyClient::event("kopete_incoming");
}
void KopeteMessageManager::addContact( const KopeteContact *c )
{
	kdDebug() << "KopeteMessageManager::addContact" <<endl;
	for ( KopeteContact *tmp = d->mContactList.first(); tmp; tmp = d->mContactList.next() )
	{
		if ( tmp == c )
		{
			kdDebug() << "[KopeteMessageManager] Contact already exists" <<endl;
			return;
		}
	}

	kdDebug() << "[KopeteMessageManager] Contact Joined session" <<endl;
	d->mContactList.append(c);
	emit contactAdded(c);
}

void KopeteMessageManager::removeContact( const KopeteContact *c )
{
	if(d->mContactList.count()==1)
	{
		kdDebug() << "KopeteMessageManager::removeContact - Contact not removed. Keep always one contact" <<endl;
	}
	else
	{
		d->mContactList.take( d->mContactList.find(c) );
		emit contactRemoved(c);
	}
}

void KopeteMessageManager::readModeChanged()
{
	if ( KopetePrefs::prefs()->useQueue() )
	{
		d->mReadMode = Queued;
	}
	else
	{
		d->mReadMode = Popup;
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

