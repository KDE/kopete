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

#include <KopeteTelepathy/telepathyaccount.h>

#include <KopeteTelepathy/telepathycontactmanager.h>

#include <KopeteTelepathy/telepathycontact.h>
#include <KopeteTelepathy/telepathyprotocolinternal.h>

#include <kopetemetacontact.h>
#include <kopetecontactlist.h>
#include <kopeteuiglobal.h>
#include <ui/avatardialog.h>

#include <KDebug>
#include <kglobal.h>
#include <kconfig.h>
#include <ksharedconfig.h>
#include <kicon.h>
#include <klocale.h>
#include <kaction.h>
#include <kactionmenu.h>
#include <kinputdialog.h>
#include <kmimetype.h>

#include <TelepathyQt4/Types>
#include <TelepathyQt4/Feature>
#include <TelepathyQt4/Account>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingContacts>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>

#include <QFile>

TelepathyAccount::TelepathyAccount(Kopete::Protocol *protocol, const QString &accountId)
        : Kopete::Account(protocol, accountId), m_connectionManager(0), m_accountManager(0),
        m_contactManager(0), m_existingAccountCounter(0), m_existingAccountsCount(0)
{
    kDebug();

    m_setStatusAfterInit = false;

    setMyself(new TelepathyContact(this, accountId, Kopete::ContactList::self()->myself()));

    initTelepathyAccount();
}

TelepathyAccount::~TelepathyAccount()
{
    kDebug();
    if (m_contactManager)
        delete m_contactManager;
}

void TelepathyAccount::connect(const Kopete::OnlineStatus &initialStatus)
{
    kDebug();

    if (!m_account || !m_account->becomeReady()) {
        m_status = initialStatus;
        m_setStatusAfterInit = true;
        initTelepathyAccount();
        return;
    }

    if (m_account->haveConnection()) {
        kDebug() << "Account have connection";
    }

    kDebug() << m_account->parameters();

    Tp::SimplePresence simplePresence;
    simplePresence.type = TelepathyProtocolInternal::protocolInternal()->kopeteStatusToTelepathy(m_status);

    simplePresence.statusMessage = m_reason.message();

    kDebug() << "Requested Presence status: " << simplePresence.type << "message:" << simplePresence.statusMessage;

    Tp::PendingOperation *op = m_account->setRequestedPresence(simplePresence);
    QObject::connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                     this,
                     SLOT(onRequestedPresence(Tp::PendingOperation*))
                    );
}

void TelepathyAccount::onRequestedPresence(Tp::PendingOperation* operation)
{
    kDebug();

    if (operation->isError()) {
        kWarning() << "Setting requested presence failed:" << operation->errorName()
                   << operation->errorMessage();
        return;
    }
}

void TelepathyAccount::fillActionMenu(KActionMenu *actionMenu)
{
    Kopete::Account::fillActionMenu(actionMenu);

    KAction *changeAliasAction = new KAction(KIcon("edit-rename"), i18n("&Change Alias..."), 0);
    changeAliasAction->setEnabled(isConnected());
    QObject::connect(changeAliasAction, SIGNAL(triggered(bool)), this, SLOT(slotSetAlias()));

    KAction *changeAvatarAction = new KAction(KIcon("user-properties"), i18n("Change &Avatar..."), 0);
    changeAvatarAction->setEnabled(isConnected());
    QObject::connect(changeAvatarAction, SIGNAL(triggered(bool)), this, SLOT(slotChangeAvatar()));

    actionMenu->addSeparator();
    actionMenu->addAction(changeAliasAction);
    actionMenu->addAction(changeAvatarAction);
}

void TelepathyAccount::disconnect()
{
    kDebug();

    if (!m_account || !m_account->haveConnection())
        return;

    Tp::ConnectionPtr connection = m_account->connection();

    QObject::connect(connection->requestDisconnect(),
                     SIGNAL(finished(Tp::PendingOperation*)),
                     this,
                     SLOT(onRequestDisconnect(Tp::PendingOperation*))
                    );
}

void TelepathyAccount::onRequestDisconnect(Tp::PendingOperation* operation)
{
    kDebug();

    if (operation->isError()) {
        kWarning() << "Request Disconnect failed:" << operation->errorName()
                   << operation->errorMessage();
        return;
    }
}

void TelepathyAccount::setOnlineStatus(const Kopete::OnlineStatus &status, const Kopete::StatusMessage &reason, const OnlineStatusOptions& options)
{
    kDebug();

    Q_UNUSED(options);

    m_status = status;
    m_reason = reason;

    if (!m_account || !m_account->isReady()) {
        m_setStatusAfterInit = true;
        initTelepathyAccount();
        return;
    }

    if (!isConnected()) {
        connect(status);
    } else if (status.status() == Kopete::OnlineStatus::Offline) {
        disconnect();
    } else {
        Tp::SimplePresence simplePresence;
        simplePresence.type = TelepathyProtocolInternal::protocolInternal()->kopeteStatusToTelepathy(status);

        kDebug() << "Requested Presence status: " << simplePresence.type << reason.message();

        simplePresence.statusMessage = reason.message();

        Tp::PendingOperation *op = m_account->setRequestedPresence(simplePresence);
        QObject::connect(op,
                         SIGNAL(finished(Tp::PendingOperation*)),
                         this,
                         SLOT(onRequestedPresence(Tp::PendingOperation*))
                        );
    }
}

void TelepathyAccount::setStatusMessage(const Kopete::StatusMessage &reason)
{
    kDebug();

    setOnlineStatus(m_status, reason);
}

bool TelepathyAccount::createContact(const QString &contactId, Kopete::MetaContact *parentContact)
{
    kDebug();

    if (!contacts()[contactId]) {
        TelepathyContact *contact = new TelepathyContact(this, contactId, parentContact);

        return contact != 0;
    } else {
        kDebug() << "Contact " << contactId << " already exists.";
    }

    return false;
}

bool TelepathyAccount::readConfig()
{
    kDebug();

    // Restore config not related to ConnectionManager parameters first
    // so that the UI for the protocol parameters will be generated
    KConfigGroup *accountConfig = configGroup();
    m_connectionManagerName = accountConfig->readEntry(QLatin1String("ConnectionManager"), QString());
    // setup CM early
    getConnectionManager();

    m_connectionProtocolName = accountConfig->readEntry(QLatin1String("TelepathyProtocol"), QString());

    // Clear current connection parameters
    m_connectionParameters.clear();

    // Get the preferences from the connection manager to get the right types
    Tp::ProtocolInfo* protocolInfo = getProtocolInfo(m_connectionProtocolName);
    if (!protocolInfo) {
        kDebug() << "Error: could not get protocol info" << m_connectionProtocolName;
        return false;
    }

    Tp::ProtocolParameterList tempParameters = protocolInfo->parameters();

    // Now update the preferences
    KSharedConfig::Ptr telepathyConfig = KGlobal::config();
    QMap<QString, QString> allEntries = telepathyConfig->entryMap(TelepathyProtocolInternal::protocolInternal()->formatTelepathyConfigGroup(m_connectionManagerName, m_connectionProtocolName, accountId()));
    QMap<QString, QString>::ConstIterator it, itEnd = allEntries.constEnd();
    for (it = allEntries.constBegin(); it != itEnd; ++it) {
        foreach(Tp::ProtocolParameter *parameter, tempParameters) {
            if (parameter->name() == it.key()) {
                kDebug() << parameter->defaultValue().toString() << it.value();
                if (parameter->defaultValue().toString() != it.value()) {
                    QVariant oldValue = parameter->defaultValue();
                    QVariant newValue(oldValue.type());
                    if (oldValue.type() == QVariant::String)
                        newValue = QVariant(it.value());
                    else if (oldValue.type() == QVariant::Int)
                        newValue = QVariant(it.value()).toInt();
                    else if (oldValue.type() == QVariant::UInt)
                        newValue = QVariant(it.value()).toUInt();
                    else if (oldValue.type() == QVariant::Double)
                        newValue = QVariant(it.value()).toDouble();
                    else if (oldValue.type() == QVariant::StringList)
                        newValue = QVariant(it.value()).toStringList();
                    else if (oldValue.type() == QVariant::Bool) {
                        if (it.value().toLower() == "true")
                            newValue = true;
                        else
                            newValue = false;
                    } else
                        newValue = QVariant(it.value());

                    kDebug() << "Name: " << parameter->name() << " Value: " << newValue << "Type: " << parameter->defaultValue().typeName();
                    m_connectionParameters.append(
                        new Tp::ProtocolParameter(
                            parameter->name(),
                            parameter->dbusSignature(),
                            newValue,
                            Tp::ConnMgrParamFlagHasDefault
                        ));
                } else {
                    kDebug() << parameter->name() << parameter->defaultValue();

                    m_connectionParameters.append(
                        new Tp::ProtocolParameter(
                            parameter->name(),
                            parameter->dbusSignature(),
                            parameter->defaultValue(),
                            Tp::ConnMgrParamFlagHasDefault
                        ));
                }
            }
        }
    }

    kDebug() << m_connectionManagerName << m_connectionProtocolName << m_connectionParameters;
    if (!m_connectionManagerName.isEmpty() &&
            !m_connectionProtocolName.isEmpty() &&
            !m_connectionParameters.isEmpty())
        return true;
    else
        return false;
}

QString TelepathyAccount::connectionProtocol() const
{
    kDebug();
    return m_connectionProtocolName;
}

Tp::ProtocolParameterList TelepathyAccount::allConnectionParameters() const
{
    kDebug();
    return m_connectionParameters;
}

Tp::AccountManagerPtr TelepathyAccount::getAccountManager()
{
    kDebug();
    if (!m_accountManager)
        m_accountManager = Tp::AccountManager::create(QDBusConnection::sessionBus());

    return m_accountManager;
}

Tp::ConnectionManagerPtr TelepathyAccount::getConnectionManager()
{
    kDebug();
    if (!m_connectionManager) {
        m_connectionManager = Tp::ConnectionManager::create(m_connectionManagerName);
        // Do not need wait until operation finished
        m_connectionManager->becomeReady(Tp::ConnectionManager::FeatureCore);
    }

    return m_connectionManager;
}

TelepathyContactManager *TelepathyAccount::getContactManager()
{
    if (!m_contactManager)
        m_contactManager = new TelepathyContactManager(this);

    return m_contactManager;
}

Tp::ProtocolInfo *TelepathyAccount::getProtocolInfo(QString protocol)
{
    Tp::ProtocolInfoList protocolList = getConnectionManager()->protocols();
    kDebug() << protocolList.size();
    foreach(Tp::ProtocolInfo* protocolInfo, protocolList) {
        if (protocolInfo->name() == protocol) {
            kDebug() << protocolInfo->name();
            return protocolInfo;
        }
    }

    kWarning() << "Protocol:" << protocol << "was not found on this CM.";
    return 0;
}

void TelepathyAccount::initTelepathyAccount()
{
    kDebug();

    // Restore config not related to ConnectionManager parameters first
    // so that the UI for the protocol parameters will be generated
    KConfigGroup *accountConfig = configGroup();
    m_connectionManagerName = accountConfig->readEntry(QLatin1String("ConnectionManager"), QString());
    m_connectionProtocolName = accountConfig->readEntry(QLatin1String("TelepathyProtocol"), QString());

    // Abort if config has not yet been written because it's a new account.
    if (m_connectionManagerName.isEmpty()) {
        kDebug() << "Config not yet written of new acocunt. Cancel initTelepathyAccount().";
        return;
    }

    // \brief: init managers early, it needed here becouse later i want
    //   to get available protocols from CM
    Tp::ConnectionManagerPtr cm = getConnectionManager();
    QObject::connect(cm->becomeReady(),
                     SIGNAL(finished(Tp::PendingOperation*)),
                     this,
                     SLOT(onConnectionManagerReady(Tp::PendingOperation*))
                    );
}

void TelepathyAccount::onConnectionManagerReady(Tp::PendingOperation* operation)
{
    kDebug();

    if (operation->isError()) {
        kWarning() << "Readying Connection Manager failed:" << operation->errorName()
                   << operation->errorMessage();
        return;
    }

    if (readConfig()) {
        Tp::AccountManagerPtr accountManager = getAccountManager();
        QObject::connect(accountManager->becomeReady(),
                         SIGNAL(finished(Tp::PendingOperation*)),
                         this,
                         SLOT(onAccountManagerReady(Tp::PendingOperation*))
                        );
        return;
    }

    // \brief: problem with config? So here we can die.
    kDebug() << "Init connection manager failed!";
}

void TelepathyAccount::onAccountManagerReady(Tp::PendingOperation* operation)
{
    kDebug();

    if (operation->isError()) {
        kWarning() << "Readying Account Manager failed:"
                   << operation->errorName()
                   << operation->errorMessage();
        return;
    }

    /*
     * get a list of all the accounts that
     * are all ready there
     */
    QStringList pathList = m_accountManager->allAccountPaths();
    kDebug() << "accounts: " << pathList.size() << pathList;
    m_existingAccountsCount = pathList.size();
    if (m_existingAccountsCount != 0)
        m_existingAccountCounter++;
    else {
        createNewAccount();
    }

    /*
     * check if account already exist
     */
    foreach(const QString &path, pathList) {
        Tp::AccountPtr a = m_accountManager->accountForPath(path);
        Tp::Features features;
        features << Tp::Account::FeatureCore
                 << Tp::Account::FeatureAvatar
                 << Tp::Account::FeatureProtocolInfo;
        QObject::connect(a->becomeReady(features), SIGNAL(finished(Tp::PendingOperation *)),
                         this, SLOT(onExistingAccountReady(Tp::PendingOperation *)));
    }
}

void TelepathyAccount::onExistingAccountReady(Tp::PendingOperation *operation)
{
    kDebug();

    if (operation->isError()) {
        kWarning() << "Readying existing account failed:"
                   << operation->errorName()
                   << operation->errorMessage();
        return;
    }

    Tp::PendingReady *p = qobject_cast<Tp::PendingReady *>(operation);
    if (!p) {
        kDebug() << "Error: problem with casting";
        return;
    }

    Tp::Account *a = qobject_cast<Tp::Account*>(p->object());
    if (!a) {
        kDebug() << "Error: problem with casting";
        return;
    }

    if (!m_account && ((a->displayName() == accountId()) && (a->protocol() == m_connectionProtocolName))) {
        kDebug() << "Account already exist " << a->cmName() << m_connectionManagerName << a->displayName() << accountId() << a->protocol() << m_connectionProtocolName << m_existingAccountCounter;
        m_account = Tp::AccountPtr(a);

        QObject::connect(m_account->becomeReady(), SIGNAL(finished(Tp::PendingOperation *)),
                         this, SLOT(onAccountReady(Tp::PendingOperation *)));

        return;
    }

    if (!m_account && (m_existingAccountCounter == m_existingAccountsCount)) {
        createNewAccount();
    }
    m_existingAccountCounter++;
}

void TelepathyAccount::createNewAccount()
{
    kDebug();

    QVariantMap parameters;
    foreach (Tp::ProtocolParameter *parameter, m_connectionParameters) {
        kDebug() << parameter->name() << parameter->defaultValue().toString();

        // Don't add empty parameters to avoid hitting an assert in qdbus.
        if (parameter->defaultValue() != QVariant()) {
            parameters[parameter->name()] = parameter->defaultValue();
        }
    }
    
    kDebug() << "Creating account: "
                                 << m_connectionManagerName
                                 << m_connectionProtocolName
                                 << accountId()
                                 << parameters;

    m_pendingAccount = m_accountManager->createAccount(m_connectionManagerName,
                                                       m_connectionProtocolName,
                                                       accountId(),
                                                       parameters);

    QObject::connect(m_pendingAccount, SIGNAL(finished(Tp::PendingOperation *)),
                     this, SLOT(newTelepathyAccountCreated(Tp::PendingOperation *)));
}

void TelepathyAccount::newTelepathyAccountCreated(Tp::PendingOperation *operation)
{
    kDebug();

    // \brief: zeroing counter
    m_existingAccountCounter = 0;

    if (operation->isError()) {
        kWarning() << "Creating new account failed:"
                                       << operation->errorName()
                                       << operation->errorMessage();
        return;
    }

    m_account = m_pendingAccount->account();

    QObject::connect(m_account->becomeReady(), SIGNAL(finished(Tp::PendingOperation *)),
                     this, SLOT(onAccountReady(Tp::PendingOperation *)));
}

void TelepathyAccount::onAccountReady(Tp::PendingOperation *operation)
{
    kDebug();

    if (operation->isError()) {
        kWarning() << "Readying account failed:"
                                       << operation->errorName()
                                       << operation->errorMessage();
        return;
    }

    kDebug() << "New account: " << m_account->cmName() << m_account->protocol() << m_account->displayName();

    // We shoudl enable the account
    QObject::connect(m_account->setEnabled(true),
                     SIGNAL(finished(Tp::PendingOperation*)),
                     SLOT(onSetEnabledFinished(Tp::PendingOperation*)));
}

void TelepathyAccount::onSetEnabledFinished(Tp::PendingOperation *op)
{
    if (op->isError()) {
        // TODO: User feedback here.
        kWarning() << "Enabling account failed:" << op->errorName() << op->errorMessage();
        return;
    }

    Tp::Account *a = m_account.data();

    QObject::connect(a, SIGNAL(displayNameChanged(const QString &)),
                     this, SLOT(displayNameChanged(const QString &)));
    QObject::connect(a, SIGNAL(iconChanged(const QString &)),
                     this, SLOT(iconChanged(const QString &)));
    QObject::connect(a, SIGNAL(nicknameChanged(const QString &)),
                     this, SLOT(nicknameChanged(const QString &)));
    QObject::connect(a, SIGNAL(normalizedNameChanged(const QString &)),
                     this, SLOT(normalizedNameChanged(const QString &)));
    QObject::connect(a, SIGNAL(validityChanged(bool)),
                     this, SLOT(validityChanged(bool)));
    QObject::connect(a, SIGNAL(stateChanged(bool)),
                     this, SLOT(stateChanged(bool)));
    QObject::connect(a, SIGNAL(connectsAutomaticallyPropertyChanged(bool)),
                     this, SLOT(connectsAutomaticallyPropertyChanged(bool)));
    QObject::connect(a, SIGNAL(parametersChanged(const QVariantMap &)),
                     this, SLOT(parametersChanged(const QVariantMap &)));
    QObject::connect(a, SIGNAL(automaticPresenceChanged(const Tp::SimplePresence &)),
                     this, SLOT(automaticPresenceChanged(const Tp::SimplePresence &)));
    QObject::connect(a, SIGNAL(currentPresenceChanged(const Tp::SimplePresence &)),
                     this, SLOT(currentPresenceChanged(const Tp::SimplePresence &)));
    QObject::connect(a, SIGNAL(requestedPresenceChanged(const Tp::SimplePresence &)),
                     this, SLOT(requestedPresenceChanged(const Tp::SimplePresence &)));
    QObject::connect(a, SIGNAL(avatarChanged(const Tp::Avatar &)),
                     this, SLOT(avatarChanged(const Tp::Avatar &)));
    QObject::connect(a, SIGNAL(connectionStatusChanged(Tp::ConnectionStatus, Tp::ConnectionStatusReason)),
                     this, SLOT(connectionStatusChanged(Tp::ConnectionStatus, Tp::ConnectionStatusReason)));
    QObject::connect(a, SIGNAL(haveConnectionChanged(bool)),
                     this, SLOT(haveConnectionChanged(bool)));

    if (m_setStatusAfterInit) {
        m_setStatusAfterInit = false;
        setOnlineStatus(m_status, m_reason);
    } else {
        myself()->setOnlineStatus(
                TelepathyProtocolInternal::protocolInternal()->telepathyStatusToKopete(
                        static_cast<Tp::ConnectionPresenceType>(m_account->currentPresence().type)));
        myself()->setNickName(m_account->nickname());
        fetchContactList();
    }
}

void TelepathyAccount::displayNameChanged(const QString &var)
{
    kDebug() << var;
}

void TelepathyAccount::iconChanged(const QString &var)
{
    kDebug() << var;
}

void TelepathyAccount::nicknameChanged(const QString &var)
{
    kDebug() << var;

    myself()->setNickName(var);
}

void TelepathyAccount::normalizedNameChanged(const QString &var)
{
    kDebug() << var;
}

void TelepathyAccount::validityChanged(bool var)
{
    kDebug() << var;
}

void TelepathyAccount::stateChanged(bool var)
{
    kDebug() << var;
}

void TelepathyAccount::connectsAutomaticallyPropertyChanged(bool var)
{
    kDebug() << var;
}

void TelepathyAccount::parametersChanged(const QVariantMap &var)
{
    kDebug() << var;
}

void TelepathyAccount::automaticPresenceChanged(const Tp::SimplePresence &) const
{
    kDebug() ;
}

void TelepathyAccount::currentPresenceChanged(const Tp::SimplePresence &) const
{
    kDebug() ;

    myself()->setOnlineStatus(
            TelepathyProtocolInternal::protocolInternal()->telepathyStatusToKopete(
                    static_cast<Tp::ConnectionPresenceType>(m_account->currentPresence().type)));
}

void TelepathyAccount::requestedPresenceChanged(const Tp::SimplePresence &) const
{
    kDebug() ;
}

void TelepathyAccount::avatarChanged(const Tp::Avatar &)
{
    kDebug() ;
}

void TelepathyAccount::connectionStatusChanged(Tp::ConnectionStatus status, Tp::ConnectionStatusReason reason)
{
    kDebug() ;
    Q_UNUSED(reason);

    switch (status) {
    case Tp::ConnectionStatusConnecting:
        kDebug() << "Connecting....";
        myself()->setOnlineStatus(TelepathyProtocolInternal::protocolInternal()->Connecting);
        break;
    case Tp::ConnectionStatusConnected:
        kDebug() << "Connected using Telepathy :)";
        // Set initial status to myself contact
        myself()->setOnlineStatus(m_status);
        // Set nickname to myself contact
        myself()->setNickName(m_account->nickname());
        // Load contact list
        fetchContactList();
        break;
    case Tp::ConnectionStatusDisconnected:
        kDebug() << "Disconnected :(";
        myself()->setOnlineStatus(TelepathyProtocolInternal::protocolInternal()->Offline);
        break;
    }

}

void TelepathyAccount::haveConnectionChanged(bool haveConnection)
{
    kDebug() << haveConnection;

    if (haveConnection) {
        kDebug() << "Have connection.";
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
                           &ok);

    if (!ok || !m_account)
        return;

    QObject::connect(m_account->setNickname(newAlias), SIGNAL(finished(Tp::PendingOperation *)),
                     this, SLOT(onAliasChanged(Tp::PendingOperation *)));
}

void TelepathyAccount::onAliasChanged(Tp::PendingOperation* operation)
{
    kDebug();

    if (operation->isError()) {
        kWarning() << "Alias Changing Failed:"
                                       << operation->errorName()
                                       << operation->errorMessage();
        return;
    }

    kDebug() << "Alias has changed.";
}

void TelepathyAccount::slotChangeAvatar()
{
    kDebug();

    if (!m_account) {
        kWarning() << "Invalid telepathy account";
        return;
    }

    QString avatarPath = Kopete::UI::AvatarDialog::getAvatar(Kopete::UI::Global::mainWidget());

    kDebug() << avatarPath;

    Tp::Avatar avatar;

    if (!avatarPath.isNull() &&
        !avatarPath.isEmpty()) {

        if (!QFile::exists(avatarPath)) {
            kWarning() << "Avatar file doesn't exist";
            return;
        }

        QFile avatarFile(avatarPath);
        if (!avatarFile.open(QIODevice::ReadOnly)) {
            kWarning() << "Error reading user defined avatar";
            return;
        }

        // Load the image data
        avatar.avatarData = avatarFile.readAll();

        avatarFile.close();

        QString avatarMimeType = KMimeType::findByUrl(KUrl(avatarPath))->name();

        // Let's try to get the image mimetype
        if (!avatarMimeType.isEmpty()) {
            kDebug() << avatarMimeType;
            avatar.MIMEType = avatarMimeType;
        }
    }

    QObject::connect(m_account->setAvatar(avatar), SIGNAL(finished(Tp::PendingOperation *)),
                     this, SLOT(onAvatarChanged(Tp::PendingOperation *)));
}

void TelepathyAccount::onAvatarChanged(Tp::PendingOperation* operation)
{
    kDebug();

    if (operation->isError()) {
        kWarning() << "Changing Avatar failed:"
                                       << operation->errorName()
                                       << operation->errorMessage();
        return;
    }

    kDebug() << "Avatar was changed";
}

void TelepathyAccount::fetchContactList()
{
    kDebug();

    if (!m_account->haveConnection()) {
        kDebug() << "Couldn't fetch contact list!";
        return;
    }

    getContactManager()->fetchContactList();
}

Tp::AccountPtr TelepathyAccount::account()
{
    return m_account;
}

void TelepathyAccount::addNewContact(const QString &id)
{
    kDebug();
    Tp::ConnectionPtr connection = account()->connection();

    if (!connection) {
        kWarning() << "TelepathyAccount->account()->connection() is null.";
        return;
    }

    if (connection->status() != Tp::ConnectionStatusConnected) {
        kWarning() << "connection.status() is not Connected.";
        return;
    }

    Tp::ContactManager *contactManager = connection->contactManager();

    Tp::PendingOperation *op = contactManager->contactsForIdentifiers(QStringList() << id);

    QObject::connect(op,
            SIGNAL(finished(Tp::PendingOperation*)), this,
            SLOT(onPendingContactsForAddingReady(Tp::PendingOperation*)));
}

void TelepathyAccount::onPendingContactsForAddingReady(Tp::PendingOperation *op)
{
    kDebug();
    Tp::PendingContacts *pendingContacts = qobject_cast<Tp::PendingContacts*>(op);

    QList<Tp::ContactPtr> contacts = pendingContacts->contacts();

    Tp::PendingOperation *opadd = contacts.at(0)->manager()->requestPresenceSubscription(contacts);
    QObject::connect(opadd, SIGNAL(finished(Tp::PendingOperation*)),
                     this, SLOT(onContactAdded(Tp::PendingOperation*)));
}

void TelepathyAccount::onContactAdded(Tp::PendingOperation *op)
{
    kDebug();

    // FIXME: Handle the case that the operation didn't finish successfully.
    Q_UNUSED(op);

    fetchContactList();
}

void TelepathyAccount::deleteContact(Tp::ContactPtr contact)
{
    kDebug();

    // FIXME: Why do we sometimes not delete from the right lists when these conditions exist?
    // When we delete a contact, remove from subscribe and publish lists.
   // if (contact->subscriptionState() == Tp::Contact::PresenceStateYes) {
        QObject::connect(contact->removePresenceSubscription(), SIGNAL(finished(Tp::PendingOperation*)),
                         this, SLOT(onContactDeleteRemoveSubscriptionFinished(Tp::PendingOperation*)));
   // }

   // if (contact->publishState() == Tp::Contact::PresenceStateYes) {
        QObject::connect(contact->removePresencePublication(), SIGNAL(finished(Tp::PendingOperation*)),
                         this, SLOT(onContactDeleteRemovePublicationFinished(Tp::PendingOperation*)));
   // }
}

void TelepathyAccount::onContactDeleteRemoveSubscriptionFinished(Tp::PendingOperation *op)
{
    kDebug();
    if (op->isError()) {
        kWarning() << "Deleting contact (removing subscription) failed:"
                                       << op->errorName()
                                       << op->errorMessage();
    }
}

void TelepathyAccount::onContactDeleteRemovePublicationFinished(Tp::PendingOperation *op)
{
    kDebug();
    if (op->isError()) {
        kWarning() << "Deleting contact (removing publication) failed:"
                                       << op->errorName()
                                       << op->errorMessage();
    }
}

void TelepathyAccount::accountEdited(const QVariantMap &setParameters,
                                     const QStringList &unsetParameters)
{
    kDebug();

    // FIXME: Handle operation results
    account()->updateParameters(setParameters, unsetParameters);
}


#include "telepathyaccount.moc"

