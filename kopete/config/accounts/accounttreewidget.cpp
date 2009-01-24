/*
    accounttreewidget.cpp  -  Kopete account tree widget

    Copyright (c) 2009      by Roman Jarosz           <kedgedev@centrum.cz>
    Kopete    (c) 2009      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "accounttreewidget.h"

#include <QDropEvent>
#include <kdebug.h>

AccountTreeWidget::AccountTreeWidget( QWidget *parent )
	: QTreeWidget( parent )
{
}

void AccountTreeWidget::dragEnterEvent( QDragEnterEvent *event )
{
	if ( event->source() == this && (event->proposedAction() == Qt::MoveAction || dragDropMode() == QAbstractItemView::InternalMove) )
	{
		QList<QTreeWidgetItem*> items = selectedItems();
		if( items.size() != 1 )
			return;

		bool dragingIdentity = !dynamic_cast<KopeteAccountLVI*>(items.first());

		// Set drop flag based on item we drag
		if ( dragingIdentity )
			invisibleRootItem()->setFlags(invisibleRootItem()->flags() | Qt::ItemIsDropEnabled);
		else
			invisibleRootItem()->setFlags(invisibleRootItem()->flags() & ~Qt::ItemIsDropEnabled);

		for ( int i = 0; i < this->topLevelItemCount(); i++ )
		{
			QTreeWidgetItem* identityItem = this->topLevelItem( i );
			if ( dragingIdentity )
				identityItem->setFlags( identityItem->flags() & ~Qt::ItemIsDropEnabled );
			else
				identityItem->setFlags( identityItem->flags() | Qt::ItemIsDropEnabled );
		}

		QTreeWidget::dragEnterEvent(event);
	}
}

void AccountTreeWidget::dropEvent(QDropEvent *event)
{
	KopeteIdentityLVI* dragItem = 0;
	if( selectedItems().size() == 1 )
		dragItem = dynamic_cast<KopeteIdentityLVI*>(selectedItems().first());

	QTreeWidget::dropEvent(event);

	if ( event->isAccepted() )
		emit itemPositionChanged();

	// Expand identity item because it will be collapsed after drag
	if ( dragItem && !dragItem->isExpanded() )
		dragItem->setExpanded( true );
}

#include "accounttreewidget.moc"
