/*
    statusconfig_manager.cpp

    Copyright (c) 2008       by Roman Jarosz           <kedgedev@centrum.cz>
    Kopete    (c) 2008       by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "statusconfig_manager.h"

#include <QtGui/QHeaderView>

#include "kopeteonlinestatusmanager.h"
#include "statusmodel.h"
#include "kopetestatusmanager.h"
#include "kopetestatusitems.h"

class StatusConfig_Manager::Private
{
public:
	Private() {}
	KopeteStatusModel *statusModel;
	Kopete::Status::StatusGroup *rootGroup;
};

StatusConfig_Manager::StatusConfig_Manager( QWidget *parent )
: QWidget( parent ), d( new Private() )
{
	setupUi( this );

	pbAddStatus->setIcon( KIcon("list-add") );
	pbRemove->setIcon( KIcon("edit-delete") );
	pbAddGroup->setIcon( KIcon("folder-new") );

	connect( pbAddStatus, SIGNAL(clicked()), SLOT(addStatus()) );
	connect( pbRemove, SIGNAL(clicked()), SLOT(removeStatus()) );
	connect( pbAddGroup, SIGNAL(clicked()), SLOT(addGroup()) );

	d->rootGroup = Kopete::StatusManager::self()->copyRootGroup();
	d->statusModel = new KopeteStatusModel( d->rootGroup );
	d->statusModel->setSupportedDragActions( Qt::MoveAction );
	statusView->setModel( d->statusModel );
	connect( d->statusModel, SIGNAL(changed()), this, SIGNAL(changed()) );

	KIcon icon = Kopete::OnlineStatusManager::pixmapForCategory( Kopete::OnlineStatusManager::Online );
	cbStatusCategory->addItem( icon, i18n("Online"), Kopete::OnlineStatusManager::Online );
	icon = Kopete::OnlineStatusManager::pixmapForCategory( Kopete::OnlineStatusManager::FreeForChat );
	cbStatusCategory->addItem( icon, i18n("Free For Chat"), Kopete::OnlineStatusManager::FreeForChat );
	icon = Kopete::OnlineStatusManager::pixmapForCategory( Kopete::OnlineStatusManager::Away );
	cbStatusCategory->addItem( icon, i18n("Away"), Kopete::OnlineStatusManager::Away );
	icon = Kopete::OnlineStatusManager::pixmapForCategory( Kopete::OnlineStatusManager::ExtendedAway );
	cbStatusCategory->addItem( icon, i18n("Extended Away"), Kopete::OnlineStatusManager::ExtendedAway );
	icon = Kopete::OnlineStatusManager::pixmapForCategory( Kopete::OnlineStatusManager::Busy );
	cbStatusCategory->addItem( icon, i18n("Busy"), Kopete::OnlineStatusManager::Busy );
	icon = Kopete::OnlineStatusManager::pixmapForCategory( Kopete::OnlineStatusManager::Idle );
	cbStatusCategory->addItem( icon, i18n("Idle"), Kopete::OnlineStatusManager::Idle );
	icon = Kopete::OnlineStatusManager::pixmapForCategory( Kopete::OnlineStatusManager::Invisible );
	cbStatusCategory->addItem( icon, i18n("Invisible"), Kopete::OnlineStatusManager::Invisible );
	icon = Kopete::OnlineStatusManager::pixmapForCategory( Kopete::OnlineStatusManager::Offline );
	cbStatusCategory->addItem( icon, i18n("Offline"), Kopete::OnlineStatusManager::Offline );
	icon = Kopete::OnlineStatusManager::pixmapForCategory( 0x00 );
	cbStatusCategory->addItem( icon, i18n("Do Not Change"), 0x00 );

	statusView->expandAll();

	connect( leStatusTitle, SIGNAL(textEdited(QString)), this, SLOT(editTitleEdited(QString)) );
	connect( cbStatusCategory, SIGNAL(currentIndexChanged(int)), this, SLOT(editTypeChanged(int)) );
	connect( teStatusMessage, SIGNAL(textChanged()), this, SLOT(editMessageChanged()) );

	QItemSelectionModel *selectionModel = statusView->selectionModel();
	//TODO change currentChanged to currentRowChanged when TT Bug 162986 is fixed
	connect( selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
	         this, SLOT(currentRowChanged(QModelIndex,QModelIndex)) );

	currentRowChanged( selectionModel->currentIndex(), QModelIndex() );
}

StatusConfig_Manager::~StatusConfig_Manager()
{
	delete d->statusModel;
	delete d->rootGroup;
	delete d;
}

void StatusConfig_Manager::load()
{
}

void StatusConfig_Manager::save()
{
	Kopete::Status::StatusGroup *group = qobject_cast<Kopete::Status::StatusGroup *>(d->rootGroup->copy());
	Kopete::StatusManager::self()->setRootGroup( group );
	Kopete::StatusManager::self()->saveXML();
}

void StatusConfig_Manager::addStatus()
{
	Kopete::Status::Status *status = new Kopete::Status::Status();
	status->setTitle( i18n( "New Status" ) );
	status->setCategory( Kopete::OnlineStatusManager::Online );

	QModelIndex index = statusView->selectionModel()->currentIndex();
	QModelIndex newIndex = d->statusModel->insertItem( index, status );

	if ( newIndex.isValid() )
		statusView->setCurrentIndex( newIndex );
	else
		delete status;
}

void StatusConfig_Manager::addGroup()
{
	Kopete::Status::StatusGroup *group = new Kopete::Status::StatusGroup();
	group->setTitle( i18n( "New Group" ) );
	group->setCategory( Kopete::OnlineStatusManager::Online );

	QModelIndex index = statusView->selectionModel()->currentIndex();
	QModelIndex newIndex = d->statusModel->insertItem( index, group );

	if ( newIndex.isValid() )
	{
		statusView->setCurrentIndex( newIndex );
		statusView->setExpanded( newIndex, true );
	}
	else
		delete group;
}

void StatusConfig_Manager::removeStatus()
{
	QModelIndex index = statusView->selectionModel()->currentIndex();

	if ( index.isValid() )
		d->statusModel->removeRow( index.row(), index.parent() );
}

void StatusConfig_Manager::currentRowChanged( const QModelIndex &current, const QModelIndex& )
{
	leStatusTitle->blockSignals( true );
	cbStatusCategory->blockSignals( true );
	teStatusMessage->blockSignals( true );

	if ( current.isValid() )
	{
		statusGroupBox->setEnabled( true );
		pbRemove->setEnabled( true );
		int categoryIndex = cbStatusCategory->findData( d->statusModel->data( current, KopeteStatusModel::Category ).toInt() );
		cbStatusCategory->setCurrentIndex( categoryIndex );
		leStatusTitle->setText( d->statusModel->data( current, KopeteStatusModel::Title ).toString() );

		if ( d->statusModel->data( current, KopeteStatusModel::Group ).toBool() )
		{
			lblStatusMessage->setEnabled( false );
			teStatusMessage->setEnabled( false );
			teStatusMessage->clear();
		}
		else
		{
			lblStatusMessage->setEnabled( true );
			teStatusMessage->setEnabled( true );
			teStatusMessage->setPlainText( d->statusModel->data( current, KopeteStatusModel::Message ).toString() );
		}
	}
	else
	{
		statusGroupBox->setEnabled( false );
		pbRemove->setEnabled( false );
		cbStatusCategory->setCurrentIndex( 0 );
		leStatusTitle->clear();
		teStatusMessage->clear();
	}

	leStatusTitle->blockSignals( false );
	cbStatusCategory->blockSignals( false );
	teStatusMessage->blockSignals( false );
}

void StatusConfig_Manager::editTitleEdited( const QString &text )
{
	QModelIndex modelIndex = statusView->selectionModel()->currentIndex();
	d->statusModel->setData( modelIndex, text, KopeteStatusModel::Title );
}

void StatusConfig_Manager::editMessageChanged()
{
	QModelIndex modelIndex = statusView->selectionModel()->currentIndex();
	d->statusModel->setData( modelIndex, teStatusMessage->toPlainText(), KopeteStatusModel::Message );
}

void StatusConfig_Manager::editTypeChanged( int index )
{
	QModelIndex modelIndex = statusView->selectionModel()->currentIndex();
	d->statusModel->setData( modelIndex, cbStatusCategory->itemData(index), KopeteStatusModel::Category );
}

#include "statusconfig_manager.moc"
