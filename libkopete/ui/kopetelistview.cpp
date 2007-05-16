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
#include <kglobal.h>
#include <kconfig.h>
#include <kdebug.h>

#include <qtimer.h>
#include <qtooltip.h>
#include <qstyle.h>

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
	QTimer sortTimer;
	std::auto_ptr<ToolTip> toolTip;
	//! C-tor
	Private() {}
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
	QListViewItem *item, const QPoint &/*point*/ )
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

} // END namespace ListView
} // END namespace UI
} // END namespace Kopete

#include "kopetelistview.moc"

// vim: set noet ts=4 sts=4 sw=4:
