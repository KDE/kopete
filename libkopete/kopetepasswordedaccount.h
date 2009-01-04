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

#include "kopete_export.h"


namespace Kopete
{

class Password;

/**
 * An account requiring a password to connect. Instead of reimplementing connect()
 * in your subclass, reimplement connectWithPassword.
 *
 * @author Richard Smith <kde@metafoo.co.uk>
 */
class KOPETE_EXPORT PasswordedAccount : public Account
{
	Q_OBJECT

public:

    /**
	 * KopetePasswordedAccount constructor
	 * @param parent The protocol this account connects via
	 * @param acctId The ID of this account - should be unique within this protocol
	 * @param maxPasswordLength The maximum length for passwords for this account, or 0 for no limit
	 * @param allowBlankPassword If this protocol allows blank passwords. Note that this will mean that
	 *
	 * @param name The name for this QObject
	 */
	PasswordedAccount( Protocol *parent, const QString &acctId,  bool allowBlankPassword = false );

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
	void connect( const OnlineStatus& initialStatus );

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
	virtual bool removeAccount();


public slots:
	/**
	 * Called when your account should attempt to connect.
	 * @param password The password to connect with, or QString()
	 *        if the user wished to cancel the connection attempt.
	 */
	virtual void connectWithPassword( const QString &password ) = 0;

protected:
	/**
	 * Returns the prompt shown to the user when requesting their password.
	 * The default implementation should be adequate in most cases; override
	 * if you have a custom message to show the user.
	 */
	virtual QString passwordPrompt();

protected slots:
	/**
	 * @internal
	 * Reimplemented to set the password wrong if the reason is BadPassword
	 */
	virtual void disconnected( Kopete::Account::DisconnectReason reason );


private:
	struct Private;
	Private * const d;
};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:
