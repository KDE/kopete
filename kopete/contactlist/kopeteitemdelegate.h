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

#ifndef KOPETEITEMDELEGATE_H
#define KOPETEITEMDELEGATE_H

#include <QAbstractItemDelegate>
#include <QSize>


class KopeteItemDelegate : public QAbstractItemDelegate
{
	public:
		KopeteItemDelegate( QAbstractItemView* parent = 0 );
		~KopeteItemDelegate();
		
		virtual void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
		virtual QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

#endif

