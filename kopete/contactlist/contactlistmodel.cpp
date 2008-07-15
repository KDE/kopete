/*
    Kopete Contactlist Model

    Copyright (c) 2007      by Aleix Pol              <aleixpol@gmail.com>

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

#include <KIcon>
#include <KDebug>
#include <KLocale>

#include "kopetegroup.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopeteitembase.h"

namespace Kopete {

namespace UI {

ContactListModel::ContactListModel(QObject* parent)
 : QAbstractItemModel(parent)
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
	foreach(Kopete::Group* g, contact->groups()) {
		m_contacts[g].append(contact);
	}
}

void ContactListModel::removeMetaContact( Kopete::MetaContact* contact )
{

}

void ContactListModel::addGroup( Kopete::Group* group )
{
	kDebug(14001) << "addGroup" << group->displayName();
	beginInsertRows (QModelIndex(), rowCount(), rowCount()+1);
	m_groups.append(group);
	m_contacts[group]=QList<Kopete::MetaContact*>();
	endInsertRows();
}

void ContactListModel::removeGroup( Kopete::Group* group )
{
	int pos=m_groups.indexOf(group);
	beginRemoveRows(QModelIndex(), pos, pos);
	m_groups.removeAt(m_groups.indexOf(group));
	m_contacts.remove(group);
	endRemoveRows();
}

int ContactListModel::childCount(const QModelIndex& parent) const
{
	int cnt=0;
	if(!parent.isValid()) { //Number of groups
		cnt = m_groups.count();
	} else {
		Kopete::ContactListElement *cle=static_cast<Kopete::ContactListElement*>(parent.internalPointer());
		Kopete::Group *g=dynamic_cast<Kopete::Group*>(cle);
		
		cnt= m_contacts[g].count();
	}
	return cnt;
}

int ContactListModel::rowCount ( const QModelIndex & parent) const
{
	Kopete::ContactListElement *cle=static_cast<Kopete::ContactListElement*>(parent.internalPointer());
	Kopete::Group *g=dynamic_cast<Kopete::Group*>(cle);
	
	int cnt=0;
	if(!parent.isValid()) { //Number of groups and contacts
		cnt = m_groups.count();
		foreach(QList<Kopete::MetaContact*> l, m_contacts.values()) {
			cnt+=l.count();
		}
	} else if(g) {
		cnt+= m_contacts[g].count();
	}
	return cnt;
}

bool ContactListModel::hasChildren ( const QModelIndex & parent) const
{
	Kopete::ContactListElement *cle=static_cast<Kopete::ContactListElement*>(parent.internalPointer());
	Kopete::Group *g=dynamic_cast<Kopete::Group*>(cle);
	
	bool res=false;
	if(!parent.isValid()) {
		res=!m_groups.isEmpty();
	} else if(g) {
		int row = parent.row();
		Kopete::Group *g=m_groups[row];
		res=!m_contacts[g].isEmpty();
	}
	return res;
}

QModelIndex ContactListModel::index ( int row, int column, const QModelIndex & parent) const
{
	kDebug(14001) << "idx" << row << column << data(parent);
	if(row<0 || row>=childCount(parent)) {
		return QModelIndex();
	}
	
	Kopete::ContactListElement *cle=static_cast<Kopete::ContactListElement*>(parent.internalPointer());
	Kopete::Group *g=dynamic_cast<Kopete::Group*>(cle);
	
	QModelIndex idx;
	Kopete::ContactListElement *itemPtr=0;
	if(!parent.isValid()) {
		itemPtr=m_groups[row];
	} else if(g) {
		itemPtr=m_contacts[g][row];
	}
	idx=createIndex(row, column, itemPtr);
	return idx;
}

int ContactListModel::countConnected(Kopete::Group* g) const
{
	int onlineCount=0;
	QList<Kopete::MetaContact*>::const_iterator it=m_contacts[g].constBegin(), itEnd=m_contacts[g].constBegin();
	for(; it!=itEnd; ++it) {
		if((*it)->isOnline())
			onlineCount++;
	}
	return onlineCount;
}

QVariant ContactListModel::data ( const QModelIndex & index, int role ) const
{
	if(!index.isValid())
		return QVariant();
	
	using namespace Kopete;
	
	/* do all the casting up front. I need to profile to see how expensive this is though */
	ContactListElement *cle = static_cast<ContactListElement*>(index.internalPointer());
	Group *g = qobject_cast<Group*>(cle);
	MetaContact *mc = qobject_cast<MetaContact*>(cle);

	if ( role == Qt::DisplayRole ) {
		QString display;
		if ( g )
		{
			display = i18n("%1 (%2/%3)", g->displayName(), countConnected(g),
			               m_contacts[g].count());
		}
		else
		{
			if ( mc )
				display = mc->displayName();
		}
		return display;
	}

	if ( role == Qt::DecorationRole )
	{
		if ( g )
		{
			if ( g->isExpanded() )
			{
				if ( g->useCustomIcon() )
					return g->icon();
				else
					return KIcon(KOPETE_GROUP_DEFAULT_OPEN_ICON);
			}
			else
			{
				if ( g->useCustomIcon() )
					return g->icon();
				else
					return KIcon(KOPETE_GROUP_DEFAULT_CLOSED_ICON);
			}
		}
	}

	if ( role == Kopete::Items::TypeRole )
	{
		if ( g )
			return Kopete::Items::Group;
		else
			return Kopete::Items::MetaContact;
	}

	return QVariant();
}

Qt::ItemFlags ContactListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;
	
	return Qt::ItemIsEnabled;
}

QModelIndex ContactListModel::parent(const QModelIndex & index) const
{
	QModelIndex parent;
	if(index.isValid()) {
		Kopete::ContactListElement *cle=static_cast<Kopete::ContactListElement*>(index.internalPointer());
		Kopete::Group *g=dynamic_cast<Kopete::Group*>(cle);
		if(!g) {
			Kopete::MetaContact *mc=dynamic_cast<Kopete::MetaContact*>(cle);
			parent=createIndex(0, 0, mc->groups().last());
		}
	}
	return parent;
}

}

}

#include "contactlistmodel.moc"
