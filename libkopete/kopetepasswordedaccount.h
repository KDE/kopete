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

class Kopete::OnlineStatus;

namespace Kopete
{

class Password;

/**
 * An account requiring a password to connect. Instead of reimplementing connect()
 * in your subclass, reimplement connectWithPassword.
 *
 * @author Richard Smith <kde@metafoo.co.uk>
 */
class PasswordedAccount : public Account
{
	Q_OBJECT
	
public:

	/**
	 * KopetePasswordedAccount constructor
	 * @param parent The protocol this account connects via
	 * @param acctId The ID of this account - should be unique within this protocol
	 * @param maxPasswordLength The maximum length for passwords for this account, or 0 for no limit
	 * @param name The name for this QObject
	 */
	PasswordedAccount( Protocol *parent, const QString &acctId, uint maxPasswordLength = 0, const char *name = 0 );
	virtual ~PasswordedAccount();

	/**
	 * Returns a reference to the password object stored in this account.
	 */
	Password &password();

	void connect();
	void connect( const OnlineStatus& );
	
	/**
	 * \brief Get the initial status
	 */
	OnlineStatus initialStatus();

public slots:
	/**
	 * Called when your account should attempt to connect.
	 * @param password The password to connect with, or QString::null
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

private:
	class Private;
	Private *d;
};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:
