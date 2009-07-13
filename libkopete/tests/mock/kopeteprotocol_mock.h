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

#ifndef _KOPETEPROTOCOL_MOCK_H_
#define _KOPETEPROTOCOL_MOCK_H_

#include "kopeteprotocol.h"

class KComponentData;
class QObject;

class KopeteEditAccountWidget;
class AddContactPage;
class KopeteEditAccountWidget;

namespace Kopete
{
namespace Test
{
namespace Mock
{

class Protocol : public Kopete::Protocol
{
public:
	Protocol( const KComponentData &instance, QObject *parent);
	// pure virtual functions implemented
	virtual Account *createNewAccount( const QString &accountId );
	virtual AddContactPage *createAddContactWidget( QWidget *parent, Kopete::Account *account );
	virtual KopeteEditAccountWidget * createEditAccountWidget( Kopete::Account *account, QWidget *parent );
};

} // end ns Kopete::Test::Mock
} // end ns Kopete::Test
} // end ns Kopete


#endif

