/*
    xtrazicqstatuseditor.h  -  Xtraz ICQ Status Editor

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

#include "xtrazicqstatuseditor.h"

#include <QHeaderView>

#include "ui_xtrazicqstatuseditorui.h"

#include "oscartypes.h"

#include "icqstatusmanager.h"
#include "xtrazstatusmodel.h"
#include "xtrazstatusdelegate.h"

namespace Xtraz
{

ICQStatusEditor::ICQStatusEditor( ICQStatusManager *statusManager, QWidget *parent )
: KDialog( parent ), mStatusManager( statusManager )
{
	setCaption( i18n( "Xtraz Status Editor" ) );
	setButtons( KDialog::Ok | KDialog::Cancel );

	mUi = new Ui::XtrazICQStatusEditorUI();
	QWidget *w = new QWidget( this );
	mUi->setupUi( w );
	setMainWidget( w );

	mUi->statusView->setAlternatingRowColors( true );
	mUi->statusView->setTabKeyNavigation( false );
	mUi->statusView->setSelectionBehavior( QAbstractItemView::SelectRows );
	mUi->statusView->setSelectionMode( QAbstractItemView::SingleSelection );
	mUi->statusView->horizontalHeader()->setClickable( false );
	mUi->statusView->horizontalHeader()->setStretchLastSection( true );

	QList<QIcon> icons;
	for ( int i = 0; i < Oscar::XSTAT_LAST; ++i )
		icons << KIcon( QString( "icq_xstatus%1" ).arg( i ) );

	mUi->statusView->setItemDelegate( new StatusDelegate( icons, this ) );

	mXtrazStatusModel = new Xtraz::StatusModel( this );
	mXtrazStatusModel->setStatuses( mStatusManager->xtrazStatuses() );
	mUi->statusView->setModel( mXtrazStatusModel );
	mUi->statusView->setCurrentIndex( mXtrazStatusModel->index( 0, 0 ) );

	connect( mUi->buttonAdd, SIGNAL(clicked()), this, SLOT(addStatus()) );
	connect( mUi->buttonDelete, SIGNAL(clicked()), this, SLOT(deleteStatus()) );
	connect( mUi->buttonUp, SIGNAL(clicked()), this, SLOT(moveUp()) );
	connect( mUi->buttonDown, SIGNAL(clicked()), this, SLOT(moveDown()) );
	connect( this, SIGNAL(okClicked()), this, SLOT(save()) );
	connect( mUi->statusView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
	         this, SLOT(updateButtons()) );

	updateButtons();
	mUi->statusView->setFocus();
}

ICQStatusEditor::~ICQStatusEditor()
{
	delete mUi;
}

void ICQStatusEditor::save()
{
	mStatusManager->setXtrazStatuses( mXtrazStatusModel->getStatuses() );
}

void ICQStatusEditor::moveUp()
{
	QModelIndex index = mUi->statusView->selectionModel()->currentIndex();

	if ( mXtrazStatusModel->swapRows( index.row() - 1, index.row() ) )
	{
		index = mXtrazStatusModel->index( index.row() - 1, index.column() );
		mUi->statusView->setCurrentIndex( index );
		updateButtons();
	}
}

void ICQStatusEditor::moveDown()
{
	QModelIndex index = mUi->statusView->selectionModel()->currentIndex();

	if ( mXtrazStatusModel->swapRows( index.row(), index.row() + 1 ) )
	{
		index = mXtrazStatusModel->index( index.row() + 1, index.column() );
		mUi->statusView->setCurrentIndex( index );
		updateButtons();
	}
}

void ICQStatusEditor::addStatus()
{
	QModelIndex index = mUi->statusView->selectionModel()->currentIndex();
	const int row = ( index.row() < 0 ) ? 0 : index.row();

	if ( mXtrazStatusModel->insertRow( row ) )
	{
		index = mXtrazStatusModel->index( row, 0 );
		mUi->statusView->setCurrentIndex( index );
		updateButtons();
	}
}

void ICQStatusEditor::deleteStatus()
{
	QModelIndex index = mUi->statusView->currentIndex();
	const int row = mUi->statusView->selectionModel()->currentIndex().row();
	if ( row < 0 )
		return;

	if ( mXtrazStatusModel->removeRow( row ) )
	{
		index = mXtrazStatusModel->index( (row == 0) ? 0 : row - 1, index.column() );
		mUi->statusView->setCurrentIndex( index );
		updateButtons();
	}
}

void ICQStatusEditor::updateButtons()
{
	QModelIndex index = mUi->statusView->currentIndex();
	const int rowCount = mXtrazStatusModel->rowCount();

	mUi->buttonUp->setEnabled( index.isValid() && index.row() > 0 );
	mUi->buttonDown->setEnabled( index.isValid() && index.row() < rowCount - 1 );
	mUi->buttonDelete->setEnabled( index.isValid() );
}

}

#include "xtrazicqstatuseditor.moc"
