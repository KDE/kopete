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

#include "kopetegroup.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopeteappearancesettings.h"
#include "kopeteitembase.h"

namespace Kopete {

namespace UI {

ContactListProxyModel::ContactListProxyModel(QObject* parent)
	: QSortFilterProxyModel(parent)
{
	setDynamicSortFilter(true);
	sort( 0, Qt::AscendingOrder );
	connect ( Kopete::AppearanceSettings::self(), SIGNAL(configChanged()), this, SLOT(slotConfigChanged()) );
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
// 	Kopete::MetaContact* mc = metaContactFromIndex( current );
	bool showEmpty = Kopete::AppearanceSettings::self()->showEmptyGroups();
	bool showOffline = Kopete::AppearanceSettings::self()->showOfflineUsers();

	if ( model->data( current, Kopete::Items::TypeRole ) == Kopete::Items::Group )
	{
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

		int mcStatus = model->data( current, Kopete::Items::OnlineStatusRole ).toInt();
		if ( mcStatus <= OnlineStatus::Offline && !showOffline )
			return false;
		else
			return true;
	}

	return false;
}

}

}

#include "contactlistproxymodel.moc"
