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

#include <qpainter.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

#include "kopetecontactlistview.h"
#include "kopetegroupviewitem.h"
#include "kopetegroup.h"
#include "kopeteonlinestatus.h"
#include "kopeteprefs.h"
#include "kopetemetacontactlvi.h"
#include "kopetemetacontact.h"

using namespace Kopete::UI;

class KopeteGroupViewItem::Private
{
public:
	ListView::ImageComponent *image;
	ListView::TextComponent *name;
	ListView::TextComponent *count;
};

KopeteGroupViewItem::KopeteGroupViewItem( KopeteGroup *group_, QListView *parent, const char *name )
: Kopete::UI::ListView::Item( parent, group_, name )
{
	m_group = group_;
	initLVI();
}

KopeteGroupViewItem::KopeteGroupViewItem( KopeteGroup *group_, QListViewItem *parent, const char *name )
 : Kopete::UI::ListView::Item( parent, group_, name )
{
	m_group = group_;
	initLVI();
}

KopeteGroupViewItem::~KopeteGroupViewItem()
{
	delete d;
}

void KopeteGroupViewItem::initLVI()
{
	d = new Private;

	using namespace ListView;
	Component *hbox = new BoxComponent( this, BoxComponent::Horizontal );
	d->image = new ImageComponent( hbox );

	QFont font = listView()->font();
	font.setBold( true );
	d->name = new TextComponent( hbox, font );
	d->name->setColor( darkRed );
	d->name->setFixedWidth( true );

	// FIXME: this is a hack. get a smaller font a nicer way
	QFont smallFont = listView()->font();
	if ( smallFont.pixelSize() != -1 )
		smallFont.setPixelSize( (font.pixelSize() * 3) / 4 );
	else
		smallFont.setPointSizeFloat( font.pointSizeFloat() * 0.75 );
	d->count = new TextComponent( hbox, smallFont );

	refreshDisplayName();
	connect( m_group, SIGNAL( renamed( KopeteGroup*, const QString& ) ),
		this, SLOT( refreshDisplayName() ) );
	connect( KopetePrefs::prefs(), SIGNAL( saved() ),
		SLOT( updateVisibility() ) );
	connect( m_group, SIGNAL( iconAppearanceChanged() ), SLOT( updateIcon() ) );
}

KopeteGroup* KopeteGroupViewItem::group() const
{
	return m_group;
}

void KopeteGroupViewItem::refreshDisplayName()
{
	QString groupName;
	// FIXME: I think handling the i18n for temporary and top level
	//        groups belongs in KopeteGroup instead.
	//        It's now duplicated in KopeteGroupListAction and
	//        KopeteGroupViewItem already - Martijn
	switch ( m_group->type() )
	{
		case KopeteGroup::Temporary:
			groupName = i18n( "Not in your contact list" );
			break;
		case KopeteGroup::TopLevel:
			groupName = i18n( "Top-Level" );
			break;
		default:
			groupName = m_group->displayName();
			break;
	}

	totalMemberCount = 0;
	onlineMemberCount = 0;

	for ( QListViewItem *lvi = firstChild(); lvi; lvi = lvi->nextSibling() )
	{
		if ( KopeteMetaContactLVI *kc = dynamic_cast<KopeteMetaContactLVI*>( lvi ) )
		{
			totalMemberCount++;
			if ( kc->metaContact()->isOnline() )
				onlineMemberCount++;
		}
	}

	m_renameText = groupName;

	d->name->setText( groupName );
	d->count->setText( i18n( "(NUMBER OF ONLINE CONTACTS/NUMBER OF CONTACTS IN GROUP)", "(%1/%2)" )
	                  .arg( QString::number( onlineMemberCount ), QString::number( totalMemberCount ) ) );

	updateVisibility();

	// Sorting in this slot is extremely expensive as it's called dozens of times and
	// the sorting itself is rather slow. Therefore we call delayedSort, which tries
	// to group multiple sort requests into one.
	if ( KopeteContactListView *lv = dynamic_cast<KopeteContactListView *>( listView() ) )
		lv->delayedSort();
	else
		listView()->sort();
}

QString KopeteGroupViewItem::key( int, bool ) const
{
	//Groups are placed after topLevel contact.
	//Exepted Temporary group which is the first group
	if ( group()->type() != KopeteGroup::Normal )
		return "0" + d->name->text();
	return "M" + d->name->text();
}

void KopeteGroupViewItem::startRename( int col )
{
	kdDebug(14000) << k_funcinfo << endl;
	if ( col != 0 ) return;
	refreshDisplayName();
	d->name->setText( m_renameText );
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
		setTargetVisibility( visible );
		if ( visible )
		{
			// When calling setVisible(true) EVERY child item will be shown,
			// even if they should be hidden.
			// We just re-update the visibility of all child items
			for ( QListViewItem *lvi = firstChild(); lvi; lvi = lvi->nextSibling() )
			{
				if ( KopeteMetaContactLVI *kmc = dynamic_cast<KopeteMetaContactLVI *>( lvi ) )
					kmc->updateVisibility();
			}
		}
	}
}

void KopeteGroupViewItem::updateIcon()
{
	bool treeView = true;
	if ( KopeteContactListView *lv = dynamic_cast<KopeteContactListView*>( listView() ) )
		treeView = lv->showAsTree();
	//kdDebug(14000) << k_funcinfo << "treeView=" << treeView << endl;

	// TODO: clever caching
	if ( treeView )
	{
		if ( isOpen() )
		{
			if ( group()->useCustomIcon() )
				open = SmallIcon( group()->icon( KopetePluginDataObject::Open ) );
			else
				open = SmallIcon( "folder_green_open" );

			d->image->setPixmap( open );
		}
		else
		{
			if ( group()->useCustomIcon() )
				closed = SmallIcon( group()->icon( KopetePluginDataObject::Closed ) );
			else
				closed = SmallIcon( "folder_green" );

			d->image->setPixmap( closed );
		}
	}
	else // classic view
	{
		if ( group()->useCustomIcon() )
			open = SmallIcon( group()->icon( KopetePluginDataObject::Open ) );
		else
			open = SmallIcon( "folder_blue" );

		d->image->setPixmap( open );
	}
}

#include "kopetegroupviewitem.moc"
// vim: set noet ts=4 sts=4 sw=4:
