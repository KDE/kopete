/*
    kopeteidletimer.cpp  -  Kopete Idle Timer

    Copyright (c) 2002      by Hendrik vom Lehn      <hvl@linux-4-ever.de>
    Copyright (c) 2003      by Olivier Goffart       <ogoffart@kde.org>
    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include <config-kopete.h>

#include "kopeteidletimer.h"

#include <QtCore/QTimer>
#include <QtCore/QTime>
#include <QtDBus/QtDBus>

#include <kdebug.h>

#ifdef Q_WS_X11
  #include <X11/Xlib.h>
  #include <X11/Xatom.h>
  #include <X11/Xresource.h>
  // The following include is to make --enable-final work
  #include <X11/Xutil.h>

  #ifdef HAVE_XSCREENSAVER
    #define HasScreenSaver
    #include <X11/extensions/scrnsaver.h>
    #include <QX11Info>
  #endif
#endif // Q_WS_X11

// As this is an untested X extension we better leave it off
#undef HAVE_XIDLE
#undef HasXidle

class Kopete::IdleTimer::Private
{
public:
	QTime idleTime;
	struct TimeoutReceiver {
		bool active;
		int msec;
		QObject * receiver;
		const char * memberActive;
		const char * memberIdle;
	};
	QList<TimeoutReceiver> receiverList;
	QTimer *timer;

	int mouse_x;
	int mouse_y;
	unsigned int mouse_mask;
#ifdef Q_WS_X11
	Window    root;               /* root window the pointer is on */
	Screen*   screen;             /* screen the pointer is on      */
	
	Time xIdleTime;
#endif
	bool useXidle;
	bool useMit;
};

Kopete::IdleTimer *Kopete::IdleTimer::instance = 0L;

Kopete::IdleTimer::IdleTimer()
: QObject(), d( new Private )
{
	int dummy = 0;	

	// set the XAutoLock info
#ifdef Q_WS_X11
	Display *dsp = QX11Info::display();
#endif
	d->mouse_x = d->mouse_y=0;
	d->mouse_mask = 0;
#ifdef Q_WS_X11
	d->root = DefaultRootWindow (dsp);
	d->screen = ScreenOfDisplay (dsp, DefaultScreen (dsp));
#endif
	d->useXidle = false;
	d->useMit = false;
#ifdef HasXidle
	d->useXidle = XidleQueryExtension(QX11Info::display(), &dummy, &dummy);
#endif
#ifdef HasScreenSaver
	if(!d->useXidle)
		d->useMit = XScreenSaverQueryExtension(QX11Info::display(), &dummy, &dummy);
#endif
#ifdef Q_WS_X11
	d->xIdleTime = 0;
#endif
	kDebug(14010) << k_funcinfo << "Idle detection methods:";
	kDebug(14010) << k_funcinfo << "\tKScreensaverIface::isBlanked()";
#ifdef Q_WS_X11
	kDebug(14010) << k_funcinfo << "\tX11 XQueryPointer()";
#endif
	if (d->useXidle)
	{
		kDebug(14010) << k_funcinfo << "\tX11 Xidle extension";
	}
	if (d->useMit)
	{
		kDebug(14010) << k_funcinfo << "\tX11 MIT Screensaver extension";
	}

	// init the timer
	d->timer = new QTimer(this);
	connect(d->timer, SIGNAL(timeout()), this, SLOT(slotTimerTimeout()));
	d->timer->start(4000);

	d->idleTime.start();
}

Kopete::IdleTimer *Kopete::IdleTimer::self()
{
	if ( !instance )
		instance = new IdleTimer;
	
	return instance;
}

Kopete::IdleTimer::~IdleTimer()
{
	delete d;
}

int Kopete::IdleTimer::idleTime()
{
	//FIXME: the time is reset to zero if more than 24 hours are elapsed
	// we can imagine someone who leave his PC for several weeks
	return (d->idleTime.elapsed() / 1000);
}

void Kopete::IdleTimer::registerTimeout( int idleSeconds, QObject *receiver,
                                         const char *memberActive, const char *memberIdle )
{
	Private::TimeoutReceiver item = { true, idleSeconds * 1000, receiver, memberActive, memberIdle };
	d->receiverList.append( item );
	connect( receiver, SIGNAL(destroyed(QObject*)), this, SLOT(unregisterTimeout(QObject*)) );
}

void Kopete::IdleTimer::unregisterTimeout( QObject *receiver )
{
	for ( int i = 0; i < d->receiverList.size(); )
	{
		if ( d->receiverList.at(i).receiver == receiver )
		{
			d->receiverList.removeAt( i );
			continue;
		}

		i++;
	}
}

void Kopete::IdleTimer::slotTimerTimeout()
{
#ifdef __GNUC__
#warning verify dcop call
#endif
	QDBusInterface caller("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver");
	QDBusReply<bool> reply = caller.call("GetActive");
	
	bool activity = ( !reply.isValid() || !reply.value() ) ? isActivity() : true;

	//kDebug(14010) << "idle: " << idleTime() << " active:" << activity;
	if ( activity )
		d->idleTime.start();

	for ( int i = 0; i < d->receiverList.size(); i++ )
	{
		Private::TimeoutReceiver item = d->receiverList.at(i);
		if ( item.active != activity && (activity == true || d->idleTime.elapsed() > item.msec ) )
		{
			d->receiverList[i].active = activity;
			if ( activity )
				QTimer::singleShot( 0, item.receiver, item.memberActive );
			else
				QTimer::singleShot( 0, item.receiver, item.memberIdle );
		}
	}
}

bool Kopete::IdleTimer::isActivity()
{
	// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
	//
	// KDE screensaver engine
	//
	// This module is a heavily modified xautolock.
	// In fact as of KDE 2.0 this code is practically unrecognisable as xautolock.
	
	bool activity = false;
	
#ifdef Q_WS_X11
	Display *dsp = QX11Info::display();
	Window           dummy_w;
	int              dummy_c;
	unsigned int     mask;               /* modifier mask                 */
	int              root_x;
	int              root_y;
	
	/*
	*  Find out whether the pointer has moved. Using XQueryPointer for this
	*  is gross, but it also is the only way never to mess up propagation
	*  of pointer events.
	*
	*  Remark : Unlike XNextEvent(), XPending () doesn't notice if the
	*           connection to the server is lost. For this reason, earlier
	*           versions of xautolock periodically called XNoOp (). But
	*           why not let XQueryPointer () do the job for us, since
	*           we now call that periodically anyway?
	*/
	if (!XQueryPointer (dsp, d->root, &(d->root), &dummy_w, &root_x, &root_y,
	                    &dummy_c, &dummy_c, &mask))
	{
		/*
		*  Pointer has moved to another screen, so let's find out which one.
		*/
		for (int i = 0; i < ScreenCount(dsp); i++)
		{
			if (d->root == RootWindow(dsp, i))
			{
				d->screen = ScreenOfDisplay (dsp, i);
				break;
			}
		}
	}
	
	// =================================================================================
	
	Time xIdleTime = 0; // millisecs since last input event
	
#ifdef HasXidle
	if (d->useXidle)
	{
		XGetIdleTime(dsp, &xIdleTime);
	}
	else
#endif /* HasXIdle */
		
	{
#ifdef HasScreenSaver
		if(d->useMit)
		{
			static XScreenSaverInfo* mitInfo = 0;
			if (!mitInfo) mitInfo = XScreenSaverAllocInfo();
			XScreenSaverQueryInfo (dsp, d->root, mitInfo);
			xIdleTime = mitInfo->idle;
		}
#endif /* HasScreenSaver */
	}
	
	// =================================================================================
	
	// Only check idle time if we have some way of measuring it, otherwise if
	// we've neither Mit nor Xidle it'll still be zero and we'll always appear active.
	// FIXME: what problem does the 2000ms fudge solve?
	if (root_x != d->mouse_x || root_y != d->mouse_y || mask != d->mouse_mask
	    || ((d->useXidle || d->useMit) && xIdleTime < d->xIdleTime + 2000))
	{
		// -1 => just gone autoaway, ignore apparent activity this time round
		// anything else => genuine activity
		// See setAutoAway().
		if (d->mouse_x != -1)
		{
			activity = true;
		}
		d->mouse_x = root_x;
		d->mouse_y = root_y;
		d->mouse_mask = mask;
		d->xIdleTime = xIdleTime;
	}
#endif // Q_WS_X11

	return activity;
}

#include "kopeteidletimer.moc"
// vim: set et ts=4 sts=4 sw=4:
