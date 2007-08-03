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
//Added by qt3to4:
#include <QVBoxLayout>
#include <qlineedit.h>
#include <kdebug.h>

#include "kopeteaccount.h"
#include "kopetemetacontact.h"
#include "ui_testbedaddui.h"

TestbedAddContactPage::TestbedAddContactPage( QWidget* parent )
		: AddContactPage(parent)
{
	kDebug(14210) << k_funcinfo;
	QVBoxLayout* l = new QVBoxLayout( this );
	QWidget* w = new QWidget();
	m_testbedAddUI = new Ui::TestbedAddUI;
	m_testbedAddUI->setupUi( w );
	l->addWidget( w );
}

TestbedAddContactPage::~TestbedAddContactPage()
{
	delete m_testbedAddUI;
}

bool TestbedAddContactPage::apply( Kopete::Account* a, Kopete::MetaContact* m )
{
    if ( validateData() )
	{
		bool ok = false;
		QString type;
		QString name;
		if ( m_testbedAddUI->m_rbEcho->isChecked() )
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
