/*
    kopeteinfoevent.cpp - Kopete Info Event

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
#include "kopeteinfoevent.h"
#include "kopeteinfoeventmanager.h"

#include <QStringList>

#include <kdebug.h>

namespace Kopete {

class InfoEvent::Private
{
public:
	QMap<uint, QString> actions;
	QString title;
	QString text;
	QString additionalText;
	bool showOnSend;
	bool closed;
};

InfoEvent::InfoEvent( QObject *parent )
 : QObject( parent ), d( new Private )
{
	d->showOnSend = false;
	d->closed = false;
}

InfoEvent::~InfoEvent()
{
	if ( !d->closed )
		emit eventClosed( this );

	delete d;
}

void InfoEvent::sendEvent()
{
	InfoEventManager::self()->addEvent( this );
}

QString InfoEvent::title() const
{
	return d->title;
}

void InfoEvent::setTitle( const QString& title )
{
	d->title = title;
	emit changed();
}

QString InfoEvent::text() const
{
	return d->text;
}

void InfoEvent::setText( const QString& text )
{
	d->text = text;
	emit changed();
}

QString InfoEvent::additionalText() const
{
	return d->additionalText;
}

void InfoEvent::setAdditionalText( const QString& text )
{
	d->additionalText = text;
	emit changed();
}

QMap<uint, QString> InfoEvent::actions() const
{
	return d->actions;
}

void InfoEvent::addAction( uint actionId, const QString& actionText )
{
	d->actions[actionId] = actionText;
	emit changed();
}

bool InfoEvent::showOnSend() const
{
	return d->showOnSend;
}

void InfoEvent::setShowOnSend( bool showOnSend )
{
	d->showOnSend = showOnSend;
}

void InfoEvent::activate( uint actionId )
{
	emit actionActivated( actionId );
}

bool InfoEvent::isClosed() const
{
	return d->closed;
}

void InfoEvent::close()
{
	if ( d->closed )
	{
		kDebug( 14010 ) << "Closing more the once!!!";
		return;
	}

	d->closed = true;
	emit eventClosed( this );
	deleteLater();
}

}

#include "kopeteinfoevent.moc"
