/*
    icquserinfo.cpp  -  ICQ Protocol Plugin

    Copyright (c) 2002 by Nick Betcher <nbetcher@kde.org>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

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

#include <qcombobox.h>
#include <qhostaddress.h>
#include <qspinbox.h>
#include <qtextedit.h>

#include <kapplication.h>
#include <kdatewidget.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurllabel.h>

//#include "client.h"
//#include "enable.h"
#include "icqaccount.h"
#include "icqcontact.h"
#include "oscarsocket.h"
#include "icquserinfowidget.h"

ICQUserInfo::ICQUserInfo(ICQContact *c, ICQAccount *account, bool editable,
	QWidget *parent, const char* name)
	: KDialogBase(parent, name, false, QString::null, Close | User1 | User2,
		Close, false, i18n("&Save Nickname"), i18n("&Fetch Again"))
{
	mAccount = account;
	mContact = c;
	mEditable = editable;

	setCaption(i18n("User Info for %1").arg(c->displayName()));

	mMainWidget = new ICQUserInfoWidget(this, "mMainWidget");
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

	setCaption( i18n("User Info for %1").arg(mContact->displayName()));

	mMainWidget->setDisabled( false );
	enableButton(User1,true);
	enableButton(User2,true);

	QString homepage;
/*
//	if ( !mEditable ) // no idea how to get ip for ourselves
	{
		QHostAddress mIP;
		QHostAddress mRealIP;

		mIP = ntohl ( static_cast<unsigned long>(mUser->IP) );
		mRealIP = ntohl ( static_cast<unsigned long>(mUser->RealIP) );
		unsigned short mPort = mUser->Port;

		if ( !(mIP == mRealIP) && !(mRealIP == QHostAddress()) )
			mMainWidget->roIPAddress->setText( QString("%1 (%2:%3)").arg(mIP.toString()).arg(mRealIP.toString()).arg(mPort) );
		else
			mMainWidget->roIPAddress->setText( QString("%1:%2").arg(mIP.toString()).arg(mPort) );
	}
*/
	mMainWidget->rwNickName->setText(mContact->generalInfo.nickName);
	mMainWidget->rwAlias->setText(mContact->displayName());
	mMainWidget->rwFirstName->setText(mContact->generalInfo.firstName);
	mMainWidget->rwLastName->setText(mContact->generalInfo.lastName);

	QString email = mContact->generalInfo.eMail;
	if (mEditable)
		mMainWidget->prsEmailEdit->setText(email);
	else
	{
		if (email.isEmpty()) // either NULL or ""
		{
			mMainWidget->prsEmailLabel->setText(i18n("unspecified"));
			mMainWidget->prsEmailLabel->setURL(QString::null);
			mMainWidget->prsEmailLabel->setDisabled( true );
			mMainWidget->prsEmailLabel->setUseCursor( false ); // disable hand cursor on mouseover
		}
		else
		{
			mMainWidget->prsEmailLabel->setText(email);
			mMainWidget->prsEmailLabel->setURL(email);
			mMainWidget->prsEmailLabel->setDisabled(false);
			mMainWidget->prsEmailLabel->setUseCursor(true); // enable hand cursor on mouseover
		}
	}

	// PRIVATE COUNTRY ==============================
/*
	initCombo(mMainWidget->rwPrsCountry, mUser->Country, countries);
	if ( !mEditable )
		mMainWidget->roPrsCountry->setText( mMainWidget->rwPrsCountry->currentText() );
*/
	mMainWidget->prsStateEdit->setText(mContact->generalInfo.state);
	mMainWidget->prsCityEdit->setText(mContact->generalInfo.city);
	mMainWidget->prsZipcodeEdit->setText(mContact->generalInfo.zip);
	mMainWidget->prsAddressEdit->setText(mContact->generalInfo.street);

	mMainWidget->prsPhoneEdit->setText(mContact->generalInfo.phoneNumber);
	mMainWidget->prsCellphoneEdit->setText(mContact->generalInfo.cellularNumber);
	mMainWidget->prsFaxEdit->setText(mContact->generalInfo.faxNumber);
/*
	// TIMEZONE ======================================
	initTZCombo (mMainWidget->rwTimezone, mUser->TimeZone );
	if ( !mEditable )
		mMainWidget->roTimezone->setText( mMainWidget->rwTimezone->currentText() );

	// AGE ===========================================
	if ( !mEditable ) // fixed value for readonly
	{
		mMainWidget->rwAge->setMinValue( mUser->Age );
		mMainWidget->rwAge->setMaxValue( mUser->Age );
		mMainWidget->rwAge->setValue( mUser->Age );
	}
	else
		mMainWidget->rwAge->setValue( mUser->Age );

	// GENDER ========================================
//	kdDebug(14200) << "[ICQUserInfo] Gender of contact is " << mUser->Gender << endl;
	initCombo( mMainWidget->rwGender, mUser->Gender, genders);
	if ( !mEditable ) // get text from hidden combobox and insert into readonly lineedit
		mMainWidget->roGender->setText( mMainWidget->rwGender->currentText() );

	// BIRTHDAY ========================================

	if ( mUser->BirthYear == 0 ) // no birthday defined
	{
		if ( mEditable )
			mMainWidget->rwBday->setDate(QDate());
		else
			mMainWidget->roBday->setText("");
	}
	else
	{
		QDate bday (mUser->BirthYear,mUser->BirthMonth,mUser->BirthDay);
		if ( bday.isValid() )
		{
			if ( mEditable )
				mMainWidget->rwBday->setDate(bday);
			else
				mMainWidget->roBday->setText(KGlobal::locale()->formatDate(bday,true));
		}
	}


	// Personal HOMEPAGE ========================================
	homepage = QString::fromLocal8Bit(mUser->Homepage.c_str());
	if ( mEditable )
	{
		mMainWidget->prsHomepageEdit->setText( homepage );
	} else {
		if ( homepage.isNull() )
		{
			mMainWidget->prsHomepageLabel->setText( i18n("unspecified") );
			mMainWidget->prsHomepageLabel->setURL( QString::null );
			mMainWidget->prsHomepageLabel->setDisabled( true );
			mMainWidget->prsHomepageLabel->setUseCursor( false ); // disable hand cursor on mouseover
		} else {
			QString tmpHP = homepage; // copy it, do not work on the original
			mMainWidget->prsHomepageLabel->setText( tmpHP );

			if ( !tmpHP.contains("://") ) // assume http-protocol if not protocol given
				tmpHP.prepend("http://");
			mMainWidget->prsHomepageLabel->setURL( tmpHP );

			mMainWidget->prsHomepageLabel->setDisabled( false );
			mMainWidget->prsHomepageLabel->setUseCursor( true ); // enable hand cursor on mouseover
		}
	}

	// LANGUAGES =========================================

	initCombo ( mMainWidget->rwLang1, mUser->Language1, languages );
	initCombo ( mMainWidget->rwLang2, mUser->Language2, languages );
	initCombo ( mMainWidget->rwLang3, mUser->Language3, languages );
	if ( !mEditable )
	{
		mMainWidget->roLang1->setText( mMainWidget->rwLang1->currentText() );
		mMainWidget->roLang2->setText( mMainWidget->rwLang2->currentText() );
		mMainWidget->roLang3->setText( mMainWidget->rwLang3->currentText() );
	}
*/

	// WORK INFO ========================================

	mMainWidget->wrkCityEdit->setText(mContact->workInfo.city);
	mMainWidget->wrkStateEdit->setText(mContact->workInfo.state);
	mMainWidget->wrkPhoneEdit->setText (mContact->workInfo.phone);
	mMainWidget->wrkFaxEdit->setText (mContact->workInfo.fax);
	mMainWidget->wrkAddressEdit->setText(mContact->workInfo.address);
	// TODO: mContact->workInfo.zip
	mMainWidget->wrkNameEdit->setText(mContact->workInfo.company);
	mMainWidget->wrkDepartmentEdit->setText(mContact->workInfo.department);
	mMainWidget->wrkPositionEdit->setText(mContact->workInfo.position);
	// TODO: mContact->workInfo.occupation

	// WORK HOMEPAGE =====================================

	homepage = mContact->workInfo.homepage;
	if ( mEditable )
	{
		mMainWidget->wrkHomepageEdit->setText(homepage);
	}
	else
	{
		if ( homepage.isEmpty() )
		{
			mMainWidget->wrkHomepageLabel->setText(i18n("unspecified"));
			mMainWidget->wrkHomepageLabel->setURL(QString::null);
			mMainWidget->wrkHomepageLabel->setDisabled(true);
			mMainWidget->wrkHomepageLabel->setUseCursor(false); // disable hand cursor on mouseover
		}
		else
		{
			QString tmpHP = homepage; // copy it, do not work on the original
			mMainWidget->wrkHomepageLabel->setText(tmpHP);

			if ( !tmpHP.contains("://") ) // assume http-protocol if not protocol given
				tmpHP.prepend("http://");
			mMainWidget->wrkHomepageLabel->setURL(tmpHP);

			mMainWidget->wrkHomepageLabel->setDisabled(false);
			mMainWidget->wrkHomepageLabel->setUseCursor(true); // enable hand cursor on mouseover
		}
	}

//mContact->workInfo.countryCode
/*
	initCombo (mMainWidget->rwWrkCountry, mUser->WorkCountry, countries );
	if ( !mEditable )
		mMainWidget->roWrkCountry->setText( mMainWidget->rwWrkCountry->currentText() );
*/
} // END slotReadInfo()

void ICQUserInfo::sendInfo()
{
	kdDebug(14200) << k_funcinfo << "called." << endl;
/*
	ICQUser *u = new ICQUser();

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
	u->Gender = getComboValue( mMainWidget->rwGender, genders );
	u->Homepage = mMainWidget->prsHomepageEdit->text().local8Bit();
	u->BirthYear = mMainWidget->rwBday->date().year();
	u->BirthMonth = mMainWidget->rwBday->date().month();
	u->BirthDay = mMainWidget->rwBday->date().day();
	u->Language1 = getComboValue( mMainWidget->rwLang1, languages );
	u->Language2 = getComboValue( mMainWidget->rwLang2, languages );
	u->Language3 = getComboValue( mMainWidget->rwLang3, languages );
//	int return = mAccount->engine()->sendAboutInfo(QString); <- string with blah blah about yourself

	mAccount->engine()->setInfo( u );
	delete u;
	kdDebug(14200) << "[ICQUserInfo] Done sending new userinfo to server" << endl;
*/
}

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

	if(e)
	{
		kdDebug(14200) << k_funcinfo << "editable mode" << endl;
		setButtonText(User1, i18n("&Send Info"));
		// updating userinfo impossible while being offline
		enableButton(User1, mAccount->isConnected() );
//		enableButton(User1, mAccount->myself()->onlineStatus().status() == KopeteOnlineStatus::Online );

		mMainWidget->rwAlias->hide();
		mMainWidget->txtAlias->hide();

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
		kdDebug(14200) << k_funcinfo << "readonly mode" << endl;
		mMainWidget->rwAlias->show();
		mMainWidget->txtAlias->show();

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
	if(mEditable)
		sendInfo();
//	else
//		emit updateNickname( mMainWidget->rwAlias->text() );
}

void ICQUserInfo::slotCloseClicked()
{
	kdDebug(14200) << k_funcinfo << "called." << endl;
	emit closing();
}

#include "icquserinfo.moc"
// vim: set noet ts=4 sts=4 sw=4:
