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
#include "icqeditaccountui.h"

#include <qlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qspinbox.h>
#include <qpushbutton.h>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kjanuswidget.h>
#include <kurllabel.h>
#include <kdatewidget.h>

#include "icquserinfowidget.h"
#include "icqprotocol.h"
#include "icqaccount.h"
#include "icqcontact.h"

ICQEditAccountWidget::ICQEditAccountWidget(ICQProtocol *protocol,
	KopeteAccount *account, QWidget *parent, const char *name)
	: QWidget(parent, name), KopeteEditAccountWidget(account)
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;

	mAccount=account;
	mProtocol=protocol;
	mModified=false;

	(new QVBoxLayout(this))->setAutoAdd(true);
	mTop = new KJanusWidget(this, "ICQEditAccountWidget::mTop",
		KJanusWidget::IconList);

	// ==========================================================================

	QFrame *acc = mTop->addPage(i18n("Account"),
		i18n("ICQ Account Settings used for connecting to the ICQ Server"),
		KGlobal::iconLoader()->loadIcon(QString::fromLatin1("connect_no"), KIcon::Desktop));
	QVBoxLayout *accLay = new QVBoxLayout(acc);
	mAccountSettings = new ICQEditAccountUI(acc,
		"ICQEditAccountWidget::mAccountSettings");
	accLay->addWidget(mAccountSettings);

	QFrame *det = mTop->addPage(i18n("Contact Details"),
		i18n("ICQ Contact Details shown to other users"),
		KGlobal::iconLoader()->loadIcon(QString::fromLatin1("identity"), KIcon::Desktop));
	QVBoxLayout *detLay = new QVBoxLayout(det);
	mUserInfoSettings = new ICQUserInfoWidget(det,
		"ICQEditAccountWidget::mUserInfoSettings");
	detLay->addWidget(mUserInfoSettings);

	// ==========================================================================

	mProtocol->initUserinfoWidget(mUserInfoSettings); // fill combos with values

	mUserInfoSettings->rwAge->setValue(0);
	mUserInfoSettings->rwBday->setDate(QDate());

	mUserInfoSettings->rwAlias->hide();
	mUserInfoSettings->lblAlias->hide();

	mUserInfoSettings->cmbEncoding->hide();
	mUserInfoSettings->lblEncoding->hide();

	mUserInfoSettings->roSignonTime->hide();
	mUserInfoSettings->lblSignonTime->hide();
	mUserInfoSettings->roCreationTime->hide();
	mUserInfoSettings->lblCreationTime->hide();

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

	connect(mAccountSettings->btnServerDefaults, SIGNAL(clicked()),
	this, SLOT(slotSetDefaultServer()));

	// ==========================================================================

	// Read in the settings from the account if it exists
	if(mAccount)
	{
		mAccountSettings->chkSavePassword->setChecked(
			mAccount->rememberPassword());

		if(mAccountSettings->chkSavePassword->isChecked())
			mAccountSettings->edtPassword->setText(mAccount->password(false, 0L, 16));

		mAccountSettings->edtAccountId->setText(mAccount->accountId());

		// TODO: Remove me after we can change Account IDs (Matt)
		mAccountSettings->edtAccountId->setDisabled(true);

		mAccountSettings->chkAutoLogin->setChecked(mAccount->autoLogin());
		mAccountSettings->edtServerAddress->setText(mAccount->pluginData(mProtocol, "Server"));
		mAccountSettings->edtServerPort->setValue(mAccount->pluginData(mProtocol, "Port").toInt());
		mAccountSettings->chkHideIP->setChecked((mAccount->pluginData(mProtocol,"HideIP").toUInt()==1));
		mAccountSettings->chkWebAware->setChecked((mAccount->pluginData(mProtocol,"WebAware").toUInt()==1));
		mAccountSettings->chkRequireAuth->setChecked((mAccount->pluginData(mProtocol,"RequireAuth").toUInt()==1));

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

		// Private TAB ==============================================================
		mUserInfoSettings->prsCityEdit->setText(mAccount->pluginData(mProtocol, "PrivateCity"));
		mUserInfoSettings->prsStateEdit->setText(mAccount->pluginData(mProtocol, "PrivateState"));
		mUserInfoSettings->prsPhoneEdit->setText(mAccount->pluginData(mProtocol, "PrivatePhone"));
		mUserInfoSettings->prsCellphoneEdit->setText(mAccount->pluginData(mProtocol, "PrivateCellular"));
		mUserInfoSettings->prsFaxEdit->setText(mAccount->pluginData(mProtocol, "PrivateFax"));
		mUserInfoSettings->prsZipcodeEdit->setText(mAccount->pluginData(mProtocol, "PrivateZip"));
		mProtocol->setComboFromTable(mUserInfoSettings->rwPrsCountry, mProtocol->countries(),
			mAccount->pluginData(mProtocol, "PrivateCountry").toInt());

		mUserInfoSettings->prsAddressEdit->setText(mAccount->pluginData(mProtocol, "PrivateAddress"));
		mUserInfoSettings->prsHomepageEdit->setText(mAccount->pluginData(mProtocol, "PrivateHomepage"));
		mUserInfoSettings->prsEmailEdit->setText(mAccount->pluginData(mProtocol, "PrivateEmail"));
		// ==========================================================================

		// Work TAB =================================================================
		mUserInfoSettings->wrkNameEdit->setText(mAccount->pluginData(mProtocol, "WorkName"));
		mUserInfoSettings->wrkDepartmentEdit->setText(mAccount->pluginData(mProtocol, "WorkDepartment"));
		mUserInfoSettings->wrkPhoneEdit->setText(mAccount->pluginData(mProtocol, "WorkPhone"));
		mUserInfoSettings->wrkCityEdit->setText(mAccount->pluginData(mProtocol, "WorkCity"));
		mUserInfoSettings->wrkZipcodeEdit->setText(mAccount->pluginData(mProtocol, "WorkZip"));
		mUserInfoSettings->wrkPositionEdit->setText(mAccount->pluginData(mProtocol, "WorkPosition"));
		mUserInfoSettings->wrkFaxEdit->setText(mAccount->pluginData(mProtocol, "WorkFax"));
		mUserInfoSettings->wrkStateEdit->setText(mAccount->pluginData(mProtocol, "WorkState"));
		mProtocol->setComboFromTable(mUserInfoSettings->rwWrkCountry, mProtocol->countries(),
			mAccount->pluginData(mProtocol, "WorkCountry").toInt());
		mUserInfoSettings->wrkAddressEdit->setText(mAccount->pluginData(mProtocol, "WorkAddress"));
		mUserInfoSettings->wrkHomepageEdit->setText(mAccount->pluginData(mProtocol, "WorkHomePage"));
		// =========================================================================

		QHBoxLayout *buttonLayout = new QHBoxLayout(detLay, KDialog::spacingHint(), "buttonLayout");
		buttonLayout->addStretch(1);
		QPushButton *fetch = new QPushButton(i18n("Fetch From Server"), det, "fetch");
		buttonLayout->addWidget(fetch);
		QPushButton *send = new QPushButton(i18n("Send to Server"), det, "send");
		buttonLayout->addWidget(send);

		fetch->setDisabled(!mAccount->isConnected());
		send->setDisabled(!mAccount->isConnected());

		connect(fetch, SIGNAL(clicked()), this, SLOT(slotFetchInfo()));
		connect(send, SIGNAL(clicked()), this, SLOT(slotSend()));
		connect(mAccount->myself(), SIGNAL(updatedUserInfo()), this, SLOT(slotReadInfo()));
	}
	else
	{
		// Just set the default saved password to true
		kdDebug(14200) << k_funcinfo <<
			"Called with no account object, setting defaults for server and port" << endl;
		mAccountSettings->chkSavePassword->setChecked(true);

		QTime current = QTime::currentTime(Qt::LocalTime);
		QTime currentUTC = QTime::currentTime(Qt::UTC);
		int diff = current.hour() - currentUTC.hour();
		kdDebug(14200) << k_funcinfo << "diff from UTC=" << diff << endl;

		mProtocol->setTZComboValue(mUserInfoSettings->rwTimezone, (diff*2));
		slotSetDefaultServer();
	}

	connect(mAccountSettings->chkWebAware, SIGNAL(toggled(bool)), this, SLOT(slotModified()));
	connect(mAccountSettings->chkRequireAuth, SIGNAL(toggled(bool)), this, SLOT(slotModified()));
	connect(mUserInfoSettings->prsCityEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotModified()));
	connect(mUserInfoSettings->prsStateEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotModified()));
	connect(mUserInfoSettings->prsPhoneEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotModified()));
	connect(mUserInfoSettings->prsCellphoneEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotModified()));
	connect(mUserInfoSettings->prsFaxEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotModified()));
	connect(mUserInfoSettings->prsZipcodeEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotModified()));
	connect(mUserInfoSettings->rwPrsCountry, SIGNAL(activated(int)), this, SLOT(slotModified()));
	connect(mUserInfoSettings->prsAddressEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotModified()));
	connect(mUserInfoSettings->prsHomepageEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotModified()));
	connect(mUserInfoSettings->prsEmailEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotModified()));
	connect(mUserInfoSettings->rwBday, SIGNAL(changed(QDate)), this, SLOT(slotRecalcAge(QDate)));
	connect(mUserInfoSettings->rwBday, SIGNAL(changed(QDate)), this, SLOT(slotModified()));

	connect(mUserInfoSettings->intrCategoryCombo1, SIGNAL(activated(int)), this, SLOT(slotCategory1Changed(int)));
	connect(mUserInfoSettings->intrCategoryCombo2, SIGNAL(activated(int)), this, SLOT(slotCategory2Changed(int)));
	connect(mUserInfoSettings->intrCategoryCombo3, SIGNAL(activated(int)), this, SLOT(slotCategory3Changed(int)));
	connect(mUserInfoSettings->intrCategoryCombo4, SIGNAL(activated(int)), this, SLOT(slotCategory4Changed(int)));
	connect(mUserInfoSettings->bgrdCurrOrgCombo1, SIGNAL(activated(int)), this, SLOT(slotOrganisation1Changed(int)));
	connect(mUserInfoSettings->bgrdCurrOrgCombo2, SIGNAL(activated(int)), this, SLOT(slotOrganisation2Changed(int)));
	connect(mUserInfoSettings->bgrdCurrOrgCombo3, SIGNAL(activated(int)), this, SLOT(slotOrganisation3Changed(int)));
	connect(mUserInfoSettings->bgrdPastOrgCombo1, SIGNAL(activated(int)), this, SLOT(slotAffiliation1Changed(int)));
	connect(mUserInfoSettings->bgrdPastOrgCombo2, SIGNAL(activated(int)), this, SLOT(slotAffiliation2Changed(int)));
	connect(mUserInfoSettings->bgrdPastOrgCombo3, SIGNAL(activated(int)), this, SLOT(slotAffiliation3Changed(int)));
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
	mAccount->setPluginData(mProtocol, "RequireAuth",
		QString::number(mAccountSettings->chkRequireAuth->isChecked()));

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

	// Private TAB
	mAccount->setPluginData(mProtocol, "PrivateCity",
		mUserInfoSettings->prsCityEdit->text());
	mAccount->setPluginData(mProtocol, "PrivateState",
		mUserInfoSettings->prsStateEdit->text());
	mAccount->setPluginData(mProtocol, "PrivatePhone",
		mUserInfoSettings->prsPhoneEdit->text());
	mAccount->setPluginData(mProtocol, "PrivateCellular",
		mUserInfoSettings->prsCellphoneEdit->text());
	mAccount->setPluginData(mProtocol, "PrivateFax",
		mUserInfoSettings->prsFaxEdit->text());
	mAccount->setPluginData(mProtocol, "PrivateZip",
		mUserInfoSettings->prsZipcodeEdit->text());
	mAccount->setPluginData(mProtocol, "PrivateCountry", QString::number(
		mProtocol->getCodeForCombo(mUserInfoSettings->rwPrsCountry, mProtocol->countries())));
	mAccount->setPluginData(mProtocol, "PrivateAddress",
		mUserInfoSettings->prsAddressEdit->text());
	mAccount->setPluginData(mProtocol, "PrivateHomepage",
		mUserInfoSettings->prsHomepageEdit->text());
	mAccount->setPluginData(mProtocol, "PrivateEmail",
		mUserInfoSettings->prsEmailEdit->text());
	// =======================================================

	// Work Tab
	mAccount->setPluginData(mProtocol, "WorkName", mUserInfoSettings->wrkNameEdit->text());
	mAccount->setPluginData(mProtocol, "WorkDepartment", mUserInfoSettings->wrkDepartmentEdit->text());
	mAccount->setPluginData(mProtocol, "WorkPhone", mUserInfoSettings->wrkPhoneEdit->text());
	mAccount->setPluginData(mProtocol, "WorkCity", mUserInfoSettings->wrkCityEdit->text());
	mAccount->setPluginData(mProtocol, "WorkZip", mUserInfoSettings->wrkZipcodeEdit->text());
	mAccount->setPluginData(mProtocol, "WorkPosition", mUserInfoSettings->wrkPositionEdit->text());
	mAccount->setPluginData(mProtocol, "WorkFax", mUserInfoSettings->wrkFaxEdit->text());
	mAccount->setPluginData(mProtocol, "WorkState", mUserInfoSettings->wrkStateEdit->text());
	mAccount->setPluginData(mProtocol, "WorkCountry", QString::number(
		mProtocol->getCodeForCombo(mUserInfoSettings->rwWrkCountry, mProtocol->countries())));
	mAccount->setPluginData(mProtocol, "WorkAddress", mUserInfoSettings->wrkAddressEdit->text());
	mAccount->setPluginData(mProtocol, "WorkHomePage", mUserInfoSettings->wrkHomepageEdit->text());
	// =======================================================

	static_cast<ICQContact *>(mAccount->myself())->setOwnDisplayName(
		mUserInfoSettings->rwNickName->text());

	static_cast<ICQAccount *>(mAccount)->reloadPluginData();

	if(mModified)
		slotSend();

	return mAccount;
}

bool ICQEditAccountWidget::validateData()
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;

	QString userName = mAccountSettings->edtAccountId->text();

	if (userName.contains(" ") || (userName.length() < 4))
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
	mModified=false;
} // END slotReadInfo()

void ICQEditAccountWidget::slotSetDefaultServer()
{
	mAccountSettings->edtServerAddress->setText(ICQ_SERVER);
	mAccountSettings->edtServerPort->setValue(ICQ_PORT);
}

void ICQEditAccountWidget::slotSend()
{
	if(!mAccount->isConnected())
		return;

	kdDebug(14200) << k_funcinfo << "Called." << endl;

	ICQGeneralUserInfo generalInfo;
	ICQWorkUserInfo workInfo;
	ICQMoreUserInfo moreInfo;

	generalInfo.uin=static_cast<ICQContact *>(
		mAccount->myself())->contactName().toULong();
	generalInfo.nickName = mUserInfoSettings->rwNickName->text();
	generalInfo.firstName = mUserInfoSettings->rwFirstName->text();
	generalInfo.lastName = mUserInfoSettings->rwLastName->text();
	generalInfo.eMail = mUserInfoSettings->prsEmailEdit->text();
	generalInfo.city = mUserInfoSettings->prsCityEdit->text();
	generalInfo.state = mUserInfoSettings->prsStateEdit->text();
	generalInfo.phoneNumber = mUserInfoSettings->prsPhoneEdit->text();
	generalInfo.faxNumber = mUserInfoSettings->prsFaxEdit->text();
	generalInfo.street = mUserInfoSettings->prsAddressEdit->text();
	generalInfo.cellularNumber = mUserInfoSettings->prsCellphoneEdit->text();
	generalInfo.zip = mUserInfoSettings->prsZipcodeEdit->text();
	generalInfo.countryCode = mProtocol->getCodeForCombo(
		mUserInfoSettings->rwPrsCountry, mProtocol->countries());
	generalInfo.timezoneCode = mProtocol->getTZComboValue(
		mUserInfoSettings->rwTimezone);
	generalInfo.publishEmail = false; // TODO
	generalInfo.showOnWeb = false; // TODO

	QDate bday = mUserInfoSettings->rwBday->date();
	if(bday.isValid())
		moreInfo.birthday = bday;
	moreInfo.age = mUserInfoSettings->rwAge->value();
	moreInfo.gender = mProtocol->getCodeForCombo(mUserInfoSettings->rwGender,
		mProtocol->genders());
	moreInfo.lang1 = mProtocol->getCodeForCombo(mUserInfoSettings->rwLang1,
		mProtocol->languages());
	moreInfo.lang2 = mProtocol->getCodeForCombo(mUserInfoSettings->rwLang2,
		mProtocol->languages());
	moreInfo.lang3 = mProtocol->getCodeForCombo(mUserInfoSettings->rwLang3,
		mProtocol->languages());
	moreInfo.homepage = mUserInfoSettings->prsHomepageEdit->text();

	//Work Info
	workInfo.company = mUserInfoSettings->wrkNameEdit->text();
	workInfo.department = mUserInfoSettings->wrkDepartmentEdit->text();
	workInfo.phone = mUserInfoSettings->wrkPhoneEdit->text();
	workInfo.city = mUserInfoSettings->wrkCityEdit->text();
	workInfo.zip = mUserInfoSettings->wrkZipcodeEdit->text();
	workInfo.position = mUserInfoSettings->wrkPositionEdit->text();
	workInfo.fax = mUserInfoSettings->wrkFaxEdit->text();
	workInfo.state = mUserInfoSettings->wrkStateEdit->text();
	workInfo.countryCode = mProtocol->getCodeForCombo(mUserInfoSettings->rwWrkCountry, mProtocol->countries());
	workInfo.address = mUserInfoSettings->wrkAddressEdit->text();
	workInfo.homepage = mUserInfoSettings->wrkHomepageEdit->text();
	workInfo.occupation = 0; // TODO: work occupation missing in UI

	OscarSocket *osocket = static_cast<ICQAccount *>(mAccount)->engine();
	if(osocket)
	{
		osocket->sendCLI_METASETGENERAL(generalInfo);
		osocket->sendCLI_METASETMORE(moreInfo);
		osocket->sendCLI_METASETWORK(workInfo);
		osocket->sendCLI_METASETSECURITY(
			mAccountSettings->chkRequireAuth->isChecked(),
			mAccountSettings->chkWebAware->isChecked(),
			0x01);
	}
	else
	{
		kdDebug(14200) << k_funcinfo <<
			"Failed to fetch engine pointer, cannot send userinfo" << endl;
	}

	mModified=false;
}

void ICQEditAccountWidget::slotModified()
{
	mModified=true;
}

void ICQEditAccountWidget::slotRecalcAge(QDate bday)
{
	QDate now = QDate::currentDate();
	kdDebug(14200) << k_funcinfo << "current year=" << now.year() << " bday year=" << bday.year() << endl;

	if(bday.year() < now.year())
	{
		int age = now.year() - bday.year();

		if(now.month() < bday.month()) // didn't have his birthday this year
		{
			kdDebug(14200) << k_funcinfo << "didn't have his birthday this year" << endl;
			age--;
		}
		else if(now.month() == bday.month()) // his birthday is in the current month
		{
			kdDebug(14200) << k_funcinfo << "birthday is in the current month" << endl;

			if(now.day() < bday.day()) // didn't have his birthday this month yet
				age--;
		}
		kdDebug(14200) << k_funcinfo << "age calculated from birthday is " << age << endl;
		mUserInfoSettings->rwAge->setValue(age);
	}
}

void ICQEditAccountWidget::slotCategory1Changed(int i)
{
	mUserInfoSettings->intrDescText1->setEnabled(i != 0);
	slotModified();
}

void ICQEditAccountWidget::slotCategory2Changed(int i)
{
	mUserInfoSettings->intrDescText2->setEnabled(i != 0);
	slotModified();
}

void ICQEditAccountWidget::slotCategory3Changed(int i)
{
	mUserInfoSettings->intrDescText3->setEnabled(i != 0);
	slotModified();
}

void ICQEditAccountWidget::slotCategory4Changed(int i)
{
	mUserInfoSettings->intrDescText4->setEnabled(i != 0);
	slotModified();
}

void ICQEditAccountWidget::slotOrganisation1Changed(int i)
{
	mUserInfoSettings->bgrdCurrOrgText1->setEnabled(i != 0);
	slotModified();
}

void ICQEditAccountWidget::slotOrganisation2Changed(int i)
{
	mUserInfoSettings->bgrdCurrOrgText2->setEnabled(i != 0);
	slotModified();
}

void ICQEditAccountWidget::slotOrganisation3Changed(int i)
{
	mUserInfoSettings->bgrdCurrOrgText3->setEnabled(i != 0);
	slotModified();
}

void ICQEditAccountWidget::slotAffiliation1Changed(int i)
{
	mUserInfoSettings->bgrdPastOrgText1->setEnabled(i != 0);
	slotModified();
}

void ICQEditAccountWidget::slotAffiliation2Changed(int i)
{
	mUserInfoSettings->bgrdPastOrgText2->setEnabled(i != 0);
	slotModified();
}

void ICQEditAccountWidget::slotAffiliation3Changed(int i)
{
	mUserInfoSettings->bgrdPastOrgText3->setEnabled(i != 0);
	slotModified();
}

#include "icqeditaccountwidget.moc"
// vim: set noet ts=4 sts=4 sw=4:
