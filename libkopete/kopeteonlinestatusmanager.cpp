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

// This code was forked from the broken KImageEffect::blendOnLower, but it's
// been so heavily fixed and rearranged it's hard to recognise that now.
static void blendOnLower( const QImage &upper_, QImage &lower )
{
	if ( upper_.width() <= 0 || upper_.height() <= 0 )
		return;
	if ( lower.width() <= 0 || lower.height() <= 0 )
		return;
	
	QImage upper = upper_;
	if ( upper.depth() != 32 )
		upper = upper.convertDepth( 32 );
	if ( lower.depth() != 32 )
		lower = lower.convertDepth( 32 );
	
	int cw = std::min( upper.width(), lower.width() ),
	    ch = std::min( upper.height(), lower.height() );
	const int m = 255;
	
	for ( int j = 0; j < ch; j++ )
	{
		QRgb *u = (QRgb*)upper.scanLine(j);
		QRgb *l = (QRgb*)lower.scanLine(j);
		
		for( int k = cw; k; ++u, ++l, --k )
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

QPixmap* OnlineStatusManager::renderIcon( const OnlineStatus &statusFor, const QString& baseIcon, int size, QColor color, bool idle) const
{
	// create an icon suiting the status from the base icon
	// use reasonable defaults if not provided or protocol not set

	if ( baseIcon == statusFor.overlayIcon() )
		kdWarning( 14010 ) << "Base and overlay icons are the same - icon effects will not be visible." << endl;
	
	QPixmap* basis = new QPixmap( SmallIcon( baseIcon ) );

	// Colorize
	if ( color.isValid() )
		*basis = KIconEffect().apply( *basis, KIconEffect::Colorize, 1, color, 0);

	// Apply standard Disabled effect to generate account-offline icons
	// Note that we do this before compositing the overlay, since we want
	// that to be colored in this case.
	if ( statusFor.internalStatus() == Kopete::OnlineStatus::AccountOffline )
		*basis = KIconEffect().apply( *basis, KIcon::Small, KIcon::DisabledState );

	//composite the iconOverlay for this status and the supplied baseIcon
	if ( !( statusFor.overlayIcon().isNull() ) ) // otherwise leave the basis as-is
	{
		KIconLoader *loader = KGlobal::instance()->iconLoader();
		QPixmap overlay = loader->loadIcon(statusFor.overlayIcon(), KIcon::Small, 0 ,  KIcon::DefaultState, 0L, /*canReturnNull=*/ true );
		
		if ( !overlay.isNull() )
		{
			// we want to preserve the alpha channels of both basis and overlay.
			// there's no way to do this in Qt. In fact, there's no way to do this
			// in KDE since KImageEffect is so badly broken.
			QImage basisImage = basis->convertToImage();
			blendOnLower( overlay.convertToImage(), basisImage );
			basis->convertFromImage( basisImage );
		}
	}

	// Apply standard Disabled effect to generate Offline icons
	// This will probably look crap on the Unknown icon
	// FIXME This won't return icons that are not installed using Martijn's
	// automake magic so we'd have to use UserIcon instead of SmallIcon
	if ( statusFor.status() == OnlineStatus::Offline )
		*basis = KIconEffect().apply( *basis, KIcon::Small, KIcon::DisabledState );

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
		if(options & OnlineStatusManager::HasAwayMessage)
		{
			action = new AwayAction( status, caption, status.iconFor(account), 0, account,
					SLOT( setOnlineStatus( const Kopete::OnlineStatus&, const QString& ) ),
					account );
		}
		else
		{
			action=new OnlineStatusAction( status, caption, status.iconFor(account) , parent  );
			connect(action,SIGNAL(activated(const Kopete::OnlineStatus&)) , account, SLOT(setOnlineStatus(const Kopete::OnlineStatus&)));
		}

		if(options & OnlineStatusManager::DisabledIfOffline  && !account->isConnected())
			action->setEnabled(false);
		
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

