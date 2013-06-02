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

#include "kopeteidletimer.h"
#include "kopeteidleplatform_p.h"

#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtGui/QCursor>

#include <kdebug.h>

class Kopete::IdleTimer::Private
{
public:
	struct TimeoutReceiver {
		bool active;
		int msec;
		QObject * receiver;
		const char * memberActive;
		const char * memberIdle;
	};
	QList<TimeoutReceiver> receiverList;
	QTimer timer;

	QPoint lastMousePos;
	QDateTime idleSince;

	Kopete::IdlePlatform *platform;
};

Kopete::IdleTimer *Kopete::IdleTimer::instance = 0L;

Kopete::IdleTimer::IdleTimer()
: QObject(), d( new Private )
{
	d->platform = 0;

	Kopete::IdlePlatform *p = new Kopete::IdlePlatform();
	if ( p->init() )
	{
		kDebug() << "Using platform idle timer";
		d->platform = p;
	}
	else
	{
		kWarning() << "Using dummy idle timer";
		delete p;

		d->lastMousePos = QCursor::pos();
		d->idleSince = QDateTime::currentDateTime();
	}

	// init the timer
	connect(&d->timer, SIGNAL(timeout()), this, SLOT(updateIdleTime()));
	d->timer.start(4000);
}

Kopete::IdleTimer *Kopete::IdleTimer::self()
{
	if ( !instance )
		instance = new IdleTimer;

	return instance;
}

Kopete::IdleTimer::~IdleTimer()
{
	instance = 0L;

	delete d->platform;

	delete d;
}

int Kopete::IdleTimer::idleTime()
{
	int idle;
	if ( d->platform )
	{
		idle = d->platform->secondsIdle();
	}
	else
	{
		QPoint curMousePos = QCursor::pos();
		QDateTime curDateTime = QDateTime::currentDateTime();
		if ( d->lastMousePos != curMousePos )
		{
			d->lastMousePos = curMousePos;
			d->idleSince = curDateTime;
		}
		idle = d->idleSince.secsTo(curDateTime);
	}

	return idle;
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

void Kopete::IdleTimer::updateIdleTime()
{
	int idle = idleTime() * 1000;
	bool activity = ( idle < 10 );

	for ( int i = 0; i < d->receiverList.size(); i++ )
	{
		Private::TimeoutReceiver item = d->receiverList.at(i);
		if ( item.active != activity && (activity == true || idle > item.msec ) )
		{
			d->receiverList[i].active = activity;
			if ( activity )
				QTimer::singleShot( 0, item.receiver, item.memberActive );
			else
				QTimer::singleShot( 0, item.receiver, item.memberIdle );
		}
	}
}

#include "kopeteidletimer.moc"
// vim: set et ts=4 sts=4 sw=4:
