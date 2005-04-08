/*
    kopetelistview.cpp - List View providing support for ListView::Items

    Copyright (c) 2004      by Engin AYDOGAN	      <engin@bzzzt.biz>
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

#include <qapplication.h>
#include <kdebug.h>

#include <qtimer.h>
#include <qtooltip.h>

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
		kdDebug( 14000 ) << k_funcinfo << "Adding tooltip: itemRect: "
		                 << toolTip.second << ", tooltip:  " << toolTip.first << endl;
		tip( toolTip.second, toolTip.first );
	}
}

struct ListView::Private
{
	QTimer sortTimer;
	std::auto_ptr<ToolTip> toolTip;
	//! The status of smooth scrolling, enabled or disabled.
	bool smoothScrollingEnabled;
	//! This will be the QTimer's ID which will be updating smooth scrolling animation.
	double smoothScrollingTimer;
	//! The time interval which the smooth scrolling will be updated.
	double smoothScrollingTimerInterval;
	//! This will be the target scroll bar position. Note that this value is in the sense of contents height in
	//! in the scroll bar, not the regular XY coordinates in the widget.
	double targetScrollBarValue;
	//! Meta current scroll bar value, this will be used to make precise calculation, note that data type is double
	//! Otherwise extra coding would be necessary to workaround lost precisions all around.
	double metaScrollBarCurrentValue;
	//! Acceleration constant. Actually deceleration.
	double scrollBarAccelerationConstant;
	//! Scroll line step size to emulate
	int smoothScrollingLineStep;
	//! Scroll page step size to emulate
	int smoothScrollingPageStep;
	//! True if the slider is being pressed. This will be used to enable slider dragging.
	bool scrollBarSliderPressed;
	//! The mouse position where the slider dragging began
	int scrollBarSliderDragStartY;
	//! True when the mouse is pressed
	bool mousePressed;
	//! This will be used to save drag auto scroll status of the scrollview
	//! we will need to restore it later.
	bool smoothScrollDragAutoScroll;
	//! Auto scroll offset, the list will automatically start scrolling when the mouse gets this much pixel closer
	//! to the upper or bottom edges of the listview.
	int smoothAutoScrollOffset;
	//! These are the state of scroll bar buttons, this are necessary if want continuous scrolling as long as we
	//!  press to these buttons.
	bool scrollBarPrevLineUpPressed;
	bool scrollBarPrevPagePressed;
	bool scrollBarNextPagePressed;
	bool scrollBarPrevLineBottomPressed;
	bool scrollBarNextLinePressed;
	//! Counter we'll use when waiting.This amount of timeouts is required before beginning to emulate continous
	//! scrollbar button presses
	int smoothScrollContinuousCounter;
	//! The timer which will simulate continous button press for line step buttons in scrollview
	int continuousLinePressTimer;
	//! How many timeouts should we wait before beginning continuous line step simulates
	int continuousLinePressTimerWait;
	//! Continuous press timer interval for next line buttton
	int continuousLinePressTimerInterval;
	//! The timer which will simulate continous page step in scrollview
	int continuousPagePressTimer;
	//! How many timeouts should we wait before beginning continuous page step simulates
	int continuousPagePressTimerWait;
	//! Continuous press timer interval for next page clicks
	int continuousPagePressTimerInterval;
	//! The timer that will manage scroll bar auto-hiding
	int scrollAutoHideTimer;
	//! Timeout counter for scroll bar auto-hiding
	int scrollAutoHideCounter;
	//! Timeout for scroll bar auto-hiding
	int scrollAutoHideTimeout;
	//! State of scroll auto hide
	bool scrollAutoHide;
	//! State of always hide scrollbar feature
	bool scrollHide;
	//! Muse navigation offset, we will ignore this much offset to the up/bottom edges
	int mouseNavigationOffset;
	//! State of mouse navigation feature
	bool mouseNavigation;
	//! C-tor
	Private()
	: smoothScrollingEnabled(false),
	  smoothScrollingTimer(0),
	  smoothScrollingTimerInterval(30),
	  targetScrollBarValue(0),
	  metaScrollBarCurrentValue(0.0),
	  scrollBarAccelerationConstant(6.0),
	  smoothScrollingLineStep(0),
	  smoothScrollingPageStep(0),
	  scrollBarSliderPressed(false),
	  scrollBarSliderDragStartY(0),
	  mousePressed(false),
	  smoothScrollDragAutoScroll(false),
	  smoothAutoScrollOffset(60),
	  scrollBarPrevLineUpPressed(false),
	  scrollBarPrevPagePressed(false),
	  scrollBarNextPagePressed(false),
	  scrollBarPrevLineBottomPressed(false),
	  scrollBarNextLinePressed(false),
	  smoothScrollContinuousCounter(0),
	  continuousLinePressTimer(0),
	  continuousLinePressTimerWait(10),
	  continuousLinePressTimerInterval(40),
	  continuousPagePressTimer(0),
	  continuousPagePressTimerWait(2),
	  continuousPagePressTimerInterval(200),
	  scrollAutoHideTimer(0),
	  scrollAutoHideCounter(10),
	  scrollAutoHideTimeout(10),
	  scrollAutoHide(false),
	  scrollHide(false),
	  mouseNavigationOffset(20),
	  mouseNavigation(false) {}
};

ListView::ListView( QWidget *parent, const char *name )
 : KListView( parent, name ), d( new Private )
{
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

	// init smooth scrolling
 	setSmoothScrolling( true );
}

ListView::~ListView()
{
	delete d;
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

//	if( Item *myItem = dynamic_cast<Item*>( item ) )
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
	else
		KListView::keyPressEvent(e);
}

void ListView::delayedSort()
{
	if ( !d->sortTimer.isActive() )
		d->sortTimer.start( 500, true );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 						Here begins the smooth scrolling hack 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// What is this: This hack enables smooth scrolling, and intercepts all events that goes to scrollbar. Unless we do so, scrollbar
//               will react to the mouse clicks and go _immedietly_ to the target value (i.e. next page) then when the smoothscroll
//		 takes effect the scrollbar slider will start to scroll smoothly, so the first slider movement due to scrollbar
//		 click causes a flickery. So we avoid all scrollbar events, and emulate them smoothly.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bugs: * Scroll bar loses it's onMouseOver color when we drag the scroll bar and move the mouse outside of the slider
//       * Next/Prev page areas don't get mouse clicks, some styles gives some feedback on such an event
//         and that feedback is unintentionally avoided by this hack. Many many contraints caused this choice.
//	 * Horizontal scroll bar seems to flicker when the scrollbars are being showed in auto-hide mode
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ListView::setSmoothScrolling( bool b )
{
	if( d->smoothScrollingEnabled == b ) 	// Is setting changed?
		return;				// If no, just return
	else					// else
		d->smoothScrollingEnabled = b;  // update the setting

	if( d->smoothScrollingEnabled )		// If enabled
	{
		// Intercept scrollbar's events
		verticalScrollBar()->installEventFilter( this );
		// Install the timer
		d->smoothScrollingTimer = startTimer( (int)d->smoothScrollingTimerInterval );
		// If we want to enable smooth scrolling when item has changed with keypresses etc, we need this
		connect( this, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotCurrentChanged(QListViewItem*) ) );
		// Disable autoscroll, we will do it the smooth way.
		d->smoothScrollDragAutoScroll = dragAutoScroll();
		setDragAutoScroll( false );
		// Init the timers to simulate continuous press
		d->continuousLinePressTimer = startTimer( d->continuousLinePressTimerInterval );
		d->continuousPagePressTimer = startTimer( d->continuousPagePressTimerInterval );
		
	}
	else	// If disabled
	{
		// Uninstall the event interception from the scroll bar
		verticalScrollBar()->removeEventFilter( this );
		// Restore line/page step sizes
		verticalScrollBar()->setLineStep( d->smoothScrollingLineStep );
		// Kill the already started timer
		killTimer( (int)d->smoothScrollingTimer );
		d->smoothScrollingTimer = 0;
		// We don't need to list currentChanged anymore
		disconnect( this, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotCurrentChanged(QListViewItem*) ) );
		// Restore the autoscroll
		setDragAutoScroll( d->smoothScrollDragAutoScroll );
		// Kill the continuous press timers
		killTimer( d->continuousLinePressTimer  );
		d->continuousLinePressTimer = 0;
		killTimer( d->continuousPagePressTimer  );
		d->continuousPagePressTimer = 0;
	}
}

bool ListView::smoothScrolling()
{
	return d->smoothScrollingEnabled;
}

void ListView::setSmoothScrollingTimerInterval( double i )
{
	d->smoothScrollingTimerInterval = i;
}

double ListView::smoothScrollingTimerInterval()
{
	return d->smoothScrollingTimerInterval;
}

void ListView::setScrollAutoHide( bool b )
{
	// If no change, just bail
	if( d->scrollAutoHide == b ) return;

	if( b )
	{
		// Set scrollbar auto-hiding state true
		d->scrollAutoHide = true;
		// Turn of the bar now
		setVScrollBarMode( AlwaysOff );
		// Start the timer to handle auto-hide
		killTimer( d->scrollAutoHideTimer );
		d->scrollAutoHideTimer = startTimer( 1000 );
	}
	else
	{
		d->scrollAutoHide = false;
		setVScrollBarMode( Auto );
		killTimer( d->scrollAutoHideTimer );
	}
}

bool ListView::scrollAutoHide() const
{
	return d->scrollAutoHide;
}

void ListView::setScrollAutoHideTimeout( int t )
{
	d->scrollAutoHideTimeout = t;
}

int ListView::scrollAutoHideTimeout() const
{
	return d->scrollAutoHideTimeout;
}

void ListView::setScrollHide( bool b )
{
	// if no change, just bail
	if( d->scrollHide == b ) return;

	d->scrollHide = b;
	if( b )
		setVScrollBarMode( AlwaysOff );
	else
		setVScrollBarMode( Auto );
}

bool ListView::scrollHide() const
{
	return d->scrollHide;
}

void ListView::setMouseNavigation( bool b )
{
	d->mouseNavigation = b;
}

bool ListView::mouseNavigation() const
{
	return d->mouseNavigation;
}

void ListView::timerEvent( QTimerEvent *e )
{
	if( e->timerId() == d->smoothScrollingTimer )
	{ // This is a smooth scroll update
		// Find how war we are away from the target scroll bar value and divide it by our constant (it can be both negative/positive)
		double offset = static_cast<double>( ( d->targetScrollBarValue - d->metaScrollBarCurrentValue ) / d->scrollBarAccelerationConstant );
		// Add the offset to our meta current value, this is the desired precise value
		d->metaScrollBarCurrentValue += offset;
		// Cast it to integer and update the vertical scroll bar value
		verticalScrollBar()->setValue( static_cast<int>( d->metaScrollBarCurrentValue ) );
	}
	else if( e->timerId() == d->continuousLinePressTimer )
	{
		// If the button is being pressed for a longer time, get faster
		// X = time spent untill continuous button press begins to take effect
		// Wait for X more amount of time after the continuous button press actually begins, the start to accelerate the scrolling linearly
		double acceleration = static_cast<double>( ( d->smoothScrollContinuousCounter - d->continuousLinePressTimerWait * 2 ) ) / 
				      static_cast<double>( d->continuousLinePressTimerWait );
		// Let's make sure if the acceleration coefficient is between 1 and 3
		acceleration = QMAX( 1, acceleration );
		acceleration = QMIN( 3, acceleration );

		// Check if any scrollbar buttons are being pressed right, if any, honor them
		if( d->scrollBarPrevLineUpPressed || d->scrollBarPrevLineBottomPressed )
		{	// Check if the user has pressed for long enough to activate continuous mouse press effect
			if( d->smoothScrollContinuousCounter++ > d->continuousLinePressTimerWait ) // pressed long enough ?
			{
				d->targetScrollBarValue -= d->smoothScrollingLineStep * acceleration; // if so start continuous scrolling
				// Make sure the target value is not below the minimum range.
				d->targetScrollBarValue = QMAX( d->targetScrollBarValue, verticalScrollBar()->minValue() );
			}
		}
		else if( d->scrollBarNextLinePressed )
		{
			if( d->smoothScrollContinuousCounter++ > d->continuousLinePressTimerWait ) // pressed long enough ?
			{
				d->targetScrollBarValue += d->smoothScrollingLineStep * acceleration; // if so start continuous scrolling
				// Make sure the target value is not aboce the maximum range.
				d->targetScrollBarValue = QMIN( d->targetScrollBarValue, verticalScrollBar()->maxValue() );
			}
		}
	}
	else if( e->timerId() == d->continuousPagePressTimer )
	{
		// If the button is being pressed for a longer time, get faster
		// X = time spent untill continuous button press begins to take effect
		// Wait for X more amount of time after the continuous button press actually begins, the start to accelerate the scrolling linearly
		double acceleration = static_cast<double>( ( d->smoothScrollContinuousCounter - d->continuousPagePressTimerWait * 2 ) ) / 
				      static_cast<double>( d->continuousPagePressTimerWait );
		// Let's make sure if the acceleration coefficient is between 1 and 3
		acceleration = QMAX( 1, acceleration );
		acceleration = QMIN( 3, acceleration );

		if( d->scrollBarPrevPagePressed )
		{
			if( d->smoothScrollContinuousCounter++ > d->continuousPagePressTimerWait ) // pressed long enough ?
			{
				d->targetScrollBarValue -= d->smoothScrollingPageStep + acceleration; // if so start continuous scrolling
				d->targetScrollBarValue = QMAX( d->targetScrollBarValue, verticalScrollBar()->minValue() );
			}
		}
		else if( d->scrollBarNextPagePressed )
		{
			if( d->smoothScrollContinuousCounter++ > d->continuousPagePressTimerWait ) // pressed long enough ?
			{
				d->targetScrollBarValue += d->smoothScrollingPageStep * acceleration; // if so start continuous scrolling
				d->targetScrollBarValue = QMIN( d->targetScrollBarValue, verticalScrollBar()->maxValue() );
			}
		}
	}
	else if( e->timerId() == d->scrollAutoHideTimer )
	{
		if( !d->scrollAutoHideCounter-- )
			setVScrollBarMode( AlwaysOff );
	}
}

bool ListView::eventFilter( QObject *o, QEvent *e )
{
	if( o == verticalScrollBar() ) // Event's to scroll bar
	{
		// Our scroll bar
		QScrollBar *bar = static_cast<QScrollBar*>(o);
		if( e->type() == QEvent::Wheel )
		{
			// OK, this is a wheel event, let's get our QWheelEvent in an unsafe way, due to a bug in RTTI of QT.
			QWheelEvent *event = static_cast<QWheelEvent*>(e);
			// Set new target value
			d->targetScrollBarValue -= event->delta();
			// Make sure it's in the boundaries of scroll bar
			d->targetScrollBarValue = QMAX( d->targetScrollBarValue, bar->minValue() );
			d->targetScrollBarValue = QMIN( d->targetScrollBarValue, bar->maxValue() );
			return true; // Ignore the event
		}
		else if( e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonDblClick )
		{
			// This is lame, and will be done with QWidget::style()::querySubControl with the next revision hopefully

			// We are intercepting all clicks and double clicks on the scrollbar. Unless we do so
			// scroll bar immediatly goes to the point wherever user's click requests it to.
			// Then smooth scroll begins, and animates the scrolling, but since the scrollbar
			// goes to the destionation point for a very small amount of time at the very beginning of
			// the click, this causes flickering. So we intercept each click, and make the scroll bar
			// go to it's destination by smoothly.

			//// Start masking the scrollbar so that we can detect where the mouse clicks on
			// The slider handle's starting position.
			int sliderStart = bar->sliderStart();
			// The slider handle's ending position
			int sliderEnd = sliderStart + bar->sliderRect().height();
			// The slider handle's width
			int width = bar->sliderRect().width();
			// This is masking the upper previous line button
			QRect prevLineUpper( 0, 0, width, 15 );
			// This is masking the previous page, which is between the upper previous line button and the slider
			QRect prevPage( 0, 15, width, sliderStart - 15 );
			// This is masking the next page, which is between bottom previous line and the slider
			QRect nextPage( 0, sliderEnd, width, bar->height() - sliderEnd - 30 );
			// This is masking the bottom previous line button
			QRect prevLineBottom( 0, bar->height() - 30, width, 15 );
			// This is masking the next line button
			QRect nextLine( 0, bar->height() - 15, width, 30 );

			// Get page/line step sizes. You may ask, why we are not doing this in setSmoothScrolling
			// the reason is, scroll bar might not be initialized at that moment. When we are receiving
			// MouseButtonPress or such event, we're sure that it's initialized!
			if( d->smoothScrollingLineStep == 0 && d->smoothScrollingPageStep == 0 ){
				d->smoothScrollingLineStep = bar->lineStep();
				d->smoothScrollingPageStep = bar->pageStep();
				// Set page/line steps of the scroll bar to zero, we'll emulate them, smoothly!
				// If we don't set this to 0, when we pass the event to the button, the scollbar
				// will scroll the list too.
				verticalScrollBar()->setLineStep( 0 );
			}

			// OK, now we can understand which partion of the scroll bar is clicked, and do the requested thing
			// animated. Then set the step sizes to zero, and pass the event to the slider, so that user can
			// feel like he/she really pressed the buttons (on click color change).

			// Get our QMouseEvent so that we can have our relative mouse position
			QMouseEvent *event = static_cast<QMouseEvent*>(e);			

			if( verticalScrollBar()->sliderRect().contains( event->pos() ) )// Click on the slider
			{
				d->scrollBarSliderDragStartY = event->y();
				d->scrollBarSliderPressed = true;
			}
			else if( prevLineUpper.contains( event->pos() ) )	// Click on the upper previous line button
			{
				d->targetScrollBarValue -= d->smoothScrollingLineStep;
				// Make sure if the targetScrollBarValue is in the scroll bar values range
				d->targetScrollBarValue = QMAX( d->targetScrollBarValue, verticalScrollBar()->minValue() );
				d->scrollBarPrevLineUpPressed = true;
				return false; // pass the event to the scroll bar so the button gets "clicked"
			}
			else if( prevPage.contains( event->pos() ) )		// Click on the previous page area
			{
				d->targetScrollBarValue -= d->smoothScrollingPageStep;
				// Make sure if the targetScrollBarValue is in the scroll bar values range
				d->targetScrollBarValue = QMAX( d->targetScrollBarValue, verticalScrollBar()->minValue() );
				d->scrollBarPrevPagePressed = true;
			}
			else if( nextPage.contains( event->pos() ) )		// Click on the next page area
			{
				d->targetScrollBarValue += d->smoothScrollingPageStep;
				// Make sure if the targetScrollBarValue is in the scroll bar values range
				d->targetScrollBarValue = QMIN( d->targetScrollBarValue, verticalScrollBar()->maxValue() );
				d->scrollBarNextPagePressed = true;
			}
			else if( prevLineBottom.contains( event->pos() ) )	// Click on the bottom previous line button
			{
				d->targetScrollBarValue -= d->smoothScrollingLineStep;
				// Make sure if the targetScrollBarValue is in the scroll bar values range
				d->targetScrollBarValue = QMAX( d->targetScrollBarValue, verticalScrollBar()->minValue() );
				d->scrollBarPrevLineBottomPressed = true;
				return false; // pass the event to the scroll bar so the button gets "clicked"
			}
			else if( nextLine.contains( event->pos() ) )		// Click on the next line button
			{
				d->targetScrollBarValue += d->smoothScrollingLineStep;
				// Make sure if the targetScrollBarValue is in the scroll bar values range
				d->targetScrollBarValue = QMIN( d->targetScrollBarValue, verticalScrollBar()->maxValue() );
				d->scrollBarNextLinePressed = true;
				return false; // pass the event to the scroll bar so the button gets "clicked"
			}
			return true; // Now, ignore the event.
		}
		else if( e->type() == QEvent::MouseMove )
		{
			// Get our QMouseEvent so that we can have our relative mouse position
			QMouseEvent *event = static_cast<QMouseEvent*>(e);
			if( d->scrollBarSliderPressed )
			{
				// Mouse movement distance for this MouseMove event
				double delta = event->y() - d->scrollBarSliderDragStartY;
				// Update the drag start value so in the next MouseMove event we can calculate new movement distance
				d->scrollBarSliderDragStartY = event->y();
				// The length which we can move the mouse over the bar
				double scale = bar->geometry().height() - bar->sliderRect().height() - 45;
				// Scale it to scroll bar value
				d->targetScrollBarValue += static_cast<int>( static_cast<double>( ( bar->maxValue() / scale ) * delta ) );
			}

			if( d->scrollAutoHide ) // If auto-hide scroll bar is enabled
			{
				d->scrollAutoHideCounter = 9999;		// Mouse is dragging the scrollbar slider
			}
		}
		else if( e->type() == QEvent::Enter )
		{
			if( d->scrollAutoHide ) // If auto-hide scroll bar is enabled
			{
				d->scrollAutoHideCounter = 9999;		// Mouse is on the scroll bar
			}
		}
		else if( e->type() == QEvent::Leave )
		{
			if( d->scrollAutoHide ) // If auto-hide scroll bar is enabled
			{		// show the scroll bar
				d->scrollAutoHideCounter = d->scrollAutoHideTimeout;		// Mouse is on the scroll bar
			}
		}
		else if( e->type() == QEvent::MouseButtonRelease )
		{
			// Reset waiting counter. This is used to wait before simulating continuous mouse press
			d->smoothScrollContinuousCounter = 0;
			// Mark all buttons as not pressed now
			d->scrollBarSliderPressed = d->scrollBarPrevLineUpPressed = d->scrollBarPrevPagePressed = false;
			d->scrollBarNextPagePressed = d->scrollBarPrevLineBottomPressed = d->scrollBarNextLinePressed = false;
			// Make sure if the targetScrollBarValue is in the scroll bar values range
			d->targetScrollBarValue = QMAX( d->targetScrollBarValue, bar->minValue() );
			d->targetScrollBarValue = QMIN( d->targetScrollBarValue, bar->maxValue() );
			return false; // Pass the release event to the scroll bar, which will put the buttons in off-state
		}
		else
		{
			return false; // Pass the event to the scroll bar
		}
	}
	else if( o == viewport() )
	{
		if( e->type() == QEvent::MouseButtonPress )
		{
			// Mark that we have pressed the button. This will prevent the list from scrolling when
			// the current item has changed due to mouse click. It's fine when the keypresses cause
			// it scroll, but not mouse.
			d->mousePressed = true;
		} 
		else if( e->type() == QEvent::MouseButtonRelease )
		{
			d->mousePressed = false;
		}
		else if( e->type() == QEvent::DragMove )
		{
			// OK, user is dragging something in the list
			QDragMoveEvent *event = static_cast<QDragMoveEvent*>(e);
			if( event->pos().y() < d->smoothAutoScrollOffset )
			{ // If he's too close to the upper edge, let's smootly scroll up
				d->targetScrollBarValue -= ( d->smoothAutoScrollOffset - event->pos().y() ) * d->scrollBarAccelerationConstant / 3;
			}
			else if( event->pos().y() > ( visibleHeight() - d->smoothAutoScrollOffset ) )
			{ // If he's too close to the bottom edege, then let's smoothle scroll down
				d->targetScrollBarValue += ( event->pos().y() - visibleHeight() + d->smoothAutoScrollOffset ) * d->scrollBarAccelerationConstant / 3;
			}
			// Make sure if the targetScrollBarValue is in the scroll bar values range
			d->targetScrollBarValue = QMAX( d->targetScrollBarValue, verticalScrollBar()->minValue() );
			d->targetScrollBarValue = QMIN( d->targetScrollBarValue, verticalScrollBar()->maxValue() );
		}
		else if( e->type() == QEvent::MouseMove ) // Activity detected ( used to aut-hide scroll bar )
		{
			// Get our QMouseEvent so that we can have our relative mouse position
			QMouseEvent *event = static_cast<QMouseEvent*>(e);

			if( d->scrollAutoHide ) // If auto-hide scroll bar is enabled
			{
				setVScrollBarMode( Auto );			// show the scroll bar
				d->scrollAutoHideCounter = 9999;		// Mouse is on the contact list, so don't hide it
			}

			if( d->mouseNavigation )
			{
				const double offset = static_cast<double>(visibleHeight())/50.0 + d->mouseNavigationOffset;
				d->targetScrollBarValue = ( event->y() - offset ) * ( static_cast<double>(verticalScrollBar()->maxValue()) /
										   ( static_cast<double>(visibleHeight()) - offset * 2 ) );
			}
		}
		else if( e->type() == QEvent::Leave )
		{
			if( d->scrollAutoHide ) // If auto-hide scroll bar is enabled
			{
				d->scrollAutoHideCounter = d->scrollAutoHideTimeout; // Mouse left the contact list, hide it after timeout
			}
		}

		return KListView::eventFilter( o, e ); // Pass the event to KListView
	}
	else
	{
// 		kdDebug( 14000 ) << k_funcinfo << "Unhandled event: [" << o << "][" << o->name() << "][" << o->className() << "][" << e->type() << "]" << endl;
		return KListView::eventFilter( o, e ); // Pass the event to KListView
	}
}

void ListView::slotCurrentChanged( QListViewItem *item )
{
	if( !item ) return;
	// If the current item changed due to mouse click then don't center it in the listview. Do this just for key presses.
	if( d->mousePressed ){ d->mousePressed = false; return; }
	d->targetScrollBarValue = itemPos(item) - static_cast<double>(visibleHeight()/2.0) + item->height();
	// Make sure it's in the boundaries of scroll bar
	d->targetScrollBarValue = QMAX( d->targetScrollBarValue, verticalScrollBar()->minValue() );
	d->targetScrollBarValue = QMIN( d->targetScrollBarValue, verticalScrollBar()->maxValue() );
}


} // END namespace ListView
} // END namespace UI
} // END namespace Kopete

#include "kopetelistview.moc"

// vim: set noet ts=4 sts=4 sw=4:
