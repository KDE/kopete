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



namespace Kopete {

class OnlineStatus::Private
{ public:
	StatusType status;
	unsigned weight;
	Protocol *protocol;
	unsigned internalStatus;
	QString overlayIcon;
	QString description;
	unsigned refCount;
	
	Private() : refCount(1) {}
	Private *ref()
	{
		++refCount;
		return this;
	}
	void unref()
	{
		if( --refCount == 0 )
			delete this;
	}
	
	QString protocolIcon() const
	{
		return protocol ?  protocol->pluginIcon() : QString::fromLatin1( "unknown" );
	}

};

OnlineStatus::OnlineStatus( StatusType status, unsigned weight, Protocol *protocol,
	unsigned internalStatus, const QString &overlayIcon,  const QString &description )
	 : d( new Private )
{
	d->status = status;
	d->internalStatus = internalStatus;
	d->weight = weight;
	d->overlayIcon = overlayIcon;
	d->protocol = protocol;
	d->description = description;
}

OnlineStatus::OnlineStatus( StatusType status, unsigned weight, Protocol *protocol, unsigned internalStatus,
   const QString &overlayIcon, const QString &description, const QString &caption, unsigned int categories , unsigned int options )
	 : d( new Private )
{
	d->status = status;
	d->internalStatus = internalStatus;
	d->weight = weight;
	d->overlayIcon = overlayIcon;
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
	case Unknown:
	default:
		d->description = i18n( "(Status not available)" );
		d->overlayIcon = "status_unknown";
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
	d->overlayIcon = QString::fromLatin1( "status_unknown" );
}

OnlineStatus::OnlineStatus( const OnlineStatus &other )
	 : d( other.d->ref() )
{
}

bool OnlineStatus::operator==( const OnlineStatus &other ) const
{
	return d->internalStatus == other.d->internalStatus && d->protocol == other.d->protocol;
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
	Private *oldD = d;
	d = other.d->ref();
	oldD->unref();
	return *this;
}

OnlineStatus::~OnlineStatus()
{
	d->unref();
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
	return OnlineStatusManager::self()->cacheLookupByMimeSource( mimeSourceFor( contact, size ) );
}


QString OnlineStatus::mimeSourceFor( const Contact *contact, int size ) const
{
	// figure out what icon we should use for this contact
 	QString iconName = contact->icon();
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
	//FIXME: support Kopete::Account having knowledge of a custom icon
	return mimeSource( d->protocolIcon(), size, account->color(), false );
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




} //END namespace Kopete 

// vim: set noet ts=4 sts=4 sw=4:
