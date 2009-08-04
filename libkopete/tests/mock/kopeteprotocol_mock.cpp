/*
    Protocol mock object class

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

#include "kopeteprotocol_mock.h"

namespace Kopete
{
namespace Test
{
namespace Mock
{

Protocol::Protocol( const KComponentData &instance, QObject *parent )
  : Kopete::Protocol(instance, parent)
{
	
}

Account* Protocol::createNewAccount( const QString &accountId )
{
	Q_UNUSED( accountId )
	return 0L;
}

AddContactPage* Protocol::createAddContactWidget( QWidget *parent, Kopete::Account *account )
{
	Q_UNUSED( parent )
	Q_UNUSED( account )
	return 0L;
}

KopeteEditAccountWidget* Protocol::createEditAccountWidget( Kopete::Account *account, QWidget *parent )
{
	Q_UNUSED( account )
	Q_UNUSED( parent )
	return 0L;
}

} // end ns mock
} // end ns test
} // end ns kopete
