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
}

void ContactListTreeModel::removeMetaContact( Kopete::MetaContact* contact )
{
	ContactListModel::removeMetaContact( contact );

	foreach(Kopete::Group *g, contact->groups())
		removeMetaContactFromGroup(contact, g);
}

void ContactListTreeModel::addGroup( Kopete::Group* group )
{
	int pos = m_groups.count();
	kDebug(14001) << "addGroup" << group->displayName();
	beginInsertRows( QModelIndex(), pos, pos );
	GroupModelItem* gmi = new GroupModelItem( group );
	m_groups.append( gmi );
	m_contacts[gmi] = QList<MetaContactModelItem*>();
	endInsertRows();
}

void ContactListTreeModel::removeGroup( Kopete::Group* group )
{
	int pos = indexOfGroup( group );
	beginRemoveRows( QModelIndex(), pos, pos );
	GroupModelItem* gmi = m_groups.takeAt( pos );
	qDeleteAll( m_contacts.value( gmi ) );
	m_contacts.remove( gmi );
	endRemoveRows();
}

void ContactListTreeModel::addMetaContactToGroup( Kopete::MetaContact *mc, Kopete::Group *group )
{
	int pos = indexOfGroup( group );
	if (pos == -1)
	{
		addGroup( group );
		pos = indexOfGroup( group );
	}

	GroupModelItem* groupModelItem = m_groups.at( pos );

	int mcIndex = indexOfMetaContact( groupModelItem, mc );
	int mcDesireIndex = m_contacts[groupModelItem].count();

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

	// Check if mcDesireIndex isn't invalid (shouldn't happen)
	if ( mcDesireIndex < 0 || mcDesireIndex > m_contacts[groupModelItem].count() )
		mcDesireIndex = m_contacts[groupModelItem].count();

	MetaContactModelItem* mcModelItem = 0;
	if ( mcIndex == -1 )
	{
		mcModelItem = new MetaContactModelItem( groupModelItem, mc );
	}
	else
	{
		// If the manual index is the same do nothing otherwise change possition
		if ( mcIndex == mcDesireIndex )
			return;

		// We're moving metaContact so temporary remove it so model is avare of the change.
		QModelIndex idx = index( pos, 0 );
		beginRemoveRows( idx, mcIndex, mcIndex );
		mcModelItem = m_contacts[groupModelItem].takeAt( mcIndex );
		endRemoveRows();

		// If mcDesireIndex was after mcIndex decrement it because we have removed metaContact
		if ( mcIndex < mcDesireIndex )
			mcDesireIndex--;
	}

	QModelIndex idx = index( pos, 0 );
	beginInsertRows( idx, mcDesireIndex, mcDesireIndex );
	m_contacts[groupModelItem].insert( mcDesireIndex, mcModelItem );
	endInsertRows();

	// emit the dataChanged signal for the group index so that the filtering proxy
	// can evaluate the row
	emit dataChanged(idx, idx);
}

void ContactListTreeModel::removeMetaContactFromGroup( Kopete::MetaContact *mc, Kopete::Group *group )
{
	int pos = indexOfGroup( group );
	Q_ASSERT( pos != -1 );

	GroupModelItem* groupModelItem = m_groups.at( pos );
	// if the mc is not on the list anymore, just returns
	int offset = indexOfMetaContact( groupModelItem, mc );
	if (offset == -1)
		return;

	QModelIndex idx = index(pos, 0);
	beginRemoveRows(idx, offset, offset);
	delete m_contacts[groupModelItem].takeAt(offset);
	endRemoveRows();

	// emit the dataChanged signal for the group index so that the filtering proxy
	// can evaluate if the group row should still be visible
	emit dataChanged(idx, idx);
}

int ContactListTreeModel::indexOfMetaContact( const GroupModelItem* inGroup, const Kopete::MetaContact* mc ) const
{
	QList<MetaContactModelItem*> mcModelItemList = m_contacts.value( inGroup );
	for ( int i = 0; i < mcModelItemList.size(); ++i )
	{
		if ( mcModelItemList.at( i )->metaContact() == mc )
			return i;
	}
	return -1;
}

int ContactListTreeModel::indexOfGroup( Kopete::Group* group ) const
{
	for ( int i = 0; i < m_groups.size(); ++i )
	{
		if ( m_groups.at( i )->group() == group )
			return i;
	}
	return -1;
}

int ContactListTreeModel::childCount( const QModelIndex& parent ) const
{
	int cnt = 0;
	if ( !parent.isValid() )
	{ //Number of groups
		cnt = m_groups.count();
	}
	else
	{
		ContactListTreeModelItem *clmi = static_cast<ContactListTreeModelItem*>( parent.internalPointer() );
		GroupModelItem *gmi = dynamic_cast<GroupModelItem*>( clmi );
		if ( gmi )
			cnt = m_contacts[gmi].count();
	}
	
	return cnt;
}

int ContactListTreeModel::rowCount( const QModelIndex& parent ) const
{
	ContactListTreeModelItem *clmi = static_cast<ContactListTreeModelItem*>( parent.internalPointer() );
	GroupModelItem *gmi = dynamic_cast<GroupModelItem*>( clmi );

	int cnt = 0;
	if ( !parent.isValid() )
		cnt = m_groups.count();
	else
	{
		if ( gmi )
			cnt+= m_contacts[gmi].count();
	}

	return cnt;
}

bool ContactListTreeModel::hasChildren( const QModelIndex& parent ) const
{
	ContactListTreeModelItem *clmi = static_cast<ContactListTreeModelItem*>( parent.internalPointer() );
	GroupModelItem *gmi = dynamic_cast<GroupModelItem*>( clmi );
	
	bool res = false;
	if ( !parent.isValid() )
		res=!m_groups.isEmpty();
	else
	{
		if ( gmi )
		{
			int row = parent.row();
			GroupModelItem *gmi = m_groups[row];
			res = !m_contacts[gmi].isEmpty();
		}
	}
	return res;
}

QModelIndex ContactListTreeModel::index( int row, int column, const QModelIndex & parent ) const
{
	if ( row < 0 || row >= childCount( parent ) )
	{
		return QModelIndex();
	}

	ContactListTreeModelItem *clmi = static_cast<ContactListTreeModelItem*>( parent.internalPointer() );
	GroupModelItem *gmi = dynamic_cast<GroupModelItem*>(clmi);
	
	QModelIndex idx;
	if( !parent.isValid() )
		idx = createIndex( row, column, m_groups[row] );
	else if ( gmi )
		idx = createIndex( row, column, m_contacts[gmi][row] );

	return idx;
}

int ContactListTreeModel::countConnected( GroupModelItem* gmi ) const
{
	int onlineCount = 0;
	
	QList<MetaContactModelItem*> metaContactList = m_contacts.value(gmi);
	QList<MetaContactModelItem*>::const_iterator it, itEnd;
	itEnd = metaContactList.constEnd();
	for (it = metaContactList.constBegin(); it != itEnd; ++it)
	{
	  if ( (*it)->metaContact()->isOnline() )
		onlineCount++;
	}
	
	return onlineCount;
}

bool ContactListTreeModel::setData( const QModelIndex & index, const QVariant & value, int role )
{
	if ( !index.isValid() )
		return false;

	if ( role == Kopete::Items::ExpandStateRole )
	{
		ContactListTreeModelItem *clmi = static_cast<ContactListTreeModelItem*>( index.internalPointer() );
		GroupModelItem *gmi = dynamic_cast<GroupModelItem*>( clmi );

		if ( gmi )
		{
			gmi->group()->setExpanded( value.toBool() );
			emit dataChanged( index, index );
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
	ContactListTreeModelItem *clmi = static_cast<ContactListTreeModelItem*>( index.internalPointer() );
	GroupModelItem *gmi = dynamic_cast<GroupModelItem*>( clmi );
	MetaContactModelItem *mcmi = dynamic_cast<MetaContactModelItem*>( clmi );

	if ( gmi )
	{
		Kopete::Group* g = gmi->group();
		switch ( role )
		{
		case Qt::DisplayRole:
			return i18n( "%1 (%2/%3)", g->displayName(), countConnected( gmi ), m_contacts[gmi].count() );
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

	if ( mcmi )
		return metaContactData( mcmi->metaContact(), role );

	return QVariant();
}

Qt::ItemFlags ContactListTreeModel::flags( const QModelIndex &index ) const
{
	if ( !index.isValid() )
		return (m_manualGroupSorting) ? Qt::ItemIsDropEnabled : Qt::NoItemFlags;

	Qt::ItemFlags f(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	
	// if it is a contact item, add the selectable flag
	if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
	{
		// TODO: for now we are only allowing drag-n-drop of a
		// metacontact if all the accounts its contacts belong are online
		ContactListTreeModelItem *clmi = static_cast<ContactListTreeModelItem*>(index.internalPointer());
		MetaContactModelItem *mcmi = dynamic_cast<MetaContactModelItem*>(clmi);
		if (mcmi)
		{
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
		return dropUrl( data, row, parent );
	}
	else if ( data->hasFormat("application/kopete.group") )
	{
		// we don't support dropping groups into another group
		if ( parent.isValid() )
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

		for (int i=0; i < groups.count(); ++i)
		{
			int gIndex = indexOfGroup( groups.at( i ) );
			Q_ASSERT( gIndex != -1 );

			int gDesireIndex = row + i;

			// Check if mcDesireIndex isn't invalid (shouldn't happen)
			if ( gDesireIndex < 0 || gDesireIndex > m_groups.count() )
				gDesireIndex = m_groups.count();
		
			// If the manual index is the same do nothing otherwise change possition
			if ( gIndex == gDesireIndex )
				continue;
			
			// We're moving group so temporary remove it so model is avare of the change.
			beginRemoveRows( QModelIndex(), gIndex, gIndex );
			GroupModelItem* gModelItem = m_groups.takeAt( gIndex );
			endRemoveRows();
			
			// If gDesireIndex was after gIndex decrement it because we have removed group
			if ( gIndex < gDesireIndex )
				gDesireIndex--;

			beginInsertRows( QModelIndex(), gDesireIndex, gDesireIndex );
			m_groups.insert( gDesireIndex, gModelItem );
			endInsertRows();
		}
	}
	else if ( data->hasFormat("application/kopete.metacontacts.list") )
	{
		// we don't support dropping things in an empty space
		if (!parent.isValid())
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

		return dropMetaContacts( row, parent, items );
	}

	return false;
}

QModelIndex ContactListTreeModel::parent(const QModelIndex & index) const
{
	QModelIndex parent;
	
	if(index.isValid())
	{
		ContactListTreeModelItem *clmi = static_cast<ContactListTreeModelItem*>( index.internalPointer() );
		GroupModelItem *gmi = dynamic_cast<GroupModelItem*>( clmi );
		if ( !gmi )
		{
			MetaContactModelItem *mcmi = dynamic_cast<MetaContactModelItem*>( clmi );
			gmi = mcmi->groupModelItem();
			parent = createIndex( m_groups.indexOf( gmi ), 0, gmi );
		}
	}
	return parent;
}

bool ContactListTreeModel::dropMetaContacts( int row, const QModelIndex &parent, const QList<GroupMetaContactPair> &items )
{
	if ( items.isEmpty() )
		return false;

	if ( ContactListModel::dropMetaContacts( row, parent, items ) )
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
				else
					pair.second->moveToGroup( pair.first, group );
			}
			else if ( pair.first != group )
			{
				pair.second->moveToGroup( pair.first, group );
			}
		}
		return true;
	}
	return false;
}

QModelIndexList ContactListTreeModel::indexListFor( Kopete::ContactListElement* cle ) const
{
	QModelIndexList indexList;
	Kopete::MetaContact *mc = dynamic_cast<Kopete::MetaContact*>(cle);

	if (mc)
	{
		// metacontact handling
		// search for all the groups in which this contact is
		foreach( Kopete::Group *g, mc->groups() )
		{
			int groupPos = indexOfGroup( g );
			int mcPos = indexOfMetaContact( m_groups.at( groupPos ), mc );
			Q_ASSERT( mcPos != -1 );

			// get  the group index to be the parent for the contact search
			QModelIndex grpIndex = index(groupPos, 0);

			QModelIndex mcIndex = index(mcPos, 0, grpIndex);
			if (mcIndex.isValid())
				indexList.append(mcIndex);
		}
	}
	else
	{
		// group handling
		Kopete::Group *g = dynamic_cast<Kopete::Group*>(cle);
		if (g)
		{
			int pos = indexOfGroup( g );
			Q_ASSERT( pos != -1 );
			indexList.append(index(pos,0));
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
		for ( int i = 0; i < m_groups.count(); ++i )
		{
			GroupModelItem* gmi = m_groups.value( i );
			QDomElement groupElement = doc.createElement( "Group" );
			groupElement.setAttribute( "uuid", gmi->group()->groupId() );
			groupElement.setAttribute( "possition", i );
			groupRootElement.appendChild( groupElement );
		}
	}

	if ( m_manualMetaContactSorting )
	{
		QDomElement metaContactRootElement = rootElement.firstChildElement("MetaContactPositions");
		if ( !metaContactRootElement.isNull() )
			rootElement.removeChild( metaContactRootElement );
		
		metaContactRootElement = doc.createElement( "MetaContactPositions" );
		rootElement.appendChild( metaContactRootElement );

		foreach( GroupModelItem* gmi, m_groups )
		{
			QDomElement groupElement = doc.createElement( "Group" );
			groupElement.setAttribute( "uuid", gmi->group()->groupId() );
			metaContactRootElement.appendChild( groupElement );

			QList<MetaContactModelItem*> metaContactList = m_contacts.value( gmi );
			for ( int i = 0; i < metaContactList.count(); ++i )
			{
				MetaContactModelItem* mcmi = metaContactList.value( i );
				QDomElement metaContactElement = doc.createElement( "MetaContact" );
				metaContactElement.setAttribute( "uuid", mcmi->metaContact()->metaContactId() );
				metaContactElement.setAttribute( "possition", i );
				groupElement.appendChild( metaContactElement );
			}
		}
	}
}

// Temporary hashes, only used for sorting when contact list is loaded.
QHash<const GroupModelItem*, int>* _groupPosition = 0;
QHash<const MetaContactModelItem*, int>* _metaContactPosition = 0;

bool manualGroupSort( const GroupModelItem *gmi1, const GroupModelItem *gmi2 )
{
	return _groupPosition->value( gmi1, -1 ) < _groupPosition->value( gmi2, -1 );
}

bool manualMetaContactSort( const MetaContactModelItem *mcmi1, const MetaContactModelItem *mcmi2 )
{
	return _metaContactPosition->value( mcmi1, -1 ) < _metaContactPosition->value( mcmi2, -1 );
}

void ContactListTreeModel::loadModelSettingsImpl( QDomElement& rootElement )
{
	if ( rootElement.isNull() || (!m_manualGroupSorting && !m_manualMetaContactSorting) )
		return;

	// Temporary hash for faster item lookup
	QHash<uint, GroupModelItem*> uuidToGroup;
	foreach( GroupModelItem* gmi, m_groups )
		uuidToGroup.insert( gmi->group()->groupId(), gmi );

	if ( m_manualGroupSorting )
	{
		QDomElement groupRootElement = rootElement.firstChildElement( "GroupPositions" );
		if ( !groupRootElement.isNull() )
		{
			_groupPosition = new QHash<const GroupModelItem*, int>();
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
					_groupPosition->insert( gmi, groupPosition++ );
			}

			qStableSort( m_groups.begin(), m_groups.end(), manualGroupSort );
			delete _groupPosition;
			_groupPosition = 0;
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
				foreach( MetaContactModelItem* mcmi, m_contacts.value( gmi ) )
					uuidToMetaContact.insert( mcmi->metaContact()->metaContactId(), mcmi );

				_metaContactPosition = new QHash<const MetaContactModelItem*, int>();
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
						_metaContactPosition->insert( mcmi, metaContactPosition );
				}

				QList<MetaContactModelItem*> mcList = m_contacts.value( gmi );
				qStableSort( mcList.begin(), mcList.end(), manualMetaContactSort );
				m_contacts.insert( gmi, mcList );

				delete _metaContactPosition;
				_metaContactPosition = 0;
			}
		}
	}
}

}

}

#include "contactlisttreemodel.moc"
//kate: tab-width 4
