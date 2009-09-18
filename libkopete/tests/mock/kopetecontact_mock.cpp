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

#include "kopetecontact_mock.h"

namespace Kopete
{
namespace Test
{
namespace Mock
{

Contact::Contact( Kopete::Account *account, const QString &id, Kopete::MetaContact *parent, const QString &icon) : Kopete::Contact( account, id, parent, icon)
{

}

Contact::~Contact()
{

}

Kopete::ChatSession* Contact::manager( CanCreateFlags canCreate)
{
	Q_UNUSED( canCreate )
	return 0L;
}

} // end ns Kopete::Test::Mock
} // end ns Kopete::Test
} // end ns Kopete
