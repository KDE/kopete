/*
    Kopete View Item Delegate

    Copyright (c) 2007 by Matt Rogers <mattr@kde.org>
    Copyright (c) 2009 by Roman Jarosz <kedgedev@gmail.com>

    Kopete    (c) 2002-2009 by the Kopete developers  <kopete-devel@kde.org>

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

#include <QStyledItemDelegate>
#include <QSize>

#include "contactlistlayoutitemconfig.h"

#include <kopete_export.h>

class QPainter;
class QAbstractItemView;

namespace Kopete { class Contact; }

class KOPETE_CONTACT_LIST_EXPORT KopeteItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	KopeteItemDelegate( QAbstractItemView* parent = 0 );
	~KopeteItemDelegate();

	static QFont normalFont( const QFont& naturalFont );
	static QFont smallFont( const QFont& naturalFont );

	virtual void paint ( QPainter * painter, 
	                     const QStyleOptionViewItem & option,
	                     const QModelIndex & index ) const;
	virtual QSize sizeHint ( const QStyleOptionViewItem & option,
	                         const QModelIndex & index ) const;

	Kopete::Contact* contactAt( const QStyleOptionViewItem& option, const QModelIndex& index, const QPoint& point ) const;

public slots:
	bool helpEvent( QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index );

private:
	void paintItem( ContactList::LayoutItemConfig config, QPainter* painter,
	                const QStyleOptionViewItem& option, const QModelIndex& index,
	                QList<QPair<QRect, Kopete::Contact*> >* contactPositionList ) const;

	QPointF centerImage( const QImage& image, const QRectF& rect ) const;
	QPointF centerImage( const QPixmap& pixmap, const QRectF& rect ) const;
	qreal calculateRowHeight( const ContactList::LayoutItemConfigRow &row, const QFont &normal, const QFont &small ) const;
	QList<Kopete::Contact*> filterContacts( const QList<Kopete::Contact*> contacts ) const;

	enum LayoutType { LayoutAuto = 0, LayoutNormal, LayoutFixed };
	struct DynamicLayoutItem {
		bool dirty;
		LayoutType type;
		qreal width;

		// Caching
		QString text;
		QFont font;
	};
};

#endif

