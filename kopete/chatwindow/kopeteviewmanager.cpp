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
#include <kgenericfactory.h>
#include "kopetenotifyclient.h"

#include "kopeteprefs.h"
#include "kopeteaccount.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "chatview.h"
#include "kopeteemailwindow.h"
#include "kopeteevent.h"
//#include "systemtray.h"

#include "kopeteviewmanager.h"

typedef KGenericFactory<KopeteViewManager> ViewManagerFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_chatwindow, ViewManagerFactory( "kopete_chatwindow" )  )



typedef QMap<KopeteMessageManager*,KopeteView*> ManagerMap;
typedef QMap<KopeteMessageManager*,KopeteEvent*> EventMap;

struct KopeteViewManagerPrivate
{
	ManagerMap managerMap;
	EventMap eventMap;
	KopeteView *activeView;

	bool useQueue;
	bool raiseWindow;
	bool foreignMessage;
};

KopeteViewManager *KopeteViewManager::s_viewManager = 0L;

KopeteViewManager *KopeteViewManager::viewManager()
{
	return s_viewManager;
}

KopeteViewManager::KopeteViewManager (QObject *parent, const char *name, const QStringList &/*args*/ )
	: KopetePlugin( ViewManagerFactory::instance(), parent, name )

{
	s_viewManager=this;
	d = new KopeteViewManagerPrivate;
	d->activeView = 0L;
	d->foreignMessage=false;
	connect( KopetePrefs::prefs(), SIGNAL( saved() ), this, SLOT( slotPrefsChanged() ) );
	connect( KopeteMessageManagerFactory::factory() , SIGNAL ( requestView(KopeteView*& , KopeteMessageManager * , KopeteMessage::MessageType  ) ) ,
		this, SLOT (slotRequestView(KopeteView*& , KopeteMessageManager * , KopeteMessage::MessageType  )));
	connect( KopeteMessageManagerFactory::factory() , SIGNAL( display( KopeteMessage &, KopeteMessageManager *) ),
		this, SLOT ( messageAppended( KopeteMessage &, KopeteMessageManager *) ) );

	connect( KopeteMessageManagerFactory::factory() , SIGNAL ( getActiveView(KopeteView*&  ) ) ,
		this, SLOT (slotGetActiveView(KopeteView*&)));

	connect( KopeteMessageManagerFactory::factory() , SIGNAL( readMessage() ),
		this, SLOT ( nextEvent() ) );


	slotPrefsChanged();
}

KopeteViewManager::~KopeteViewManager()
{
//	kdDebug( 14000) << k_funcinfo << endl;

	//delete all open chatwindow.
	ManagerMap::Iterator it;
	for ( it = d->managerMap.begin(); it != d->managerMap.end(); ++it )
		it.data()->closeView( true ); //this does not clean the map, but we don't care

	delete d;
}

void KopeteViewManager::slotPrefsChanged()
{
	d->useQueue = KopetePrefs::prefs()->useQueue();
	d->raiseWindow = KopetePrefs::prefs()->raiseMsgWindow();
}

KopeteView *KopeteViewManager::view( KopeteMessageManager* manager, bool /*foreignMessage*/, KopeteMessage::MessageType type )
{
	/*if( d->eventMap.contains( manager ) )
	{
		d->eventMap[ manager ]->deleteLater();
		d->eventMap.remove( manager );
	}*/

	if( d->managerMap.contains( manager ) && d->managerMap[ manager ] )
	{
		return d->managerMap[ manager ];
	}
	else
	{
		KopeteView *newView;
		QWidget *newViewWidget;

		if( type == KopeteMessage::Undefined )
		{
			int t = KopetePrefs::prefs()->interfacePreference();
			type = static_cast<KopeteMessage::MessageType>( t );    
		}

		if( type == KopeteMessage::Chat )
		{
			newView = new ChatView( manager );
			newViewWidget = newView->mainWidget();

			connect (newViewWidget, SIGNAL( typing(bool) ), manager, SLOT( typing(bool) ) );
			connect (manager, SIGNAL( remoteTyping( const KopeteContact *, bool) ), newViewWidget, SLOT( remoteTyping(const KopeteContact *, bool) ) );
		}
		else
		{
			newView = new KopeteEmailWindow( manager, d->foreignMessage );
			newViewWidget = newView->mainWidget();
		}
		d->foreignMessage=false;

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
		d->foreignMessage=!outgoingMessage;
		manager->view(true)->appendMessage( msg );

		if ( d->useQueue && !view( manager, outgoingMessage )->isVisible()  )
		{
			if (!outgoingMessage && !d->eventMap.contains( manager ))
			{
				//FIXME: currently there are maximum only one event per kmm.
				KopeteEvent *event=new KopeteEvent(msg,manager);
				d->eventMap.insert( manager, event );
				connect(event, SIGNAL(done(KopeteEvent *)), this, SLOT(slotEventDeleted(KopeteEvent *)));
				KopeteMessageManagerFactory::factory()->postNewEvent(event);
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

			int winId = 0;  //KopeteSystemTray::systemTray() ? KopeteSystemTray::systemTray()->winId() : 0;
			QWidget *w=dynamic_cast<QWidget*>(manager->view(false));
			if(w) winId=w->topLevelWidget()->winId();

			switch( msg.importance() )
			{
				case KopeteMessage::Low:
					//TODO: add an event for this (like a little beep)
					break;
				default:
				{
					QString event = QString::fromLatin1( "kopete_incoming" );
					QString body = i18n( "<qt>Incoming message from %1<br>\"%2\"</qt>" );

					if( msg.importance() == KopeteMessage::Highlight )
					{
						event = QString::fromLatin1( "kopete_highlight" );
						body = i18n( "<qt>A highlighted message arrived from %1<br>\"%2\"</qt>" );
					}
#if QT_VERSION < 0x030200
					KNotifyClient::event( winId, event, body.arg( msgFrom ).arg( msgText ) ,
#else
					KNotifyClient::event(winId,  event, body.arg( msgFrom, msgText ) ,
#endif
						i18n("View") , const_cast<KopeteContact*>(msg.from()) , SLOT(execute()) );
				}
			}
		}
	}
}

void KopeteViewManager::readMessages( KopeteMessageManager *manager, bool outgoingMessage )
{
//	kdDebug( 14000 ) << k_funcinfo << endl;
	d->foreignMessage=!outgoingMessage;
	KopeteView *thisView = manager->view( true );
 	if( ( outgoingMessage && !thisView->isVisible() ) || d->raiseWindow )
		thisView->raise();

	else if( !thisView->isVisible() )
		thisView->makeVisible();

	if(d->eventMap.contains(manager))
		d->eventMap[manager]->apply();
}

void KopeteViewManager::slotEventDeleted( KopeteEvent *event )
{
	KopeteMessageManager *kmm=event->message().manager();
	if(!kmm)
		return;
	if(d->eventMap.contains(kmm) && d->eventMap[kmm]==event)
	{
		if(event->state()==KopeteEvent::Applied)
		{
			readMessages( kmm, false );
		}
		else if(event->state()==KopeteEvent::Ignored)
		{
			if(kmm->view(false))
				kmm->view()->closeView(true);
		}

		d->eventMap.remove(kmm);
	}
}

void KopeteViewManager::nextEvent()
{
//	kdDebug( 14000 ) << k_funcinfo << endl;

	if( d->eventMap.isEmpty() )
		return;

	KopeteEvent *e= *(d->eventMap.begin());

	if(e)
		e->apply();
}

void KopeteViewManager::slotViewActivated( KopeteView *view )
{
//	kdDebug( 14000 ) << k_funcinfo << endl;
	d->activeView = view;
	if(d->eventMap.contains(view->msgManager()))
	{
		KopeteEvent *e=d->eventMap[view->msgManager()];
		if(e)
			e->deleteLater();
	}

}

void KopeteViewManager::slotViewDestroyed( KopeteView *closingView )
{
//	kdDebug( 14000 ) << k_funcinfo << endl;

	if( d->managerMap.contains( closingView->msgManager() ) )
	{
		d->managerMap.remove( closingView->msgManager() );
//		closingView->msgManager()->setCanBeDeleted( true );
	}

	if( closingView == d->activeView )
		d->activeView = 0L;
}

void KopeteViewManager::slotMessageManagerDestroyed( KopeteMessageManager *manager )
{
//	kdDebug( 14000 ) << k_funcinfo << endl;

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

