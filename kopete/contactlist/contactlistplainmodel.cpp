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

#include "contactlistplainmodel.h"

#include <QMimeData>
#include <QDomDocument>

#include "kopeteaccount.h"
#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopeteitembase.h"
#include "kopeteappearancesettings.h"

namespace Kopete {

namespace UI {

ContactListPlainModel::ContactListPlainModel( QObject* parent )
 : ContactListModel( parent )
{
}

ContactListPlainModel::~ContactListPlainModel()
{
	saveModelSettings( "Plain" );
}

void ContactListPlainModel::addMetaContact( Kopete::MetaContact *mc )
{
	ContactListModel::addMetaContact( mc );
	addMetaContactImpl( mc );
}

void ContactListPlainModel::addMetaContactImpl( Kopete::MetaContact *mc )
{
	int mcIndex = m_contacts.indexOf( mc );
	int mcDesireIndex = m_contacts.count();

	// If we use manual sorting we most likely will have possition where the metaContact should be inserted.
	if ( m_manualMetaContactSorting )
	{
		if ( m_addContactPosition.contains( mc ) )
		{
			mcDesireIndex = m_addContactPosition.value( mc );
			m_addContactPosition.remove( mc );
		}
	}

	// Check if mcDesireIndex isn't invalid (shouldn't happen)
	if ( mcDesireIndex < 0 || mcDesireIndex > m_contacts.count() )
		mcDesireIndex = m_contacts.count();

	if ( mcIndex != -1 )
	{
		// If the manual index is the same do nothing otherwise change possition
		if ( mcIndex == mcDesireIndex )
			return;

		// We're moving metaContact so temporary remove it so model is aware of the change.
		beginRemoveRows( QModelIndex(), mcIndex, mcIndex );
		m_contacts.removeAt( mcIndex );
		endRemoveRows();

		// If mcDesireIndex was after mcIndex decrement it because we have removed metaContact
		if ( mcIndex < mcDesireIndex )
			mcDesireIndex--;
	}

	beginInsertRows( QModelIndex(), mcDesireIndex, mcDesireIndex );
	m_contacts.insert( mcDesireIndex, mc );
	endInsertRows();
}

void ContactListPlainModel::removeMetaContact( Kopete::MetaContact *mc )
{
	ContactListModel::removeMetaContact( mc );

	// if the mc is not on the list anymore, just returns
	int offset = m_contacts.indexOf( mc );
	if (offset == -1)
		return;

	beginRemoveRows(QModelIndex(), offset, offset);
	m_contacts.removeAt(offset);
	endRemoveRows();
}

int ContactListPlainModel::rowCount( const QModelIndex& parent ) const
{
	if ( !parent.isValid() )
		return m_contacts.count();
	else
		return 0;
}

bool ContactListPlainModel::hasChildren( const QModelIndex& parent ) const
{
	if ( !parent.isValid() )
		return !m_contacts.isEmpty();
	else
		return false;
}

QModelIndex ContactListPlainModel::index( int row, int column, const QModelIndex & parent ) const
{
	if ( row < 0 || row >= rowCount( parent ) )
		return QModelIndex();

	QModelIndex idx;
	if ( !parent.isValid() )
		return createIndex( row, column, m_contacts[row] );
	else
		return QModelIndex();
}


QVariant ContactListPlainModel::data ( const QModelIndex & index, int role ) const
{
	if ( !index.isValid() )
		return QVariant();
	
	using namespace Kopete;
	
	/* do all the casting up front. I need to profile to see how expensive this is though */
	Kopete::ContactListElement *cle = static_cast<Kopete::ContactListElement*>( index.internalPointer() );
	Kopete::MetaContact *mc = dynamic_cast<Kopete::MetaContact*>(cle);

	return metaContactData( mc, role );
}

Qt::ItemFlags ContactListPlainModel::flags( const QModelIndex &index ) const
{
	if ( !index.isValid() )
		return (m_manualMetaContactSorting) ? Qt::ItemIsDropEnabled : Qt::NoItemFlags;

	Qt::ItemFlags f(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	
	// if it is a contact item, add the selectable flag
	if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
	{
		// TODO: for now we are only allowing drag-n-drop of a
		// metacontact if all the accounts its contacts belong are online
		Kopete::ContactListElement *cle = static_cast<Kopete::ContactListElement*>( index.internalPointer() );
		Kopete::MetaContact *mc = dynamic_cast<Kopete::MetaContact*>(cle);
		if (mc)
		{
			f |= Qt::ItemIsEditable;
			bool online = true;
			foreach(Kopete::Contact *c, mc->contacts())
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
	
	return f;
}

bool ContactListPlainModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                    int row, int column, const QModelIndex &parent)
{
	if ( action == Qt::IgnoreAction )
		return true;

	// for now only accepting drop of metacontacts
	// TODO: support dropping of files in metacontacts to allow file transfers
	if ( !data->hasFormat("application/kopete.metacontacts.list") && !data->hasUrls() )
		return false;

	// contactlist has only one column
	if (column > 0)
		return false;

	if ( data->hasUrls() )
	{
		return dropUrl( data, row, parent, action );
	}
	else if ( data->hasFormat("application/kopete.metacontacts.list") )
	{
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

bool ContactListPlainModel::dropMetaContacts( int row, const QModelIndex &parent, Qt::DropAction action, const QList<GroupMetaContactPair> &items )
{
	if ( items.isEmpty() )
		return false;

	if ( ContactListModel::dropMetaContacts( row, parent, action, items ) )
	     return true;

	if ( !parent.isValid() )
	{
		for ( int i = 0; i < items.count(); ++i )
		{
			if ( m_manualMetaContactSorting )
			{
				GroupMetaContactPair pair = items.at( i );
				m_addContactPosition.insert( pair.second, row + i );
				addMetaContactImpl( pair.second );
			}
		}
		return true;
	}

	return false;
}

QModelIndex ContactListPlainModel::parent( const QModelIndex& ) const
{
	return QModelIndex();
}

QModelIndexList ContactListPlainModel::indexListFor( Kopete::ContactListElement* cle ) const
{
	QModelIndexList indexList;
	Kopete::MetaContact *mc = dynamic_cast<Kopete::MetaContact*>(cle);

	// Contact list doesn't contain myself account contact so ignore it
	if (mc && mc != Kopete::ContactList::self()->myself())
	{
		int mcPos = m_contacts.indexOf( mc );
		if ( mcPos != -1 )
		{
			QModelIndex mcIndex = index(mcPos, 0);
			if (mcIndex.isValid())
					indexList.append(mcIndex);
		}
	}

	return indexList;
}

void ContactListPlainModel::handleContactDataChange(Kopete::MetaContact* mc)
{
	// get all the indexes for this metacontact
	QModelIndexList indexList = indexListFor(mc);

	// and now notify all the changes
	foreach(QModelIndex index, indexList)
		emit dataChanged(index, index);
}

void ContactListPlainModel::appearanceConfigChanged()
{
	AppearanceSettings* as = AppearanceSettings::self();
	bool manualGroupSorting = (as->contactListGroupSorting() == AppearanceSettings::EnumContactListGroupSorting::Manual);
	bool manualMetaContactSorting = (as->contactListMetaContactSorting() == AppearanceSettings::EnumContactListMetaContactSorting::Manual);

	if ( m_manualMetaContactSorting != manualMetaContactSorting )
	{
		saveModelSettings( "Plain" );
		m_manualGroupSorting = manualGroupSorting;
		m_manualMetaContactSorting = manualMetaContactSorting;
		loadModelSettings( "Plain" );
	}
}

void ContactListPlainModel::loadContactList()
{
	ContactListModel::loadContactList();

	foreach ( Kopete::MetaContact* mc, Kopete::ContactList::self()->metaContacts() )
		addMetaContact( mc );

	if ( m_manualMetaContactSorting )
	{
		loadModelSettings( "Plain" );
		reset();
	}
}

void ContactListPlainModel::saveModelSettingsImpl( QDomDocument& doc, QDomElement& rootElement )
{
	if ( m_manualMetaContactSorting )
	{
		QDomElement metaContactRootElement = rootElement.firstChildElement( "MetaContactPositions" );
		if ( !metaContactRootElement.isNull() )
			rootElement.removeChild( metaContactRootElement );

		metaContactRootElement = doc.createElement( "MetaContactPositions" );
		rootElement.appendChild( metaContactRootElement );

		for ( int i = 0; i < m_contacts.count(); ++i )
		{
			Kopete::MetaContact* mc = m_contacts.value( i );
			QDomElement metaContactElement = doc.createElement( "MetaContact" );
			metaContactElement.setAttribute( "uuid", mc->metaContactId() );
			metaContactElement.setAttribute( "possition", i );
			metaContactRootElement.appendChild( metaContactElement );
		}
	}
}

// Temporary hash, only used for sorting when contact list is loaded.
QHash<const Kopete::MetaContact*, int>* _metaContactPositionPlain = 0;

bool manualMetaContactSort( const Kopete::MetaContact *mc1, const Kopete::MetaContact *mc2 )
{
	return _metaContactPositionPlain->value( mc1, -1 ) < _metaContactPositionPlain->value( mc2, -1 );
}

void ContactListPlainModel::loadModelSettingsImpl( QDomElement& rootElement )
{
	if ( !m_manualMetaContactSorting )
		return;

	if ( m_manualMetaContactSorting )
	{
		QDomElement metaContactRootElement = rootElement.firstChildElement( "MetaContactPositions" );
		if ( !metaContactRootElement.isNull() )
		{
			// Temporary hash for faster item lookup
			QHash<QUuid, const Kopete::MetaContact*> uuidToMetaContact;
			foreach( const Kopete::MetaContact* mc, m_contacts )
				uuidToMetaContact.insert( mc->metaContactId(), mc );

			_metaContactPositionPlain = new QHash<const Kopete::MetaContact*, int>();
			QDomNodeList metaContactList = metaContactRootElement.elementsByTagName( "MetaContact" );

			for ( int index = 0; index < metaContactList.size(); ++index )
			{
				QDomElement metaContactElement = metaContactList.item( index ).toElement();
				if ( metaContactElement.isNull() )
					continue;

				// Put position into hash.
				QUuid uuid( metaContactElement.attribute( "uuid" ) );
				int metaContactPosition = metaContactElement.attribute( "possition", "-1" ).toInt();
				const Kopete::MetaContact* mc = uuidToMetaContact.value( uuid, 0 );
				if ( mc )
					_metaContactPositionPlain->insert( mc, metaContactPosition );
			}

			qStableSort( m_contacts.begin(), m_contacts.end(), manualMetaContactSort );

			delete _metaContactPositionPlain;
			_metaContactPositionPlain = 0;
		}
	}
}

}

}

#include "contactlistplainmodel.moc"
//kate: tab-width 4
