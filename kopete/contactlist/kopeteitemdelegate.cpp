/*
    Kopete View Item Delegate

    Copyright (c) 2007 by Matt Rogers <mattr@kde.org>
    Based on code by Trolltech.

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
    QVariant value = index.data(Qt::SizeHintRole);
    if ( value.isValid() )
        return qvariant_cast<QSize>(value);
    
    if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
        return QItemDelegate::sizeHint( option, index );
        
    QSize hint;
    
    hint.setHeight(32); //hardcoded for now
    hint.setWidth( option.rect.width() );
    
    return hint;
}

void KopeteItemDelegate::paint( QPainter* painter, 
                                const QStyleOptionViewItem& option,
                                const QModelIndex& index ) const
{
    //pull in contact settings: idleContactColor, greyIdleMetaContacts
    //pull in contact list settings: contactListDisplayMode
    QStyleOptionViewItem opt = option;
    if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
        opt.decorationPosition = QStyleOptionViewItem::Right;

    QItemDelegate::paint( painter, opt, index );
}                            

