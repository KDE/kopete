/*
    Kopete Contactlist Model

    Copyright (c) 2007      by Matt Rogers            <mattr@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "contactlistmodel.h"

#include <QStandardItem>
#include <QList>

#include "kopetegroup.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetemetacontactitem.h"
#include "kopetegroupitem.h"

namespace Kopete {

namespace UI {

ContactListModel::ContactListModel(QObject* parent)
 : QStandardItemModel(parent)
{
	Kopete::ContactList* kcl = Kopete::ContactList::self();
	connect( kcl, SIGNAL( metaContactAdded( Kopete::MetaContact* ) ),
	         this, SLOT( addMetaContact( Kopete::MetaContact* ) ) );
	connect( kcl, SIGNAL( groupAdded( Kopete::Group* ) ),
	         this, SLOT( addGroup( Kopete::Group* ) ) );
}


ContactListModel::~ContactListModel()
{
}

void ContactListModel::addMetaContact( Kopete::MetaContact* contact )
{
	//create the metacontact item
	KopeteMetaContactItem* mcItem = new KopeteMetaContactItem( contact );
	//find the group item for this metacontact
	Kopete::Group* group = contact->groups().first();
	QList<QStandardItem*> groupItems = findItems( group->displayName() );

	QList<QStandardItem*>::iterator it, itEnd = groupItems.end();
	for ( it = groupItems.begin(); it != itEnd; ++it )
	{
		QStandardItem* standardItem  = (*it);
		KopeteGroupItem* groupItem = dynamic_cast<KopeteGroupItem*>( standardItem );
		if ( groupItem != 0 && groupItem->group() == group )
			//add the metacontact item
			standardItem->appendRow( mcItem );
	}
}

void ContactListModel::removeMetaContact( Kopete::MetaContact* contact )
{

}

void ContactListModel::addGroup( Kopete::Group* group )
{
	KopeteGroupItem* groupItem = new KopeteGroupItem( group );
	appendRow( groupItem );
}

void ContactListModel::removeGroup( Kopete::Group* group )
{
}


}

}

#include "contactlistmodel.moc"
