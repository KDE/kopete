#include "aimeditaccountwidget.h"
#include "aimeditaccountui.h"

#include <qlayout.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qspinbox.h>

#include <kdebug.h>
#include <krun.h>
#include <kpassdlg.h>
#include <kconfig.h>

#include "kopetepassword.h"
#include "kopetepasswordwidget.h"

#include "aimprotocol.h"
#include "aimaccount.h"

AIMEditAccountWidget::AIMEditAccountWidget( AIMProtocol *protocol,
        Kopete::Account *account, QWidget *parent, const char *name )
		: QWidget( parent, name ), KopeteEditAccountWidget( account )
{
	//kdDebug(14152) << k_funcinfo << "Called." << endl;

	mAccount = dynamic_cast<AIMAccount*>( account );
	mProtocol = protocol;

	// create the gui (generated from a .ui file)
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	mGui = new aimEditAccountUI( this, "AIMEditAccountWidget::mGui" );

	// Read in the settings from the account if it exists
	if ( mAccount )
	{
		mGui->mPasswordWidget->load( &mAccount->password() );
		mGui->edtAccountId->setText( account->accountId() );
		//Remove me after we can change Account IDs (Matt)
		mGui->edtAccountId->setDisabled( true );
		mGui->mAutoLogon->setChecked( account->excludeConnect() );
		QString serverEntry = account->configGroup()->readEntry( "Server", "login.oscar.aol.com" );
		int portEntry = account->configGroup()->readNumEntry( "Port", 5190 );
		if ( serverEntry != "login.oscar.aol.com" || portEntry != 5190 )
			mGui->optionOverrideServer->setChecked( true );
		else
			mGui->optionOverrideServer->setChecked( false );

		mGui->edtServerAddress->setText( serverEntry );
		mGui->sbxServerPort->setValue( portEntry );

		using namespace AIM::PrivacySettings;

		int privacySetting = mAccount->configGroup()->readNumEntry( "PrivacySetting", AllowAll );
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

		// Global Identity
		mGui->mGlobalIdentity->setChecked( account->configGroup()->readBoolEntry("ExcludeGlobalIdentity", false) );
    }
	QObject::connect( mGui->buttonRegister, SIGNAL( clicked() ), this, SLOT( slotOpenRegister() ) );

	/* Set tab order to password custom widget correctly */
	QWidget::setTabOrder( mGui->edtAccountId, mGui->mPasswordWidget->mRemembered );
	QWidget::setTabOrder( mGui->mPasswordWidget->mRemembered, mGui->mPasswordWidget->mPassword );
	QWidget::setTabOrder( mGui->mPasswordWidget->mPassword, mGui->mAutoLogon );
}

AIMEditAccountWidget::~AIMEditAccountWidget()
{}

Kopete::Account *AIMEditAccountWidget::apply()
{
	kdDebug( 14152 ) << k_funcinfo << "Called." << endl;

	// If this is a new account, create it
	if ( !mAccount )
	{
		kdDebug( 14152 ) << k_funcinfo << "creating a new account" << endl;
		QString newId = mGui->edtAccountId->text();
		mAccount = new AIMAccount( mProtocol, newId );
	}

	mGui->mPasswordWidget->save( &mAccount->password() );

	mAccount->setExcludeConnect( mGui->mAutoLogon->isChecked() ); // save the autologon choice
	if ( mGui->optionOverrideServer->isChecked() )
	{
		static_cast<OscarAccount *>( mAccount )->setServerAddress( mGui->edtServerAddress->text() );
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

	// Global Identity
	mAccount->configGroup()->writeEntry( "ExcludeGlobalIdentity", mGui->mGlobalIdentity->isChecked() );
	return mAccount;
}

bool AIMEditAccountWidget::validateData()
{
	//kdDebug(14152) << k_funcinfo << "Called." << endl;

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
	//kdDebug(14152) << k_funcinfo << "Account data validated successfully." << endl;
	return true;
}

void AIMEditAccountWidget::slotOpenRegister()
{
	KRun::runURL( "http://my.screenname.aol.com/_cqr/login/login.psp?siteId=snshomepage&mcState=initialized&createSn=1", "text/html" );
}

#include "aimeditaccountwidget.moc"
// vim: set noet ts=4 sts=4 sw=4:
