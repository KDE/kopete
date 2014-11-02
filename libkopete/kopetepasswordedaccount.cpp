/*
    kopetepasswordedaccount.cpp - Kopete Account with a password

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

#include "kopetepasswordedaccount.h"
#include "kopetepassword.h"
#include "kopeteprotocol.h"
#include "kopeteonlinestatus.h"

#include <klocale.h>

#include <qpixmap.h>

struct Kopete::PasswordedAccount::Private
{
	Private( const QString &group, bool allowBlankPassword ) :
		password( group, allowBlankPassword ) {}
	Kopete::Password password;
	Kopete::OnlineStatus initialStatus;
};

Kopete::PasswordedAccount::PasswordedAccount( Kopete::Protocol *parent, const QString &acctId, bool allowBlankPassword )
 : Kopete::Account( parent, acctId ), d( new Private( QString::fromLatin1("Account_")+ parent->pluginId() + QString::fromLatin1("_") + acctId , allowBlankPassword  ) )
{
}

Kopete::PasswordedAccount::~PasswordedAccount()
{
	delete d;
}

Kopete::Password &Kopete::PasswordedAccount::password()
{
	return d->password;
}

void Kopete::PasswordedAccount::connect( )
{
	Kopete::OnlineStatus s(Kopete::OnlineStatus::Online);
	connect( s );
}

void Kopete::PasswordedAccount::connect( const Kopete::OnlineStatus& initialStatus )
{
	// warn user somewhere
	d->initialStatus = initialStatus;
	QString cached = password().cachedValue();
	if( !cached.isNull() || d->password.allowBlankPassword() )
	{
		connectWithPassword( cached );
		return;
	}

	QString prompt = passwordPrompt();
	Kopete::Password::PasswordSource src = password().isWrong() ? Kopete::Password::FromUser : Kopete::Password::FromConfigOrUser;

	password().request( this, SLOT(connectWithPassword(QString)), accountIcon( Kopete::Password::preferredImageSize() ), prompt, src );
}

QString Kopete::PasswordedAccount::passwordPrompt()
{
	if ( password().isWrong() )
		return i18n( "<qt><b>The password was wrong.</b> Please re-enter your password for %1 account <b>%2</b></qt>", protocol()->displayName(), accountId() );
	else
		return i18n( "<qt>Please enter your password for %1 account <b>%2</b></qt>", protocol()->displayName(), accountId() );
}

Kopete::OnlineStatus Kopete::PasswordedAccount::initialStatus()
{
	return d->initialStatus;
}

bool Kopete::PasswordedAccount::removeAccount()
{
	password().set(QString());
	return Kopete::Account::removeAccount();
}

void Kopete::PasswordedAccount::disconnected( Kopete::Account::DisconnectReason reason )
{
	if(reason==Kopete::Account::BadPassword || reason==Kopete::Account::BadUserName)
	{
		password().setWrong(true);
	}
	Kopete::Account::disconnected(reason);
}


#include "kopetepasswordedaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:
