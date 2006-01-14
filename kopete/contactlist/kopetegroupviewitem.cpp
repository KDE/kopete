/*
    kopeteonlinestatus.cpp - Kopete Online Status

    Copyright (c) 2002-2004 by Olivier Goffart       <ogoffart @ kde.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

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
#include <kapplication.h>

#include "kopetecontactlistview.h"
#include "kopetegroupviewitem.h"
#include "kopetegroup.h"
#include "kopeteonlinestatus.h"
#include "kopeteprefs.h"
#include "kopetemetacontactlvi.h"
#include "kopetemetacontact.h"

#include <memory>

//using namespace Kopete::UI;

class KopeteGroupViewItem::Private
{
public:
	Kopete::UI::ListView::ImageComponent *image;
	Kopete::UI::ListView::DisplayNameComponent *name;
	Kopete::UI::ListView::TextComponent *count;
	std::auto_ptr<Kopete::UI::ListView::ToolTipSource> toolTipSource;
};

namespace Kopete {
namespace UI {
namespace ListView {

class GroupToolTipSource : public ToolTipSource
{
public:
	GroupToolTipSource( KopeteGroupViewItem *gp )
	 : group( gp )
	{
	}
	QString operator()( ComponentBase *, const QPoint &, QRect & )
	{
		return group->toolTip();
	}
private:
	KopeteGroupViewItem *group;
};

} // END namespace ListView
} // END namespace UI
} // END namespace Kopete

KopeteGroupViewItem::KopeteGroupViewItem( Kopete::Group *group_, QListView *parent, const char *name )
: Kopete::UI::ListView::Item( parent, group_, name )
{
	m_group = group_;
	initLVI();
}

KopeteGroupViewItem::KopeteGroupViewItem( Kopete::Group *group_, QListViewItem *parent, const char *name )
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

	d->toolTipSource.reset( new Kopete::UI::ListView::GroupToolTipSource( this ) );

	using namespace Kopete::UI::ListView;
	Component *hbox = new BoxComponent( this, BoxComponent::Horizontal );
	d->image = new ImageComponent( hbox );
	d->name = new DisplayNameComponent( hbox );
	d->count = new TextComponent( hbox );

	d->image->setToolTipSource( d->toolTipSource.get() );
	d->name->setToolTipSource( d->toolTipSource.get() );
	d->count->setToolTipSource( d->toolTipSource.get() );

	connect( m_group, SIGNAL( displayNameChanged( Kopete::Group*, const QString& ) ),
		this, SLOT( refreshDisplayName() ) );

	connect( KopetePrefs::prefs(), SIGNAL( contactListAppearanceChanged() ),
		SLOT( slotConfigChanged() ) );
	connect( kapp, SIGNAL( appearanceChanged() ),  SLOT( slotConfigChanged() ) );

	connect( m_group, SIGNAL( iconAppearanceChanged() ), SLOT( updateIcon() ) );

	refreshDisplayName();
	slotConfigChanged();
}

Kopete::Group* KopeteGroupViewItem::group() const
{
	return m_group;
}

QString KopeteGroupViewItem::toolTip() const
{
	// TODO: add icon, and some more information than that which
	// is already displayed in the list view item
	// FIXME: post-KDE-3.3, make this better i18n-able
	//  currently it can't cause more problems than the contact list itself, at least.
	return "<b>" + d->name->text() + "</b> " + d->count->text();
}

void KopeteGroupViewItem::slotConfigChanged()
{
	updateIcon();
	updateVisibility();

	d->name->setColor( KopetePrefs::prefs()->contactListGroupNameColor() );

	QFont font = listView()->font();
	if ( KopetePrefs::prefs()->contactListUseCustomFonts() )
		font = KopetePrefs::prefs()->contactListCustomNormalFont();
	d->name->setFont( font );

	d->count->setFont( KopetePrefs::prefs()->contactListSmallFont() );
}

void KopeteGroupViewItem::refreshDisplayName()
{
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

	d->name->setText( m_group->displayName() );
	d->count->setText( i18n( "(NUMBER OF ONLINE CONTACTS/NUMBER OF CONTACTS IN GROUP)", "(%1/%2)" )
	                  .arg( QString::number( onlineMemberCount ), QString::number( totalMemberCount ) ) );

	updateVisibility();

	// Sorting in this slot is extremely expensive as it's called dozens of times and
	// the sorting itself is rather slow. Therefore we call delayedSort, which tries
	// to group multiple sort requests into one.
	using namespace Kopete::UI::ListView;
	if ( ListView::ListView *lv = dynamic_cast<ListView::ListView *>( listView() ) )
		lv->delayedSort();
	else
		listView()->sort();
}

QString KopeteGroupViewItem::key( int, bool ) const
{
	//Groups are placed after topLevel contact.
	//Exepted Temporary group which is the first group
	if ( group()->type() != Kopete::Group::Normal )
		return "0" + d->name->text();
	return "M" + d->name->text();
}

void KopeteGroupViewItem::startRename( int /*col*/ )
{
	//kdDebug(14000) << k_funcinfo << endl;
	KListViewItem::startRename( 0 );
}

void KopeteGroupViewItem::okRename( int col )
{
	//kdDebug(14000) << k_funcinfo << endl;
	KListViewItem::okRename(col);
	setRenameEnabled( 0, false );
}

void KopeteGroupViewItem::cancelRename( int col )
{
	//kdDebug(14000) << k_funcinfo << endl;
	KListViewItem::cancelRename(col);
	setRenameEnabled( 0, false );
}

void KopeteGroupViewItem::updateVisibility()
{
	//FIXME: A contact can ve visible if he has a unknwon status (it's not online)
	//       or if he has an event (blinking icon).  If such as contact is not with
	//       others inline contact in the group. the group will stay hidden.
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
	// TODO: clever caching
	if ( isOpen() )
	{
		if ( group()->useCustomIcon() && !group()->icon( Kopete::ContactListElement::Open ).isEmpty() )
			open = SmallIcon( group()->icon( Kopete::ContactListElement::Open ) );
		else
			open = SmallIcon( KOPETE_GROUP_DEFAULT_OPEN_ICON );

		d->image->setPixmap( open );
	}
	else
	{
		if ( group()->useCustomIcon() && !group()->icon( Kopete::ContactListElement::Closed ).isEmpty() )
			closed = SmallIcon( group()->icon( Kopete::ContactListElement::Closed ) );
		else
			closed = SmallIcon( KOPETE_GROUP_DEFAULT_CLOSED_ICON );

		d->image->setPixmap( closed );
	}
}

QString KopeteGroupViewItem::text( int column ) const
{
	if ( column == 0 )
		return d->name->text();
	else
		return KListViewItem::text( column );
}

void KopeteGroupViewItem::setText( int column, const QString &text )
{
	if ( column == 0 )
	{
		KopeteContactListView *lv = dynamic_cast<KopeteContactListView *>( listView() );
		if ( lv )
		{
			KopeteContactListView::UndoItem *u=new KopeteContactListView::UndoItem(KopeteContactListView::UndoItem::GroupRename, 0L, m_group);
			u->args << m_group->displayName();
			lv->insertUndoItem(u);
		}
		group()->setDisplayName( text );
	}
	else
		KListViewItem::setText( column, text );
}

#include "kopetegroupviewitem.moc"
// vim: set noet ts=4 sts=4 sw=4:


