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

#include "kopeteaway.h"
#include "kopetechatwindow.h"
#include "kopeteemailwindow.h"
#include "kopeteevent.h"
#include "kopetemessagelog.h"
#include "kopetemessagemanager.h"
#include "kopetenotifier.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"

#include "kopetemetacontact.h"

#include <qregexp.h>

#include <kcolorbutton.h>
#include <kdebug.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kwin.h>

#include <qstylesheet.h>

struct KMMPrivate
{
	KopeteContactPtrList mContactList;
	const KopeteContact *mUser;
	KopeteChatWindow *mChatWindow;
	KopeteEmailWindow *mEmailWindow, *mEmailReplyWindow;
	KopeteEvent	*mUnreadMessageEvent;
	KopeteMessageList mMessageQueue;
	KopeteMessageLog *mLogger;
	int mReadMode;
	KopeteMessageManager::WidgetType mWidget;
	QMap<const KopeteContact *, QStringList> resources;
	KopeteProtocol *mProtocol;
	bool mSendEnabled;
	int mId;
	bool mLog;
	bool isEmpty;
	bool mCanBeDeleted;


	//say if ye're currently reading message, and don't accept new message during this time
	bool isBusy;
};

KopeteMessageManager::KopeteMessageManager( const KopeteContact *user,
	KopeteContactPtrList others, KopeteProtocol *protocol, int id,
	enum WidgetType widget, QObject *parent, const char *name )
: QObject( parent, name )
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
	d->mLog = true;
	d->isEmpty= others.isEmpty();
	d->mCanBeDeleted= false;
	d->isBusy=false;

	readModeChanged();
	connect( KopetePrefs::prefs(), SIGNAL(queueChanged()), this, SLOT(readModeChanged()));

	// Replace '.', '/' and '~' in the user id with '-' to avoid possible
	// directory traversal, although appending '.log' and the rest of the
	// code should really make overwriting files possible anyway.
	KopeteContact *c = others.first();
	QString logFileName = "kopete/" + QString( c->protocol()->pluginId() ) +
		"/" + c->contactId().replace( QRegExp( "[./~]" ), "-" ) + ".log";
	d->mLogger = new KopeteMessageLog( logFileName, this );

//	connect(protocol, SIGNAL(destroyed()), this, SLOT(slotProtocolUnloading()));
}

KopeteMessageManager::~KopeteMessageManager()
{
	kdDebug(14010) << "KopeteMessageManager::~KopeteMessageManager" <<endl;
	d->mCanBeDeleted=false; //prevent double deletion
	emit dying(this);
	delete widget();
	delete d;
	d = 0;
}

void KopeteMessageManager::slotSendEnabled(bool e)
{
	d->mSendEnabled = e;
	if (d->mWidget == ChatWindow)
	{
		if (d->mChatWindow)
			d->mChatWindow->setSendEnabled(e);
	}
	if (d->mWidget == Email)
	{
		if (d->mEmailWindow)
			d->mEmailWindow->setSendEnabled(e);
	}
}

void KopeteMessageManager::setLogging( bool on )
{
	d->mLog = on;
}

bool KopeteMessageManager::logging() const
{
	return d->mLog;
}

void KopeteMessageManager::newChatWindow()
{
	if (d->mWidget == ChatWindow)
	{
		d->mChatWindow = new KopeteChatWindow(this);
		d->mChatWindow->setSendEnabled(d->mSendEnabled);

		/* When the window is shown, we have to delete this contact event */
//		kdDebug(14010) << "[KopeteMessageManager] Connecting message box shown() to event killer" << endl;
		connect (d->mChatWindow, SIGNAL(shown()), this, SLOT(slotCancelUnreadMessageEvent()));
		connect (d->mChatWindow, SIGNAL(sendMessage(KopeteMessage &)), this, SLOT(slotMessageSent(KopeteMessage &)));
		connect (d->mChatWindow, SIGNAL(closeClicked()), this, SLOT(slotChatWindowClosing()));

		connect (this, SIGNAL(contactAdded(const KopeteContact *)), d->mChatWindow, SLOT(slotContactAdded(const KopeteContact *)));
		connect (this, SIGNAL(contactRemoved(const KopeteContact *)), d->mChatWindow, SLOT(slotContactRemoved(const KopeteContact *)));
		connect (d->mChatWindow, SIGNAL(TypingMessage(bool)), this, SLOT(slotTyping(bool)));

	}
	if (d->mWidget == Email)
	{
		d->mEmailWindow = new KopeteEmailWindow(d->mUser, d->mContactList);
		d->mEmailWindow->setSendEnabled(d->mSendEnabled);
		connect (d->mEmailWindow, SIGNAL(shown()), this, SLOT(slotCancelUnreadMessageEvent()));
		connect (d->mEmailWindow, SIGNAL(sendMessage(KopeteMessage &)),
			 this, SLOT(slotMessageSent(KopeteMessage &)));
		connect (d->mEmailWindow, SIGNAL(closeClicked()), this, SLOT(slotChatWindowClosing()));
		connect (d->mEmailWindow, SIGNAL(replyClicked()), this, SLOT(slotReply()));
	}
}

void KopeteMessageManager::newReplyWindow()
{
	if (d->mWidget == Email)
	{
		kdDebug(14010) << "[KopeteMessageManager] newReplyWindow() called for email-type window" << endl;
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
		kdDebug(14010) << "[KopeteMessageManager] ERROR: unknown reading method, setting to default" << endl;
		d->mReadMode = Queued;
	}
}

int KopeteMessageManager::readMode() const
{
	return d->mReadMode;
}

KopeteMessageManager::WidgetType KopeteMessageManager::widgetType() const
{
	return d->mWidget;
}

QWidget *KopeteMessageManager::widget()  const
{
	if (d->mWidget == ChatWindow)
		return d->mChatWindow;
	if(d->mWidget == Email)
		return d->mEmailWindow;
	return 0L;
}


bool KopeteMessageManager::emptyMessageBuffer()
{
	if (!widget() )
	{
		kdDebug(14010) << "KopeteMessageManager::emptyMessageBuffer: mChatWindow just doesn't exist" << endl;
		newChatWindow();
	}

	bool foreignMessage = false;
	for (KopeteMessageList::Iterator it = d->mMessageQueue.begin(); it != d->mMessageQueue.end(); it = d->mMessageQueue.begin())
	{
//		kdDebug(14010) << "KopeteMessageManager::emptyMessageBuffer: Inserting message from " << (*it).from()->displayName() << endl;
		if ( (*it).from() != d->mUser )
			foreignMessage = true;

		emit messageReceived( *it );
		if ( d->mWidget == ChatWindow ) // ### why don't they implement the same interface?
			d->mChatWindow->messageReceived(*it);
		else if ( d->mWidget == Email )
			d->mEmailWindow->messageReceived(*it);

		d->mMessageQueue.remove(it);
	}
	d->mMessageQueue.clear();
	return foreignMessage;
}

void KopeteMessageManager::readMessages()
{
	if(d->isBusy)
	{
		kdDebug(14010) << "KopeteMessageManager::readMessages: Busy! A plugin is working on a precedent message" << endl;
		return;
	}

	if (!widget())
	{
		kdDebug(14010) << "KopeteMessageManager::readMessages: mChatWindow just doesn't exist" << endl;
		newChatWindow();
	}
	QWidget *window = widget();
	if ( !window )
	{
		kdDebug(14010) << "KopeteMessageManager::readMessages: WARNING: cannot get the ChatWindow"  << endl;
		d->mMessageQueue.clear();
		return;
	}

/*	if (window->isMinimized())
		kdDebug(14010) << "[KopeteMessageManager] window is minimized" << endl;
	if (window->isHidden())
		kdDebug(14010) << "[KopeteMessageManager] window is hidden" << endl;*/

	d->isBusy=true;
	bool queueEmpty = d->mMessageQueue.isEmpty();
	bool foreignMessage = emptyMessageBuffer();

	// only show the window when a message from someone else (i.e. not an own message) arrived or
	// when no message at all arrived (happens when you click on a contact, creating the window)
	if ( foreignMessage || queueEmpty )
	{
		if(KopetePrefs::prefs()->raiseMsgWindow())
		{
			KWin::setOnDesktop(window->winId() , KWin::currentDesktop()); //set on the desktop
			window->raise(); // make it top window
		}
		window->show();	// show message window again (but not for own messages)
	}
	d->isBusy=false;
}

const KopeteContactPtrList& KopeteMessageManager::members() const
{
	return d->mContactList;
}

const KopeteContact* KopeteMessageManager::user() const
{
	return d->mUser;
}

KopeteProtocol* KopeteMessageManager::protocol() const
{
	return d->mProtocol;
}

int KopeteMessageManager::mmId() const
{
	return d->mId;
}

void KopeteMessageManager::setMMId( int id )
{
	d->mId = id;
}

void KopeteMessageManager::slotReadMessages()
{
	readMessages();
}

void KopeteMessageManager::slotReply()
{
	kdDebug(14010) << "[KopeteMessageManager] slotReply() called." << endl;
	if (d->mEmailReplyWindow == 0L)
	{
		/* PLTHARG! */
		kdDebug(14010) << "[KopeteMessageManager] mEmailReplyWindow == 0L, calling nRW()" << endl;
		newReplyWindow();
	}
	else
	{
		kdDebug(14010) << "[KopeteMessageManager] mEmailWindow != 0L, not starting a new one (duh)." << endl;
	}
}

void KopeteMessageManager::slotMessageSent(KopeteMessage &message)
{

	emit messageQueued( message );
	emit messageSent(message, this);

	if ( KopetePrefs::prefs()->soundNotify() )
	{
		if ( !protocol()->isAway() || KopetePrefs::prefs()->soundIfAway() )
		    KNotifyClient::event("kopete_outgoing");
	}
}

void KopeteMessageManager::slotChatWindowClosing()
{
	if (d->mWidget == ChatWindow)
	{
		kdDebug(14010) << "KopeteMessageManager::slotChatWindowClosing : Chat Window closed, now 0L" << endl;
		d->mChatWindow = 0L;
	}
	else if (d->mWidget == Email)
	{
		kdDebug(14010) << "KopeteMessageManager::slotChatWindowClosing : Email Window closed, now 0L." << endl;
		delete d->mEmailWindow;
		d->mEmailWindow = 0L;
	}
	if(d->mCanBeDeleted)
	{
		kdDebug(14010) << "KopeteMessageManager::slotChatWindowClosing : delete KMM" << endl;
		deleteLater();
	}
}

void KopeteMessageManager::slotReplyWindowClosing()
{
	if (d->mWidget == Email)
	{
		delete d->mEmailReplyWindow;
		d->mEmailReplyWindow = 0L;
	}
}

void KopeteMessageManager::slotCancelUnreadMessageEvent()
{
	if (d->mUnreadMessageEvent == 0L)
	{
		kdDebug(14010) << "[KopeteMessageManager] No event to delete" << endl;
	}
	else
	{
		kdDebug(14010) << "[KopeteMessageManager] cancelUnreadMessageEvent Deleting Event" << endl;
		delete d->mUnreadMessageEvent;
		d->mUnreadMessageEvent = 0L;
		kdDebug(14010) << "[KopeteMessageManager] cancelUnreadMessageEvent Event Deleted" << endl;
	}
	emptyMessageBuffer();
}

void KopeteMessageManager::slotEventDeleted(KopeteEvent *e)
{
	if ( e == d->mUnreadMessageEvent)
	{
		kdDebug(14010) << "[KopeteMessageManager] Event done(), now 0L" << endl;
		d->mUnreadMessageEvent = 0L;
	}
}

void KopeteMessageManager::appendMessage( const KopeteMessage &msg )
{
	d->mMessageQueue.append(msg);

	if( d->mLogger && d->mLog )
		d->mLogger->append( msg );

	// First stage, see what to do
	bool isvisible = false;

	if (!widget())
		newChatWindow();
	else
		isvisible = widget()->isVisible();


	if (d->mReadMode == Popup)
	{
		readMessages();
	}
	else if (d->mReadMode == Queued)
	{
		// Second stage, do it
		if (isvisible)
		{
			readMessages();
		}
		else
		{
			// Create an event if a previous one does not exist
			if ((d->mUnreadMessageEvent == 0L) && (msg.direction() == KopeteMessage::Inbound))
			{
				if (msg.from()->metaContact())
				{
					d->mUnreadMessageEvent = new KopeteEvent( i18n("Message from %1").arg(msg.from()->metaContact()->displayName()),
						"kopete/pics/newmsg.png", this, SLOT(slotReadMessages()));
				}
				else
				{
					d->mUnreadMessageEvent = new KopeteEvent( i18n("Message from %1").arg(msg.from()->displayName()),
						"kopete/pics/newmsg.png", this, SLOT(slotReadMessages()));
				}
				connect(d->mUnreadMessageEvent, SIGNAL(done(KopeteEvent *)),
					this, SLOT(slotEventDeleted(KopeteEvent *)));
				KopeteNotifier::notifier()->notifyEvent( d->mUnreadMessageEvent );
			}
		}
	}

	if ( KopetePrefs::prefs()->soundNotify() && (isvisible || d->mReadMode == Popup) && (msg.direction() != KopeteMessage::Outbound) )
	{
		if ( !protocol()->isAway() || KopetePrefs::prefs()->soundIfAway() )
		    KNotifyClient::event("kopete_incoming");
	}
}

void KopeteMessageManager::addContact( const KopeteContact *c )
{
	if ( d->mContactList.contains(c) )
	{
		kdDebug(14010) << "KopeteMessageManager::addContact: Contact already exists" <<endl;
		emit contactAdded(c);
	}
	else
	{
		if(d->mContactList.count()==1 && d->isEmpty)
		{
			KopeteContact *old=d->mContactList.first();
			kdDebug(14010) << "KopeteMessageManager::addContact: " <<old->displayName() << " left and " << c->displayName() << " joined " <<endl;
			d->mContactList.remove(old);
			d->mContactList.append(c);
			emit contactAdded(c);
			emit contactRemoved(old);
		}
		else
		{
			kdDebug(14010) << "KopeteMessageManager::addContact: Contact Joined session : " <<c->displayName() <<endl;
			d->mContactList.append(c);
			emit contactAdded(c);
		}
	}
	d->isEmpty=false;
}

void KopeteMessageManager::removeContact( const KopeteContact *c )
{
	if(!c || !d->mContactList.contains(c))
		return;

	if(d->mContactList.count()==1)
	{
		kdDebug(14010) << "KopeteMessageManager::removeContact - Contact not removed. Keep always one contact" <<endl;
		d->isEmpty=true;
	}
	else
	{
		d->mContactList.remove( c );
	}
	emit contactRemoved(c);
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

void KopeteMessageManager::userTypingMsg ( const KopeteContact *c , bool t )
{
	if (d->mWidget == ChatWindow)
	{
		if (d->mChatWindow)
		{
			d->mChatWindow->slotAnyTyping( c , t );
		}
	}
}

void KopeteMessageManager::slotTyping ( bool t )
{
//	kdDebug(14010) << "KopeteMessageManager::slotTyping "<< t << endl;
	emit typingMsg(t);
}

void KopeteMessageManager::setCanBeDeleted ( bool b )
{
	d->mCanBeDeleted =b;
	if(b && !widget())
		deleteLater();
}

KopeteMessage KopeteMessageManager::currentMessage()
{
	if (d->mWidget == ChatWindow)
	{
		if (d->mChatWindow)
			return d->mChatWindow->currentMessage();
	}
	kdDebug(14010) << "KopeteMessageManager::currentMessage(); ChatWindow does not exist!" <<endl;
	return KopeteMessage();
}

void KopeteMessageManager::setCurrentMessage(const KopeteMessage& t)
{
	if (d->mWidget == ChatWindow)
	{
		if (d->mChatWindow)
			d->mChatWindow->setCurrentMessage(t);
	}
}

#include "kopetemessagemanager.moc"

// vim: set noet ts=4 sts=4 sw=4:
