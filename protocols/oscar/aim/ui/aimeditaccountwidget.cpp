#include "aimeditaccountwidget.h"
#include "aimeditaccountui.h"

#include <qlayout.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qspinbox.h>

#include <kdebug.h>
#include <krun.h>
#include <kpassdlg.h>

#include "kopetepassword.h"
#include "kopetepasswordwidget.h"

#include "aimprotocol.h"
#include "aimaccount.h"
#include "oscartypes.h"

AIMEditAccountWidget::AIMEditAccountWidget(AIMProtocol *protocol,
	Kopete::Account *account, QWidget *parent, const char *name)
	: QWidget(parent, name), KopeteEditAccountWidget(account)
{
	//kdDebug(14152) << k_funcinfo << "Called." << endl;

	mAccount = dynamic_cast<AIMAccount*>(account);
	mProtocol = protocol;

	// create the gui (generated from a .ui file)
	(new QVBoxLayout(this))->setAutoAdd(true);
	mGui = new aimEditAccountUI(this, "AIMEditAccountWidget::mGui");

	// Read in the settings from the account if it exists
	if (mAccount)
	{
		mGui->mPasswordWidget->load( &mAccount->password() );
		mGui->edtAccountId->setText(account->accountId());
		//Remove me after we can change Account IDs (Matt)
		mGui->edtAccountId->setDisabled(true);
		mGui->mAutoLogon->setChecked(account->excludeConnect());
        if (account->pluginData(protocol, "Server") != "login.oscar.aol.com" || (account->pluginData(protocol, "Port").toInt() != 5190)) {
            mGui->optionOverrideServer->setChecked( true );
        }
		mGui->edtServerAddress->setText(account->pluginData(protocol, "Server"));
		mGui->sbxServerPort->setValue(account->pluginData(protocol,"Port").toInt());
	}
	QObject::connect(mGui->buttonRegister, SIGNAL(clicked()), this, SLOT(slotOpenRegister()));
}

AIMEditAccountWidget::~AIMEditAccountWidget()
{
}

Kopete::Account *AIMEditAccountWidget::apply()
{
	kdDebug(14152) << k_funcinfo << "Called." << endl;

	// If this is a new account, create it
	if (!mAccount)
	{
		kdDebug(14152) << k_funcinfo << "creating a new account" << endl;
		QString newId = mGui->edtAccountId->text();
		mAccount = new AIMAccount(mProtocol, newId);
	}

	mGui->mPasswordWidget->save( &mAccount->password() );

	mAccount->setExcludeConnect(mGui->mAutoLogon->isChecked()); // save the autologon choice
	if (mGui->optionOverrideServer->isChecked()) {
		static_cast<OscarAccount *>(mAccount)->setServerAddress(mGui->edtServerAddress->text());
		static_cast<OscarAccount *>(mAccount)->setServerPort(mGui->sbxServerPort->value());
	}
	else {
		static_cast<OscarAccount *>(mAccount)->setServerAddress("login.oscar.aol.com");
		static_cast<OscarAccount *>(mAccount)->setServerPort(5190);
	}

	return mAccount;
}

bool AIMEditAccountWidget::validateData()
{
	//kdDebug(14152) << k_funcinfo << "Called." << endl;

	QString userName = mGui->edtAccountId->text();
	QString server = mGui->edtServerAddress->text();
	int port = mGui->sbxServerPort->value();

	if (userName.length() < 1)
		return false;

	if (port < 1)
		return false;

	if (server.length() < 1)
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
