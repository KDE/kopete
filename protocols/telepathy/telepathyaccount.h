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

#include <TelepathyQt4/Client/ConnectionManager>
#include <TelepathyQt4/Client/AccountManager>
#include <TelepathyQt4/Client/PendingOperation>
#include <TelepathyQt4/Client/PendingAccount>

namespace Kopete
{
    class MetaContact;
}

class TelepathyProtocol;

class TelepathyAccount : public Kopete::Account
{
    Q_OBJECT

public:
    TelepathyAccount(TelepathyProtocol *protocol, const QString &accountId);
    ~TelepathyAccount();

	bool readConfig();
	QString connectionProtocol() const;
	Telepathy::Client::ProtocolParameterList allConnectionParameters() const;

public slots:
    virtual void connect (const Kopete::OnlineStatus &initialStatus = Kopete::OnlineStatus());
    virtual void disconnect ();
    virtual void setOnlineStatus (const Kopete::OnlineStatus &status, const Kopete::StatusMessage &reason = Kopete::StatusMessage(), const OnlineStatusOptions& options = None);
    virtual void setStatusMessage (const Kopete::StatusMessage &statusMessage);

private slots:
	void onAccountManagerReady(Telepathy::Client::PendingOperation *);
	void newTelepathyAccountCreated(Telepathy::Client::PendingOperation *);
	void onAccountReady(Telepathy::Client::PendingOperation *);

    void displayNameChanged (const QString &);
    void iconChanged (const QString &);
    void nicknameChanged (const QString &);
    void normalizedNameChanged (const QString &);
    void validityChanged (bool);
    void stateChanged (bool);
    void connectsAutomaticallyPropertyChanged (bool);
    void parametersChanged (const QVariantMap &);
    void automaticPresenceChanged (const Telepathy::SimplePresence &) const;
    void currentPresenceChanged (const Telepathy::SimplePresence &) const;
    void requestedPresenceChanged (const Telepathy::SimplePresence &) const;
    void avatarChanged (const Telepathy::Avatar &);
    void connectionStatusChanged (Telepathy::ConnectionStatus, Telepathy::ConnectionStatusReason);
    void haveConnectionChanged (bool haveConnection);

protected:
    virtual bool createContact( const QString &contactId, Kopete::MetaContact *parentContact );

private:
	Telepathy::Client::ProtocolInfo *getProtocolInfo(QString protocol);
	Telepathy::Client::ConnectionManager *getConnectionManager();
	Telepathy::Client::AccountManager *getAccountManager();
	void initTelepathyAccount();
	void createNewAccount();
	bool isOperationError(Telepathy::Client::PendingOperation*);

	QString m_connectionManagerName;
	QString m_connectionProtocolName;
	Telepathy::Client::ProtocolParameterList m_connectionParameters;
	Telepathy::Client::ConnectionManager *m_connectionManager;
	Telepathy::Client::AccountManager *m_accountManager;
	QSharedPointer<Telepathy::Client::Account> m_account;
	Telepathy::Client::PendingAccount *m_pendingAccount;

	Kopete::OnlineStatus m_initialStatus;
	uint m_existingAccountCounter;
	uint m_existingAccountsCount;
	bool m_setStatusAfterInit;
	Kopete::OnlineStatus m_status;
	Kopete::StatusMessage m_reason;
};

#endif // TELEPATHYACCOUNT_H_