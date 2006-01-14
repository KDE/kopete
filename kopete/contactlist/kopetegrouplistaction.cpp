/*
    kopetegrouplistcation.cpp   -  the action used for Move To and copy To

    Copyright (c) 2002-2004 by Olivier Goffart       <ogoffart @ kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2001-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
/*  This code was previously in libkopete/ui/kopetestdactions.cpp  */

#include "kopetegrouplistaction.h"

#include <kdebug.h>
#include <kguiitem.h>
#include <klocale.h>
#include <kaction.h>
#include <kwin.h>
#include <kcmultidialog.h>

#include "kopetecontactlist.h"
#include "kopetegroup.h"

KopeteGroupListAction::KopeteGroupListAction( const QString &text, const QString &pix, const KShortcut &cut, const QObject *receiver,
	const char *slot, QObject *parent, const char *name )
: KListAction( text, pix, cut, parent, name )
{
	connect( this, SIGNAL( activated() ), receiver, slot );

	connect( Kopete::ContactList::self(), SIGNAL( groupAdded( Kopete::Group * ) ), this, SLOT( slotUpdateList() ) );
	connect( Kopete::ContactList::self(), SIGNAL( groupRemoved( Kopete::Group * ) ), this, SLOT( slotUpdateList() ) );
	connect( Kopete::ContactList::self(), SIGNAL( groupRenamed(Kopete::Group*, const QString& ) ), this, SLOT( slotUpdateList() ) );
	slotUpdateList();
}

KopeteGroupListAction::~KopeteGroupListAction()
{
}

void KopeteGroupListAction::slotUpdateList()
{
	QStringList groupList;

	// Add groups to our list
	QPtrList<Kopete::Group> groups = Kopete::ContactList::self()->groups();
	for ( Kopete::Group *it = groups.first(); it; it = groups.next() )
	{
		if(it->type() == Kopete::Group::Normal)
			groupList.append( it->displayName() );
	}

	groupList.sort();
	groupList.prepend(QString::null); //add a separator;
	groupList.prepend( i18n("Top Level") ); //the top-level group, with the id 0
	setItems( groupList );
}

#include "kopetegrouplistaction.moc"
