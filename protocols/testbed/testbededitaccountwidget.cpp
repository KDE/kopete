/*
    testbededitaccountwidget.h - Kopete Testbed Protocol

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

#include "testbededitaccountwidget.h"

#include <qlayout.h>
#include <qlineedit.h>
#include <QVBoxLayout>
#include <kdebug.h>
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "ui_testbedaccountpreferences.h"
#include "testbedaccount.h"
#include "testbedprotocol.h"

TestbedEditAccountWidget::TestbedEditAccountWidget( QWidget* parent, Kopete::Account* account)
: QWidget( parent ), KopeteEditAccountWidget( account )
{
	QVBoxLayout *layout = new QVBoxLayout( this );
				kDebug(14210) ;
	QWidget *widget = new QWidget( this );
	m_preferencesWidget = new Ui::TestbedAccountPreferences();
	m_preferencesWidget->setupUi( widget );
	layout->addWidget( widget );
}

TestbedEditAccountWidget::~TestbedEditAccountWidget()
{
	delete m_preferencesWidget;
}

Kopete::Account* TestbedEditAccountWidget::apply()
{
	QString accountName;
	if ( m_preferencesWidget->m_acctName->text().isEmpty() )
		accountName = "Testbed Account";
	else
		accountName = m_preferencesWidget->m_acctName->text();
	
	if ( account() )
		// FIXME: ? account()->setAccountLabel(accountName);
		account()->myself()->setNickName( accountName );
	else
		setAccount( new TestbedAccount( TestbedProtocol::protocol(), accountName ) );

	return account();
}

bool TestbedEditAccountWidget::validateData()
{
    //return !( m_preferencesWidget->m_acctName->text().isEmpty() );
	return true;
}

#include "testbededitaccountwidget.moc"
