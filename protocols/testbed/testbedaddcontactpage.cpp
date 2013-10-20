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
#include <QVBoxLayout>
#include <qlineedit.h>
#include <kdebug.h>

#include "kopeteaccount.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"

#include "testbedcontact.h"

TestbedAddContactPage::TestbedAddContactPage( QWidget* parent )
		: AddContactPage(parent)
{
	kDebug(14210) ;
	QVBoxLayout* l = new QVBoxLayout( this );
	QWidget* w = new QWidget();
	m_testbedAddUI.setupUi( w );
	l->addWidget( w );
	m_testbedAddUI.m_uniqueName->setFocus();
}

TestbedAddContactPage::~TestbedAddContactPage()
{
}

bool TestbedAddContactPage::apply( Kopete::Account* a, Kopete::MetaContact* m )
{
	if ( validateData() )
	{
		QString name = m_testbedAddUI.m_uniqueName->text();

		if ( a->addContact(name, m, Kopete::Account::ChangeKABC ) )
		{
			TestbedContact * newContact = qobject_cast<TestbedContact*>( Kopete::ContactList::self()->findContact( a->protocol()->pluginId(), a->accountId(), name ) );
			if ( newContact )
			{
				newContact->setType( m_testbedAddUI.m_rbEcho->isChecked() ? TestbedContact::Echo : TestbedContact::Group );
				return true;
			}
		}
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
