/*
    kopeteonlinestatusmanager.cpp

    Copyright (c) 2004 by Olivier Goffart  <ogoffart @ tiscalinet . be>
    Copyright (c) 2003 by Will Stephenson <lists@stevello.free-online.co.uk>
	
    Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteonlinestatusmanager.h"

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


class OnlineStatusManager::Private
{public:
	QPixmap *nullPixmap;
	QMap<Protocol* , QMap< OnlineStatus , QPair<QString, unsigned int> > > registeredStatus;
	QDict< QPixmap > iconCache;
};

OnlineStatusManager *OnlineStatusManager::s_self=0L;

OnlineStatusManager *OnlineStatusManager::self()
{
	static KStaticDeleter<OnlineStatusManager> deleter;
	if(!s_self)
		deleter.setObject( s_self, new OnlineStatusManager() );
	return s_self;	
}

OnlineStatusManager::OnlineStatusManager()
 : d( new Private )
{
	d->iconCache.setAutoDelete( true );
	d->nullPixmap = new QPixmap;
	connect( kapp, SIGNAL( iconChanged(int) ), this, SLOT( slotIconsChanged() ) );
}

OnlineStatusManager::~OnlineStatusManager()
{
	delete d->nullPixmap;
	delete d;
}

void OnlineStatusManager::slotIconsChanged()
{
	d->iconCache.clear();
	emit iconsChanged();
}

void OnlineStatusManager::registerOnlineStatus( const OnlineStatus &status, const QString & caption, unsigned int categories, unsigned int options)
{
	//TODO: use options
	d->registeredStatus[status.protocol()].insert(status, qMakePair(caption, categories)  );
}

QString OnlineStatusManager::fingerprint( const OnlineStatus &statusFor, const QString& icon, int size, QColor color, bool idle)
{
	// create a 'fingerprint' to use as a hash key
	// fingerprint consists of description/icon name/color/overlay name/size/idle state
	return QString::fromLatin1("%1/%2/%3/%4/%5/%6")
								.arg( statusFor.description() )
								.arg( icon )
								.arg( color.name() )
								.arg( statusFor.overlayIcon())
								.arg( size )
								.arg( idle ? 'i' : 'a' );
}

QPixmap OnlineStatusManager::cacheLookupByObject( const OnlineStatus &statusFor, const QString& icon, int size, QColor color, bool idle)
{
	QString fp = fingerprint( statusFor, icon, size, color, idle );

	// look it up in the cache
	QPixmap *theIcon= d->iconCache.find( fp );
	if ( !theIcon  )
	{
		// cache miss
//		kdDebug(14010) << k_funcinfo << "Missed " << fingerprint << " in icon cache!" << endl;
		theIcon = renderIcon( statusFor, icon, size, color, idle);
		d->iconCache.insert( fp, theIcon );
	}
	return *theIcon;
}

QPixmap OnlineStatusManager::cacheLookupByMimeSource( const QString &mimeSource )
{
	// look it up in the cache
	const QPixmap *theIcon= d->iconCache.find( mimeSource );
	if ( !theIcon )
	{
		// need to return an invalid pixmap
		theIcon = d->nullPixmap;
	}
	return *theIcon;
}

QPixmap* OnlineStatusManager::renderIcon( const OnlineStatus &statusFor, const QString& baseIcon, int size, QColor color, bool idle) const
{
	// create an icon suiting the status from the base icon
	// use reasonable defaults if not provided or protocol not set

	if ( baseIcon == statusFor.overlayIcon() )
		kdWarning( 14010 ) << "Base and overlay icons are the same - icon effects will not be visible." << endl;
	
	QPixmap* basis = new QPixmap( SmallIcon( baseIcon ) );

	// Colorize
	if ( color.isValid() )
	{
		KIconEffect effect;
		*basis = effect.apply( *basis, KIconEffect::Colorize, 1, color, 0);
	}

	//composite the iconOverlay for this status and the supplied baseIcon
	if ( !( statusFor.overlayIcon().isNull() ) ) // otherwise leave the basis as-is
	{
	
		KIconLoader *loader = KGlobal::instance()->iconLoader();
		QPixmap overlay = loader->loadIcon(statusFor.overlayIcon(), KIcon::Small, 0 ,  KIcon::DefaultState, 0L, /*canReturnNull=*/ true );
		if ( !overlay.isNull() )
		{
			// first combine the masks of both pixmaps
			if ( overlay.mask() )
			{
				QBitmap mask = *(basis->mask());
				bitBlt( &mask, 0, 0, const_cast<QBitmap *>(overlay.mask()),
					0, 0, overlay.width(), overlay.height(), Qt::OrROP );

				basis->setMask(mask);
			}
			// draw the overlay on top of it
			QPainter p( basis );
			p.drawPixmap( 0, 0, overlay );
		}
	}

	if ( statusFor.status() == OnlineStatus::Offline)
	{
		// Apply standard Disabled effect to generate Offline icons
		// This will probably look crap on the Unknown icon
		// FIXME This won't return icons that are not installed using Martijn's
		// automake magic so we'd have to use UserIcon instead of SmallIcon
		*basis = KIconEffect().apply( *basis, KIcon::Small, KIcon::DisabledState );
	}

	// no need to scale if the icon is already of the required size (assuming height == width!)
	if ( basis->width() != size )
	{
		QImage scaledImg = basis->convertToImage().smoothScale( size, size );
		*basis = QPixmap( scaledImg );
	}

	// if idle, apply effects
	if ( idle )
		KIconEffect::semiTransparent( *basis );

	return basis;
}

} //END namespace Kopete 

#include "kopeteonlinestatusmanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

