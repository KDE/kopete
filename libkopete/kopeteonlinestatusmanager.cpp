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
#include <kdebug.h>
#include <klocale.h>
#include <kstaticdeleter.h>
#include <kapplication.h>
#include <kcpuinfo.h> // for WORDS_BIGENDIAN

#include <algorithm> // for min

namespace Kopete {


class OnlineStatusManager::Private
{public:

	struct RegisteredStatusStruct
	{
		QString caption;
		unsigned int categories;
		unsigned int options;
	};

	typedef QMap< OnlineStatus , RegisteredStatusStruct >  ProtocolMap ;

	QPixmap *nullPixmap;
	QMap<Protocol* , ProtocolMap > registeredStatus;
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
			unsigned int catgs=it.data().categories;
			if(catgs & (1<<(categ_nb)))
				return it.key();
		}
		//no status found in this category, try the previous one.
		categ_nb=(int)(categ_nb/2);
	} while (categ_nb > 0);
	
	kdWarning() << "No status in the category " << category << " for the protocol " << protocol->displayName() <<endl;
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
		upper = upper.convertDepth( 32 );
	if ( lower.depth() != 32 )
		lower = lower.convertDepth( 32 );

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

		for( int k = width-1; k >= 0; --i, --k )
		{
			if ( qAlpha(*i) )
			{
				x2 = std::max( x2, k );
				y2 = std::max( y2, j );
				break;
			}
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

	if ( baseIcon == statusFor.overlayIcons().first() )
		kdWarning( 14010 ) << "Base and overlay icons are the same - icon effects will not be visible." << endl;

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
		KIconLoader *loader = KGlobal::instance()->iconLoader();

		int i = 0;
		for( QStringList::iterator it = overlays.begin(), end = overlays.end(); it != end; ++it )
		{
			QPixmap overlay = loader->loadIcon(*it, KIcon::Small, 0 ,
				KIcon::DefaultState, 0L, /*canReturnNull=*/ true );

			if ( !overlay.isNull() )
			{
				// we want to preserve the alpha channels of both basis and overlay.
				// there's no way to do this in Qt. In fact, there's no way to do this
				// in KDE since KImageEffect is so badly broken.
				QImage basisImage = basis->convertToImage();
				QImage overlayImage = overlay.convertToImage();
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
				basis->convertFromImage( basisImage );
			}
		}
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

void OnlineStatusManager::createAccountStatusActions( Account *account , KActionMenu *parent)
{
	Private::ProtocolMap protocolMap=d->registeredStatus[account->protocol()];
	Private::ProtocolMap::Iterator it;
	for ( it = --protocolMap.end(); it != protocolMap.end(); --it )
	{
		unsigned int options=it.data().options;
		if(options & OnlineStatusManager::HideFromMenu)
			continue;

		OnlineStatus status=it.key();
		QString caption=it.data().caption;
		KAction *action;

		// Any existing actions owned by the account are reused by recovering them
		// from the parent's child list.
		// The description of the onlinestatus is used as the qobject name
		// This is safe as long as OnlineStatus are immutable
		QCString actionName = status.description().ascii();
		if ( !( action = static_cast<KAction*>( account->child( actionName ) ) ) )
		{
			if(options & OnlineStatusManager::HasAwayMessage)
			{
				action = new AwayAction( status, caption, status.iconFor(account), 0, account,
						SLOT( setOnlineStatus( const Kopete::OnlineStatus&, const QString& ) ),
						account, actionName );
			}
			else
			{
				action=new OnlineStatusAction( status, caption, status.iconFor(account) , account, actionName );
				connect(action,SIGNAL(activated(const Kopete::OnlineStatus&)) ,
						account, SLOT(setOnlineStatus(const Kopete::OnlineStatus&)));
			}
		}

#if 0
		//disabled because since action are reused, they are not enabled back if the account is online.
		if(options & OnlineStatusManager::DisabledIfOffline  && !account->isConnected())
			action->setEnabled(false);
#endif

		if(parent)
			parent->insert(action);

	}
}


OnlineStatusAction::OnlineStatusAction( const OnlineStatus& status, const QString &text, const QIconSet &pix, QObject *parent, const char *name)
		: KAction( text, pix, KShortcut() , parent, name) , m_status(status)
{
	connect(this,SIGNAL(activated()),this,SLOT(slotActivated()));
}

void OnlineStatusAction::slotActivated()
{
	emit activated(m_status);
}


} //END namespace Kopete

#include "kopeteonlinestatusmanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

