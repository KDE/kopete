#include "icqeditaccountwidget.h"
#include "icqeditaccountui.h"

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
//#include <qtabwidget.h>

#include <kdebug.h>
#include <klocale.h>
#include <kjanuswidget.h>
#include <kurllabel.h>
#include "icquserinfowidget.h"

#include "icqprotocol.h"
#include "icqaccount.h"

ICQEditAccountWidget::ICQEditAccountWidget(ICQProtocol *protocol,
	KopeteAccount *account, QWidget *parent, const char *name)
	: QWidget(parent, name), EditAccountWidget(account)
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;

	mAccount = account;
	mProtocol = protocol;

	(new QVBoxLayout(this))->setAutoAdd(true);
//	mTop = new QTabWidget(this, "ICQEditAccountWidget::mTop");
	mTop = new KJanusWidget(this, "ICQEditAccountWidget::mTop", KJanusWidget::IconList);
	QFrame *acc = mTop->addPage(i18n("Account"), i18n("ICQ Account Settings"));
	QFrame *det = mTop->addPage(i18n("Contact"), i18n("ICQ Contact Details") );

	(new QVBoxLayout(acc))->setAutoAdd(true);
	mAccountSettings = new OscarEditAccountUI(acc,
		"ICQEditAccountWidget::mAccountSettings");
//	mTop->addTab( mAccountSettings, i18n("&Account Settings") );

	(new QVBoxLayout(det))->setAutoAdd(true);
	mUserInfoSettings = new ICQUserInfoWidget(det,
		"ICQEditAccountWidget::mUserInfoSettings");
//	mTop->addTab( mUserInfoSettings, i18n("&User Details") );

	// ----------------------------------------------------------------

	// Read in the settings from the account if it exists
	if(account)
	{
		if(account->rememberPassword())
		{
			mAccountSettings->mSavePassword->setChecked(true);
			mAccountSettings->mPassword->setText(account->getPassword());
		}

		mAccountSettings->mAccountId->setText(account->accountId());
		mAccountSettings->mAutoLogon->setChecked(account->autoLogin());
		mAccountSettings->mServer->setText(account->pluginData(protocol, "Server"));
		mAccountSettings->mPort->setValue(account->pluginData(protocol, "Port").toInt());

		mUserInfoSettings->roUIN->setText(account->accountId());
		mUserInfoSettings->rwNickName->setText(account->pluginData(protocol,"NickName"));
		mUserInfoSettings->rwFirstName->setText(account->pluginData(protocol,"FirstName"));
		mUserInfoSettings->rwLastName->setText(account->pluginData(protocol,"LastName"));
		// TODO: allow fetching userinfo from server
	}
	else
	{
		// Just set the default saved password to true
		mAccountSettings->mSavePassword->setChecked(true);
		// These come from OscarSocket where they are #defined
		mAccountSettings->mServer->setText(ICQ_SERVER);
		mAccountSettings->mPort->setValue(ICQ_PORT);
	}

	mUserInfoSettings->rwAlias->hide();
	mUserInfoSettings->roSignonTime->hide();
	mUserInfoSettings->roIPAddress->hide();
	mUserInfoSettings->roBday->hide();
	mUserInfoSettings->roGender->hide();
	mUserInfoSettings->roTimezone->hide();
	mUserInfoSettings->roLang1->hide();
	mUserInfoSettings->roLang2->hide();
	mUserInfoSettings->roLang3->hide();
	mUserInfoSettings->roPrsCountry->hide();
	mUserInfoSettings->prsEmailLabel->hide();
	mUserInfoSettings->prsHomepageLabel->hide();
	mUserInfoSettings->roWrkCountry->hide();
	mUserInfoSettings->wrkHomepageLabel->hide();
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
		QString newId = mAccountSettings->mAccountId->text();
		mAccount = new ICQAccount(mProtocol, newId);
		if(!mAccount)
			return NULL;
	}

	// Check to see if we're saving the password, and set it if so
	if (mAccountSettings->mSavePassword->isChecked())
		mAccount->setPassword(mAccountSettings->mPassword->text());
	else
		mAccount->setPassword(QString::null);

	mAccount->setAutoLogin(mAccountSettings->mAutoLogon->isChecked());
	static_cast<OscarAccount *>(mAccount)->setServer(mAccountSettings->mServer->text());
	static_cast<OscarAccount *>(mAccount)->setPort(mAccountSettings->mPort->value());

	mAccount->setPluginData(mProtocol, "NickName", mUserInfoSettings->rwNickName->text());
	mAccount->setPluginData(mProtocol, "FirstName", mUserInfoSettings->rwFirstName->text());
	mAccount->setPluginData(mProtocol, "LastName", mUserInfoSettings->rwLastName->text());

	// FIXME: bad place for doing this
	mAccount->myself()->rename(mUserInfoSettings->rwNickName->text());
	// TODO: send updated userinfo to server if connected

	return mAccount;
}

bool ICQEditAccountWidget::validateData()
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;

	QString userName = mAccountSettings->mAccountId->text();
	QString server = mAccountSettings->mServer->text();
	int port = mAccountSettings->mPort->value();

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
