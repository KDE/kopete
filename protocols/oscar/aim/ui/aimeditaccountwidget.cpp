#include "aimeditaccountwidget.h"
#include "aimeditaccountui.h"

#include <qlayout.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qspinbox.h>

#include <kdebug.h>

#include "aimprotocol.h"
#include "aimaccount.h"
#include "oscartypes.h"

AIMEditAccountWidget::AIMEditAccountWidget(AIMProtocol *protocol,
	KopeteAccount *account, QWidget *parent, const char *name)
	: QWidget(parent, name), KopeteEditAccountWidget(account)
{
	//kdDebug(14190) << k_funcinfo << "Called." << endl;

	mAccount = account;
	mProtocol = protocol;

	// create the gui (generated from a .ui file)
	(new QVBoxLayout(this))->setAutoAdd(true);
	mGui = new aimEditAccountUI(this, "AIMEditAccountWidget::mGui");

	connect(mGui->btnServerDefaults, SIGNAL(clicked()),
	this, SLOT(slotSetDefaultServer()));


	// Read in the settings from the account if it exists
	if (account)
	{
		if (account->rememberPassword())
		{ // If we want to remember the password
			mGui->mSavePassword->setChecked(true);
			mGui->edtPassword->setText(account->password(false, 0L, 16));
		}
		mGui->edtAccountId->setText(account->accountId());
		//Remove me after we can change Account IDs (Matt)
		mGui->edtAccountId->setDisabled(true);
		mGui->mAutoLogon->setChecked(account->autoLogin());
		mGui->edtServerAddress->setText(account->pluginData(protocol, "Server"));
		mGui->sbxServerPort->setValue(account->pluginData(protocol,"Port").toInt());
	}
	else
	{
		// Just set the default saved password to true
		mGui->mSavePassword->setChecked(false);
		slotSetDefaultServer();
	}
}

AIMEditAccountWidget::~AIMEditAccountWidget()
{
}

KopeteAccount *AIMEditAccountWidget::apply()
{
	kdDebug(14190) << k_funcinfo << "Called." << endl;

	// If this is a new account, create it
	if (!mAccount)
	{
		kdDebug(14190) << k_funcinfo << "creating a new account" << endl;
		QString newId = mGui->edtAccountId->text();
		mAccount = new AIMAccount(mProtocol, newId);
	}

	// Check to see if we're saving the password, and set it if so
	if (mGui->mSavePassword->isChecked())
		mAccount->setPassword(mGui->edtPassword->text());
	else
		mAccount->setPassword(QString::null);

	mAccount->setAutoLogin(mGui->mAutoLogon->isChecked()); // save the autologon choice
	static_cast<OscarAccount *>(mAccount)->setServerAddress(mGui->edtServerAddress->text());
	static_cast<OscarAccount *>(mAccount)->setServerPort(mGui->sbxServerPort->value());

	return mAccount;
}

bool AIMEditAccountWidget::validateData()
{
	//kdDebug(14190) << k_funcinfo << "Called." << endl;

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
	//kdDebug(14190) << k_funcinfo << "Account data validated successfully." << endl;
	return true;
}

void AIMEditAccountWidget::slotSetDefaultServer()
{
	mGui->edtServerAddress->setText(AIM_SERVER);
	mGui->sbxServerPort->setValue(AIM_PORT);
}

#include "aimeditaccountwidget.moc"
// vim: set noet ts=4 sts=4 sw=4:
