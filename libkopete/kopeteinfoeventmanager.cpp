/*
    kopeteinfoeventmanager.cpp - Kopete Info Event Manager

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
#include "kopeteinfoeventmanager.h"
#include "kopeteinfoevent.h"

#include <QApplication>
#include <QStringList>

namespace Kopete {

class InfoEventManager::Private
{
public:
	QList<InfoEvent*> eventList;
};

InfoEventManager *InfoEventManager::instance = 0L;

InfoEventManager::InfoEventManager()
 : QObject( qApp ), d( new Private )
{
}


InfoEventManager::~InfoEventManager()
{
	instance = 0L;
	delete d;
}

InfoEventManager *InfoEventManager::self()
{
	if ( !instance )
		instance = new InfoEventManager;
	
	return instance;
}

void InfoEventManager::addEvent( Kopete::InfoEvent* event )
{
	emit eventAboutToBeAdded( event );

	if ( !event->isClosed() )
	{
		connect( event, SIGNAL(eventClosed(Kopete::InfoEvent*)),
		         this, SLOT(eventClosed(Kopete::InfoEvent*)) );

		d->eventList.append( event );
		emit eventAdded( event );
		emit changed();
	}
}

QList<InfoEvent*> InfoEventManager::events() const
{
	return d->eventList;
}

int InfoEventManager::eventCount() const
{
	return d->eventList.count();
}

Kopete::InfoEvent* InfoEventManager::event( int i ) const
{
	return d->eventList.at( i );
}

void InfoEventManager::eventClosed( Kopete::InfoEvent* event )
{
	d->eventList.removeAll( event );
	emit changed();
}

}

#include "kopeteinfoeventmanager.moc"
