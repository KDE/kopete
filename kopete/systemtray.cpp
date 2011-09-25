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

#include <kaboutdata.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <kmenu.h>
#include <klocale.h>
#include <kdebug.h>
#include "kopetechatsessionmanager.h"
#include "kopetebehaviorsettings.h"
#include "kopetemetacontact.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetecontact.h"
#include "kopetewindow.h"

KopeteSystemTray* KopeteSystemTray::s_systemTray = 0;

KopeteSystemTray* KopeteSystemTray::systemTray( QWidget *parent )
{
	if( !s_systemTray )
		s_systemTray = new KopeteSystemTray( parent );

	return s_systemTray;
}

KopeteSystemTray::KopeteSystemTray(QWidget* parent)
	: KStatusNotifierItem(parent)
{
	kDebug(14010) ;
    setCategory(Communications);
	setToolTip("kopete", "Kopete", KGlobal::mainComponent().aboutData()->shortDescription());
	setStatus(Passive);

	mIsBlinkIcon = false;
	mBlinkTimer = new QTimer(this);
	mBlinkTimer->setObjectName("mBlinkTimer");

	mKopeteIcon = "kopete";

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
	connect( quit, SIGNAL(activated()), myParent, SLOT(slotQuit()) );

	setIconByName(mKopeteIcon);
	setAttentionMovieByName( QLatin1String( "newmessage" ) );
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

void KopeteSystemTray::activate(const QPoint &pos)
{
	if ( isBlinking() &&  Kopete::BehaviorSettings::self()->trayflashNotifyLeftClickOpensMessage() )
	{
		if ( !mEventList.isEmpty() )
			mEventList.first()->apply();
	}
    else
    {
        KStatusNotifierItem::activate(pos);
    }
}

// void KopeteSystemTray::contextMenuAboutToShow( KMenu *me )
// {
// 	//kDebug(14010) << "Called.";
// 	emit aboutToShowMenu( me );
// }


void KopeteSystemTray::startBlink( const QString &icon )
{
	mBlinkIcon = icon;
	if ( mBlinkTimer->isActive() )
	{
		mBlinkTimer->stop();
	}
	mIsBlinkIcon = true;
	mBlinkTimer->setSingleShot( false );
	mBlinkTimer->start( 1000 );
}

void KopeteSystemTray::startBlink()
{
    setStatus(NeedsAttention);
}

void KopeteSystemTray::stopBlink()
{
    setStatus(Passive);

	if ( mBlinkTimer->isActive() )
		mBlinkTimer->stop();

	mIsBlinkIcon = false;
	//setPixmap( mKopeteIcon );
	slotReevaluateAccountStates();
}

void KopeteSystemTray::slotBlink()
{
	setIconByName( mIsBlinkIcon ? mKopeteIcon : mBlinkIcon );

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
#if 0
//	kDebug(14010) << "called.";
	if ( Kopete::BehaviorSettings::self()->showSystemTray() )
		show();
	else
		hide(); // for users without kicker or a similar docking app
#endif
}

void KopeteSystemTray::slotReevaluateAccountStates()
{
	// If there is a pending message, we don't need to refresh the system tray now.
	// This function will even be called when the animation will stop.
	if ( mBlinkTimer->isActive() )
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
			setIconByName("kopete-offline");
			setOverlayIconByName("user-offline");
			break;
		}
		case Kopete::OnlineStatus::Invisible:
		{
			setIconByName(mKopeteIcon);
			setOverlayIconByName("user-invisible");
			break;
		}
		case Kopete::OnlineStatus::Away:
		{
			setIconByName(mKopeteIcon);
			setOverlayIconByName("user-away");
			break;
		}
		case Kopete::OnlineStatus::Busy:
		{
			setIconByName(mKopeteIcon);
			setOverlayIconByName("user-busy");
			break;
		}
		case Kopete::OnlineStatus::Online:
		{
			setIconByName(mKopeteIcon);
			setOverlayIconByName(QString());
			break;
		}
	}
}


bool KopeteSystemTray::isBlinking() const
{
	return mBlinkTimer->isActive() || (status() == NeedsAttention);
}


#include "systemtray.moc"
// vim: set noet ts=4 sts=4 sw=4:
