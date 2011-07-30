/*
    iconcells.cpp  -  Icon Cells

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

#include "iconcells.h"

#include <math.h>

#include <QList>
#include <QHeaderView>

class IconCells::IconCellsPrivate
{
public:
	IconCellsPrivate()
	{
		selected = -1;
	}

	QList<QIcon> icons;
	int selected;
};

IconCells::IconCells( QWidget *parent )
: QTableWidget( parent ), d(new IconCellsPrivate())
{
	setColumnCount( 0 );
	setRowCount( 0 );

	verticalHeader()->hide();
	horizontalHeader()->hide();

	d->selected = 0;
	int pm = style()->pixelMetric( QStyle::PM_SmallIconSize, 0, this );
	setIconSize( QSize( pm, pm ) );

	setSelectionMode( QAbstractItemView::SingleSelection );
	setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	viewport()->setBackgroundRole( QPalette::Background );
	setBackgroundRole( QPalette::Background );

	// HACK: Looks like any antialiased font brakes grid and icon painting.
	// We only have icons so we can set the font to one which isn't antialiased.
	QFont timesFont( "Times", 10, QFont::Normal );
	setFont( timesFont );

	connect( this, SIGNAL(cellActivated(int,int)), this, SLOT(selected(int,int)) );
	connect( this, SIGNAL(cellPressed(int,int)), this, SLOT(selected(int,int)) );
}

IconCells::~IconCells()
{
	delete d;
}

QIcon IconCells::icon( int index ) const
{
	return d->icons.at( index );
}

int IconCells::count() const
{
	return d->icons.count();
}

void IconCells::setSelectedIndex( int index )
{
	Q_ASSERT( index >= 0 && index < d->icons.count() );

	d->selected = index;

	const int column = columnFromIndex( index );
	const int row = rowFromIndex( index );
	setCurrentCell( row, column );
}

int IconCells::selectedIndex() const
{
	return d->selected;
}

QSize IconCells::sizeHint () const
{
	int width = columnCount() * (iconSize().width() + 8) + 2 * frameWidth();
	int height = rowCount() * (iconSize().height() + 8) + 2 * frameWidth();
	return QSize( width, height );
}

void IconCells::setIcons( const QList<QIcon> &icons )
{
	d->icons = icons;
	setRowCount( (int)ceil( (double)d->icons.size() / columnCount() ) );

	for ( int row = 0; row < rowCount(); ++row )
	{
		for ( int column = 0; column < columnCount(); ++column )
		{
			int index = row * columnCount() + column;
			QTableWidgetItem* tableItem = item( row, column );

			if ( tableItem == 0 )
			{
				tableItem = new QTableWidgetItem();
				tableItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
				setItem( row, column, tableItem );
			}

			if ( index < d->icons.count() )
			{
				QIcon icon = d->icons.at(index);
				tableItem->setData( Qt::DecorationRole, icon );
			}
		}
	}
	setMinimumSize( sizeHint() );
}

void IconCells::selected( int row, int column )
{
	int index = row * columnCount() + column;
	
	if ( index < d->icons.count() )
	{
		d->selected = index;
		emit selected( index );
	}
}

void IconCells::resizeEvent( QResizeEvent* )
{
	// iterate over each column and each row and resize them to fit their contents.
	// resizeColumnsToContents() and resizeRowsToContents() are not used because
	// of 150382 bug in Qt 4.2 where the aforementioned methods do not ignore the height/width
	// of hidden headers as they should do.

	for ( int index = 0 ; index < columnCount() ; index++ )
		resizeColumnToContents(index);
	for ( int index = 0 ; index < rowCount() ; index++ )
		resizeRowToContents(index);

	// use the method below when the bug is fixed:
	//resizeColumnsToContents();
	//resizeRowsToContents();
}

int IconCells::sizeHintForColumn(int /*column*/) const
{
	return (int)floor((double)(width() - 2 * frameWidth()) / columnCount());
}

int IconCells::sizeHintForRow(int /*row*/) const
{
	return (int)floor((double)(height() - 2 * frameWidth()) / rowCount());
}

int IconCells::rowFromIndex( int index ) const
{
	return (int)floor( (double)index / columnCount() );
}

int IconCells::columnFromIndex( int index ) const
{
	return index % columnCount();
}

#include "iconcells.moc"
