/*
    systemtray.cpp  -  Kopete Tray Dock Icon

    Copyright (c) 2002      by Nick Betcher           <nbetcher@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2003      by Olivier Goffart        <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

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

#include <qcursor.h>
//#include <qpixmap.h>
#include <qpoint.h>
#include <qtimer.h>
#include <qtooltip.h>

#include <klocale.h> 
#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include "kopeteevent.h"
#include "kopeteviewmanager.h"
#include "kopetemessage.h"
#include "kopeteballoon.h"
#include "kopeteprefs.h"
#include "kopetemetacontact.h"
#include "kopeteprotocol.h"

KopeteSystemTray* KopeteSystemTray::s_systemTray = 0L;

KopeteSystemTray* KopeteSystemTray::systemTray( QWidget *parent, const char* name )
{
	if( !s_systemTray )
		s_systemTray = new KopeteSystemTray( parent, name );

	return s_systemTray;
}

KopeteSystemTray::KopeteSystemTray(QWidget* parent, const char* name)
	: KSystemTray(parent,name)
{
//	kdDebug(14010) << "Creating KopeteSystemTray" << endl;
	QToolTip::add( this, kapp->aboutData()->shortDescription() );

	mIsBlinkIcon = false;
	mIsBlinking = false;
	mBlinkTimer = new QTimer(this, "mBlinkTimer");

	//mKopeteIcon = kapp->miniIcon(); 
	mKopeteIcon = QPixmap( BarIcon( QString::fromLatin1( "kopete" ) ) );
	mBlinkIcon = KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "newmsg" ), KIcon::User);
	mMovie = KGlobal::iconLoader()->loadMovie( QString::fromLatin1( "newmessage" ), KIcon::User);

	QObject::connect(mBlinkTimer, SIGNAL(timeout()), this, SLOT(slotBlink()));

	connect(KopeteViewManager::viewManager() , SIGNAL(newMessageEvent(KopeteEvent*)),
		this, SLOT(slotNewEvent(KopeteEvent*)));

	setPixmap(mKopeteIcon);

	connect ( KopetePrefs::prefs(), SIGNAL(saved()), this, SLOT(slotConfigChanged()) );
	slotConfigChanged();

	m_balloon=0l;
}

KopeteSystemTray::~KopeteSystemTray()
{
//	kdDebug(14010) << "[KopeteSystemTray] ~KopeteSystemTray" << endl;
//	delete mBlinkTimer;
}

void KopeteSystemTray::mousePressEvent( QMouseEvent *me )
{
	if ( me->button() == QEvent::MidButton )
	{
		if ( mIsBlinking )
		{
			mouseDoubleClickEvent( me );
			return;
		}
	}
	KSystemTray::mousePressEvent( me );
}

void KopeteSystemTray::mouseDoubleClickEvent( QMouseEvent *me )
{
	if ( !mIsBlinking )
	{
		KSystemTray::mousePressEvent( me );
	}
	else
	{
		if(!mEventList.isEmpty())
			mEventList.first()->apply();
	}
}


void KopeteSystemTray::startBlink( QString icon )
{
	mBlinkIcon = QPixmap ( KGlobal::iconLoader()->loadIcon( icon , KIcon::User) );
	startBlinkPrivate();
}

void KopeteSystemTray::startBlink( QPixmap icon )
{
	mBlinkIcon = icon;
	startBlinkPrivate();
}

void KopeteSystemTray::startBlink( QMovie icon )
{
	kdDebug(14010) << k_funcinfo << "starting movie." << endl;
	setMovie(icon);
	mIsBlinking = true;
}

void KopeteSystemTray::startBlink()
{
//	mBlinkIcon = QPixmap ( KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "newmsg" ), KIcon::User) );
//	startBlinkPrivate();
	startBlink(mMovie);
}

void KopeteSystemTray::startBlinkPrivate()
{
	if (mBlinkTimer->isActive() == false)
	{
		mIsBlinkIcon = true;
		mIsBlinking = true;
		mBlinkTimer->start(1000, false);
	}
	else
	{
		mBlinkTimer->stop();
		mIsBlinkIcon = true;
		mIsBlinking = true;
		mBlinkTimer->start(1000, false);
	}
}

void KopeteSystemTray::stopBlink()
{
	if(movie())
	{
		kdDebug(14010) << k_funcinfo << "stopping movie." << endl;
		setPixmap(mKopeteIcon);
		mIsBlinkIcon = false;
		mIsBlinking=false;
		return;
	}

	if (mBlinkTimer->isActive() == true)
	{
		mBlinkTimer->stop();
		setPixmap(mKopeteIcon);
		mIsBlinkIcon = false;
		mIsBlinking = false;

	}
	else
	{
		setPixmap(mKopeteIcon);
		mIsBlinkIcon = false;
		mIsBlinking = false;
	}
}

void KopeteSystemTray::slotBlink()
{
	if (mIsBlinkIcon == true)
	{
		setPixmap(mKopeteIcon);
		mIsBlinkIcon = false;
	}
	else
	{
		setPixmap(mBlinkIcon);
		mIsBlinkIcon = true;
	}
}

void KopeteSystemTray::slotNewEvent(KopeteEvent *event)
{
	mEventList.append( event );
	connect( event , SIGNAL(done(KopeteEvent*)) , this, SLOT(slotEventDone(KopeteEvent*)));
	
	//balloon
	addBalloon();

	//flash
	if ( KopetePrefs::prefs()->trayflashNotify() )
		startBlink();
}


void KopeteSystemTray::slotEventDone(KopeteEvent *event)
{
	bool current= event==mEventList.first();
	mEventList.remove(event);

	if(current && m_balloon)
	{
		m_balloon->deleteLater();
		m_balloon=0l;
		if(!mEventList.isEmpty())
			addBalloon();
	}

	if(mEventList.isEmpty())
		stopBlink();
}

void KopeteSystemTray::addBalloon()
{
	if( !m_balloon && KopetePrefs::prefs()->showTray() && KopetePrefs::prefs()->balloonNotify() && !mEventList.isEmpty() )
	{
		KopeteMessage msg = mEventList.first()->message();
		if( (!msg.from()->protocol()->isAway() || KopetePrefs::prefs()->soundIfAway()) &&
			msg.from() && msg.from()->metaContact() )
		{
			QString msgText = msg.plainBody();
			if( msgText.length() > 30 )
				msgText = msgText.left(30) + QString::fromLatin1("...");

			m_balloon= new KopeteBalloon(
				i18n("<qt><nobr><b>New Message from %1:</b></nobr><br><br><nobr>\"%2\"</nobr></qt>")
					.arg( msg.from()->metaContact()->displayName() ).arg( msgText ), QString::null );
			connect(m_balloon, SIGNAL(signalBalloonClicked()), mEventList.first() , SLOT(apply()));
			connect(m_balloon, SIGNAL(signalButtonClicked()), mEventList.first() , SLOT(apply()));
			connect(m_balloon, SIGNAL(signalIgnoreButtonClicked()), mEventList.first() , SLOT(ignore()));
			m_balloon->setAnchor( KopeteSystemTray::systemTray()->mapToGlobal(pos()) );
			m_balloon->show();
		}
	}

}

void KopeteSystemTray::slotConfigChanged()
{
	kdDebug(14010) << k_funcinfo << "called." << endl;
	if ( KopetePrefs::prefs()->showTray())
		show();
	else
		hide(); // for users without kicker or a similar docking app
}

#include "systemtray.moc"

// vim: set noet ts=4 sts=4 sw=4:
