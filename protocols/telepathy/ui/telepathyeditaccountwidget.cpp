/*
 * telepathyeditaccountwidget.h - UI to edit Telepathy account settings
 *
 * Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>
 * 
 * Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#include "telepathyeditaccountwidget.h"
#include "ui_telepathyeditaccountwidget.h"

// Qt includes
#include <QtCore/QTimer>
#include <QtCore/QStringList>
#include <QtCore/QPointer>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QTreeWidgetItem>

// KDE includes
#include <kmessagebox.h>
#include <klocale.h>

// QtTapioca includes
#include <CMFactory>
#include <CM>

// Kopete includes
#include <kopeteaccount.h>

// Local includes

using namespace QtTapioca;

class ConnectionManagerItem : public QTreeWidgetItem
{
public:
	ConnectionManagerItem(CM *connectionManager, QTreeWidget *parent)
	 : QTreeWidgetItem(parent)
	{
		m_connectionManager = connectionManager;

		setText(0, connectionManager->name());
		setText(1, QString::number(connectionManager->supportedProtocols().size()));
	}

	CM *connectionManager()
	{
		return m_connectionManager;
	}

private:
	QPointer<CM> m_connectionManager;
};

class TelepathyEditAccountWidget::Private
{
public:
	Ui::TelepathyEditAccountWidget ui;
	CMFactory *cmFactory;
};

TelepathyEditAccountWidget::TelepathyEditAccountWidget(Kopete::Account *account, QWidget *parent)
 : QWidget(parent), KopeteEditAccountWidget(account), d(new Private)
{
	d->cmFactory = new QtTapioca::CMFactory(this);

	d->ui.setupUi(this);

	// Setup signal/slot connection
	connect(d->ui.treeConnectionManager, SIGNAL(itemActivated ( QTreeWidgetItem *, int)), this, SLOT(connectionManagerActivated(QTreeWidgetItem*, int)));

	// List connection manager after the constructor.
	QTimer::singleShot(0, this, SLOT(listConnectionManager()));
}

TelepathyEditAccountWidget::~TelepathyEditAccountWidget()
{
	delete d;
}

bool TelepathyEditAccountWidget::validateData()
{
	// You must fill the form to move to the next step
	if( !d->ui.treeConnectionManager->selectedItems().isEmpty() &&
		!d->ui.treeProtocol->selectedItems().isEmpty() )
		return true;
	else
	{
		KMessageBox::error(this, i18n("Please fill the dialog. First select a connection manager then select a protocol."));
		return false;
	}
}

Kopete::Account *TelepathyEditAccountWidget::apply()
{
	QString selectedConnectionManager = d->ui.treeConnectionManager->selectedItems().first()->text(0);
	QString selectedProtocol = d->ui.treeProtocol->selectedItems().first()->text(0);

	KMessageBox::information(this, QString("CM: %1\nProtocol: %2").arg(selectedConnectionManager).arg(selectedProtocol) );

	return account();
}

void TelepathyEditAccountWidget::listConnectionManager()
{
	// List all available connection managers in the tree widget
	QList<CM*> connectionManagers = d->cmFactory->getAllCMs();
	foreach(CM *connectionManager, connectionManagers)
	{
		new ConnectionManagerItem(connectionManager, d->ui.treeConnectionManager);
	}
}

void TelepathyEditAccountWidget::connectionManagerActivated(QTreeWidgetItem *item, int column)
{
	ConnectionManagerItem *itemActivated = static_cast<ConnectionManagerItem*>(item);
	if( itemActivated )
	{
		// Clear protocol list
		d->ui.treeProtocol->clear();
		// List supported protocols by this connetion manager.
		QStringList supportedProtocols = itemActivated->connectionManager()->supportedProtocols();
		foreach(QString protocol, supportedProtocols)
		{
			new QTreeWidgetItem(d->ui.treeProtocol, QStringList(protocol));
		}
	}
}

#include "telepathyeditaccountwidget.moc"
