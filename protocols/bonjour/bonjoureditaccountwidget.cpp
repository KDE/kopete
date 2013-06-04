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
#include <kdebug.h>
#include <KConfigGroup>
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "ui_bonjouraccountpreferences.h"
#include "bonjouraccount.h"
#include "bonjourprotocol.h"
#include <kconfigdialog.h>
#include <kuser.h>
#include <kpimidentities/identity.h>
#include <kpimidentities/identitymanager.h>

BonjourEditAccountWidget::BonjourEditAccountWidget( QWidget* parent, Kopete::Account* account)
: QWidget( parent ), KopeteEditAccountWidget( account )
{
				kDebug() ;
	m_preferencesWidget = new Ui::BonjourAccountPreferences();
	m_preferencesWidget->setupUi( this );

	if (account) {
		group = account->configGroup();
	
		m_preferencesWidget->kcfg_username->setText(group->readEntry("username"));
		m_preferencesWidget->kcfg_firstName->setText(group->readEntry("firstName"));
		m_preferencesWidget->kcfg_lastName->setText(group->readEntry("lastName"));
		m_preferencesWidget->kcfg_emailAddress->setText(group->readEntry("emailAddress"));
	} else {

		// In this block, we populate the default values
		QString firstName, lastName, login, emailAddress;
		QStringList names;

		// Create a KUser object with default values
		// We May be able to get username and Real Name from here
		KUser user = KUser();

		if (user.isValid()) {
			// Get the login name from KUser
			login = user.loginName();

			// First Get the Names from KUser
			names = user.property(KUser::FullName).toString().split(' ');
		}

		// Next try via the default identity
		KPIMIdentities::IdentityManager manager(true);
		const KPIMIdentities::Identity & ident = manager.defaultIdentity();

		if (! ident.isNull()) {
			// Get the full name from identity (only if not available via KUser)
			if ( names.isEmpty() )
				names = ident.fullName().split(' ');

			// Get the email address
			emailAddress = ident.primaryEmailAddress();
		}

		// Split the names array into firstName and lastName
		if (! names.isEmpty()) {
			firstName = names.takeFirst();
			lastName = names.join(" ");
		}

		if (! login.isEmpty())
			m_preferencesWidget->kcfg_username->setText(login);
		if (! firstName.isEmpty())
			m_preferencesWidget->kcfg_firstName->setText(firstName);
		if (! lastName.isEmpty())
			m_preferencesWidget->kcfg_lastName->setText(lastName);
		if (! emailAddress.isEmpty())
			m_preferencesWidget->kcfg_emailAddress->setText(emailAddress);

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
