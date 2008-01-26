/*
    icqeditaccountwidget.cpp - ICQ Account Widget

    Copyright (c) 2003 by Chris TenHarmsel  <tenharmsel@staticmethod.net>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "icqeditaccountwidget.h"
#include "icqeditaccountui.h"

#include <qlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qspinbox.h>
#include <qpushbutton.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kjanuswidget.h>
#include <kurllabel.h>
#include <kdatewidget.h>
#include <krun.h>
#include <kpassdlg.h>

#include "kopetepassword.h"
#include "kopetepasswordwidget.h"

#include "icqprotocol.h"
#include "icqaccount.h"
#include "icqcontact.h"

ICQEditAccountWidget::ICQEditAccountWidget(ICQProtocol *protocol,
	Kopete::Account *account, QWidget *parent, const char *name)
	: QWidget(parent, name), KopeteEditAccountWidget(account)
{
	kdDebug(14153) << k_funcinfo << "Called." << endl;

	mAccount=dynamic_cast<ICQAccount*>(account);
	mProtocol=protocol;

	(new QVBoxLayout(this))->setAutoAdd(true);
	mAccountSettings = new ICQEditAccountUI( this );

    mProtocol->fillComboFromTable( mAccountSettings->encodingCombo, mProtocol->encodings() );

	// Read in the settings from the account if it exists
	if(mAccount)
	{
		mAccountSettings->edtAccountId->setText(mAccount->accountId());

		// TODO: Remove me after we can change Account IDs (Matt)
		mAccountSettings->edtAccountId->setDisabled(true);
		mAccountSettings->mPasswordWidget->load(&mAccount->password());
		mAccountSettings->chkAutoLogin->setChecked(mAccount->excludeConnect());

		QString serverEntry = mAccount->configGroup()->readEntry("Server", "login.oscar.aol.com");
		int portEntry = mAccount->configGroup()->readNumEntry("Port", 5190);
		if ( serverEntry != "login.oscar.aol.com" || ( portEntry != 5190) )
			mAccountSettings->optionOverrideServer->setChecked( true );

		mAccountSettings->edtServerAddress->setText( serverEntry );
		mAccountSettings->edtServerPort->setValue( portEntry );

        bool configValue = mAccount->configGroup()->readBoolEntry( "RequireAuth", false );
		mAccountSettings->chkRequireAuth->setChecked( configValue );

        configValue = mAccount->configGroup()->readBoolEntry( "HideIP", true );
		mAccountSettings->chkHideIP->setChecked( configValue );

        configValue = mAccount->configGroup()->readBoolEntry( "WebAware", false );
		mAccountSettings->chkWebAware->setChecked( configValue );

        int encodingValue = mAccount->configGroup()->readNumEntry( "DefaultEncoding", 4 );
        mProtocol->setComboFromTable( mAccountSettings->encodingCombo,
                                      mProtocol->encodings(),
                                      encodingValue );

		// Global Identity
		mAccountSettings->chkGlobalIdentity->setChecked( mAccount->configGroup()->readBoolEntry("ExcludeGlobalIdentity", false) );
	}
	else
	{
		mProtocol->setComboFromTable( mAccountSettings->encodingCombo,
		                              mProtocol->encodings(),
		                              4 );
	}

	QObject::connect(mAccountSettings->buttonRegister, SIGNAL(clicked()), this, SLOT(slotOpenRegister()));

	/* Set tab order to password custom widget correctly */
	QWidget::setTabOrder( mAccountSettings->edtAccountId, mAccountSettings->mPasswordWidget->mRemembered );
	QWidget::setTabOrder( mAccountSettings->mPasswordWidget->mRemembered, mAccountSettings->mPasswordWidget->mPassword );
	QWidget::setTabOrder( mAccountSettings->mPasswordWidget->mPassword, mAccountSettings->chkAutoLogin );

}

Kopete::Account *ICQEditAccountWidget::apply()
{
	kdDebug(14153) << k_funcinfo << "Called." << endl;

	// If this is a new account, create it
	if (!mAccount)
	{
		kdDebug(14153) << k_funcinfo << "Creating a new account" << endl;
		mAccount = new ICQAccount(mProtocol, mAccountSettings->edtAccountId->text());
		if(!mAccount)
			return NULL;
	}

	mAccountSettings->mPasswordWidget->save(&mAccount->password());
	mAccount->setExcludeConnect(mAccountSettings->chkAutoLogin->isChecked());

    bool configValue = mAccountSettings->chkRequireAuth->isChecked();
	mAccount->configGroup()->writeEntry( "RequireAuth", configValue );

    configValue = mAccountSettings->chkHideIP->isChecked();
	mAccount->configGroup()->writeEntry( "HideIP", configValue );

    configValue = mAccountSettings->chkWebAware->isChecked();
	mAccount->configGroup()->writeEntry( "WebAware", configValue );

    int encodingMib = mProtocol->getCodeForCombo( mAccountSettings->encodingCombo,
                                                  mProtocol->encodings() );
    mAccount->configGroup()->writeEntry( "DefaultEncoding", encodingMib );

	if ( mAccountSettings->optionOverrideServer->isChecked() )
	{
		mAccount->setServerAddress(mAccountSettings->edtServerAddress->text());
		mAccount->setServerPort(mAccountSettings->edtServerPort->value());
	}
	else
	{
		mAccount->setServerAddress("login.oscar.aol.com");
		mAccount->setServerPort(5190);
	}
	
	// Global Identity
	mAccount->configGroup()->writeEntry( "ExcludeGlobalIdentity", mAccountSettings->chkGlobalIdentity->isChecked() );

	return mAccount;
}

bool ICQEditAccountWidget::validateData()
{
	kdDebug(14153) << k_funcinfo << "Called." << endl;

	QString userName = mAccountSettings->edtAccountId->text();

	if (userName.isEmpty())
		return false;

	for (unsigned int i=0; i<userName.length(); i++)
	{
		if(!(userName[i]).isNumber())
			return false;
	}

	// No need to check port, min and max values are properly defined in .ui

	if (mAccountSettings->edtServerAddress->text().isEmpty())
		return false;

	// Seems good to me
	kdDebug(14153) << k_funcinfo <<
		"Account data validated successfully." << endl;
	return true;
}

void ICQEditAccountWidget::slotOpenRegister()
{
	KRun::runURL( "http://go.icq.com/register/", "text/html" );
}

#include "icqeditaccountwidget.moc"
// vim: set noet ts=4 sts=4 sw=4:
// kate: indent-mode csands; space-indent off; replace-tabs off;
