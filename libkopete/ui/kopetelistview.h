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

	/**
	 * Sets the smooth scrolling.
	 */
	void setSmoothScrolling( bool );

	/**
	 * Gets the smooth scrolling status
	 */
	bool smoothScrolling();

	/**
	 * Sets the update interval of smooth scrolling animation.
	 * @param interval is the interval in ms.
	 */
	void setSmoothScrollingTimerInterval( int interval );

	/**
	 * Gets the current update interval.
	 */
	int smoothScrollingTimerInterval();

	/**
	 * Sets scroll auto hide feature
	 * This method implicitly calls setIgnoreGlobalScrolLAutoHide(true) by default.
	 * If you want not to ignore global changes, explicitly call setIgnoreGlobalScrollAutoHide(false).
	 */
	void setScrollAutoHide( bool b );
	
	/**
	 * Gets scroll auto hide feature status
	 */
	bool scrollAutoHide();

	/**
	 * If b is true global settings changes won't effect the status of
	 * scroll auto-hide feature.
	 */
	void setIgnoreGlobalScrollAutoHide( bool b );

	/**
	 * If returns true, it means global changes won't effect the status
	 * of the scroll auto-hide feature.
	 */
	bool ignoreGlobalScrollAutoHide();

public slots:
	/**
	 * Calls QListView::sort()
	 */
	void slotSort() { sort(); }

	/**
	 * The function to be triggered when the configuration is changed.
	 */
	void slotConfigChanged();

protected:
	virtual void keyPressEvent( QKeyEvent *e );
	/**
	 * Invoked on each timeout of a QTimer of this listview,
	 * This will manage the smooth scrolling animation, continuous presses to the scrollbars.
	 */
	virtual void timerEvent( QTimerEvent *e );
	
	/**
	 * To make smooth scrolling work well, we need extensive event intercepting.
	 * This event filter is suppposed to achive that.
	 */
	virtual bool eventFilter( QObject *o, QEvent *e );

private slots:
	void slotContextMenu(KListView*,QListViewItem *item, const QPoint &point );
	void slotDoubleClicked( QListViewItem *item );
	/**
	 * To enable smooth scroll to focus on highlighted items when they are highlighted
	 * by a key press we use this slot. slotCurrentChanged is connected to the currentChanged
	 * signal, it's being invoked in every selection change. If the selection change was made
	 * by the mouse, then we don't do anything, since the item is on the viewable area already.
	 * Otherwise, we focus (bring it to the center of the list) smoothly.
	 */
	void slotCurrentChanged( QListViewItem *item );
private:
	void setScrollAutoHideInternal( bool b );
	struct Private;
	Private *d;
};

} // END namespace ListView
} // END namespace UI
} // END namespace Kopete

#endif

// vim: set noet ts=4 sts=4 sw=4:
