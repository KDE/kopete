/*
    Kopete View Item Delegate

    Copyright (c) 2007 by Matt Rogers <mattr@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteitemdelegate.h"
#include "kopeteitembase.h"

#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QAbstractItemView>
#include <QItemDelegate>

#include <KIconLoader>

#include "kopetemetacontact.h"
#include "kopeteappearancesettings.h"

KopeteItemDelegate::KopeteItemDelegate( QAbstractItemView* parent )
: QItemDelegate( parent )
{
}

KopeteItemDelegate::~KopeteItemDelegate()
{
}

QSize KopeteItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
	if ( index.data( Kopete::Items::TypeRole ) ==
		Kopete::Items::MetaContact )
	{
		QSize photoSize = metaContactIconSize( index );
		QSize defaultSize = QItemDelegate::sizeHint( option, index );
		return QSize(defaultSize.width(), photoSize.height());
	}
	else
		return QItemDelegate::sizeHint( option, index );
}

void KopeteItemDelegate::paint( QPainter* painter, 
                                const QStyleOptionViewItem& option,
                                const QModelIndex& index ) const
{
	//pull in contact settings: idleContactColor, greyIdleMetaContacts
	//pull in contact list settings: contactListDisplayMode
	QStyleOptionViewItem opt = option;
	
	if ( index.data( Kopete::Items::TypeRole ) ==
		Kopete::Items::MetaContact )
	{
		//check the idle state of the metacontact and apply the appropriate
		//color
		QVariant v = index.data( Kopete::Items::IdleTimeRole );
		if ( Kopete::AppearanceSettings::self()->greyIdleMetaContacts() &&
		     v.toInt() > 0 )
		{
		QColor idleColor( Kopete::AppearanceSettings::self()->idleContactColor() );
		opt.palette.setColor( QPalette::Text, idleColor );
		}
		
		opt.decorationSize = metaContactIconSize( index );
	}
	
	if (  index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
	{
		QColor gc( Kopete::AppearanceSettings::self()->groupNameColor() );
		opt.palette.setColor( QPalette::Text, gc );
	}
	
	QItemDelegate::paint( painter, opt, index );
	
}

QSize KopeteItemDelegate::metaContactIconSize( const QModelIndex& index ) const
{
	Q_ASSERT( index.isValid() );
	using namespace Kopete; 
	int displayMode = AppearanceSettings::self()->contactListDisplayMode();
	int iconSize = AppearanceSettings::self()->contactListIconMode();
	int displaySize = IconSize( KIconLoader::Small );
	if ( displayMode == AppearanceSettings::EnumContactListDisplayMode::Detailed )
	{
		displaySize = ( iconSize == AppearanceSettings::EnumContactListIconMode::IconPic ? 
		                KIconLoader::SizeMedium : KIconLoader::SizeLarge );
	}
	else
	{
		displaySize = ( iconSize == Kopete::AppearanceSettings::EnumContactListIconMode::IconPic ?
		                IconSize( KIconLoader::Small ) :  KIconLoader::SizeMedium );
	}
	return QSize(displaySize, displaySize);
}

