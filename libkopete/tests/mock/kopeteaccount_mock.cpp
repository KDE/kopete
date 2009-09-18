/*
    Account mock object class

    Copyright (c) 2005 by Duncan Mac-Vicar Prett  <duncan@kde.org>

    Kopete (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteaccount_mock.h"
#include "kopetemetacontact.h"
#include "kopetestatusmessage.h"

namespace Kopete
{
namespace Test
{
namespace Mock
{

Account::Account(Kopete::Protocol *parent, const QString &accountID) 
  : Kopete::Account(parent, accountID)
{

}

Account::~Account()
{

}

bool Account::createContact( const QString &contactId, Kopete::MetaContact *parentContact )
{
	Q_UNUSED( contactId )
	Q_UNUSED( parentContact )
	return true;
}

void Account::connect( const Kopete::OnlineStatus& initialStatus)
{
	Q_UNUSED( initialStatus )
	// do nothing
}

void Account::disconnect()
{
	// do nothing
}

void Account::setOnlineStatus( const Kopete::OnlineStatus& status , const Kopete::StatusMessage &statusMessage, const OnlineStatusOptions& options )
{
	Q_UNUSED( status )
	Q_UNUSED( statusMessage )
	Q_UNUSED( options )
	// do nothing
}

void Account::setStatusMessage( const Kopete::StatusMessage& statusMessage )
{
	Q_UNUSED( statusMessage )
	// do nothing
}

} // end ns Kopete::Test::Mock
} // end ns Kopete::Test
} // end ns Kopete
