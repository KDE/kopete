/*
    kopeteonlinestatus.cpp - Kopete Online Status

    Copyright (c) 2003 by Martijn Klingens <klingens@kde.org>
    Copyright (c) 2003 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2003 by Will Stephenson <lists@stevello.free-online.co.uk>

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
#include "kopeteonlinestatusmanager.h"

// necessary for renderIcon, remove after!
#include <qpainter.h>
#include <qbitmap.h>
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include <kiconloader.h>
#include <kiconeffect.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstaticdeleter.h>
#include <kapplication.h>

namespace Kopete {

class OnlineStatus::Private
{ public:
	OnlineStatus::Status status;
	unsigned weight;
	Protocol *protocol;
	unsigned internalStatus;
	QString overlayIcon;
	QString description;
	unsigned refCount;
};

OnlineStatus::OnlineStatus( Status status, unsigned weight, Protocol *protocol,
	unsigned internalStatus, const QString &overlayIcon,  const QString &description )
{
	d = new Private;

	d->refCount = 1;

	d->status = status;
	d->internalStatus = internalStatus;
	d->weight = weight;
	d->overlayIcon = overlayIcon;
	d->protocol = protocol;
	d->description = description;
}

OnlineStatus::OnlineStatus( Status status )
{
	d = new Private;

	d->refCount = 1;

	d->status = status;
	d->internalStatus = 0;
	d->weight = 0;
	d->protocol = 0L;

	switch( status )
	{
	case Online:
		 d->description = i18n( "Online" );
		break;
	case Away:
		d->description = i18n( "Away" );
		break;
	case Connecting:
		d->description = i18n( "Connecting" );
		break;
	case Invisible:
		d->description = i18n( "Invisible" );
		break;
	case Offline:
		d->description = i18n( "Offline" );
	case Unknown:
	default:
		d->description = i18n( "(Status not available)" );
		d->overlayIcon = "status_unknown";
		break;
		
	}
}

OnlineStatus::OnlineStatus()
{
	d = new Private;

	d->refCount = 1;

	d->status = Unknown;
	d->internalStatus = 0;
	d->weight = 0;
	d->protocol = 0L;
	d->overlayIcon = "status_unknown";
}

OnlineStatus::OnlineStatus( const OnlineStatus &other )
{
	d = other.d;

	d->refCount++;
}

bool OnlineStatus::operator==( const OnlineStatus &other ) const
{
	return d && other.d && d->internalStatus == other.d->internalStatus && d->protocol == other.d->protocol;
}

bool OnlineStatus::operator!=( const OnlineStatus &other ) const
{
	return !d || !other.d || d->internalStatus != other.d->internalStatus || d->protocol != other.d->protocol;
}


bool OnlineStatus::operator>( const OnlineStatus &other ) const
{
	if( d->status == other.d->status )
		return d->weight > other.d->weight;
	else
		return d->status > other.d->status;
}

bool OnlineStatus::operator<( const OnlineStatus &other ) const
{
	if( d->status == other.d->status )
		return d->weight < other.d->weight;
	else
		return d->status < other.d->status;
}

OnlineStatus & OnlineStatus::operator=( const OnlineStatus &other )
{
	d->refCount--;
	if( !d->refCount )
		delete d;

	d = other.d;

	d->refCount++;

	return *this;
}

OnlineStatus::~OnlineStatus()
{
	d->refCount--;
	if( !d->refCount )
		delete d;
}

OnlineStatus::Status OnlineStatus::status() const
{
	return d->status;
}

unsigned OnlineStatus::internalStatus() const
{
	return d->internalStatus;
}

unsigned OnlineStatus::weight() const
{
	return d->weight;
}

QString OnlineStatus::overlayIcon() const
{
	return d->overlayIcon;
}


QString OnlineStatus::description() const
{
	return d->description;
}

Protocol* OnlineStatus::protocol() const
{
	return d->protocol;
}

QPixmap OnlineStatus::iconFor( const Contact *contact, int size ) const
{
	// figure out what icon we should use for this contact
 	QString iconName;
	if ( contact->icon().isNull() )
	{
		if ( d->protocol )
			iconName = d->protocol->pluginIcon();
		else
			iconName = QString::fromLatin1( "unknown" );
	}
	else
		iconName = contact->icon();

	return cacheLookupByObject( iconName, size, contact->account()->color(),/* TODO contact->idleTime() >= 10*60 */ false);

}

QString OnlineStatus::mimeSourceFor( const Contact *contact, int size ) const
{
	// figure out what icon we should use for this contact
 	QString iconName;
	if ( contact->icon().isNull() )
	{
		if ( d->protocol )
			iconName = d->protocol->pluginIcon();
		else
			iconName = QString::fromLatin1( "unknown" );
	}
	else
		iconName = contact->icon();

	return mimeSource( iconName, size, contact->account()->color(), /* TODO contact->idleTime() >= 10*60 */ false );

}

QPixmap OnlineStatus::iconFor( const Account *account, int size ) const
{
	//FIXME: support KopeteAccount having knowledge of a custom icon
	QString iconName;
	if ( d->protocol )
		iconName = d->protocol->pluginIcon();
	else
		iconName = QString::fromLatin1( "unknown" );

	QColor color = account->color();

	return cacheLookupByObject( iconName, size, color, false );
}

QString OnlineStatus::mimeSourceFor( const Account *account, int size ) const
{
	//FIXME: support Account having knowledge of a custom icon
	QString iconName;
	if ( d->protocol )
		iconName = d->protocol->pluginIcon();
	else
		iconName = QString::fromLatin1( "unknown" );

	QColor color = account->color();

	return mimeSource( iconName, size, color, false );
}

QPixmap OnlineStatus::iconFor( const QString &mimeSource ) const
{
	return cacheLookupByMimeSource( mimeSource );
}

QPixmap OnlineStatus::protocolIcon() const
{
	QString iconName;
	if ( d->protocol )
		iconName = d->protocol->pluginIcon();
	else
		iconName = QString::fromLatin1( "unknown" );

	return cacheLookupByObject( iconName, 16, QColor() );
}

QPixmap OnlineStatus::cacheLookupByObject( const QString& icon, int size, QColor color, bool idle) const
{
	return OnlineStatusManager::self()->cacheLookupByObject( *this, icon, size, color, idle );
}

QPixmap OnlineStatus::cacheLookupByMimeSource( const QString &mimeSource ) const
{
	return OnlineStatusManager::self()->cacheLookupByMimeSource( mimeSource );
}

QString OnlineStatus::mimeSource( const QString& icon, int size, QColor color, bool idle) const
{
	// make sure the item is in the cache
	OnlineStatusManager::self()->cacheLookupByObject( *this, icon, size, color, idle );
	// now return the fingerprint instead
	return OnlineStatusManager::self()->fingerprint( *this, icon, size, color, idle );
}


} //END namespace Kopete 


// vim: set noet ts=4 sts=4 sw=4:

