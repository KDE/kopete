/*
    kopeteviewmanager.cpp - View Manager

    Copyright (c) 2003      by Jason Keirstead       <jason@keirstead.org>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kwin.h>

#include "kopeteprefs.h"
#include "kopeteaccount.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "chatview.h"
#include "kopeteemailwindow.h"
#include "kopeteevent.h"
#include "systemtray.h"

#include "kopeteviewmanager.h"

typedef QMap<KopeteMessageManager*,KopeteView*> ManagerMap;
typedef QMap<KopeteMessageManager*,KopeteEvent*> EventMap;
typedef QPtrList<KopeteEvent> EventQueue;

struct KopeteViewManagerPrivate
{
	ManagerMap managerMap;
	EventMap eventMap;
	KopeteView *activeView;
	EventQueue eventQueue;

	bool useQueue;
	bool raiseWindow;
};

KopeteViewManager *KopeteViewManager::s_viewManager = 0L;

KopeteViewManager *KopeteViewManager::viewManager()
{
	if( !s_viewManager )
		s_viewManager = new KopeteViewManager;

	return s_viewManager;
}

KopeteViewManager::KopeteViewManager() : QObject( kapp, "KopeteViewManager" )
{
	d = new KopeteViewManagerPrivate;
	d->activeView = 0L;
	connect( KopetePrefs::prefs(), SIGNAL( saved() ), this, SLOT( slotPrefsChanged() ) );
	connect( KopeteMessageManagerFactory::factory() , SIGNAL ( requestView(KopeteView*& , KopeteMessageManager * , KopeteMessage::MessageType  ) ) ,
		this, SLOT (slotRequestView(KopeteView*& , KopeteMessageManager * , KopeteMessage::MessageType  )));
	connect( KopeteMessageManagerFactory::factory() , SIGNAL( display( KopeteMessage &, KopeteMessageManager *) ),
		this, SLOT ( messageAppended( KopeteMessage &, KopeteMessageManager *) ) );

	connect( KopeteMessageManagerFactory::factory() , SIGNAL ( getActiveView(KopeteView*&  ) ) ,
		this, SLOT (slotGetActiveView(KopeteView*&)));

	slotPrefsChanged();
}

KopeteViewManager::~KopeteViewManager()
{
	kdDebug( 14000) << k_funcinfo << endl;
	delete d;
}

void KopeteViewManager::slotPrefsChanged()
{
	d->useQueue = KopetePrefs::prefs()->useQueue();
	d->raiseWindow = KopetePrefs::prefs()->raiseMsgWindow();
}

KopeteView *KopeteViewManager::view( KopeteMessageManager* manager, bool foreignMessage, KopeteMessage::MessageType type )
{
	if( d->eventMap.contains( manager ) )
	{
		d->eventMap[ manager ]->deleteLater();
		d->eventMap.remove( manager );
	}

	if( d->managerMap.contains( manager ) && d->managerMap[ manager ] )
	{
		return d->managerMap[ manager ];
	}
	else
	{
		KopeteView *newView;
		QWidget *newViewWidget;

		if( type == KopeteMessage::Undefined )
			type = static_cast<KopeteMessage::MessageType>( KopetePrefs::prefs()->interfacePreference() );

		if( type == KopeteMessage::Chat )
		{
			newView = new Kopete::ChatView( manager );
			newViewWidget = newView->mainWidget();

			connect (newViewWidget, SIGNAL( typing(bool) ), manager, SLOT( typing(bool) ) );
			connect (manager, SIGNAL( remoteTyping( const KopeteContact *, bool) ), newViewWidget, SLOT( remoteTyping(const KopeteContact *, bool) ) );
		}
		else
		{
			newView = new KopeteEmailWindow( manager, foreignMessage );
			newViewWidget = newView->mainWidget();
		}

		d->managerMap.insert( manager, newView );

		connect( newViewWidget, SIGNAL( closing( KopeteView * ) ), this, SLOT( slotViewDestroyed( KopeteView * ) ) );
		connect( newViewWidget, SIGNAL( messageSent(KopeteMessage &) ), manager, SLOT( sendMessage(KopeteMessage &) ) );
		connect( newViewWidget, SIGNAL( activated( KopeteView * ) ), this, SLOT( slotViewActivated( KopeteView * ) ) );
		connect( manager, SIGNAL( messageSuccess() ), newViewWidget, SLOT( messageSentSuccessfully() ));
		connect( manager, SIGNAL( closing(KopeteMessageManager *) ), this, SLOT(slotMessageManagerDestroyed(KopeteMessageManager*)) );

		return newView;
	}
}


void KopeteViewManager::messageAppended( KopeteMessage &msg, KopeteMessageManager *manager)
{
//	kdDebug( 14000 ) << k_funcinfo << endl;

	bool outgoingMessage = ( msg.direction() == KopeteMessage::Outbound );

	if( !outgoingMessage || d->managerMap.contains( manager ) )
	{
		view( manager, outgoingMessage )->messageReceived( msg );

		if ( !outgoingMessage && d->useQueue && !view( manager, outgoingMessage )->isVisible()  )
		{
			if (!d->eventMap.contains( manager ))
			{
				//FIXME: currently there are maximum only one event per kmm.
				KopeteEvent *event=new KopeteEvent(msg,manager);
				d->eventMap.insert( manager, event );
				d->eventQueue.append( event );
				connect(event, SIGNAL(applied(KopeteEvent *)), this, SLOT(slotEventApplied(KopeteEvent *)));
				connect(event, SIGNAL(done(KopeteEvent *)), this, SLOT(slotEventDeleted(KopeteEvent *)));
				emit newMessageEvent(event);
			}
		}
		else
		{
			readMessages( manager, outgoingMessage );
		}

		if ( !outgoingMessage && ( !manager->account()->isAway() || KopetePrefs::prefs()->soundIfAway() ) )
		{
			QString msgFrom = QString::null;
			if( msg.from()->metaContact() )
				msgFrom = msg.from()->metaContact()->displayName();
			else
				msgFrom = msg.from()->displayName();

			QString msgText = msg.plainBody();
			if( msgText.length() > 90 )
				msgText = msgText.left(88) + QString::fromLatin1("...");

			int winId = KopeteSystemTray::systemTray() ? KopeteSystemTray::systemTray()->winId() : 0;

			switch(msg.importance())
			{
				case KopeteMessage::Low:
					//TODO: add an event for this (like a litle beep)
					break;
				case KopeteMessage::Highlight:
					KNotifyClient::event( winId , QString::fromLatin1( "kopete_highlight"), i18n("<qt>An Highlighted message arrived from %1<br>\"%2\"</qt>").arg(msgFrom).arg(msgText) );
					break;
				case KopeteMessage::Normal:
				default:
					KNotifyClient::event( winId , QString::fromLatin1( "kopete_incoming"), i18n("<qt>Incoming message from %1<br>\"%2\"</qt>").arg(msgFrom).arg(msgText) );
					break;
			}
		}
	}
}

void KopeteViewManager::readMessages( KopeteMessageManager *manager, bool outgoingMessage )
{
	kdDebug( 14000 ) << k_funcinfo << endl;
	KopeteView *thisView = view( manager, !outgoingMessage );

 	if( ( outgoingMessage && !thisView->isVisible() ) || d->raiseWindow )
		thisView->raise();

	else if( !thisView->isVisible() )
		thisView->makeVisible();

	if(d->eventMap.contains(manager))
		d->eventMap[manager]->apply();
}


void KopeteViewManager::slotEventApplied( KopeteEvent *event )
{
	for ( EventMap::Iterator it = d->eventMap.begin(); it != d->eventMap.end(); ++it )
	{
		if( it.data() == event )
		{
			KopeteMessageManager *kmm=it.key();
			d->eventMap.remove( kmm );
			readMessages( kmm, false );
			break;
		}
	}
}

void KopeteViewManager::slotEventDeleted( KopeteEvent *event )
{
	for ( EventMap::Iterator it = d->eventMap.begin(); it != d->eventMap.end(); ++it )
	{
		if( it.data() == event )
		{
			//If this event is still in the map, then it has not been applied.
			//Close the view associated with it.
			view( it.key(), false )->closeView();
		}
	}
	d->eventQueue.remove( event );
}

void KopeteViewManager::nextEvent()
{
	kdDebug( 14000 ) << k_funcinfo << endl;

	if( d->eventQueue.isEmpty() )
		return;

	d->eventQueue.first()->apply();
	d->eventQueue.removeFirst();
}

void KopeteViewManager::slotViewActivated( KopeteView *view )
{
	kdDebug( 14000 ) << k_funcinfo << endl;
	d->activeView = view;
}

void KopeteViewManager::slotViewDestroyed( KopeteView *closingView )
{
	kdDebug( 14000 ) << k_funcinfo << endl;

	if( d->managerMap.contains( closingView->msgManager() ) )
	{
		d->managerMap.remove( closingView->msgManager() );
		closingView->msgManager()->setCanBeDeleted( true );
	}

	if( closingView == d->activeView )
		d->activeView = 0L;
}

void KopeteViewManager::slotMessageManagerDestroyed( KopeteMessageManager *manager )
{
	kdDebug( 14000 ) << k_funcinfo << endl;

	if( d->managerMap.contains( manager ) )
	{
		d->managerMap[ manager ]->closeView( true );
	}
}

KopeteView* KopeteViewManager::activeView() const
{
	return d->activeView;
}

void KopeteViewManager::slotRequestView(KopeteView*& v, KopeteMessageManager *kmm , KopeteMessage::MessageType type )
{
	v=view(kmm, false , type);
}

void KopeteViewManager::slotGetActiveView(KopeteView*&v)
{
	v=activeView();
}


#include "kopeteviewmanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

