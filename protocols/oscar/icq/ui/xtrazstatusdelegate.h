/*
    xtrazstatusdelegate.h  -  Xtraz Status Delegate

    Copyright (c) 2007 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef XTRAZSTATUSDELEGATE_H
#define XTRAZSTATUSDELEGATE_H

#include <QItemDelegate>
#include <QModelIndex>

#include <QList>

namespace Xtraz
{

class StatusDelegate : public QItemDelegate
{
    Q_OBJECT

public:
	explicit StatusDelegate( const QList<QIcon> &icons, QObject *parent = 0 );

	QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;

	void setEditorData( QWidget *editor, const QModelIndex &index ) const;

	void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;

	void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const;

private:
	QList<QIcon> mIcons;

};

}

#endif
