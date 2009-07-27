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

#include <k3listview.h>
#include <kopete_export.h>

class QKeyEvent;
class QTimerEvent;
class QEvent;

namespace Kopete {
namespace UI {
namespace ListView {

/**
 * @author Engin AYDOGAN <engin@bzzzt.biz>
 * @author Richard Smith <kde@metafoo.co.uk>
 */
class KOPETE_EXPORT ListView : public K3ListView
{
	Q_OBJECT

public:
	ListView( QWidget *parent = 0 );
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
	bool smoothScrolling() const;

	/**
	 * Sets the update interval of smooth scrolling animation.
	 * @param interval is the interval in ms.
	 */
	void setSmoothScrollingTimerInterval( double interval );

	/**
	 * Gets the current update interval.
	 */
	double smoothScrollingTimerInterval() const;

	/**
	 * Sets scroll auto hide feature
	 * This method implicitly calls setIgnoreGlobalScrolLAutoHide(true) by default.
	 * If you want not to ignore global changes, explicitly call setIgnoreGlobalScrollAutoHide(false).
	 *
	 * Note: No matter what is the previous vScrollBarMode is, when `false' is provided to this method
	 * new vScrollBarMode will be `Auto'. So it's programmers responsibility to keep track of vScrollBarMode
	 * @see setVScrollBarMode( ScrollBarMode )
	 */
	void setScrollAutoHide( bool b );

	/**
	 * Gets scroll auto hide feature status
	 */
	bool scrollAutoHide() const;

	/**
	 * Sets the amount of time to hide scrollbar after inactivity. ( in seconds )
	 * Note: If not called, default value 10 will be used.
	 */
	void setScrollAutoHideTimeout( int );

	/**
	 * Gets the scroll bar auto-hide timeout value.
	 */
	int scrollAutoHideTimeout() const;

	/**
	 * Sets always hide feature
	 * Note: No matter what is the previous vScrollBarMode is, when `false' is provided to this method
	 * new vScrollBarMode will be `Auto'. So it's programmers responsibility to keep track of vScrollBarMode
	 * @see setVScrollBarMode( ScrollBarMode )
	 */
	void setScrollHide( bool b );

	/**
	 * Gets always hide feature state
	 */
	bool scrollHide() const;

	/**
	 * Sets the mouse navigation status.
	 * Contact list slider is always automagically adjusted to the content's Y position,
	 * which means, the scroll bar's slider will follow your cursor while it on the contact
	 * list.
	 */
	void setMouseNavigation( bool b );

	/**
	 * Gets the mouse navigation status
	 */
	bool mouseNavigation() const;
public slots:
	/**
	 * Calls QListView::sort()
	 */
	void slotSort() { sort(); }

protected:
	virtual void keyPressEvent( QKeyEvent *e );
	/**
	 * Invoked on each timeout of a QTimer of this listview,
	 * This will manage the smooth scrolling animation, continuous presses to the scrollbars.
	 */
	virtual void timerEvent( QTimerEvent *e );

	/**
	 * To make smooth scrolling work well, we need extensive event intercepting.
	 * This event filter is suppposed to achieve that.
	 */
	virtual bool eventFilter( QObject *o, QEvent *e );

signals:
	void visibleSizeChanged ();

private slots:
	void slotContextMenu(K3ListView*,Q3ListViewItem *item, const QPoint &point );
	void slotDoubleClicked( Q3ListViewItem *item );
	/**
	 * To enable smooth scroll to focus on highlighted items when they are highlighted
	 * by a key press we use this slot. slotCurrentChanged is connected to the currentChanged
	 * signal, it's being invoked in every selection change. If the selection change was made
	 * by the mouse, then we don't do anything, since the item is on the viewable area already.
	 * Otherwise, we focus (bring it to the center of the list) smoothly.
	 */
	void slotCurrentChanged( Q3ListViewItem *item );

private:
	class Private;
	Private * const d;
};

} // END namespace ListView
} // END namespace UI
} // END namespace Kopete

#endif

// vim: set noet ts=4 sts=4 sw=4:
