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
	return QSize(45, 20);
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
	}
	
	if (  index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
	{
		QColor gc( Kopete::AppearanceSettings::self()->groupNameColor() );
		opt.palette.setColor( QPalette::Text, gc );
	}
	
}

