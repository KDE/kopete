/*
    kopeteonlinestatus.cpp - Kopete Online Status

    Copyright (c) 2003      by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteonlinestatus.h"

#include <qstring.h>

#include <klocale.h>

class KopeteOnlineStatusPrivate
{
public:
	KopeteOnlineStatus::OnlineStatus status;
	unsigned weight;
	KopeteProtocol *protocol;
	unsigned internalStatus;
	QString icon;
	QString caption;
	QString description;
	unsigned refCount;
};

KopeteOnlineStatus::KopeteOnlineStatus( OnlineStatus status, unsigned weight, KopeteProtocol *protocol,
	unsigned internalStatus, const QString &icon, const QString &caption, const QString &description )
{
	d = new KopeteOnlineStatusPrivate;

	d->refCount = 1;

	d->status = status;
	d->internalStatus = internalStatus;
	d->weight = weight;
	d->icon = icon;
	d->caption = caption;
	d->protocol = protocol;
	d->description = description;
}

KopeteOnlineStatus::KopeteOnlineStatus( OnlineStatus status )
{
	d = new KopeteOnlineStatusPrivate;

	d->refCount = 1;

	d->status = status;
	d->internalStatus = 0;
	d->weight = 0;
	d->protocol = 0L;

	switch( status )
	{
	case Online:
		d->caption = d->description = i18n( "Online" );
		d->icon = QString::fromLatin1( "unknown" );
		break;
	case Away:
		d->caption = d->description = i18n( "Away" );
		d->icon = QString::fromLatin1( "unknown" );
		break;
	case Unknown:
		d->caption = d->description = i18n( "Status not available" );
		d->icon = QString::fromLatin1( "unknown" );
		break;
	case Offline:
	default:
		d->caption = d->description = i18n( "Offline" );
		d->icon = QString::fromLatin1( "unknown" );
	}
}

KopeteOnlineStatus::KopeteOnlineStatus()
{
	d = new KopeteOnlineStatusPrivate;

	d->refCount = 1;

	d->status = Unknown;
	d->internalStatus = 0;
	d->weight = 0;
	d->protocol = 0L;
}

KopeteOnlineStatus::KopeteOnlineStatus( const KopeteOnlineStatus &other )
{
	d = other.d;

	d->refCount++;
}

bool KopeteOnlineStatus::operator==( const KopeteOnlineStatus &other ) const
{
	return d && other.d && d->internalStatus == other.d->internalStatus && d->protocol == other.d->protocol;
}

bool KopeteOnlineStatus::operator>( const KopeteOnlineStatus &other ) const
{
	if( d->status == other.d->status )
		return d->weight > other.d->weight;
	else
		return d->status > other.d->status;
}

bool KopeteOnlineStatus::operator<( const KopeteOnlineStatus &other ) const
{
	if( d->status == other.d->status )
		return d->weight < other.d->weight;
	else
		return d->status < other.d->status;
}

KopeteOnlineStatus & KopeteOnlineStatus::operator=( const KopeteOnlineStatus &other )
{
	d->refCount--;
	if( !d->refCount )
		delete d;

	d = other.d;

	d->refCount++;

	return *this;
}

KopeteOnlineStatus::~KopeteOnlineStatus()
{
	d->refCount--;
	if( !d->refCount )
		delete d;
}

KopeteOnlineStatus::OnlineStatus KopeteOnlineStatus::status() const
{
	return d->status;
}

unsigned KopeteOnlineStatus::internalStatus() const
{
	return d->internalStatus;
}

unsigned KopeteOnlineStatus::weight() const
{
	return d->weight;
}

QString KopeteOnlineStatus::icon() const
{
	return d->icon;
}

QString KopeteOnlineStatus::caption() const
{
	return d->caption;
}

QString KopeteOnlineStatus::description() const
{
	return d->description;
}

// vim: set noet ts=4 sts=4 sw=4:

