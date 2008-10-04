/*
    kopeteidleplatform_x11.cpp  -  Kopete Idle Platform

    Copyright (C) 2003      by Justin Karneges       <justin@affinix.com>       (from KVIrc source code)
    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteidleplatform_p.h"

#include <QtCore/QObject>
#include <QtCore/QDateTime>
#include <QtGui/QX11Info>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>

static XErrorHandler old_handler = 0;
extern "C" int xerrhandler( Display* dpy, XErrorEvent* err )
{
	if ( err->error_code == BadDrawable )
		return 0;

	return (*old_handler)(dpy, err);
}


class Kopete::IdlePlatform::Private : public QObject
{
	Q_OBJECT
public:
	Private() {}

	XScreenSaverInfo *ss_info;
	QDateTime screenSaverIdleSince;
private slots:
	void activeChanged( bool active )
	{
		if ( active )
		{
			if ( ss_info && XScreenSaverQueryInfo( QX11Info::display(), QX11Info::appRootWindow(), ss_info ) )
				screenSaverIdleSince = QDateTime::currentDateTime().addMSecs(-ss_info->idle);
			else
				screenSaverIdleSince = QDateTime::currentDateTime();
		}
		else
		{
			screenSaverIdleSince = QDateTime();
		}
	}
};

Kopete::IdlePlatform::IdlePlatform()
: d( new Private() )
{
	d->ss_info = 0;
}

Kopete::IdlePlatform::~IdlePlatform()
{
	if ( d->ss_info )
		XFree(d->ss_info);

	if ( old_handler )
	{
		XSetErrorHandler( old_handler );
		old_handler = 0;
	}

	delete d;
}

bool Kopete::IdlePlatform::init()
{
	if ( d->ss_info )
		return true;

	old_handler = XSetErrorHandler( xerrhandler );

	int event_base, error_base;
	if ( XScreenSaverQueryExtension( QX11Info::display(), &event_base, &error_base ) )
	{
		d->ss_info = XScreenSaverAllocInfo();

		// Init ScreenSaver so we can detect screen lock and screen saver.
		QDBusConnection::sessionBus().connect("org.freedesktop.ScreenSaver", "/ScreenSaver",
		                                      "org.freedesktop.ScreenSaver", "ActiveChanged",
		                                      d, SLOT(activeChanged(bool)));

		// If screen saver is already active set the screenSaverIdleSince
		QDBusInterface screenSaver("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver");
		QDBusReply<bool> reply = screenSaver.call("GetActive");
		if ( reply.isValid() && reply.value() )
			d->screenSaverIdleSince = QDateTime::currentDateTime();

		return true;
	}

	return false;
}

int Kopete::IdlePlatform::secondsIdle()
{
	if ( !d->ss_info )
		return 0;

	// Take idle from screenSaverIdleSince if screen saver/lock is enabled.
	if ( d->screenSaverIdleSince.isValid() ) {
		return d->screenSaverIdleSince.secsTo(QDateTime::currentDateTime());
	}

	if ( !XScreenSaverQueryInfo( QX11Info::display(), QX11Info::appRootWindow(), d->ss_info ) )
		return 0;

	return d->ss_info->idle / 1000;
}

#include "kopeteidleplatform_x11.moc"
