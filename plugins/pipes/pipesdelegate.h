/*
    pipesdelegate.h

    Copyright (c) 2007      by Charles Connell <charles@connells.org>

    Kopete    (c) 2007      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef PIPESDELEGATE_H
#define PIPESDELEGATE_H

#include <QItemDelegate>

class QSize;

/**
 * Delegate that allows options for pipes to be edited in place
 * @author Charles Connell <charles@connells.org>
 */

class PipesDelegate : public QItemDelegate
{
		Q_OBJECT

	public:
		enum PipesColumns { EnabledColumn = 0, DirectionColumn = 1, ContentsColumn = 2, PathColumn = 3 };

		enum { TotalColumns = 4 };

	public:
		PipesDelegate ( QObject *parent = 0 );

		QWidget *createEditor ( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
		void setEditorData ( QWidget *editor, const QModelIndex &index ) const;
		void setModelData ( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;
		void updateEditorGeometry ( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
		void paint ( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
};

#endif
