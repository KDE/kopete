/*
    kopetelistview.cpp - List View providing support for ListView::Items

    Copyright (c) 2005      by Engin Aydogan          <engin@bzzzt.biz>
    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>

    Kopete    (c) 2004      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetelistview.h"
#include "kopetelistviewitem.h"
#include "kopeteuiglobal.h"
#include "kopeteglobal.h"
#include "kopeteprefs.h"

#include <kdebug.h>

#include <qapplication.h>
#include <qcursor.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qevent.h>

#include <utility>
#include <memory>

namespace Kopete {
namespace UI {
namespace ListView {

/*
 Custom QToolTip for the list view.
 The decision whether or not to show tooltips is taken in
 maybeTip(). See also the QListView sources from Qt itself.
 Delegates to the list view items.
*/
class ToolTip : public QToolTip
{
public:
	ToolTip( QWidget *parent, ListView *lv );
	virtual ~ToolTip();

	void maybeTip( const QPoint &pos );

private:
	ListView *m_listView;
};

ToolTip::ToolTip( QWidget *parent, ListView *lv )
 : QToolTip( parent )
{
	m_listView = lv;
}

ToolTip::~ToolTip()
{
}

void ToolTip::maybeTip( const QPoint &pos )
{
	if( !parentWidget() || !m_listView )
		return;

	if( Item *item = dynamic_cast<Item*>( m_listView->itemAt( pos ) ) )
	{
		QRect itemRect = m_listView->itemRect( item );

		uint leftMargin = m_listView->treeStepSize() *
		   ( item->depth() + ( m_listView->rootIsDecorated() ? 1 : 0 ) ) +
		   m_listView->itemMargin();

		uint xAdjust = itemRect.left() + leftMargin;
		uint yAdjust = itemRect.top();
		QPoint relativePos( pos.x() - xAdjust, pos.y() - yAdjust );

		std::pair<QString,QRect> toolTip = item->toolTip( relativePos );
		if ( toolTip.first.isEmpty() )
			return;

		toolTip.second.moveBy( xAdjust, yAdjust );
// 		kdDebug( 14000 ) << k_funcinfo << "Adding tooltip: itemRect: "
// 		                 << toolTip.second << ", tooltip:  " << toolTip.first << endl;
		tip( toolTip.second, toolTip.first );
	}
}

struct ListView::Private
{
	/**
	 * This will trigger an update request which will makes the items try to catch
	 * the cursor and react to user behaviour.
	 */ 
	int animateTimer;
	//! Current positions of mouse cursor on the contact list
	int x, y;
	//! this is the value which scroll bar will be adjusted to by timerEvent
	int targetScrollBarValue;
	int mouseDragStartY;
	double currentMetaScrollBarValue; // cool name eh?
	int constMouseDragScale;
	int constMouseDragOffset;
	int prevMouseShape;
	//! true if the mouse is over the ListView widget
	bool mouseOver;
	//! true if the vertical scroll bar is pressed
	bool scrollPress;
	//! true if user selected Mouse Navigation from configuration dialog
	bool prefMouseNavigation;
	//! true if user selected auto-hide scrollbar from configuration dialog
	bool prefVScrollHide;
	bool mousePressed;
	bool mouseMidButtonPressed;
	QTimer sortTimer;
	std::auto_ptr<ToolTip> toolTip;
	Private() : animateTimer(0), x(0), y(0), mouseOver(false),
		    scrollPress(false), prefMouseNavigation(false),
		    prefVScrollHide(0), targetScrollBarValue(0),
		    currentMetaScrollBarValue(0.0), mousePressed(false),
		    mouseMidButtonPressed(false), mouseDragStartY(0),
		    constMouseDragScale(2), constMouseDragOffset(10),
		    prevMouseShape(0) {}
};

ListView::ListView( QWidget *parent, const char *name )
 : KListView( parent, name ), d( new Private )
{
	connect ( KopetePrefs::prefs(), SIGNAL(saved()), this, SLOT(slotConfigChanged()) );

	connect( &d->sortTimer, SIGNAL( timeout() ), this, SLOT( slotSort() ) );

	// We have our own tooltips, don't use the default QListView ones
	setShowToolTips( false );
	d->toolTip.reset( new ToolTip( viewport(), this ) );

	connect( this, SIGNAL( contextMenu( KListView *, QListViewItem *, const QPoint & ) ),
	         SLOT( slotContextMenu( KListView *, QListViewItem *, const QPoint & ) ) );
	connect( this, SIGNAL( doubleClicked( QListViewItem * ) ),
	         SLOT( slotDoubleClicked( QListViewItem * ) ) );

	// set up flags for nicer painting
	clearWFlags( WStaticContents );
	setWFlags( WNoAutoErase );

	// clear the appropriate flags from the viewport - qt docs say we have to mask
	// these flags out of the QListView to make weirdly painted list items work, but
	// that doesn't do the job. masking them out of the viewport does.
//	class MyWidget : public QWidget { public: using QWidget::clearWFlags; };
//	static_cast<MyWidget*>( viewport() )->clearWFlags( WStaticContents );
//	static_cast<MyWidget*>( viewport() )->setWFlags( WNoAutoErase );

	// The above causes compiler errors with the (broken) native TRU64 and IRIX compilers.
	// This should make it compile for both platforms and still seems to work.
	// This is, of course, a nasty hack, but it works, so...
	static_cast<ListView*>(viewport())->clearWFlags( WStaticContents );
	static_cast<ListView*>(viewport())->setWFlags( WNoAutoErase );

	d->animateTimer = startTimer( 30 );

	connect( this, SIGNAL( onItem( QListViewItem* ) ), this, SLOT( slotOnItem( QListViewItem* ) ) );

	connect( QScrollView::verticalScrollBar(), SIGNAL( sliderMoved(int) ), this, SLOT( slotSliderMoved(int) ) );
	connect( QScrollView::verticalScrollBar(), SIGNAL( nextPage() ), this, SLOT( slotSliderNextPage() ) );
	connect( QScrollView::verticalScrollBar(), SIGNAL( prevPage() ), this, SLOT( slotSliderPrevPage() ) );
	connect( QScrollView::verticalScrollBar(), SIGNAL( nextLine() ), this, SLOT( slotSliderNextLine() ) );
	connect( QScrollView::verticalScrollBar(), SIGNAL( prevLine() ), this, SLOT( slotSliderPrevLine() ) );
	connect( this, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotCurrentChanged(QListViewItem*) ) );

	slotConfigChanged();
}

ListView::~ListView()
{
	delete d;
}

void ListView::slotConfigChanged()
{
	KopetePrefs *p = KopetePrefs::prefs();
	// Check if config changed or not
	if( ( d->prefMouseNavigation != p->contactListMouseNavigation() ) ){
		// if the mouse navigation is just enabled, make the scrolling stay at the same place
		if( p->contactListMouseNavigation() )
		{
			d->targetScrollBarValue = QScrollView::verticalScrollBar()->value();
		}
		// update the configuration parameter
		d->prefMouseNavigation = p->contactListMouseNavigation();
	}
	
	// Check if config changed or not
	if( d->prefVScrollHide != p->contactListAutoHideVScroll() )
	{
		d->prefVScrollHide = p->contactListAutoHideVScroll();
	}

	if( d->prefMouseNavigation || d->prefVScrollHide )
	{
		setVScrollBarMode( AlwaysOff );
		connect( QScrollView::verticalScrollBar(), SIGNAL( sliderPressed() ),this, SLOT( slotSliderPress() ) );
		connect( QScrollView::verticalScrollBar(), SIGNAL( sliderReleased() ), this, SLOT( slotSliderRelease() ) );
	} else {
		setVScrollBarMode( Auto );
		disconnect( QScrollView::verticalScrollBar(), SIGNAL( sliderPressed() ),this, SLOT( slotSliderPress() ) );
		disconnect( QScrollView::verticalScrollBar(), SIGNAL( sliderReleased() ), this, SLOT( slotSliderRelease() ) );
	}
}

/**
 * This is the function which tries to adjust the scroll bar to it's target value
 */
void ListView::timerEvent( QTimerEvent *e )
{
	if( e->timerId() == d->animateTimer ){
		QScrollBar *verticalScrollBar = QScrollView::verticalScrollBar();
		const double k = 6.0;
		double offset = static_cast<double>( ( d->targetScrollBarValue - d->currentMetaScrollBarValue ) / k );
		d->currentMetaScrollBarValue += offset;
		verticalScrollBar->setValue( static_cast<int>(d->currentMetaScrollBarValue ) );
	}
}

bool ListView::eventFilter ( QObject * o, QEvent * e )
{
	// To make the list smooth scrool when the selection changed due to every possible key event
	// without re-implementing keyPressEvent from scratch which is error-prone, we used signal currentChanged
	// which will be emitted when the item of the listview is changed due to anything. With currentChange signal we do smooth
	// scroll  for every selection change, even for mouse clicks. But we only want to do smooth scrolling for
	// key events within currentChanged. This code eleminates mousePressEvent.

	// Note: we're using static_cast due to a bug in Qt
	
	if( e->type() == QEvent::MouseButtonPress )
	{
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);	
		// mark that mouse is pressed, so don't scroll the list to the 
		// currentChanged() item
		mousePressEvent( mouseEvent );
	}
	/* MIDBUTTON DRAG
	else if( e->type() == QEvent::DragEnter )
	{
		if( !d->prefMouseNavigation )
		{
			QDragEnterEvent *event = static_cast<QDragEnterEvent*>(e);
			mouseDragEnterEvent( event );
		}
	}
	else if( e->type() == QEvent::DragMove )
	{
		if( !d->prefMouseNavigation )
		{
			QDragMoveEvent *event = static_cast<QDragMoveEvent*>(e);
			mouseDragMoveEvent( event );
		}
	}
	else if( e->type() == QEvent::DragLeave )
	{
		if( !d->prefMouseNavigation )
		{
			QDragLeaveEvent *event = static_cast<QDragLeaveEvent*>(e);
			mouseDragLeaveEvent( event );
		}
	}
	*/
	KListView::eventFilter( o, e );
}

/*
void ListView::mouseDragEnterEvent( QDragEnterEvent *e )
{
	d->prevMouseShape = cursor().shape();
	setCursor(Qt::SizeAllCursor);
	d->mouseDragStartY = e->pos().y();
}

void ListView::mouseDragMoveEvent( QDragMoveEvent *e )
{
	d->targetScrollBarValue += ( e->pos().y() - d->mouseDragStartY ) * d->constMouseDragScale;
	d->mouseDragStartY = e->pos().y();
}

void ListView::mouseDragLeaveEvent( QDragLeaveEvent *e )
{
	setCursor( d->prevMouseShape );
}

void ListView::contentsDragEnterEvent( QDragEnterEvent *e )
{
}

void ListView::contentsDragLeaveEvent( QDragLeaveEvent *e )
{
}

void ListView::contentsDragMoveEvent( QDragMoveEvent *e )
{
}
*/

void ListView::slotCurrentChanged( QListViewItem *item )
{
	kdDebug( 14000 ) << k_funcinfo << endl;
	if( d->mousePressed ){ d->mousePressed = false; return; }
	d->targetScrollBarValue = itemPos(item) - static_cast<double>(visibleHeight()/2.0) + item->height();
}

void ListView::contentsWheelEvent( QWheelEvent *e )
{
	// Smoothen the mouse wheel
	if( !d->prefMouseNavigation ){
		d->targetScrollBarValue = QScrollView::verticalScrollBar()->value() - e->delta();
		e->accept();
	}
}

