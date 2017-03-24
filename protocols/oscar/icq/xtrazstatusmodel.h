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
	StatusModel( QObject * parent = nullptr );

	QList<Xtraz::Status> getStatuses() const;
	void setStatuses( const QList<Xtraz::Status> &statusList );

	int rowCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
	int columnCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;

	bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) Q_DECL_OVERRIDE;
	QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const Q_DECL_OVERRIDE;

	QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const Q_DECL_OVERRIDE;

	Qt::ItemFlags flags( const QModelIndex &index ) const Q_DECL_OVERRIDE;

	bool swapRows( int i, int j );

	bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() ) Q_DECL_OVERRIDE;
	bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) Q_DECL_OVERRIDE;

private:
	QList<Xtraz::Status> mStatuses;

};

}

#endif
