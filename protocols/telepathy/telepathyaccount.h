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

#ifndef TELEPATHYACCOUNT_H_
#define TELEPATHYACCOUNT_H_

#include <QObject>
#include <QSharedPointer>

#include <kopeteaccount.h>

#include <TelepathyQt4/ConnectionManager>
#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingAccount>

namespace Kopete
{
class MetaContact;
}

class TelepathyProtocol;
class TelepathyContactManager;

class TelepathyAccount : public Kopete::Account
{
    Q_OBJECT

public:
    TelepathyAccount(TelepathyProtocol *protocol, const QString &accountId);
    virtual ~TelepathyAccount();

    bool readConfig();
    QString connectionProtocol() const;
    Tp::ProtocolParameterList allConnectionParameters() const;

public slots:
    virtual void connect(const Kopete::OnlineStatus &initialStatus = Kopete::OnlineStatus());
    virtual void disconnect();
    virtual void setOnlineStatus(const Kopete::OnlineStatus &status, const Kopete::StatusMessage &reason = Kopete::StatusMessage(), const OnlineStatusOptions& options = None);
    virtual void setStatusMessage(const Kopete::StatusMessage &statusMessage);
    virtual void fillActionMenu(KActionMenu *actionMenu);

private slots:
    void onAccountManagerReady(Tp::PendingOperation*);
    void newTelepathyAccountCreated(Tp::PendingOperation*);
    void onAccountReady(Tp::PendingOperation*);
    void onConnectionManagerReady(Tp::PendingOperation*);
    void onExistingAccountReady(Tp::PendingOperation*);
    void onRequestedPresence(Tp::PendingOperation*);
    void onAccountConnecting(Tp::PendingOperation*);
    void onRequestDisconnect(Tp::PendingOperation*);
    void onAliasChanged(Tp::PendingOperation*);
    void onAvatarChanged(Tp::PendingOperation*);

    void displayNameChanged(const QString &);
    void iconChanged(const QString &);
    void nicknameChanged(const QString &);
    void normalizedNameChanged(const QString &);
    void validityChanged(bool);
    void stateChanged(bool);
    void connectsAutomaticallyPropertyChanged(bool);
    void parametersChanged(const QVariantMap &);
    void automaticPresenceChanged(const Tp::SimplePresence &) const;
    void currentPresenceChanged(const Tp::SimplePresence &) const;
    void requestedPresenceChanged(const Tp::SimplePresence &) const;
    void avatarChanged(const Tp::Avatar &);
    void connectionStatusChanged(Tp::ConnectionStatus, Tp::ConnectionStatusReason);
    void haveConnectionChanged(bool haveConnection);

    void slotSetAlias();
    void slotChangeAvatar();

protected:
    virtual bool createContact(const QString &contactId, Kopete::MetaContact *parentContact);

private:
    Tp::ProtocolInfo *getProtocolInfo(QString protocol);
    Tp::ConnectionManagerPtr getConnectionManager();
    Tp::AccountManagerPtr getAccountManager();
    TelepathyContactManager *getContactManager();
    void initTelepathyAccount();
    void createNewAccount();
    void fetchContactList();

    QString m_connectionManagerName;
    QString m_connectionProtocolName;
    Tp::ProtocolParameterList m_connectionParameters;
    Tp::ConnectionManagerPtr m_connectionManager;
    Tp::AccountManagerPtr m_accountManager;
    TelepathyContactManager *m_contactManager;
    Tp::AccountPtr m_account;
    Tp::PendingAccount *m_pendingAccount;

    uint m_existingAccountCounter;
    uint m_existingAccountsCount;
    bool m_setStatusAfterInit;
    Kopete::OnlineStatus m_status;
    Kopete::StatusMessage m_reason;

    friend class TelepathyContactManager;
};


#endif // TELEPATHYACCOUNT_H_

