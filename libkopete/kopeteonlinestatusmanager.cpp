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

#include "kopeteawayaction.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"

#include <kiconloader.h>
#include <kiconeffect.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstaticdeleter.h>
#include <kcpuinfo.h> // for WORDS_BIGENDIAN
#include <kicon.h>

#include <algorithm> // for min
#include <QPixmap>
#include <QByteArray>
#include <QHash>

namespace Kopete {


class OnlineStatusManager::Private
{
public:
	struct RegisteredStatusStruct
	{
		QString caption;
		OnlineStatusManager::Categories categories;
		OnlineStatusManager::Options options;
	};

	typedef QMap< OnlineStatus , RegisteredStatusStruct >  ProtocolMap ;

	QPixmap *nullPixmap;
	QMap<Protocol* , ProtocolMap > registeredStatus;
	QHash< QString, QPixmap* > iconCache;
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
	// no autodelete, removing everything in destructor
//	d->iconCache.setAutoDelete( true );
	d->nullPixmap = new QPixmap;
	connect( KGlobalSettings::self(), SIGNAL( iconChanged(int) ), this, SLOT( slotIconsChanged() ) );
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

void OnlineStatusManager::registerOnlineStatus( const OnlineStatus &status, const QString & caption, Categories categories, Options options)
{
	Private::RegisteredStatusStruct s;
	s.caption=caption;
	s.categories=categories;
	s.options=options;
	d->registeredStatus[status.protocol()].insert(status, s );
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

	Private::ProtocolMap protocolMap=d->registeredStatus[protocol];

	int categ_nb=-1;  //the logaritm of category
	uint category_=category;
	while(category_)
	{
		category_ >>= 1;
		categ_nb++;
	} //that code will give the log +1

	do
	{
		Private::ProtocolMap::Iterator it;
		for ( it = protocolMap.begin(); it != protocolMap.end(); it++ )
		{
			unsigned int catgs=it.value().categories;
			if(catgs & (1<<(categ_nb)))
				return it.key();
		}
		//no status found in this category, try the previous one.
		categ_nb=(int)(categ_nb/2);
	} while (categ_nb > 0);

	kWarning() << "No status in the category " << category << " for the protocol " << protocol->displayName() <<endl;
	return OnlineStatus();
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

	kDebug(14010) << "finger print:" << fp << ", icon: " << icon << endl;
	// look it up in the cache
	QPixmap *theIcon = d->iconCache.value(fp);
	if ( !theIcon )
	{
		// cache miss
		kDebug(14010) << k_funcinfo << "Missed " << fp << " in icon cache!" << endl;
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

	kDebug( 14010) << k_funcinfo << "overlayIcons size: " << statusFor.overlayIcons().count() <<endl;

	// NOTE: overlayIcons car be empty
	if ( !statusFor.overlayIcons().empty() && baseIcon == statusFor.overlayIcons().first() )
		kWarning( 14010 ) << "Base and overlay icons are the same - icon effects will not be visible." << endl;

	QPixmap* basis = new QPixmap( SmallIcon( baseIcon ) );

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
			QPixmap overlay = loader->loadIcon(*it, K3Icon::Small, 0 ,
				K3Icon::DefaultState, 0L, /*canReturnNull=*/ true );

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

void OnlineStatusManager::createAccountStatusActions( Account *account , KActionMenu *parent)
{
	Private::ProtocolMap protocolMap=d->registeredStatus[account->protocol()];
	Private::ProtocolMap::Iterator it;
	for ( it = --protocolMap.end(); it != protocolMap.end(); --it )
	{
		unsigned int options=it.value().options;
		if(options & OnlineStatusManager::HideFromMenu)
			continue;

		OnlineStatus status=it.key();
		QString caption=it.value().caption;
		KAction *action;

		// Any existing actions owned by the account are reused by recovering them
		// from the parent's child list.
		// The description of the onlinestatus is used as the qobject name
		// This is safe as long as OnlineStatus are immutable
		QByteArray actionName = status.description().toAscii();
		if ( !( action = account->findChild<KAction*>( actionName ) ) )
		{
			if(options & OnlineStatusManager::HasStatusMessage)
			{
				action = new AwayAction( status, caption, status.iconFor(account), KShortcut(), account,
						SLOT( setOnlineStatus( const Kopete::OnlineStatus&, const Kopete::StatusMessage& ) ),
						0 );
			}
			else
			{
				action = new OnlineStatusAction( status, caption, status.iconFor(account), account );
				connect(action, SIGNAL(activated(const Kopete::OnlineStatus&)) ,
                                        account, SLOT(setOnlineStatus(const Kopete::OnlineStatus&)));
			}
                        action->setObjectName( actionName ); // for the lookup by name above
		}

		if( options & OnlineStatusManager::DisabledIfOffline )
			action->setEnabled( account->isConnected() );

		if(parent)
			parent->addAction(action);

	}
}


class OnlineStatusAction::Private
{
public:
	Private( const OnlineStatus& t_status )
	 : status(t_status)
	{}

	OnlineStatus status;
};
OnlineStatusAction::OnlineStatusAction( const OnlineStatus& status, const QString &text, const QIcon &pix, QObject *parent )
	: KAction( KIcon(pix), text, parent) , d( new Private(status) )
{
	setShortcut( KShortcut() );

	connect(this, SIGNAL(triggered(bool)), this, SLOT(slotActivated()));

	connect(parent,SIGNAL(destroyed()),this,SLOT(deleteLater()));
}

OnlineStatusAction::~OnlineStatusAction()
{
	delete d;
}

void OnlineStatusAction::slotActivated()
{
	emit activated(d->status);
}


} //END namespace Kopete

#include "kopeteonlinestatusmanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

