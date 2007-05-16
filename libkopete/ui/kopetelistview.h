/*
    kopetelistview.h - List View providing extra support for ListView::Items
    
    Copyright (c) 2005      by Engin AYDOGAN <engin@bzzzt.biz>
    Copyright (c) 2004      by Richard Smith <kde@metafoo.co.uk>

    Kopete    (c) 2004      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETE_LISTVIEW_H
#define KOPETE_LISTVIEW_H

#include <klistview.h>

namespace Kopete {
namespace UI {
namespace ListView {

/**
 * @author Engin AYDOGAN <engin@bzzzt.biz>
 * @author Richard Smith <kde@metafoo.co.uk>
 */
class ListView : public KListView
{
	Q_OBJECT

public:
	ListView( QWidget *parent = 0, const char *name = 0 );
	~ListView();

	/**
	 * Schedule a delayed sort operation. Sorts will be withheld for at most
	 * half a second, after which they will be performed. This way multiple
	 * sort calls can be safely bundled without writing complex code to avoid
	 * the sorts entirely.
	 */
	void delayedSort();

	/**
	 * Set whether to show the lines and +/- boxes in the tree
	 */
	void setShowTreeLines( bool bShowAsTree );

public slots:
	/**
	 * Calls QListView::sort()
	 */
	void slotSort() { sort(); }
protected:
	virtual void keyPressEvent( QKeyEvent *e );
private slots:
	void slotContextMenu(KListView*,QListViewItem *item, const QPoint &point );
	void slotDoubleClicked( QListViewItem *item );
private:
	struct Private;
	Private *d;
};

} // END namespace ListView
} // END namespace UI
} // END namespace Kopete

#endif

// vim: set noet ts=4 sts=4 sw=4:
