/*
    xtrazstatusmodel.cpp  -  Xtraz Status Model

    Copyright (c) 2007 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "xtrazstatusmodel.h"

#include <kicon.h>
#include <klocale.h>

namespace Xtraz
{

StatusModel::StatusModel( QObject * parent )
: QAbstractTableModel( parent )
{
}

QList<Xtraz::Status> StatusModel::getStatuses() const
{
	return mStatuses;
}

void StatusModel::setStatuses( const QList<Xtraz::Status> &statusList )
{
	mStatuses = statusList;
	reset();
}

int StatusModel::rowCount( const QModelIndex& ) const
{
	return mStatuses.count();
}

int StatusModel::columnCount( const QModelIndex& ) const
{
	return 2;
}

bool StatusModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
	if ( !index.isValid() )
		return false;
	
	if ( index.row() >= mStatuses.count() || index.row() < 0 )
		return false;
	
	if ( role == Qt::EditRole )
	{
		switch ( index.column() )
		{
		case 0:
			mStatuses[index.row()].setDescription( value.toString() );
			return true;
		case 1:
			mStatuses[index.row()].setMessage( value.toString() );
			return true;
		}
	}
	else if ( role == Qt::UserRole )
	{
		if ( index.column() == 0 )
		{
			mStatuses[index.row()].setStatus( value.toInt() );
			return true;
		}
	}
	return false;
}

QVariant StatusModel::data( const QModelIndex &index, int role ) const
{
	if ( !index.isValid() )
		return QVariant();

	if ( index.row() >= mStatuses.count() || index.row() < 0 )
		return QVariant();

	Xtraz::Status status = mStatuses.at(index.row());

	if ( role == Qt::DisplayRole )
	{
		switch ( index.column() )
		{
		case 0:
			return status.description();
		case 1:
			return status.message();
		}
	}
	else if ( role == Qt::UserRole )
	{
		if ( index.column() == 0 )
			return status.status();
	}
	else if ( role == Qt::DecorationRole )
	{
		if ( index.column() == 0 )
		{
			return KIcon( QString( "icq_xstatus%1" ).arg( status.status() ) );
		}
	}

	return QVariant();
}


QVariant StatusModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if ( orientation == Qt::Vertical && role == Qt::DisplayRole )
		return (section + 1);
	
	if ( role == Qt::DisplayRole )
	{
		switch ( section )
		{
		case 0:
			return i18n( "Description" );
		case 1:
			return i18n( "Message" );
			break;
		default:
			return QVariant();
		}
	}
	
	return QVariant();
}

Qt::ItemFlags StatusModel::flags( const QModelIndex &index ) const
{
	if ( !index.isValid() )
		return Qt::ItemIsEnabled;
	
	if ( index.row() >= mStatuses.count() || index.row() < 0 )
		return Qt::ItemIsEnabled;

	return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
}

bool StatusModel::swapRows( int i, int j )
{
	if ( i == j || i < 0 || mStatuses.count() <= i ||
	     j < 0 || mStatuses.count() <= j )
		return false;
	
	mStatuses.swap( i, j );
	emit dataChanged( index( qMin( i, j ), 0 ), index( qMax( i, j ), columnCount() ) );
	return true;
}

bool StatusModel::insertRows( int row, int count, const QModelIndex &parent )
{
	if ( row > mStatuses.count() || row < 0 )
		return false;

	beginInsertRows( parent, row, row + count - 1 );

	for ( int i = 0; i < count; i++ )
		mStatuses.insert( row, Xtraz::Status() );

	endInsertRows();
	return true;
}

bool StatusModel::removeRows( int row, int count, const QModelIndex &parent )
{
	if ( row >= mStatuses.count() || row < 0 || row + count > mStatuses.count() )
		return false;
	
	beginRemoveRows( parent, row, row + count - 1 );

	for ( int i = 0; i < count; i++ )
		mStatuses.removeAt( row );

	endRemoveRows();
	return true;
}

}
