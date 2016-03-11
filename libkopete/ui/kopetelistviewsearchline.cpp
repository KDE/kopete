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

SearchLine::SearchLine( QWidget *parent, ListView *listView )
	: KTreeWidgetSearchLine( parent, listView ), searchEmpty(true)
{
}

SearchLine::~SearchLine()
{
}


void SearchLine::updateSearch( const QString &s )
{
	// we copy a huge chunk of code here simply in order to override
	// the way items are shown/hidden. KSearchLine rudely
	// calls setVisible() on items with no way to customize this behaviour.
	
	//BEGIN code from KSearchLine::updateSearch
		if( !( this->treeWidget() ) )
		return;
	
	search = s.isNull() ? text() : s;
	searchEmpty = search.isEmpty();

	// If there's a selected item that is visible, make sure that it's visible
	// when the search changes too (assuming that it still matches).
	
	QTreeWidgetItem *currentItem = 0;
	
	switch( this->treeWidget()->selectionMode() )
	{
	case QTreeWidget::NoSelection:
		break;
	case QTreeWidget::SingleSelection:
		currentItem = this->treeWidget()->currentItem();
		break;
	default:
		for( QTreeWidgetItemIterator it(this->treeWidget(), QTreeWidgetItemIterator::Selected | QTreeWidgetItemIterator::NotHidden);
			 (*it) && !currentItem; ++it )
		{
			if( this->treeWidget()->visualItemRect( (*it) ).isValid() )
				currentItem = (*it);
		}
	}
	
	if( keepParentsVisible() )
		checkItemParentsVisible( this->treeWidget()->topLevelItem(0) );
	else
		checkItemParentsNotVisible();
	
	if( currentItem )
		this->treeWidget()->scrollToItem( currentItem );
	//END code from KSearchLine::updateSearch
}

void SearchLine::checkItemParentsNotVisible()
{
	//BEGIN code from KSearchLine::checkItemParentsNotVisible
	QTreeWidgetItemIterator it( this->treeWidget() );
	for( ; (*it); ++it )
	{
		QTreeWidgetItem *item = (*it);
		if( itemMatches( item, search ) )
			this->setItemVisible( item, true );
		else
			this->setItemVisible( item, false );
	}
	//END code from KSearchLine::checkItemParentsNotVisible
}

bool SearchLine::checkItemParentsVisible( QTreeWidgetItem *item )
{
	//BEGIN code from KSearchLine::checkItemParentsVisible
	bool visible = false;
	QTreeWidgetItemIterator it(item);
	for( ; (*it); ++it ) {
		if( ( (*it)->child(0) && checkItemParentsVisible( (*it)->child(0) ) ) ||
		    itemMatches( item, search ) )
		{
			(*it)->setHidden( false );
			// OUCH! this operation just became exponential-time.
			// however, setting an item visible sets all its descendents
			// visible too, which we definitely don't want.
			// plus, in Kopete the nesting is never more than 2 deep,
			// so this really just doubles the runtime, if that.
			// this still can be done in O(n) time by a mark-set process,
			// but that's overkill in our case.
			checkItemParentsVisible( (*it)->child(0) );
			visible = true;
		}
		else
			(*it)->setHidden( true );
	}
	return visible;
	//END code from KSearchLine::checkItemParentsVisible
}

void SearchLine::setItemVisible( QTreeWidgetItem *it, bool b )
{
	if( Item *item = dynamic_cast<Item*>( it ) )
		item->setSearchMatch( b, !searchEmpty );
	else
		it->setHidden( !b );
}

} // namespace ListView
} // namespace UI
} // namespace Kopete

#include "kopetelistviewsearchline.moc"
