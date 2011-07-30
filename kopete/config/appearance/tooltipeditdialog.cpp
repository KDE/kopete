/*
    tooltipeditdialog.cpp  -  Kopete Tooltip Editor

    Copyright (c) 2004 by Stefan Gehn <metz@gehn.net>

    Kopete    (c) 2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "tooltipeditdialog.h"

#include "kopeteproperty.h"
#include "kopeteglobal.h"
#include "kopeteappearancesettings.h"

#include <QStringList>
#include <QApplication>
#include <QToolButton>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include <kiconloader.h>
#include <klocale.h>

TooltipEditDialog::TooltipEditDialog(QWidget *parent)
	: KDialog(parent)
{
	setCaption( i18n("Tooltip Editor") );
	setButtons( KDialog::Ok | KDialog::Cancel );
	setDefaultButton(KDialog::Ok);
	showButtonSeparator(true);

	mMainWidget = new QWidget(this);
	mMainWidget->setObjectName("TooltipEditDialog::mMainWidget");
	setupUi(mMainWidget);

	setMainWidget(mMainWidget);

	/*
	 * Fill the model with the appropriates entries (pairs label/internal string)
	 */
	mUnusedEntries = new QStandardItemModel(this);
	mUsedEntries = new QStandardItemModel(this);

	const Kopete::PropertyTmpl::Map propmap(
		Kopete::Global::Properties::self()->templateMap());
	const QStringList usedKeys = Kopete::AppearanceSettings::self()->toolTipContents();

	// first fill the "used" list
	foreach(const QString &usedProp, usedKeys)
	{
		if(propmap.contains(usedProp) && !propmap[usedProp].isPrivate())
		{
			QStandardItem *item = new QStandardItem( propmap[usedProp].label() );
			item->setData( usedProp );
			mUsedEntries->appendRow( item );
		}
	}

	// then iterate over all known properties and insert the remaining ones
	// into the "unused" list
	Kopete::PropertyTmpl::Map::ConstIterator it;
	for(it = propmap.begin(); it != propmap.end(); ++it)
	{
		if((usedKeys.contains(it.key())==0) && (!it.value().isPrivate()))
		{
			QStandardItem *item = new QStandardItem( it.value().label() );
			item->setData( it.key() );
			mUnusedEntries->appendRow( item );
		}
	}

	// We use a proxy for mUnusedEntries because it needs to be alphabetically sorted
	QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel( this );
	proxyModel->setSourceModel( mUnusedEntries );
	proxyModel->sort( 0, Qt::AscendingOrder );
	unusedItemsListView->setModel( proxyModel );
	usedItemsListView->setModel( mUsedEntries );

	/*
	 * Ui setup
	 */
	connect(unusedItemsListView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
				this, SLOT(slotUnusedSelected(QItemSelection)));
	connect(usedItemsListView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
		this, SLOT(slotUsedSelected(QItemSelection)));
	connect(unusedItemsListView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotAddButton()));
	connect(usedItemsListView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotRemoveButton()));

	tbUp->setIcon(KIcon("go-up"));
	tbUp->setEnabled(false);
	tbUp->setAutoRepeat(true);
	connect(tbUp, SIGNAL(clicked()), SLOT(slotUpButton()));

	tbDown->setIcon(KIcon("go-down"));
	tbDown->setEnabled(false);
	tbDown->setAutoRepeat(true);
	connect(tbDown, SIGNAL(clicked()), SLOT(slotDownButton()));

	KIcon left = KIcon("go-previous");
	KIcon right = KIcon("go-next");

	tbAdd->setIcon(QApplication::isRightToLeft() ? left : right);
	tbAdd->setEnabled(false);
	connect(tbAdd, SIGNAL(clicked()), SLOT(slotAddButton()));

	tbRemove->setIcon(QApplication::isRightToLeft() ? right : left);
	tbRemove->setEnabled(false);
	connect(tbRemove, SIGNAL(clicked()), SLOT(slotRemoveButton()));

	connect(this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));

	resize(QSize(450, 450));
}

void TooltipEditDialog::slotOkClicked()
{
	QStringList oldList = Kopete::AppearanceSettings::self()->toolTipContents();
	QStringList newList;

	QString keyname;

	int max = mUsedEntries->rowCount( );
	for ( int i=0; i < max; i++ )
	{
		QStandardItem *item = mUsedEntries->item( i, 0 );
		keyname = item->data().value<QString>();
		newList += keyname;
		// kDebug(14000) <<
		//	"Adding key '" << keyname << "' to tooltip list" << endl;
	}

	if(oldList != newList)
	{
		Kopete::AppearanceSettings::self()->setToolTipContents(newList);
		emit changed(true);
		kDebug(14000) << "tooltip fields changed, emitting changed()";
	}
}


void TooltipEditDialog::slotUnusedSelected(const QItemSelection& selected)
{
	tbAdd->setEnabled(selected.indexes().count());
}

void TooltipEditDialog::slotUsedSelected(const QItemSelection& selected)
{
	tbRemove->setEnabled( selected.indexes().count() );
	tbUp->setEnabled( selected.indexes().count() );
	tbDown->setEnabled( selected.indexes().count() );

	if ( !selected.indexes().count() )
		return;

	// Disable Up button if we are at the top, disable Down one if we are at bottom
	if ( selected.indexes().first().row() == 0 )
		tbUp->setEnabled( false );
	else
		tbUp->setEnabled( true );

	if ( selected.indexes().last().row() == mUsedEntries->rowCount() - 1 )
		tbDown->setEnabled( false );
	else
		tbDown->setEnabled( true );
}

void TooltipEditDialog::slotUpButton()
{
	QModelIndexList indexList = usedItemsListView->selectionModel()->selectedIndexes();
	usedItemsListView->selectionModel()->clear();

	foreach( const QModelIndex &index, indexList )
	{
		int row = index.row();

		if ( row - 1 < 0 )
			return;

		// Move it up
		mUsedEntries->insertRow( row - 1, mUsedEntries->takeRow( row ) );

		// Keep the element selected
		usedItemsListView->selectionModel()->select( mUsedEntries->index( row - 1, 0 ) , QItemSelectionModel::Select );
		usedItemsListView->scrollTo( mUsedEntries->index( row - 1, 0 ) );

		// Check for the up and down buttons
		if ( row - 1 == 0 )
			tbUp->setEnabled( false );
		tbDown->setEnabled( true );
	}
}

void TooltipEditDialog::slotDownButton()
{
	QModelIndexList indexList = usedItemsListView->selectionModel()->selectedIndexes();
	usedItemsListView->selectionModel()->clear();

	foreach( const QModelIndex &index, indexList )	{
		int row = index.row();

		if ( row + 1 > mUsedEntries->rowCount() )
			return;

		// Move it down
		mUsedEntries->insertRow( row + 1, mUsedEntries->takeRow( row ) );

		// Keep it selected
		usedItemsListView->selectionModel()->select( mUsedEntries->index( row + 1, 0 ) , QItemSelectionModel::Select );
		usedItemsListView->scrollTo( mUsedEntries->index( row + 1, 0 ) );


		// Check for the up and down buttons
		if ( row + 1 == mUsedEntries->rowCount() - 1 )
			tbDown->setEnabled( false );
		tbUp->setEnabled( true );
	}
}

void TooltipEditDialog::slotAddButton()
{
	QModelIndexList indexList = unusedItemsListView->selectionModel()->selectedIndexes();

	foreach( const QModelIndex &index_, indexList )
	{
		QModelIndex index = static_cast<QSortFilterProxyModel*>( unusedItemsListView->model() )->mapToSource( index_ );

		// We insert it after the last selected one if there is a selection,
		// at the end else.
		QModelIndex insertAfter;
		if ( !usedItemsListView->selectionModel()->selectedIndexes().isEmpty() )
			insertAfter = usedItemsListView->selectionModel()->selectedIndexes().last();
		else
			insertAfter = mUsedEntries->index( mUsedEntries->rowCount() - 1, 0 );

		// Move the row from the unused items list to the used items one.
		mUsedEntries->insertRow( insertAfter.row() + 1, mUnusedEntries->takeRow( index.row() ) );

		// Make the newly inserted item current
		QModelIndex newIndex = mUsedEntries->index( insertAfter.row() + 1, 0 );
		usedItemsListView->setCurrentIndex( newIndex );
	}
}

void TooltipEditDialog::slotRemoveButton()
{
	QModelIndexList indexList = usedItemsListView->selectionModel()->selectedIndexes();

	foreach( const QModelIndex &index, indexList )
	{
		int row = index.row();


		mUnusedEntries->insertRow( 0, mUsedEntries->takeRow( index.row() ) );

		// Move selection
		if ( row > 0 )
			usedItemsListView->selectionModel()->select( mUsedEntries->index( row - 1, 0 ), QItemSelectionModel::Select );
		else
			usedItemsListView->selectionModel()->select( mUsedEntries->index( row, 0 ), QItemSelectionModel::Select );

	}
}

#include "tooltipeditdialog.moc"
