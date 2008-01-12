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
#include <KConfigGroup>
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "ui_bonjouraccountpreferences.h"
#include "bonjouraccount.h"
#include "bonjourprotocol.h"
#include <kconfigdialog.h>

BonjourEditAccountWidget::BonjourEditAccountWidget( QWidget* parent, Kopete::Account* account)
: QWidget( parent ), KopeteEditAccountWidget( account )
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );
				kDebug(14210) ;
	m_preferencesWidget = new Ui::BonjourAccountPreferences();
	m_preferencesWidget->setupUi( this );

	if (account) {
		group = account->configGroup();
	
		m_preferencesWidget->kcfg_username->setText(group->readEntry("username"));
		m_preferencesWidget->kcfg_firstName->setText(group->readEntry("firstName"));
		m_preferencesWidget->kcfg_lastName->setText(group->readEntry("lastName"));
		m_preferencesWidget->kcfg_emailAddress->setText(group->readEntry("emailAddress"));
	}
}

BonjourEditAccountWidget::~BonjourEditAccountWidget()
{
	delete m_preferencesWidget;
}

Kopete::Account* BonjourEditAccountWidget::apply()
{
	if (! account() ) {
		setAccount( new BonjourAccount ( BonjourProtocol::protocol(), m_preferencesWidget->kcfg_username->text()));
		group = account()->configGroup();
	}

	group->writeEntry("username", m_preferencesWidget->kcfg_username->text());
	group->writeEntry("firstName", m_preferencesWidget->kcfg_firstName->text());
	group->writeEntry("lastName", m_preferencesWidget->kcfg_lastName->text());
	group->writeEntry("emailAddress", m_preferencesWidget->kcfg_emailAddress->text());

	((BonjourAccount *)account())->parseConfig();

	return account();
}

bool BonjourEditAccountWidget::validateData()
{
   	return !( m_preferencesWidget->kcfg_username->text().isEmpty() );
}

#include "bonjoureditaccountwidget.moc"
