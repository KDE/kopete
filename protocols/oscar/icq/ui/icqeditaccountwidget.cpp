#include "icqeditaccountwidget.h"
#include "icqeditaccountui.h"

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qpushbutton.h>

#include <kdebug.h>
#include <klocale.h>
#include <kjanuswidget.h>
#include <kurllabel.h>
#include <kdatewidget.h>

#include "icquserinfowidget.h"
#include "icqprotocol.h"
#include "icqaccount.h"
#include "icqcontact.h"

ICQEditAccountWidget::ICQEditAccountWidget(ICQProtocol *protocol,
	KopeteAccount *account, QWidget *parent, const char *name)
	: QWidget(parent, name), EditAccountWidget(account)
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;

	mAccount = account;
	mProtocol = protocol;

	(new QVBoxLayout(this))->setAutoAdd(true);
	mTop = new KJanusWidget(this, "ICQEditAccountWidget::mTop", KJanusWidget::IconList);

	// ----------------------------------------------------------------

	QFrame *acc = mTop->addPage(i18n("Account"),
		i18n("ICQ Account Settings used for connecting to the ICQ Server"));
	QVBoxLayout *accLay = new QVBoxLayout(acc);
	mAccountSettings = new OscarEditAccountUI(acc,
		"ICQEditAccountWidget::mAccountSettings");
	accLay->addWidget(mAccountSettings);

	// ----------------------------------------------------------------

	QFrame *det = mTop->addPage(i18n("Contact Details"),
		i18n("ICQ Contact Details shown to other users"));
	QVBoxLayout *detLay = new QVBoxLayout(det);
	mUserInfoSettings = new ICQUserInfoWidget(det,
		"ICQEditAccountWidget::mUserInfoSettings");
	detLay->addWidget(mUserInfoSettings);

	// ----------------------------------------------------------------

	mProtocol->initUserinfoWidget(mUserInfoSettings); // fill combos with values

	mUserInfoSettings->rwAge->setValue(0);
	mUserInfoSettings->rwBday->setDate(QDate());

	mUserInfoSettings->rwAlias->hide();
	mUserInfoSettings->lblAlias->hide();

	mUserInfoSettings->roSignonTime->hide();
	mUserInfoSettings->lblSignonTime->hide();

	mUserInfoSettings->roUIN->hide();
	mUserInfoSettings->lblICQUIN->hide();

	mUserInfoSettings->lblIP->hide();
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

	// ----------------------------------------------------------------

	// Read in the settings from the account if it exists
	if(mAccount)
	{
		if(mAccount->rememberPassword())
		{
			mAccountSettings->chkSavePassword->setChecked(true);
			mAccountSettings->edtPassword->setText(mAccount->getPassword());
		}

		mAccountSettings->edtAccountId->setText(mAccount->accountId());
		mAccountSettings->chkAutoLogin->setChecked(mAccount->autoLogin());
		mAccountSettings->edtServerAddress->setText(mAccount->pluginData(mProtocol, "Server"));
		mAccountSettings->edtServerPort->setValue(mAccount->pluginData(mProtocol, "Port").toInt());
		mAccountSettings->chkHideIP->setChecked((mAccount->pluginData(mProtocol,"HideIP").toUInt()==1));
		mAccountSettings->chkWebAware->setChecked((mAccount->pluginData(mProtocol,"WebAware").toUInt()==1));

		mUserInfoSettings->rwNickName->setText(
			mAccount->pluginData(mProtocol,"NickName"));

		mUserInfoSettings->rwFirstName->setText(
			mAccount->pluginData(mProtocol,"FirstName"));

		mUserInfoSettings->rwLastName->setText(
			mAccount->pluginData(mProtocol,"LastName"));

		mUserInfoSettings->rwBday->setDate(
			QDate::fromString(mAccount->pluginData(mProtocol,"Birthday"), Qt::ISODate));

		mUserInfoSettings->rwAge->setValue(
			mAccount->pluginData(mProtocol, "Age").toInt());

		mProtocol->setComboFromTable(mUserInfoSettings->rwGender, mProtocol->genders(),
			mAccount->pluginData(mProtocol, "Gender").toInt());

		mProtocol->setComboFromTable(mUserInfoSettings->rwLang1, mProtocol->languages(),
			mAccount->pluginData(mProtocol, "Lang1").toInt());

		mProtocol->setComboFromTable(mUserInfoSettings->rwLang2, mProtocol->languages(),
			mAccount->pluginData(mProtocol, "Lang2").toInt());

		mProtocol->setComboFromTable(mUserInfoSettings->rwLang3, mProtocol->languages(),
			mAccount->pluginData(mProtocol, "Lang3").toInt());

		QString tmpTz = mAccount->pluginData(mProtocol, "Timezone");
		if(tmpTz.isEmpty())
			mProtocol->setTZComboValue(mUserInfoSettings->rwTimezone, 24);
		else
			mProtocol->setTZComboValue(mUserInfoSettings->rwTimezone, tmpTz.toInt());

		QHBoxLayout *buttons = new QHBoxLayout(detLay);
		buttons->addStretch(1);
		QPushButton *fetch = new QPushButton(i18n("Fetch From Server"), det, "fetch");
		buttons->addWidget(fetch);
		QPushButton *send = new QPushButton(i18n("Send to Server"), det, "send");
		buttons->addWidget(send);

		fetch->setDisabled(!mAccount->isConnected());
		send->setDisabled(!mAccount->isConnected());

		connect(fetch, SIGNAL(clicked()), this, SLOT(slotFetchInfo()));
// 		connect(send, SIGNAL(clicked()), this, SLOT(slotSend()));
		connect(
			mAccount->myself(), SIGNAL(updatedUserInfo()),
			this, SLOT(slotReadInfo()));
	}
	else
	{
		// Just set the default saved password to true
		mAccountSettings->chkSavePassword->setChecked(true);
		mAccountSettings->edtServerAddress->setText(ICQ_SERVER);
		mAccountSettings->edtServerPort->setValue(ICQ_PORT);
	}
}

ICQEditAccountWidget::~ICQEditAccountWidget()
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;
}

KopeteAccount *ICQEditAccountWidget::apply()
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;

	// If this is a new account, create it
	if (!mAccount)
	{
		kdDebug(14200) << k_funcinfo << "Creating a new account" << endl;
		mAccount = new ICQAccount(mProtocol, mAccountSettings->edtAccountId->text());
		if(!mAccount)
			return NULL;
	}

	// Check to see if we're saving the password, and set it if so
	if (mAccountSettings->chkSavePassword->isChecked())
		mAccount->setPassword(mAccountSettings->edtPassword->text());
	else
		mAccount->setPassword(QString::null);

	mAccount->setAutoLogin(mAccountSettings->chkAutoLogin->isChecked());
	static_cast<OscarAccount *>(mAccount)->setServerAddress(
		mAccountSettings->edtServerAddress->text());
	static_cast<OscarAccount *>(mAccount)->setServerPort(
		mAccountSettings->edtServerPort->value());
	mAccount->setPluginData(mProtocol, "HideIP",
		QString::number(mAccountSettings->chkHideIP->isChecked()));
	mAccount->setPluginData(mProtocol, "WebAware",
		QString::number(mAccountSettings->chkWebAware->isChecked()));

	mAccount->setPluginData(mProtocol, "NickName",
		mUserInfoSettings->rwNickName->text());
	mAccount->setPluginData(mProtocol, "FirstName",
		mUserInfoSettings->rwFirstName->text());
	mAccount->setPluginData(mProtocol, "LastName",
		mUserInfoSettings->rwLastName->text());
	mAccount->setPluginData(mProtocol, "Birthday",
		(mUserInfoSettings->rwBday->date()).toString(Qt::ISODate));
	mAccount->setPluginData(mProtocol, "Age",
		QString::number(mUserInfoSettings->rwAge->value()));
	mAccount->setPluginData(mProtocol, "Gender", QString::number(
		mProtocol->getCodeForCombo(mUserInfoSettings->rwGender, mProtocol->genders())));
	mAccount->setPluginData(mProtocol, "Lang1", QString::number(
		mProtocol->getCodeForCombo(mUserInfoSettings->rwLang1, mProtocol->languages())));
	mAccount->setPluginData(mProtocol, "Lang2", QString::number(
		mProtocol->getCodeForCombo(mUserInfoSettings->rwLang2, mProtocol->languages())));
	mAccount->setPluginData(mProtocol, "Lang3", QString::number(
		mProtocol->getCodeForCombo(mUserInfoSettings->rwLang3, mProtocol->languages())));

	mAccount->setPluginData(mProtocol, "Timezone", QString::number(
		mProtocol->getTZComboValue(mUserInfoSettings->rwTimezone)));

	static_cast<ICQContact *>(mAccount->myself())->setOwnDisplayName(
		mUserInfoSettings->rwNickName->text());

	static_cast<ICQAccount *>(mAccount)->reloadPluginData();

	// TODO: optionally send updated userinfo to server if connected
	return mAccount;
}

bool ICQEditAccountWidget::validateData()
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;

	QString userName = mAccountSettings->edtAccountId->text();
	QString server = mAccountSettings->edtServerAddress->text();
	int port = mAccountSettings->edtServerPort->value();

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
	kdDebug(14200) << k_funcinfo <<
		"Account data validated successfully." << endl;
	return true;
}

void ICQEditAccountWidget::slotFetchInfo()
{
	if(mAccount->isConnected())
	{
		kdDebug(14200) << k_funcinfo << "Fetching User Info for '" <<
			mAccount->myself()->displayName() << "'." << endl;

		mUserInfoSettings->setDisabled(true);

		static_cast<ICQContact *>(mAccount->myself())->requestUserInfo(); // initiate retrival of userinfo
	}
	else
		kdDebug(14200) << k_funcinfo <<
			"Ignore request to fetch User Info, NOT online!" << endl;
}

void ICQEditAccountWidget::slotReadInfo()
{
	kdDebug(14200) << k_funcinfo << "Called for user '" <<
		mAccount->myself()->displayName() << "'." << endl;

	mUserInfoSettings->setDisabled(false);

	mProtocol->contactInfo2UserInfoWidget(
		static_cast<ICQContact *>(mAccount->myself()), mUserInfoSettings, true);
} // END slotReadInfo()

#include "icqeditaccountwidget.moc"
// vim: set noet ts=4 sts=4 sw=4:
