/*
    systemtray.cpp  -  Kopete Tray Dock Icon

    Copyright (c) 2002      by Nick Betcher           <nbetcher@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2003-2004 by Olivier Goffart        <ogoffart@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

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

#include <qregexp.h>
#include <QMouseEvent>
#include <QPixmap>
#include <QEvent>
#include <QPainter>

#include <kaboutdata.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <kmenu.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include "kopeteuiglobal.h"
#include "kopetechatsessionmanager.h"
#include "kopetebehaviorsettings.h"
#include "kopetemetacontact.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetecontact.h"
#include "kopetewindow.h"
#include <kiconeffect.h>

KopeteSystemTray* KopeteSystemTray::s_systemTray = 0;

KopeteSystemTray* KopeteSystemTray::systemTray( QWidget *parent )
{
	if( !s_systemTray )
		s_systemTray = new KopeteSystemTray( parent );

	return s_systemTray;
}

KopeteSystemTray::KopeteSystemTray(QWidget* parent)
	: KSystemTrayIcon(parent)
	, mMovie(0)
{
	kDebug(14010) ;
	setToolTip(KGlobal::mainComponent().aboutData()->shortDescription());

	mIsBlinkIcon = false;
	mIsBlinking = false;
	mBlinkTimer = new QTimer(this);
	mBlinkTimer->setObjectName("mBlinkTimer");

	mKopeteIcon = loadIcon("kopete");

	// Hack which allow us to disable window restoring or hiding when we should process event (BUG:157663)
	disconnect( this, SIGNAL(activated( QSystemTrayIcon::ActivationReason )), 0 ,0 );

	connect(contextMenu(), SIGNAL(aboutToShow()), this, SLOT(slotAboutToShowMenu()));

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
	QAction *quit = actionCollection()->action( "file_quit" );
	quit->disconnect();
	KopeteWindow *myParent = static_cast<KopeteWindow *>( parent );
	connect( quit, SIGNAL( activated() ), myParent, SLOT( slotQuit() ) );
	connect( this, SIGNAL( activated( QSystemTrayIcon::ActivationReason ) ), SLOT( slotActivated( QSystemTrayIcon::ActivationReason ) ) );

	setIcon(mKopeteIcon);
	slotReevaluateAccountStates();
	slotConfigChanged();
}

KopeteSystemTray::~KopeteSystemTray()
{
	kDebug(14010) ;
//	delete mBlinkTimer;
}

void KopeteSystemTray::slotAboutToShowMenu()
{
	emit aboutToShowMenu(qobject_cast<KMenu *>(contextMenu()));
}

void KopeteSystemTray::slotActivated( QSystemTrayIcon::ActivationReason reason )
{
	bool shouldProcessEvent(
		reason == QSystemTrayIcon::MiddleClick
		|| reason == QSystemTrayIcon::DoubleClick
		|| ( reason == QSystemTrayIcon::Trigger
			&& Kopete::BehaviorSettings::self()->trayflashNotifyLeftClickOpensMessage()));
	if ( isBlinking() && shouldProcessEvent )
	{
		if ( !mEventList.isEmpty() )
			mEventList.first()->apply();
	}
	else if ( reason == QSystemTrayIcon::Trigger )
	{
		toggleActive();
	}
}

// void KopeteSystemTray::contextMenuAboutToShow( KMenu *me )
// {
// 	//kDebug(14010) << "Called.";
// 	emit aboutToShowMenu( me );
// }

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

void KopeteSystemTray::startBlink()
{
	if ( !mMovie )
		mMovie = KIconLoader::global()->loadMovie( QLatin1String( "newmessage" ), KIconLoader::Panel );
	// KIconLoader already checked isValid()
	if ( !mMovie) return;

	if (!movie())
		setMovie( mMovie );
	mMovie->start();
}

void KopeteSystemTray::stopBlink()
{
	if ( mMovie )
		kDebug( 14010 ) << "stopping movie.";
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
	mEventList.append( event );

	connect(event, SIGNAL(done(Kopete::MessageEvent*)),
		this, SLOT(slotEventDone(Kopete::MessageEvent*)));

	// tray animation
	if ( Kopete::BehaviorSettings::self()->trayflashNotify() )
		startBlink();
}

void KopeteSystemTray::slotEventDone(Kopete::MessageEvent *event)
{
	mEventList.removeAll(event);

	if(mEventList.isEmpty())
		stopBlink();
}

void KopeteSystemTray::slotConfigChanged()
{
//	kDebug(14010) << "called.";
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

	Kopete::OnlineStatus highestStatus;
	foreach ( Kopete::Account *account, Kopete::AccountManager::self()->accounts())
	{
		if ( account->myself() && account->myself()->onlineStatus() > highestStatus )
		{
			highestStatus = account->myself()->onlineStatus();
		}
	}

	switch ( highestStatus.status() )
	{
		case Kopete::OnlineStatus::Unknown:
		case Kopete::OnlineStatus::Offline:
		case Kopete::OnlineStatus::Connecting:
		{
			QImage offlineIcon = mKopeteIcon.pixmap(22,22).toImage();
			KIconEffect::toGray( offlineIcon, 0.85f );
			setIcon( QPixmap::fromImage( offlineIcon ) );
			break;
		}
		case Kopete::OnlineStatus::Invisible:
		{
			QPixmap statusOverlay = loadIcon("user-invisible").pixmap(11,11);
			QPixmap statusIcon = mKopeteIcon.pixmap(22,22);
			if (!statusIcon.isNull() && !statusOverlay.isNull())
			{
				QPainter painter(&statusIcon);
				painter.drawPixmap(QPoint(11,11), statusOverlay);
			}
			setIcon( statusIcon );
			break;
		}
		case Kopete::OnlineStatus::Away:
		{
			QPixmap statusOverlay = loadIcon("user-away").pixmap(11,11);
			QPixmap statusIcon = mKopeteIcon.pixmap(22,22);
			if (!statusIcon.isNull() && !statusOverlay.isNull())
			{
				QPainter painter(&statusIcon);
				painter.drawPixmap(QPoint(11,11), statusOverlay);
			}
			setIcon( statusIcon );
			break;
		}
		case Kopete::OnlineStatus::Online:
			setIcon( mKopeteIcon );
			break;
	}
}

#include "systemtray.moc"
// vim: set noet ts=4 sts=4 sw=4:
