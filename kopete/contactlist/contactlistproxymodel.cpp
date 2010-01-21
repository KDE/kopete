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
#include "kopeteappearancesettings.h"
#include "kopeteitembase.h"

namespace Kopete {

namespace UI {

ContactListProxyModel::ContactListProxyModel(QObject* parent)
	: QSortFilterProxyModel(parent), rootRowCount(0), sortScheduled(false)
{
	setDynamicSortFilter(true);
	sort( 0, Qt::AscendingOrder );
	connect ( Kopete::AppearanceSettings::self(), SIGNAL(configChanged()), this, SLOT(slotConfigChanged()) );

	// Workaround Qt sorting bug
	connect( this, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
	         this, SLOT(proxyRowsInserted(const QModelIndex&, int, int)) );
	connect( this, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
	         this, SLOT(proxyRowsRemoved(const QModelIndex&, int, int)) );
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

		// TODO: Find out how to check if we should hide the group if no metaContact was found.
		if ( !filterRegExp().isEmpty() )
			return true;

		if ( !showEmpty && totalContactsCount == 0 )
			return false;

		if ( !showEmpty && !showOffline && connectedContactsCount == 0 )
			return false;

		return true;
	}

	if ( model->data( current, Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
	{
		if ( !filterRegExp().isEmpty() )
		{
			QString mcName = model->data( current, Qt::DisplayRole ).toString();
			return mcName.contains( filterRegExp() );
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

}

}

#include "contactlistproxymodel.moc"
