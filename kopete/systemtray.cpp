/*
    systemtray.cpp  -  Kopete Tray Dock Icon

    Copyright (c) 2002      by Nick Betcher           <nbetcher@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2003-2004 by Olivier Goffart        <ogoffart @ kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "systemtray.h"

#include <qtimer.h>
#include <qtooltip.h>
#include <qregexp.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QPixmap>
#include <QEvent>

#include <kwin.h>
#include <kaboutdata.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <klocale.h>	
#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include "kopeteuiglobal.h"
#include "kopetechatsessionmanager.h"
#include "kopeteballoon.h"
#include "kopetebehaviorsettings.h"
#include "kopetemetacontact.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetecontact.h"
#include "kopetewindow.h"

KopeteSystemTray* KopeteSystemTray::s_systemTray = 0L;

KopeteSystemTray* KopeteSystemTray::systemTray( QWidget *parent )
{
	if( !s_systemTray )
		s_systemTray = new KopeteSystemTray( parent  );

	return s_systemTray;
}

KopeteSystemTray::KopeteSystemTray(QWidget* parent)
	: KSystemTrayIcon(parent), mMovie(0)
{
//	kDebug(14010) << "Creating KopeteSystemTray" << endl;
	setToolTip(kapp->aboutData()->shortDescription());

	mIsBlinkIcon = false;
	mIsBlinking = false;
	mBlinkTimer = new QTimer(this);
	mBlinkTimer->setObjectName("mBlinkTimer");

	mKopeteIcon = loadIcon("kopete");

	connect(mBlinkTimer, SIGNAL(timeout()), this, SLOT(slotBlink()));
	connect(Kopete::ChatSessionManager::self() , SIGNAL(newEvent(Kopete::MessageEvent*)),
		this, SLOT(slotNewEvent(Kopete::MessageEvent*)));
	connect(Kopete::BehaviorSettings::self(), SIGNAL(configChanged()), this, SLOT(slotConfigChanged()));

	connect(Kopete::AccountManager::self(),
		SIGNAL(accountOnlineStatusChanged(Kopete::Account *,
		const Kopete::OnlineStatus &, const Kopete::OnlineStatus &)),
	this, SLOT(slotReevaluateAccountStates()));

	// the slot called by default by the quit action, KSystemTray::maybeQuit(),
	// just closes the parent window, which is hard to distinguish in that window's closeEvent()
	// from a click on the window's close widget
	// in the quit case, we want to quit the application
 	// in the close widget click case, we only want to hide the parent window
	// so instead, we make it call our general purpose quit slot on the window, which causes a window close and everything else we need
	// KDE4 - app will have to listen for quitSelected instead
	KAction *quit = actionCollection()->action( "file_quit" );
	quit->disconnect();
	KopeteWindow *myParent = static_cast<KopeteWindow *>( parent );
    connect( quit, SIGNAL( activated() ), myParent, SLOT( slotQuit() ) );

	//setPixmap(mKopeteIcon);
	slotReevaluateAccountStates();
	slotConfigChanged();

	m_balloon=0l;
}

KopeteSystemTray::~KopeteSystemTray()
{
//	kDebug(14010) << "[KopeteSystemTray] ~KopeteSystemTray" << endl;
//	delete mBlinkTimer;
	Kopete::UI::Global::setSysTrayWId( 0 );
	delete mMovie;
}

void KopeteSystemTray::mousePressEvent( QMouseEvent *me )
{
#warning PORT ME
#if 0
	if (
		(me->button() == Qt::MidButton ||
			(me->button() == Qt::LeftButton && Kopete::BehaviorSettings::self()->trayflashNotifyLeftClickOpensMessage())) &&
		mIsBlinking )
	{
		mouseDoubleClickEvent( me );
		return;
	}

	KSystemTray::mousePressEvent( me );
#endif
}

void KopeteSystemTray::mouseDoubleClickEvent( QMouseEvent *me )
{
#warning PORT ME
#if 0
	if ( !mIsBlinking )
	{
		KSystemTray::mousePressEvent( me );
	}
	else
	{
		if(!mEventList.isEmpty())
			mEventList.first()->apply();
	}
#endif
}

void KopeteSystemTray::contextMenuAboutToShow( KMenu *me )
{
	//kDebug(14010) << k_funcinfo << "Called." << endl;
	emit aboutToShowMenu( me );
}

void KopeteSystemTray::startBlink( const QString &icon )
{
	startBlink( loadIcon( icon ) );
}

void KopeteSystemTray::startBlink( const QIcon &icon )
{
	mBlinkIcon = icon;
	if ( mBlinkTimer->isActive() == false )
	{
		mIsBlinkIcon = true;
		mIsBlinking = true;
		mBlinkTimer->setSingleShot( false );
		mBlinkTimer->start( 1000 );
	}
	else
	{
		mBlinkTimer->stop();
		mIsBlinkIcon = true;
		mIsBlinking = true;
		mBlinkTimer->setSingleShot( false );
		mBlinkTimer->start( 1000 );
	}
}

void KopeteSystemTray::startBlink( QMovie *movie )
{
#warning PORT ME
#if 0
	//kDebug( 14010 ) << k_funcinfo << "starting movie." << endl;
	kDebug( 14010 ) << "Movie is " << movie->loopCount() << " loops, " << movie->frameCount() << " frames " << endl;
	movie->setPaused(false);
	setMovie( movie );
	mIsBlinking = true;
#endif
}

void KopeteSystemTray::startBlink()
{
	if ( !mMovie )
		mMovie = KGlobal::iconLoader()->loadMovie( QLatin1String( "newmessage" ), K3Icon::Panel );
	// KIconLoader already checked isValid()
	if ( !mMovie) return;
	
	startBlink( mMovie );
}

void KopeteSystemTray::stopBlink()
{
	if ( mMovie )
		kDebug( 14010 ) << k_funcinfo << "stopping movie." << endl;
	else if ( mBlinkTimer->isActive() )
		mBlinkTimer->stop();

	if ( mMovie )
		mMovie->setPaused(true);

	mIsBlinkIcon = false;
	mIsBlinking = false;
	//setPixmap( mKopeteIcon );
	slotReevaluateAccountStates();
}

void KopeteSystemTray::slotBlink()
{
	setIcon( mIsBlinkIcon ? mKopeteIcon : mBlinkIcon );

	mIsBlinkIcon = !mIsBlinkIcon;
}

void KopeteSystemTray::slotNewEvent( Kopete::MessageEvent *event )
{
	if( Kopete::BehaviorSettings::self()->useMessageStack() )
	{
		mEventList.prepend( event );
		mBalloonEventList.prepend( event );
	}
	else
	{
		mEventList.append( event );
		mBalloonEventList.append( event );
	}

	connect(event, SIGNAL(done(Kopete::MessageEvent*)),
		this, SLOT(slotEventDone(Kopete::MessageEvent*)));

	if( event->message().manager() != 0 )
	{
		if( event->message().manager()->account() )
		{
			if( !event->message().manager()->account()->isAway() ||
				Kopete::BehaviorSettings::self()->enableEventsWhileAway() )
			{
				addBalloon();
			}
			else
			{
				kDebug(14000) << k_funcinfo << "Supressing balloon, account is away" << endl;
			}
		}
	}
	else
		kDebug(14000) << k_funcinfo << "NULL message().manager()!" << endl;

	// tray animation
	if ( Kopete::BehaviorSettings::self()->trayflashNotify() )
		if( mBalloonEventList.count() == mEventList.count() )
			startBlink();
		else
			stopBlink();
}

void KopeteSystemTray::slotEventDone(Kopete::MessageEvent *event)
{
	mEventList.removeAll(event);

	removeBalloonEvent(event);

	if(mEventList.isEmpty())
		stopBlink();
}

void KopeteSystemTray::slotRemoveBalloon()
{
	removeBalloonEvent(mBalloonEventList.first());
}

void KopeteSystemTray::removeBalloonEvent(Kopete::MessageEvent *event)
{
#warning PORT ME
#if 0
	bool current= event==mBalloonEventList.first();
	mBalloonEventList.removeAll(event);

	if(current && m_balloon)
	{
		m_balloon->deleteLater();
		m_balloon=0l;
		if(!mBalloonEventList.isEmpty())
		{
			//delay the addBalloon to let the time to event be deleted
			//in case a contact has been deleted   cf Bug 100196
			QTimer::singleShot(0, this, SLOT(addBalloon()));
		}
		else
		{
			if(Kopete::BehaviorSettings::self()->trayflashNotify() && !mEventList.isEmpty())
				startBlink();
		}
	}
#endif
}

void KopeteSystemTray::addBalloon()
{
#warning PORT ME
#if 0
	/*kDebug(14010) << k_funcinfo <<
		m_balloon << ":" << KopetePrefs::prefs()->showTray() <<
		":" << KopetePrefs::prefs()->balloonNotify()
		<< ":" << !mBalloonEventList.isEmpty() << endl;*/

	if( m_balloon && Kopete::BehaviorSettings::self()->useMessageStack() )
	{
		m_balloon->deleteLater();
		m_balloon=0l;
	}

	if( !m_balloon && Kopete::BehaviorSettings::self()->showSystemTray() && Kopete::BehaviorSettings::self()->balloonNotify() && !mBalloonEventList.isEmpty() )
	{
		Kopete::Message msg = mBalloonEventList.first()->message();

		if ( msg.from() )
		{
			QString msgText = squashMessage( msg );
			kDebug(14010) << k_funcinfo << "msg text=" << msgText << endl;

			QString msgFrom;
			if( msg.from()->metaContact() )
				msgFrom = msg.from()->metaContact()->displayName();
			else
				msgFrom = msg.from()->contactId();
			m_balloon = new KopeteBalloon(
				i18n( "<qt><nobr><b>New Message from %1:</b></nobr><br><nobr>\"%2\"</nobr></qt>",
				      msgFrom, msgText ), QString::null );
			connect(m_balloon, SIGNAL(signalBalloonClicked()), mBalloonEventList.first() , SLOT(apply()));
			connect(m_balloon, SIGNAL(signalButtonClicked()), mBalloonEventList.first() , SLOT(apply()));
			connect(m_balloon, SIGNAL(signalIgnoreButtonClicked()), mBalloonEventList.first() , SLOT(ignore()));
			connect(m_balloon, SIGNAL(signalTimeout()), this , SLOT(slotRemoveBalloon()));
			m_balloon->setAnchor(mapToGlobal(pos()));
			m_balloon->show();
			KWin::setOnAllDesktops(m_balloon->winId(), true);
		}
	}
#endif
}

void KopeteSystemTray::slotConfigChanged()
{
//	kDebug(14010) << k_funcinfo << "called." << endl;
	if ( Kopete::BehaviorSettings::self()->showSystemTray() )
		show();
	else
		hide(); // for users without kicker or a similar docking app
}

void KopeteSystemTray::slotReevaluateAccountStates()
{
	// If there is a pending message, we don't need to refresh the system tray now.
	// This function will even be called when the animation will stop.
	if ( mIsBlinking )
		return;

	
	//kDebug(14010) << k_funcinfo << endl;
	bool bOnline = false;
	bool bAway = false;
	bool bOffline = false;
	Kopete::Contact *c = 0;

	QListIterator<Kopete::Account *> it(Kopete::AccountManager::self()->accounts());
	while ( it.hasNext() )
	{
		c = it.next()->myself();
		if (!c)
			continue;

		if (c->onlineStatus().status() == Kopete::OnlineStatus::Online)
		{
			bOnline = true; // at least one contact is online
		}
		else if (c->onlineStatus().status() == Kopete::OnlineStatus::Away
		      || c->onlineStatus().status() == Kopete::OnlineStatus::Invisible)
		{
			bAway = true; // at least one contact is away or invisible
		}
		else // this account must be offline (or unknown, which I don't know how to handle)
		{
			bOffline = true;
		}
	}

	if (!bOnline && !bAway && !bOffline) // special case, no accounts defined (yet)
		bOffline = true;

	if (bAway)
	{
		if (!bOnline && !bOffline) // none online and none offline -> all away
			setIcon(loadIcon("kopete_all_away"));
		else
			setIcon(loadIcon("kopete_some_away"));
	}
	else if(bOnline)
	{
		/*if(bOffline) // at least one offline and at least one online -> some accounts online
			setIcon(loadIcon("kopete_some_online"));
		else*/ // none offline and none away -> all online
			setIcon(mKopeteIcon);
	}
	else // none away and none online -> all offline
	{
		//kDebug(14010) << k_funcinfo << "All Accounts offline!" << endl;
		setIcon(loadIcon("kopete_offline"));
	}
}


QString KopeteSystemTray::squashMessage( const Kopete::Message& msg )
{
	QString msgText = msg.parsedBody();

	QRegExp rx( "(<a.*>((http://)?(.+))</a>)" );
	rx.setMinimal( true );
	if ( rx.indexIn( msgText ) == -1 )
	{
		// no URLs in text, just pick the first 30 chars of
		// the parsed text if necessary. We used parsed text
		// so that things like "<knuff>" show correctly
		//  Escape it after snipping it to not snip entities
		msgText =msg.plainBody() ;
		if( msgText.length() > 30 )
			msgText = msgText.left( 30 ) + QLatin1String( " ..." );
		msgText=Kopete::Message::escape(msgText);
	}
	else
	{
		QString plainText = msg.plainBody();
		if ( plainText.length() > 30 )
		{
			QString fullUrl = rx.cap( 2 );
			QString shorterUrl;
			if ( fullUrl.length() > 30 )
			{
				QString urlWithoutProtocol = rx.cap( 4 );
				shorterUrl = urlWithoutProtocol.left( 27 )
						+ QLatin1String( "... " );
			}
			else
			{
				shorterUrl = fullUrl.left( 27 )
						+ QLatin1String( "... " );
			}
			// remove message text
			msgText = QLatin1String( "... " ) +
					rx.cap( 1 ) +
					QLatin1String( " ..." );
			// find last occurrence of URL (the one inside the <a> tag)
			int revUrlOffset = msgText.lastIndexOf( fullUrl );
			msgText.replace( revUrlOffset,
						fullUrl.length(), shorterUrl );
		}
	}
	return msgText;
}

#include "systemtray.moc"
// vim: set noet ts=4 sts=4 sw=4:
