/*
    icquserinfo.cpp  -  ICQ Protocol Plugin

    Copyright (c) 2002 by Nick Betcher <nbetcher@kde.org>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    language, country and other tables taken from libicq
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

#include <kdatewidget.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurllabel.h>


ICQUserInfo::ICQUserInfo(ICQContact *c, QWidget *parent, const char* name)
	: KDialogBase(parent, name, false, QString::null, Close | User1 | User2,
		Close, false, i18n("&Save Settings"), i18n("&Fetch Again"))
{
	mAccount = static_cast<ICQAccount*>(c->account());
	mContact = c;
	p = ICQProtocol::protocol(); // I am SO lazy ;)

	setCaption(i18n("User Info for %1").arg(c->displayName()));

	mMainWidget = new ICQUserInfoWidget(this, "ICQUserInfo::mMainWidget");
	setReadonly();
	setMainWidget(mMainWidget);


	// Defaults
	mMainWidget->rwAge->setValue(0);
	mMainWidget->rwBday->setDate(QDate());
	mMainWidget->roBday->setText("");
	mMainWidget->roUIN->setText(c->contactName());
	p->initUserinfoWidget(mMainWidget); // fill combos with values
	p->setComboFromTable(mMainWidget->cmbEncoding, p->encodings(), c->encoding());

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
	connect(mMainWidget->wrkHomepageLabel, SIGNAL(leftClickedURL(const QString &)),
		this, SLOT(slotHomePageClicked(const QString &)));
	connect(c, SIGNAL(updatedUserInfo()),
		this, SLOT(slotReadInfo()));

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

	mMainWidget->setDisabled(false);
	enableButton(User1,true);
	enableButton(User2,true);

	p->contactInfo2UserInfoWidget(mContact, mMainWidget, false);
} // END slotReadInfo()

void ICQUserInfo::setReadonly()
{
	mMainWidget->rwNickName->setReadOnly(true);

	mMainWidget->rwAlias->setReadOnly(false);

	mMainWidget->rwFirstName->setReadOnly(true);
	mMainWidget->rwLastName->setReadOnly(true);

	mMainWidget->prsCityEdit->setReadOnly(true);
	mMainWidget->prsPhoneEdit->setReadOnly(true);
	mMainWidget->prsStateEdit->setReadOnly(true);
	mMainWidget->prsCellphoneEdit->setReadOnly(true);
	mMainWidget->prsFaxEdit->setReadOnly(true);
	mMainWidget->prsZipcodeEdit->setReadOnly(true);
	mMainWidget->prsAddressEdit->setReadOnly(true);

	mMainWidget->wrkNameEdit->setReadOnly(true);
	mMainWidget->wrkDepartmentEdit->setReadOnly(true);
	mMainWidget->wrkPositionEdit->setReadOnly(true);
	mMainWidget->wrkPhoneEdit->setReadOnly(true);
	mMainWidget->wrkFaxEdit->setReadOnly(true);
	mMainWidget->wrkCityEdit->setReadOnly(true);
	mMainWidget->wrkStateEdit->setReadOnly(true);
	mMainWidget->wrkZipcodeEdit->setReadOnly(true);
	mMainWidget->wrkAddressEdit->setReadOnly(true);

	mMainWidget->rwAboutUser->setReadOnly(true);

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

	if(mContact->displayName() != mMainWidget->rwAlias->text())
		mContact->rename(mMainWidget->rwAlias->text());

	int enc = p->getCodeForCombo(mMainWidget->cmbEncoding, p->encodings());
	kdDebug(14200) << k_funcinfo <<
		"setting encoding to MIB:" << enc <<
		"(" << static_cast<const QString&>(p->encodings()[enc]) << ")" << endl;
	mContact->setEncoding(enc);
}

void ICQUserInfo::slotCloseClicked()
{
//	kdDebug(14200) << k_funcinfo << "called." << endl;
	emit closing();
}

#include "icquserinfo.moc"
// vim: set noet ts=4 sts=4 sw=4:
