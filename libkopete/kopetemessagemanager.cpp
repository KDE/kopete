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

#include <kcolorbutton.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdockwidget.h>
#include <knotifyclient.h>
#include <kwin.h>
#include <qstylesheet.h>
#include <qregexp.h>
#include <qmap.h>

#include "kopetechatwindow.h"
#include "kopeteaway.h"
#include "kopeteemailwindow.h"
#include "kopeteevent.h"
#include "kopetemessagelog.h"
#include "kopetemessagemanager.h"
#include "kopetenotifier.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "kopetemetacontact.h"
#include "chatview.h"

#define NEW_WINDOW 0
#define GROUP_BY_PROTOCOL 1
#define GROUP_ALL 2

struct KMMPrivate
{
	KopeteContactPtrList mContactList;
	const KopeteContact *mUser;
	KopeteEmailWindow *mEmailWindow, *mEmailReplyWindow;
	KopeteEvent *mUnreadMessageEvent;
	KopeteMessageList mMessageQueue;
	KopeteMessageLog *mLogger;
	ChatView	*mView;
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
	d->mEmailWindow = 0L;
	d->mView = 0L;
	d->mEmailReplyWindow = 0L;
	d->mUnreadMessageEvent = 0L;
	d->mProtocol = protocol;
	d->mWidget = widget;
	d->mId = id;
	d->mLog = true;
	d->isEmpty= others.isEmpty();
	d->mCanBeDeleted= false;
	d->isBusy=false;
	myWindow = 0L;
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
	kdDebug(14010) << k_funcinfo <<endl;
	d->mCanBeDeleted=false; //prevent double deletion
	emit dying(this);
	delete widget();
	delete d;
	d = 0;
}

void KopeteMessageManager::slotSendEnabled(bool e)
{
	d->mSendEnabled = e;
	if (d->mWidget == Email)
	{
		if (d->mEmailWindow)
			d->mEmailWindow->setSendEnabled(e);
	}
}

void KopeteMessageManager::setChatView( Kopete::ChatView *newView )
{
	d->mView = newView;
	myWindow = newView->mainWindow();
}

void KopeteMessageManager::setLogging( bool on )
{
	d->mLog = on;
}

bool KopeteMessageManager::logging() const
{
	return d->mLog;
}

const QString KopeteMessageManager::chatName()
{
	QString chatName, nextDisplayName;

	KopeteContact *c = d->mContactList.first();
	if( c->metaContact() )
		chatName = c->metaContact()->displayName();
	else
		chatName = c->displayName();

	while( ( c = d->mContactList.next() ) )
	{
		if( c->metaContact() )
			nextDisplayName = c->metaContact()->displayName();
		else
			nextDisplayName = c->displayName();
		chatName.append(", ").append( nextDisplayName );
	}

	return chatName;
}

/*
 * This method returns the static chatWindowMap pointer for the protocol<->window mapping
 */
ChatWindowMap *KopeteMessageManager::chatWindowMap()
{
	static ChatWindowMap *m_chatWindowMap = new ChatWindowMap();
	return m_chatWindowMap;
}

/*
 * This method returns the window pointer for our policy type
 */
KopeteChatWindow *KopeteMessageManager::newWindow()
{
	bool windowCreated = false;
	
	//Determine tabbed window settings
	switch( KopetePrefs::prefs()->chatWindowPolicy() )
	{
		case NEW_WINDOW: //Open every chat in a new window
			kdDebug(14010) << k_funcinfo << "Always open new window" << endl;
			
			//Always create new window
			myWindow = new KopeteChatWindow();
			connect (myWindow, SIGNAL(Closing()), this, SLOT(slotChatWindowClosing()));
			windowCreated = true;
			
			break;

		case GROUP_BY_PROTOCOL: //Open chats in the same protocol in the same window
			kdDebug(14010) << k_funcinfo << "Group by protocol" << endl;
			
			//Check if we have a window for this protocol
			if( chatWindowMap()->contains( d->mProtocol ) )
			{
				ChatWindowMap windowMap = *(chatWindowMap());
				myWindow = windowMap[d->mProtocol];
			}
			else
			{
				//A window for this protocol does not exist, create new window
				myWindow = new KopeteChatWindow();
				connect (myWindow, SIGNAL(Closing()), this, SLOT(slotChatWindowClosing()));
				windowCreated = true;
			}
			break;

		case GROUP_ALL: //Open all chats in the same window
			kdDebug(14010) << k_funcinfo << "Group all" << endl;
			kdDebug(14010) << k_funcinfo << "chatWindowMap contains " << chatWindowMap()->count() << endl;
			
			//Check if a window exists
			if( chatWindowMap()->isEmpty() )
			{
				//No window exists, create a new window
				myWindow = new KopeteChatWindow();
				connect (myWindow, SIGNAL(Closing()), this, SLOT(slotChatWindowClosing()));
				windowCreated = true;
			}
			else
			{
				//A window does exist. Just use the first one on our map.
				ChatWindowMap::Iterator it = chatWindowMap()->begin();
				myWindow = it.data();
			}
			break;
	}
	
	//Add this protocol to the map no matter what the preference, in case it is switched while windows are open
	if( windowCreated && !chatWindowMap()->contains( d->mProtocol ) )
		chatWindowMap()->insert(d->mProtocol, myWindow);
	
	return myWindow;
}

