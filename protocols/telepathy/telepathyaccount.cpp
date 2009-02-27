/*
 * This file is part of Kopete
 *
 * Copyright (C) 2009 Collabora Ltd. <http://www.collabora.co.uk/>
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

#include "telepathyaccount.h"
#include "telepathyprotocol.h"
#include "telepathycontact.h"

#include <kopetemetacontact.h>
#include <kopetecontactlist.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kconfig.h>
#include <ksharedconfig.h>

#include <TelepathyQt4/Client/Account>
#include <TelepathyQt4/Client/PendingReadyAccountManager>
#include <TelepathyQt4/Client/PendingReadyConnectionManager>
#include <TelepathyQt4/Client/PendingReadyAccount>

TelepathyAccount::TelepathyAccount(TelepathyProtocol *protocol, const QString &accountId)
    : Kopete::Account(protocol, accountId), m_connectionManager(0), m_accountManager(0),
	m_existingAccountCounter(0), m_existingAccountsCount(0)
{
    kDebug(TELEPATHY_DEBUG_AREA);
	
	m_setStatusAfterInit = false;

    setMyself( new TelepathyContact(this, accountId, Kopete::ContactList::self()->myself()) );
}

TelepathyAccount::~TelepathyAccount()
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

bool TelepathyAccount::isOperationError(Telepathy::Client::PendingOperation* operation)
{
    if(operation->isError())
    {
        kDebug(TELEPATHY_DEBUG_AREA) << operation->errorName() << operation->errorMessage();
#ifdef SHOW_MESSAGEBOX_ERRORS
        KMessageBox::information(0, i18n("Error: %1\n%2", operation->errorName() , operation->errorMessage()));
#endif
        return true;
    }

    return false;
}

void TelepathyAccount::connect (const Kopete::OnlineStatus &initialStatus)
{
    kDebug(TELEPATHY_DEBUG_AREA);
	
	if(!m_account || !m_account->becomeReady())
	{
		m_initialStatus = initialStatus;
		initTelepathyAccount();
		return;
	}
	
	kDebug(TELEPATHY_DEBUG_AREA) << m_account->parameters();

    Telepathy::SimplePresence simplePresence;
//    simplePresence.type = TelepathyProtocol::protocol()->kopeteStatusToTelepathy(statusInit);

    kDebug(TELEPATHY_DEBUG_AREA) << "Requested Presence status: " << simplePresence.type;
    
    simplePresence.statusMessage = m_reason.message();
/*    
    Telepathy::Client::PendingOperation *op = account->setRequestedPresence(simplePresence);
    QObject::connect(op, SIGNAL(finished(Telepathy::Client::PendingOperation*)),
        this,
        SLOT(onRequestedPresence(Telepathy::Client::PendingOperation*))
    );*/
}

void TelepathyAccount::disconnect ()
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

void TelepathyAccount::setOnlineStatus (const Kopete::OnlineStatus &status, const Kopete::StatusMessage &reason, const OnlineStatusOptions& options)
{
    kDebug(TELEPATHY_DEBUG_AREA);
	
	Q_UNUSED(options);
	
	m_status = status;
	m_reason = reason;
	m_setStatusAfterInit = true;
}

void TelepathyAccount::setStatusMessage (const Kopete::StatusMessage &statusMessage)
{
    kDebug(TELEPATHY_DEBUG_AREA);
	
	Q_UNUSED(statusMessage);
}

bool TelepathyAccount::createContact( const QString &contactId, Kopete::MetaContact *parentContact )
{
    kDebug(TELEPATHY_DEBUG_AREA);
	
	Q_UNUSED(contactId);
	Q_UNUSED(parentContact);
	
    return false;
}

bool TelepathyAccount::readConfig()
{
    kDebug(TELEPATHY_DEBUG_AREA);

    // Restore config not related to ConnectionManager parameters first
    // so that the UI for the protocol parameters will be generated
    KConfigGroup *accountConfig = configGroup();
    m_connectionManagerName = accountConfig->readEntry( QLatin1String("ConnectionManager"), QString() );
	// setup CM early
	getConnectionManager();
	
    m_connectionProtocolName = accountConfig->readEntry( QLatin1String("SelectedProtocol"), QString() );

    // Clear current connection parameters
    m_connectionParameters.clear();

    // Get the preferences from the connection manager to get the right types
    Telepathy::Client::ProtocolInfo* protocolInfo = getProtocolInfo(m_connectionProtocolName);
    if(!protocolInfo)
    {
        kDebug(TELEPATHY_DEBUG_AREA) << "Error: could not get protocol info" << m_connectionProtocolName;
        return false;
    }

    Telepathy::Client::ProtocolParameterList tempParameters = protocolInfo->parameters();

    // Now update the preferences
    KSharedConfig::Ptr telepathyConfig = KGlobal::config();
    QMap<QString,QString> allEntries = telepathyConfig->entryMap( TelepathyProtocol::protocol()->formatTelepathyConfigGroup(m_connectionManagerName, m_connectionProtocolName, accountId()));
    QMap<QString,QString>::ConstIterator it, itEnd = allEntries.constEnd();
    for(it = allEntries.constBegin(); it != itEnd; ++it)
    {
        foreach(Telepathy::Client::ProtocolParameter *parameter, tempParameters)
        {
            if( parameter->name() == it.key() )
            {
                kDebug(TELEPATHY_DEBUG_AREA) << parameter->defaultValue().toString() << it.value();
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
                    m_connectionParameters.append(
                        new Telepathy::Client::ProtocolParameter(
                            parameter->name(),
                            parameter->dbusSignature(),
                            newValue,
                            Telepathy::ConnMgrParamFlagHasDefault
                        ));
                }
                else
                {
                    kDebug(TELEPATHY_DEBUG_AREA) << parameter->name() << parameter->defaultValue();

                    m_connectionParameters.append(
                        new Telepathy::Client::ProtocolParameter(
                            parameter->name(),
                            parameter->dbusSignature(),
                            parameter->defaultValue(),
                            Telepathy::ConnMgrParamFlagHasDefault
                        ));
                }
            }
        }
    }

    kDebug(TELEPATHY_DEBUG_AREA) << m_connectionManagerName << m_connectionProtocolName << m_connectionParameters;
    if( !m_connectionManagerName.isEmpty() &&
        !m_connectionProtocolName.isEmpty() &&
        !m_connectionParameters.isEmpty() )
            return true;
    else
        return false;
}

QString TelepathyAccount::connectionProtocol() const
{
    kDebug(TELEPATHY_DEBUG_AREA);
	return m_connectionProtocolName;
}

Telepathy::Client::ProtocolParameterList TelepathyAccount::allConnectionParameters() const
{
    kDebug(TELEPATHY_DEBUG_AREA);
	return m_connectionParameters;
}

Telepathy::Client::AccountManager *TelepathyAccount::getAccountManager()
{
    kDebug(TELEPATHY_DEBUG_AREA);
    if(!m_accountManager)
        m_accountManager = new Telepathy::Client::AccountManager(QDBusConnection::sessionBus());

    return m_accountManager;
}

Telepathy::Client::ConnectionManager *TelepathyAccount::getConnectionManager()
{
    kDebug(TELEPATHY_DEBUG_AREA);
	if(!m_connectionManager)
	{
		m_connectionManager = new Telepathy::Client::ConnectionManager(m_connectionManagerName);
		// dont need wait until operation finished
		m_connectionManager->becomeReady();
	}
	
	return m_connectionManager;
}

Telepathy::Client::ProtocolInfo *TelepathyAccount::getProtocolInfo(QString protocol)
{
	Telepathy::Client::ProtocolInfoList protocolList = getConnectionManager()->protocols();
	kDebug(TELEPATHY_DEBUG_AREA) << protocolList.size();
    foreach(Telepathy::Client::ProtocolInfo* protocolInfo, protocolList)
    {
		if(protocolInfo->name() == protocol)
		{
			kDebug(TELEPATHY_DEBUG_AREA) << protocolInfo->name();
	        return protocolInfo;
		}
    }
	return NULL;
}

void TelepathyAccount::initTelepathyAccount()
{
    kDebug(TELEPATHY_DEBUG_AREA);

	// Restore config not related to ConnectionManager parameters first
    // so that the UI for the protocol parameters will be generated
    KConfigGroup *accountConfig = configGroup();
    m_connectionManagerName = accountConfig->readEntry( QLatin1String("ConnectionManager"), QString() );
    m_connectionProtocolName = accountConfig->readEntry( QLatin1String("SelectedProtocol"), QString() );
	
	// \brief: init managers early, it needed here becouse later i want 
	//	  to get available protocols from CM
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
    
    if(isOperationError(operation))
        return;

    if(readConfig())
    {
        Telepathy::Client::AccountManager *accountManager = getAccountManager();
            QObject::connect(accountManager->becomeReady(),
            SIGNAL(finished(Telepathy::Client::PendingOperation*)),
            this,
            SLOT(onAccountManagerReady(Telepathy::Client::PendingOperation*))
        );
		return;
    }

	// \brief: problem with config? So here we can die.
    kDebug(TELEPATHY_DEBUG_AREA) << "Init connection manager failed!";
}

void TelepathyAccount::onAccountManagerReady(Telepathy::Client::PendingOperation* operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if(isOperationError(operation))
        return;

    /*
     * get a list of all the accounts that
     * are all ready there
     */
    QStringList pathList = m_accountManager->allAccountPaths();
    kDebug(TELEPATHY_DEBUG_AREA) << "accounts: " << pathList.size() << pathList;
    m_existingAccountsCount = pathList.size();
    if(m_existingAccountsCount != 0)
        m_existingAccountCounter++;
    else
    {
        createNewAccount();
    }

    /*
     * check if account already exist
     */
    foreach(const QString &path, pathList)
    {
        QSharedPointer<Telepathy::Client::Account> a = m_accountManager->accountForPath(path);
        QObject::connect(a->becomeReady(), SIGNAL(finished(Telepathy::Client::PendingOperation *)),
            this, SLOT(onExistingAccountReady(Telepathy::Client::PendingOperation *)));
    }
}

void TelepathyAccount::onExistingAccountReady(Telepathy::Client::PendingOperation *operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if(isOperationError(operation))
        return;

    Telepathy::Client::PendingReadyAccount *pa = dynamic_cast<Telepathy::Client::PendingReadyAccount *>(operation);
    if(!pa)
        return;

    Telepathy::Client::Account *a = pa->account();

    if( !m_account && ((a->displayName() == accountId()) && (a->protocol() == m_connectionProtocolName)) )
    {
        kDebug(TELEPATHY_DEBUG_AREA) << "Account already exist " << a->cmName() << m_connectionManagerName << a->displayName() << accountId() << a->protocol() << m_connectionProtocolName << m_existingAccountCounter;
        m_account = QSharedPointer<Telepathy::Client::Account>(a);

        QObject::connect(m_account->becomeReady(), SIGNAL(finished(Telepathy::Client::PendingOperation *)),
            this, SLOT(onAccountReady(Telepathy::Client::PendingOperation *)));
        
        return;
    }

    if( !m_account && (m_existingAccountCounter == m_existingAccountsCount) )
    {
        createNewAccount();
    }
    m_existingAccountCounter++;
}

void TelepathyAccount::createNewAccount()
{
    kDebug(TELEPATHY_DEBUG_AREA);

    QVariantMap parameters;
    foreach(Telepathy::Client::ProtocolParameter *parameter, m_connectionParameters)
    {
        kDebug(TELEPATHY_DEBUG_AREA) << parameter->name() << parameter->defaultValue().toString();
        parameters[parameter->name()] = parameter->defaultValue();
    }

    kDebug(TELEPATHY_DEBUG_AREA) << "Creating account: " << m_connectionManagerName << m_connectionProtocolName << accountId() << parameters;
    m_pendingAccount = m_accountManager->createAccount(m_connectionManagerName, m_connectionProtocolName, accountId(), parameters);

    QObject::connect(m_pendingAccount, SIGNAL(finished(Telepathy::Client::PendingOperation *)),
        this, SLOT(newTelepathyAccountCreated(Telepathy::Client::PendingOperation *)));
}

void TelepathyAccount::newTelepathyAccountCreated(Telepathy::Client::PendingOperation *operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    // \brief: zeroing counter
    m_existingAccountCounter = 0;

    if(isOperationError(operation))
        return;

    m_account = m_pendingAccount->account();

    QObject::connect(m_account->becomeReady(), SIGNAL(finished(Telepathy::Client::PendingOperation *)),
        this, SLOT(onAccountReady(Telepathy::Client::PendingOperation *)));
}

void TelepathyAccount::onAccountReady(Telepathy::Client::PendingOperation *operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if(isOperationError(operation))
        return;

    kDebug(TELEPATHY_DEBUG_AREA) << "New account: " << m_account->cmName() << m_account->protocol() << m_account->displayName();
	
	Telepathy::Client::Account *a = m_account.data();

    QObject::connect(a, SIGNAL(displayNameChanged(const QString &)),
        this, SLOT(displayNameChanged(const QString &)));
    QObject::connect(a, SIGNAL(iconChanged (const QString &)),
        this, SLOT(iconChanged (const QString &)));
    QObject::connect(a, SIGNAL(nicknameChanged (const QString &)),
        this, SLOT(nicknameChanged (const QString &)));
    QObject::connect(a, SIGNAL(normalizedNameChanged (const QString &)),
        this, SLOT(normalizedNameChanged (const QString &)));
    QObject::connect(a, SIGNAL(validityChanged (bool)),
        this, SLOT(validityChanged (bool)));
    QObject::connect(a, SIGNAL(stateChanged (bool)),
        this, SLOT(stateChanged (bool)));
    QObject::connect(a, SIGNAL(connectsAutomaticallyPropertyChanged (bool)),
        this, SLOT(connectsAutomaticallyPropertyChanged (bool)));
    QObject::connect(a, SIGNAL(parametersChanged (const QVariantMap &)),
        this, SLOT(parametersChanged (const QVariantMap &)));
    QObject::connect(a, SIGNAL(automaticPresenceChanged (const Telepathy::SimplePresence &)),
        this, SLOT(automaticPresenceChanged (const Telepathy::SimplePresence &)));
    QObject::connect(a, SIGNAL(currentPresenceChanged (const Telepathy::SimplePresence &)),
        this, SLOT(currentPresenceChanged (const Telepathy::SimplePresence &)));
    QObject::connect(a, SIGNAL(requestedPresenceChanged (const Telepathy::SimplePresence &)),
        this, SLOT(requestedPresenceChanged (const Telepathy::SimplePresence &)));
    QObject::connect(a, SIGNAL(avatarChanged (const Telepathy::Avatar &)),
        this, SLOT(avatarChanged (const Telepathy::Avatar &)));
    QObject::connect(a, SIGNAL(connectionStatusChanged (Telepathy::ConnectionStatus, Telepathy::ConnectionStatusReason)),
        this, SLOT(connectionStatusChanged (Telepathy::ConnectionStatus, Telepathy::ConnectionStatusReason)));
    QObject::connect(a, SIGNAL(haveConnectionChanged (bool)),
        this, SLOT(haveConnectionChanged (bool)));

	if(m_setStatusAfterInit)
    {
        m_setStatusAfterInit = false;
        setOnlineStatus(m_status, m_reason);
    }
	else
	{
	    connect(m_initialStatus);
	}
}

void TelepathyAccount::displayNameChanged (const QString &var)
{
    kDebug(TELEPATHY_DEBUG_AREA) << var;
}

void TelepathyAccount::iconChanged (const QString &var)
{
    kDebug(TELEPATHY_DEBUG_AREA) << var;
}

void TelepathyAccount::nicknameChanged (const QString &var)
{
    kDebug(TELEPATHY_DEBUG_AREA) << var;
}

void TelepathyAccount::normalizedNameChanged (const QString &var)
{
    kDebug(TELEPATHY_DEBUG_AREA) << var;
}

void TelepathyAccount::validityChanged (bool var)
{
    kDebug(TELEPATHY_DEBUG_AREA) << var;
}

void TelepathyAccount::stateChanged (bool var)
{
    kDebug(TELEPATHY_DEBUG_AREA) << var;
}

void TelepathyAccount::connectsAutomaticallyPropertyChanged (bool var)
{
    kDebug(TELEPATHY_DEBUG_AREA) << var;
}

void TelepathyAccount::parametersChanged (const QVariantMap &var)
{
    kDebug(TELEPATHY_DEBUG_AREA) << var;
}

void TelepathyAccount::automaticPresenceChanged (const Telepathy::SimplePresence &) const
{
    kDebug(TELEPATHY_DEBUG_AREA) ;
}

void TelepathyAccount::currentPresenceChanged (const Telepathy::SimplePresence &) const
{
    kDebug(TELEPATHY_DEBUG_AREA) ;
}

void TelepathyAccount::requestedPresenceChanged (const Telepathy::SimplePresence &) const
{
    kDebug(TELEPATHY_DEBUG_AREA) ;
}

void TelepathyAccount::avatarChanged (const Telepathy::Avatar &)
{
    kDebug(TELEPATHY_DEBUG_AREA) ;
}

void TelepathyAccount::connectionStatusChanged (Telepathy::ConnectionStatus status, Telepathy::ConnectionStatusReason reason)
{
    kDebug(TELEPATHY_DEBUG_AREA) ;
    Q_UNUSED(reason);

	switch(status)
	{
        case Telepathy::ConnectionStatusConnecting:
			kDebug(TELEPATHY_DEBUG_AREA) << "Connecting....";
			break;
        case Telepathy::ConnectionStatusConnected:
			kDebug(TELEPATHY_DEBUG_AREA) << "Connected using Telepathy :)";
            // Set initial status to myself contact
            myself()->setOnlineStatus( m_status );
            // Set nickname to myself contact
            //myself()->setNickName( d->currentConnection->userContact()->alias() );
            // Load contact list
            //fetchContactList();

			break;
        case Telepathy::ConnectionStatusDisconnected:
			kDebug(TELEPATHY_DEBUG_AREA) << "Disconnected :(";
			break;
	}

    // \todo: reason
}

void TelepathyAccount::haveConnectionChanged (bool haveConnection)
{
    kDebug(TELEPATHY_DEBUG_AREA) << haveConnection;

    if(haveConnection)
    {
        kDebug(TELEPATHY_DEBUG_AREA) << "Have connection.";
    }
}
