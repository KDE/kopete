/***************************************************************************
                          kopetegroupviewitem.cpp  -  description
                             -------------------
    begin                : lun oct 28 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart@tiscalinet.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klocale.h>
#include <kdebug.h>

#include "kopetegroupviewitem.h"
#include "kopetegroup.h"
#include "kopeteprefs.h"
#include "kopetemetacontactlvi.h"
#include "kopetemetacontact.h"

KopeteGroupViewItem::KopeteGroupViewItem(KopeteGroup *group_, QListView *parent, const char *name )
		: QObject(group_), QListViewItem(parent,name)
{
	setVisible(false);
	m_group=group_;
	refreshDisplayName();
	connect( m_group, SIGNAL( renamed( KopeteGroup*, const QString& ) ),
		this, SLOT( refreshDisplayName() ) );
	connect( KopetePrefs::prefs(), SIGNAL( saved() ),
		SLOT( updateVisibility() ) );
}

KopeteGroupViewItem::KopeteGroupViewItem(KopeteGroup *group_, QListViewItem *parent, const char *name )
		: QObject(group_), QListViewItem(parent,name)
{
	setVisible(false);
	m_group=group_;
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
//	if(!m_group) return;
	QString newText = m_group->displayName();

	if( m_group == KopeteGroup::temporary )
		newText = i18n( "Not in your contact list" );

	totalMemberCount=0;
	onlineMemberCount=0;

	for(QListViewItem *lvi = firstChild() ; lvi; lvi = lvi->nextSibling() )
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
	newText += "  ("+QString::number(onlineMemberCount)+"/"+QString::number(totalMemberCount)+")";

//	kdDebug(14000) << k_funcinfo << "newText='" << newText <<
//		"', old text= " << text(0) << endl;

	setText( 0, newText );
	updateVisibility();
	listView()->sort();
}

QString KopeteGroupViewItem::key( int, bool ) const
{
	//Groups are placed after topLevel contact.
	//Temporary group is the first group
	if(group()->type()!=KopeteGroup::Classic)
		return "L"+text(0);
	return "M"+text(0);
}

void KopeteGroupViewItem::startRename( int col )
{
	kdDebug(14000) << k_funcinfo << endl;
	refreshDisplayName();
	setText( 0, m_renameText );
	QListViewItem::startRename(col);
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
	if( KopetePrefs::prefs()->showOffline() )
		visibleUsers = totalMemberCount;

	if( KopetePrefs::prefs()->showEmptyGroups() )
		setVisible(true);
	else if( visibleUsers > 0 )
		setVisible(true);
	else
		setVisible(false);

	for(QListViewItem *lvi = firstChild() ; lvi; lvi = lvi->nextSibling() )
	{
		KopeteMetaContactLVI *kc = dynamic_cast<KopeteMetaContactLVI*>( lvi );
		if ( kc )
		{
			if ( kc->metaContact()->isOnline() )
				kc->setVisible(true);
			else if ( KopetePrefs::prefs()->showOffline() )
				kc->setVisible(true);
			else
				kc->setVisible(false);
		}
	}

}
#include "kopetegroupviewitem.moc"
