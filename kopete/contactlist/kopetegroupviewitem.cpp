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
#include "kopetemetacontactlvi.h"

KopeteGroupViewItem::KopeteGroupViewItem(KopeteGroup *group_ , QListView *parent, const char *name )
		: QObject(group_) , QListViewItem(parent,name)
{
	m_group=group_;
	refreshDisplayName();
	connect( m_group , SIGNAL ( renamed(KopeteGroup*, const QString&)) , this, SLOT(refreshDisplayName()));
}

KopeteGroupViewItem::KopeteGroupViewItem(KopeteGroup *group_ , QListViewItem *parent, const char *name )
		: QObject(group_) , QListViewItem(parent,name)
{
	m_group=group_;
	refreshDisplayName();
	connect( m_group , SIGNAL ( renamed(KopeteGroup*, const QString&)) , this, SLOT(refreshDisplayName()));
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
	QString text=m_group->displayName();
	if(m_group== KopeteGroup::temporary)
		text=i18n( "Not in your contact list" );

	unsigned int tot=0;
	unsigned int onl=0;
	
	for(QListViewItem *lvi = firstChild() ; lvi; lvi = lvi->nextSibling() )
	{
		KopeteMetaContactLVI *kc = dynamic_cast<KopeteMetaContactLVI*>( lvi );
		if ( kc )
		{
			tot++;
			if ( kc->metaContact()->isOnline() )
				onl++;
		}
	}

	m_renameText = text;
//	text= QStyleSheet::escape(text)+ " <font color='gray' size='-1'><i>("+QString::number(onl)+"/"+QString::number(tot)+")</i></font>";
	text= text +"  ("+QString::number(onl)+"/"+QString::number(tot)+")";

//	kdDebug(14000) << "KopeteGroupViewItem::refreshDisplayName : " << text << endl;

	setText( 0, text );
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
	refreshDisplayName();
	QListViewItem::startRename(col);
}

void KopeteGroupViewItem::okRename( int col )
{
	QListViewItem::okRename(col);
	if ( col == 0 )
		group()->setDisplayName(text(0));
	refreshDisplayName();
}

void KopeteGroupViewItem::cancelRename( int col )
{
	QListViewItem::cancelRename(col);
	refreshDisplayName();
}



#include "kopetegroupviewitem.moc"

