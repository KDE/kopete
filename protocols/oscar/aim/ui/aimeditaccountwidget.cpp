#include "aimeditaccountwidget.h"
#include "aimeditaccountui.h"

#include <qlayout.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qradiobutton.h>

#include <kdebug.h>

#include "aimprotocol.h"
#include "aimaccount.h"
//#include "oscarsocket.h"

AIMEditAccountWidget::AIMEditAccountWidget(AIMProtocol *protocol,
	KopeteAccount *account, QWidget *parent, const char *name)
	: QWidget(parent, name), EditAccountWidget(account)
{
	kdDebug(14190) << k_funcinfo << "Called." << endl;

	mAccount = account;
	mProtocol = protocol;

	// create the gui (generated from a .ui file)
	(new QVBoxLayout(this))->setAutoAdd(true);
	mGui = new aimEditAccountUI(this, "AIMEditAccountWidget::mGui");

	// Read in the settings from the account if it exists
	if (account)
	{
		if (account->rememberPassword())
		{ // If we want to remember the password
			mGui->mSavePassword->setChecked(true);
			mGui->mPassword->setText(account->getPassword());
		}
		mGui->mAccountId->setText(account->accountId());
		mGui->mAutoLogon->setChecked(account->autoLogin());
		mGui->mServer->setText(account->pluginData(protocol, "Server"));
		mGui->mPort->setValue(account->pluginData(protocol,"Port").toInt());
	}
	else
	{
		// Just set the default saved password to true
		mGui->mSavePassword->setChecked(true);
		// These come from OscarSocket where they are #defined
		mGui->mServer->setText( OSCAR_SERVER );
		mGui->mPort->setValue( OSCAR_PORT );
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
		QString newId = mGui->mAccountId->text();
		mAccount = new AIMAccount(mProtocol, newId);
	}

	// Check to see if we're saving the password, and set it if so
	if (mGui->mSavePassword->isChecked())
		mAccount->setPassword(mGui->mPassword->text());
	else
		mAccount->setPassword(QString::null);

	mAccount->setAutoLogin(mGui->mAutoLogon->isChecked()); // save the autologon choice
	static_cast<AIMAccount *>(mAccount)->setServer(mGui->mServer->text());
	static_cast<AIMAccount *>(mAccount)->setPort(mGui->mPort->value());

	return mAccount;
}

bool AIMEditAccountWidget::validateData()
{
	kdDebug(14190) << k_funcinfo << "Called." << endl;

	QString userName = mGui->mAccountId->text();
	QString server = mGui->mServer->text();
	int port = mGui->mPort->value();

	if (userName.length() < 1)
		return false;

	if (port < 1)
		return false;

	if (server.length() < 1)
		return false;

	// Seems good to me
	kdDebug(14190) << k_funcinfo << "Account data validated successfully." << endl;
	return true;
}

#include "aimeditaccountwidget.moc"
// vim: set noet ts=4 sts=4 sw=4:
