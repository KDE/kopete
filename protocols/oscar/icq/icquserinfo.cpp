/*
    icquserinfo.cpp  -  ICQ Protocol Plugin

    Copyright (c) 2002 by Nick Betcher <nbetcher@kde.org>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    language, country and other tables are taken from libicq
    file                 : country.cpp
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "icquserinfo.h"

#include "icqprotocol.h"
#include "icqaccount.h"
#include "icqcontact.h"
#include "icquserinfowidget.h"

#include <qcombobox.h>
#include <qspinbox.h>
#include <qtextedit.h>

#include <kapplication.h>
#include <kmessagebox.h>

#include <kdatewidget.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurllabel.h>


ICQUserInfo::ICQUserInfo(ICQContact *c, ICQAccount *account, bool editable,
	QWidget *parent, const char* name)
	: KDialogBase(parent, name, false, QString::null, Close | User1 | User2,
		Close, false, i18n("&Save Nickname"), i18n("&Fetch Again"))
{
	mAccount = account;
	mContact = c;
	mEditable = editable;
	p = ICQProtocol::protocol(); // I am SO lazy ;)

	setCaption(i18n("User Info for %1").arg(c->displayName()));

	mMainWidget = new ICQUserInfoWidget(this, "ICQUserInfo::mMainWidget");
	setEditable(mEditable);
	setMainWidget(mMainWidget);

	mMainWidget->roUIN->setText(c->contactname());

	// Defaults
	mMainWidget->rwAge->setValue(0);
	mMainWidget->rwBday->setDate(QDate());
	mMainWidget->roBday->setText("");

	connect(this, SIGNAL(user1Clicked()),
		this, SLOT(slotSaveClicked()));
	connect(this, SIGNAL(user2Clicked()),
		this, SLOT(slotFetchInfo()));
	connect(this, SIGNAL(closeClicked()),
		this, SLOT(slotCloseClicked()));
	connect(mMainWidget->prsHomepageLabel, SIGNAL(leftClickedURL(const QString &)),
		this, SLOT(slotHomePageClicked(const QString &)));
	connect(mMainWidget->prsEmailLabel, SIGNAL(leftClickedURL(const QString &)),
		this, SLOT(slotEmailClicked(const QString &)));
	connect(
		mMainWidget->wrkHomepageLabel, SIGNAL(leftClickedURL(const QString &)),
		this, SLOT(slotHomePageClicked(const QString &)));
	connect(
		c, SIGNAL(updatedUserInfo()),
		this, SLOT(slotReadInfo()));

	p->initUserinfoWidget(mMainWidget); // fill combos with values

	slotFetchInfo();
}

void ICQUserInfo::slotFetchInfo()
{
	if(mAccount->isConnected())
	{
		kdDebug(14200) << k_funcinfo << "fetching User Info for '" <<
			mContact->displayName() << "'." << endl;

		mMainWidget->setDisabled(true);
		enableButton(User1,false);
		enableButton(User2,false);

		mContact->requestUserInfo(); // initiate retrival of userinfo

		setCaption( i18n("Fetching User Info for %1...").arg(mContact->displayName()));
	}
	else
		kdDebug(14200) << k_funcinfo << "Ignore request to fetch User Info, NOT online!" << endl;
}

void ICQUserInfo::slotReadInfo()
{
	kdDebug(14200) << k_funcinfo << "called for user '" <<
		mContact->displayName() << "'." << endl;

	setCaption(i18n("User Info for %1").arg(mContact->displayName()));

	mMainWidget->setDisabled( false );
	enableButton(User1,true);
	enableButton(User2,true);

	p->contactInfo2UserInfoWidget(mContact, mMainWidget, false);
} // END slotReadInfo()

/*
void ICQUserInfo::sendInfo()
{
	kdDebug(14200) << k_funcinfo << "called." << endl;

	KMessageBox::sorry(
		qApp->mainWidget(),
		"<qt>Changing your own user information is not done yet.</qt>",
		"unfinished code");

	u->Nick = mMainWidget->rwNickName->text().local8Bit();
	u->FirstName = mMainWidget->rwFirstName->text().local8Bit();
	u->LastName = mMainWidget->rwLastName->text().local8Bit();
	u->City = mMainWidget->prsCityEdit->text().local8Bit();
	u->State = mMainWidget->prsStateEdit->text().local8Bit();
	u->Address = mMainWidget->prsAddressEdit->text().local8Bit();
	u->Zip = mMainWidget->prsZipcodeEdit->text().local8Bit();
	u->Country = getComboValue( mMainWidget->rwPrsCountry, countries );
	u->TimeZone = getTZComboValue( mMainWidget->rwTimezone );
	u->HomePhone = mMainWidget->prsPhoneEdit->text().local8Bit();
	u->HomeFax = mMainWidget->prsFaxEdit->text().local8Bit();
	u->PrivateCellular = mMainWidget->prsCellphoneEdit->text().local8Bit();
	u->EMail = mMainWidget->prsEmailLabel->text().local8Bit();

	u->WorkCity = mMainWidget->wrkCityEdit->text().local8Bit();
	u->WorkState = mMainWidget->wrkStateEdit->text().local8Bit();
	u->WorkZip = mMainWidget->wrkZipcodeEdit->text().local8Bit();
	u->WorkAddress = mMainWidget->wrkAddressEdit->text().local8Bit();
	u->WorkName = mMainWidget->wrkNameEdit->text().local8Bit();
	u->WorkDepartment = mMainWidget->wrkDepartmentEdit->text().local8Bit();
	u->WorkPosition = mMainWidget->wrkPositionEdit->text().local8Bit();
	u->WorkCountry = getComboValue( mMainWidget->rwWrkCountry, countries );
//	u->Occupation = .local8Bit();
	u->WorkHomepage = mMainWidget->wrkHomepageEdit->text().local8Bit();
	u->WorkPhone = mMainWidget->wrkPhoneEdit->text().local8Bit();
	u->WorkFax = mMainWidget->wrkFaxEdit->text().local8Bit();

	u->Age = mMainWidget->rwAge->text().toInt();
	u->Gender = getComboValue( mMainWidget->rwGender, p->genders() );
	u->Homepage = mMainWidget->prsHomepageEdit->text().local8Bit();
	u->BirthYear = mMainWidget->rwBday->date().year();
	u->BirthMonth = mMainWidget->rwBday->date().month();
	u->BirthDay = mMainWidget->rwBday->date().day();
	u->Language1 = getComboValue( mMainWidget->rwLang1, p->languages());
	u->Language2 = getComboValue( mMainWidget->rwLang2, p->languages());
	u->Language3 = getComboValue( mMainWidget->rwLang3, p->languages());
//	int return = mAccount->engine()->sendAboutInfo(QString); <- string with blah blah about yourself

	mAccount->engine()->setInfo( u );
	kdDebug(14200) << "[ICQUserInfo] Done sending new userinfo to server" << endl;
}
*/

void ICQUserInfo::setEditable(bool e)
{
	kdDebug(14200) << k_funcinfo << "called. e=" << e << endl;

	// this one is only editable for setting users own info
	// for contacts it displays their real nickname fetched from server
	mMainWidget->rwNickName->setReadOnly ( !e );

	// This one is editable in read-only
	// user can set arbitrary nicknames for contacts this way
	mMainWidget->rwAlias->setReadOnly ( e );

	mMainWidget->rwFirstName->setReadOnly ( !e );
	mMainWidget->rwLastName->setReadOnly ( !e );

	mMainWidget->prsCityEdit->setReadOnly ( !e );
	mMainWidget->prsPhoneEdit->setReadOnly ( !e );
	mMainWidget->prsStateEdit->setReadOnly ( !e );
	mMainWidget->prsCellphoneEdit->setReadOnly ( !e );
	mMainWidget->prsFaxEdit->setReadOnly ( !e );
	mMainWidget->prsZipcodeEdit->setReadOnly ( !e );
	mMainWidget->prsAddressEdit->setReadOnly ( !e );

	mMainWidget->wrkNameEdit->setReadOnly ( !e );
	mMainWidget->wrkDepartmentEdit->setReadOnly ( !e );
	mMainWidget->wrkPositionEdit->setReadOnly ( !e );
	mMainWidget->wrkPhoneEdit->setReadOnly ( !e );
	mMainWidget->wrkFaxEdit->setReadOnly ( !e );
	mMainWidget->wrkCityEdit->setReadOnly ( !e );
	mMainWidget->wrkStateEdit->setReadOnly ( !e );
	mMainWidget->wrkZipcodeEdit->setReadOnly ( !e );
	mMainWidget->wrkAddressEdit->setReadOnly ( !e );

	mMainWidget->rwAboutUser->setReadOnly ( !e );

	if(e)
	{
//		kdDebug(14200) << k_funcinfo << "editable mode" << endl;
		setButtonText(User1, i18n("&Send Info"));
		// updating userinfo impossible while being offline
		enableButton(User1, mAccount->isConnected() );

		mMainWidget->rwAlias->hide();
		mMainWidget->lblAlias->hide();

		mMainWidget->roBday->hide();
		mMainWidget->rwBday->show();

		mMainWidget->roGender->hide();
		mMainWidget->rwGender->show();

		mMainWidget->roTimezone->hide();
		mMainWidget->rwTimezone->show();

		mMainWidget->roLang1->hide();
		mMainWidget->rwLang1->show();
		mMainWidget->roLang2->hide();
		mMainWidget->rwLang2->show();
		mMainWidget->roLang3->hide();
		mMainWidget->rwLang3->show();

		mMainWidget->roPrsCountry->hide();
		mMainWidget->rwPrsCountry->show();
		mMainWidget->prsEmailLabel->hide();
		mMainWidget->prsEmailEdit->show();
		mMainWidget->prsHomepageLabel->hide();
		mMainWidget->prsHomepageEdit->show();

		mMainWidget->roWrkCountry->hide();
		mMainWidget->rwWrkCountry->show();
		mMainWidget->wrkHomepageLabel->hide();
		mMainWidget->wrkHomepageEdit->show();
	}
	else
	{
//		kdDebug(14200) << k_funcinfo << "readonly mode" << endl;
		mMainWidget->rwAlias->show();
		mMainWidget->lblAlias->show();

		mMainWidget->rwBday->hide();
		mMainWidget->roBday->show();

		mMainWidget->rwGender->hide();
		mMainWidget->roGender->show();

		mMainWidget->rwTimezone->hide();
		mMainWidget->roTimezone->show();

		mMainWidget->rwLang1->hide();
		mMainWidget->roLang1->show();
		mMainWidget->rwLang2->hide();
		mMainWidget->roLang2->show();
		mMainWidget->rwLang3->hide();
		mMainWidget->roLang3->show();

		mMainWidget->rwPrsCountry->hide();
		mMainWidget->roPrsCountry->show();
		mMainWidget->prsEmailEdit->hide();
		mMainWidget->prsEmailLabel->show();
		mMainWidget->prsHomepageEdit->hide();
		mMainWidget->prsHomepageLabel->show();

		mMainWidget->rwWrkCountry->hide();
		mMainWidget->roWrkCountry->show();
		mMainWidget->wrkHomepageEdit->hide();
		mMainWidget->wrkHomepageLabel->show();
	}
}

void ICQUserInfo::slotEmailClicked(const QString &email)
{
	kapp->invokeMailer(email, QString::null);
}

void ICQUserInfo::slotHomePageClicked(const QString &url)
{
	kapp->invokeBrowser(url);
}

void ICQUserInfo::slotSaveClicked()
{
	kdDebug(14200) << k_funcinfo << "called." << endl;
//	if(mEditable)
//		sendInfo();
//	else

	if(mContact->displayName() != mMainWidget->rwAlias->text())
		mContact->rename(mMainWidget->rwAlias->text());
}

void ICQUserInfo::slotCloseClicked()
{
//	kdDebug(14200) << k_funcinfo << "called." << endl;
	emit closing();
}

#include "icquserinfo.moc"
// vim: set noet ts=4 sts=4 sw=4:
