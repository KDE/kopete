/*
    kopeteonlinestatus.cpp - Kopete Online Status

    Copyright (c) 2003 by Martijn Klingens <klingens@kde.org>
    Copyright (c) 2003 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2003 by Will Stephenson <lists@stevello.free-online.co.uk>
    Copyright (c) 2004 by Olivier Goffart <ogoffart @ tiscalinet.be>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

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

#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include <kiconloader.h>
#include <kiconeffect.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstaticdeleter.h>
#include <kapplication.h>



using namespace Kopete;

class OnlineStatus::Private
	: public KShared
{
public:
	StatusType status;
	unsigned weight;
	Protocol *protocol;
	unsigned internalStatus;
	QStringList overlayIcons;
	QString description;
	unsigned refCount;

	QString protocolIcon() const
	{
		return protocol ?  protocol->pluginIcon() : QString::fromLatin1( "unknown" );
	}

};

/**
 * This is required by some plugins, when a status need to be stored on
 * the disk, to avoid problems.
 */
static struct
{
	OnlineStatus::StatusType status;
	const char *name;
} statusNames[] = {
	{ OnlineStatus::Unknown, "Unknown" },
	{ OnlineStatus::Offline, "Offline" },
	{ OnlineStatus::Connecting, "Connecting" },
	{ OnlineStatus::Invisible, "Invisible" },
	{ OnlineStatus::Online, "Online"},
	{ OnlineStatus::Away, "Away" } };

OnlineStatus::OnlineStatus( StatusType status, unsigned weight, Protocol *protocol,
	unsigned internalStatus, const QStringList &overlayIcons,  const QString &description )
	 : d( new Private )
{
	d->status = status;
	d->internalStatus = internalStatus;
	d->weight = weight;
	d->overlayIcons = overlayIcons;
	d->protocol = protocol;
	d->description = description;
}

OnlineStatus::OnlineStatus( StatusType status, unsigned weight, Protocol *protocol, unsigned internalStatus,
   const QStringList &overlayIcons, const QString &description, const QString &caption, unsigned int categories , unsigned int options )
	 : d( new Private )
{
	d->status = status;
	d->internalStatus = internalStatus;
	d->weight = weight;
	d->overlayIcons = overlayIcons;
	d->protocol = protocol;
	d->description = description;

	OnlineStatusManager::self()->registerOnlineStatus(*this, caption, categories, options );
}

OnlineStatus::OnlineStatus( StatusType status )
 : d( new Private )
{
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
		break;
	case Unknown:
	default:
		d->description = i18n( "Unknown" );
		d->overlayIcons = QString::fromLatin1("status_unknown");
		break;

	}
}

OnlineStatus::OnlineStatus()
 : d( new Private )
{
	d->status = Unknown;
	d->internalStatus = 0;
	d->weight = 0;
	d->protocol = 0L;
	d->overlayIcons = QString::fromLatin1( "status_unknown" );
}

OnlineStatus::OnlineStatus( const OnlineStatus &other )
	 : d( other.d )
{
}

bool OnlineStatus::operator==( const OnlineStatus &other ) const
{
	if ( d->internalStatus == other.d->internalStatus && d->protocol == other.d->protocol &&
	     d->weight == other.d->weight && d->overlayIcons == other.d->overlayIcons &&
	     d->description == other.d->description )
	{
		return true;
	}

	return false;
}

bool OnlineStatus::operator!=( const OnlineStatus &other ) const
{
	return !(*this == other);
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
	d = other.d;
	return *this;
}

OnlineStatus::~OnlineStatus()
{
}

OnlineStatus::StatusType OnlineStatus::status() const
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

QStringList OnlineStatus::overlayIcons() const
{
	return d->overlayIcons;
}

QString OnlineStatus::description() const
{
	return d->description;
}

Protocol* OnlineStatus::protocol() const
{
	return d->protocol;
}

bool OnlineStatus::isDefinitelyOnline() const
{
	if ( status() == Offline || status() == Connecting || status() == Unknown )
		return false;
	return true;
}

QPixmap OnlineStatus::iconFor( const Contact *contact, int size ) const
{
	return OnlineStatusManager::self()->cacheLookupByMimeSource( mimeSourceFor( contact, size ) );
}


QString OnlineStatus::mimeSourceFor( const Contact *contact, int size ) const
{
	// figure out what icon we should use for this contact
 	QString iconName = contact->icon();
	if ( iconName.isNull() )
		iconName = contact->account()->customIcon();
	if ( iconName.isNull() )
		iconName = d->protocolIcon();


	return mimeSource( iconName, size, contact->account()->color(),contact->idleTime() >= 10*60 );
}

QPixmap OnlineStatus::iconFor( const Account *account, int size ) const
{
	return OnlineStatusManager::self()->cacheLookupByMimeSource( mimeSourceFor( account, size ) );
}

QString OnlineStatus::mimeSourceFor( const Account *account, int size ) const
{
	QString iconName = account->customIcon();
	if ( iconName.isNull() )
		iconName = d->protocolIcon();

	return mimeSource( iconName, size, account->color(), false );
}

QPixmap OnlineStatus::iconFor( const QString &mimeSource ) const
{
	return OnlineStatusManager::self()->cacheLookupByMimeSource( mimeSource );
}

QPixmap OnlineStatus::protocolIcon() const
{
	return OnlineStatusManager::self()->cacheLookupByObject( *this, d->protocolIcon() , 16, QColor() );
}

QString OnlineStatus::mimeSource( const QString& icon, int size, QColor color, bool idle) const
{
	// make sure the item is in the cache
	OnlineStatusManager::self()->cacheLookupByObject( *this, icon, size, color, idle );
	// now return the fingerprint instead
	return OnlineStatusManager::self()->fingerprint( *this, icon, size, color, idle );
}

QString OnlineStatus::statusTypeToString(OnlineStatus::StatusType statusType)
{
	const int size = sizeof(statusNames) / sizeof(statusNames[0]);

	for (int i=0; i< size; i++)
		if (statusNames[i].status == statusType)
			return QString::fromLatin1(statusNames[i].name);

	return QString::fromLatin1(statusNames[0].name); // Unknown
}

OnlineStatus::StatusType OnlineStatus::statusStringToType(QString& string)
{
	int size = sizeof(statusNames) / sizeof(statusNames[0]);

	for (int i=0; i< size; i++)
		if (QString::fromLatin1(statusNames[i].name) == string)
			return statusNames[i].status;

	return OnlineStatus::Unknown;
}

// vim: set noet ts=4 sts=4 sw=4:
