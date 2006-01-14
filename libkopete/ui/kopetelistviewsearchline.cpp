/*
    kopetelistviewsearchline.cpp - a widget for performing quick searches on Kopete::ListViews
    Based on code from KMail, copyright (c) 2004 Till Adam <adam@kde.org>

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

#include "kopetelistviewsearchline.h"
#include "kopetelistviewitem.h"
#include "kopetelistview.h"

namespace Kopete {
namespace UI {
namespace ListView {

SearchLine::SearchLine( QWidget *parent, ListView *listView, const char *name )
	: KListViewSearchLine( parent, listView, name )
{
}

SearchLine::SearchLine(QWidget *parent, const char *name)
	: KListViewSearchLine( parent, 0, name )
{
}

SearchLine::~SearchLine()
{
}


void SearchLine::updateSearch( const QString &s )
{
	// we copy a huge chunk of code here simply in order to override
	// the way items are shown/hidden. KSearchLine rudely
	// calls setVisible() on items with no way to customise this behaviour.
	
	//BEGIN code from KSearchLine::updateSearch
		if( !listView() )
		return;
	
	search = s.isNull() ? text() : s;
	
	// If there's a selected item that is visible, make sure that it's visible
	// when the search changes too (assuming that it still matches).
	
	QListViewItem *currentItem = 0;
	
	switch( listView()->selectionMode() )
	{
	case KListView::NoSelection:
		break;
	case KListView::Single:
		currentItem = listView()->selectedItem();
		break;
	default:
		for( QListViewItemIterator it(listView(), QListViewItemIterator::Selected | QListViewItemIterator::Visible);
		     it.current() && !currentItem; ++it )
		{
			if( listView()->itemRect( it.current() ).isValid() )
				currentItem = it.current();
		}
	}
	
	if( keepParentsVisible() )
		checkItemParentsVisible( listView()->firstChild() );
	else
		checkItemParentsNotVisible();
	
	if( currentItem )
		listView()->ensureItemVisible( currentItem );
	//END code from KSearchLine::updateSearch
}

void SearchLine::checkItemParentsNotVisible()
{
	//BEGIN code from KSearchLine::checkItemParentsNotVisible
	QListViewItemIterator it( listView() );
	for( ; it.current(); ++it )
	{
		QListViewItem *item = it.current();
		if( itemMatches( item, search ) )
			setItemVisible( item, true );
		else
			setItemVisible( item, false );
	}
	//END code from KSearchLine::checkItemParentsNotVisible
}

bool SearchLine::checkItemParentsVisible( QListViewItem *item )
{
	//BEGIN code from KSearchLine::checkItemParentsVisible
	bool visible = false;
	for( ; item; item = item->nextSibling() ) {
		if( ( item->firstChild() && checkItemParentsVisible( item->firstChild() ) ) ||
		    itemMatches( item, search ) )
		{
			setItemVisible( item, true );
			// OUCH! this operation just became exponential-time.
			// however, setting an item visible sets all its descendents
			// visible too, which we definitely don't want.
			// plus, in Kopete the nesting is never more than 2 deep,
			// so this really just doubles the runtime, if that.
			// this still can be done in O(n) time by a mark-set process,
			// but that's overkill in our case.
			checkItemParentsVisible( item->firstChild() );
			visible = true;
		}
		else
			setItemVisible( item, false );
	}
	return visible;
	//END code from KSearchLine::checkItemParentsVisible
}

void SearchLine::setItemVisible( QListViewItem *it, bool b )
{
	if( Item *item = dynamic_cast<Item*>( it ) )
		item->setSearchMatch( b );
	else
		it->setVisible( b );
}

} // namespace ListView
} // namespace UI
} // namespace Kopete

#include "kopetelistviewsearchline.moc"
