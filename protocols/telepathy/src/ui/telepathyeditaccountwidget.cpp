/*
 * This file is part of Kopete
 *
 * Copyright (C) 2009 Collabora Ltd. <info@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "ui/telepathyeditaccountwidget.h"

#include "telepathyprotocol.h"

#include <telepathyaccount.h>
#include <telepathyprotocolinternal.h>

#include <KCMTelepathyAccounts/AbstractAccountUi>
#include <KCMTelepathyAccounts/AbstractAccountUi>
#include <KCMTelepathyAccounts/ConnectionManagerItem>
#include <KCMTelepathyAccounts/AccountEditWidget>
#include <KCMTelepathyAccounts/PluginManager>
#include <KCMTelepathyAccounts/ProtocolItem>
#include <KCMTelepathyAccounts/ProtocolSelectWidget>

#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KSharedConfig>
#include <KTabWidget>

#include <kopeteaccount.h>

#include <QtGui/QGridLayout>

#include <TelepathyQt4/PendingStringList>

class TelepathyEditAccountWidget::Private
{
public:
    Private()
     : tabWidget(0),
       protocolSelectWidget(0),
       accountEditWidget(0),
       mainLayout(0)
    {
        kDebug();
    }

    KTabWidget *tabWidget;
    ProtocolSelectWidget *protocolSelectWidget;
    AccountEditWidget *accountEditWidget;
    QGridLayout *mainLayout;
};

TelepathyEditAccountWidget::TelepathyEditAccountWidget(Kopete::Account *account, QWidget *parent)
 : QWidget(parent),
   KopeteEditAccountWidget(account),
   d(new Private)
{
    kDebug();

    // When account is 0, we are creating a new account.
    if (!account) {
        // Add account UI
        setupAddAccountUi();
    } else {
        // Edit account UI
        setupEditAccountUi();
    }
}

TelepathyEditAccountWidget::~TelepathyEditAccountWidget()
{
    kDebug();

    delete d;
}

TelepathyAccount *TelepathyEditAccountWidget::account()
{
    kDebug();

    return qobject_cast<TelepathyAccount*>(KopeteEditAccountWidget::account());
}

bool TelepathyEditAccountWidget::validateData()
{
    kDebug();

    // Check there is an account edit widget. If not, then fail.
    if (!d->accountEditWidget)
        return false;

    // Validate data
    if (!d->accountEditWidget->validateParameterValues()) {
        kDebug() << "A widget failed parameter validation. Not accepting wizard.";
        return false;
    }

    return true;
}

Kopete::Account *TelepathyEditAccountWidget::apply()
{
    kDebug();

    // Check if we are applying an added account or edited account.
    if (account()) {
        // Edited an account.
        return applyEditedAccount();
    }

    // Added an account.
    return applyAddedAccount();
}

Kopete::Account *TelepathyEditAccountWidget::applyAddedAccount()
{
    kDebug();

    // Get the parameters.
    QMap<Tp::ProtocolParameter*, QVariant> parameterValues;
    parameterValues = d->accountEditWidget->parameterValues();

    // Get the ProtocolItem that was selected and the corresponding ConnectionManagerItem.
    ProtocolItem *protocolItem = d->protocolSelectWidget->selectedProtocol();
    ConnectionManagerItem *connectionManagerItem = qobject_cast<ConnectionManagerItem*>(protocolItem->parent());

    if (!connectionManagerItem) {
        kWarning() << "Invalid ConnectionManager item.";
        return 0;
    }

    // Merge the parameters into a QVariantMap for submitting to the Telepathy AM.
    QVariantMap parameters;


    foreach (Tp::ProtocolParameter *pp, parameterValues.keys()) {
        QVariant value = parameterValues.value(pp);

        // Don't try and add empty parameters or ones where the default value is still set.
        if ((!value.isNull()) && (value != pp->defaultValue())) {

            // Check for params where they are empty and the default is null.
            if (pp->type() == QVariant::String) {
                if ((pp->defaultValue() == QVariant()) && (value.toString().isEmpty())) {
                    continue;
                }
            }

            parameters.insert(pp->name(), value);
        }
    }

    setAccount(TelepathyProtocol::protocol()->createNewAccount(parameters.value("account").toString()));

    writeConfig(connectionManagerItem->connectionManager()->name(),
                protocolItem->protocol(),
                parameters);

    return account();
}

Kopete::Account *TelepathyEditAccountWidget::applyEditedAccount()
{
    kDebug();

    // Get the parameters.
    QMap<Tp::ProtocolParameter*, QVariant> parameterValues;
    parameterValues = d->accountEditWidget->parameterValues();

    // Merge the parameters into a QVariantMap for submitting to the Telepathy AM.
    QVariantMap parameters;
    QVariantMap allParameters;
    QStringList unsetParameters;

    foreach (Tp::ProtocolParameter *pp, parameterValues.keys()) {
        QVariant value = parameterValues.value(pp);

        allParameters.insert(pp->name(), value);

       // Unset null parameters.
        if (value.isNull()) {
            unsetParameters.append(pp->name());
            continue;
        }

        // Unset any parameters where the default value is equal to the current value.
        if (pp->defaultValue() == value) {
            unsetParameters.append(pp->name());
            continue;
        }

        // Unset any strings where the default is empty, and the value is an empty string
        if (pp->type() == QVariant::String) {
            if ((pp->defaultValue().isNull()) && value.toString().isEmpty()) {
                unsetParameters.append(pp->name());
                continue;
            }
        }

        // Parameter has a valid value, so set it.
        parameters.insert(pp->name(), value);
    }

    // Write the kopete config file.
    writeConfig(account()->account()->cmName(), account()->account()->protocol(), allParameters);

    // Tell the account to update with these parameters.
    account()->accountEdited(parameters, unsetParameters);

    return account();
}

void TelepathyEditAccountWidget::writeConfig(const QString &connectionManager,
                                             const QString &protocol,
                                             const QVariantMap &parameters)
{
    kDebug();

    QString accountId = account()->accountId();

    // Write basic account configuration
    KConfigGroup *basicConfig = account()->configGroup();
    basicConfig->writeEntry(QLatin1String("ConnectionManager"), connectionManager);
    basicConfig->writeEntry(QLatin1String("TelepathyProtocol"), protocol);

    // Write config related to ConnectionManager Parameters
    KConfigGroup parametersConfig = 
            KGlobal::config()->group(TelepathyProtocolInternal::protocolInternal()->
                                     formatTelepathyConfigGroup(connectionManager, 
                                                                protocol, 
                                                                accountId));

    QVariantMap::const_iterator i = parameters.constBegin();
    while (i != parameters.constEnd()) {
        parametersConfig.writeEntry(i.key(), i.value());
        ++i;
    }
}

void TelepathyEditAccountWidget::setupAddAccountUi()
{
    kDebug();

    // Set up the Add Account UI.
    d->tabWidget = new KTabWidget(this);
    d->mainLayout = new QGridLayout(this);
    d->mainLayout->addWidget(d->tabWidget);

    d->protocolSelectWidget = new ProtocolSelectWidget(d->tabWidget);
    d->tabWidget->addTab(d->protocolSelectWidget, i18n("Select Protocol"));

    connect(d->protocolSelectWidget, SIGNAL(protocolGotSelected(bool)),
            SLOT(onProtocolGotSelected(bool)));
}

void TelepathyEditAccountWidget::setupEditAccountUi()
{
    kDebug();

    // Set up the Edit Account UI.
    d->tabWidget = new KTabWidget(this);
    d->mainLayout = new QGridLayout(this);
    d->mainLayout->addWidget(d->tabWidget);
    resize(400, 480);

    if(!account()->account())
    {
        kDebug() << "Account not exist!";
        return;
    }

    // Get the protocol's parameters.
    Tp::ProtocolInfo *protocolInfo = account()->account()->protocolInfo();
    Tp::ProtocolParameterList protocolParameters = protocolInfo->parameters();

    // Get the parameter values.
    QVariantMap parameterValues = account()->account()->parameters();

    QString connectionManager = account()->account()->cmName();
    QString protocol = account()->account()->protocol();

    d->accountEditWidget = new AccountEditWidget(connectionManager,
                                                 protocol,
                                                 protocolParameters,
                                                 parameterValues,
                                                 d->tabWidget);
    d->tabWidget->addTab(d->accountEditWidget, i18n("Options"));
}

void TelepathyEditAccountWidget::onProtocolGotSelected(bool selected)
{
    kDebug();

    // If protocol was not selected, return.
    if (!selected) {
        kWarning() << "Protocol unselected.";
        return;
    }

    // Get the ProtocolItem and check it is valid.
    ProtocolItem *item = d->protocolSelectWidget->selectedProtocol();

    if (!item) {
        kWarning() << "Selected protocol is not valid.";
        return;
    }

    ConnectionManagerItem *cmItem = qobject_cast<ConnectionManagerItem*>(item->parent());
    if (!cmItem) {
        kWarning() << "cmItem is invalid.";
    }

    QString connectionManager = cmItem->connectionManager()->name();
    QString protocol = item->protocol();

    kDebug() << connectionManager;

    // Get the list of parameters
    Tp::ProtocolParameterList protocolParameters = item->parameters();

    if (d->accountEditWidget) {
        d->tabWidget->removePage(d->accountEditWidget);
        d->accountEditWidget->deleteLater();
        d->accountEditWidget = 0;
    }

    d->accountEditWidget = new AccountEditWidget(connectionManager,
                                                 protocol,
                                                 protocolParameters,
                                                 QVariantMap(),
                                                 d->tabWidget);
    d->tabWidget->addTab(d->accountEditWidget, i18n("Options"));
}

#include "telepathyeditaccountwidget.moc"

