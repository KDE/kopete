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
#include "ui_icqeditaccountui.h"

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <QLatin1String>

#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kurllabel.h>
#include <kdatewidget.h>
#include <ktoolinvocation.h>
#include <kpassworddialog.h>

#include "kopetepassword.h"
#include "kopetepasswordwidget.h"

#include "icqprotocol.h"
#include "icqaccount.h"
#include "icqcontact.h"

ICQEditAccountWidget::ICQEditAccountWidget(ICQProtocol *protocol,
	Kopete::Account *account, QWidget *parent)
	: QWidget(parent), KopeteEditAccountWidget(account)
{
	kDebug(14153) << k_funcinfo << "Called." << endl;

	mAccount=dynamic_cast<ICQAccount*>(account);
	mProtocol=protocol;

	mAccountSettings = new Ui::ICQEditAccountUI();
	mAccountSettings->setupUi( this );

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
		int portEntry = mAccount->configGroup()->readEntry("Port", 5190);
		if ( serverEntry != "login.oscar.aol.com" || ( portEntry != 5190) )
			mAccountSettings->optionOverrideServer->setChecked( true );

		mAccountSettings->edtServerAddress->setText( serverEntry );
		mAccountSettings->edtServerPort->setValue( portEntry );

		bool configChecked = mAccount->configGroup()->readEntry( "RequireAuth", false );
		mAccountSettings->chkRequireAuth->setChecked( configChecked );

		configChecked = mAccount->configGroup()->readEntry( "RespectRequireAuth", true );
		mAccountSettings->chkRespectRequireAuth->setChecked( configChecked );

		configChecked = mAccount->configGroup()->readEntry( "HideIP", true );
		mAccountSettings->chkHideIP->setChecked( configChecked );

		configChecked = mAccount->configGroup()->readEntry( "WebAware", false );
		mAccountSettings->chkWebAware->setChecked( configChecked );

		int configValue = mAccount->configGroup()->readEntry( "DefaultEncoding", 4 );
		mProtocol->setComboFromTable( mAccountSettings->encodingCombo,
                                      mProtocol->encodings(),
                                      configValue );

		//set filetransfer stuff
		configChecked = mAccount->configGroup()->readEntry( "FileProxy", false );
		mAccountSettings->chkFileProxy->setChecked( configChecked );
		configValue = mAccount->configGroup()->readEntry( "FirstPort", 5190 );
		mAccountSettings->sbxFirstPort->setValue( configValue );
		configValue = mAccount->configGroup()->readEntry( "LastPort", 5199 );
		mAccountSettings->sbxLastPort->setValue( configValue );
		configValue = mAccount->configGroup()->readEntry( "Timeout", 10 );
		mAccountSettings->sbxTimeout->setValue( configValue );

		// Global Identity
		mAccountSettings->chkGlobalIdentity->setChecked( mAccount->configGroup()->readEntry("ExcludeGlobalIdentity", false) );
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
	kDebug(14153) << k_funcinfo << "Called." << endl;

	// If this is a new account, create it
	if (!mAccount)
	{
		kDebug(14153) << k_funcinfo << "Creating a new account" << endl;
		mAccount = new ICQAccount(mProtocol, mAccountSettings->edtAccountId->text());
		if(!mAccount)
			return NULL;
	}

	mAccountSettings->mPasswordWidget->save(&mAccount->password());
	mAccount->setExcludeConnect(mAccountSettings->chkAutoLogin->isChecked());

	bool configChecked = mAccountSettings->chkRequireAuth->isChecked();
	mAccount->configGroup()->writeEntry( "RequireAuth", configChecked );

	configChecked = mAccountSettings->chkRespectRequireAuth->isChecked();
	mAccount->configGroup()->writeEntry( "RespectRequireAuth", configChecked );

	configChecked = mAccountSettings->chkHideIP->isChecked();
	mAccount->configGroup()->writeEntry( "HideIP", configChecked );

	configChecked = mAccountSettings->chkWebAware->isChecked();
	mAccount->configGroup()->writeEntry( "WebAware", configChecked );

	int configValue = mProtocol->getCodeForCombo( mAccountSettings->encodingCombo,
                                                  mProtocol->encodings() );
	mAccount->configGroup()->writeEntry( "DefaultEncoding", configValue );

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

	//set filetransfer stuff
	configChecked = mAccountSettings->chkFileProxy->isChecked();
	mAccount->configGroup()->writeEntry( "FileProxy", configChecked );
	configValue = mAccountSettings->sbxFirstPort->value();
	mAccount->configGroup()->writeEntry( "FirstPort", configValue );
	configValue = mAccountSettings->sbxLastPort->value();
	mAccount->configGroup()->writeEntry( "LastPort", configValue );
	configValue = mAccountSettings->sbxTimeout->value();
	mAccount->configGroup()->writeEntry( "Timeout", configValue );

	// Global Identity
	mAccount->configGroup()->writeEntry( "ExcludeGlobalIdentity", mAccountSettings->chkGlobalIdentity->isChecked() );

	return mAccount;
}

bool ICQEditAccountWidget::validateData()
{
	kDebug(14153) << k_funcinfo << "Called." << endl;

	QString userName = mAccountSettings->edtAccountId->text();

	if (userName.contains(" "))
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
	kDebug(14153) << k_funcinfo <<
		"Account data validated successfully." << endl;
	return true;
}

void ICQEditAccountWidget::slotOpenRegister()
{
	KToolInvocation::invokeBrowser( QLatin1String("http://go.icq.com/register/") );
}

#include "icqeditaccountwidget.moc"
// vim: set noet ts=4 sts=4 sw=4:
// kate: indent-mode csands; space-indent off; replace-tabs off;
