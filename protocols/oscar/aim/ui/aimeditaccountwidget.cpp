#include "aimeditaccountwidget.h"
#include "ui_aimeditaccountui.h"

#include <qlayout.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <QLatin1String>

#include <kdebug.h>
#include <ktoolinvocation.h>
#include <kpassworddialog.h>
#include <kconfig.h>

#include "kopetepassword.h"
#include "kopetepasswordwidget.h"

#include "aimprotocol.h"
#include "aimaccount.h"

#include "oscarprivacyengine.h"

AIMEditAccountWidget::AIMEditAccountWidget( AIMProtocol *protocol,
        Kopete::Account *account, QWidget *parent )
		: QWidget( parent ), KopeteEditAccountWidget( account )
{
	//kDebug(14152) << "Called.";

	mAccount = dynamic_cast<AIMAccount*>( account );
	mProtocol = protocol;

	m_visibleEngine = 0;
	m_invisibleEngine = 0;
	
	// create the gui (generated from a .ui file)
	mGui = new Ui::aimEditAccountUI();
	mGui->setupUi( this );

	// Read in the settings from the account if it exists
	if ( mAccount )
	{
		mGui->mPasswordWidget->load( &mAccount->password() );
		mGui->edtAccountId->setText( account->accountId() );
		//Remove me after we can change Account IDs (Matt)
		mGui->edtAccountId->setReadOnly( true );
		mGui->mAutoLogon->setChecked( account->excludeConnect() );
		QString serverEntry = account->configGroup()->readEntry( "Server", "login.oscar.aol.com" );
		int portEntry = account->configGroup()->readEntry( "Port", 5190 );
		if ( serverEntry != "login.oscar.aol.com" || portEntry != 5190 )
			mGui->optionOverrideServer->setChecked( true );
		else
			mGui->optionOverrideServer->setChecked( false );

		mGui->edtServerAddress->setText( serverEntry );
		mGui->sbxServerPort->setValue( portEntry );

		using namespace AIM::PrivacySettings;

		int privacySetting = mAccount->configGroup()->readEntry( "PrivacySetting", int(AllowAll) );
		switch( privacySetting )
		{
			case AllowAll:
				mGui->rbAllowAll->setChecked( true );
				break;
			case AllowMyContacts:
				mGui->rbAllowMyContacts->setChecked( true );
				break;
			case AllowPremitList:
				mGui->rbAllowPerimtList->setChecked( true );
				break;
			case BlockAll:
				mGui->rbBlockAll->setChecked( true );
				break;
			case BlockAIM:
				mGui->rbBlockAIM->setChecked( true );
				break;
			case BlockDenyList:
				mGui->rbBlockDenyList->setChecked( true );
				break;
			default:
				mGui->rbAllowAll->setChecked( true );
		}

		//set filetransfer stuff
		bool configChecked = mAccount->configGroup()->readEntry( "FileProxy", true );
		mGui->chkFileProxy->setChecked( configChecked );
		int configValue = mAccount->configGroup()->readEntry( "FirstPort", 5190 );
		mGui->sbxFirstPort->setValue( configValue );
		//mGui->sbxFirstPort->setDisabled( configChecked );
		configValue = mAccount->configGroup()->readEntry( "LastPort", 5199 );
		mGui->sbxLastPort->setValue( configValue );
		//mGui->sbxLastPort->setDisabled( configChecked );
		configValue = mAccount->configGroup()->readEntry( "Timeout", 10 );
		mGui->sbxTimeout->setValue( configValue );


		if ( mAccount->engine()->isActive() )
		{
			m_visibleEngine = new OscarPrivacyEngine( mAccount, OscarPrivacyEngine::Visible );
			m_visibleEngine->setAllContactsView( mGui->visibleAllContacts );
			m_visibleEngine->setContactsView( mGui->visibleContacts );
			QObject::connect( mGui->visibleAdd, SIGNAL(clicked()), m_visibleEngine, SLOT(slotAdd()) );
			QObject::connect( mGui->visibleRemove, SIGNAL(clicked()), m_visibleEngine, SLOT(slotRemove()) );

			m_invisibleEngine = new OscarPrivacyEngine( mAccount, OscarPrivacyEngine::Invisible );
			m_invisibleEngine->setAllContactsView( mGui->invisibleAllContacts );
			m_invisibleEngine->setContactsView( mGui->invisibleContacts );
			QObject::connect( mGui->invisibleAdd, SIGNAL(clicked()), m_invisibleEngine, SLOT(slotAdd()) );
			QObject::connect( mGui->invisibleRemove, SIGNAL(clicked()), m_invisibleEngine, SLOT(slotRemove()) );
		}

		// Hide the register UI if editing an existing account
		mGui->registrationGroupBox->hide();
    }
	QObject::connect( mGui->buttonRegister, SIGNAL(clicked()), this, SLOT(slotOpenRegister()) );

	if ( !mAccount || !mAccount->engine()->isActive() )
	{
		mGui->visibleTab->setEnabled( false );
		mGui->invisibleTab->setEnabled( false );
	}

	/* Set tab order to password custom widget correctly */
	QWidget::setTabOrder( mGui->edtAccountId, mGui->mPasswordWidget->mRemembered );
	QWidget::setTabOrder( mGui->mPasswordWidget->mRemembered, mGui->mPasswordWidget->mPassword );
	QWidget::setTabOrder( mGui->mPasswordWidget->mPassword, mGui->mAutoLogon );
}

AIMEditAccountWidget::~AIMEditAccountWidget()
{
	delete m_visibleEngine;
	delete m_invisibleEngine;
	delete mGui;
}

Kopete::Account *AIMEditAccountWidget::apply()
{
	kDebug( 14152 ) << "Called.";

	// If this is a new account, create it
	if ( !mAccount )
	{
		kDebug( 14152 ) << "creating a new account";
		QString newId = mGui->edtAccountId->text();
		mAccount = new AIMAccount( mProtocol, newId );
	}

	mGui->mPasswordWidget->save( &mAccount->password() );

	mAccount->setExcludeConnect( mGui->mAutoLogon->isChecked() ); // save the autologon choice
	if ( mGui->optionOverrideServer->isChecked() )
	{
		static_cast<OscarAccount *>( mAccount )->setServerAddress( mGui->edtServerAddress->text().trimmed() );
		static_cast<OscarAccount *>( mAccount )->setServerPort( mGui->sbxServerPort->value() );
	}
	else
	{
		static_cast<OscarAccount *>( mAccount )->setServerAddress( "login.oscar.aol.com" );
		static_cast<OscarAccount *>( mAccount )->setServerPort( 5190 );
	}

	using namespace AIM::PrivacySettings;
	int privacySetting = AllowAll;

	if ( mGui->rbAllowAll->isChecked() )
		privacySetting = AllowAll;
	else if ( mGui->rbAllowMyContacts->isChecked() )
		privacySetting = AllowMyContacts;
	else if ( mGui->rbAllowPerimtList->isChecked() )
		privacySetting = AllowPremitList;
	else if ( mGui->rbBlockAll->isChecked() )
		privacySetting = BlockAll;
	else if ( mGui->rbBlockAIM->isChecked() )
		privacySetting = BlockAIM;
	else if ( mGui->rbBlockDenyList->isChecked() )
		privacySetting = BlockDenyList;

	mAccount->configGroup()->writeEntry( "PrivacySetting", privacySetting );
	mAccount->setPrivacySettings( privacySetting );

	//set filetransfer stuff
	bool configChecked = mGui->chkFileProxy->isChecked();
	mAccount->configGroup()->writeEntry( "FileProxy", configChecked );
	int configValue = mGui->sbxFirstPort->value();
	mAccount->configGroup()->writeEntry( "FirstPort", configValue );
	configValue = mGui->sbxLastPort->value();
	mAccount->configGroup()->writeEntry( "LastPort", configValue );
	configValue = mGui->sbxTimeout->value();
	mAccount->configGroup()->writeEntry( "Timeout", configValue );

	if ( mAccount->engine()->isActive() )
	{
		if ( m_visibleEngine )
			m_visibleEngine->storeChanges();
		
		if ( m_invisibleEngine )
			m_invisibleEngine->storeChanges();
	}
	return mAccount;
}

bool AIMEditAccountWidget::validateData()
{
	//kDebug(14152) << "Called.";

	QString userName = mGui->edtAccountId->text();
	QString server = mGui->edtServerAddress->text();
	int port = mGui->sbxServerPort->value();

	if ( userName.length() < 1 )
		return false;

	if ( port < 1 )
		return false;

	if ( server.length() < 1 )
		return false;

	// Seems good to me
	//kDebug(14152) << "Account data validated successfully.";
	return true;
}

void AIMEditAccountWidget::slotOpenRegister()
{
	KToolInvocation::invokeBrowser( QString::fromLatin1("http://my.screenname.aol.com/_cqr/login/login.psp?siteId=snshomepage&mcState=initialized&createSn=1") );
}

#include "aimeditaccountwidget.moc"
// vim: set noet ts=4 sts=4 sw=4:
