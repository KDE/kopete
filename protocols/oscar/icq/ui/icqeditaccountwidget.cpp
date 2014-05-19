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

#include <QLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QValidator>
#include <QLatin1String>
#include <QLocale>
#include <QPointer>

#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kurllabel.h>
#include <kdatewidget.h>
#include <ktoolinvocation.h>
#include <kpassworddialog.h>
#include <kmessagebox.h>

#include "kopetepassword.h"
#include "kopetepasswordwidget.h"

#include "icqprotocol.h"
#include "icqaccount.h"
#include "icqcontact.h"
#include "oscarprivacyengine.h"
#include "oscarsettings.h"
#include "icqchangepassworddialog.h"

ICQEditAccountWidget::ICQEditAccountWidget(ICQProtocol *protocol,
	Kopete::Account *account, QWidget *parent)
	: QWidget(parent), KopeteEditAccountWidget(account)
{
	kDebug(14153) << "Called.";

	mAccount=dynamic_cast<ICQAccount*>(account);
	mProtocol=protocol;

	m_visibleEngine = 0;
	m_invisibleEngine = 0;
	m_ignoreEngine = 0;

	mAccountSettings = new Ui::ICQEditAccountUI();
	mAccountSettings->setupUi( this );

	mProtocol->fillComboFromTable( mAccountSettings->encodingCombo, mProtocol->encodings() );

	//Setup the edtAccountId
	QRegExp rx("[0-9]{9}");
	QValidator* validator = new QRegExpValidator( rx, this );
	mAccountSettings->edtAccountId->setValidator(validator);

	// Read in the settings from the account if it exists
	if(mAccount)
	{
		mAccountSettings->edtAccountId->setText(mAccount->accountId());

		// TODO: Remove me after we can change Account IDs (Matt)
		mAccountSettings->edtAccountId->setReadOnly(true);
		mAccountSettings->mPasswordWidget->load(&mAccount->password());
		mAccountSettings->chkAutoLogin->setChecked(mAccount->excludeConnect());

		QString serverEntry = mAccount->configGroup()->readEntry("Server", "slogin.icq.com");
		int portEntry = mAccount->configGroup()->readEntry("Port", 443);
		bool encryptedEntry = mAccount->configGroup()->readEntry("Encrypted", ( serverEntry == "slogin.icq.com" && portEntry == 443 ));
		if ( ( encryptedEntry && ( serverEntry != "slogin.icq.com" || portEntry != 443 ) ) ||
				( ! encryptedEntry && ( serverEntry != "login.icq.com" || ( portEntry != 5190 ) ) ) )
			mAccountSettings->optionOverrideServer->setChecked( true );
		else
			mAccountSettings->optionOverrideServer->setChecked( false );

		mAccountSettings->edtServerAddress->setText( serverEntry );
		mAccountSettings->edtServerPort->setValue( portEntry );
		mAccountSettings->edtServerEncrypted->setChecked( encryptedEntry );

		bool proxyServerEnableEntry = mAccount->configGroup()->readEntry("ProxyEnable", false);
		bool proxyServerSocks5Entry = mAccount->configGroup()->readEntry("ProxySocks5", false);
		QString proxyServerEntry = mAccount->configGroup()->readEntry("ProxyServer", QString());
		int proxyPortEntry = mAccount->configGroup()->readEntry("ProxyPort", 443);
		mAccountSettings->optionEnableProxy->setChecked( proxyServerEnableEntry );
		mAccountSettings->edtProxyServerSocks5->setChecked( proxyServerSocks5Entry );
		mAccountSettings->edtProxyServerAddress->setText( proxyServerEntry );
		mAccountSettings->edtProxyServerPort->setValue( proxyPortEntry );


		bool configChecked = mAccount->configGroup()->readEntry( "RequireAuth", false );
		mAccountSettings->chkRequireAuth->setChecked( configChecked );

		configChecked = mAccount->configGroup()->readEntry( "HideIP", true );
		mAccountSettings->chkHideIP->setChecked( configChecked );

		configChecked = mAccount->configGroup()->readEntry( "WebAware", false );
		mAccountSettings->chkWebAware->setChecked( configChecked );

		int configValue = mAccount->configGroup()->readEntry( "DefaultEncoding", 4 );
		mProtocol->setComboFromTable( mAccountSettings->encodingCombo,
                                      mProtocol->encodings(),
                                      configValue );

		//set filetransfer stuff
		configChecked = mAccount->configGroup()->readEntry( "FileProxy", true );
		mAccountSettings->chkFileProxy->setChecked( configChecked );
		configValue = mAccount->configGroup()->readEntry( "FirstPort", 5190 );
		mAccountSettings->sbxFirstPort->setValue( configValue );
		configValue = mAccount->configGroup()->readEntry( "LastPort", 5199 );
		mAccountSettings->sbxLastPort->setValue( configValue );
		configValue = mAccount->configGroup()->readEntry( "Timeout", 10 );
		mAccountSettings->sbxTimeout->setValue( configValue );

		if ( mAccount->engine()->isActive() )
		{
			m_visibleEngine = new OscarPrivacyEngine( mAccount, OscarPrivacyEngine::Visible );
			m_visibleEngine->setAllContactsView( mAccountSettings->visibleAllContacts );
			m_visibleEngine->setContactsView( mAccountSettings->visibleContacts );
			QObject::connect( mAccountSettings->visibleAdd, SIGNAL(clicked()), m_visibleEngine, SLOT(slotAdd()) );
			QObject::connect( mAccountSettings->visibleRemove, SIGNAL(clicked()), m_visibleEngine, SLOT(slotRemove()) );

			m_invisibleEngine = new OscarPrivacyEngine( mAccount, OscarPrivacyEngine::Invisible );
			m_invisibleEngine->setAllContactsView( mAccountSettings->invisibleAllContacts );
			m_invisibleEngine->setContactsView( mAccountSettings->invisibleContacts );
			QObject::connect( mAccountSettings->invisibleAdd, SIGNAL(clicked()), m_invisibleEngine, SLOT(slotAdd()) );
			QObject::connect( mAccountSettings->invisibleRemove, SIGNAL(clicked()), m_invisibleEngine, SLOT(slotRemove()) );

			m_ignoreEngine = new OscarPrivacyEngine( mAccount, OscarPrivacyEngine::Ignore );
			m_ignoreEngine->setAllContactsView( mAccountSettings->ignoreAllContacts );
			m_ignoreEngine->setContactsView( mAccountSettings->ignoreContacts );
			QObject::connect( mAccountSettings->ignoreAdd, SIGNAL(clicked()), m_ignoreEngine, SLOT(slotAdd()) );
			QObject::connect( mAccountSettings->ignoreRemove, SIGNAL(clicked()), m_ignoreEngine, SLOT(slotRemove()) );
		}
		// Hide the registration UI when editing an existing account
		mAccountSettings->registrationGroupBox->hide();
	}
	else
	{
		int encodingId=4; //see ../icqprotocol.* for mappings
		switch (QLocale::system().language())
		{
			case QLocale::Russian:
			case QLocale::Ukrainian:
			case QLocale::Byelorussian:
			case QLocale::Bulgarian:
				encodingId=CP1251;
				break;
			case QLocale::English:
			case QLocale::German:
			case QLocale::Italian:
			case QLocale::Spanish:
			case QLocale::Portuguese:
			case QLocale::French:
			case QLocale::Dutch:
			case QLocale::Danish:
			case QLocale::Swedish:
			case QLocale::Norwegian:
			case QLocale::Icelandic:
				encodingId=CP1252;
				break;
			case QLocale::Greek:
				encodingId=CP1253;
				break;
			case QLocale::Turkish:
				encodingId=CP1254;
				break;
			case QLocale::Hebrew:
				encodingId=CP1255;
				break;
			case QLocale::Arabic:
				encodingId=CP1256;
				break;
			default:
				encodingId=4;
		}

		mProtocol->setComboFromTable( mAccountSettings->encodingCombo,
		                              mProtocol->encodings(),
		                              encodingId );
		mAccountSettings->changePasswordGroupBox->hide();
	}

	if ( !mAccount || !mAccount->engine()->isActive() )
	{
		mAccountSettings->tabVisible->setEnabled( false );
		mAccountSettings->tabInvisible->setEnabled( false );
		mAccountSettings->tabIgnore->setEnabled( false );
		mAccountSettings->buttonChangePassword->setEnabled( false );
	}

	QObject::connect(mAccountSettings->buttonRegister, SIGNAL(clicked()), this, SLOT(slotOpenRegister()));
	QObject::connect(mAccountSettings->buttonChangePassword, SIGNAL(clicked()), this, SLOT(slotChangePassword()));

	/* Set tab order to password custom widget correctly */
	QWidget::setTabOrder( mAccountSettings->edtAccountId, mAccountSettings->mPasswordWidget->mRemembered );
	QWidget::setTabOrder( mAccountSettings->mPasswordWidget->mRemembered, mAccountSettings->mPasswordWidget->mPassword );
	QWidget::setTabOrder( mAccountSettings->mPasswordWidget->mPassword, mAccountSettings->chkAutoLogin );

}

ICQEditAccountWidget::~ICQEditAccountWidget()
{
	delete m_visibleEngine;
	delete m_invisibleEngine;
	delete m_ignoreEngine;
	delete mAccountSettings;
}

Kopete::Account *ICQEditAccountWidget::apply()
{
	kDebug(14153) << "Called.";

	// If this is a new account, create it
	if (!mAccount)
	{
		kDebug(14153) << "Creating a new account";
		mAccount = new ICQAccount(mProtocol, mAccountSettings->edtAccountId->text());
	}

	mAccountSettings->mPasswordWidget->save(&mAccount->password());
	mAccount->setExcludeConnect(mAccountSettings->chkAutoLogin->isChecked());

	Oscar::Settings* oscarSettings = mAccount->engine()->clientSettings();

	bool configChecked = mAccountSettings->chkRequireAuth->isChecked();
	mAccount->configGroup()->writeEntry( "RequireAuth", configChecked );
	oscarSettings->setRequireAuth( configChecked );

	configChecked = mAccountSettings->chkHideIP->isChecked();
	mAccount->configGroup()->writeEntry( "HideIP", configChecked );
	oscarSettings->setHideIP( configChecked );

	configChecked = mAccountSettings->chkWebAware->isChecked();
	mAccount->configGroup()->writeEntry( "WebAware", configChecked );
	oscarSettings->setWebAware( configChecked );

	int configValue = mProtocol->getCodeForCombo( mAccountSettings->encodingCombo,
                                                  mProtocol->encodings() );
	mAccount->configGroup()->writeEntry( "DefaultEncoding", configValue );

	bool encrypted = mAccountSettings->edtServerEncrypted->isChecked();

	mAccount->setServerEncrypted(encrypted);

	if ( mAccountSettings->optionOverrideServer->isChecked() )
	{
		mAccount->setServerAddress(mAccountSettings->edtServerAddress->text().trimmed());
		mAccount->setServerPort(mAccountSettings->edtServerPort->value());
	}
	else
	{
		mAccount->setServerAddress(encrypted ? "slogin.icq.com" : "login.icq.com");
		mAccount->setServerPort(encrypted ? 443 : 5190);
	}

	bool useProxy=mAccountSettings->optionEnableProxy->isChecked();
	mAccount->setProxyServerEnabled( useProxy );
	if ( mAccountSettings->optionEnableProxy->isChecked() )
	{
		mAccount->setProxyServerSocks5(mAccountSettings->edtProxyServerSocks5->isChecked());
		mAccount->setProxyServerAddress(mAccountSettings->edtProxyServerAddress->text().trimmed());
		mAccount->setProxyServerPort(mAccountSettings->edtProxyServerPort->value());
	}

	//set filetransfer stuff
	configChecked = mAccountSettings->chkFileProxy->isChecked();
	mAccount->configGroup()->writeEntry( "FileProxy", configChecked );
	oscarSettings->setFileProxy( configChecked );

	configValue = mAccountSettings->sbxFirstPort->value();
	mAccount->configGroup()->writeEntry( "FirstPort", configValue );
	oscarSettings->setFirstPort( configValue );

	configValue = mAccountSettings->sbxLastPort->value();
	mAccount->configGroup()->writeEntry( "LastPort", configValue );
	oscarSettings->setLastPort( configValue );

	configValue = mAccountSettings->sbxTimeout->value();
	mAccount->configGroup()->writeEntry( "Timeout", configValue );
	oscarSettings->setTimeout( configValue );

	if ( mAccount->engine()->isActive() )
	{
		if ( m_visibleEngine )
			m_visibleEngine->storeChanges();

		if ( m_invisibleEngine )
			m_invisibleEngine->storeChanges();

		if ( m_ignoreEngine )
			m_ignoreEngine->storeChanges();

		//Update Oscar settings
		static_cast<ICQMyselfContact*>( mAccount->myself() )->fetchShortInfo();
	}

	return mAccount;
}

bool ICQEditAccountWidget::validateData()
{
	kDebug(14153) << "Called.";
	bool bOk;
	QString userId = mAccountSettings->edtAccountId->text();
	qulonglong uid = userId.toULongLong( &bOk );

	if( !bOk || uid == 0 || userId.isEmpty() )
	{	KMessageBox::queuedMessageBox(this, KMessageBox::Sorry,
	 	                              i18n("<qt>You must enter a valid ICQ No.</qt>"), i18n("ICQ"));
		return false;
	}

	// No need to check port, min and max values are properly defined in .ui

	if (mAccountSettings->edtServerAddress->text().isEmpty())
		return false;

	// Seems good to me
	kDebug(14153) <<
		"Account data validated successfully." << endl;
	return true;
}

void ICQEditAccountWidget::slotOpenRegister()
{
	KToolInvocation::invokeBrowser( QLatin1String("https://www.icq.com/register/") );
}

void ICQEditAccountWidget::slotChangePassword()
{
	QPointer <ICQChangePasswordDialog> passwordDlg = new ICQChangePasswordDialog( mAccount, this );
	passwordDlg->exec();
	delete passwordDlg;
}

#include "icqeditaccountwidget.moc"
// vim: set noet ts=4 sts=4 sw=4:
// kate: indent-mode csands; space-indent off; replace-tabs off;
