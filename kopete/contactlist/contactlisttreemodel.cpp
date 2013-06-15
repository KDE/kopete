/*
    Kopete Contactlist Model

    Copyright (c) 2007      by Aleix Pol              <aleixpol@gmail.com>
    Copyright (c) 2008      by Matt Rogers            <mattr@kde.org>
    Copyright (c) 2009      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright     2009      by Roman Jarosz           <kedgedev@gmail.com>

    Kopete    (c) 2002-2009 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "contactlisttreemodel.h"
#include "contactlisttreemodel_p.h"

#include <QMimeData>
#include <QDomDocument>

#include <KIcon>
#include <KLocale>

#include "kopeteaccount.h"
#include "kopetegroup.h"
#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopeteitembase.h"
#include "kopeteappearancesettings.h"

namespace Kopete {

namespace UI {

ContactListTreeModel::ContactListTreeModel( QObject* parent )
	: ContactListModel( parent )
{
	m_topLevelGroup = new GroupModelItem( Kopete::Group::topLevel() );
	m_groups.insert( m_topLevelGroup->group(), m_topLevelGroup );
}

ContactListTreeModel::~ContactListTreeModel()
{
	saveModelSettings( "Tree" );
}

void ContactListTreeModel::addMetaContact( Kopete::MetaContact* contact )
{
	ContactListModel::addMetaContact( contact );

	foreach( Kopete::Group* g, contact->groups() )
		addMetaContactToGroup( contact, g );

	if (  Kopete::AppearanceSettings::self()->showOfflineGrouped() )
	{
		if ( !contact->isOnline() )
			addMetaContactToGroup( contact, Kopete::Group::offline() );
		else
			removeMetaContactFromGroup( contact, Kopete::Group::offline() );
	}
}

void ContactListTreeModel::removeMetaContact( Kopete::MetaContact* contact )
{
	ContactListModel::removeMetaContact( contact );

	foreach(Kopete::Group *g, contact->groups())
		removeMetaContactFromGroup(contact, g);
}

void ContactListTreeModel::addGroup( Kopete::Group* group )
{
	if ( group == Kopete::Group::topLevel() )
	{
		Q_ASSERT( m_topLevelGroup->group() == group );
	}
	else if ( !m_groups.contains( group ) )
	{
		kDebug(14001) << "addGroup" << group->displayName();

		GroupModelItem* gmi = new GroupModelItem( group );
		m_groups.insert( group, gmi );

		int pos = m_topLevelGroup->count();
		beginInsertRows( indexFor( m_topLevelGroup ), pos, pos );
		m_topLevelGroup->append( gmi );
		Q_ASSERT( gmi->index() == pos );
		endInsertRows();
	}
}

void ContactListTreeModel::removeGroup( Kopete::Group* group )
{
	Q_ASSERT( group != Kopete::Group::topLevel() );

	GroupModelItem* gmi = m_groups.value( group );
	int pos = gmi->index();

	beginRemoveRows( indexFor( gmi->parent() ), pos, pos );
	gmi->remove();
	m_groups.remove( group );
	endRemoveRows();

	delete gmi;
}

void ContactListTreeModel::addMetaContactToGroup( Kopete::MetaContact *mc, Kopete::Group *group )
{
	GroupModelItem* groupModelItem = m_groups.value( group );
	if ( !groupModelItem )
	{
		addGroup( group );
		groupModelItem = m_groups.value( group );
	}
	QModelIndex parent = indexFor( groupModelItem );

	GroupMetaContactPair groupMetaContactPair( group, mc );
	int mcDesireIndex = groupModelItem->count();

	// If we use manual sorting we most likely will have possition where the metaContact should be inserted.
	if ( m_manualMetaContactSorting )
	{
		GroupMetaContactPair groupMetaContactPair( group, mc );
		if ( m_addContactPosition.contains( groupMetaContactPair ) )
		{
			mcDesireIndex = m_addContactPosition.value( groupMetaContactPair );
			m_addContactPosition.remove( groupMetaContactPair );
		}
	}

	// Check if mcDesireIndex isn't invalid (in group area)
	int metaContactCount = groupModelItem->metaContactCount();
	if ( mcDesireIndex < 0 || mcDesireIndex > metaContactCount )
		mcDesireIndex = metaContactCount;

	MetaContactModelItem* mcModelItem = m_metaContacts.value( groupMetaContactPair );
	if ( mcModelItem )
	{
		int mcIndex = mcModelItem->index();
		// If the manual index is the same do nothing otherwise change possition
		if ( mcIndex == mcDesireIndex )
			return;

		// We're moving metaContact so temporary remove it so model is aware of the change.
		beginRemoveRows( parent, mcIndex, mcIndex );
		mcModelItem->remove();
		endRemoveRows();

		// If mcDesireIndex was after mcIndex decrement it because we have removed metaContact
		if ( mcIndex < mcDesireIndex )
			mcDesireIndex--;
	}
	else
	{
		mcModelItem = new MetaContactModelItem( mc );
		m_metaContacts.insert( groupMetaContactPair, mcModelItem );
	}

	beginInsertRows( parent, mcDesireIndex, mcDesireIndex );
	groupModelItem->insert( mcDesireIndex, mcModelItem );
	endInsertRows();

	// emit the dataChanged signal for the group index so that the filtering proxy
	// can evaluate the row
	emit dataChanged( parent, parent );
}

void ContactListTreeModel::removeMetaContactFromGroup( Kopete::MetaContact *mc, Kopete::Group *group )
{
	GroupMetaContactPair groupMetaContactPair( group, mc );
	MetaContactModelItem* mcModelItem = m_metaContacts.value( groupMetaContactPair );
	if ( !mcModelItem )
		return;

	int offset = mcModelItem->index();
	QModelIndex parent = indexFor( mcModelItem->parent() );

	beginRemoveRows( parent, offset, offset);
	mcModelItem->remove();
	m_metaContacts.remove( groupMetaContactPair );
	endRemoveRows();

	delete mcModelItem;

	// emit the dataChanged signal for the group index so that the filtering proxy
	// can evaluate if the group row should still be visible
	emit dataChanged( parent, parent );
}

int ContactListTreeModel::rowCount( const QModelIndex& parent ) const
{
	if ( !parent.isValid() )
		return 1;

	return itemFor( parent )->count();
}

bool ContactListTreeModel::hasChildren( const QModelIndex& parent ) const
{
	if ( !parent.isValid() )
		return true;

	return itemFor( parent )->hasChildren();
}

QModelIndex ContactListTreeModel::index( int row, int column, const QModelIndex & parent ) const
{
	if ( row < 0 || row >= rowCount( parent ) )
		return QModelIndex();

	if ( !parent.isValid() )
		return createIndex( row, column, m_topLevelGroup );

	GroupModelItem *gmi = dynamic_cast<GroupModelItem*>( itemFor( parent ) );
	return createIndex( row, column, gmi->at( row ) );
}

int ContactListTreeModel::countConnected( GroupModelItem* gmi ) const
{
	int onlineCount = 0;

	QList<ContactListModelItem*> items = gmi->items();
	foreach ( ContactListModelItem* clmi, items )
	{
		MetaContactModelItem* mcmi = dynamic_cast<MetaContactModelItem*>(clmi);
		if ( mcmi && mcmi->metaContact() && ( mcmi->metaContact()->isOnline() || mcmi->metaContact()->isAlwaysVisible() ) )
			onlineCount++;
	}

	return onlineCount;
}

bool ContactListTreeModel::setData( const QModelIndex & index, const QVariant & value, int role )
{
	if ( !index.isValid() )
		return false;

	ContactListModel::setData(index,value,role);

	if ( role == Kopete::Items::ExpandStateRole )
	{
		ContactListModelItem *clmi = itemFor( index );
		GroupModelItem *gmi = dynamic_cast<GroupModelItem*>( clmi );

		if ( gmi && gmi->group() )
		{
			if ( gmi->group()->isExpanded() != value.toBool() )
			{
				gmi->group()->setExpanded( value.toBool() );
				emit dataChanged( index, index );
			}
			return true;
		}
	}

	return false;
}

QVariant ContactListTreeModel::data ( const QModelIndex & index, int role ) const
{
	if ( !index.isValid() )
		return QVariant();

	using namespace Kopete;

	/* do all the casting up front. I need to profile to see how expensive this is though */
	ContactListModelItem *clmi = itemFor( index );
	GroupModelItem *gmi = dynamic_cast<GroupModelItem*>( clmi );
	MetaContactModelItem *mcmi = dynamic_cast<MetaContactModelItem*>( clmi );

	if ( gmi && gmi->group() )
	{
		Kopete::Group* g = gmi->group();
		switch ( role )
		{
		case Qt::DisplayRole:
			if ( g == Kopete::Group::offline() )
				return i18n( "%1 (%2)", g->displayName(), gmi->count() );
			else
				return i18n( "%1 (%2/%3)", g->displayName(), countConnected( gmi ), gmi->count() );
			break;
		case Qt::DecorationRole:
			if ( g->isExpanded() )
			{
				if ( g->useCustomIcon() )
					return g->icon();
				else
					return KIcon( KOPETE_GROUP_DEFAULT_OPEN_ICON );
			}
			else
			{
				if ( g->useCustomIcon() )
					return g->icon();
				else
					return KIcon( KOPETE_GROUP_DEFAULT_CLOSED_ICON );
			}
			break;
		case Kopete::Items::TypeRole:
			return Kopete::Items::Group;
			break;
		case Kopete::Items::ObjectRole:
			return qVariantFromValue( (QObject*)g );
			break;
		case Kopete::Items::UuidRole:
			return QUuid().toString();
			break;
		case Kopete::Items::TotalCountRole:
			return g->members().count();
			break;
		case Kopete::Items::ConnectedCountRole:
			return countConnected( gmi );
			break;
		case Kopete::Items::OnlineStatusRole:
			return OnlineStatus::Unknown;
			break;
		case Kopete::Items::IdRole:
			return QString::number(g->groupId());
			break;
		case Kopete::Items::ExpandStateRole:
			return g->isExpanded();
			break;
		}
	}

	if ( mcmi && mcmi->metaContact() )
	{
		if ( role == Kopete::Items::MetaContactGroupRole )
			return qVariantFromValue( (QObject*)mcmi->parent()->group() );
		else if ( role == Kopete::Items::AlwaysVisible )
			return mcmi->metaContact()->isAlwaysVisible();
		else
			return metaContactData( mcmi->metaContact(), role );
	}

	return QVariant();
}

Qt::ItemFlags ContactListTreeModel::flags( const QModelIndex &index ) const
{
	if ( !index.isValid() )
		return Qt::NoItemFlags;

	Qt::ItemFlags f(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	
	// if it is a contact item, add the selectable flag
	if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
	{
		// TODO: for now we are only allowing drag-n-drop of a
		// metacontact if all the accounts its contacts belong are online
		ContactListModelItem *clmi = itemFor( index );
		MetaContactModelItem *mcmi = dynamic_cast<MetaContactModelItem*>(clmi);
		if (mcmi && mcmi->metaContact())
		{
			f |= Qt::ItemIsEditable;
			bool online = true;
			foreach(Kopete::Contact *c, mcmi->metaContact()->contacts())
			{
				if (!c->account()->isConnected())
				{
					online = false;
					break;
				}
			}
			if (online)
				f = f | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
		}
	}
	else if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
	{
		f |= Qt::ItemIsDropEnabled;
		if ( m_manualGroupSorting )
			f |= Qt::ItemIsDragEnabled;
	}
	
	return f;
}

bool ContactListTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                    int row, int column, const QModelIndex &parent)
{
	if (action == Qt::IgnoreAction)
		return true;

	// for now only accepting drop of metacontacts
	if (!data->hasFormat("application/kopete.metacontacts.list") && !data->hasFormat("application/kopete.group") &&
	    !data->hasUrls())
		return false;

	// contactlist has only one column
	if (column > 0)
		return false;

	if ( data->hasUrls() )
	{
		return dropUrl( data, row, parent, action );
	}
	else if ( data->hasFormat("application/kopete.group") )
	{
		// we don't support dropping groups into another group or copying groups
		if ( itemFor( parent ) != m_topLevelGroup || action != Qt::MoveAction )
			return false;

		// decode the mime data
		QByteArray encodedData = data->data("application/kopete.group");
		QDataStream stream(&encodedData, QIODevice::ReadOnly);
		QList<Kopete::Group*> groups;

		while (!stream.atEnd())
		{
			QString groupUuid;
			stream >> groupUuid;
			groups.append(Kopete::ContactList::self()->group( groupUuid.toUInt() ));
		}

		GroupModelItem *newParent = dynamic_cast<GroupModelItem*>( itemFor( parent ) );
		for ( int i=0; i < groups.count(); ++i )
		{
			GroupModelItem* gmi = m_groups.value( groups.at( i ) );
			Q_ASSERT( gmi );

			int gIndex = gmi->index();
			int gDesireIndex = row + i;

			// Check if mcDesireIndex isn't invalid
			int mcCount = newParent->metaContactCount();
			if ( gDesireIndex < mcCount )
				gDesireIndex = mcCount;
			else if ( gDesireIndex > newParent->count() )
				gDesireIndex = newParent->count();

			// If the manual index is the same do nothing otherwise change possition
			if ( gIndex == gDesireIndex )
				continue;

			// We're moving group so temporary remove it so model is aware of the change.
			beginRemoveRows( indexFor( gmi->parent() ), gIndex, gIndex );
			gmi->remove();
			endRemoveRows();

			// If gDesireIndex was after gIndex decrement it because we have removed group
			if ( gIndex < gDesireIndex )
				gDesireIndex--;

			beginInsertRows( parent, gDesireIndex, gDesireIndex );
			newParent->insert( gDesireIndex, gmi );
			endInsertRows();
		}
	}
	else if ( data->hasFormat("application/kopete.metacontacts.list") )
	{
		// we don't support dropping things in an empty space
		if ( !parent.isValid() )
			return false;

		// decode the mime data
		QByteArray encodedData = data->data("application/kopete.metacontacts.list");
		QDataStream stream(&encodedData, QIODevice::ReadOnly);
		QList<GroupMetaContactPair> items;

		while (!stream.atEnd())
		{
			QString line;
			stream >> line;

			QStringList entry = line.split("/");

			QString grp = entry[0];
			QString id = entry[1];

			GroupMetaContactPair pair;
			pair.first = Kopete::ContactList::self()->group( grp.toUInt() );
			pair.second = Kopete::ContactList::self()->metaContact( QUuid(id) );
			items.append( pair );
		}

		return dropMetaContacts( row, parent, action, items );
	}

	return false;
}

QModelIndex ContactListTreeModel::parent( const QModelIndex & index ) const
{
	if ( !index.isValid() )
		return QModelIndex();

	return indexFor( itemFor( index )->parent() );
}

bool ContactListTreeModel::dropMetaContacts( int row, const QModelIndex &parent, Qt::DropAction action, const QList<GroupMetaContactPair> &items )
{
	if ( items.isEmpty() )
		return false;

	if ( ContactListModel::dropMetaContacts( row, parent, action, items ) )
		return true;

	if ( parent.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
	{
		QObject* groupObject = qVariantValue<QObject*>( parent.data( Kopete::Items::ObjectRole ) );
		Kopete::Group* group = qobject_cast<Kopete::Group*>(groupObject);

		// if we have metacontacts on the mime data, move them to this group
		for ( int i = 0; i < items.count(); ++i )
		{
			GroupMetaContactPair pair = items.at( i );
			if ( m_manualMetaContactSorting )
			{
				m_addContactPosition.insert( pair, row + i );
				if ( pair.first == group )
					addMetaContactToGroup( pair.second, group );
				else if ( action == Qt::MoveAction )
					pair.second->moveToGroup( pair.first, group );
				else if ( action == Qt::CopyAction )
					pair.second->addToGroup( group );
			}
			else if ( pair.first != group )
			{
				if ( action == Qt::MoveAction )
					pair.second->moveToGroup( pair.first, group );
				else if ( action == Qt::CopyAction )
					pair.second->addToGroup( group );
			}
		}
		return true;
	}
	return false;
}

ContactListModelItem* ContactListTreeModel::itemFor( const QModelIndex& index ) const
{
	Q_ASSERT( index.isValid() );
	return static_cast<ContactListModelItem*>( index.internalPointer() );
}

QModelIndex ContactListTreeModel::indexFor( ContactListModelItem* modelItem ) const
{
	if ( modelItem == 0 )
		return QModelIndex(); // Invisible Root Item

	if ( modelItem == m_topLevelGroup )
		return createIndex( 0, 0, modelItem ); // TopLevel Group (is hidden in view with setRootIndex)
	else
		return createIndex( modelItem->index(), 0, modelItem );
}

QModelIndexList ContactListTreeModel::indexListFor( Kopete::ContactListElement* cle ) const
{
	QModelIndexList indexList;
	Kopete::MetaContact *mc = dynamic_cast<Kopete::MetaContact*>(cle);

	// Contact list doesn't contain myself account contact so ignore it
	if (mc && mc != Kopete::ContactList::self()->myself())
	{
		// metacontact handling
		// search for all the groups in which this contact is
		foreach( Kopete::Group *g, mc->groups() )
		{
			GroupMetaContactPair groupMetaContactPair( g, mc );
			MetaContactModelItem* mcmi = m_metaContacts.value( groupMetaContactPair );
			if ( mcmi )
			{
				QModelIndex mcIndex = indexFor( mcmi );
				if ( mcIndex.isValid() )
					indexList.append( mcIndex );
			}
		}
	}
	else if (!mc)
	{
		// group handling
		Kopete::Group *g = dynamic_cast<Kopete::Group*>(cle);
		if (g)
		{
			GroupModelItem* gmi = m_groups.value( g );
			Q_ASSERT( gmi );
			indexList.append( indexFor( gmi ) );
		}
	}

	return indexList;
}

void ContactListTreeModel::handleContactDataChange(Kopete::MetaContact* mc)
{
	// get all the indexes for this metacontact
	QModelIndexList indexList = indexListFor(mc);

	// and now notify all the changes
	foreach(QModelIndex index, indexList)
	{
		if ( !mc->isOnline() )
			addMetaContactToGroup( mc, Kopete::Group::offline() );
		else
			removeMetaContactFromGroup( mc, Kopete::Group::offline() );

		// we need to emit the dataChanged signal to the groups this metacontact belongs to
		// so that proxy filtering is aware of the changes, first we have to update the parent
		// otherwise the group won't be expandable.
		emit dataChanged(index.parent(), index.parent());
		emit dataChanged(index, index);
	}
}

void ContactListTreeModel::appearanceConfigChanged()
{
	AppearanceSettings* as = AppearanceSettings::self();
	bool manualGroupSorting = (as->contactListGroupSorting() == AppearanceSettings::EnumContactListGroupSorting::Manual);
	bool manualMetaContactSorting = (as->contactListMetaContactSorting() == AppearanceSettings::EnumContactListMetaContactSorting::Manual);

	if ( m_manualGroupSorting != manualGroupSorting || m_manualMetaContactSorting != manualMetaContactSorting )
	{
		saveModelSettings( "Tree" );
		m_manualGroupSorting = manualGroupSorting;
		m_manualMetaContactSorting = manualMetaContactSorting;
		loadModelSettings( "Tree" );
	}
}

void ContactListTreeModel::loadContactList()
{
	ContactListModel::loadContactList();

	addGroup( Kopete::Group::topLevel() );

	foreach ( Kopete::Group* g, Kopete::ContactList::self()->groups() )
		addGroup( g );

	foreach ( Kopete::MetaContact* mc, Kopete::ContactList::self()->metaContacts() )
		addMetaContact( mc );

	if ( m_manualGroupSorting || m_manualMetaContactSorting )
	{
		loadModelSettings( "Tree" );
		reset();
	}
}

void ContactListTreeModel::saveModelSettingsImpl( QDomDocument& doc, QDomElement& rootElement )
{
	if ( !m_manualGroupSorting && !m_manualMetaContactSorting )
		return;

	if ( m_manualGroupSorting )
	{
		QDomElement groupRootElement = rootElement.firstChildElement( "GroupPositions" );
		if ( !groupRootElement.isNull() )
			rootElement.removeChild( groupRootElement );

		groupRootElement = doc.createElement( "GroupPositions" );
		rootElement.appendChild( groupRootElement );

		int index = 0;
		foreach ( ContactListModelItem* clmi, m_topLevelGroup->items() )
		{
			if ( clmi->isGroup() )
			{
				GroupModelItem* gmi = dynamic_cast<GroupModelItem*>( clmi );
				if ( gmi->group() ) {
					QDomElement groupElement = doc.createElement( "Group" );
					groupElement.setAttribute( "uuid", gmi->group()->groupId() );
					groupElement.setAttribute( "possition", index++ );
					groupRootElement.appendChild( groupElement );
				}
			}
		}
	}

	if ( m_manualMetaContactSorting )
	{
		QDomElement metaContactRootElement = rootElement.firstChildElement("MetaContactPositions");
		if ( !metaContactRootElement.isNull() )
			rootElement.removeChild( metaContactRootElement );

		metaContactRootElement = doc.createElement( "MetaContactPositions" );
		rootElement.appendChild( metaContactRootElement );

		QHashIterator<Kopete::Group*, GroupModelItem*> it( m_groups );
		while ( it.hasNext() )
		{
			GroupModelItem* gmi = it.next().value();
			QDomElement groupElement = doc.createElement( "Group" );
			if ( ! gmi->group() ) continue;
			groupElement.setAttribute( "uuid", gmi->group()->groupId() );
			metaContactRootElement.appendChild( groupElement );

			int index = 0;
			foreach ( ContactListModelItem* clmi, gmi->items() )
			{
				if ( !clmi->isGroup() )
				{
					MetaContactModelItem* mcmi = dynamic_cast<MetaContactModelItem*>( clmi );
					if ( mcmi->metaContact() ) {
						QDomElement metaContactElement = doc.createElement( "MetaContact" );
						metaContactElement.setAttribute( "uuid", mcmi->metaContact()->metaContactId() );
						metaContactElement.setAttribute( "possition", index++ );
						groupElement.appendChild( metaContactElement );
					}
				}
			}
		}
	}
}

// Temporary hash, only used for sorting when contact list is loaded.
QHash<const ContactListModelItem*, int>* _contactListModelItemPosition = 0;

bool contactListModelItemSort( const ContactListModelItem *item1, const ContactListModelItem *item2 )
{

	if ( item1->isGroup() != item2->isGroup() )
	{	//Groups are always after metaContacts
		return !item1->isGroup();
	}

	return _contactListModelItemPosition->value( item1, -1 ) < _contactListModelItemPosition->value( item2, -1 );
}

void ContactListTreeModel::loadModelSettingsImpl( QDomElement& rootElement )
{
	if ( rootElement.isNull() || (!m_manualGroupSorting && !m_manualMetaContactSorting) )
		return;

	// Temporary hash for faster item lookup
	QHash<uint, GroupModelItem*> uuidToGroup;
	QHashIterator<Kopete::Group*, GroupModelItem*> it( m_groups );
	while ( it.hasNext() )
	{
		GroupModelItem* gmi = it.next().value();
		if ( ! gmi->group() ) continue;
		uuidToGroup.insert( gmi->group()->groupId(), gmi );
	}

	_contactListModelItemPosition = new QHash<const ContactListModelItem*, int>();
	if ( m_manualGroupSorting )
	{
		QDomElement groupRootElement = rootElement.firstChildElement( "GroupPositions" );
		if ( !groupRootElement.isNull() )
		{
			QDomNodeList groupList = groupRootElement.elementsByTagName("Group");

			for ( int index = 0; index < groupList.size(); ++index )
			{
				QDomElement groupElement = groupList.item( index ).toElement();
				if ( groupElement.isNull() )
					continue;

				// Put position into hash.
				int uuid = groupElement.attribute( "uuid", "-1" ).toInt();
				int groupPosition = groupElement.attribute( "possition", "-1" ).toInt();
				GroupModelItem* gmi = uuidToGroup.value( uuid, 0 );
				if ( gmi )
					_contactListModelItemPosition->insert( gmi, groupPosition );
			}
		}
	}

	if ( m_manualMetaContactSorting )
	{
		QDomElement metaContactRootElement = rootElement.firstChildElement("MetaContactPositions");
		if ( !metaContactRootElement.isNull() )
		{
			QDomNodeList groupList = metaContactRootElement.elementsByTagName("Group");

			for ( int groupIndex = 0; groupIndex < groupList.size(); ++groupIndex )
			{
				QDomElement groupElement = groupList.item( groupIndex ).toElement();
				if ( groupElement.isNull() )
					continue;

				int gUuid = groupElement.attribute( "uuid", "-1" ).toInt();
				GroupModelItem* gmi = uuidToGroup.value( gUuid, 0 );
				if ( !gmi )
					continue;

				// Temporary hash for faster item lookup
				QHash<QUuid, MetaContactModelItem*> uuidToMetaContact;
				foreach ( ContactListModelItem* clmi, gmi->items() )
				{
					if ( !clmi->isGroup() )
					{
						MetaContactModelItem* mcmi = dynamic_cast<MetaContactModelItem*>(clmi);
						if ( mcmi->metaContact() )
							uuidToMetaContact.insert( mcmi->metaContact()->metaContactId(), mcmi );
					}
				}

				QDomNodeList metaContactList = groupElement.elementsByTagName("MetaContact");
				for ( int index = 0; index < metaContactList.size(); ++index )
				{
					QDomElement metaContactElement = metaContactList.item( index ).toElement();
					if ( metaContactElement.isNull() )
						continue;

					// Put position into hash.
					QUuid uuid( metaContactElement.attribute( "uuid" ) );
					int metaContactPosition = metaContactElement.attribute( "possition", "-1" ).toInt();
					MetaContactModelItem* mcmi = uuidToMetaContact.value( uuid, 0 );
					if ( mcmi )
						_contactListModelItemPosition->insert( mcmi, metaContactPosition );
				}
			}
		}
	}

	m_topLevelGroup->sort( contactListModelItemSort );
	delete _contactListModelItemPosition;
	_contactListModelItemPosition = 0;
}


int ContactListModelItem::index() const
{
	if ( mParent )
		return mParent->indexOf( this );
	else
		return -1;
}

bool ContactListModelItem::remove()
{
	int i = index();
	if ( i == -1 )
		return false;
	
	mParent->removeAt( i );
	mParent = 0;
	return true;
}

int GroupModelItem::metaContactCount() const
{
	int mcCount = 0;
	foreach ( ContactListModelItem* clmi, mItems )
	{
		if ( !clmi->isGroup() )
			mcCount++;
	}

	return mcCount;
}

void GroupModelItem::sort( bool (*lessThan)(const ContactListModelItem*, const ContactListModelItem*) )
{
	foreach ( ContactListModelItem* clmi, mItems )
		clmi->sort( lessThan );

	qStableSort( mItems.begin(), mItems.end(), lessThan );
}

int GroupModelItem::indexOf( const ContactListModelItem* item ) const
{
	// FIXME: why I cannot use "return mItems.indexOf( item )" I get compilation error???
	for ( int i = 0; i < mItems.count(); ++i )
	{
		if ( mItems.at( i ) == item )
			return i;
	}

	return -1;
}

}

}

#include "contactlisttreemodel.moc"
//kate: tab-width 4
