/*
    kopetelistview.h - List View providing extra support for ListView::Items

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
 * This class is a List View with extra support for component enabled list view items
 *
 * @author Engin AYDOGAN <engin.bzzzt.biz>
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
	void slotConfigChanged();

protected:
	virtual void keyPressEvent( QKeyEvent *e );
	virtual void enterEvent( QEvent *e );
	virtual void leaveEvent( QEvent *e );
	virtual void contentsMouseMoveEvent( QMouseEvent *e );
/*
	virtual void contentsDragEnterEvent( QDragEnterEvent *e );
	virtual void contentsDragLeaveEvent( QDragLeaveEvent *e );
	virtual void contentsDragMoveEvent( QDragMoveEvent *e );
*/
	virtual void mousePressEvent( QMouseEvent *e );
/*
	virtual void mouseDragEnterEvent( QDragEnterEvent *e );
	virtual void mouseDragMoveEvent( QDragMoveEvent *e );
	virtual void mouseDragLeaveEvent( QDragLeaveEvent *e );
*/
	virtual void contentsWheelEvent( QWheelEvent *e );
	virtual void timerEvent( QTimerEvent *e );
	virtual bool eventFilter ( QObject * o, QEvent * e );
private slots:
	void slotContextMenu(KListView*,QListViewItem *item, const QPoint &point );
	void slotDoubleClicked( QListViewItem *item );
	void slotOnItem( QListViewItem *item );
	void slotCurrentChanged( QListViewItem *item );
	void slotSliderPress();
	void slotSliderRelease();
	void slotSliderMoved( int pos );
	void slotSliderNextPage();
	void slotSliderPrevPage();
	void slotSliderNextLine();
	void slotSliderPrevLine();
private:
	void transformListViewYtoScrollY( int y );
	struct Private;
	Private *d;
};

} // END namespace ListView
} // END namespace UI
} // END namespace Kopete

#endif

// vim: set noet ts=4 sts=4 sw=4:
