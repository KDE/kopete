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

#include "kopeteaway.h"
#include "kopeteemailwindow.h"
#include "kopeteevent.h"
#include "kopetemessagelog.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetenotifier.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "kopetemetacontact.h"
#include "kopeteviewmanager.h"

#define NEW_WINDOW 0
#define GROUP_BY_PROTOCOL 1
#define GROUP_ALL 2

struct KMMPrivate
{
	KopeteContactPtrList mContactList;
	const KopeteContact *mUser;
	KopeteEvent *mUnreadMessageEvent;
	KopeteMessageList mMessageQueue;
	KopeteMessageLog *mLogger;
	int mReadMode;
	QMap<const KopeteContact *, QStringList> resources;
	KopeteProtocol *mProtocol;
	bool mSendEnabled;
	int mId;
	bool mLog;
	bool isEmpty;
	bool mCanBeDeleted;
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
	d->mUnreadMessageEvent = 0L;
	d->mProtocol = protocol;
	d->mId = id;
	d->mLog = true;
	d->isEmpty= others.isEmpty();
	d->mCanBeDeleted = false;
	d->isBusy=false;

	readModeChanged();
	connect( KopetePrefs::prefs(), SIGNAL(queueChanged()), this, SLOT(readModeChanged()));

	// Replace '.', '/' and '~' in the user id with '-' to avoid possible
	// directory traversal, although appending '.log' and the rest of the
	// code should really make overwriting files possible anyway.
	KopeteContact *c = others.first();
	QString logFileName = QString::fromLatin1( "kopete/" ) + QString::fromLatin1( c->protocol()->pluginId() ) +
		QString::fromLatin1( "/" ) + c->contactId().replace( QRegExp( QString::fromLatin1( "[./~]" ) ),
		QString::fromLatin1( "-" ) ) + QString::fromLatin1( ".log" );
	d->mLogger = new KopeteMessageLog( logFileName, this );

//	connect(protocol, SIGNAL(destroyed()), this, SLOT(slotProtocolUnloading()));

	kdDebug(14010) << k_funcinfo << endl;
}

KopeteMessageManager::~KopeteMessageManager()
{
	kdDebug(14010) << k_funcinfo << endl;
	d->mCanBeDeleted = false; //prevent double deletion
	KopeteMessageManagerFactory::factory()->removeSession( this );
	emit(closing( this ) );
	delete d;
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

	//If we have only 1 contact, add the status of him
	if( d->mContactList.count() == 1 )
	{
		chatName.append( QString::fromLatin1(" (") + c->statusText() + QString::fromLatin1(")") );
	}
	else
	{
		while( ( c = d->mContactList.next() ) )
		{
			if( c->metaContact() )
				nextDisplayName = c->metaContact()->displayName();
			else
				nextDisplayName = c->displayName();
			chatName.append( QString::fromLatin1( ", " ) ).append( nextDisplayName );
		}
	}

	return chatName;
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

bool KopeteMessageManager::emptyMessageBuffer()
{
	bool foreignMessage = false;
	for (KopeteMessageList::Iterator it = d->mMessageQueue.begin(); it != d->mMessageQueue.end(); it = d->mMessageQueue.begin())
	{
//		kdDebug(14010) << "KopeteMessageManager::emptyMessageBuffer: Inserting message from " << (*it).from()->displayName() << endl;
		if ( (*it).from() != d->mUser )
			foreignMessage = true;

		emit messageReceived( *it );

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
	d->isBusy = true;

	bool queueEmpty = d->mMessageQueue.isEmpty();
	bool foreignMessage = emptyMessageBuffer();

	// only show the window when a message from someone else (i.e. not an own message) arrived or
	// when no message at all arrived (happens when you click on a contact, creating the window)
	if ( foreignMessage || queueEmpty )
	{
		KopeteView *thisView = KopeteViewManager::viewManager()->view(this);

		if( !thisView->isVisible() )
			thisView->makeVisible();

		if( queueEmpty || KopetePrefs::prefs()->raiseMsgWindow() )
			thisView->raise();
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
 /* Used for single shot window */
}

void KopeteMessageManager::messageSent(const KopeteMessage &message)
{
	KopeteMessage sentMessage = message;
	emit messageQueued( sentMessage );
	emit messageSent(sentMessage, this);

	if ( KopetePrefs::prefs()->soundNotify() )
	{
		if ( !protocol()->isAway() || KopetePrefs::prefs()->soundIfAway() )
			KNotifyClient::event( QString::fromLatin1( "kopete_outgoing" ) );
	}
}

void KopeteMessageManager::cancelUnreadMessageEvent()
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
	bool isVisible = KopeteViewManager::viewManager()->view(this)->isVisible();

	d->mMessageQueue.append(msg);

	if( d->mLogger && d->mLog )
		d->mLogger->append( msg );

	if ( isVisible || d->mReadMode == Popup )
		readMessages();

	else if (d->mReadMode == Queued)
	{
		// Create an event if a previous one does not exist
		if ((d->mUnreadMessageEvent == 0L) && (msg.direction() == KopeteMessage::Inbound))
		{
			if (msg.from()->metaContact())
			{
				d->mUnreadMessageEvent = new KopeteEvent( i18n("Message from %1").arg(msg.from()->metaContact()->displayName()),
					QString::fromLatin1( "kopete/pics/newmsg.png" ), this, SLOT(slotReadMessages()));
			}
			else
			{
				d->mUnreadMessageEvent = new KopeteEvent( i18n("Message from %1").arg(msg.from()->displayName()),
					QString::fromLatin1( "kopete/pics/newmsg.png" ), this, SLOT(slotReadMessages()));
			}
			connect(d->mUnreadMessageEvent, SIGNAL(done(KopeteEvent *)),
				this, SLOT(slotEventDeleted(KopeteEvent *)));
			KopeteNotifier::notifier()->notifyEvent( d->mUnreadMessageEvent );
		}
	}

	if ( KopetePrefs::prefs()->soundNotify() && (isVisible || d->mReadMode == Popup) && (msg.direction() != KopeteMessage::Outbound) )
	{
		if ( !protocol()->isAway() || KopetePrefs::prefs()->soundIfAway() )
			KNotifyClient::event( QString::fromLatin1( "kopete_incoming" ) );
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
			disconnect (old->metaContact(), SIGNAL(displayNameChanged(KopeteMetaContact *, const QString)), this, SIGNAL(chatNameChanged()));
			connect (c->metaContact(), SIGNAL(displayNameChanged(KopeteMetaContact *, const QString)), this, SIGNAL(chatNameChanged()));
			emit contactAdded(c);
			emit contactRemoved(old);
		}
		else
		{
			kdDebug(14010) << k_funcinfo << "Contact Joined session : " <<c->displayName() <<endl;
			connect (c->metaContact(), SIGNAL(displayNameChanged(KopeteMetaContact *, const QString)), this, SIGNAL(chatNameChanged()));
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
		disconnect (c->metaContact(), SIGNAL(displayNameChanged(KopeteMetaContact *, const QString)), this, SIGNAL(chatNameChanged()));
	}
	emit contactRemoved(c);
}

void KopeteMessageManager::readModeChanged()
{
	if ( KopetePrefs::prefs()->useQueue() )
		d->mReadMode = Queued;
	else
		d->mReadMode = Popup;
}

void KopeteMessageManager::receivedTypingMsg( const KopeteContact *c , bool t )
{
	emit(remoteTyping( c, t ));
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

void KopeteMessageManager::typing ( bool t )
{
	emit typingMsg(t);
}

void KopeteMessageManager::setCanBeDeleted ( bool b )
{
	d->mCanBeDeleted = b;
	if(b)
		deleteLater();
}

KopeteMessage KopeteMessageManager::currentMessage()
{
	return KopeteViewManager::viewManager()->view(this)->currentMessage();
}

void KopeteMessageManager::setCurrentMessage(const KopeteMessage &message)
{
	KopeteViewManager::viewManager()->view(this)->setCurrentMessage(message);
}

#include "kopetemessagemanager.moc"

// vim: set noet ts=4 sts=4 sw=4:
