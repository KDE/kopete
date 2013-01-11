/*
    kopeteviewmanager.cpp - View Manager

    Copyright (c) 2003      by Jason Keirstead       <jason@keirstead.org>
    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteviewmanager.h"

#include <QList>
#include <QTextDocument>
#include <QtAlgorithms>

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kplugininfo.h>
#include <knotification.h>
#include <kglobal.h>
#include <kwindowsystem.h>

#include "kopetebehaviorsettings.h"
#include "kopeteaccount.h"
#include "kopetepluginmanager.h"
#include "kopeteviewplugin.h"
#include "kopetechatsessionmanager.h"
#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopetemessageevent.h"
#include "kopeteview.h"
#include "kopetechatsession.h"
#include "kopetegroup.h"
#include "kopetepicture.h"
#include "kopeteemoticons.h"
#include "kopeteactivenotification.h"

/**
 * Used to exrtract the message that will be shown in the notification popup.
 *
 * Was in KopeteSystemTray::squashMessage in KDE3
 */

static QString squashMessage( const Kopete::Message& msg, const int len = 30 )
{
	QString msgText = msg.parsedBody();

	QRegExp rx( "(<a.*>((http://)?(.+))</a>)" );
	rx.setMinimal( true );
	if ( rx.indexIn( msgText ) == -1 )
	{
		// no URLs in text, just pick the first chars of
		// the parsed text if necessary. We used parsed text
		// so that things like "<knuff>" show correctly
		//  Escape it after snipping it to not snip entities
		msgText = msg.plainBody();
		if( msgText.length() > len )
			msgText = msgText.left( len ) + QString::fromLatin1( " ..." );
		msgText=Kopete::Emoticons::parseEmoticons(Qt::escape(msgText));
	}
	else
	{
		QString plainText = msg.plainBody();
		if ( plainText.length() > len )
		{
			QString fullUrl = rx.cap( 2 );
			QString shorterUrl;
			if ( fullUrl.length() > len )
			{
				QString urlWithoutProtocol = rx.cap( 4 );
				shorterUrl = urlWithoutProtocol.left( len - 3 )
						+ QString::fromLatin1( "... " );
			}
			else
			{
				shorterUrl = fullUrl.left( len - 3 )
						+ QString::fromLatin1( "... " );
			}
			// remove message text
			msgText = QString::fromLatin1( "... " ) +
					rx.cap( 1 ) +
					QString::fromLatin1( " ..." );
			// find last occurrence of URL (the one inside the <a> tag)
			int revUrlOffset = msgText.lastIndexOf( fullUrl );
			msgText.replace( revUrlOffset,
						fullUrl.length(), shorterUrl );
		}
	}
 	kDebug(14000) << msgText;
	return msgText;
}

typedef QMap<Kopete::ChatSession*, KopeteView*> SessionMap;
typedef QList<Kopete::MessageEvent*> EventList;

struct KopeteViewManagerPrivate
{
    ~KopeteViewManagerPrivate()
    {
        qDeleteAll(sessionMap);
        sessionMap.clear();
        qDeleteAll(eventList);
        eventList.clear();
    }

    SessionMap sessionMap;
    EventList eventList;
    KopeteView *activeView;

    bool useQueue;
    bool raiseWindow;
    bool queueUnreadMessages;
    bool queueOnlyHighlightedMessagesInGroupChats;
    bool queueOnlyMessagesOnAnotherDesktop;
    bool balloonNotifyIgnoreClosesChatView;
    bool balloonGroupMessageNotificationsPerSender;
    bool foreignMessage;
    bool animateOnMessageWithOpenChat;
    bool enableEventsWhileAway;
    Kopete::ActiveNotifications activeNotifications;
};

KopeteViewManager *KopeteViewManager::s_viewManager = 0L;

KopeteViewManager *KopeteViewManager::viewManager()
{
    if( !s_viewManager )
            s_viewManager = new KopeteViewManager();
    return s_viewManager;
}

KopeteViewManager::KopeteViewManager()
{
    s_viewManager=this;
    d = new KopeteViewManagerPrivate;
    d->activeView = 0L;
    d->foreignMessage=false;

    connect( Kopete::BehaviorSettings::self(), SIGNAL(configChanged()), this, SLOT(slotPrefsChanged()) );

    connect( Kopete::ChatSessionManager::self() , SIGNAL(display(Kopete::Message&,Kopete::ChatSession*)),
            this, SLOT (messageAppended(Kopete::Message&,Kopete::ChatSession*)) );

    connect( Kopete::ChatSessionManager::self() , SIGNAL(readMessage()),
            this, SLOT (nextEvent()) );

    slotPrefsChanged();
}

KopeteViewManager::~KopeteViewManager()
{
// 	kDebug(14000) ;

    //delete all open chatwindow.
    SessionMap::Iterator it;
    for ( it = d->sessionMap.begin(); it != d->sessionMap.end(); ++it )
    {
        it.value()->closeView( true ); //this does not clean the map, but we don't care
    }

    delete d;
}

void KopeteViewManager::slotPrefsChanged()
{
	Kopete::BehaviorSettings *const bs = Kopete::BehaviorSettings::self();

	d->useQueue = bs->useMessageQueue();
	d->raiseWindow = bs->raiseMessageWindow();
	d->queueUnreadMessages = bs->queueUnreadMessages();
	d->queueOnlyHighlightedMessagesInGroupChats = bs->queueOnlyHighlightedMessagesInGroupChats();
	d->queueOnlyMessagesOnAnotherDesktop = bs->queueOnlyMessagesOnAnotherDesktop();
	d->balloonNotifyIgnoreClosesChatView = bs->balloonNotifyIgnoreClosesChatView();
	d->balloonGroupMessageNotificationsPerSender = bs->balloonGroupMessageNotificationsPerSender();
	d->animateOnMessageWithOpenChat = bs->animateOnMessageWithOpenChat();
	d->enableEventsWhileAway = bs->enableEventsWhileAway();
}

KopeteView *KopeteViewManager::view( Kopete::ChatSession* session, const QString &requestedPlugin )
{
    // kDebug(14000) ;

    if( d->sessionMap.contains( session ) && d->sessionMap[ session ] )
    {
        return d->sessionMap[ session ];
    }
    else
    {
        Kopete::PluginManager *pluginManager = Kopete::PluginManager::self();
        Kopete::ViewPlugin *viewPlugin = 0L;

        QString pluginName = requestedPlugin.isEmpty() ? Kopete::BehaviorSettings::self()->viewPlugin() : requestedPlugin;
        if( !pluginName.isEmpty() )
        {
            viewPlugin = (Kopete::ViewPlugin*)pluginManager->loadPlugin( pluginName );

            if( !viewPlugin )
            {
                kWarning(14000) << "Requested view plugin, " << pluginName
                                << ", was not found. Falling back to chat window plugin" << endl;
            }
        }

        if( !viewPlugin )
        {
            viewPlugin = (Kopete::ViewPlugin*)pluginManager->loadPlugin( QLatin1String("kopete_chatwindow") );
        }

        if( viewPlugin )
        {
            KopeteView *newView = viewPlugin->createView(session);

            d->foreignMessage = false;
            d->sessionMap.insert( session, newView );

            connect( session, SIGNAL(closing(Kopete::ChatSession*)),
                     this, SLOT(slotChatSessionDestroyed(Kopete::ChatSession*)) );

            return newView;
        }
        else
        {
            kError(14000) << "Could not create a view, no plugins available!" << endl;
            return 0L;
        }
    }
}


void KopeteViewManager::messageAppended( Kopete::Message &msg, Kopete::ChatSession *session)
{
	const bool isOutboundMessage = msg.direction() == Kopete::Message::Outbound;

	if ( isOutboundMessage && !d->sessionMap.contains( session ) )
		return;

	// Get an early copy of the plain message body before the chat view works on it
	// Otherwise toPlainBody() will ignore smileys if they were turned into images during
	// the html conversion. See bug 161651.
	const QString squashedMessage = squashMessage( msg, 300 );

	d->foreignMessage = !isOutboundMessage; // for the view we are about to create
	session->view( true, msg.requestedPlugin() )->appendMessage( msg );
	d->foreignMessage = false; // the view has been created, reset the flag

	bool appendMessageEvent = d->useQueue;
	bool isViewOnCurrentDesktop = true;

	QWidget *w = dynamic_cast< QWidget * >( view( session ) );
#ifdef Q_WS_X11
	if ( w )
	{
		isViewOnCurrentDesktop = KWindowSystem::windowInfo( w->topLevelWidget()->winId(),
		                                                    NET::WMDesktop ).isOnCurrentDesktop();
	}
#endif

	if ( w && d->queueUnreadMessages )
	{
		// append msg event to queue if chat window is active but not the chat view in it...
		appendMessageEvent = appendMessageEvent && !( w->isActiveWindow()
		                                              && session->view() == d->activeView );
		// ...and chat window is on another desktop
		appendMessageEvent = appendMessageEvent && !( d->queueOnlyMessagesOnAnotherDesktop
		                                              && isViewOnCurrentDesktop );
	}
	else
	{
		// append if no chat window exists already
		appendMessageEvent = appendMessageEvent && !view( session )->isVisible();
	}

	// in groupchats, don't notify on non-highlighted messages if configured that way.
	const bool isIgnoredGroupChatMessage = session->members().count() > 1
	                                       && d->queueOnlyHighlightedMessagesInGroupChats
	                                       && msg.importance() != Kopete::Message::Highlight;

	appendMessageEvent = appendMessageEvent && !isIgnoredGroupChatMessage;

	QWidget *viewWidget = 0;
	bool showNotification = false;
	bool isActiveWindow = false;
	Kopete::MessageEvent *event = 0;

	if ( !isOutboundMessage )
	{
		if ( ( !session->account()->isAway() || d->enableEventsWhileAway )
	         && msg.direction() != Kopete::Message::Internal )
		{
			viewWidget = dynamic_cast< QWidget * >( session->view( false ) );
			// note that session->view( true [default value] ) can create a view, so watch the
			// side-effects...
			isActiveWindow = session->view( false ) && viewWidget
			                 && session->view() == d->activeView && viewWidget->isActiveWindow();
			showNotification = msg.from() != 0;
		}

		if ( appendMessageEvent || showNotification )
		{
			if ( msg.from() && d->eventList.isEmpty() ) { // may happen for internal messages
				if ( session->account()->isBusy() || ( session->account()->isAway() && !d->enableEventsWhileAway ) )
					showNotification = false;
				else
					showNotification = true;
			}

			event = new Kopete::MessageEvent( msg, session );
			d->eventList.append( event );

			// Don't call readMessages twice. We call it later in this method. Fixes bug 168978.
			if ( d->useQueue )
				connect( event, SIGNAL(done(Kopete::MessageEvent*)),
				         this, SLOT(slotEventDeleted(Kopete::MessageEvent*)) );
		}
	}

	if ( msg.delayed() )
		showNotification = false;

	if ( showNotification )
		createNotification( msg, squashedMessage, session, event, viewWidget,
		                    isActiveWindow, isViewOnCurrentDesktop );

	if (!d->useQueue)
	{
		// "Open messages instantly" setting
		readMessages( session, isOutboundMessage );
	}

	KopeteView *view = session->view( false );
	if ( d->raiseWindow && view && view->isVisible() )
	{
		// "Raise window on incoming message" setting
		view->raise();
	}

	if ( event && ( appendMessageEvent || ( d->animateOnMessageWithOpenChat && !isActiveWindow ) )
	     && !isIgnoredGroupChatMessage )
		Kopete::ChatSessionManager::self()->postNewEvent(event);
}

void KopeteViewManager::createNotification( Kopete::Message &msg, const QString &squashedMessage,
                                            Kopete::ChatSession *session, Kopete::MessageEvent *event,
	                                        QWidget *viewWidget,
	                                        bool isActiveWindow, bool isViewOnCurrentDesktop )
{

	if ( msg.from()->account()->isBusy() )
		return;

    if ( d->balloonGroupMessageNotificationsPerSender )
	{
		Kopete::ActiveNotifications::iterator notifyIt =
			d->activeNotifications.find( msg.from()->account()->accountLabel() + msg.from()->contactId() );
		if (notifyIt != d->activeNotifications.end())
		{
			( *notifyIt )->incrementMessages();
			return;
		}
	}

	const QString msgFrom = msg.from()->metaContact() ? msg.from()->metaContact()->displayName()
	                                                  : msg.from()->contactId();

	QString eventId, titleString;
    QString bodyString = squashedMessage;
	if ( msg.type() == Kopete::Message::TypeFileTransferRequest)
    {
		eventId = QLatin1String( "kopete_incoming_file_transfer" );
		titleString = i18nc( "@title %1 is contact's name", "Incoming file transfer request from <i>%1</i>", Qt::escape( msgFrom ) );
		if ( squashedMessage.isEmpty() )
        {
			bodyString = i18nc( "@info", "A user is trying to send you a file <filename>%1</filename>", Qt::escape( msg.fileName() ) );
        }
        else
        {
			bodyString = i18nc( "@info %2 is message", "A user is trying to send you a file <filename>%1</filename> with the message:<nl/>\"%2\"",
				Qt::escape( msg.fileName() ), squashedMessage );
        }
    }
    else
    {
        KLocalizedString title = ki18n( "Incoming message from <i>%1</i>" );
        switch( msg.importance() )
        {
        case Kopete::Message::Low:
            eventId = QLatin1String( "kopete_contact_lowpriority" );
            break;
        case Kopete::Message::Highlight:
            eventId = QLatin1String( "kopete_contact_highlight" );
            title = ki18n( "A highlighted message arrived from <i>%1</i>" );
            break;
        default:
            if ( isActiveWindow || (d->queueOnlyMessagesOnAnotherDesktop
                 && isViewOnCurrentDesktop ) )
            {
                eventId = QLatin1String( "kopete_contact_incoming_active_window" );
            }
            else
            {
                eventId = QLatin1String( "kopete_contact_incoming" );
            }
        }
    }
	KNotification *notify = new KNotification(eventId, viewWidget, isActiveWindow
	                                                               ? KNotification::CloseOnTimeout
	                                                               : KNotification::Persistent);
	notify->setPixmap( QPixmap::fromImage( msg.from()->metaContact()->picture().image() ) );
	notify->setActions( QStringList() << i18nc( "@action", "View" )
	                                  << i18nc( "@action", "Close" ) );

	if ( d->balloonGroupMessageNotificationsPerSender )
	{
		// notify is parent, will die with it
		new Kopete::ActiveNotification(	notify,
		                msg.from()->account()->accountLabel() + msg.from()->contactId(),
		                d->activeNotifications,
						titleString,
		                bodyString );
	}
	else
	{
		notify->setTitle( titleString );
		notify->setText( squashedMessage );
	}

	foreach ( const QString& cl , msg.classes() )
		notify->addContext( qMakePair( QString::fromLatin1("class"), cl ) );

	Kopete::MetaContact *mc= msg.from()->metaContact();
	if ( mc )
	{
		notify->addContext( qMakePair( QString::fromLatin1("contact"),
		                               mc->metaContactId().toString()) );
		foreach( Kopete::Group *g , mc->groups() )
		{
			notify->addContext( qMakePair( QString::fromLatin1("group"),
			                               QString::number(g->groupId())) );
		}
	}
	connect( notify, SIGNAL(activated()), session, SLOT(raiseView()) );
	connect( notify, SIGNAL(action1Activated()), session, SLOT(raiseView()) );
	connect( notify, SIGNAL(action2Activated()), event, SLOT(discard()) );
	connect( event, SIGNAL(done(Kopete::MessageEvent*)), notify, SLOT(close()) );
	notify->sendEvent();
}

void KopeteViewManager::readMessages( Kopete::ChatSession *session, bool isOutboundMessage, bool activate )
{
    // kDebug( 14000 ) ;
    d->foreignMessage = !isOutboundMessage; //for the view we are about to create
    KopeteView *thisView = session->view( true );
    d->foreignMessage = false; //the view is created, reset the flag
    if ( ( isOutboundMessage && !thisView->isVisible() ) || d->raiseWindow || activate )
            thisView->raise( activate );
    else if ( !thisView->isVisible() )
            thisView->makeVisible();

    foreach ( Kopete::MessageEvent *event, d->eventList )
    {
        if ( event->message().manager() == session )
        {
            event->apply();
            d->eventList.removeAll( event );
        }
    }
}

void KopeteViewManager::slotEventDeleted( Kopete::MessageEvent *event )
{
    // d->eventList.remove( event );
    d->eventList.removeAll(event);

//    kDebug(14000) ;
    Kopete::ChatSession *kmm=event->message().manager();
    if(!kmm)
    {
            // this can be NULL for example if KopeteViewManager::slotViewActivated()
            // got called which does deleteLater() the event what may result in the
            // case, that the QPointer<ChatSession> in Message::Private got removed
            // aka set to NULL already.
            return;
    }

    if ( event->state() == Kopete::MessageEvent::Applied )
    {
        readMessages( kmm, false, true );
    }
    else if ( event->state() == Kopete::MessageEvent::Ignored && d->balloonNotifyIgnoreClosesChatView )
    {
        bool bAnotherWithThisManager = false;
        foreach (Kopete::MessageEvent *event, d->eventList)
        {
            if ( event->message().manager() == kmm )
            {
                bAnotherWithThisManager = true;
            }
        }
        if ( !bAnotherWithThisManager && kmm->view( false ) )
            kmm->view()->closeView( true );
    }
}

void KopeteViewManager::nextEvent()
{
    // kDebug( 14000 ) ;

    if( d->eventList.isEmpty() )
        return;

    Kopete::MessageEvent* event = d->eventList.first();

    if ( event )
        event->apply();
}

void KopeteViewManager::slotViewActivated( KopeteView *view )
{
    // kDebug( 14000 ) ;
    d->activeView = view;

    foreach (Kopete::MessageEvent *event, d->eventList)
    {
        if ( event->message().manager() == view->msgManager() )
        {
            event->deleteLater();
        }
    }
}

void KopeteViewManager::slotViewDestroyed( KopeteView *closingView )
{
    // kDebug( 14000 ) ;

    if ( d->sessionMap.contains( closingView->msgManager() ) )
    {
        d->sessionMap.remove( closingView->msgManager() );
        // closingView->msgManager()->setCanBeDeleted( true );
    }

    if( closingView == d->activeView )
        d->activeView = 0L;
}

void KopeteViewManager::slotChatSessionDestroyed( Kopete::ChatSession *session )
{
	// kDebug( 14000 ) ;

	if ( d->sessionMap.contains( session ) )
	{
		KopeteView *v = d->sessionMap[ session ];
		v->closeView( true );
		delete v;   //closeView call deleteLater,  but in this case this is not enough, because some signal are called that case crash
		d->sessionMap.remove( session );
	}
}

KopeteView* KopeteViewManager::activeView() const
{
    return d->activeView;
}


QList<Kopete::MessageEvent*> KopeteViewManager::pendingMessages( Kopete::Contact *contact )
{
	QList<Kopete::MessageEvent*> pending;
	foreach (Kopete::MessageEvent *event, d->eventList)
	{
		const Kopete::Message &message = event->message();
		if ( event->state() == Kopete::MessageEvent::Nothing
			 && message.direction() == Kopete::Message::Inbound
			 && message.from() == contact )
		{
			pending << event;
		}
	}

	return pending;
}

#include "kopeteviewmanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

