/*
    Contact mock object class

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

#ifndef _KOPETECONTACT_MOCK_H_
#define _KOPETECONTACT_MOCK_H_

#include "kopetecontact.h"

namespace Kopete
{
class MetaContact;
class Account;
class ChatSession;
}
class QString;

namespace Kopete
{
namespace Test
{
namespace Mock
{

class Contact : public Kopete::Contact
{
public:
	Contact( Kopete::Account *account, const QString &id, Kopete::MetaContact *parent, const QString &icon = QString() );
	~Contact();
	virtual Kopete::ChatSession* manager( CanCreateFlags canCreate = CannotCreate );
};

} // end ns Kopete::Test::Mock
} // end ns Kopete::Test
} // end ns Kopete


#endif

