/*
 * telepathyaccount.h
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

#ifndef TELEPATHYACCOUNT_H
#define TELEPATHYACCOUNT_H

#include <kopeteaccount.h>

#include <TelepathyQt4/Client/ConnectionManager>
#include <TelepathyQt4/Client/PendingAccount>
#include <TelepathyQt4/Constants>

#include <QSharedPointer>

namespace Kopete
{
        class MetaContact;
        class StatusMessage;
}

namespace QtTapioca
{
        class Channel;
        class TextChannel;
        class Contact;
}

namespace Telepathy
{
    namespace Client
    {
        class AccountManager;
        class Account;
        class PendingOperation;
    }
}

class TelepathyProtocol;

class TelepathyAccount : public Kopete::Account
{
    Q_OBJECT
public:
    TelepathyAccount(TelepathyProtocol *protocol, const QString &accountId);
    ~TelepathyAccount();

    void createTextChatSession(QtTapioca::TextChannel *newChannel);
    QtTapioca::TextChannel *createTextChannel(QtTapioca::Contact *internalContact);

    QString connectionManager();
    QString connectionProtocol();

    bool readConfig();

    /**
    * @brief Get all connection parameters merged with values from the config
    *
    * @return all connection parameters.
    */
    Telepathy::Client::ProtocolParameterList allConnectionParameters();

    Telepathy::Client::ConnectionManager *getConnectionManager();
    Telepathy::Client::AccountManager *getAccountManager();

public slots:
    virtual void connect (const Kopete::OnlineStatus &initialStatus = Kopete::OnlineStatus());
    virtual void disconnect ();
    virtual void setOnlineStatus (const Kopete::OnlineStatus &status, const Kopete::StatusMessage &reason = Kopete::StatusMessage(), const OnlineStatusOptions& options = None);
    virtual void setStatusMessage (const Kopete::StatusMessage &statusMessage);

protected:
    bool createContact (const QString &contactId, Kopete::MetaContact *parentContact);

private slots:
    void onConnectionManagerReady(Telepathy::Client::PendingOperation*);
    void onConnectionCreated(Telepathy::Client::PendingOperation*);
    void onAccountManagerReady(Telepathy::Client::PendingOperation*);
    void newTelepathyAccountCreated(Telepathy::Client::PendingOperation *);
    void onAccountReady(Telepathy::Client::PendingOperation *);
    void onExistingAccountReady(Telepathy::Client::PendingOperation *);
    void onConnectionReady(Telepathy::Client::PendingOperation*);
    void onRequestedPresence(Telepathy::Client::PendingOperation*);
    void onConnectionConnected(Telepathy::Client::PendingOperation*);
    void onRequestConnectReady(Telepathy::Client::PendingOperation*);

private:
    void initTelepathyAccount();
    void createNewAccount();
    bool isOperationError(Telepathy::Client::PendingOperation*);

    Telepathy::Client::ConnectionManager *currentConnectionManager;
    Telepathy::Client::AccountManager *currentAccountManager;
    QSharedPointer<Telepathy::Client::Account> account;
    QSharedPointer<Telepathy::Client::Connection> m_connection;
    Telepathy::Client::ProtocolParameterList m_allConnectionParameters;
    Telepathy::Client::ProtocolParameterList connectionParameters;
    Telepathy::Client::PendingAccount *paccount;

    QString connectionManagerName;
    QString connectionProtocolName;

    bool connectAfterInit;
    bool setStatusAfterInit;
    Kopete::OnlineStatus statusInit;
    Kopete::StatusMessage reasonInit;
    int existingAccountsCount;
    int existingAccountCounter;
};

#endif //TELEPATHACCOUNT_H


