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

// necessary for renderIcon, remove after!
// after what? -- lilachaze
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

struct KopeteOnlineStatus::Private
{
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

	QString protocolIcon() const;

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
 : d( new Private )
{
	d->status = status;
	d->internalStatus = internalStatus;
	d->weight = weight;
	d->overlayIcon = overlayIcon;
	d->caption = caption;
	d->protocol = protocol;
	d->description = description;
}

KopeteOnlineStatus::KopeteOnlineStatus( OnlineStatus status )
 : d( new Private )
{
	d->status = status;
	d->internalStatus = 0;
	d->weight = 0;
	d->protocol = 0L;

	switch( status )
	{
	case Online:
		d->caption = d->description = i18n( "Online" );
		break;
	case Away:
		d->caption = d->description = i18n( "Away" );
		break;
	case Unknown:
		d->caption = d->description = i18n( "(Status not available)" );
		d->overlayIcon = "status_unknown";
		break;
	case Connecting:
		d->caption = d->description = i18n( "Connecting" );
		break;
	case Invisible:
		d->caption = d->description = i18n( "Invisible" );
		break;
	case Offline:
	default:
		d->caption = d->description = i18n( "Offline" );
	}
}

KopeteOnlineStatus::KopeteOnlineStatus()
 : d( new Private )
{
	d->status = Unknown;
	d->internalStatus = 0;
	d->weight = 0;
	d->protocol = 0L;
	d->overlayIcon = "status_unknown";
}

KopeteOnlineStatus::KopeteOnlineStatus( const KopeteOnlineStatus &other )
 : d( other.d->ref() )
{
}

bool KopeteOnlineStatus::operator==( const KopeteOnlineStatus &other ) const
{
	return d->internalStatus == other.d->internalStatus && d->protocol == other.d->protocol;
}

bool KopeteOnlineStatus::operator!=( const KopeteOnlineStatus &other ) const
{
	return !(*this == other);
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
	Private *oldD = d;
	d = other.d->ref();
	oldD->unref();
	return *this;
}

KopeteOnlineStatus::~KopeteOnlineStatus()
{
	d->unref();
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

QPixmap KopeteOnlineStatus::iconFor( const KopeteContact *contact, int size ) const
{
	return cacheLookupByMimeSource( mimeSourceFor( contact, size ) );
}

QString KopeteOnlineStatus::Private::protocolIcon() const
{
	if ( !protocol )
		return QString::fromLatin1( "unknown" );
	return protocol->pluginIcon();
}

QString KopeteOnlineStatus::mimeSourceFor( const KopeteContact *contact, int size ) const
{
	// figure out what icon we should use for this contact
 	QString iconName = contact->icon();
	if ( iconName.isNull() )
		iconName = d->protocolIcon();

	return mimeSource( iconName, size, contact->account()->color(),contact->idleTime() >= 10*60 );
}

QPixmap KopeteOnlineStatus::iconFor( const KopeteAccount *account, int size ) const
{
	return cacheLookupByMimeSource( mimeSourceFor( account, size ) );
}

QString KopeteOnlineStatus::mimeSourceFor( const KopeteAccount *account, int size ) const
{
	//FIXME: support KopeteAccount having knowledge of a custom icon
	QString iconName = d->protocolIcon();
	QColor color = account->color();

	return mimeSource( iconName, size, color, false );
}

QPixmap KopeteOnlineStatus::iconFor( const QString &mimeSource ) const
{
	return cacheLookupByMimeSource( mimeSource );
}

QPixmap KopeteOnlineStatus::protocolIcon() const
{
	QString iconName;
	if ( d->protocol )
		iconName = d->protocol->pluginIcon();
	else
		iconName = QString::fromLatin1( "unknown" );

	return cacheLookupByObject( iconName, 16, QColor() );
}

QPixmap KopeteOnlineStatus::cacheLookupByObject( const QString& icon, int size, QColor color, bool idle) const
{
	return Kopete::Global::onlineStatusIconCache()->cacheLookupByObject( *this, icon, size, color, idle );
}

QPixmap KopeteOnlineStatus::cacheLookupByMimeSource( const QString &mimeSource ) const
{
	return Kopete::Global::onlineStatusIconCache()->cacheLookupByMimeSource( mimeSource );
}

QString KopeteOnlineStatus::mimeSource( const QString& icon, int size, QColor color, bool idle) const
{
	// make sure the item is in the cache
	Kopete::Global::onlineStatusIconCache()->cacheLookupByObject( *this, icon, size, color, idle );
	// now return the fingerprint instead
	return Kopete::Global::onlineStatusIconCache()->fingerprint( *this, icon, size, color, idle );
}

Kopete::OnlineStatusIconCache *Kopete::Global::onlineStatusIconCache()
{
	static KStaticDeleter<Kopete::OnlineStatusIconCache> deleter;
	static Kopete::OnlineStatusIconCache *cache = 0L;
	if ( !cache )
		deleter.setObject( cache, new Kopete::OnlineStatusIconCache() );
	return cache;
}

class Kopete::OnlineStatusIconCache::Private
{
public:
	Private() {}
	QDict< QPixmap > iconCache;
};

Kopete::OnlineStatusIconCache::OnlineStatusIconCache()
 : d( new Private() )
{
	d->iconCache.setAutoDelete( true );
	connect( kapp, SIGNAL( iconChanged(int) ), this, SLOT( slotIconsChanged() ) );
}

Kopete::OnlineStatusIconCache::~OnlineStatusIconCache()
{
	delete d;
}

void Kopete::OnlineStatusIconCache::slotIconsChanged()
{
	d->iconCache.clear();
	emit iconsChanged();
}

QString Kopete::OnlineStatusIconCache::fingerprint( const KopeteOnlineStatus &statusFor, const QString& icon, int size, QColor color, bool idle)
{
	// create a 'fingerprint' to use as a hash key
	// fingerprint consists of description/icon name/color/overlay name/size/idle state
	// FIXME: slashes in the following strings could cause false matches.
	QStringList items;
	items << statusFor.d->description << icon << color.name()
	      << statusFor.d->overlayIcon << QString::number(size) << QChar( idle ? 'i' : 'a' );
	return items.join( QString::fromLatin1("/") );
}

QPixmap Kopete::OnlineStatusIconCache::cacheLookupByObject( const KopeteOnlineStatus &statusFor, const QString& icon, int size, QColor color, bool idle)
{
	QString fp = fingerprint( statusFor, icon, size, color, idle );

	// look it up in the cache
	QPixmap *theIcon = d->iconCache.find( fp );
	if ( !theIcon  )
	{
		// cache miss
//		kdDebug(14010) << k_funcinfo << "Missed " << fingerprint << " in icon cache!" << endl;
		theIcon = new QPixmap( renderIcon( statusFor, icon, size, color, idle) );
		d->iconCache.insert( fp, theIcon );
	}
	return *theIcon;
}

QPixmap Kopete::OnlineStatusIconCache::cacheLookupByMimeSource( const QString &mimeSource )
{
	// look it up in the cache
	const QPixmap *theIcon = d->iconCache.find( mimeSource );
	if ( !theIcon ) return QPixmap();
	return *theIcon;
}

static QBitmap overlayMasks( const QBitmap *under, const QBitmap *over )
{
	if ( !under && !over ) return QBitmap();
	if ( !under ) return *over;
	if ( !over ) return *under;

	QBitmap result = *under;
	bitBlt( &result, 0, 0, over, 0, 0, over->width(), over->height(), Qt::OrROP );
	return result;
}

static QPixmap overlayPixmaps( const QPixmap &under, const QPixmap &over )
{
	if ( over.isNull() ) return under;

	QPixmap result = under;
	result.setMask( overlayMasks( under.mask(), over.mask() ) );

	QPainter p( &result );
	p.drawPixmap( 0, 0, over );
	return result;
}

QPixmap Kopete::OnlineStatusIconCache::renderIcon( const KopeteOnlineStatus &statusFor, const QString& baseIcon, int size, QColor color, bool idle) const
{
	// create an icon suiting the status from the base icon
	// use reasonable defaults if not provided or protocol not set

	if ( baseIcon == statusFor.d->overlayIcon )
		kdWarning( 14010 ) << "Base and overlay icons are the same - icon effects will not be visible." << endl;

	QPixmap basis = SmallIcon( baseIcon );

	// Colorize
	if ( color.isValid() )
	{
		KIconEffect effect;
		basis = effect.apply( basis, KIconEffect::Colorize, 1, color, 0);
	}

	// composite the iconOverlay for this status and the supplied baseIcon
	if ( !statusFor.d->overlayIcon.isNull() )
		basis = overlayPixmaps( basis, SmallIcon( statusFor.d->overlayIcon ) );

	if ( statusFor.d->status == KopeteOnlineStatus::Offline)
	{
		// Apply standard Disabled effect to generate Offline icons
		// This will probably look crap on the Unknown icon
		// FIXME This won't return icons that are not installed using Martijn's
		// automake magic so we'd have to use UserIcon instead of SmallIcon
		basis = KIconEffect().apply( basis, KIcon::Small, KIcon::DisabledState );
	}

	// no need to scale if the icon is already of the required size (assuming height == width!)
	if ( basis.width() != size )
	{
		QImage scaledImg = basis.convertToImage().smoothScale( size, size );
		basis = QPixmap( scaledImg );
	}

	// if idle, apply effects
	if ( idle )
		KIconEffect::semiTransparent( basis );

	return basis;
}

#include "kopeteonlinestatus.moc"

// vim: set noet ts=4 sts=4 sw=4:
