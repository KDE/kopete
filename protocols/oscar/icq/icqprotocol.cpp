/*
  oscarprotocol.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2003 by Olivier Goffart <ogoffart@tiscalinet.be>
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

#include "icqprotocol.h"
#include "icqaccount.h"
#include "icqcontact.h"
#include "icqaddcontactpage.h"
#include "icqeditaccountwidget.h"
#include "icquserinfowidget.h"

#include <netinet/in.h> // for ntohl()

#include <qcombobox.h>
#include <qspinbox.h>
#include <qtextedit.h>
#include <qlistbox.h>
#include <qhostaddress.h>

#include <kdatewidget.h>
#include <klineedit.h>
#include <kurllabel.h>

#include <kdebug.h>
#include <kgenericfactory.h>

#include <kopeteaccountmanager.h>

//#include "oscarpreferences.h" // TODO: remove this

typedef KGenericFactory<ICQProtocol> ICQProtocolFactory;

K_EXPORT_COMPONENT_FACTORY( kopete_icq, ICQProtocolFactory( "kopete_icq_protocol" ) )

ICQProtocol* ICQProtocol::protocolStatic_ = 0L;

ICQProtocol::ICQProtocol(QObject *parent, const char *name, const QStringList&)
: KopeteProtocol( ICQProtocolFactory::instance(), parent, name ),
	statusOnline(KopeteOnlineStatus::Online, 1, this, OSCAR_ONLINE, QString::null , i18n("Online"), i18n("Online")),
	statusFFC(KopeteOnlineStatus::Online, 2, this, OSCAR_FFC, "icq_ffc", i18n("&Free for Chat"), i18n("Free For Chat")),
	statusOffline(KopeteOnlineStatus::Offline, 1, this, OSCAR_OFFLINE, QString::null, i18n("Offline"), i18n("Offline")),
	statusAway(KopeteOnlineStatus::Away, 1, this, OSCAR_AWAY, "icq_away", i18n("Away"), i18n("Away")),
	statusDND(KopeteOnlineStatus::Away, 2, this, OSCAR_DND, "icq_dnd", i18n("&Do Not Disturb"), i18n("Do not Disturb")),
	statusNA(KopeteOnlineStatus::Away, 3, this, OSCAR_NA, "icq_na", i18n("Not A&vailable"), i18n("Not Available")),
	statusOCC(KopeteOnlineStatus::Away, 4, this, OSCAR_OCC,"icq_occupied" , i18n("O&ccupied"), i18n("Occupied")),
	statusConnecting(KopeteOnlineStatus::Connecting, 99, this, OSCAR_CONNECTING, "icq_connecting", i18n("Connecting..."), i18n("Connecting..."))

{
	if (protocolStatic_)
		kdDebug(14200) << k_funcinfo << "ICQ plugin already initialized" << endl;
	else
	{
		protocolStatic_ = this;
		// Create the config widget, this does it's magic I think
//		new OscarPreferences("icq_protocol", this);
	}
	addAddressBookField("messaging/icq", KopetePlugin::MakeIndexField);

	initGenders();
	initLang();
	initCountries();
	initEncodings();
}

ICQProtocol::~ICQProtocol()
{
	protocolStatic_ =0L;
}


void ICQProtocol::initGenders()
{
	mGenders.insert(0, "");
	mGenders.insert(2, i18n("Female"));
	mGenders.insert(1, i18n("Male"));
}

void ICQProtocol::initCountries()
{
	mCountries.insert(0, ""); // unspecified
	mCountries.insert(93, i18n("Afghanistan"));
	mCountries.insert(355, i18n("Albania"));
	mCountries.insert(213, i18n("Algeria"));
	mCountries.insert(684, i18n("American Samoa"));
	mCountries.insert(376, i18n("Andorra"));
	mCountries.insert(244, i18n("Angola"));
	mCountries.insert(101, i18n("Anguilla"));
	mCountries.insert(102, i18n("Antigua"));
	mCountries.insert(54, i18n("Argentina"));
	mCountries.insert(374, i18n("Armenia"));
	mCountries.insert(297, i18n("Aruba"));
	mCountries.insert(247, i18n("Ascension Island"));
	mCountries.insert(61, i18n("Australia"));
	mCountries.insert(6721, i18n("Australian Antarctic Territory"));
	mCountries.insert(43, i18n("Austria"));
	mCountries.insert(994, i18n("Azerbaijan"));
	mCountries.insert(103, i18n("Bahamas"));
	mCountries.insert(973, i18n("Bahrain"));
	mCountries.insert(880, i18n("Bangladesh"));
	mCountries.insert(104, i18n("Barbados"));
	mCountries.insert(120, i18n("Barbuda"));
	mCountries.insert(375, i18n("Belarus"));
	mCountries.insert(32, i18n("Belgium"));
	mCountries.insert(501, i18n("Belize"));
	mCountries.insert(229, i18n("Benin"));
	mCountries.insert(105, i18n("Bermuda"));
	mCountries.insert(975, i18n("Bhutan"));
	mCountries.insert(591, i18n("Bolivia"));
	mCountries.insert(387, i18n("Bosnia and Herzegovina"));
	mCountries.insert(267, i18n("Botswana"));
	mCountries.insert(55, i18n("Brazil"));
	mCountries.insert(106, i18n("British Virgin Islands"));
	mCountries.insert(673, i18n("Brunei"));
	mCountries.insert(359, i18n("Bulgaria"));
	mCountries.insert(226, i18n("Burkina Faso"));
	mCountries.insert(257, i18n("Burundi"));
	mCountries.insert(855, i18n("Cambodia"));
	mCountries.insert(237, i18n("Cameroon"));
	mCountries.insert(107, i18n("Canada"));
	mCountries.insert(238, i18n("Cape Verde Islands"));
	mCountries.insert(108, i18n("Cayman Islands"));
	mCountries.insert(236, i18n("Central African Republic"));
	mCountries.insert(235, i18n("Chad"));
	mCountries.insert(56, i18n("Chile"));
	mCountries.insert(86, i18n("China"));
	mCountries.insert(672, i18n("Christmas Island"));
	mCountries.insert(6101, i18n("Cocos-Keeling Islands"));
	mCountries.insert(57, i18n("Colombia"));
	mCountries.insert(2691, i18n("Comoros"));
	mCountries.insert(242, i18n("Congo"));
	mCountries.insert(682, i18n("Cook Islands"));
	mCountries.insert(506, i18n("Costa Rica"));
	mCountries.insert(385, i18n("Croatia"));
	mCountries.insert(53, i18n("Cuba"));
	mCountries.insert(357, i18n("Cyprus"));
	mCountries.insert(420, i18n("Czech Republic"));
	mCountries.insert(45, i18n("Denmark"));
	mCountries.insert(246, i18n("Diego Garcia"));
	mCountries.insert(253, i18n("Djibouti"));
	mCountries.insert(109, i18n("Dominica"));
	mCountries.insert(110, i18n("Dominican Republic"));
	mCountries.insert(593, i18n("Ecuador"));
	mCountries.insert(20, i18n("Egypt"));
	mCountries.insert(503, i18n("El Salvador"));
	mCountries.insert(240, i18n("Equatorial Guinea"));
	mCountries.insert(291, i18n("Eritrea"));
	mCountries.insert(372, i18n("Estonia"));
	mCountries.insert(251, i18n("Ethiopia"));
	mCountries.insert(298, i18n("Faeroe Islands"));
	mCountries.insert(500, i18n("Falkland Islands"));
	mCountries.insert(679, i18n("Fiji Islands"));
	mCountries.insert(358, i18n("Finland"));
	mCountries.insert(33, i18n("France"));
	mCountries.insert(5901, i18n("French Antilles"));
	mCountries.insert(594, i18n("French Guiana"));
	mCountries.insert(689, i18n("French Polynesia"));
	mCountries.insert(241, i18n("Gabon"));
	mCountries.insert(220, i18n("Gambia"));
	mCountries.insert(995, i18n("Georgia"));
	mCountries.insert(49, i18n("Germany"));
	mCountries.insert(233, i18n("Ghana"));
	mCountries.insert(350, i18n("Gibraltar"));
	mCountries.insert(30, i18n("Greece"));
	mCountries.insert(299, i18n("Greenland"));
	mCountries.insert(111, i18n("Grenada"));
	mCountries.insert(590, i18n("Guadeloupe"));
	mCountries.insert(671, i18n("Guam"));
	mCountries.insert(5399, i18n("Guantanamo Bay"));
	mCountries.insert(502, i18n("Guatemala"));
	mCountries.insert(224, i18n("Guinea"));
	mCountries.insert(245, i18n("Guinea-Bissau"));
	mCountries.insert(592, i18n("Guyana"));
	mCountries.insert(509, i18n("Haiti"));
	mCountries.insert(504, i18n("Honduras"));
	mCountries.insert(852, i18n("Hong Kong"));
	mCountries.insert(36, i18n("Hungary"));
	mCountries.insert(871, i18n("INMARSAT (Atlantic-East)"));
	mCountries.insert(874, i18n("INMARSAT (Atlantic-West)"));
	mCountries.insert(873, i18n("INMARSAT (Indian)"));
	mCountries.insert(872, i18n("INMARSAT (Pacific)"));
	mCountries.insert(870, i18n("INMARSAT"));
	mCountries.insert(354, i18n("Iceland"));
	mCountries.insert(91, i18n("India"));
	mCountries.insert(62, i18n("Indonesia"));
	mCountries.insert(800, i18n("International Freephone Service"));
	mCountries.insert(98, i18n("Iran"));
	mCountries.insert(964, i18n("Iraq"));
	mCountries.insert(353, i18n("Ireland"));
	mCountries.insert(972, i18n("Israel"));
	mCountries.insert(39, i18n("Italy"));
	mCountries.insert(225, i18n("Ivory Coast"));
	mCountries.insert(112, i18n("Jamaica"));
	mCountries.insert(81, i18n("Japan"));
	mCountries.insert(962, i18n("Jordan"));
	mCountries.insert(705, i18n("Kazakhstan"));
	mCountries.insert(254, i18n("Kenya"));
	mCountries.insert(686, i18n("Kiribati Republic"));
	mCountries.insert(850, i18n("Korea (North)"));
	mCountries.insert(82, i18n("Korea (Republic of)"));
	mCountries.insert(965, i18n("Kuwait"));
	mCountries.insert(706, i18n("Kyrgyz Republic"));
	mCountries.insert(856, i18n("Laos"));
	mCountries.insert(371, i18n("Latvia"));
	mCountries.insert(961, i18n("Lebanon"));
	mCountries.insert(266, i18n("Lesotho"));
	mCountries.insert(231, i18n("Liberia"));
	mCountries.insert(218, i18n("Libya"));
	mCountries.insert(4101, i18n("Liechtenstein"));
	mCountries.insert(370, i18n("Lithuania"));
	mCountries.insert(352, i18n("Luxembourg"));
	mCountries.insert(853, i18n("Macau"));
	mCountries.insert(261, i18n("Madagascar"));
	mCountries.insert(265, i18n("Malawi"));
	mCountries.insert(60, i18n("Malaysia"));
	mCountries.insert(960, i18n("Maldives"));
	mCountries.insert(223, i18n("Mali"));
	mCountries.insert(356, i18n("Malta"));
	mCountries.insert(692, i18n("Marshall Islands"));
	mCountries.insert(596, i18n("Martinique"));
	mCountries.insert(222, i18n("Mauritania"));
	mCountries.insert(230, i18n("Mauritius"));
	mCountries.insert(269, i18n("Mayotte Island"));
	mCountries.insert(52, i18n("Mexico"));
	mCountries.insert(691, i18n("Micronesia, Federated States of"));
	mCountries.insert(373, i18n("Moldova"));
	mCountries.insert(377, i18n("Monaco"));
	mCountries.insert(976, i18n("Mongolia"));
	mCountries.insert(113, i18n("Montserrat"));
	mCountries.insert(212, i18n("Morocco"));
	mCountries.insert(258, i18n("Mozambique"));
	mCountries.insert(95, i18n("Myanmar"));
	mCountries.insert(264, i18n("Namibia"));
	mCountries.insert(674, i18n("Nauru"));
	mCountries.insert(977, i18n("Nepal"));
	mCountries.insert(599, i18n("Netherlands Antilles"));
	mCountries.insert(31, i18n("Netherlands"));
	mCountries.insert(114, i18n("Nevis"));
	mCountries.insert(687, i18n("New Caledonia"));
	mCountries.insert(64, i18n("New Zealand"));
	mCountries.insert(505, i18n("Nicaragua"));
	mCountries.insert(227, i18n("Niger"));
	mCountries.insert(234, i18n("Nigeria"));
	mCountries.insert(683, i18n("Niue"));
	mCountries.insert(6722, i18n("Norfolk Island"));
	mCountries.insert(47, i18n("Norway"));
	mCountries.insert(968, i18n("Oman"));
	mCountries.insert(92, i18n("Pakistan"));
	mCountries.insert(680, i18n("Palau"));
	mCountries.insert(507, i18n("Panama"));
	mCountries.insert(675, i18n("Papua New Guinea"));
	mCountries.insert(595, i18n("Paraguay"));
	mCountries.insert(51, i18n("Peru"));
	mCountries.insert(63, i18n("Philippines"));
	mCountries.insert(48, i18n("Poland"));
	mCountries.insert(351, i18n("Portugal"));
	mCountries.insert(121, i18n("Puerto Rico"));
	mCountries.insert(974, i18n("Qatar"));
	mCountries.insert(389, i18n("Republic of Macedonia"));
	mCountries.insert(262, i18n("Reunion Island"));
	mCountries.insert(40, i18n("Romania"));
	mCountries.insert(6701, i18n("Rota Island"));
	mCountries.insert(7, i18n("Russia"));
	mCountries.insert(250, i18n("Rwanda"));
	mCountries.insert(122, i18n("Saint Lucia"));
	mCountries.insert(670, i18n("Saipan Island"));
	mCountries.insert(378, i18n("San Marino"));
	mCountries.insert(239, i18n("Sao Tome and Principe"));
	mCountries.insert(966, i18n("Saudi Arabia"));
	mCountries.insert(221, i18n("Senegal Republic"));
	mCountries.insert(248, i18n("Seychelle Islands"));
	mCountries.insert(232, i18n("Sierra Leone"));
	mCountries.insert(65, i18n("Singapore"));
	mCountries.insert(421, i18n("Slovak Republic"));
	mCountries.insert(386, i18n("Slovenia"));
	mCountries.insert(677, i18n("Solomon Islands"));
	mCountries.insert(252, i18n("Somalia"));
	mCountries.insert(27, i18n("South Africa"));
	mCountries.insert(34, i18n("Spain"));
	mCountries.insert(94, i18n("Sri Lanka"));
	mCountries.insert(290, i18n("St. Helena"));
	mCountries.insert(115, i18n("St. Kitts"));
	mCountries.insert(508, i18n("St. Pierre and Miquelon"));
	mCountries.insert(116, i18n("St. Vincent and the Grenadines"));
	mCountries.insert(249, i18n("Sudan"));
	mCountries.insert(597, i18n("Suriname"));
	mCountries.insert(268, i18n("Swaziland"));
	mCountries.insert(46, i18n("Sweden"));
	mCountries.insert(41, i18n("Switzerland"));
	mCountries.insert(963, i18n("Syria"));
	mCountries.insert(886, i18n("Taiwan, Republic of China"));
	mCountries.insert(708, i18n("Tajikistan"));
	mCountries.insert(255, i18n("Tanzania"));
	mCountries.insert(66, i18n("Thailand"));
	mCountries.insert(6702, i18n("Tinian Island"));
	mCountries.insert(228, i18n("Togo"));
	mCountries.insert(690, i18n("Tokelau"));
	mCountries.insert(676, i18n("Tonga"));
	mCountries.insert(117, i18n("Trinidad and Tobago"));
	mCountries.insert(216, i18n("Tunisia"));
	mCountries.insert(90, i18n("Turkey"));
	mCountries.insert(709, i18n("Turkmenistan"));
	mCountries.insert(118, i18n("Turks and Caicos Islands"));
	mCountries.insert(688, i18n("Tuvalu"));
	mCountries.insert(1, i18n("USA"));
	mCountries.insert(256, i18n("Uganda"));
	mCountries.insert(380, i18n("Ukraine"));
	mCountries.insert(971, i18n("United Arab Emirates"));
	mCountries.insert(44, i18n("United Kingdom"));
	mCountries.insert(123, i18n("United States Virgin Islands"));
	mCountries.insert(598, i18n("Uruguay"));
	mCountries.insert(711, i18n("Uzbekistan"));
	mCountries.insert(678, i18n("Vanuatu"));
	mCountries.insert(379, i18n("Vatican City"));
	mCountries.insert(58, i18n("Venezuela"));
	mCountries.insert(84, i18n("Vietnam"));
	mCountries.insert(681, i18n("Wallis and Futuna Islands"));
	mCountries.insert(685, i18n("Western Samoa"));
	mCountries.insert(967, i18n("Yemen"));
	mCountries.insert(381, i18n("Yugoslavia"));
	mCountries.insert(243, i18n("Zaire"));
	mCountries.insert(260, i18n("Zambia"));
	mCountries.insert(263, i18n("Zimbabwe"));
}

void ICQProtocol::initLang()
{
	mLanguages.insert(0 , "");
	mLanguages.insert(1 , i18n("Arabic"));
	mLanguages.insert(2 , i18n("Bhojpuri"));
	mLanguages.insert(3 , i18n("Bulgarian"));
	mLanguages.insert(4 , i18n("Burmese"));
	mLanguages.insert(5 , i18n("Cantonese"));
	mLanguages.insert(6 , i18n("Catalan"));
	mLanguages.insert(7 , i18n("Chinese"));
	mLanguages.insert(8 , i18n("Croatian"));
	mLanguages.insert(9 , i18n("Czech"));
	mLanguages.insert(10, i18n("Danish"));
	mLanguages.insert(11, i18n("Dutch"));
	mLanguages.insert(12, i18n("English"));
	mLanguages.insert(13, i18n("Esperanto"));
	mLanguages.insert(14, i18n("Estonian"));
	mLanguages.insert(15, i18n("Farsi"));
	mLanguages.insert(16, i18n("Finnish"));
	mLanguages.insert(17, i18n("French"));
	mLanguages.insert(18, i18n("Gaelic"));
	mLanguages.insert(19, i18n("German"));
	mLanguages.insert(20, i18n("Greek"));
	mLanguages.insert(21, i18n("Hebrew"));
	mLanguages.insert(22, i18n("Hindi"));
	mLanguages.insert(23, i18n("Hungarian"));
	mLanguages.insert(24, i18n("Icelandic"));
	mLanguages.insert(25, i18n("Indonesian"));
	mLanguages.insert(26, i18n("Italian"));
	mLanguages.insert(27, i18n("Japanese"));
	mLanguages.insert(28, i18n("Khmer"));
	mLanguages.insert(29, i18n("Korean"));
	mLanguages.insert(30, i18n("Lao"));
	mLanguages.insert(31, i18n("Latvian"));
	mLanguages.insert(32, i18n("Lithuanian"));
	mLanguages.insert(33, i18n("Malay"));
	mLanguages.insert(34, i18n("Norwegian"));
	mLanguages.insert(35, i18n("Polish"));
	mLanguages.insert(36, i18n("Portuguese"));
	mLanguages.insert(37, i18n("Romanian"));
	mLanguages.insert(38, i18n("Russian"));
	mLanguages.insert(39, i18n("Serbian"));
	mLanguages.insert(40, i18n("Slovak"));
	mLanguages.insert(41, i18n("Slovenian"));
	mLanguages.insert(42, i18n("Somali"));
	mLanguages.insert(43, i18n("Spanish"));
	mLanguages.insert(44, i18n("Swahili"));
	mLanguages.insert(45, i18n("Swedish"));
	mLanguages.insert(46, i18n("Tagalog"));
	mLanguages.insert(47, i18n("Tatar"));
	mLanguages.insert(48, i18n("Thai"));
	mLanguages.insert(49, i18n("Turkish"));
	mLanguages.insert(50, i18n("Ukrainian"));
	mLanguages.insert(51, i18n("Urdu"));
	mLanguages.insert(52, i18n("Vietnamese"));
	mLanguages.insert(53, i18n("Yiddish"));
	mLanguages.insert(54, i18n("Yoruba"));
	mLanguages.insert(55, i18n("Taiwanese"));
	mLanguages.insert(56, i18n("Afrikaans"));
	mLanguages.insert(57, i18n("Persian"));
	mLanguages.insert(58, i18n("Albanian"));
	mLanguages.insert(59, i18n("Armenian"));
}

void ICQProtocol::initEncodings()
{
	mEncodings.insert(0 , i18n("Automatic")); // guess encoding instead of hardcoding

	mEncodings.insert(2026, i18n("Big5"));
	mEncodings.insert(2101, i18n("Big5-HKSCS"));
	mEncodings.insert(18, i18n("euc-JP Japanese"));
	mEncodings.insert(38, i18n("euc-KR Korean"));
	mEncodings.insert(57, i18n("GB-2312 Chinese"));
	mEncodings.insert(113, i18n("GBK Chinese"));
	mEncodings.insert(114, i18n("GB18030 Chinese"));

	mEncodings.insert(16, i18n("JIS Japanese"));
	mEncodings.insert(17, i18n("Shift-JIS Japanese"));

	mEncodings.insert(2084, i18n("KOI8-R Russian"));
	mEncodings.insert(2088, i18n("KOI8-U Ukrainian"));

	mEncodings.insert(4, i18n("ISO-8859-1 Western"));
	mEncodings.insert(5, i18n("ISO-8859-2 Central European"));
	mEncodings.insert(6, i18n("ISO-8859-3 Central European"));
	mEncodings.insert(7, i18n("ISO-8859-4 Baltic"));
	mEncodings.insert(8, i18n("ISO-8859-5 Cyrillic"));
	mEncodings.insert(9, i18n("ISO-8859-6 Arabic"));
	mEncodings.insert(10, i18n("ISO-8859-7 Greek"));
	mEncodings.insert(11, i18n("ISO-8859-8 Hebrew, visually ordered"));
	mEncodings.insert(85, i18n("ISO-8859-8-I Hebrew, logically ordered"));
	mEncodings.insert(12, i18n("ISO-8859-9 Turkish"));
	mEncodings.insert(13, i18n("ISO-8859-10"));
	mEncodings.insert(109, i18n("ISO-8859-13"));
	mEncodings.insert(110, i18n("ISO-8859-14"));
	mEncodings.insert(111, i18n("ISO-8859-15 Western"));

	mEncodings.insert(2250, i18n("Windows-1250 Central European"));
	mEncodings.insert(2251, i18n("Windows-1251 Cyrillic"));
	mEncodings.insert(2252, i18n("Windows-1252 Western"));
	mEncodings.insert(2253, i18n("Windows-1253 Greek"));
	mEncodings.insert(2254, i18n("Windows-1254 Turkish"));
	mEncodings.insert(2255, i18n("Windows-1255 Hebrew"));
	mEncodings.insert(2256, i18n("Windows-1256 Arabic"));
	mEncodings.insert(2257, i18n("Windows-1257 Baltic"));
	mEncodings.insert(2258, i18n("Windows-1258 Viet Nam"));

	mEncodings.insert(2009, i18n("IBM 850"));
	mEncodings.insert(2085, i18n("IBM 866"));

	mEncodings.insert(2259, i18n("TIS-620 Thai"));

/*
Missing ones (copied from qtextcodec doc):
TSCII -- Tamil
utf8 -- Unicode, 8-bit
utf16 -- Unicode
CP874
Apple Roman
*/
}

void ICQProtocol::fillComboFromTable(QComboBox *box, const QMap<int, QString> &map)
{
//	kdDebug(14200) << k_funcinfo << "Called." << endl;

	QStringList list = map.values();
	list.sort();
	box->insertStringList(list);
}

void ICQProtocol::setComboFromTable(QComboBox *box, const QMap<int, QString> &map, int value)
{
//	kdDebug(14200) << k_funcinfo << "Called." << endl;
	QMap<int, QString>::ConstIterator it;
	it = map.find(value);
	if (!(*it))
		return;

	for(int i=0; i<box->count(); i++)
	{
		if((*it) == box->text(i))
		{
			box->setCurrentItem(i);
			return;
		}
	}
}

int ICQProtocol::getCodeForCombo(QComboBox *cmb, const QMap<int, QString> &map)
{
	const QString curText = cmb->currentText();

	QMap<int, QString>::ConstIterator it;
	for(it = map.begin(); it != map.end(); ++it)
	{
		if(it.data() == curText)
			return it.key();
	}
	return 0; // unspecified is always first 0
}

void ICQProtocol::fillTZCombo(QComboBox *combo)
{
	QTime time(12, 0);
	QTime done(0, 0);

	while(time > done)
	{
		combo->insertItem("GMT-" + time.toString("h:mm"));
		// subtract 30 minutes
		time = time.addSecs(-30 * 60);
	}

	time = QTime(0, 0);
	done = QTime(12, 0);

	while(time <= done)
	{
		combo->insertItem("GMT+" + time.toString("h:mm"));
		// add 30 minutes
		time = time.addSecs(30 * 60);
	}
}

void ICQProtocol::setTZComboValue(QComboBox *combo, const char &tz)
{
// 	kdDebug(14200) << k_funcinfo << "tz=" << int(tz) << endl;
	if ((tz < -24) || (tz > 24))
		combo->setCurrentItem(24); // GMT+0:00 as default
	else
		combo->setCurrentItem(24 + tz);
}

char ICQProtocol::getTZComboValue(QComboBox *combo)
{
	char ret =  combo->currentItem() - 24;
// 	kdDebug(14200) << k_funcinfo << "return value=" << int(ret) << endl;
	return ret;
}

void ICQProtocol::initUserinfoWidget(ICQUserInfoWidget *widget)
{
	fillComboFromTable(widget->rwGender, genders());
	fillComboFromTable(widget->rwLang1, languages());
	fillComboFromTable(widget->rwLang2, languages());
	fillComboFromTable(widget->rwLang3, languages());
	fillComboFromTable(widget->rwPrsCountry, countries());
	fillComboFromTable(widget->rwWrkCountry, countries());
	fillComboFromTable(widget->cmbEncoding, encodings());
	fillTZCombo(widget->rwTimezone);
}


void ICQProtocol::contactInfo2UserInfoWidget(ICQContact *c, ICQUserInfoWidget *widget, bool editMode)
{
	QString homepage;

	// General tab
	if(!editMode) // no idea how to get ip for ourselves
	{
		QHostAddress ip(ntohl(c->userInfo().localip));
		QHostAddress realip(ntohl(c->userInfo().realip));
		unsigned short port = c->userInfo().port;

		if ( !(ip == realip) && !(realip == QHostAddress()) )
		{
			widget->roIPAddress->setText(
				QString("%1 (%2:%3)").arg(ip.toString()).arg(realip.toString()).arg(port)
				);
		}
		else
		{
			widget->roIPAddress->setText(
				QString("%1:%2").arg(ip.toString()).arg(port)
				);
		}
	}

	if(c->userInfo().onlinesince.isValid())
		widget->roSignonTime->setText(c->userInfo().onlinesince.toString(Qt::LocalDate));
	if(c->userInfo().membersince.isValid())
		widget->roCreationTime->setText(c->userInfo().membersince.toString(Qt::LocalDate));

	widget->rwNickName->setText(c->generalInfo.nickName);
	widget->rwAlias->setText(c->displayName());
	widget->rwFirstName->setText(c->generalInfo.firstName);
	widget->rwLastName->setText(c->generalInfo.lastName);

	// Private details tab
	QString email = c->generalInfo.eMail;
	if (editMode)
		widget->prsEmailEdit->setText(email);
	else
	{
		if (email.isEmpty()) // either NULL or ""
		{
			widget->prsEmailLabel->setText(i18n("unspecified"));
			widget->prsEmailLabel->setURL(QString::null);
			widget->prsEmailLabel->setDisabled( true );
			widget->prsEmailLabel->setUseCursor( false ); // disable hand cursor on mouseover
		}
		else
		{
			widget->prsEmailLabel->setText(email);
			widget->prsEmailLabel->setURL(email);
			widget->prsEmailLabel->setDisabled(false);
			widget->prsEmailLabel->setUseCursor(true); // enable hand cursor on mouseover
		}
	}

	// PRIVATE COUNTRY ==============================
	setComboFromTable(widget->rwPrsCountry,countries(),
		c->generalInfo.countryCode);
	if (!editMode)
		widget->roPrsCountry->setText( widget->rwPrsCountry->currentText() );

	widget->prsStateEdit->setText(c->generalInfo.state);
	widget->prsCityEdit->setText(c->generalInfo.city);
	widget->prsZipcodeEdit->setText(c->generalInfo.zip);
	widget->prsAddressEdit->setText(c->generalInfo.street);

	widget->prsPhoneEdit->setText(c->generalInfo.phoneNumber);
	widget->prsCellphoneEdit->setText(c->generalInfo.cellularNumber);
	widget->prsFaxEdit->setText(c->generalInfo.faxNumber);

	// TIMEZONE ======================================
	fillTZCombo(widget->rwTimezone);
	setTZComboValue(widget->rwTimezone, c->generalInfo.timezoneCode);
	kdDebug(14200) << k_funcinfo << "timezonecode=" << c->generalInfo.timezoneCode << endl;
	/*
	widget->rwTimezone->setCurrentText(c->generalInfo.timezoneCode)
		QString("GMT%1%2:%3")
			.arg(c->generalInfo.timezoneCode > 0 ? "-" : "+")
			.arg(abs(c->generalInfo.timezoneCode / 2))
			.arg(c->generalInfo.timezoneCode % 2 ? "30" : "00")
			);*/

	if(!editMode)
		widget->roTimezone->setText(widget->rwTimezone->currentText());

	// AGE ===========================================
	if(!editMode) // fixed value for readonly
	{
		widget->rwAge->setMinValue(c->moreInfo.age);
		widget->rwAge->setMaxValue(c->moreInfo.age);
	}
	widget->rwAge->setValue(c->moreInfo.age);

	// GENDER ========================================

	setComboFromTable(widget->rwGender, genders(), c->moreInfo.gender);
	if(!editMode) // get text from hidden combobox and insert into readonly lineedit
		widget->roGender->setText( widget->rwGender->currentText() );

	// BIRTHDAY ========================================

	if(!c->moreInfo.birthday.isValid()) // no birthday defined
	{
		if(editMode)
			widget->rwBday->setDate(QDate());
		else
			widget->roBday->setText("");
	}
	else
	{
		if(editMode)
		{
			widget->rwBday->setDate(c->moreInfo.birthday);
		}
		else
		{
			widget->roBday->setText(
				KGlobal::locale()->formatDate(c->moreInfo.birthday,true));
		}
	}

	// Personal HOMEPAGE ========================================
	homepage = c->moreInfo.homepage;
	if(editMode)
	{
		widget->prsHomepageEdit->setText( homepage );
	}
	else
	{
		if(homepage.isEmpty())
		{
			widget->prsHomepageLabel->setText( i18n("unspecified") );
			widget->prsHomepageLabel->setURL( QString::null );
			widget->prsHomepageLabel->setDisabled( true );
			widget->prsHomepageLabel->setUseCursor( false ); // disable hand cursor on mouseover
		}
		else
		{
			QString tmpHP = homepage; // copy it, do not work on the original
			widget->prsHomepageLabel->setText( tmpHP );

			if ( !tmpHP.contains("://") ) // assume http-protocol if no protocol given
				tmpHP.prepend("http://");
			widget->prsHomepageLabel->setURL( tmpHP );

			widget->prsHomepageLabel->setDisabled( false );
			widget->prsHomepageLabel->setUseCursor( true ); // enable hand cursor on mouseover
		}
	}

	// LANGUAGES =========================================

	setComboFromTable(widget->rwLang1, languages(), c->moreInfo.lang1);
	setComboFromTable(widget->rwLang2, languages(), c->moreInfo.lang2);
	setComboFromTable(widget->rwLang3, languages(), c->moreInfo.lang3);
	if(!editMode)
	{
		widget->roLang1->setText( widget->rwLang1->currentText() );
		widget->roLang2->setText( widget->rwLang2->currentText() );
		widget->roLang3->setText( widget->rwLang3->currentText() );
	}

	// WORK INFO ========================================

	widget->wrkCityEdit->setText(c->workInfo.city);
	widget->wrkStateEdit->setText(c->workInfo.state);
	widget->wrkPhoneEdit->setText (c->workInfo.phone);
	widget->wrkFaxEdit->setText (c->workInfo.fax);
	widget->wrkAddressEdit->setText(c->workInfo.address);
	widget->wrkZipcodeEdit->setText(c->workInfo.zip);
	widget->wrkNameEdit->setText(c->workInfo.company);
	widget->wrkDepartmentEdit->setText(c->workInfo.department);
	widget->wrkPositionEdit->setText(c->workInfo.position);
	// TODO: c->workInfo.occupation

	// WORK HOMEPAGE =====================================

	homepage = c->workInfo.homepage;
	if ( editMode )
	{
		widget->wrkHomepageEdit->setText(homepage);
	}
	else
	{
		if(homepage.isEmpty())
		{
			widget->wrkHomepageLabel->setText(i18n("unspecified"));
			widget->wrkHomepageLabel->setURL(QString::null);
			widget->wrkHomepageLabel->setDisabled(true);
			widget->wrkHomepageLabel->setUseCursor(false); // disable hand cursor on mouseover
		}
		else
		{
			QString tmpHP = homepage; // copy it, do not work on the original
			widget->wrkHomepageLabel->setText(tmpHP);

			if(!tmpHP.contains("://")) // assume http-protocol if not protocol given
				tmpHP.prepend("http://");
			widget->wrkHomepageLabel->setURL(tmpHP);

			widget->wrkHomepageLabel->setDisabled(false);
			widget->wrkHomepageLabel->setUseCursor(true); // enable hand cursor on mouseover
		}
	}

	setComboFromTable(widget->rwWrkCountry, countries(), c->workInfo.countryCode);
	if (!editMode)
		widget->roWrkCountry->setText(widget->rwWrkCountry->currentText());

	// ABOUT USER ========================================
	widget->rwAboutUser->setText(c->aboutInfo);

	ICQMailList::iterator it;
	for (it = c->emailInfo.begin(); it != c->emailInfo.end(); ++it )
	{
		widget->lstEmails->insertItem(it.key());
	}

	// USER INTERESTS ==========================================================
	// set all widgets to None/disabled

	int i = 0;
	for ( ICQInfoItemList::iterator it = c->interestInfo.begin(); it != c->interestInfo.end(); ++it )
	{
		// fill in any interests we know about
		// set interest categories and populate combo boxes
		const int INTEREST_OFFSET = 99;
		switch (i)
		{
			case 0:
				widget->intrCategoryCombo1->setCurrentItem( (*it).category - INTEREST_OFFSET );
				widget->intrDescText1->setText( (*it).description );
				break;
			case 1:
				widget->intrCategoryCombo2->setCurrentItem( (*it).category - INTEREST_OFFSET );
				widget->intrDescText2->setText( (*it).description );
				break;
			case 2:
				widget->intrCategoryCombo3->setCurrentItem( (*it).category - INTEREST_OFFSET );
				widget->intrDescText3->setText( (*it).description );
				break;
			case 3:
				widget->intrCategoryCombo4->setCurrentItem( (*it).category - INTEREST_OFFSET );
				widget->intrDescText4->setText( (*it).description );
				break;
			default:
				break;
		}
		i++;
	}

	// USER BACKGROUND - CURRENT MEMBERSHIPS====================================
	i = 0;
	for ( ICQInfoItemList::iterator it = c->currentBackground.begin(); it != c->currentBackground.end(); ++it )
	{
		// fill in any interests we know about
		// set interest categories and populate combo boxes
		const int ORGANISATION_OFFSET = 199;
		switch (i)
		{
			case 0:
				widget->bgrdCurrOrgCombo1->setCurrentItem( (*it).category - ORGANISATION_OFFSET );
				widget->bgrdCurrOrgText1->setText( (*it).description );
				break;
			case 1:
				widget->bgrdCurrOrgCombo2->setCurrentItem( (*it).category - ORGANISATION_OFFSET );
				widget->bgrdCurrOrgText2->setText( (*it).description );
				break;
			case 2:
				widget->bgrdCurrOrgCombo3->setCurrentItem( (*it).category - ORGANISATION_OFFSET );
				widget->bgrdCurrOrgText3->setText( (*it).description );
				break;
			default:
				break;
		}
		i++;
	}

	// USER BACKGROUND - PREVIOUS AFFILIATIONS==================================
	i = 0;
	for ( ICQInfoItemList::iterator it = c->pastBackground.begin(); it != c->pastBackground.end(); ++it )
	{
		// fill in any interests we know about
		// set interest categories and populate combo boxes
		const int ORGANISATION_OFFSET = 299;
		switch (i)
		{
			case 0:
				widget->bgrdPastOrgCombo1->setCurrentItem( (*it).category - ORGANISATION_OFFSET );
				widget->bgrdPastOrgText1->setText( (*it).description );
				break;
			case 1:
				widget->bgrdPastOrgCombo2->setCurrentItem( (*it).category - ORGANISATION_OFFSET );
				widget->bgrdPastOrgText2->setText( (*it).description );
				break;
			case 2:
				widget->bgrdPastOrgCombo3->setCurrentItem( (*it).category - ORGANISATION_OFFSET );
				widget->bgrdPastOrgText3->setText( (*it).description );
				break;
			default:
				break;
		}
		i++;
	}

} // END contactInfo2UserInfoWidget()

// Called when we want to return the active instance of the protocol
ICQProtocol *ICQProtocol::protocol()
{
	return protocolStatic_;
}

bool ICQProtocol::canSendOffline() const
{
	return true;
}

// This will be called when Kopete reads the contact list
void ICQProtocol::deserializeContact(KopeteMetaContact *metaContact,
	const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> &/*addressBookData*/)
{
	QString contactId=serializedData["contactId"];
	QString accountId=serializedData["accountId"];
	QString displayName=serializedData["displayName"];

	// Get the account it belongs to
	QDict<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts(this);
	ICQAccount *account = static_cast<ICQAccount*>(accounts[accountId]);

	if(!account)
	{
		kdDebug(14200) << k_funcinfo << "WARNING: Account for contact does not exist, skipping." << endl;
		return;
	}
	ICQContact *c = new ICQContact(contactId, displayName, account, metaContact);
	c->setGroupId(serializedData["groupID"].toInt());
	c->setEncoding(serializedData["Encoding"].toInt());
	c->setWaitAuth(serializedData["awaitingAuth"] == "1");
}

AddContactPage *ICQProtocol::createAddContactWidget(QWidget *parent, KopeteAccount *account)
{
	return (new ICQAddContactPage(static_cast<ICQAccount*>(account) , parent));
}

KopeteEditAccountWidget *ICQProtocol::createEditAccountWidget(KopeteAccount *account, QWidget *parent)
{
	return (new ICQEditAccountWidget(this, account, parent));
}

KopeteAccount *ICQProtocol::createNewAccount(const QString &accountId)
{
	return (new ICQAccount(this, accountId));
}

#include "icqprotocol.moc"
// vim: set noet ts=4 sts=4 sw=4:
