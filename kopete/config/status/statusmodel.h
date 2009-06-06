/*
    kopetestatusmodel.h - Kopete Status Model

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
#ifndef KOPETESTATUSMODEL_H
#define KOPETESTATUSMODEL_H

#include <QAbstractItemModel>

namespace Kopete {
	namespace Status {
		class StatusItem;
		class StatusGroup;
		class Status;
	}
}

/**
	@author Roman Jarosz <kedgedev@centrum.cz>
*/

class KopeteStatusModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	KopeteStatusModel( Kopete::Status::StatusGroup *rootItem, QObject *parent = 0 );
	~KopeteStatusModel();

	enum ItemRole
	{
		Category = Qt::UserRole,
		Title,
		Message,
		Group
	};

	virtual QVariant data( const QModelIndex &index, int role ) const;
	virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );
	virtual Qt::ItemFlags flags( const QModelIndex &index ) const;
	virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
	virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
	virtual QModelIndex parent( const QModelIndex &index ) const;
	virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
	virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

	virtual Qt::DropActions supportedDropActions() const;

	QModelIndex insertItem( const QModelIndex &index, Kopete::Status::StatusItem *item );
	virtual bool removeRows ( int row, int count, const QModelIndex &parent = QModelIndex() );

	QStringList mimeTypes() const;
	QMimeData *mimeData( const QModelIndexList &indexes ) const;
	bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );

Q_SIGNALS:
	void changed();

private:
	Kopete::Status::StatusItem *getStatusItem( const QModelIndex &parent ) const;
	Kopete::Status::Status *getStatus( Kopete::Status::StatusItem *item ) const;
	Kopete::Status::StatusGroup *getGroup( Kopete::Status::StatusItem *item ) const;
	
	Kopete::Status::StatusGroup *mRootItem;
};

#endif
