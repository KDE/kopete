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
#include "telepathycontactmanager.h"

#include <kopetemetacontact.h>
#include <kopetecontactlist.h>
#include <kopeteuiglobal.h>
#include <avatardialog.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kconfig.h>
#include <ksharedconfig.h>
#include <kicon.h>
#include <klocale.h>
#include <kaction.h>
#include <kactionmenu.h>
#include <kinputdialog.h>

#include <TelepathyQt4/Types>
#include <TelepathyQt4/Client/Account>
#include <TelepathyQt4/Client/Connection>
#include <TelepathyQt4/Client/PendingReadyAccountManager>
#include <TelepathyQt4/Client/PendingReadyConnectionManager>
#include <TelepathyQt4/Client/PendingReadyAccount>
#include <TelepathyQt4/Client/PendingReady>

#include <QFile>

TelepathyAccount::TelepathyAccount(TelepathyProtocol *protocol, const QString &accountId)
    : Kopete::Account(protocol, accountId), m_connectionManager(0), m_accountManager(0),
	m_existingAccountCounter(0), m_existingAccountsCount(0), m_contactManager(0)
{
    kDebug(TELEPATHY_DEBUG_AREA);
	
	m_setStatusAfterInit = false;

    setMyself( new TelepathyContact(this, accountId, Kopete::ContactList::self()->myself()) );
}

TelepathyAccount::~TelepathyAccount()
{
    kDebug(TELEPATHY_DEBUG_AREA);
	if(m_contactManager)
		delete m_contactManager;
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
		m_status = initialStatus;
		initTelepathyAccount();
		return;
	}
	
	if(m_account->haveConnection())
		kDebug(TELEPATHY_DEBUG_AREA) << "Account have connection";
	
	kDebug(TELEPATHY_DEBUG_AREA) << m_account->parameters();

    Telepathy::SimplePresence simplePresence;
    simplePresence.type = TelepathyProtocol::protocol()->kopeteStatusToTelepathy(m_status);

    simplePresence.statusMessage = m_reason.message();

	kDebug(TELEPATHY_DEBUG_AREA) << "Requested Presence status: " << simplePresence.type << "message:" << simplePresence.statusMessage;
    
    Telepathy::Client::PendingOperation *op = m_account->setRequestedPresence(simplePresence);
    QObject::connect(op, SIGNAL(finished(Telepathy::Client::PendingOperation*)),
        this,
        SLOT(onRequestedPresence(Telepathy::Client::PendingOperation*))
    );
}

void TelepathyAccount::onRequestedPresence(Telepathy::Client::PendingOperation* operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if(isOperationError(operation))
        return;

    QObject::connect(m_account->setConnectsAutomatically(true),
        SIGNAL(finished(Telepathy::Client::PendingOperation*)),
        this,
        SLOT(onAccountConnecting(Telepathy::Client::PendingOperation*))
    );
}

void TelepathyAccount::onAccountConnecting(Telepathy::Client::PendingOperation* operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if(isOperationError(operation))
        return;
	
	if(m_account->haveConnection())
		kDebug(TELEPATHY_DEBUG_AREA) << "Account have connection";
}

void TelepathyAccount::fillActionMenu( KActionMenu *actionMenu )
{
	Kopete::Account::fillActionMenu( actionMenu );

	KAction *changeAliasAction = new KAction( KIcon("edit-rename"), i18n("&Change Alias..."), 0 );
	changeAliasAction->setEnabled( isConnected() );
	QObject::connect(changeAliasAction, SIGNAL(triggered(bool)), this, SLOT(slotSetAlias()));

	KAction *changeAvatarAction = new KAction( KIcon("user-properties"), i18n("Change &Avatar..."), 0 );
	changeAvatarAction->setEnabled( isConnected() );
	QObject::connect(changeAvatarAction, SIGNAL(triggered(bool)), this, SLOT(slotChangeAvatar()));

	actionMenu->addSeparator();
	actionMenu->addAction( changeAliasAction );
	actionMenu->addAction( changeAvatarAction );
}

void TelepathyAccount::disconnect ()
{
    kDebug(TELEPATHY_DEBUG_AREA);
	
	if(!m_account || !m_account->haveConnection())
		return;
	
	QSharedPointer<Telepathy::Client::Connection> connection = m_account->connection();
	
	QObject::connect(connection->requestDisconnect(),
		SIGNAL(finished(Telepathy::Client::PendingOperation*)),
        this,
		SLOT(onRequestDisconnect(Telepathy::Client::PendingOperation*))
    );
}

void TelepathyAccount::onRequestDisconnect(Telepathy::Client::PendingOperation* operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);
    
    if(isOperationError(operation))
        return;
}

void TelepathyAccount::setOnlineStatus (const Kopete::OnlineStatus &status, const Kopete::StatusMessage &reason, const OnlineStatusOptions& options)
{
    kDebug(TELEPATHY_DEBUG_AREA);
	
	Q_UNUSED(options);

	m_status = status;
	m_reason = reason;

    if(!m_account || !m_account->isReady())
	{
		m_setStatusAfterInit = true;
		initTelepathyAccount();
		return;
	}

	if(!isConnected())
	{
		connect(status);
	}
	else if(status.status() == Kopete::OnlineStatus::Offline)
	{
		disconnect();
	}
	else
	{
	    Telepathy::SimplePresence simplePresence;
		simplePresence.type = TelepathyProtocol::protocol()->kopeteStatusToTelepathy(status);

	    kDebug(TELEPATHY_DEBUG_AREA) << "Requested Presence status: " << simplePresence.type << reason.message();

		simplePresence.statusMessage = reason.message();

	    Telepathy::Client::PendingOperation *op = m_account->setRequestedPresence(simplePresence);
		QObject::connect(op,
			SIGNAL(finished(Telepathy::Client::PendingOperation*)),
	        this,
			SLOT(onRequestedPresence(Telepathy::Client::PendingOperation*))
	    );
	}
}

void TelepathyAccount::setStatusMessage (const Kopete::StatusMessage &reason)
{
    kDebug(TELEPATHY_DEBUG_AREA);
	
	setOnlineStatus(m_status, reason);
}

bool TelepathyAccount::createContact( const QString &contactId, Kopete::MetaContact *parentContact )
{
    kDebug(TELEPATHY_DEBUG_AREA);
	
	if( !contacts()[contactId] )
	{
		TelepathyContact *contact = new TelepathyContact(this, contactId, parentContact);
		
		return contact != 0;
	}
	else
	{
		kDebug(TELEPATHY_DEBUG_AREA) << "Contact " << contactId << " already exists.";
	}

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

TelepathyContactManager *TelepathyAccount::getContactManager()
{
	if(!m_contactManager)
		m_contactManager = new TelepathyContactManager(this);
	
	return m_contactManager;
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

    Telepathy::Client::PendingReady *p = qobject_cast<Telepathy::Client::PendingReady *>(operation);
    if(!p)
	{
		kDebug(TELEPATHY_DEBUG_AREA) << "Error: problem with casting";
        return;
	}

    Telepathy::Client::Account *a = qobject_cast<Telepathy::Client::Account*>(p->object());
	if(!a)
	{
		kDebug(TELEPATHY_DEBUG_AREA) << "Error: problem with casting";
		return;
	}

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
	    connect(m_status);
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
	
	myself()->setNickName( var );
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
            myself()->setNickName( m_account->nickname() );
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

void TelepathyAccount::slotSetAlias()
{
	QString currentAlias = myself()->nickName();

	bool ok = false;
	QString newAlias = KInputDialog::getText(
			i18n("Change alias"), 
			i18n("Enter the new alias by which you want to be visible to your friends:"), 
			currentAlias,
			&ok );

	if(!ok || !m_account)
		return;
	
	QObject::connect(m_account->setNickname(newAlias), SIGNAL(finished(Telepathy::Client::PendingOperation *)),
        this, SLOT(onAliasChanged(Telepathy::Client::PendingOperation *)));
}

void TelepathyAccount::onAliasChanged(Telepathy::Client::PendingOperation* operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if(isOperationError(operation))
        return;
	
	kDebug(TELEPATHY_DEBUG_AREA) << "Alias has changed.";
}

void TelepathyAccount::slotChangeAvatar()
{
	// \todo: add here some error message?
	if(!m_account)
		return;

	QString avatarPath = Kopete::UI::AvatarDialog::getAvatar(Kopete::UI::Global::mainWidget());
	
	Telepathy::Avatar avatar;
	
	QFile avatarFile;
    if (!avatarFile.open(QIODevice::ReadOnly))
		return;
	
	avatar.avatarData = avatarFile.readAll();
	// \todo: add here mime type for avatar
	
	QObject::connect(m_account->setAvatar(avatar), SIGNAL(finished(Telepathy::Client::PendingOperation *)),
        this, SLOT(onAvatarChanged(Telepathy::Client::PendingOperation *)));
}

void TelepathyAccount::onAvatarChanged(Telepathy::Client::PendingOperation* operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if(isOperationError(operation))
        return;
	
	kDebug(TELEPATHY_DEBUG_AREA) << "Avatar has changed.";
}

