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
	if (d->mWidget == Email)
	{
		if (d->mEmailWindow)
			d->mEmailWindow->setSendEnabled(e);
	}
}

void KopeteMessageManager::setChatView( Kopete::ChatView *newView )
{
	d->mView = newView;
	mainWindow( newView->mainWindow() );
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
 * This method sets and returns a static mainWindow pointer - shared across all KMMs
 */
KopeteChatWindow *KopeteMessageManager::mainWindow(KopeteChatWindow *newValue)
{
	static KopeteChatWindow *m_mainWindow = 0L;
	if( newValue != 0L )
		m_mainWindow = newValue;
	return m_mainWindow;
}

/*
 * This method returns the static chatWindowMap pointer for the protocol<->window mapping
 */
QMap<KopeteProtocol*,KopeteChatWindow*> KopeteMessageManager::chatWindowMap()
{
	static QMap<KopeteProtocol*,KopeteChatWindow*> m_chatWindowMap;
	return m_chatWindowMap;
}


/*
 * This method returns the static mainWindow pointer for our policy type
 */
KopeteChatWindow *KopeteMessageManager::newMainWindow()
{
	//Determine tabbed window settings
	switch( KopetePrefs::prefs()->chatWindowPolicy() )
	{
		case NEW_WINDOW: //Open every chat in a new window
			kdDebug(14010) << "KopeteMessageManager::newMainWindow() : Always open new window" << endl;
			
			//Always create new window
			mainWindow( new KopeteChatWindow() );
			connect (mainWindow(), SIGNAL(Closing()), this, SLOT(slotChatWindowClosing()));
			break;

		case GROUP_BY_PROTOCOL: //Open chats in the same protocol in the same window
			kdDebug(14010) << "KopeteMessageManager::newMainWindow() : Group by protocol" << endl;
			
			//Check if we have a window for this protocol
			if( chatWindowMap().contains( d->mProtocol ) )
				mainWindow( chatWindowMap()[d->mProtocol] );
			else
			{
				//A window for this protocol does not exist, create new window
				mainWindow( new KopeteChatWindow() );
				connect (mainWindow(), SIGNAL(Closing()), this, SLOT(slotChatWindowClosing()));
			}
			break;

		case GROUP_ALL: //Open all chats in the same window
			kdDebug(14010) << "KopeteMessageManager::newMainWindow() : Group all" << endl;
			
			//Check if a window exists
			if( !mainWindow() ) {
				//No window exists, create new window
				mainWindow( new KopeteChatWindow() );
				connect (mainWindow(), SIGNAL(Closing()), this, SLOT(slotChatWindowClosing()));
			}
			break;
	}

	//Set *our* window equal to this window
	myWindow = mainWindow();
	
	//Add this protocol to the map no matter what the preference, in case it is switched while windows are open
	if( !chatWindowMap().contains( d->mProtocol ) )
		chatWindowMap().insert(d->mProtocol, mainWindow());

	return mainWindow();
}

void KopeteMessageManager::newChatView()
{
	if (d->mWidget == ChatWindow)
	{
		if(d->mView == 0L)
		{
			kdDebug(14010) << "KopeteMessageManager::newChatView() : Adding a new chat window/view" << endl;
			d->mView = newMainWindow()->addChatView( this );
		} else {
			kdDebug(14010) << "KopeteMessageManager::newChatView() : Adding a new chat view" << endl;
			d->mView = newMainWindow()->addChatView( this );
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
		kdDebug(14010) << "[KopeteMessageManager] newReplyWindow() called for email-type window" << endl;
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
		return d->mView;
	if(d->mWidget == Email)
		return d->mEmailWindow;
	return 0L;
}


bool KopeteMessageManager::emptyMessageBuffer()
{
	if (!widget() )
	{
		kdDebug(14010) << "KopeteMessageManager::emptyMessageBuffer: ChatView doesn't exist" << endl;
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
		kdDebug(14010) << "KopeteMessageManager::readMessages: Busy! A plugin is working on a precedent message" << endl;
		return;
	}

	if ( widget() == 0L )
	{
		kdDebug(14010) << "KopeteMessageManager::readMessages: ChatView doesn't exist" << endl;
		newChatView();
	}
	QWidget *mainView = widget();
	if ( mainView == 0L )
	{
		kdDebug(14010) << "KopeteMessageManager::readMessages: WARNING: cannot get the ChatWindow"  << endl;
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
		mainWindow()->show();
		if( queueEmpty || KopetePrefs::prefs()->raiseMsgWindow() )
		{
			KWin::setOnDesktop(mainWindow()->winId() , KWin::currentDesktop()); //set on the desktop
			mainWindow()->raise(); // make it top window
			mainWindow()->makeWidgetDockVisible(mainView);
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
		kdDebug(14010) << "KopeteMessageManager::slotChatViewClosing : delete KMM" << endl;
		deleteLater();
	}
}

void KopeteMessageManager::slotChatWindowClosing()
{
	if (d->mWidget == ChatWindow)
	{
		//We are deleting this window instance
		kdDebug(14010) << "KopeteMessageManager::mainWindow() : Chat Window closed, now 0L" << endl;
		if( chatWindowMap().contains( d->mProtocol ) && (chatWindowMap()[ d->mProtocol ] == myWindow) )
			chatWindowMap().remove( d->mProtocol );
		
		//Close *our* window
		myWindow->deleteLater();
		
		d->mView = 0L;
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
		newChatView();
	else
		isvisible = mainWindow()->isVisible();


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
		//if (d->mView)
		//	return d->mView->currentMessage();
	}
	kdDebug(14010) << "KopeteMessageManager::currentMessage(); ChatWindow does not exist!" <<endl;
	return KopeteMessage();
}

void KopeteMessageManager::setCurrentMessage(const KopeteMessage& t)
{
	if (d->mWidget == ChatWindow)
	{
		//if (d->mView)
		//	d->mView->setCurrentMessage(t);
	}
}

#include "kopetemessagemanager.moc"

// vim: set noet ts=4 sts=4 sw=4:
