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

#include <qcombobox.h>
#include <qhostaddress.h>
#include <qspinbox.h>
#include <qtextedit.h>

#include <kapplication.h>
#include <kmessagebox.h>
#include <kdatewidget.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurllabel.h>

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

	initGenders();
	initLang();
	initCountries();

	fillCombo(mMainWidget->rwGender, genders);
	fillCombo(mMainWidget->rwLang1, languages);
	fillCombo(mMainWidget->rwLang2, languages);
	fillCombo(mMainWidget->rwLang3, languages);

	slotFetchInfo();
}

void ICQUserInfo::fillCombo(QComboBox *box, QMap<int, QString> &map)
{
//	kdDebug(14200) << k_funcinfo << "Called." << endl;

	QStringList list = map.values();
	list.sort();
	box->insertStringList(list);
}

void ICQUserInfo::setCombo(QComboBox *box, QMap<int, QString> &map, int value)
{
//	kdDebug(14200) << k_funcinfo << "Called." << endl;
	QMap<int, QString>::Iterator it;
	it = map.find(value);
	if (*it)
	{
		for(int i=0; i<box->count(); i++)
		{
			if((*it) == box->text(i))
			{
				box->setCurrentItem(i);
				return;
			}
		}
	}
}

void ICQUserInfo::initGenders()
{
	genders.insert(0, i18n("Unspecified"));
	genders.insert(1, i18n("Female"));
	genders.insert(2, i18n("Male"));
}

void ICQUserInfo::initCountries()
{
/*
        {I18N_NOOP("Afghanistan"), 93 },
        {I18N_NOOP("Albania"), 355 },
        {I18N_NOOP("Algeria"), 213 },
        {I18N_NOOP("American Samoa"), 684 },
        {I18N_NOOP("Andorra"), 376 },
        {I18N_NOOP("Angola"), 244 },
        {I18N_NOOP("Anguilla"), 101 },
        {I18N_NOOP("Antigua"), 102 },
        {I18N_NOOP("Argentina"), 54 },
        {I18N_NOOP("Armenia"), 374 },
        {I18N_NOOP("Aruba"), 297 },
        {I18N_NOOP("Ascension Island"), 247 },
        {I18N_NOOP("Australia"), 61 },
        {I18N_NOOP("Australian Antarctic Territory"), 6721 },
        {I18N_NOOP("Austria"), 43 },
        {I18N_NOOP("Azerbaijan"), 994 },
        {I18N_NOOP("Bahamas"), 103 },
        {I18N_NOOP("Bahrain"), 973 },
        {I18N_NOOP("Bangladesh"), 880 },
        {I18N_NOOP("Barbados"), 104 },
        {I18N_NOOP("Barbuda"), 120 },
        {I18N_NOOP("Belarus"), 375 },
        {I18N_NOOP("Belgium"), 32 },
        {I18N_NOOP("Belize"), 501 },
        {I18N_NOOP("Benin"), 229 },
        {I18N_NOOP("Bermuda"), 105 },
        {I18N_NOOP("Bhutan"), 975 },
        {I18N_NOOP("Bolivia"), 591 },
        {I18N_NOOP("Bosnia and Herzegovina"), 387 },
        {I18N_NOOP("Botswana"), 267 },
        {I18N_NOOP("Brazil"), 55 },
        {I18N_NOOP("British Virgin Islands"), 106 },
        {I18N_NOOP("Brunei"), 673 },
        {I18N_NOOP("Bulgaria"), 359 },
        {I18N_NOOP("Burkina Faso"), 226 },
        {I18N_NOOP("Burundi"), 257 },
        {I18N_NOOP("Cambodia"), 855 },
        {I18N_NOOP("Cameroon"), 237 },
        {I18N_NOOP("Canada"), 107 },
        {I18N_NOOP("Cape Verde Islands"), 238 },
        {I18N_NOOP("Cayman Islands"), 108 },
        {I18N_NOOP("Central African Republic"), 236 },
        {I18N_NOOP("Chad"), 235 },
        {I18N_NOOP("Chile"), 56 },
        {I18N_NOOP("China"), 86 },
        {I18N_NOOP("Christmas Island"), 672 },
        {I18N_NOOP("Cocos-Keeling Islands"), 6101 },
        {I18N_NOOP("Colombia"), 57 },
        {I18N_NOOP("Comoros"), 2691 },
        {I18N_NOOP("Congo"), 242 },
        {I18N_NOOP("Cook Islands"), 682 },
        {I18N_NOOP("Costa Rica"), 506 },
        {I18N_NOOP("Croatia"), 385 },
        {I18N_NOOP("Cuba"), 53 },
        {I18N_NOOP("Cyprus"), 357 },
        {I18N_NOOP("Czech Republic"), 420 },
        {I18N_NOOP("Denmark"), 45 },
        {I18N_NOOP("Diego Garcia"), 246 },
        {I18N_NOOP("Djibouti"), 253 },
        {I18N_NOOP("Dominica"), 109 },
        {I18N_NOOP("Dominican Republic"), 110 },
        {I18N_NOOP("Ecuador"), 593 },
        {I18N_NOOP("Egypt"), 20 },
        {I18N_NOOP("El Salvador"), 503 },
        {I18N_NOOP("Equatorial Guinea"), 240 },
        {I18N_NOOP("Eritrea"), 291 },
        {I18N_NOOP("Estonia"), 372 },
        {I18N_NOOP("Ethiopia"), 251 },
        {I18N_NOOP("Faeroe Islands"), 298 },
        {I18N_NOOP("Falkland Islands"), 500 },
        {I18N_NOOP("Fiji Islands"), 679 },
        {I18N_NOOP("Finland"), 358 },
        {I18N_NOOP("France"), 33 },
        {I18N_NOOP("French Antilles"), 5901 },
        {I18N_NOOP("French Guiana"), 594 },
        {I18N_NOOP("French Polynesia"), 689 },
        {I18N_NOOP("Gabon"), 241 },
        {I18N_NOOP("Gambia"), 220 },
        {I18N_NOOP("Georgia"), 995 },
        {I18N_NOOP("Germany"), 49 },
        {I18N_NOOP("Ghana"), 233 },
        {I18N_NOOP("Gibraltar"), 350 },
        {I18N_NOOP("Greece"), 30 },
        {I18N_NOOP("Greenland"), 299 },
        {I18N_NOOP("Grenada"), 111 },
        {I18N_NOOP("Guadeloupe"), 590 },
        {I18N_NOOP("Guam"), 671 },
        {I18N_NOOP("Guantanamo Bay"), 5399 },
        {I18N_NOOP("Guatemala"), 502 },
        {I18N_NOOP("Guinea"), 224 },
        {I18N_NOOP("Guinea-Bissau"), 245 },
        {I18N_NOOP("Guyana"), 592 },
        {I18N_NOOP("Haiti"), 509 },
        {I18N_NOOP("Honduras"), 504 },
        {I18N_NOOP("Hong Kong"), 852 },
        {I18N_NOOP("Hungary"), 36 },
        {I18N_NOOP("INMARSAT (Atlantic-East)"), 871 },
        {I18N_NOOP("INMARSAT (Atlantic-West)"), 874 },
        {I18N_NOOP("INMARSAT (Indian)"), 873 },
        {I18N_NOOP("INMARSAT (Pacific)"), 872 },
        {I18N_NOOP("INMARSAT"), 870 },
        {I18N_NOOP("Iceland"), 354 },
        {I18N_NOOP("India"), 91 },
        {I18N_NOOP("Indonesia"), 62 },
        {I18N_NOOP("International Freephone Service"), 800 },
        {I18N_NOOP("Iran"), 98 },
        {I18N_NOOP("Iraq"), 964 },
        {I18N_NOOP("Ireland"), 353 },
        {I18N_NOOP("Israel"), 972 },
        {I18N_NOOP("Italy"), 39 },
        {I18N_NOOP("Ivory Coast"), 225 },
        {I18N_NOOP("Jamaica"), 112 },
        {I18N_NOOP("Japan"), 81 },
        {I18N_NOOP("Jordan"), 962 },
        {I18N_NOOP("Kazakhstan"), 705 },
        {I18N_NOOP("Kenya"), 254 },
        {I18N_NOOP("Kiribati Republic"), 686 },
        {I18N_NOOP("Korea (North)"), 850 },
        {I18N_NOOP("Korea (Republic of)"), 82 },
        {I18N_NOOP("Kuwait"), 965 },
        {I18N_NOOP("Kyrgyz Republic"), 706 },
        {I18N_NOOP("Laos"), 856 },
        {I18N_NOOP("Latvia"), 371 },
        {I18N_NOOP("Lebanon"), 961 },
        {I18N_NOOP("Lesotho"), 266 },
        {I18N_NOOP("Liberia"), 231 },
        {I18N_NOOP("Libya"), 218 },
        {I18N_NOOP("Liechtenstein"), 4101 },
        {I18N_NOOP("Lithuania"), 370 },
        {I18N_NOOP("Luxembourg"), 352 },
        {I18N_NOOP("Macau"), 853 },
        {I18N_NOOP("Madagascar"), 261 },
        {I18N_NOOP("Malawi"), 265 },
        {I18N_NOOP("Malaysia"), 60 },
        {I18N_NOOP("Maldives"), 960 },
        {I18N_NOOP("Mali"), 223 },
        {I18N_NOOP("Malta"), 356 },
        {I18N_NOOP("Marshall Islands"), 692 },
        {I18N_NOOP("Martinique"), 596 },
        {I18N_NOOP("Mauritania"), 222 },
        {I18N_NOOP("Mauritius"), 230 },
        {I18N_NOOP("Mayotte Island"), 269 },
        {I18N_NOOP("Mexico"), 52 },
        {I18N_NOOP("Micronesia, Federated States of"), 691 },
        {I18N_NOOP("Moldova"), 373 },
        {I18N_NOOP("Monaco"), 377 },
        {I18N_NOOP("Mongolia"), 976 },
        {I18N_NOOP("Montserrat"), 113 },
        {I18N_NOOP("Morocco"), 212 },
        {I18N_NOOP("Mozambique"), 258 },
        {I18N_NOOP("Myanmar"), 95 },
        {I18N_NOOP("Namibia"), 264 },
        {I18N_NOOP("Nauru"), 674 },
        {I18N_NOOP("Nepal"), 977 },
        {I18N_NOOP("Netherlands Antilles"), 599 },
        {I18N_NOOP("Netherlands"), 31 },
        {I18N_NOOP("Nevis"), 114 },
        {I18N_NOOP("New Caledonia"), 687 },
        {I18N_NOOP("New Zealand"), 64 },
        {I18N_NOOP("Nicaragua"), 505 },
        {I18N_NOOP("Niger"), 227 },
        {I18N_NOOP("Nigeria"), 234 },
        {I18N_NOOP("Niue"), 683 },
        {I18N_NOOP("Norfolk Island"), 6722 },
        {I18N_NOOP("Norway"), 47 },
        {I18N_NOOP("Oman"), 968 },
        {I18N_NOOP("Pakistan"), 92 },
        {I18N_NOOP("Palau"), 680 },
        {I18N_NOOP("Panama"), 507 },
        {I18N_NOOP("Papua New Guinea"), 675 },
        {I18N_NOOP("Paraguay"), 595 },
        {I18N_NOOP("Peru"), 51 },
        {I18N_NOOP("Philippines"), 63 },
        {I18N_NOOP("Poland"), 48 },
        {I18N_NOOP("Portugal"), 351 },
        {I18N_NOOP("Puerto Rico"), 121 },
        {I18N_NOOP("Qatar"), 974 },
        {I18N_NOOP("Republic of Macedonia"), 389 },
        {I18N_NOOP("Reunion Island"), 262 },
        {I18N_NOOP("Romania"), 40 },
        {I18N_NOOP("Rota Island"), 6701 },
        {I18N_NOOP("Russia"), 7 },
        {I18N_NOOP("Rwanda"), 250 },
        {I18N_NOOP("Saint Lucia"), 122 },
        {I18N_NOOP("Saipan Island"), 670 },
        {I18N_NOOP("San Marino"), 378 },
        {I18N_NOOP("Sao Tome and Principe"), 239 },
        {I18N_NOOP("Saudi Arabia"), 966 },
        {I18N_NOOP("Senegal Republic"), 221 },
        {I18N_NOOP("Seychelle Islands"), 248 },
        {I18N_NOOP("Sierra Leone"), 232 },
        {I18N_NOOP("Singapore"), 65 },
        {I18N_NOOP("Slovak Republic"), 421 },
        {I18N_NOOP("Slovenia"), 386 },
        {I18N_NOOP("Solomon Islands"), 677 },
        {I18N_NOOP("Somalia"), 252 },
        {I18N_NOOP("South Africa"), 27 },
        {I18N_NOOP("Spain"), 34 },
        {I18N_NOOP("Sri Lanka"), 94 },
        {I18N_NOOP("St. Helena"), 290 },
        {I18N_NOOP("St. Kitts"), 115 },
        {I18N_NOOP("St. Pierre and Miquelon"), 508 },
        {I18N_NOOP("St. Vincent and the Grenadines"), 116 },
        {I18N_NOOP("Sudan"), 249 },
        {I18N_NOOP("Suriname"), 597 },
        {I18N_NOOP("Swaziland"), 268 },
        {I18N_NOOP("Sweden"), 46 },
        {I18N_NOOP("Switzerland"), 41 },
        {I18N_NOOP("Syria"), 963 },
        {I18N_NOOP("Taiwan, Republic of China"), 886 },
        {I18N_NOOP("Tajikistan"), 708 },
        {I18N_NOOP("Tanzania"), 255 },
        {I18N_NOOP("Thailand"), 66 },
        {I18N_NOOP("Tinian Island"), 6702 },
        {I18N_NOOP("Togo"), 228 },
        {I18N_NOOP("Tokelau"), 690 },
        {I18N_NOOP("Tonga"), 676 },
        {I18N_NOOP("Trinidad and Tobago"), 117 },
        {I18N_NOOP("Tunisia"), 216 },
        {I18N_NOOP("Turkey"), 90 },
        {I18N_NOOP("Turkmenistan"), 709 },
        {I18N_NOOP("Turks and Caicos Islands"), 118 },
        {I18N_NOOP("Tuvalu"), 688 },
        {I18N_NOOP("USA"), 1 },
        {I18N_NOOP("Uganda"), 256 },
        {I18N_NOOP("Ukraine"), 380 },
        {I18N_NOOP("United Arab Emirates"), 971 },
        {I18N_NOOP("United Kingdom"), 44 },
        {I18N_NOOP("United States Virgin Islands"), 123 },
        {I18N_NOOP("Uruguay"), 598 },
        {I18N_NOOP("Uzbekistan"), 711 },
        {I18N_NOOP("Vanuatu"), 678 },
        {I18N_NOOP("Vatican City"), 379 },
        {I18N_NOOP("Venezuela"), 58 },
        {I18N_NOOP("Vietnam"), 84 },
        {I18N_NOOP("Wallis and Futuna Islands"), 681 },
        {I18N_NOOP("Western Samoa"), 685 },
        {I18N_NOOP("Yemen"), 967 },
        {I18N_NOOP("Yugoslavia"), 381 },
        {I18N_NOOP("Zaire"), 243 },
        {I18N_NOOP("Zambia"), 260 },
        {I18N_NOOP("Zimbabwe"), 263 },
        {"", 0 },
*/
	countries.insert(0, "");
}

void ICQUserInfo::initLang()
{
	languages.insert(0 , "");
	languages.insert(1 , i18n("Arabic"));
	languages.insert(2 , i18n("Bhojpuri"));
	languages.insert(3 , i18n("Bulgarian"));
	languages.insert(4 , i18n("Burmese"));
	languages.insert(5 , i18n("Cantonese"));
	languages.insert(6 , i18n("Catalan"));
	languages.insert(7 , i18n("Chinese"));
	languages.insert(8 , i18n("Croatian"));
	languages.insert(9 , i18n("Czech"));
	languages.insert(10, i18n("Danish"));
	languages.insert(11, i18n("Dutch"));
	languages.insert(12, i18n("English"));
	languages.insert(13, i18n("Esperanto"));
	languages.insert(14, i18n("Estonian"));
	languages.insert(15, i18n("Farsi"));
	languages.insert(16, i18n("Finnish"));
	languages.insert(17, i18n("French"));
	languages.insert(18, i18n("Gaelic"));
	languages.insert(19, i18n("German"));
	languages.insert(20, i18n("Greek"));
	languages.insert(21, i18n("Hebrew"));
	languages.insert(22, i18n("Hindi"));
	languages.insert(23, i18n("Hungarian"));
	languages.insert(24, i18n("Icelandic"));
	languages.insert(25, i18n("Indonesian"));
	languages.insert(26, i18n("Italian"));
	languages.insert(27, i18n("Japanese"));
	languages.insert(28, i18n("Khmer"));
	languages.insert(29, i18n("Korean"));
	languages.insert(30, i18n("Lao"));
	languages.insert(31, i18n("Latvian"));
	languages.insert(32, i18n("Lithuanian"));
	languages.insert(33, i18n("Malay"));
	languages.insert(34, i18n("Norwegian"));
	languages.insert(35, i18n("Polish"));
	languages.insert(36, i18n("Portuguese"));
	languages.insert(37, i18n("Romanian"));
	languages.insert(38, i18n("Russian"));
	languages.insert(39, i18n("Serbian"));
	languages.insert(40, i18n("Slovak"));
	languages.insert(41, i18n("Slovenian"));
	languages.insert(42, i18n("Somali"));
	languages.insert(43, i18n("Spanish"));
	languages.insert(44, i18n("Swahili"));
	languages.insert(45, i18n("Swedish"));
	languages.insert(46, i18n("Tagalog"));
	languages.insert(47, i18n("Tatar"));
	languages.insert(48, i18n("Thai"));
	languages.insert(49, i18n("Turkish"));
	languages.insert(50, i18n("Ukrainian"));
	languages.insert(51, i18n("Urdu"));
	languages.insert(52, i18n("Vietnamese"));
	languages.insert(53, i18n("Yiddish"));
	languages.insert(54, i18n("Yoruba"));
	languages.insert(55, i18n("Taiwanese"));
	languages.insert(56, i18n("Afrikaans"));
	languages.insert(57, i18n("Persian"));
	languages.insert(58, i18n("Albanian"));
	languages.insert(59, i18n("Armenian"));
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
*/

	// AGE ===========================================
	if ( !mEditable ) // fixed value for readonly
	{
		mMainWidget->rwAge->setMinValue(mContact->moreInfo.age);
		mMainWidget->rwAge->setMaxValue(mContact->moreInfo.age);
	}
	mMainWidget->rwAge->setValue(mContact->moreInfo.age);

	// GENDER ========================================

//	initCombo( mMainWidget->rwGender, mUser->Gender, genders);
	setCombo(mMainWidget->rwGender, genders, mContact->moreInfo.gender);
	if ( !mEditable ) // get text from hidden combobox and insert into readonly lineedit
		mMainWidget->roGender->setText( mMainWidget->rwGender->currentText() );

	// BIRTHDAY ========================================

	if(!mContact->moreInfo.birthday.isValid()) // no birthday defined
	{
		if(mEditable)
			mMainWidget->rwBday->setDate(QDate());
		else
			mMainWidget->roBday->setText("");
	}
	else
	{
		if(mEditable)
		{
			mMainWidget->rwBday->setDate(mContact->moreInfo.birthday);
		}
		else
		{
			mMainWidget->roBday->setText(
				KGlobal::locale()->formatDate(mContact->moreInfo.birthday,true));
		}
	}

	// Personal HOMEPAGE ========================================
	homepage = QString::fromLocal8Bit(mContact->moreInfo.homepage);
	if(mEditable)
	{
		mMainWidget->prsHomepageEdit->setText( homepage );
	}
	else
	{
		if(homepage.isEmpty())
		{
			mMainWidget->prsHomepageLabel->setText( i18n("unspecified") );
			mMainWidget->prsHomepageLabel->setURL( QString::null );
			mMainWidget->prsHomepageLabel->setDisabled( true );
			mMainWidget->prsHomepageLabel->setUseCursor( false ); // disable hand cursor on mouseover
		}
		else
		{
			QString tmpHP = homepage; // copy it, do not work on the original
			mMainWidget->prsHomepageLabel->setText( tmpHP );

			if ( !tmpHP.contains("://") ) // assume http-protocol if no protocol given
				tmpHP.prepend("http://");
			mMainWidget->prsHomepageLabel->setURL( tmpHP );

			mMainWidget->prsHomepageLabel->setDisabled( false );
			mMainWidget->prsHomepageLabel->setUseCursor( true ); // enable hand cursor on mouseover
		}
	}

	// LANGUAGES =========================================

	setCombo(mMainWidget->rwLang1, languages, mContact->moreInfo.lang1);
	setCombo(mMainWidget->rwLang2, languages, mContact->moreInfo.lang2);
	setCombo(mMainWidget->rwLang3, languages, mContact->moreInfo.lang3);
// 	initCombo ( mMainWidget->rwLang1, mContact->moreInfo.lang1, languages );
// 	initCombo ( mMainWidget->rwLang2, mContact->moreInfo.lang2, languages );
// 	initCombo ( mMainWidget->rwLang3, mContact->moreInfo.lang3, languages );
	if ( !mEditable )
	{
		mMainWidget->roLang1->setText( mMainWidget->rwLang1->currentText() );
		mMainWidget->roLang2->setText( mMainWidget->rwLang2->currentText() );
		mMainWidget->roLang3->setText( mMainWidget->rwLang3->currentText() );
	}

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
		if(homepage.isEmpty())
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

	// ABOUT USER ========================================
	mMainWidget->rwAboutUser->setText(mContact->aboutInfo);

} // END slotReadInfo()

void ICQUserInfo::sendInfo()
{
	kdDebug(14200) << k_funcinfo << "called." << endl;

	KMessageBox::sorry(
		qApp->mainWidget(),
		"<qt>Changing your own user information is not done yet.</qt>",
		"unfinished code");
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
