/*
    pipesmodel.cpp

    Copyright (c) 2007      by Charles Connell <charles@connells.org>

    Kopete    (c) 2007      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef PIPESMODEL_H
#define PIPESMODEL_H

#include "pipesdelegate.h"
#include "pipesplugin.h"

#include <QAbstractTableModel>

#include <QUuid>

/**
 * Model representing pipes' configurations
 * @author Charles Connell <charles@connells.org>
 */

class PipesModel : public QAbstractTableModel
{
		Q_OBJECT

	public:
		PipesModel ( QObject *parent = 0 );

		int rowCount ( const QModelIndex &parent ) const;
		int columnCount ( const QModelIndex &parent ) const;
		QVariant data ( const QModelIndex &index, int role = Qt::DisplayRole ) const;
		bool setData ( const QModelIndex &index, const QVariant &value, int role );
		bool removeRow ( int row, const QModelIndex& parent );
		QVariant headerData ( int section, Qt::Orientation orientation, int role ) const;
		Qt::ItemFlags flags ( const QModelIndex &index ) const;
		
		void setPipes (PipesPlugin::PipeOptionsList pipes);
		PipesPlugin::PipeOptionsList pipes();
		void addPipe ( const PipesPlugin::PipeOptions& pipe);

	private:
		PipesPlugin::PipeOptionsList mPipesList;
};


#endif