void ListView::slotSliderNextPage()
{
	// Using QMIN trick because otherwise the targetScrollBarValue will be much more than maximum value
	// so excessive nextPage clicks will point to very high value, then to make it point a valid lower value
	// we should make as much prevPage clicks as we made to nextPage
	d->targetScrollBarValue = QMIN( d->targetScrollBarValue + QScrollView::verticalScrollBar()->pageStep(),
				        QScrollView::verticalScrollBar()->maxValue() );
}

void ListView::slotSliderPrevPage()
{
	d->targetScrollBarValue = QMAX( d->targetScrollBarValue - QScrollView::verticalScrollBar()->pageStep(),
					QScrollView::verticalScrollBar()->minValue() );
}

void ListView::slotSliderNextLine()
{
	d->targetScrollBarValue = QMIN( d->targetScrollBarValue + QScrollView::verticalScrollBar()->lineStep(),
					QScrollView::verticalScrollBar()->maxValue() );
}

void ListView::slotSliderPrevLine()
{
	d->targetScrollBarValue = QMAX( d->targetScrollBarValue - QScrollView::verticalScrollBar()->lineStep(),
					QScrollView::verticalScrollBar()->minValue() );
}

void ListView::slotSliderMoved( int pos )
{
	d->targetScrollBarValue = pos;
	d->currentMetaScrollBarValue = pos;
}

void ListView::slotSliderPress()
{
	kdDebug( 14000 ) << k_funcinfo << "mouse over:" << (d->mouseOver?"true":"false") << endl;
	if( !d->prefMouseNavigation && d->prefVScrollHide )
	{
		d->scrollPress = true;
	}
}

void ListView::slotSliderRelease()
{
	kdDebug( 14000 ) << k_funcinfo << endl;
	
	if( !d->prefMouseNavigation && d->prefVScrollHide )
	{
		d->scrollPress = false;
		if( !geometry().contains( mapFromGlobal( QCursor::pos() ) ) )
			setVScrollBarMode( AlwaysOff );
	}
}
void ListView::mousePressEvent( QMouseEvent *e )
{
	kdDebug(14000) << k_funcinfo << endl;
	d->mousePressed = true;
	if( e->button() == MidButton ){
		kdDebug( 14000 ) << k_funcinfo << "mid button" << endl;
		d->mouseMidButtonPressed = true;
	}
}

void ListView::transformListViewYtoScrollY( int y )
{
	const int offset = ( static_cast<double>(visibleHeight()) / 50.0 ) + d->constMouseDragOffset;
	QScrollBar *verticalScrollBar = QScrollView::verticalScrollBar();
	int newValue= ( y - offset ) * ( static_cast<double>(verticalScrollBar->maxValue()) / static_cast<double>( ( visibleHeight() - offset * 2 ) ) );
	d->targetScrollBarValue = newValue;
}

void ListView::contentsMouseMoveEvent( QMouseEvent *e )
{
	if( e->button() != MidButton )
		QListView::contentsMouseMoveEvent( e ); // without this onItem signal won't be emitted

	d->x = e->x() - contentsX();
	d->y = e->y() - contentsY();

	if( d->prefMouseNavigation )
	{
		transformListViewYtoScrollY( d->y );
	}
	else 
	{
		if( d->prefVScrollHide )
		{
			// Work around for making scrollbar available when mouse 
			// is released in the kopete contact list after being pressed on
			// scroll bar
			setVScrollBarMode( Auto );
		}
	}
}

void ListView::enterEvent( QEvent *e )
{
	kdDebug( 14000 ) << k_funcinfo << "mouse in: " << e->type() << endl;
	d->mouseOver = true;
	if( !d->prefMouseNavigation && d->prefVScrollHide )
	{
		setVScrollBarMode( Auto );
	}
}

void ListView::leaveEvent( QEvent *e )
{
	kdDebug( 14000 ) << k_funcinfo << "mouse out: " << e->type() << endl;
	d->mouseOver = false;
	if( !d->prefMouseNavigation && d->prefVScrollHide )
	{
		if( !d->scrollPress )
			setVScrollBarMode( AlwaysOff );
	}
}

void ListView::slotOnItem( QListViewItem *i )
{
	kdDebug( 14000 ) << k_funcinfo << endl;
	if( i ){
		Item *item = dynamic_cast<Item*>(i);
		if( item ){
		} else {
			kdDebug( 14000 ) << k_funcinfo << "dynamic_cast<KopeteMetaContactLVI*> failed." << endl;
		}
	}
}

void ListView::slotDoubleClicked( QListViewItem *item )
{
	kdDebug( 14000 ) << k_funcinfo << endl;

	if ( item )
		setOpen( item, !isOpen( item ) );
}

void ListView::slotContextMenu( KListView * /*listview*/,
	QListViewItem *item, const QPoint &point )
{
	if ( item && !item->isSelected() )
	{
		clearSelection();
		item->setSelected( true );
	}
	if ( !item )
		clearSelection();

	if( Item *myItem = dynamic_cast<Item*>( item ) )
		;// TODO: myItem->contextMenu( point );
}

void ListView::setShowTreeLines( bool bShowAsTree )
{
	if ( bShowAsTree )
	{
		setRootIsDecorated( true );
		setTreeStepSize( 20 );
	}
	else
	{
		setRootIsDecorated( false );
		setTreeStepSize( 0 );
	}
	// TODO: relayout all items. their width may have changed, but they won't know about it.
}

/* This is a small hack ensuring that only F2 triggers inline
 * renaming. Won't win a beauty award, but whoever wrote it thinks
 * relying on the fact that QListView intercepts and processes the
 * F2 event through this event filter is sorta safe.
 *
 * Also use enter to execute the item since executed is not usually
 * called when enter is pressed.
 */
void ListView::keyPressEvent( QKeyEvent *e )
{
	QListViewItem *item = currentItem();
	if ( (e->key() == Qt::Key_F2) && item && item->isVisible() )
		rename( item, 0 );
	else if ( (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) && item && item->isVisible() )
	{
		// must provide a point within the item; emitExecute checks for this
		QPoint p = viewport()->mapToGlobal(itemRect(item).center());
		emitExecute( currentItem(), p, 0 );
	}
	/*
	// UP key was somehow grabbed and implemented before that I couldn't handle it for smooth scrolling so I made this!
	else if( e->key() == Qt::Key_Up )
	{
		QListViewItem *item = currentItem();
		if( item )
		{
			item->setSelected( false );
			if( item = item->itemAbove() )
			{
				setCurrentItem( item );
			}
		} else {
			if( item = firstChild() ){
				setCurrentItem( item );
			}
		}
		if( item )
		{
			item->setSelected( true );
			d->targetScrollBarValue = itemPos( item );
		}
		e->ignore(); // let the parent handle it now
	}
	else if( e->key() == Qt::Key_Down )
	{
		QListViewItem *item = currentItem();
		if( item )
		{
			item->setSelected( false );
			if( item = item->itemBelow() )
			{
				setCurrentItem( item );
			}
		} else {
			if( item = firstChild() ){
				setCurrentItem( item );
			}
		}
		if( item )
		{
			item->setSelected( true );
			d->targetScrollBarValue = itemPos( item );
		}
		e->ignore(); // let the parent handle it now
	}  */
	else {
		KListView::keyPressEvent(e);
	}
}

void ListView::delayedSort()
{
	if ( !d->sortTimer.isActive() )
		d->sortTimer.start( 500, true );
}

} // END namespace ListView
} // END namespace UI
} // END namespace Kopete

#include "kopetelistview.moc"

// vim: set noet ts=4 sts=4 sw=4:
