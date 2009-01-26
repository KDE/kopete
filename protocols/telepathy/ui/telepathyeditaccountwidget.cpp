/*
 * telepathyeditaccountwidget.h - UI to edit Telepathy account settings
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
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

// KDE includes
#include <kmessagebox.h>
#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>

//TelepathyQt4 includes
#include <TelepathyQt4/Client/ConnectionManager>
#include <TelepathyQt4/Client/PendingStringList>

// QtTapioca includes
#include <QtTapioca/ConnectionManagerFactory>
#include <QtTapioca/ConnectionManager>

// Kopete includes
#include <kopeteaccount.h>

// Local includes
#include "telepathyeditparameterwidget.h"
#include "telepathyprotocol.h"
#include "telepathyaccount.h"

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

    Ui::TelepathyEditAccountWidget ui;
    TelepathyEditParameterWidget *paramWidget;
    QList<ConnectionManager::Parameter> savedParameterList;
    Telepathy::Client::ConnectionManager *mCM;
};

// TODO: Required flags for parameters.
TelepathyEditAccountWidget::TelepathyEditAccountWidget(Kopete::Account *account, QWidget *parent)
 : QWidget(parent), KopeteEditAccountWidget(account), d(new Private)
{
    Telepathy::registerTypes();

    d->ui.setupUi(this);

    // Setup signal/slot connection
    connect(d->ui.treeConnectionManager, SIGNAL(itemSelectionChanged()), this, SLOT(connectionManagerSelectionChanged()));
    connect(d->ui.treeProtocol, SIGNAL(itemSelectionChanged()), this, SLOT(protocolSelectionChanged()));

    // List connection manager after the constructor.
    QTimer::singleShot(0, this, SLOT(listConnectionManager()));
}

TelepathyEditAccountWidget::~TelepathyEditAccountWidget()
{
    kDebug(TELEPATHY_DEBUG_AREA) ;
    delete d;
}

TelepathyAccount *TelepathyEditAccountWidget::account()
{
    return static_cast<TelepathyAccount*>( KopeteEditAccountWidget::account() );
}

bool TelepathyEditAccountWidget::validateData()
{
    kDebug(TELEPATHY_DEBUG_AREA) ;
    // You must fill the form to move to the next step
    if( !d->ui.treeConnectionManager->selectedItems().isEmpty() &&
    	!d->ui.treeProtocol->selectedItems().isEmpty() )
        return true;
    else
    {
	KMessageBox::error(this, i18n("Please fill in the fields in the dialog. First select a connection manager, then select a protocol."));
	return false;
    }
}

Kopete::Account *TelepathyEditAccountWidget::apply()
{
    // Get parameter list
    if( d->paramWidget )
    {
    	kDebug(TELEPATHY_DEBUG_AREA) ;
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
                    kDebug(TELEPATHY_DEBUG_AREA) << "Found account id: " << newAccountId;
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
    kDebug(TELEPATHY_DEBUG_AREA) ;
    // Restore config not related to ConnectionManager parameters first
    // so that the UI for the protocol parameters will be generated
    if( account()->readConfig() )
    {
        QString readConnectionManager = account()->connectionManager();
	QString readProtocol = account()->connectionProtocol();

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

                    // Update the parameters in the UI
                    d->paramWidget->setParameterList( account()->allConnectionParameters() );
		}
            }
	}
    }
}

void TelepathyEditAccountWidget::writeConfig()
{
    kDebug(TELEPATHY_DEBUG_AREA) ;
    QString selectedConnectionManager = d->ui.treeConnectionManager->selectedItems().first()->text(0);
    QString selectedProtocol = d->ui.treeProtocol->selectedItems().first()->text(0);
    QString accountId = account()->accountId();

    KMessageBox::information(this, i18n("ConnectionManager: %1\nProtocol: %2\nAccount: %3",selectedConnectionManager,selectedProtocol,accountId) );

    // Write config not related to ConnectionManager Parameters
    KConfigGroup *accountConfig = account()->configGroup();
    accountConfig->writeEntry( QLatin1String("ConnectionManager"), selectedConnectionManager );
    accountConfig->writeEntry( QLatin1String("SelectedProtocol"), selectedProtocol );
	
    // Write config related to ConnectionManager Parameter
    QString telepathyGroup(TelepathyProtocol::protocol()->formatTelepathyConfigGroup(selectedConnectionManager, selectedProtocol, accountId));
    KConfigGroup telepathyConfig = KGlobal::config()->group(telepathyGroup);
	
    foreach(ConnectionManager::Parameter parameter, d->savedParameterList)
    {
        telepathyConfig.writeEntry( parameter.name(), parameter.value() );
    }
}

void TelepathyEditAccountWidget::showConnectionManagerInfo(Telepathy::Client::PendingOperation *operation)
{
    kDebug() << "onAccountReady() called";
    if(operation->isError())
    {
        kDebug() << operation->errorName() << ": " << operation->errorMessage();
        return;
    }

    kDebug() << "listNames";
    Telepathy::Client::PendingStringList *stringList = d->mCM->listNames(QDBusConnection::sessionBus());

    foreach(QString name, stringList->result())
    {
        kDebug() << name;
    }

    kDebug() << "name: " << d->mCM->name();

    kDebug() << "interfaces";
    QStringList interfaces = d->mCM->interfaces();
    foreach(QString inter, interfaces)
    {
        kDebug() << inter;
    }

    kDebug() << "supportedProtocols";
    QStringList suppProtocols = d->mCM->supportedProtocols();
    foreach(QString protocol, suppProtocols)
    {
        kDebug() << protocol;
    }

    kDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!! END!!!!!!!!!!!!";
}

void TelepathyEditAccountWidget::listConnectionManager()
{
    if (!QDBusConnection::sessionBus().isConnected())
    {
        kDebug() << "listConnectionManager(): cannot connect to session bus.";
        return;
    }

    kDebug() << "testBasics()";
    d->mCM = new Telepathy::Client::ConnectionManager("gabble", 0);

    Telepathy::Client::PendingOperation *op = d->mCM->becomeReady();
    connect(op, SIGNAL(finished(Telepathy::Client::PendingOperation *)),
            this, SLOT(showConnectionManagerInfo(Telepathy::Client::PendingOperation *)));

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

	// Use saved parameters if available
	QList<QtTapioca::ConnectionManager::Parameter> tabParameter;
	if( account() && protocol == account()->connectionProtocol() )
	{
            tabParameter = account()->allConnectionParameters();
	}
	else
	{
            tabParameter = cmItem->connectionManager()->protocolParameters(protocol);
	}

	d->paramWidget = new TelepathyEditParameterWidget(tabParameter, this);
	d->ui.tabWidget->addTab(d->paramWidget, i18n("Protocol Parameters"));
    }
}

#include "telepathyeditaccountwidget.moc"
