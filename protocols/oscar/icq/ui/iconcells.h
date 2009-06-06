/*
    iconcells.h  -  Icon Cells

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

#ifndef ICONCELLS_H
#define ICONCELLS_H

#include <QTableWidget>

/**
* A table of icons cells.
*
* @author Roman Jarosz <kedgedev@centrum.cz>
*/

class IconCells : public QTableWidget
{
	Q_OBJECT
public:
	/**
	 * Constructs a new table of icon cells
	 *
	 * @param parent The parent of the new widget
	 */
	IconCells( QWidget *parent );

	~IconCells();

	/** Sets the icons */
	void setIcons( const QList<QIcon> &icons );

	/** Returns the icon at a given index in the table */
	QIcon icon( int index ) const;

	/** Returns the total number of icon cells in the table */
	int count() const;

	/** Sets the currently selected cell to @p index */
	void setSelectedIndex( int index );

	/** Returns the index of the cell which is currently selected */
	int selectedIndex() const;

	virtual QSize sizeHint () const;

signals:
	/** Emitted when a icon is selected in the table */
	void selected( int index );

protected slots:
	void selected( int row, int column );

protected:
	// the three methods below are used to ensure equal column widths and row heights
	// for all cells and to update the widths/heights when the widget is resized
	virtual void resizeEvent( QResizeEvent* event );
	virtual int sizeHintForColumn( int column ) const;
	virtual int sizeHintForRow( int row ) const;

	int rowFromIndex( int index ) const;
	int columnFromIndex( int index ) const;

private:
	class IconCellsPrivate;
	IconCellsPrivate *const d;
	
	Q_DISABLE_COPY(IconCells)
};

#endif
