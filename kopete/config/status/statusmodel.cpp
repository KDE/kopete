/*
    kopetestatusmodel.cpp - Kopete Status Model

    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "statusmodel.h"

#include <klocale.h>
#include <kicon.h>

#include <QtCore/QMimeData>
#include <QtXml/QDomDocument>

#include "kopetestatusitems.h"
#include "kopetestatusmanager.h"

KopeteStatusModel::KopeteStatusModel( Kopete::Status::StatusGroup *rootItem, QObject *parent )
: QAbstractItemModel( parent ), mRootItem( rootItem )
{
}

KopeteStatusModel::~KopeteStatusModel()
{
}

QVariant KopeteStatusModel::data( const QModelIndex &index, int role ) const
{
	if ( !index.isValid() )
		return QVariant();

	QVariant result;

	switch ( role )
	{
	case Qt::DisplayRole:
		if ( index.column() == 0 )
			result = getStatusItem( index )->title();
		break;
	case Qt::DecorationRole:
		if ( index.column() == 0 )
			result = Kopete::OnlineStatusManager::pixmapForCategory( getStatusItem( index )->category() );
		break;
	case KopeteStatusModel::Group:
		result = getStatusItem( index )->isGroup();
		break;
	case KopeteStatusModel::Category:
		result = (int)getStatusItem( index )->category();
		break;
	case KopeteStatusModel::Title:
		result = getStatusItem( index )->title();
		break;
	case KopeteStatusModel::Message:
		{
			Kopete::Status::Status *s = getStatus( getStatusItem( index ) );
			if ( s )
				result = s->message();
		}
		break;
	default:
		return result;
	}

	return result;
}

bool KopeteStatusModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
	if ( !index.isValid() )
		return false;

	switch ( role )
	{
	case KopeteStatusModel::Category:
		getStatusItem( index )->setCategory( (Kopete::OnlineStatusManager::Categories)value.toInt() );
		break;
	case KopeteStatusModel::Title:
		getStatusItem( index )->setTitle( value.toString() );
		break;
	case KopeteStatusModel::Message:
		{
			Kopete::Status::Status *s = getStatus( getStatusItem( index ) );
			if ( !s )
				return false;

			s->setMessage( value.toString() );
		}
		break;
	default:
		return false;
	}

	emit dataChanged( index, index );
	emit changed();
	return true;
}


Qt::ItemFlags KopeteStatusModel::flags( const QModelIndex &index ) const
{
	if ( !index.isValid() )
		return Qt::ItemIsDropEnabled;

	if ( getStatusItem( index )->isGroup() )
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
	else
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
}

QVariant KopeteStatusModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if ( orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0 )
		return i18n( "Title" );

	return QVariant();
}

QModelIndex KopeteStatusModel::index( int row, int column, const QModelIndex &parent ) const
{
	if ( !hasIndex( row, column, parent ) )
		return QModelIndex();

	Kopete::Status::StatusItem *childItem = getStatusItem( parent )->child( row );
	if ( childItem )
		return createIndex( row, column, childItem );
	else
		return QModelIndex();
}

QModelIndex KopeteStatusModel::parent( const QModelIndex &index ) const
{
	if ( !index.isValid() )
		return QModelIndex();

	Kopete::Status::StatusItem *parentItem = getStatusItem( index )->parentGroup();
	if ( parentItem == mRootItem )
		return QModelIndex();

	return createIndex( parentItem->index(), 0, parentItem );
}

int KopeteStatusModel::rowCount( const QModelIndex &parent ) const
{
	if ( parent.column() > 0 )
		return 0;

	return getStatusItem( parent )->childCount();
}

int KopeteStatusModel::columnCount( const QModelIndex & ) const
{
	return 1;
}

Qt::DropActions KopeteStatusModel::supportedDropActions() const
{
	return Qt::MoveAction;
}

QModelIndex KopeteStatusModel::insertItem( const QModelIndex &index, Kopete::Status::StatusItem *item )
{
	int row = 0;
	QModelIndex parentIndex;
	if ( index.isValid() )
	{
		// Don't create nasted groups
		if ( getStatusItem( index )->isGroup() && !item->isGroup() )
		{
			parentIndex = index;
		}
		else
		{
			parentIndex = index.parent();
			row = index.row() + 1;
		}
	}

	Kopete::Status::StatusGroup *group = getGroup( getStatusItem( parentIndex ) );
	if ( !group )
		return QModelIndex();

	emit layoutAboutToBeChanged();
	beginInsertRows( parentIndex, row, row );

	group->insertChild( row, item );

	endInsertRows();
	emit layoutChanged();

	emit changed();
	return this->index(row, 0, parentIndex);
}

bool KopeteStatusModel::removeRows( int row, int count, const QModelIndex &parent )
{
	if ( count == 0 )
		return false;

	Kopete::Status::StatusGroup *group = getGroup( getStatusItem( parent ) );
	if ( !group )
		return false;

	emit layoutAboutToBeChanged();
	beginRemoveRows( parent, row, row + count - 1 );

	while( (count--) > 0 )
		delete group->child( row );

	endRemoveRows();
	emit layoutChanged();
	emit changed();
	return true;
}

QStringList KopeteStatusModel::mimeTypes() const
{
	QStringList types;
	types << "application/xml-kopete-status";
	return types;
}

QMimeData *KopeteStatusModel::mimeData( const QModelIndexList &indexes ) const
{
	using namespace Kopete;
	QMimeData *mimeData = new QMimeData();
	QByteArray encodedData;

	QDataStream stream( &encodedData, QIODevice::WriteOnly );

	foreach ( const QModelIndex &index, indexes )
	{
		if ( index.isValid() && index.column() == 0 )
		{
			Status::StatusItem *item = getStatusItem( index );
			QDomDocument doc( QString::fromLatin1( "kopete-status" ) );
			doc.appendChild( StatusManager::storeStatusItem( item ) );
			stream << doc.toString();
		}
	}

	mimeData->setData( "application/xml-kopete-status", encodedData );
	return mimeData;
}


bool KopeteStatusModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
	if ( action == Qt::IgnoreAction )
		return true;

	if ( !data->hasFormat( "application/xml-kopete-status" ) )
		return false;
	
	if ( column > 0 )
		return false;

	int beginRow;

	if ( row != -1 )
		beginRow = row;
	else if ( parent.isValid() )
		beginRow = parent.row();
	else
		beginRow = rowCount( QModelIndex() );

	QByteArray encodedData = data->data( "application/xml-kopete-status" );
	QDataStream stream( &encodedData, QIODevice::ReadOnly );

	using namespace Kopete;
	Status::StatusGroup *parentItem = getGroup( getStatusItem( parent ) );
	if ( !parentItem )
		return false;

	QStringList newItems;
	int rows = 0;

	while ( !stream.atEnd() ) {
		QString text;
		stream >> text;
		newItems << text;
		++rows;
	}

	emit layoutAboutToBeChanged();
	for ( int i = 0; i < newItems.size(); ++i )
	{
		QDomDocument doc;
		doc.setContent( newItems.at(i) );
		if ( !doc.isNull() )
		{
			Status::StatusItem *item = StatusManager::parseStatusItem( doc.documentElement() );
			
			QDomDocument doc2( QString::fromLatin1( "kopete-status" ) );
			doc2.appendChild( StatusManager::storeStatusItem( item ) );

			// Don't create nasted groups
			if ( item->isGroup() && parentItem != mRootItem )
			{
				int parentRow = parent.row();
				beginInsertRows( parent.parent(), parentRow, parentRow );
				parentItem->parentGroup()->insertChild( parentRow, item );
				endInsertRows();
			}
			else
			{
				beginInsertRows( parent, beginRow, beginRow );
				parentItem->insertChild( beginRow++, item );
				endInsertRows();
			}
		}
	}
	emit layoutChanged();
	emit changed();
	return true;
}

Kopete::Status::StatusItem *KopeteStatusModel::getStatusItem( const QModelIndex &index ) const
{
	if ( !index.isValid() )
		return mRootItem;

	return static_cast<Kopete::Status::StatusItem*>( index.internalPointer() );
}

Kopete::Status::Status *KopeteStatusModel::getStatus( Kopete::Status::StatusItem *item ) const
{
	if ( !item )
		return 0;
	
	return qobject_cast<Kopete::Status::Status*>( item );
}

Kopete::Status::StatusGroup *KopeteStatusModel::getGroup( Kopete::Status::StatusItem *item ) const
{
	if ( !item )
		return 0;
	
	return qobject_cast<Kopete::Status::StatusGroup*>( item );
}

#include "statusmodel.moc"
