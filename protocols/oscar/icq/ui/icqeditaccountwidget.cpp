#include "icqeditaccountwidget.h"
#include "icqeditaccountui.h"

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>

#include <kdebug.h>

#include "icqprotocol.h"
#include "icqaccount.h"

ICQEditAccountWidget::ICQEditAccountWidget(ICQProtocol *protocol,
	KopeteAccount *account, QWidget *parent, const char *name)
	: QWidget(parent, name), EditAccountWidget(account)
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;

	mAccount = account;
	mProtocol = protocol;

	// create the gui (generated from a .ui file)
	(new QVBoxLayout(this))->setAutoAdd(true);
	mGui = new OscarEditAccountUI(this, "ICQEditAccountWidget::mGui");

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
		mGui->mServer->setText(ICQ_SERVER);
		mGui->mPort->setValue(ICQ_PORT);
	}
}

ICQEditAccountWidget::~ICQEditAccountWidget()
{
}

KopeteAccount *ICQEditAccountWidget::apply()
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;

	// If this is a new account, create it
	if (!mAccount)
	{
		kdDebug(14200) << k_funcinfo << "creating a new account" << endl;
		QString newId = mGui->mAccountId->text();
		mAccount = new ICQAccount(mProtocol, newId);
	}

	// Check to see if we're saving the password, and set it if so
	if (mGui->mSavePassword->isChecked())
		mAccount->setPassword(mGui->mPassword->text());
	else
		mAccount->setPassword(QString::null);

	mAccount->setAutoLogin(mGui->mAutoLogon->isChecked()); // save the autologon choice
	static_cast<ICQAccount *>(mAccount)->setServer(mGui->mServer->text());
	static_cast<ICQAccount *>(mAccount)->setPort(mGui->mPort->value());

	return mAccount;
}

bool ICQEditAccountWidget::validateData()
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;

	QString userName = mGui->mAccountId->text();
	QString server = mGui->mServer->text();
	int port = mGui->mPort->value();

	if (userName.contains(" "))
		return false;

	if (userName.length() < 4)
		return false;

	for (unsigned int i=0; i<userName.length(); i++)
	{
		if(!(userName[i]).isNumber())
			return false;
	}

	if (port < 1)
		return false;

	if (server.length() < 1)
		return false;

	// Seems good to me
	kdDebug(14200) << k_funcinfo << "Account data validated successfully." << endl;
	return true;
}

#include "icqeditaccountwidget.moc"
// vim: set noet ts=4 sts=4 sw=4:
