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

#include <klocale.h>

#include <qpixmap.h>

struct KopetePasswordedAccount::Private
{
	Private( const QString &group, uint maxLen ) : password( group, maxLen, "mPassword" ) {}
	KopetePassword password;
};

KopetePasswordedAccount::KopetePasswordedAccount( KopeteProtocol *parent, const QString &acctId, uint maxLen, const char *name )
 : KopeteAccount( parent, acctId, name ), d( new Private( configGroup(), maxLen ) )
{
}

KopetePasswordedAccount::~KopetePasswordedAccount()
{
	delete d;
}

KopetePassword &KopetePasswordedAccount::password()
{
	return d->password;
}

void KopetePasswordedAccount::connect()
{
	QString prompt = passwordPrompt();
	KopetePassword::PasswordSource src = password().isWrong() ? KopetePassword::FromUser : KopetePassword::FromConfigOrUser;

	password().request( this, SLOT( connectWithPassword( const QString & ) ), accountIcon( KopetePassword::preferredImageSize() ), prompt, src );
}

QString KopetePasswordedAccount::passwordPrompt()
{
	if ( password().isWrong() )
		return i18n( "<b>The password was wrong!</b> Please re-enter your password for %1 account <b>%2</b>" ).arg( protocol()->displayName(), accountId() );
	else
		return i18n( "Please enter your password for %1 account <b>%2</b>" ).arg( protocol()->displayName(), accountId() );
}

#include "kopetepasswordedaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:
