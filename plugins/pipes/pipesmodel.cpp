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
#include "pipesmodel.h"

#include "pipesdelegate.h"
#include "pipesplugin.h"

#include <kglobal.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocale.h>

PipesModel::PipesModel ( QObject *parent )
		: QAbstractTableModel ( parent )
{
}

int PipesModel::rowCount ( const QModelIndex & /* parent */ ) const
{
	return mPipesList.count();
}

int PipesModel::columnCount ( const QModelIndex & /* parent */ ) const
{
	return PipesDelegate::TotalColumns;
}

QVariant PipesModel::data ( const QModelIndex &index, int role ) const
{
	if ( !index.isValid() )
		return QVariant();
	
	PipesPlugin::PipeOptions pipe = mPipesList.value ( index.row() );

	if ( role == Qt::TextAlignmentRole )
		return (int)(Qt::AlignLeft | Qt::AlignVCenter);
	else if ( role == Qt::CheckStateRole ) {
		if ( index.column() == PipesDelegate::EnabledColumn )
			return ( pipe.enabled ?  Qt::Checked : Qt::Unchecked );
	}
	else if ( role == Qt::DisplayRole )
	{
		if ( !pipe.uid.isNull() )
		{
			if ( index.column() == PipesDelegate::PathColumn )
				return pipe.path;
			else if ( index.column() == PipesDelegate::DirectionColumn )
				return pipe.direction;
			else if ( index.column() == PipesDelegate::ContentsColumn )
				return pipe.pipeContents;
		}
	}
	return QVariant();
}

bool PipesModel::removeRow (int row, const QModelIndex& /* parent */ )
{
	if ( row < mPipesList.size() && row >= 0 ){
		mPipesList.removeAt ( row );
		reset();
		return true;
	}
	return false;
}

QVariant PipesModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
	if ( orientation == Qt::Horizontal )
	{
		if ( role == Qt::DisplayRole )
		{
			if ( section == PipesDelegate::EnabledColumn )
				return QVariant();
			else if ( section == PipesDelegate::PathColumn )
				return i18n ( "Path" );
			else if ( section == PipesDelegate::DirectionColumn )
				return i18n ( "Direction" );
			else if ( section == PipesDelegate::ContentsColumn )
				return i18n ( "Pipe Contents" );
		}
		else if ( role == Qt::TextAlignmentRole )
			return Qt::AlignCenter;
	}
	return QAbstractItemModel::headerData ( section, orientation, role );
}

bool PipesModel::setData ( const QModelIndex &index, const QVariant &value, int role )
{
	if ( index.isValid() ) {
		// basically, the row index is the same as the index in mPipesList, so we can
		// associate these and work from there
		if ( role == Qt::EditRole )
		{
			if ( index.column() == PipesDelegate::PathColumn )
				mPipesList[index.row()].path = value.toString();
			else if ( index.column() == PipesDelegate::DirectionColumn )
				mPipesList[index.row()].direction = (PipesPlugin::PipeDirection)value.toInt();
			else if ( index.column() == PipesDelegate::ContentsColumn )
				mPipesList[index.row()].pipeContents = (PipesPlugin::PipeContents)value.toInt();
			else
				return false;

			emit dataChanged ( index, index );
			return true;
		}
		else if ( role == Qt::CheckStateRole ) {
			if ( index.column() == PipesDelegate::EnabledColumn )
			{
				mPipesList[index.row()].enabled = value.toBool();
				emit dataChanged( index, index );
				return true;
			}
		}
	}
	return false;
}

Qt::ItemFlags PipesModel::flags ( const QModelIndex &index ) const
{
	Qt::ItemFlags flags = QAbstractItemModel::flags ( index );
	// enabled, direction, and contents is editable, while path is not
	if ( index.column() == PipesDelegate::EnabledColumn || index.column() == PipesDelegate::DirectionColumn || index.column() == PipesDelegate::ContentsColumn )
		flags |= Qt::ItemIsEditable;
	return flags;
}

void PipesModel::setPipes ( PipesPlugin::PipeOptionsList pipes )
{
	mPipesList = pipes;
	reset();
}

PipesPlugin::PipeOptionsList PipesModel::pipes ()
{
	return mPipesList;
}

void PipesModel::addPipe ( const PipesPlugin::PipeOptions & pipe )
{
	mPipesList.append ( pipe );
	reset();
}


