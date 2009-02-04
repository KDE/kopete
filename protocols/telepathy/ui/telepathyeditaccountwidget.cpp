/*
 * telepathyeditaccountwidget.h - UI to edit Telepathy account settings
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 *               2009 by Dariusz Mikulski <dariusz.mikulski@gmail.com>
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
#include <TelepathyQt4/Client/ConnectionInterface>

// Kopete includes
#include <kopeteaccount.h>

// Local includes
#include "telepathyeditparameterwidget.h"
#include "telepathyprotocol.h"
#include "telepathyaccount.h"
#include "connectionmanageritem.h"

class TelepathyEditAccountWidget::Private
{
public:
    Private()
     : paramWidget(0)
    {}

    Ui::TelepathyEditAccountWidget ui;
    TelepathyEditParameterWidget *paramWidget;
    Telepathy::Client::ProtocolParameterList savedParameterList;
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
    kDebug(TELEPATHY_DEBUG_AREA) << "validateData() called";
    // You must fill the form to move to the next step
    if( !d->ui.treeConnectionManager->selectedItems().isEmpty() &&
    	!d->ui.treeProtocol->selectedItems().isEmpty() &&
        validAccountData())
        return true;
    else
    {
        KMessageBox::error(this, i18n("Please fill in the fields in the dialog. First select a connection manager, then select a protocol."));
        return false;
    }
}

bool TelepathyEditAccountWidget::validAccountData()
{
    if( d->paramWidget )
    {
    	kDebug(TELEPATHY_DEBUG_AREA);
        d->savedParameterList = d->paramWidget->parameterList();

        // Look for a parameter that begin with "account"
        foreach(Telepathy::Client::ProtocolParameter *parameter, d->savedParameterList)
        {
            if( parameter->name().startsWith( QLatin1String("account") ) )
            {
                QString accountId = parameter->defaultValue().toString();
                if(!accountId.isEmpty())
                {
                    return true;
                    break;
                }
            }
        }
    }
    return false;
}

Kopete::Account *TelepathyEditAccountWidget::apply()
{
    kDebug(TELEPATHY_DEBUG_AREA) << "apply() called";
    // Get parameter list
    if( d->paramWidget )
    {
    	kDebug(TELEPATHY_DEBUG_AREA) ;
        d->savedParameterList = d->paramWidget->parameterList();

        if( !account() )
        {
            QString newAccountId;
            // Look for a parameter that begin with "account"
            foreach(Telepathy::Client::ProtocolParameter *parameter, d->savedParameterList)
            {
                if( parameter->name().startsWith( QLatin1String("account") ) )
                {
                    newAccountId = parameter->defaultValue().toString();
                    kDebug(TELEPATHY_DEBUG_AREA) << "Found account id: " << newAccountId;
                    break;
                }
            }
            setAccount( TelepathyProtocol::protocol()->createNewAccount(newAccountId) );
        }
        writeConfig();
        account()->initTelepathyAccount();
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
    kDebug(TELEPATHY_DEBUG_AREA) << "writeConfig() called";
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
	
    foreach(Telepathy::Client::ProtocolParameter *parameter, d->savedParameterList)
    {
        telepathyConfig.writeEntry( parameter->name(), parameter->defaultValue() );
    }
}

void TelepathyEditAccountWidget::onListNames(Telepathy::Client::PendingOperation *operation)
{
    // List all available connection managers in the tree widget
    Telepathy::Client::PendingStringList *p = static_cast<Telepathy::Client::PendingStringList*>(operation);
    foreach(QString name, p->result())
    {
        kDebug(TELEPATHY_DEBUG_AREA) << name;
        new ConnectionManagerItem(name, d->ui.treeConnectionManager);
    }

    // Read config if account() return true
    // FIXME: Shouldn't be called here
    if( account() )
        readConfig();
}

void TelepathyEditAccountWidget::listConnectionManager()
{
    if (!QDBusConnection::sessionBus().isConnected())
    {
        kDebug(TELEPATHY_DEBUG_AREA) << "listConnectionManager(): cannot connect to session bus.";
        return;
    }

    kDebug(TELEPATHY_DEBUG_AREA) << "Connect listNames";
    connect(Telepathy::Client::ConnectionManager::listNames(),
            SIGNAL(finished(Telepathy::Client::PendingOperation *)),
            this, SLOT(onListNames(Telepathy::Client::PendingOperation *)));
}

void TelepathyEditAccountWidget::connectionManagerSelectionChanged()
{
    QTreeWidgetItem *item = d->ui.treeConnectionManager->selectedItems().first();
    ConnectionManagerItem *itemActivated = dynamic_cast<ConnectionManagerItem*>(item);
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
    QList<QTreeWidgetItem*> cmItems = d->ui.treeConnectionManager->selectedItems();
    if(cmItems.isEmpty())
        return;

    QTreeWidgetItem *connectionItem = cmItems.first();
    ConnectionManagerItem *cmItem = dynamic_cast<ConnectionManagerItem*>(connectionItem);

    QList<QTreeWidgetItem*> protocolItems = d->ui.treeProtocol->selectedItems();
    if(protocolItems.isEmpty())
        return;
    
    QTreeWidgetItem *protocolItem = protocolItems.first();
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
        Telepathy::Client::ProtocolParameterList tabParameter;
        if( account() && protocol == account()->connectionProtocol() )
        {
            tabParameter = account()->allConnectionParameters();
        }
        else
        {
            Telepathy::Client::ProtocolInfoList protocolInfoList
                = cmItem->connectionManager()->protocols();
            foreach(Telepathy::Client::ProtocolInfo *protocolInfo, protocolInfoList)
            {
                if(protocolInfo->name() == protocol)
                {
                    tabParameter = protocolInfo->parameters();
                    break;
                }
            }
        }

        d->paramWidget = new TelepathyEditParameterWidget(tabParameter, this);
        d->ui.tabWidget->addTab(d->paramWidget, i18n("Protocol Parameters"));
    }
}

#include "telepathyeditaccountwidget.moc"
