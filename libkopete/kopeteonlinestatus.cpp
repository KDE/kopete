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
// necessary for renderIcon, remove after!
#include <qpainter.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include "kopeteprotocol.h"
#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>

struct KopeteOnlineStatusPrivate
{
	KopeteOnlineStatus::OnlineStatus status;
	unsigned weight;
	KopeteProtocol *protocol;
	unsigned internalStatus;
	QString overlayIcon;
	QString caption;
	QString description;
	unsigned refCount;
};

KopeteOnlineStatus::KopeteOnlineStatus( OnlineStatus status, unsigned weight, KopeteProtocol *protocol,
	unsigned internalStatus, const QString &overlayIcon, const QString &caption, const QString &description )
{
	d = new KopeteOnlineStatusPrivate;

	d->refCount = 1;

	d->status = status;
	d->internalStatus = internalStatus;
	d->weight = weight;
	d->overlayIcon = overlayIcon;
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
		// This might be problematic
		d->overlayIcon = QString::null;
		break;
	case Away:
		d->caption = d->description = i18n( "Away" );
		d->overlayIcon = QString::null;
		break;
	case Unknown:
		d->caption = d->description = i18n( "Status not available" );
		d->overlayIcon = QString::null;
		break;
	case Offline:
	default:
		d->caption = d->description = i18n( "Offline" );
		d->overlayIcon = QString::null;
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

bool KopeteOnlineStatus::operator!=( const KopeteOnlineStatus &other ) const
{
	return !d || !other.d || d->internalStatus != other.d->internalStatus || d->protocol != other.d->protocol;
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

QString KopeteOnlineStatus::overlayIcon() const
{
	return d->overlayIcon;
}

QString KopeteOnlineStatus::caption() const
{
	return d->caption;
}

QString KopeteOnlineStatus::description() const
{
	return d->description;
}

KopeteProtocol* KopeteOnlineStatus::protocol() const
{
	return d->protocol;
}

QPixmap KopeteOnlineStatus::iconFor( const KopeteContact *contact ) const
{
	kdDebug() << k_funcinfo << contact->displayName() << " : " << contact->icon() << endl;
	if ( contact->icon().isNull() )
	{
		if ( d->protocol )
			return renderIcon( d->protocol->pluginIcon() );
		else
			return SmallIcon( QString::fromLatin1( "unknown" ) );
	}
	else
		return renderIcon( contact->icon() );
}

QPixmap KopeteOnlineStatus::iconFor( const KopeteAccount *account ) const
{
	//FIXME: support KopeteAccount having knowledge of a custom icon
	if ( d->protocol )
		return renderIcon( d->protocol->pluginIcon() );
	else
		return SmallIcon( QString::fromLatin1( "unknown" ) );
}

QPixmap KopeteOnlineStatus::protocolIcon() const
{
	if ( d->protocol )
		return renderIcon( d->protocol->pluginIcon() );
	else
		return SmallIcon( QString::fromLatin1( "unknown" ) );
}

QPixmap KopeteOnlineStatus::renderIcon( const QString& baseIcon ) const
{
	// create an icon suiting the status from the base icon
	// use reasonable defaults if not provided or protocol not set
	QPixmap basis;

	switch ( d->status )
	{
		case Offline:
			// Apply standard Disabled effect to generate Offline iconOverlay
			// This will probably look crap on the Unknown icon
			// FIXME This won't return icons that are not installed using Martijn's
			// automake magic so we'd have to use UserIcon instead of SmallIcon
			if ( baseIcon.isNull() )
				if ( d->protocol )
					basis = SmallIcon( d->protocol->pluginIcon(), 0,
							KIcon::DisabledState );
				else
					basis = SmallIcon( QString::fromLatin1( "unknown" ), 0,
							KIcon::DisabledState );
			else
				basis = SmallIcon( baseIcon, 0, KIcon::DisabledState );
			break;
		case Online:
		default:
			// get the base icon
			if ( baseIcon.isNull() )
				if ( d->protocol )
					basis = SmallIcon( d->protocol->pluginIcon() );
				else
					basis = SmallIcon( QString::fromLatin1( "unknown" ) );
			else
				basis = SmallIcon( baseIcon );

			//composite the iconOverlay for this status and the supplied baseIcon
			if ( !( d->overlayIcon.isNull() ) ) // otherwise leave the basis as-is
			{
				QPixmap overlay = SmallIcon( d->overlayIcon );
				if ( !overlay.isNull() )
				{
					// first combine the masks of both pixmaps
					if ( overlay.mask() )
					{
						QBitmap mask = *basis.mask();
						bitBlt( &mask, 0, 0, const_cast<QBitmap *>(overlay.mask()),
							0, 0, overlay.width(), overlay.height(), Qt::OrROP );

						basis.setMask(mask);
					}
					// draw the overlay on top of it
					QPainter p( &basis );
					p.drawPixmap( 0, 0, overlay );
				}
			}
			break;
	}
	return basis;
}

// vim: set noet ts=4 sts=4 sw=4:

