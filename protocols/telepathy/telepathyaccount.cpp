/*
 * telepathyaccount.cpp
 *
 * Copyright (c) 2009 by Dariusz Mikulski <dariusz.mikulski@gmail.com>
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

#include "telepathyaccount.h"

// KDE includes
#include <kaction.h>
#include <kactionmenu.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kicon.h>

// Local includes
#include "telepathyprotocol.h"
#include "telepathycontact.h"
#include "telepathycontactmanager.h"
#include "telepathychatsession.h"
#include "common.h"

// Kopete includes
#include <kopetemetacontact.h>
#include <kopeteonlinestatus.h>
#include <kopetecontactlist.h>
#include <kopetechatsessionmanager.h>
#include <kopeteuiglobal.h>
#include <avatardialog.h>

#include <QtTapioca/TextChannel>

#include <TelepathyQt4/Client/Connection>
#include <TelepathyQt4/Client/AccountManager>
#include <TelepathyQt4/Client/Account>
#include <TelepathyQt4/Client/PendingReadyConnection>
#include <TelepathyQt4/Client/PendingOperation>
#include <TelepathyQt4/Client/PendingAccount>
#include <TelepathyQt4/Client/PendingReadyAccount>
#include <TelepathyQt4/Client/PendingReadyConnectionManager>
#include <TelepathyQt4/Client/PendingReadyAccountManager>

#define SHOW_MESSAGEBOX_ERRORS

TelepathyAccount::TelepathyAccount(TelepathyProtocol *protocol, const QString &accountId)
    : Kopete::Account(protocol, accountId.toLower()),
    currentConnectionManager(0), currentAccountManager(0), account(0)
{
    kDebug(TELEPATHY_DEBUG_AREA);
    Telepathy::registerTypes();
    kDebug(TELEPATHY_DEBUG_AREA);
    setMyself( new TelepathyContact(this, accountId, Kopete::ContactList::self()->myself()) );
    connectAfterInit = false;
    setStatusAfterInit = false;
}

TelepathyAccount::~TelepathyAccount()
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

void TelepathyAccount::connect (const Kopete::OnlineStatus &initialStatus)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if(!account || !account->isReady())
    {
        initTelepathyAccount();
        connectAfterInit = true;
        statusInit = initialStatus;
        return;
    }

    // \todo: here we must add setAutomaticPresence???
    if(!account->haveConnection())
    {
        kDebug(TELEPATHY_DEBUG_AREA) << "Ups! We cant connect :(";
        return;
    }
    
    QSharedPointer<Telepathy::Client::Connection> connection = account->connection();

    QObject::connect(connection->becomeReady(),
        SIGNAL(finished(Telepathy::Client::PendingOperation*)),
        this,
        SLOT(onConnectionReady(Telepathy::Client::PendingOperation*))
    );
}

void TelepathyAccount::onConnectionReady(Telepathy::Client::PendingOperation* operation)
{
    if(operation->isError())
    {
        kDebug(TELEPATHY_DEBUG_AREA) << operation->errorName() << operation->errorMessage();
#ifdef SHOW_MESSAGEBOX_ERRORS
        KMessageBox::information(0, i18n("Error: %1\n%2", operation->errorName() , operation->errorMessage()));
#endif
        return;
    }

    Telepathy::SimplePresence simplePresence;
    simplePresence.type = TelepathyProtocol::protocol()->kopeteStatusToTelepathy(statusInit);
    kDebug(TELEPATHY_DEBUG_AREA) << "Requested Presence status: " << simplePresence.type;
    simplePresence.statusMessage = reasonInit.message();
    Telepathy::Client::PendingOperation *op = account->setRequestedPresence(simplePresence);
    QObject::connect(op, SIGNAL(finished(Telepathy::Client::PendingOperation*)),
        this,
        SLOT(onRequestedPresence(Telepathy::Client::PendingOperation*))
    );

    // \todo: Add here connect(connection, statusChanged...)
}

void TelepathyAccount::onRequestedPresence(Telepathy::Client::PendingOperation* operation)
{
    if(operation->isError())
    {
        kDebug(TELEPATHY_DEBUG_AREA) << operation->errorName() << operation->errorMessage();
#ifdef SHOW_MESSAGEBOX_ERRORS
        KMessageBox::information(0, i18n("Error: %1\n%2", operation->errorName() , operation->errorMessage()));
#endif
        return;
    }

    QSharedPointer<Telepathy::Client::Connection> connection = account->connection();

    Telepathy::Client::PendingOperation *op = connection->requestConnect();
    QObject::connect(op, SIGNAL(finished(Telepathy::Client::PendingOperation*)),
        this,
        SLOT(onConnectionConnected(Telepathy::Client::PendingOperation*))
    );
}

void TelepathyAccount::onConnectionConnected(Telepathy::Client::PendingOperation*)
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

void TelepathyAccount::disconnect ()
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

void TelepathyAccount::setOnlineStatus (const Kopete::OnlineStatus &status, const Kopete::StatusMessage &reason, const OnlineStatusOptions& options)
{
    Q_UNUSED(options);
    
    kDebug(TELEPATHY_DEBUG_AREA);

    if(!account || !account->isReady())
    {
        initTelepathyAccount();
        setStatusAfterInit = true;
        statusInit = status;
        reasonInit = reason;
        return;
    }
}

void TelepathyAccount::setStatusMessage (const Kopete::StatusMessage &statusMessage)
{
    Q_UNUSED(statusMessage);
    
    kDebug(TELEPATHY_DEBUG_AREA);
}

bool TelepathyAccount::createContact (const QString &contactId, Kopete::MetaContact *parentContact)
{
    Q_UNUSED(contactId);
    Q_UNUSED(parentContact);
    
    kDebug(TELEPATHY_DEBUG_AREA);
    return false;
}

QtTapioca::TextChannel *TelepathyAccount::createTextChannel(QtTapioca::Contact *internalContact)
{
    Q_UNUSED(internalContact);
    
    kDebug(TELEPATHY_DEBUG_AREA);
    return NULL;
}

QString TelepathyAccount::connectionManager()
{
    kDebug(TELEPATHY_DEBUG_AREA);
    return QString();
}

QString TelepathyAccount::connectionProtocol()
{
    kDebug(TELEPATHY_DEBUG_AREA);
    return QString();
}

bool TelepathyAccount::readConfig()
{
    kDebug(TELEPATHY_DEBUG_AREA);

    // Restore config not related to ConnectionManager parameters first
    // so that the UI for the protocol parameters will be generated
    KConfigGroup *accountConfig = configGroup();
    connectionManagerName = accountConfig->readEntry( QLatin1String("ConnectionManager"), QString() );
    connectionProtocolName = accountConfig->readEntry( QLatin1String("SelectedProtocol"), QString() );

    // Clear current connection parameters
    m_allConnectionParameters.clear();
    connectionParameters.clear();

    // Get the preferences from the connection manager to get the right types
    Telepathy::Client::ProtocolInfo* protocolInfo = getProtocolInfo(getConnectionManager(), connectionProtocolName);
    if(!protocolInfo)
    {
        kDebug(TELEPATHY_DEBUG_AREA) << "Error: could not get protocol info" << connectionProtocolName;
        return false;
    }

    Telepathy::Client::ProtocolParameterList tempParameters = protocolInfo->parameters();

    // Now update the preferences
    KSharedConfig::Ptr telepathyConfig = KGlobal::config();
    QMap<QString,QString> allEntries = telepathyConfig->entryMap( TelepathyProtocol::protocol()->formatTelepathyConfigGroup(connectionManagerName, connectionProtocolName, accountId()));
    QMap<QString,QString>::ConstIterator it, itEnd = allEntries.constEnd();
    for(it = allEntries.constBegin(); it != itEnd; ++it)
    {
        foreach(Telepathy::Client::ProtocolParameter *parameter, tempParameters)
        {
            if( parameter->name() == it.key() )
            {
                if( parameter->defaultValue().toString() != it.value() )
                {
                    QVariant oldValue = parameter->defaultValue();
                    QVariant newValue(oldValue.type());
                    if ( oldValue.type() == QVariant::String )
                        newValue = QVariant(it.value());
                    else if( oldValue.type() == QVariant::Int )
                        newValue = QVariant(it.value()).toInt();
                    else if( oldValue.type() == QVariant::UInt )
                        newValue = QVariant(it.value()).toUInt();
                    else if( oldValue.type() == QVariant::Double )
                        newValue = QVariant(it.value()).toDouble();
                    else if( oldValue.type() == QVariant::Bool)
                    {
                        if( it.value().toLower() == "true")
                            newValue = true;
                        else
                            newValue = false;
                        }
                        else
                            newValue = QVariant(it.value());

                        kDebug(TELEPATHY_DEBUG_AREA) << "Name: " << parameter->name() << " Value: " << newValue << "Type: " << parameter->defaultValue().typeName();
                        connectionParameters.append(
                            new Telepathy::Client::ProtocolParameter(
                                parameter->name(),
                                parameter->dbusSignature(),
                                newValue,
                                Telepathy::ConnMgrParamFlagHasDefault
                            ));
                        break;
                    }
                }
            }
        }

        if( !connectionManagerName.isEmpty() &&
            !connectionProtocolName.isEmpty() &&
            !connectionParameters.isEmpty() )
                return true;
        else
            return false;
}

Telepathy::Client::ProtocolParameterList TelepathyAccount::allConnectionParameters()
{
    kDebug(TELEPATHY_DEBUG_AREA);
    
    if( m_allConnectionParameters.isEmpty() )
    {
        if( connectionProtocolName.isEmpty() )
            readConfig();

        Telepathy::Client::ProtocolInfo *protocolInfo = getProtocolInfo(getConnectionManager(), connectionProtocolName);
        Telepathy::Client::ProtocolParameterList allParameters = protocolInfo->parameters();
        foreach(Telepathy::Client::ProtocolParameter *parameter, allParameters)
        {
            Telepathy::Client::ProtocolParameter *newParameter = parameter;
            foreach(Telepathy::Client::ProtocolParameter *connectionParameter, connectionParameters)
            {
                // Use value from the saved connection parameter
                if( parameter->name() == connectionParameter->name() )
                {
                    newParameter = new Telepathy::Client::ProtocolParameter(
                    parameter->name(), parameter->dbusSignature(), connectionParameter->defaultValue(), Telepathy::ConnMgrParamFlagHasDefault);
                    break;
                }
             }
            m_allConnectionParameters.append( newParameter );
         }
    }

    return m_allConnectionParameters;
}

Telepathy::Client::ConnectionManager *TelepathyAccount::getConnectionManager()
{
    if(!currentConnectionManager)
        currentConnectionManager = new Telepathy::Client::ConnectionManager(connectionManagerName);

    return currentConnectionManager;
}

Telepathy::Client::AccountManager *TelepathyAccount::getAccountManager()
{
    if(!currentAccountManager)
        currentAccountManager = new Telepathy::Client::AccountManager(QDBusConnection::sessionBus());

    return currentAccountManager;
}

void TelepathyAccount::initTelepathyAccount()
{
    kDebug(TELEPATHY_DEBUG_AREA);

    // Restore config not related to ConnectionManager parameters first
    // so that the UI for the protocol parameters will be generated
    KConfigGroup *accountConfig = configGroup();
    connectionManagerName = accountConfig->readEntry( QLatin1String("ConnectionManager"), QString() );
    connectionProtocolName = accountConfig->readEntry( QLatin1String("SelectedProtocol"), QString() );

    // \brief: init managers early
    Telepathy::Client::ConnectionManager *cm = getConnectionManager();
    QObject::connect(cm->becomeReady(),
        SIGNAL(finished(Telepathy::Client::PendingOperation*)),
        this,
        SLOT(onConnectionManagerReady(Telepathy::Client::PendingOperation*))
    );
}

void TelepathyAccount::onConnectionManagerReady(Telepathy::Client::PendingOperation* operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if(operation->isError())
    {
        kDebug(TELEPATHY_DEBUG_AREA) << operation->errorName() << operation->errorMessage();
#ifdef SHOW_MESSAGEBOX_ERRORS
        KMessageBox::information(0, i18n("Error: %1\n%2", operation->errorName() , operation->errorMessage()));
#endif
        return;
    }

    if(readConfig())
    {
        Telepathy::Client::AccountManager *accountManager = getAccountManager();
            QObject::connect(accountManager->becomeReady(),
            SIGNAL(finished(Telepathy::Client::PendingOperation*)),
            this,
            SLOT(onAccountManagerReady(Telepathy::Client::PendingOperation*))
        );
    }
    else
    {
        // \brief: problem with config? So here we can die.
        kDebug(TELEPATHY_DEBUG_AREA) << "Init connection manager failed!";
        delete currentConnectionManager;
        currentConnectionManager = 0;
        delete currentAccountManager;
        currentAccountManager = 0;
    }
}

void TelepathyAccount::onAccountManagerReady(Telepathy::Client::PendingOperation* operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if(operation->isError())
    {
        kDebug() << operation->errorName() << ": " << operation->errorMessage();
#ifdef SHOW_MESSAGEBOX_ERRORS
        KMessageBox::information(0, i18n("Error: %1\n%2", operation->errorName() , operation->errorMessage()));
#endif
        return;
    }

    /*
     * get a list of all the accounts that
     * are all ready there
     */
    QList<QSharedPointer<Telepathy::Client::Account> > accounts = currentAccountManager->allAccounts();
    kDebug(TELEPATHY_DEBUG_AREA) << "accounts: " << accounts.size();

    /*
     * check if account already exist
     */
    foreach(QSharedPointer<Telepathy::Client::Account> a, accounts)
    {
        if(a->displayName() == accountId() && a->protocol() == connectionProtocolName)
        {
            kDebug(TELEPATHY_DEBUG_AREA) << "Account already exist " << accountId();
            account = a;
            return;
        }
    }

    if(!account)
    {
        QVariantMap parameters;
        foreach(Telepathy::Client::ProtocolParameter *parameter, connectionParameters)
        {
            kDebug(TELEPATHY_DEBUG_AREA) << parameter->name() << parameter->defaultValue().toString();
            parameters[parameter->name()] = parameter->defaultValue();
        }

        kDebug(TELEPATHY_DEBUG_AREA) << "Creating account: " << connectionManagerName << connectionProtocolName << accountId() << parameters;
        Telepathy::Client::PendingAccount *paccount =
            currentAccountManager->createAccount(connectionManagerName, connectionProtocolName, accountId(), parameters);

        QObject::connect(paccount, SIGNAL(finished(Telepathy::Client::PendingOperation *)),
                         this, SLOT(newTelepathyAccountCreated(Telepathy::Client::PendingOperation *)));
    }
}

void TelepathyAccount::newTelepathyAccountCreated(Telepathy::Client::PendingOperation *operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if(operation->isError())
    {
        kDebug(TELEPATHY_DEBUG_AREA) << "Error: " << operation->errorName() << operation->errorMessage();
#ifdef SHOW_MESSAGEBOX_ERRORS
        KMessageBox::information(0, i18n("Error: %1\n%2", operation->errorName() , operation->errorMessage()));
#endif

        
        return;
    }

    Telepathy::Client::PendingAccount *paccount = static_cast<Telepathy::Client::PendingAccount *>(operation);
    account = paccount->account();

    QObject::connect(account->becomeReady(), SIGNAL(finished(Telepathy::Client::PendingOperation *)),
        this, SLOT(onAccountReady(Telepathy::Client::PendingOperation *)));
}

void TelepathyAccount::onAccountReady(Telepathy::Client::PendingOperation *operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if(operation->isError())
    {
        kDebug(TELEPATHY_DEBUG_AREA) << "Error: " << operation->errorName() << operation->errorMessage();
#ifdef SHOW_MESSAGEBOX_ERRORS
        KMessageBox::information(0, i18n("Error: %1\n%2", operation->errorName() , operation->errorMessage()));
#endif
        account = QSharedPointer<Telepathy::Client::Account>();
        return;
    }

    kDebug(TELEPATHY_DEBUG_AREA) << "New account created: " << account->cmName() << account->protocol() << account->displayName();

    if(connectAfterInit)
    {
        connectAfterInit = false;
        connect(statusInit);
    }
    else if(setStatusAfterInit)
    {
        setStatusAfterInit = false;
        setOnlineStatus(statusInit, reasonInit);
    }
}





