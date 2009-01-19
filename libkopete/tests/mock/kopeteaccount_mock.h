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

#ifndef _KOPETEACCOUNT_MOCK_H_
#define _KOPETEACCOUNT_MOCK_H_

#include "kopeteaccount.h"

namespace Kopete
{
class Protocol;
class OnlineStatus;
class StatusMessage;
}

class QString;

namespace Kopete
{
namespace Test
{
namespace Mock
{

class Account : public Kopete::Account
{
public:
	Account(Kopete::Protocol *parent, const QString &accountID);
	~Account();
	// pure virtual functions implementation
	virtual bool createContact( const QString &contactId, MetaContact *parentContact );
	virtual void connect( const Kopete::OnlineStatus& initialStatus = OnlineStatus() );
	virtual void disconnect();
	virtual void setOnlineStatus( const Kopete::OnlineStatus& status , const Kopete::StatusMessage &statusMessage = Kopete::StatusMessage(),
	                              const OnlineStatusOptions& options = None );
	virtual void setStatusMessage( const Kopete::StatusMessage &statusMessage );
};

} // end ns Kopete::Test::Mock
} // end ns Kopete::Test
} // end ns Kopete


#endif

