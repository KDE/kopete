/*
    kopeteonlinestatus.cpp - Kopete Online Status

    Copyright (c) 2002-2003 by Olivier Goffart       <ogoffart@tiscalinet.be>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

#include "kopetecontactlistview.h"
#include "kopetegroupviewitem.h"
#include "kopetegroup.h"
#include "kopeteprefs.h"
#include "kopetemetacontactlvi.h"
#include "kopetemetacontact.h"

KopeteGroupViewItem::KopeteGroupViewItem( KopeteGroup *group_, QListView *parent, const char *name )
: QObject( group_ ), QListViewItem( parent, name )
{
	setVisible( false );
	m_group = group_;
	refreshDisplayName();
	connect( m_group, SIGNAL( renamed( KopeteGroup*, const QString& ) ),
		this, SLOT( refreshDisplayName() ) );
	connect( KopetePrefs::prefs(), SIGNAL( saved() ),
		SLOT( updateVisibility() ) );
}

KopeteGroupViewItem::KopeteGroupViewItem(KopeteGroup *group_,
	QListViewItem *parent, const char *name )
		: QObject(group_), QListViewItem(parent,name)
{
	setVisible( false );
	m_group = group_;
	refreshDisplayName();
	connect( m_group, SIGNAL( renamed( KopeteGroup*, const QString& ) ),
		this, SLOT( refreshDisplayName() ) );
	connect( KopetePrefs::prefs(), SIGNAL( saved() ),
		SLOT( updateVisibility() ) );
}

KopeteGroupViewItem::~KopeteGroupViewItem()
{
}

KopeteGroup* KopeteGroupViewItem::group() const
{
	return m_group;
}

void KopeteGroupViewItem::refreshDisplayName()
{
	//if ( !m_group )
	//	return;

	QString newText;
	// FIXME: I think handling the i18n for temporary and top level
	//        groups belongs in KopeteGroup instead.
	//        It's now duplicated in KopeteGroupListAction and
	//        KopeteGroupViewItem already - Martijn
	switch ( m_group->type() )
	{
	case KopeteGroup::Temporary:
		newText = i18n( "Not in your contact list" );
		break;
	case KopeteGroup::TopLevel:
		newText = i18n( "Top-Level" );
		break;
	default:
		newText = m_group->displayName();
		break;
	}

	totalMemberCount = 0;
	onlineMemberCount = 0;

	for ( QListViewItem *lvi = firstChild(); lvi; lvi = lvi->nextSibling() )
	{
		KopeteMetaContactLVI *kc = dynamic_cast<KopeteMetaContactLVI*>( lvi );
		if ( kc )
		{
			totalMemberCount++;
			if ( kc->metaContact()->isOnline() )
				onlineMemberCount++;
		}
	}

	m_renameText = newText;
	newText += " (" + QString::number( onlineMemberCount ) + "/" + QString::number( totalMemberCount ) + ")";
	//kdDebug( 14000 ) << k_funcinfo << "newText='" << newText << "', old text= " << text( 0 ) << endl;

	setText( 0, newText );
	updateVisibility();

	// Sorting in this slot is extremely expensive as it's called dozens of times and
	// the sorting itself is rather slow. Therefore we call delayedSort, which tries
	// to group multiple sort requests into one.
	KopeteContactListView *lv = dynamic_cast<KopeteContactListView *>( listView() );
	if ( lv )
		lv->delayedSort();
	else
		listView()->sort();
}

QString KopeteGroupViewItem::key( int, bool ) const
{
	//Groups are placed after topLevel contact.
	//Exepted Temporary group which is the first group
	if ( group()->type() != KopeteGroup::Normal )
		return "0" + text( 0 );
	return "M" + text( 0 );
}

void KopeteGroupViewItem::startRename( int col )
{
	kdDebug(14000) << k_funcinfo << endl;
	if ( col != 0 ) return;
	refreshDisplayName();
	setText( 0, m_renameText );
	setRenameEnabled( 0, true );
	QListViewItem::startRename( 0 );
}

void KopeteGroupViewItem::okRename( int col )
{
	kdDebug(14000) << k_funcinfo << endl;
	QListViewItem::okRename(col);
	if ( col == 0 )
		group()->setDisplayName(text(0));
	refreshDisplayName();
}

void KopeteGroupViewItem::cancelRename( int col )
{
	kdDebug(14000) << k_funcinfo << endl;
	QListViewItem::cancelRename(col);
	refreshDisplayName();
}

void KopeteGroupViewItem::updateVisibility()
{
	int visibleUsers = onlineMemberCount;
	if ( KopetePrefs::prefs()->showOffline() )
		visibleUsers = totalMemberCount;

	bool visible = KopetePrefs::prefs()->showEmptyGroups() || ( visibleUsers > 0 );

	if ( isVisible() != visible )
	{
		setVisible( visible );
		if ( visible )
		{
			// When calling setVisible(true) EVERY child item will be shown,
			// even if they should be hidden.
			// We just re-update the visibility of all child items
			QListViewItem *lvi;
			for ( lvi = firstChild(); lvi; lvi = lvi->nextSibling() )
			{
				KopeteMetaContactLVI *kmc = dynamic_cast<KopeteMetaContactLVI *>( lvi );
				if ( kmc )
					kmc->updateVisibility();
			}
		}
	}
}

void KopeteGroupViewItem::updateCustomIcons(bool treeView)
{
	//kdDebug( 14000 ) << k_funcinfo << "treeView=" << treeView << endl;

	// TODO: clever caching
	if ( !group()->useCustomIcon() )
		return;

	if ( treeView )
	{
		if ( isOpen() )
		{
			open = SmallIcon( group()->icon( KopetePluginDataObject::Open ) );
			setPixmap( 0, open );
			/*kdDebug(14000) << k_funcinfo << "setting open icon name='" <<
				group()->icon( KopetePluginDataObject::Open ) << "'" << endl;*/
		}
		else
		{
			closed = SmallIcon( group()->icon( KopetePluginDataObject::Closed ) );
			setPixmap( 0, closed );
			/*kdDebug(14000) << k_funcinfo << "setting closed icon name='" <<
				group()->icon( KopetePluginDataObject::Closed ) << "'" << endl;*/
		}
	}
	else // classic view
	{
		open = SmallIcon( group()->icon( KopetePluginDataObject::Open ) );
		setPixmap( 0, open );
	}
}

#include "kopetegroupviewitem.moc"

// vim: set noet ts=4 sts=4 sw=4:

