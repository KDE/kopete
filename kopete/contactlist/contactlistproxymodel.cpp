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

#include "contactlistproxymodel.h"

#include <QStandardItem>
#include <QList>
#include <QTimer>

#include "kopetegroup.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetecontact.h"
#include "kopeteappearancesettings.h"
#include "kopeteitembase.h"
#include <kabc/stdaddressbook.h>

namespace Kopete {

namespace UI {

ContactListProxyModel::ContactListProxyModel(QObject* parent)
	: QSortFilterProxyModel(parent), rootRowCount(0), sortScheduled(false)
{
	setDynamicSortFilter(true);
	sort( 0, Qt::AscendingOrder );
	connect ( Kopete::AppearanceSettings::self(), SIGNAL(configChanged()), this, SLOT(slotConfigChanged()) );

	// Workaround Qt sorting bug
	connect( this, SIGNAL(rowsInserted(QModelIndex,int,int)),
	         this, SLOT(proxyRowsInserted(QModelIndex,int,int)) );
	connect( this, SIGNAL(rowsRemoved(QModelIndex,int,int)),
	         this, SLOT(proxyRowsRemoved(QModelIndex,int,int)) );
	connect( this, SIGNAL(modelReset()),
	         this, SLOT(proxyCheckSort()) );
	connect( this, SIGNAL(layoutChanged()),
	         this, SLOT(proxyCheckSort()) );
	
}

ContactListProxyModel::~ContactListProxyModel()
{

}

void ContactListProxyModel::slotConfigChanged()
{
	kDebug(14001) << "config changed";
	invalidate();
}

bool ContactListProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	int leftType = left.data( Kopete::Items::TypeRole ).toInt();
	if ( leftType != right.data( Kopete::Items::TypeRole ).toInt() )
		return (leftType == Kopete::Items::MetaContact); // MetaContacts are always on top.

	if ( leftType == Kopete::Items::Group )
	{
		switch ( Kopete::AppearanceSettings::self()->contactListGroupSorting() )
		{
		case Kopete::AppearanceSettings::EnumContactListGroupSorting::Manual:
			return left.row() < right.row();
		case Kopete::AppearanceSettings::EnumContactListGroupSorting::Name:
		default:
			QString leftName = left.data( Qt::DisplayRole ).toString();
			QString rightName = right.data( Qt::DisplayRole ).toString();

			// Force the offline group to the bottom
			QObject* groupObjectLeft = qVariantValue<QObject*>( sourceModel()->data( left, Kopete::Items::ObjectRole ) );
			QObject* groupObjectRight = qVariantValue<QObject*>( sourceModel()->data( right, Kopete::Items::ObjectRole ) );

			if ( groupObjectLeft == Kopete::Group::offline() )
				return false;
			else if ( groupObjectRight == Kopete::Group::offline() )
				return true;
			return QString::localeAwareCompare( leftName, rightName ) < 0;
		}
	}
	else if ( leftType == Kopete::Items::MetaContact )
	{
		switch ( Kopete::AppearanceSettings::self()->contactListMetaContactSorting() )
		{
		case Kopete::AppearanceSettings::EnumContactListMetaContactSorting::Manual:
			return left.row() < right.row();
		case Kopete::AppearanceSettings::EnumContactListMetaContactSorting::Status:
		{
			int leftStatus = left.data( Kopete::Items::OnlineStatusRole ).toInt();
			int rightStatus = right.data( Kopete::Items::OnlineStatusRole ).toInt();
			if ( leftStatus != rightStatus )
			{
				return !(leftStatus < rightStatus);
			}
			else
			{
				QString leftName = left.data( Qt::DisplayRole ).toString();
				QString rightName = right.data( Qt::DisplayRole ).toString();
				return (QString::localeAwareCompare( leftName, rightName ) < 0);
			}
		}
		case Kopete::AppearanceSettings::EnumContactListMetaContactSorting::Name:
		default:
			QString leftName = left.data( Qt::DisplayRole ).toString();
			QString rightName = right.data( Qt::DisplayRole ).toString();
			return (QString::localeAwareCompare( leftName, rightName ) < 0);
		}
	}

	return false;
}

bool ContactListProxyModel::filterAcceptsRow ( int sourceRow, const QModelIndex & sourceParent ) const
{
	QAbstractItemModel* model = sourceModel();
	QModelIndex current = model->index(sourceRow, 0, sourceParent);
	bool showEmpty = Kopete::AppearanceSettings::self()->showEmptyGroups();
	bool showOffline = Kopete::AppearanceSettings::self()->showOfflineUsers();

	if ( model->data( current, Kopete::Items::TypeRole ) == Kopete::Items::Group )
	{
		QObject* groupObject = qVariantValue<QObject*>( model->data( current, Kopete::Items::ObjectRole ) );
		if ( qobject_cast<Kopete::Group*>(groupObject) == Kopete::Group::topLevel() )
			return true;

		int connectedContactsCount = model->data( current, Kopete::Items::ConnectedCountRole ).toInt();
		int totalContactsCount = model->data( current, Kopete::Items::TotalCountRole ).toInt();

		bool isOfflineGroup = ( groupObject == Kopete::Group::offline() );
		if ( !filterRegExp().isEmpty() && isOfflineGroup )
			return false;

		if ( !filterRegExp().isEmpty() )
		{
			// This shows or hides the contacts group folder if something was found.
			// Walk through the group's metacontacts and see it the search result was found
			// If it is found then we can show this group folder.
			// This is fairly slow with > group 10000 contacts. Reasonable? (ginge)
			for ( int i = 0; i < model->rowCount( current ); i++ )
			{
				QModelIndex qmi = model->index( i, 0, current );

				if ( model->data( qmi, Kopete::Items::TypeRole ) != Kopete::Items::MetaContact )
					continue;

				QObject* mcObject = qVariantValue<QObject*>( model->data( qmi, Kopete::Items::ObjectRole ) );
				Kopete::MetaContact *mc = qobject_cast<Kopete::MetaContact*>( mcObject );

				// Do a better search.
				if ( searchContactInfo( mc, filterRegExp() ) )
				{
					qobject_cast<Kopete::Group*>(groupObject)->setExpanded(true);
					return true;
				}
			}

			return false;
		}


		if ( !Kopete::AppearanceSettings::self()->showOfflineGrouped() && isOfflineGroup )
			return false;

		if ( !showEmpty && totalContactsCount == 0 && !isOfflineGroup)
			return false;

		// do not display offline when viewing in grouped offline mode
		if ( showOffline && isOfflineGroup )
			return false;

		if ( !showEmpty && !showOffline && connectedContactsCount == 0 && !isOfflineGroup )
			return false;

		return true;
	}

	if ( model->data( current, Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
	{
		if ( !filterRegExp().isEmpty() )
		{
			QObject* contactObject = qVariantValue<QObject*>( model->data( current, Kopete::Items::ObjectRole ) );
			Kopete::MetaContact *mc = qobject_cast<Kopete::MetaContact*>(contactObject);

			// Do a better search
			return searchContactInfo( mc, filterRegExp() );
		}

		// Get the MetaContacts parent group name
		QObject* groupObject = qVariantValue<QObject*>( model->data( sourceParent, Kopete::Items::ObjectRole ) );

		if ( Kopete::AppearanceSettings::self()->groupContactByGroup() && qobject_cast<Kopete::Group*>(groupObject) != 0 )
		{
			// If this contact's group is called Offline, and we are not globally 
			// showing offline all users show the offline group folder
			if ( groupObject == Kopete::Group::offline() )
				return !showOffline;
		}

		bool alwaysVisible = model->data( current, Kopete::Items::AlwaysVisible ).toBool();
		int mcStatus = model->data( current, Kopete::Items::OnlineStatusRole ).toInt();
		if ( mcStatus <= OnlineStatus::Offline && !showOffline && !alwaysVisible )
			return false;
		else
			return true;
	}

	return false;
}

void ContactListProxyModel::proxyRowsInserted( const QModelIndex& parent, int start, int end )
{
	if (parent.isValid())
		return;
	
	int count = (end - start) + 1;
	if (rootRowCount <= 0 && count > 0 && !sortScheduled)
	{
		sortScheduled = true;
		QTimer::singleShot( 0, this, SLOT(forceSort()) );
	}
	rootRowCount += count;
}

void ContactListProxyModel::proxyRowsRemoved( const QModelIndex& parent, int start, int end )
{
	if (parent.isValid())
		return;
	
	int count = (end - start) + 1;
	rootRowCount -= count;
}

void ContactListProxyModel::proxyCheckSort()
{
	int count = rowCount();
	if (rootRowCount <= 0 && count > 0 && !sortScheduled)
	{
		sortScheduled = true;
		QTimer::singleShot( 0, this, SLOT(forceSort()) );
	}
	rootRowCount = count;
}

void ContactListProxyModel::forceSort()
{
	if (!sortScheduled)
		return;

	sortScheduled = false;
	sort( -1, Qt::AscendingOrder );
	sort( 0, Qt::AscendingOrder );
}

// Better search. Now a search will look in the metacontact display name, the invidual account contact names and any email addresses.
bool ContactListProxyModel::searchContactInfo(Kopete::MetaContact *mc, QRegExp searchPattern) const
{
	// Check the display name
	if ( mc->displayName().contains( searchPattern ) )
		return true;

	// Check the address book
	KABC::Addressee addressee = KABC::StdAddressBook::self()->findByUid( mc->kabcId() );
	if ( !addressee.isEmpty() )
	{
		QString emailAddr = addressee.fullEmail();

		if ( emailAddr.contains( searchPattern ) )
			return true;
	}

	// Check alternative names
	foreach( Kopete::Contact* c , mc->contacts() )
	{
		// Search each metacontacts' contacts
		if ( c->contactId().contains( searchPattern ) )
			return true;
	}

	return false;
}

}

}

#include "contactlistproxymodel.moc"
