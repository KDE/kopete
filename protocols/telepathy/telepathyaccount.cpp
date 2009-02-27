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

TelepathyAccount::TelepathyAccount(TelepathyProtocol *protocol, const QString &accountId)
    : Kopete::Account(protocol, accountId), m_connectionManager(0), m_accountManager(0),
	m_existingAccountCounter(0), m_existingAccountsCount(0)
{
    kDebug(TELEPATHY_DEBUG_AREA);

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
}

void TelepathyAccount::disconnect ()
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

void TelepathyAccount::setOnlineStatus (const Kopete::OnlineStatus &status, const Kopete::StatusMessage &reason, const OnlineStatusOptions& options)
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

void TelepathyAccount::setStatusMessage (const Kopete::StatusMessage &statusMessage)
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

bool TelepathyAccount::createContact( const QString &contactId, Kopete::MetaContact *parentContact )
{
    kDebug(TELEPATHY_DEBUG_AREA);
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
    }
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
/*        QSharedPointer<Telepathy::Client::Account> a = m_accountManager->accountForPath(path);
        QObject::connect(a->becomeReady(), SIGNAL(finished(Telepathy::Client::PendingOperation *)),
            this, SLOT(onExistingAccountReady(Telepathy::Client::PendingOperation *)));*/
    }
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
/*
    QObject::connect(account->becomeReady(), SIGNAL(finished(Telepathy::Client::PendingOperation *)),
        this, SLOT(onAccountReady(Telepathy::Client::PendingOperation *)));*/
}



