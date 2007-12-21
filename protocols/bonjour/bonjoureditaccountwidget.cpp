/*
    bonjoureditaccountwidget.h - Kopete Bonjour Protocol

    Copyright (c) 2007      by Tejas Dinkar	<tejas@gja.in>
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

#include "bonjoureditaccountwidget.h"

#include <qlayout.h>
#include <qlineedit.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <kdebug.h>
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "ui_bonjouraccountpreferences.h"
#include "bonjouraccount.h"
#include "bonjourprotocol.h"

BonjourEditAccountWidget::BonjourEditAccountWidget( QWidget* parent, Kopete::Account* account)
: QWidget( parent ), KopeteEditAccountWidget( account )
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );
				kDebug(14210) ;
	m_preferencesWidget = new Ui::BonjourAccountPreferences();
	m_preferencesWidget->setupUi( this );
}

BonjourEditAccountWidget::~BonjourEditAccountWidget()
{
	delete m_preferencesWidget;
}

Kopete::Account* BonjourEditAccountWidget::apply()
{
	if (! account() )
		setAccount( new BonjourAccount ( BonjourProtocol::protocol(), m_preferencesWidget->m_name->text()));

	QByteArray name =  m_preferencesWidget->m_name->text().toUtf8();
	QByteArray fname =  m_preferencesWidget->m_first->text().toUtf8();
	QByteArray lname =  m_preferencesWidget->m_last->text().toUtf8();
	QByteArray email =  m_preferencesWidget->m_email->text().toUtf8();

	account()->setProperty("fullName", name);
	account()->setProperty("firstName", fname);
	account()->setProperty("lastName", lname);
	account()->setProperty("emailAddress", email);

	return account();
}

bool BonjourEditAccountWidget::validateData()
{
   	return !( m_preferencesWidget->m_name->text().isEmpty() );
}

#include "bonjoureditaccountwidget.moc"
