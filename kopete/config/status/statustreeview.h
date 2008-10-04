/*
    statustreeview.h

    Copyright (c) 2008       by Roman Jarosz           <kedgedev@centrum.cz>
    Kopete    (c) 2008       by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef STATUSTREEVIEW_H
#define STATUSTREEVIEW_H

#include <QtGui/QTreeView>
#include <QtGui/QDrag>

class StatusTreeView : public QTreeView
{
public:
	StatusTreeView( QWidget * parent = 0 ) : QTreeView( parent ) {}
	
protected:
	// FIXME: is there an easier way to set default action of QDrag to Qt::MoveAction
	virtual void startDrag( Qt::DropActions supportedActions )
	{
		QModelIndexList indexes = selectedIndexes();
		if ( indexes.count() > 0 )
		{
			QMimeData *data = model()->mimeData( indexes );
			if ( !data )
				return;

			QDrag *drag = new QDrag( this );
			drag->setMimeData( data );
			if ( drag->exec( supportedActions, Qt::MoveAction ) == Qt::MoveAction )
			{
				const QItemSelection selection = selectionModel()->selection();
				QList<QItemSelectionRange>::const_iterator it = selection.begin();

				for ( ; it != selection.end(); ++it )
				{
					QModelIndex parent = (*it).parent();
					if ( (*it).left() != 0 )
						continue;
					if ( (*it).right() != ( model()->columnCount(parent) - 1 ) )
						continue;
					int count = (*it).bottom() - (*it).top() + 1;
					model()->removeRows( (*it).top(), count, parent );
				}
			}
		}
	}
};

#endif
