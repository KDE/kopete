/*
    xtrazstatusmodel.h  -  Xtraz Status Model

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

#ifndef XTRAZSTATUSMODEL_H
#define XTRAZSTATUSMODEL_H

#include <QAbstractTableModel>

#include "xtrazstatus.h"

namespace Xtraz
{

class StatusModel : public QAbstractTableModel
{
public:
	StatusModel( QObject * parent = 0 );

	QList<Xtraz::Status> getStatuses() const;
	void setStatuses( const QList<Xtraz::Status> &statusList );

	virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
	virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

	virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );
	virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

	virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

	virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

	bool swapRows( int i, int j );

	virtual bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() );
	virtual bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );

private:
	QList<Xtraz::Status> mStatuses;

};

}

#endif
