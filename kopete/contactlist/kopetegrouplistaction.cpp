/*
    kopetegrouplistcation.cpp   -  the action used for Move To and copy To

    Copyright (c) 2002-2004 by Olivier Goffart       <ogoffart@kde.org>
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
#include <kicon.h>
#include <kaction.h>
#include <kcmultidialog.h>

#include "kopetecontactlist.h"
#include "kopetegroup.h"

KopeteGroupListAction::KopeteGroupListAction( const QString &text, const QString &pix, const KShortcut &cut, const QObject *receiver,
                                              const char *slot, QObject* parent )
: KSelectAction( KIcon(pix), text, parent )
{
	setShortcut(cut);
	if( receiver && slot )
		connect( this, SIGNAL(triggered(int)), receiver, slot );

	connect( Kopete::ContactList::self(), SIGNAL(groupAdded(Kopete::Group*)), this, SLOT(slotUpdateList()) );
	connect( Kopete::ContactList::self(), SIGNAL(groupRemoved(Kopete::Group*)), this, SLOT(slotUpdateList()) );
	connect( Kopete::ContactList::self(), SIGNAL(groupRenamed(Kopete::Group*,QString)), this, SLOT(slotUpdateList()) );
	slotUpdateList();
}

KopeteGroupListAction::~KopeteGroupListAction()
{
}

void KopeteGroupListAction::slotUpdateList()
{
	QMap<QString, uint> groupMap;

	// Add groups to our map
	foreach ( const Kopete::Group* group, Kopete::ContactList::self()->groups() )
	{
		if( group->type() == Kopete::Group::Normal )
			groupMap.insertMulti( group->displayName(), group->groupId() ); // Use insertMulti to be safer
	}

	clear();

	KAction* topLevelAction = addAction( Kopete::Group::topLevel()->displayName() );
	topLevelAction->setData( Kopete::Group::topLevel()->groupId() );

	QAction* separator = new QAction( this );
	separator->setSeparator( true );
	addAction( separator );

	QMapIterator<QString, uint> it( groupMap );
	while ( it.hasNext() )
	{
		it.next();
		QAction* action = addAction( it.key() );
		action->setData( it.value() );
	}
}

#include "kopetegrouplistaction.moc"
