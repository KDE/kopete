/*
    kopetepasswordedaccount.h - Kopete Account with a password

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEPASSWORDEDACCOUNT_H
#define KOPETEPASSWORDEDACCOUNT_H

#include "kopeteaccount.h"

#include "libkopete_export.h"

namespace Kopete {
class Password;

/**
 * An account requiring a password to connect. Instead of reimplementing connect()
 * in your subclass, reimplement connectWithPassword.
 *
 * @author Richard Smith <kde@metafoo.co.uk>
 */
class LIBKOPETE_EXPORT PasswordedAccount : public Account
{
    Q_OBJECT

public:

    /**
     * KopetePasswordedAccount constructor
     * @param parent The protocol this account connects via
     * @param acctId The ID of this account - should be unique within this protocol
     * @param allowBlankPassword If this protocol allows blank passwords. Note that this will mean that
     */
    PasswordedAccount(Protocol *parent, const QString &acctId, bool allowBlankPassword = false);

    virtual ~PasswordedAccount();

    /**
     * Returns a reference to the password object stored in this account.
     */
    Password &password();

    void connect();

    /**
     * @brief Go online for this service.
     *
     * @param initialStatus is the status to connect with. If it is an invalid status for this
     * account, the default online for the account should be used.
     */
    void connect(const OnlineStatus &initialStatus) Q_DECL_OVERRIDE;

    /**
     * \brief Get the initial status
     */
    OnlineStatus initialStatus();

    /**
     * @brief Remove the account from the server.
     *
     * Reimplementation of Account::removeAccount() to remove the password from the wallet.
     * if your protocol reimplements this function, this function should still be called.
     *
     * @return Always true
     */
    bool removeAccount() Q_DECL_OVERRIDE;

public Q_SLOTS:
    /**
     * Called when your account should attempt to connect.
     * @param password The password to connect with, or QString()
     *        if the user wished to cancel the connection attempt.
     */
    virtual void connectWithPassword(const QString &password) = 0;

protected:
    /**
     * Returns the prompt shown to the user when requesting their password.
     * The default implementation should be adequate in most cases; override
     * if you have a custom message to show the user.
     */
    virtual QString passwordPrompt();

protected Q_SLOTS:
    /**
     * @internal
     * Reimplemented to set the password wrong if the reason is BadPassword
     */
    void disconnected(Kopete::Account::DisconnectReason reason) Q_DECL_OVERRIDE;

private:
    struct Private;
    Private *const d;
};
}

#endif

// vim: set noet ts=4 sts=4 sw=4:
