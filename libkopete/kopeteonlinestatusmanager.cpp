/*
    kopeteonlinestatusmanager.cpp

    Copyright (c) 2004 by Olivier Goffart  <ogoffart@kde.fr>
    Copyright (c) 2003 by Will Stephenson <wstephenson@kde.org>

    Kopete    (c) 2003-2007 by the Kopete developers  <kopete-devel@kde.org>

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

#include "kopeteprotocol.h"
#include "kopetebehaviorsettings.h"

#include <kiconloader.h>
#include <kiconeffect.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kicon.h>

#include <algorithm> // for min

namespace Kopete {


class OnlineStatusManager::Private
{
public:
	struct QMapDummyValue
	{
	};

	typedef QMap< OnlineStatus , QMapDummyValue >  RegisteredStatusMap;

	QPixmap *nullPixmap;
	QMap< Protocol* , RegisteredStatusMap > registeredStatus;
	QHash< QString, QPixmap* > iconCache;
};

OnlineStatusManager *OnlineStatusManager::self()
{
	static OnlineStatusManager s;
	return &s;
}

OnlineStatusManager::OnlineStatusManager()
 : d( new Private )
{
	// no autodelete, removing everything in destructor
//	d->iconCache.setAutoDelete( true );
	d->nullPixmap = new QPixmap;
	connect( KGlobalSettings::self(), SIGNAL(iconChanged(int)), this, SLOT(slotIconsChanged()) );
}

OnlineStatusManager::~OnlineStatusManager()
{
	QHashIterator<QString, QPixmap*> it(d->iconCache);
	while ( it.hasNext() )
	{
		it.next();
		delete it.value();
	}

	delete d->nullPixmap;
	delete d;
}

void OnlineStatusManager::slotIconsChanged()
{
	// shall we delete all of em first ?
	d->iconCache.clear();
	emit iconsChanged();
}

void OnlineStatusManager::registerOnlineStatus( const OnlineStatus &status )
{
	d->registeredStatus[status.protocol()].insert( status, Private::QMapDummyValue() );
}

OnlineStatus OnlineStatusManager::onlineStatus(Protocol * protocol, Categories category) const
{
	/* Each category has a number which is a power of two, so it is possible to have several categories per online status
	 * the logaritm in base two if this number, which represent the bit which is equal to 1 in the number is chosen to be in a tree
	 *           1     (0 is reserved for Offline)
	 *         /   \
	 *        2      3
	 *      / \     /  \
	 *     4  5     6    7
	 *   /\  / \   / \   / \
	 *  8 9 10 11 12 13 14 15
	 *  To get the parent of a key, one just divide per two the number
	 */

	const Private::RegisteredStatusMap protocolMap=d->registeredStatus[protocol];

	int categ_nb=-1;  //the logaritm of category
	uint category_=category;
	while(category_)
	{
		category_ >>= 1;
		categ_nb++;
	} //that code will give the log +1

	do
	{
		Private::RegisteredStatusMap::ConstIterator it;
		for ( it = protocolMap.begin(); it != protocolMap.end(); ++it )
		{
			unsigned int catgs=it.key().categories();
			if(catgs & (1<<(categ_nb)))
				return it.key();
		}
		//no status found in this category, try the previous one.
		categ_nb=(int)(categ_nb/2);
	} while (categ_nb > 0);

	kWarning() << "No status in the category " << category << " for the protocol " << protocol->displayName();
	return OnlineStatus();
}

OnlineStatusManager::Category OnlineStatusManager::initialStatus() const
{
    Kopete::OnlineStatusManager::Category statusValue = Kopete::OnlineStatusManager::Offline;
		switch( Kopete::BehaviorSettings::self()->initialStatus() )
		{
		  case Kopete::BehaviorSettings::EnumInitialStatus::Offline:
			statusValue = Kopete::OnlineStatusManager::Offline;
		  break;
		  case Kopete::BehaviorSettings::EnumInitialStatus::Online:
			statusValue = Kopete::OnlineStatusManager::Online;
		  break;
		  case Kopete::BehaviorSettings::EnumInitialStatus::Away:
			statusValue = Kopete::OnlineStatusManager::Away;
		  break;
		  case Kopete::BehaviorSettings::EnumInitialStatus::Busy:
			statusValue = Kopete::OnlineStatusManager::Busy;
		  break;
		  case Kopete::BehaviorSettings::EnumInitialStatus::Invisible:
			statusValue = Kopete::OnlineStatusManager::Invisible;
		  break;
		}

    return statusValue;
}

QString OnlineStatusManager::fingerprint( const OnlineStatus &statusFor, const QString& icon, int size, QColor color, bool idle)
{
	// create a 'fingerprint' to use as a hash key
	// fingerprint consists of description/icon name/color/overlay name/size/idle state
	return QString::fromLatin1("%1/%2/%3/%4/%5/%6")
	                           .arg( statusFor.description() )
	                           .arg( icon )
	                           .arg( color.name() )
	                           .arg( statusFor.overlayIcons().join( QString::fromLatin1( "," ) ) )
	                           .arg( size )
	                           .arg( idle ? 'i' : 'a' );
}

QPixmap OnlineStatusManager::cacheLookupByObject( const OnlineStatus &statusFor, const QString& icon, int size, QColor color, bool idle)
{
	QString fp = fingerprint( statusFor, icon, size, color, idle );

	//kDebug(14010) << "finger print:" << fp << ", icon: " << icon;
	// look it up in the cache
	QPixmap *theIcon = d->iconCache.value(fp);
	if ( !theIcon )
	{
		// cache miss
		kDebug(14010) << "Missed " << fp << " in icon cache!";
		theIcon = renderIcon( statusFor, icon, size, color, idle);
		d->iconCache[fp] = theIcon;
	}
	return *theIcon;
}

QPixmap OnlineStatusManager::cacheLookupByMimeSource( const QString &mimeSource )
{
	// look it up in the cache
	const QPixmap *theIcon= d->iconCache.find( mimeSource ).value();
	if ( !theIcon )
	{
		// need to return an invalid pixmap
		theIcon = d->nullPixmap;
	}
	return *theIcon;
}

// This code was forked from the broken KImageEffect::blendOnLower, but it's
// been so heavily fixed and rearranged it's hard to recognise that now.
static void blendOnLower( const QImage &upper_, QImage &lower, const QPoint &offset )
{
	if ( upper_.width() <= 0 || upper_.height() <= 0 )
		return;
	if ( lower.width() <= 0 || lower.height() <= 0 )
		return;
	if ( offset.x() < 0 || offset.x() >= lower.width() )
		return;
	if ( offset.y() < 0 || offset.y() >= lower.height() )
		return;

	QImage upper = upper_;
	if ( upper.depth() != 32 )
		upper = upper.convertToFormat( QImage::Format_RGB32 );
	if ( lower.depth() != 32 )
		lower = lower.convertToFormat( QImage::Format_RGB32 );

	const int cx = offset.x();
	const int cy = offset.y();
	const int cw = std::min( upper.width() + cx, lower.width() );
	const int ch = std::min( upper.height() + cy, lower.height() );
	const int m = 255;

	for ( int j = cy; j < ch; ++j )
	{
		QRgb *u = (QRgb*)upper.scanLine(j - cy);
		QRgb *l = (QRgb*)lower.scanLine(j) + cx;

		for( int k = cx; k < cw; ++u, ++l, ++k )
		{
			int ua = qAlpha(*u);
			if ( !ua )
				continue;

			int la = qAlpha(*l);

			int   d =                       ua * m +              la * (m - ua);
			uchar r = uchar( (   qRed(*u) * ua * m +   qRed(*l) * la * (m - ua) ) / d );
			uchar g = uchar( ( qGreen(*u) * ua * m + qGreen(*l) * la * (m - ua) ) / d );
			uchar b = uchar( (  qBlue(*u) * ua * m +  qBlue(*l) * la * (m - ua) ) / d );
			uchar a = uchar( (         ua * ua * m +         la * la * (m - ua) ) / d );
			*l = qRgba( r, g, b, a );
		}
	}
}

// Get bounding box of image via alpha channel
static QRect getBoundingBox( const QImage& image )
{
	const int width = image.width();
	const int height = image.height();
	if ( width <= 0 || height <= 0 )
		return QRect();

	// scan image from left to right and top to bottom
	// to get upper left corner of bounding box
	int x1 = width - 1;
	int y1 = height - 1;
	for ( int j = 0; j < height; ++j )
	{
		QRgb *i = (QRgb*)image.scanLine(j);

		for( int k = 0; k < width; ++i, ++k )
		{
			if ( qAlpha(*i) )
			{
				x1 = std::min( x1, k );
				y1 = std::min( y1, j );
				break;
			}
		}
	}

	// scan image from right to left and bottom to top
	// to get lower right corner of bounding box
	int x2 = 0;
	int y2 = 0;
	for ( int j = height-1; j >= 0; --j )
	{
		QRgb *i = (QRgb*)image.scanLine(j) + width-1;

		int k;
		for( k = width-1; k >= 0; --i, --k )
		{
			if ( qAlpha(*i) )
			{
				x2 = std::max( x2, k );
				y2 = std::max( y2, j );
				break;
			}
		}
			if ( qAlpha(*i) )
			{
				x2 = std::max( x2, k );
				y2 = std::max( y2, j );
				break;
			}
	}
	return QRect( x1, y1, std::max( 0, x2-x1+1 ), std::max( 0, y2-y1+1 ) );
}

// Get offset for upperImage to blend it in the i%4-th corner of lowerImage:
// bottom right, bottom left, top left, top right
static QPoint getOffsetForCorner( const QImage& upperImage, const QImage& lowerImage, const int i )
{
	const int dX = lowerImage.width() - upperImage.width();
	const int dY = lowerImage.height() - upperImage.height();
	const int corner = i % 4;
	QPoint offset;
	switch( corner ) {
		case 0:
			// bottom right
			offset = QPoint( dX, dY );
			break;
		case 1:
			// bottom left
			offset = QPoint( 0, dY );
			break;
		case 2:
			// top left
			offset = QPoint( 0, 0 );
			break;
		case 3:
			// top right
			offset = QPoint( dX, 0 );
			break;
	}
	return offset;
}

QPixmap* OnlineStatusManager::renderIcon( const OnlineStatus &statusFor, const QString& baseIcon, int size, QColor color, bool idle) const
{
	// create an icon suiting the status from the base icon
	// use reasonable defaults if not provided or protocol not set

	//kDebug( 14010) << "overlayIcons size: " << statusFor.overlayIcons().count();

	// NOTE: overlayIcons car be empty
	if ( !statusFor.overlayIcons().empty() && baseIcon == statusFor.overlayIcons().first() )
	{
		kWarning( 14010 ) << "Base and overlay icons are the same - icon effects will not be visible.";
	}

	QPixmap* basis = new QPixmap( KIcon(baseIcon).pixmap(size) );

	// Colorize
	if ( color.isValid() )
		*basis = KIconEffect().apply( *basis, KIconEffect::Colorize, 1, color, 0);

	// Note that we do this before compositing the overlay, since we want
	// that to be colored in this case.
	if ( statusFor.internalStatus() == Kopete::OnlineStatus::AccountOffline || statusFor.status() == Kopete::OnlineStatus::Offline )
	{
		*basis = KIconEffect().apply( *basis, KIconEffect::ToGray , 0.85, QColor() , false  );
	}

	//composite the iconOverlay for this status and the supplied baseIcon
	QStringList overlays = statusFor.overlayIcons();
	if ( !( overlays.isEmpty() ) ) // otherwise leave the basis as-is
	{
		KIconLoader *loader = KIconLoader::global();

		int i = 0;
		for( QStringList::iterator it = overlays.begin(), end = overlays.end(); it != end; ++it )
		{
			QPixmap overlay = loader->loadIcon(*it, KIconLoader::NoGroup, size,
				KIconLoader::DefaultState, QStringList(), 0L, /*canReturnNull=*/ true );

			if ( !overlay.isNull() )
			{
				// we want to preserve the alpha channels of both basis and overlay.
				// there's no way to do this in Qt. In fact, there's no way to do this
				// in KDE since KImageEffect is so badly broken.
				QImage basisImage = basis->toImage();
				QImage overlayImage = overlay.toImage();
				QPoint offset;
				if ( (*it).endsWith( QString::fromLatin1( "_overlay" ) ) )
				{
					// it is possible to have more than one overlay icon
					// to avoid overlapping we place them in different corners
					overlayImage = overlayImage.copy( getBoundingBox( overlayImage ) );
					offset = getOffsetForCorner( overlayImage, basisImage, i );
					++i;
				}
				blendOnLower( overlayImage, basisImage, offset );
				*basis = QPixmap::fromImage( basisImage );
			}
		}
	}

	// no need to scale if the icon is already of the required size (assuming height == width!)
	if ( basis->width() != size )
	{
		QImage scaledImg = basis->toImage().scaled( size, size , Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
		*basis = QPixmap::fromImage( scaledImg );
	}

	// if idle, apply effects
	if ( idle )
		KIconEffect::semiTransparent( *basis );

	return basis;
}

QList<OnlineStatus> OnlineStatusManager::registeredStatusList( Protocol *protocol ) const
{
	return d->registeredStatus.value( protocol ).keys();
}

KIcon OnlineStatusManager::pixmapForCategory( Categories category )
{
	switch ( category )
	{
	case Kopete::OnlineStatusManager::Online:
		return KIcon("user-online");
	case Kopete::OnlineStatusManager::FreeForChat:
		return KIcon("user-online");
	case Kopete::OnlineStatusManager::Away:
		return KIcon("user-away");
	case Kopete::OnlineStatusManager::Idle:
		return KIcon("user-away");
	case Kopete::OnlineStatusManager::ExtendedAway:
		return KIcon("user-away-extended");
	case Kopete::OnlineStatusManager::Busy:
		return KIcon("user-busy");
	case Kopete::OnlineStatusManager::Invisible:
		return KIcon("user-invisible");
	case Kopete::OnlineStatusManager::Offline:
		return KIcon("user-offline");
	default:
		return KIcon("user-identity");
	}
}

} //END namespace Kopete

#include "kopeteonlinestatusmanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

