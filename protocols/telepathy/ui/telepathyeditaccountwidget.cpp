/*
 * telepathyeditaccountwidget.h - UI to edit Telepathy account settings
 *
 * Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>
 * 
 * Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>
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
#include <QtCore/QLatin1String>
#include <QtCore/QStringList>
#include <QtCore/QPointer>
#include <QtCore/QMap>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QTreeWidgetItem>

// KDE includes
#include <kmessagebox.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>

// QtTapioca includes
#include <QtTapioca/ConnectionManagerFactory>
#include <QtTapioca/ConnectionManager>

// Kopete includes
#include <kopeteaccount.h>

// Local includes
#include "telepathyeditparameterwidget.h"
#include "telepathyprotocol.h"

using namespace QtTapioca;

class ConnectionManagerItem : public QTreeWidgetItem
{
public:
	ConnectionManagerItem(ConnectionManager *connectionManager, QTreeWidget *parent)
	 : QTreeWidgetItem(parent)
	{
		m_connectionManager = connectionManager;

		setText(0, connectionManager->name());
		setText(1, QString::number(connectionManager->supportedProtocols().size()));
	}

	ConnectionManager *connectionManager()
	{
		return m_connectionManager;
	}

private:
	QPointer<ConnectionManager> m_connectionManager;
};

class TelepathyEditAccountWidget::Private
{
public:
	Private()
	 : paramWidget(0)
	{}

	QString formatTelepathyGroup(const QString &connectionManager, const QString &protocol, const QString &accountId);

	Ui::TelepathyEditAccountWidget ui;
	TelepathyEditParameterWidget *paramWidget;
	QList<ConnectionManager::Parameter> savedParameterList;
};

// TODO: Required flags for parameters.
TelepathyEditAccountWidget::TelepathyEditAccountWidget(Kopete::Account *account, QWidget *parent)
 : QWidget(parent), KopeteEditAccountWidget(account), d(new Private)
{
	d->ui.setupUi(this);

	// Setup signal/slot connection
	connect(d->ui.treeConnectionManager, SIGNAL(itemSelectionChanged()), this, SLOT(connectionManagerSelectionChanged()));
	connect(d->ui.treeProtocol, SIGNAL(itemSelectionChanged()), this, SLOT(protocolSelectionChanged()));

	// List connection manager after the constructor.
	QTimer::singleShot(0, this, SLOT(listConnectionManager()));
}

TelepathyEditAccountWidget::~TelepathyEditAccountWidget()
{
	delete d;
}

bool TelepathyEditAccountWidget::validateData()
{
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << endl;
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
	// Get parameter list
	if( d->paramWidget )
	{
		d->savedParameterList = d->paramWidget->parameterList();
	
		if( !account() )
		{
			QString newAccountId;
			// Look for a parameter that begin with "account"
			foreach(ConnectionManager::Parameter parameter, d->savedParameterList)
			{
				if( parameter.name().startsWith( QLatin1String("account") ) )
				{
					newAccountId = parameter.value().toString();
					kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Found account id: " << newAccountId << endl;
					break;
				}
			}
		
			setAccount( TelepathyProtocol::protocol()->createNewAccount(newAccountId) );
		}

		writeConfig();
	}

	return account();
}

void TelepathyEditAccountWidget::readConfig()
{
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << endl;
	// Restore config not related to ConnectionManager parameters first
	// so that the UI for the protocol parameters will be generated
	KConfigGroup *accountConfig = account()->configGroup();
	QString readConnectionManager = accountConfig->readEntry( QLatin1String("ConnectionManager"), QString() );
	QString readProtocol = accountConfig->readEntry( QLatin1String("SelectedProtocol"), QString() );
	
	// Do nothing if the config return empty values
	if( !readConnectionManager.isEmpty() && !readProtocol.isEmpty() )
	{
		// Look for the connection manager in the tree widget
		QList<QTreeWidgetItem*> availableConnectionManager = d->ui.treeConnectionManager->findItems( readConnectionManager, Qt::MatchStartsWith );
		if( !availableConnectionManager.isEmpty() )
		{
			// Select the connection manager to generate the protocol list
			d->ui.treeConnectionManager->setCurrentItem( availableConnectionManager.first() );
			
			// At this point, the protocol tree widget is filled

			// Look for the protocol in the tree widget
			QList<QTreeWidgetItem*> availableProtocol = d->ui.treeProtocol->findItems( readProtocol, Qt::MatchStartsWith );
			if( !availableProtocol.isEmpty() )
			{
				d->ui.treeProtocol->setCurrentItem( availableProtocol.first() );

				// At this point, the protocol preferences tab is created

				// Now fill the preferences
				QList<ConnectionManager::Parameter> readParameters;
				KConfig *telepathyConfig = KGlobal::config();
				QMap<QString,QString> allEntries = telepathyConfig->entryMap( d->formatTelepathyGroup(readConnectionManager, readProtocol, account()->accountId()) );
				QMap<QString,QString>::ConstIterator it, itEnd = allEntries.constEnd();
				for(it = allEntries.constBegin(); it != itEnd; ++it)
				{
					ConnectionManager::Parameter parameter(it.key(), it.value());
					readParameters.append(parameter);
				}

				// Update the parameters in the UI
				d->paramWidget->setParameterList(readParameters);
			}
		}
	}
}

void TelepathyEditAccountWidget::writeConfig()
{
	QString selectedConnectionManager = d->ui.treeConnectionManager->selectedItems().first()->text(0);
	QString selectedProtocol = d->ui.treeProtocol->selectedItems().first()->text(0);
	QString accountId = account()->accountId();

	KMessageBox::information(this, QString("ConnectionManager: %1\nProtocol: %2\nAccount: %3").arg(selectedConnectionManager).arg(selectedProtocol).arg(accountId) );

	// Write config not related to ConnectionManager Parameters
	KConfigGroup *accountConfig = account()->configGroup();
	accountConfig->writeEntry( QLatin1String("ConnectionManager"), selectedConnectionManager );
	accountConfig->writeEntry( QLatin1String("SelectedProtocol"), selectedProtocol );
	
	// Write config related to ConnectionManager Parameter
	KConfig *telepathyConfig = KGlobal::config();
	telepathyConfig->setGroup( d->formatTelepathyGroup(selectedConnectionManager, selectedProtocol, accountId) );
	
	foreach(ConnectionManager::Parameter parameter, d->savedParameterList)
	{
		telepathyConfig->writeEntry( parameter.name(), parameter.value() );
	}
}

void TelepathyEditAccountWidget::listConnectionManager()
{
	// List all available connection managers in the tree widget
	QList<ConnectionManager*> connectionManagers = ConnectionManagerFactory::self()->getAllConnectionManagers();
	foreach(ConnectionManager *connectionManager, connectionManagers)
	{
		new ConnectionManagerItem(connectionManager, d->ui.treeConnectionManager);
	}

	// Read config if account() return true
	// FIXME: Shouldn't be called here
	if( account() )
		readConfig();
}

void TelepathyEditAccountWidget::connectionManagerSelectionChanged()
{
	QTreeWidgetItem *item = d->ui.treeConnectionManager->selectedItems().first();
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

void TelepathyEditAccountWidget::protocolSelectionChanged()
{
	QTreeWidgetItem *connectionItem = d->ui.treeConnectionManager->selectedItems().first();
	ConnectionManagerItem *cmItem = static_cast<ConnectionManagerItem*>(connectionItem);

	QTreeWidgetItem *protocolItem = d->ui.treeProtocol->selectedItems().first();
	if( protocolItem && cmItem )
	{
		// Check if existing tab exists and remove it (and delete the widget)
		if( d->paramWidget )
		{
			d->ui.tabWidget->removeTab(1);
			d->paramWidget->deleteLater();
		}

		// Add new tab
		QString protocol = protocolItem->text(0);
		d->paramWidget = new TelepathyEditParameterWidget(cmItem->connectionManager()->protocolParameters(protocol), this);
		d->ui.tabWidget->addTab(d->paramWidget, i18n("Protocol Parameters"));
	}
}

QString TelepathyEditAccountWidget::Private::formatTelepathyGroup(const QString &connectionManager, const QString &protocol, const QString &accountId)
{
	return QString("Telepathy_%1_%2_%3").arg(connectionManager).arg(protocol).arg(accountId);
}

#include "telepathyeditaccountwidget.moc"
