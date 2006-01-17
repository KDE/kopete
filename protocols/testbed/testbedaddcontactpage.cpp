/*
    testbedaddcontactpage.cpp - Kopete Testbed Protocol

    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "testbedaddcontactpage.h"

#include <qlayout.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <kdebug.h>

#include "kopeteaccount.h"
#include "kopetemetacontact.h"

#include "testbedaddui.h"

TestbedAddContactPage::TestbedAddContactPage( QWidget* parent, const char* name )
		: AddContactPage(parent, name)
{
	kdDebug(14210) << k_funcinfo << endl;
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	m_testbedAddUI = new TestbedAddUI( this );
}

TestbedAddContactPage::~TestbedAddContactPage()
{
}

bool TestbedAddContactPage::apply( Kopete::Account* a, Kopete::MetaContact* m )
{
    if ( validateData() )
	{
		bool ok = false;
		QString type;
		QString name;
		if ( m_testbedAddUI->m_rbEcho->isOn() )
		{
			type = m_testbedAddUI->m_uniqueName->text();
			name = QString::fromLatin1( "Echo Contact" );
			ok = true;
		}
		if ( ok )
			return a->addContact(type, /* FIXME: ? name, */ m, Kopete::Account::ChangeKABC );
		else
			return false;
	}
	return false;
}

bool TestbedAddContactPage::validateData()
{
    return true;
}


#include "testbedaddcontactpage.moc"
