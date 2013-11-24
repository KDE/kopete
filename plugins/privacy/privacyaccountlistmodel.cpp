/*
    privacyaccountlist.cpp - a list of accounts that are part of the black/whitelist

    Copyright (c) 2006 by Andre Duffeck             <duffeck@kde.org>
    Kopete    (c) 2003-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "privacyaccountlistmodel.h"

#include <QStringList>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

#include "kopeteprotocol.h"
#include "kopetepluginmanager.h"

PrivacyAccountListModel::PrivacyAccountListModel( QObject *parent )
	: QAbstractTableModel(parent)
{
}

PrivacyAccountListModel::~PrivacyAccountListModel()
{
}

void PrivacyAccountListModel::loadAccounts( const QStringList &accounts )
{
	m_list.clear();
	beginInsertRows(QModelIndex(), 0, accounts.size());
	foreach( const QString &entry, accounts )
	{
		Kopete::Plugin *protocol = Kopete::PluginManager::self()->plugin( entry.split(':')[0] );
		if( !protocol )
			continue;

		m_list.append( AccountListEntry( entry.split(':')[1], (Kopete::Protocol *)protocol ) );
	}
	endInsertRows();
}

void PrivacyAccountListModel::addAccount(const QString &accountId, Kopete::Protocol *protocol)
{
	beginInsertRows(QModelIndex(), m_list.size(), m_list.size()+1);

	m_list.append( AccountListEntry( accountId, protocol ) );

	endInsertRows();
}

void PrivacyAccountListModel::addAccount(const AccountListEntry &entry)
{
	addAccount( entry.first, entry.second );
}

int PrivacyAccountListModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return m_list.count();
}

int PrivacyAccountListModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 2;
}

QVariant PrivacyAccountListModel::data(const QModelIndex &index, int role) const
{
	if( !index.isValid() )
		return QVariant();
	
	if( index.row() >= m_list.size() )
		return QVariant();
		
	if( index.column() > 2 )
		return QVariant();

	if( role == Qt::DisplayRole && index.column() == 0 )
	{
		return m_list[index.row()].first;
	}
	else if( role == Qt::DecorationRole && index.column() == 1 )
	{
		return SmallIcon( m_list[index.row()].second->pluginIcon() );
	}
	else
		return QVariant();
}

bool PrivacyAccountListModel::removeRow(int position, const QModelIndex &index)
{
	Q_UNUSED(index);
	beginRemoveRows(QModelIndex(), position, position);
	
	m_list.removeAt(position);
	
	endRemoveRows();
	return true;
}

bool PrivacyAccountListModel::removeRows(int position, int rows, const QModelIndex &index)
{
	Q_UNUSED(index);
	beginRemoveRows(QModelIndex(), position, position+rows-1);
	
	for (int row = 0; row < rows; ++row)
	{
		m_list.removeAt(position);
	}
	
	endRemoveRows();
	return true;
}

QStringList PrivacyAccountListModel::toStringList() const
{
	QStringList list;
	foreach( const AccountListEntry &entry, m_list )
	{
		list.append( entry.second->pluginId() + ':' + entry.first );
	}
	
	return list;
}

#include "privacyaccountlistmodel.moc"