void KopeteMessageManager::newChatView()
{
	if (d->mWidget == ChatWindow)
	{
		if(d->mView == 0L)
		{
			kdDebug(14010) << k_funcinfo << "Adding a new chat window/view" << endl;
			d->mView = newWindow()->addChatView( this );
		} else {
			kdDebug(14010) << k_funcinfo << "Adding a new chat view" << endl;
			d->mView = newWindow()->addChatView( this );
		}

		/* When the window is shown, we have to delete this contact event */
		//kdDebug(14010) << "[KopeteMessageManager] Connecting message box shown() to event killer" << endl;
		connect (d->mView, SIGNAL(Shown()), this, SLOT(slotCancelUnreadMessageEvent()));
		connect (d->mView, SIGNAL(SendMessage(const KopeteMessage &)), this, SLOT(slotMessageSent(const KopeteMessage &)));
		connect (d->mView, SIGNAL(Closing()), this, SLOT(slotChatViewClosing()));
		connect (d->mView, SIGNAL(Typing(bool)), this, SLOT(slotTyping(bool)));

		connect (this, SIGNAL(contactAdded(const KopeteContact *)), d->mView, SLOT(contactAdded(const KopeteContact *)));
		connect (this, SIGNAL(contactRemoved(const KopeteContact *)), d->mView, SLOT(contactRemoved(const KopeteContact *)));
	}

	if (d->mWidget == Email)
	{
		d->mEmailWindow = new KopeteEmailWindow(d->mUser, d->mContactList);
		d->mEmailWindow->setSendEnabled(d->mSendEnabled);
		connect (d->mEmailWindow, SIGNAL(shown()), this, SLOT(slotCancelUnreadMessageEvent()));
		connect (d->mEmailWindow, SIGNAL(sendMessage(KopeteMessage &)),
			 this, SLOT(slotMessageSent(const KopeteMessage &)));
		connect (d->mEmailWindow, SIGNAL(closeClicked()), this, SLOT(slotChatWindowClosing()));
		connect (d->mEmailWindow, SIGNAL(replyClicked()), this, SLOT(slotReply()));
	}
}

void KopeteMessageManager::newReplyWindow()
{
	if (d->mWidget == Email)
	{
		kdDebug(14010) << k_funcinfo << "newReplyWindow() called for email-type window" << endl;
		d->mEmailReplyWindow = new KopeteEmailWindow(d->mUser, d->mContactList);
		d->mEmailReplyWindow->setSendEnabled(true);
		d->mEmailReplyWindow->setReplyMode(true);
		d->mEmailReplyWindow->show();
		d->mEmailReplyWindow->raise();
		connect (d->mEmailReplyWindow, SIGNAL(sendMessage(const KopeteMessage &)),
			 this, SLOT(slotMessageSent(const KopeteMessage &)));
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
		kdDebug(14010) << k_funcinfo << "ERROR: unknown reading method, setting to default" << endl;
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
		return d->mView;
	if(d->mWidget == Email)
		return d->mEmailWindow;
	return 0L;
}


bool KopeteMessageManager::emptyMessageBuffer()
{
	if (!widget() )
	{
		kdDebug(14010) << k_funcinfo << "ChatView doesn't exist" << endl;
		newChatView();
	}

	bool foreignMessage = false;
	for (KopeteMessageList::Iterator it = d->mMessageQueue.begin(); it != d->mMessageQueue.end(); it = d->mMessageQueue.begin())
	{
//		kdDebug(14010) << "KopeteMessageManager::emptyMessageBuffer: Inserting message from " << (*it).from()->displayName() << endl;
		if ( (*it).from() != d->mUser )
			foreignMessage = true;

		emit messageReceived( *it );
		if ( d->mWidget == ChatWindow ) // ### why don't they implement the same interface?
			d->mView->messageReceived(*it);
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
		kdDebug(14010) << k_funcinfo << "Busy! A plugin is working on a precedent message" << endl;
		return;
	}

	if ( widget() == 0L )
	{
		kdDebug(14010) << k_funcinfo << "ChatView doesn't exist" << endl;
		newChatView();
	}
	QWidget *mainView = widget();
	if ( mainView == 0L )
	{
		kdDebug(14010) << k_funcinfo << "WARNING: cannot get the ChatWindow"  << endl;
		d->mMessageQueue.clear();
		return;
	}

	d->isBusy = true;
	bool queueEmpty = d->mMessageQueue.isEmpty();
	bool foreignMessage = emptyMessageBuffer();

	// only show the window when a message from someone else (i.e. not an own message) arrived or
	// when no message at all arrived (happens when you click on a contact, creating the window)
	if ( foreignMessage || queueEmpty )
	{
		myWindow->show();
		if( queueEmpty || KopetePrefs::prefs()->raiseMsgWindow() )
		{
			KWin::setOnDesktop(myWindow->winId() , KWin::currentDesktop()); //set on the desktop
			myWindow->raise(); // make it top window
			myWindow->makeWidgetDockVisible(mainView);
			myWindow->setActiveWindow();
		}
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
	kdDebug(14010) << k_funcinfo << endl;
	if (d->mEmailReplyWindow == 0L)
	{
		/* PLTHARG! */
		kdDebug(14010) << k_funcinfo << "mEmailReplyWindow == 0L, calling nRW()" << endl;
		newReplyWindow();
	}
	else
	{
		kdDebug(14010) << k_funcinfo << "mEmailWindow != 0L, not starting a new one (duh)." << endl;
	}
}

void KopeteMessageManager::slotMessageSent(const KopeteMessage &message)
{
	KopeteMessage sentMessage = message;
	emit messageQueued( sentMessage );
	emit messageSent(sentMessage, this);

	if ( KopetePrefs::prefs()->soundNotify() )
	{
		if ( !protocol()->isAway() || KopetePrefs::prefs()->soundIfAway() )
		    KNotifyClient::event("kopete_outgoing");
	}
}

void KopeteMessageManager::slotChatViewClosing()
{
	d->mView = 0L;
	if(d->mCanBeDeleted)
	{
		kdDebug(14010) << k_funcinfo << "delete KMM" << endl;
		deleteLater();
	}
}

void KopeteMessageManager::slotChatWindowClosing()
{
	if (d->mWidget == ChatWindow)
	{
		//We are deleting this window instance
		kdDebug(14010) << k_funcinfo << "Chat Window closed, now 0L" << endl;
		ChatWindowMap windowMap = *(chatWindowMap());
		if( windowMap.contains( d->mProtocol ) && (windowMap[ d->mProtocol ] == myWindow) )
			chatWindowMap()->remove( d->mProtocol );
		
		//Close *our* window
		myWindow->deleteLater();
		
		d->mView = 0L;
	}
	else if (d->mWidget == Email)
	{
		kdDebug(14010) << k_funcinfo << "Email Window closed, now 0L." << endl;
		delete d->mEmailWindow;
		d->mEmailWindow = 0L;
	}
	if(d->mCanBeDeleted)
	{
		kdDebug(14010) << k_funcinfo << "delete KMM" << endl;
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
		kdDebug(14010) << k_funcinfo << "No event to delete" << endl;
	}
	else
	{
		delete d->mUnreadMessageEvent;
		d->mUnreadMessageEvent = 0L;
		kdDebug(14010) << k_funcinfo << "Event Deleted" << endl;
	}
	emptyMessageBuffer();
}

void KopeteMessageManager::slotEventDeleted(KopeteEvent *e)
{
	if ( e == d->mUnreadMessageEvent)
	{
		kdDebug(14010) << k_funcinfo << "Event done(), now 0L" << endl;
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
		newChatView();
	else
		isvisible = myWindow->isVisible();


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
		kdDebug(14010) << k_funcinfo << "Contact already exists" <<endl;
		emit contactAdded(c);
	}
	else
	{
		if(d->mContactList.count()==1 && d->isEmpty)
		{
			KopeteContact *old=d->mContactList.first();
			kdDebug(14010) << k_funcinfo << old->displayName() << " left and " << c->displayName() << " joined " <<endl;
			d->mContactList.remove(old);
			d->mContactList.append(c);
			emit contactAdded(c);
			emit contactRemoved(old);
		}
		else
		{
			kdDebug(14010) << k_funcinfo << "Contact Joined session : " <<c->displayName() <<endl;
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
		kdDebug(14010) << k_funcinfo << "Contact not removed. Keep always one contact" <<endl;
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

void KopeteMessageManager::receivedTypingMsg( const KopeteContact *c , bool t )
{
	if( d->mWidget == ChatWindow )
	{
		if (d->mView)
		{
			d->mView->anyTyping( c , t );
		}
	}
}

void KopeteMessageManager::receivedTypingMsg( const QString &contactId , bool t )
{
	for( KopeteContact *it = d->mContactList.first(); it; it = d->mContactList.next() )
	{
		if( it->contactId() == contactId )
		{
			receivedTypingMsg( it, t );
			return;
		}
	}
}

void KopeteMessageManager::slotTyping ( bool t )
{
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
		if (d->mView)
			return d->mView->currentMessage();
	}
	kdDebug(14010) << k_funcinfo << "ChatWindow does not exist!" <<endl;
	return KopeteMessage();
}

#include "kopetemessagemanager.moc"

// vim: set noet ts=4 sts=4 sw=4:
