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
#include "kopeteglobal.h"

#include <netinet/in.h> // for ntohl()

#include <qcombobox.h>
#include <qspinbox.h>
#include <qtextedit.h>
#include <qlistbox.h>
#include <qhostaddress.h>

#include <klocale.h>
#include <kdatewidget.h>
#include <klineedit.h>
#include <kurllabel.h>

#include <kdebug.h>
#include <kgenericfactory.h>

#include <kopeteaccountmanager.h>


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
	statusConnecting(KopeteOnlineStatus::Connecting, 99, this, OSCAR_CONNECTING, "icq_connecting", i18n("Connecting..."), i18n("Connecting...")),
	firstName(Kopete::Global::Properties::self()->firstName()),
	lastName(Kopete::Global::Properties::self()->lastName()),
	awayMessage(Kopete::Global::Properties::self()->awayMessage())
{
	if (protocolStatic_)
		kdDebug(14200) << k_funcinfo << "ICQ plugin already initialized" << endl;
	else
		protocolStatic_ = this;
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
	mGenders.insert(0, i18n("Unspecified"));
	mGenders.insert(1, i18n("Female"));
	mGenders.insert(2, i18n("Male"));
}

void ICQProtocol::initCountries()
{
	mCountries.insert(0, ""); // unspecified
	//Shut the translators up (bug 77828)
	KLocale *kl = KGlobal::locale(); //KLocale(QString::fromLatin1("kopete"));

	mCountries.insert(93, kl->twoAlphaToCountryName("af"));
	mCountries.insert(355, kl->twoAlphaToCountryName("al"));
	mCountries.insert(213, kl->twoAlphaToCountryName("dz"));
	mCountries.insert(684, kl->twoAlphaToCountryName("as"));
	mCountries.insert(376, kl->twoAlphaToCountryName("ad"));
	mCountries.insert(244, kl->twoAlphaToCountryName("ao"));
	mCountries.insert(101, kl->twoAlphaToCountryName("ai"));
	mCountries.insert(102, kl->twoAlphaToCountryName("ag"));
	mCountries.insert(54, kl->twoAlphaToCountryName("ar"));
	mCountries.insert(374, kl->twoAlphaToCountryName("am"));
	mCountries.insert(297, kl->twoAlphaToCountryName("aw"));
	mCountries.insert(247, i18n("Ascension Island"));
	mCountries.insert(61, kl->twoAlphaToCountryName("au"));
	mCountries.insert(6721, i18n("Australian Antarctic Territory"));
	mCountries.insert(43, kl->twoAlphaToCountryName("at"));
	mCountries.insert(994, kl->twoAlphaToCountryName("az"));
	mCountries.insert(103, kl->twoAlphaToCountryName("bs"));
	mCountries.insert(973, kl->twoAlphaToCountryName("bh"));
	mCountries.insert(880, kl->twoAlphaToCountryName("bd"));
	mCountries.insert(104, kl->twoAlphaToCountryName("bb"));
	mCountries.insert(120, i18n("Barbuda"));
	mCountries.insert(375, kl->twoAlphaToCountryName("by"));
	mCountries.insert(32, kl->twoAlphaToCountryName("be"));
	mCountries.insert(501, kl->twoAlphaToCountryName("bz"));
	mCountries.insert(229, kl->twoAlphaToCountryName("bj"));
	mCountries.insert(105, kl->twoAlphaToCountryName("bm"));
	mCountries.insert(975, kl->twoAlphaToCountryName("bt"));
	mCountries.insert(591, kl->twoAlphaToCountryName("bo"));
	mCountries.insert(387, kl->twoAlphaToCountryName("ba"));
	mCountries.insert(267, kl->twoAlphaToCountryName("bw"));
	mCountries.insert(55, kl->twoAlphaToCountryName("br"));
	mCountries.insert(106, i18n("British Virgin Islands"));
	mCountries.insert(673, kl->twoAlphaToCountryName("bn"));
	mCountries.insert(359, kl->twoAlphaToCountryName("bg"));
	mCountries.insert(226, kl->twoAlphaToCountryName("bf"));
	mCountries.insert(257, kl->twoAlphaToCountryName("bi"));
	mCountries.insert(855, kl->twoAlphaToCountryName("kh"));
	mCountries.insert(237, kl->twoAlphaToCountryName("cm"));
	mCountries.insert(107, kl->twoAlphaToCountryName("ca"));
	mCountries.insert(238, kl->twoAlphaToCountryName("cv"));
	mCountries.insert(108, kl->twoAlphaToCountryName("ky"));
	mCountries.insert(236, kl->twoAlphaToCountryName("cf"));
	mCountries.insert(235, kl->twoAlphaToCountryName("td"));
	mCountries.insert(56, kl->twoAlphaToCountryName("cl"));
	mCountries.insert(86, kl->twoAlphaToCountryName("cn"));
	mCountries.insert(672, kl->twoAlphaToCountryName("cx"));
	mCountries.insert(6101, kl->twoAlphaToCountryName("c"));
	mCountries.insert(57, kl->twoAlphaToCountryName("co"));
	mCountries.insert(2691, kl->twoAlphaToCountryName("km"));
	mCountries.insert(242, kl->twoAlphaToCountryName("cg"));
	mCountries.insert(682, kl->twoAlphaToCountryName("ck"));
	mCountries.insert(506, kl->twoAlphaToCountryName("cr"));
	mCountries.insert(385, kl->twoAlphaToCountryName("hr"));
	mCountries.insert(53, kl->twoAlphaToCountryName("cu"));
	mCountries.insert(357, kl->twoAlphaToCountryName("cy"));
	mCountries.insert(420, kl->twoAlphaToCountryName("cz"));
	mCountries.insert(45, kl->twoAlphaToCountryName("dk"));
	mCountries.insert(246, i18n("Diego Garcia"));
	mCountries.insert(253, kl->twoAlphaToCountryName("dj"));
	mCountries.insert(109, kl->twoAlphaToCountryName("dm"));
	mCountries.insert(110, kl->twoAlphaToCountryName("do"));
	mCountries.insert(593, kl->twoAlphaToCountryName("ec"));
	mCountries.insert(20, kl->twoAlphaToCountryName("eg"));
	mCountries.insert(503, kl->twoAlphaToCountryName("sv"));
	mCountries.insert(240, kl->twoAlphaToCountryName("gq"));
	mCountries.insert(291, kl->twoAlphaToCountryName("er"));
	mCountries.insert(372, kl->twoAlphaToCountryName("ee"));
	mCountries.insert(251, kl->twoAlphaToCountryName("et"));
	mCountries.insert(298, kl->twoAlphaToCountryName("fo"));
	mCountries.insert(500, kl->twoAlphaToCountryName("fk"));
	mCountries.insert(679, kl->twoAlphaToCountryName("fj"));
	mCountries.insert(358, kl->twoAlphaToCountryName("fi"));
	mCountries.insert(33, kl->twoAlphaToCountryName("fr"));
	mCountries.insert(5901, i18n("French Antilles"));
	mCountries.insert(594, kl->twoAlphaToCountryName("gf"));
	mCountries.insert(689, kl->twoAlphaToCountryName("pf"));
	mCountries.insert(241, kl->twoAlphaToCountryName("ga"));
	mCountries.insert(220, kl->twoAlphaToCountryName("gm"));
	mCountries.insert(995, kl->twoAlphaToCountryName("ge"));
	mCountries.insert(49, kl->twoAlphaToCountryName("de"));
	mCountries.insert(233, kl->twoAlphaToCountryName("gh"));
	mCountries.insert(350, kl->twoAlphaToCountryName("gi"));
	mCountries.insert(30, kl->twoAlphaToCountryName("gr"));
	mCountries.insert(299, kl->twoAlphaToCountryName("gl"));
	mCountries.insert(111, kl->twoAlphaToCountryName("gd"));
	mCountries.insert(590, kl->twoAlphaToCountryName("gp"));
	mCountries.insert(671, kl->twoAlphaToCountryName("gu"));
	mCountries.insert(5399, i18n("Guantanamo Bay"));
	mCountries.insert(502, kl->twoAlphaToCountryName("gt"));
	mCountries.insert(224, kl->twoAlphaToCountryName("gn"));
	mCountries.insert(245, kl->twoAlphaToCountryName("gw"));
	mCountries.insert(592, kl->twoAlphaToCountryName("gy"));
	mCountries.insert(509, kl->twoAlphaToCountryName("ht"));
	mCountries.insert(504, kl->twoAlphaToCountryName("hn"));
	mCountries.insert(852, kl->twoAlphaToCountryName("hk"));
	mCountries.insert(36, kl->twoAlphaToCountryName("hu"));
	mCountries.insert(871, i18n("INMARSAT (Atlantic-East)"));
	mCountries.insert(874, i18n("INMARSAT (Atlantic-West)"));
	mCountries.insert(873, i18n("INMARSAT (Indian)"));
	mCountries.insert(872, i18n("INMARSAT (Pacific)"));
	mCountries.insert(870, i18n("INMARSAT"));
	mCountries.insert(354, kl->twoAlphaToCountryName("is"));
	mCountries.insert(91, kl->twoAlphaToCountryName("in"));
	mCountries.insert(62, kl->twoAlphaToCountryName("id"));
	mCountries.insert(800, i18n("International Freephone Service"));
	mCountries.insert(98, kl->twoAlphaToCountryName("ir"));
	mCountries.insert(964, kl->twoAlphaToCountryName("iq"));
	mCountries.insert(353, kl->twoAlphaToCountryName("ie"));
	mCountries.insert(972, kl->twoAlphaToCountryName("il"));
	mCountries.insert(39, kl->twoAlphaToCountryName("it"));
	mCountries.insert(225, i18n("Ivory Coast"));
	mCountries.insert(112, kl->twoAlphaToCountryName("jm"));
	mCountries.insert(81, kl->twoAlphaToCountryName("jp"));
	mCountries.insert(962, kl->twoAlphaToCountryName("jo"));
	mCountries.insert(705, kl->twoAlphaToCountryName("kz"));
	mCountries.insert(254, kl->twoAlphaToCountryName("ke"));
	mCountries.insert(686, kl->twoAlphaToCountryName("ki"));
	mCountries.insert(850, kl->twoAlphaToCountryName("kp"));
	mCountries.insert(82, kl->twoAlphaToCountryName("kr"));
	mCountries.insert(965, kl->twoAlphaToCountryName("kw"));
	mCountries.insert(706, kl->twoAlphaToCountryName("kg"));
	mCountries.insert(856, kl->twoAlphaToCountryName("la"));
	mCountries.insert(371, kl->twoAlphaToCountryName("kv"));
	mCountries.insert(961, kl->twoAlphaToCountryName("kb"));
	mCountries.insert(266, kl->twoAlphaToCountryName("ls"));
	mCountries.insert(231, kl->twoAlphaToCountryName("lr"));
	mCountries.insert(218, kl->twoAlphaToCountryName("ly"));
	mCountries.insert(4101, kl->twoAlphaToCountryName("li"));
	mCountries.insert(370, kl->twoAlphaToCountryName("lt"));
	mCountries.insert(352, kl->twoAlphaToCountryName("lu"));
	mCountries.insert(853, kl->twoAlphaToCountryName("mo"));
	mCountries.insert(261, kl->twoAlphaToCountryName("mg"));
	mCountries.insert(265, kl->twoAlphaToCountryName("mw"));
	mCountries.insert(60, kl->twoAlphaToCountryName("my"));
	mCountries.insert(960, kl->twoAlphaToCountryName("mv"));
	mCountries.insert(223, kl->twoAlphaToCountryName("ml"));
	mCountries.insert(356, kl->twoAlphaToCountryName("mt"));
	mCountries.insert(692, kl->twoAlphaToCountryName("mh"));
	mCountries.insert(596, kl->twoAlphaToCountryName("mq"));
	mCountries.insert(222, kl->twoAlphaToCountryName("mr"));
	mCountries.insert(230, kl->twoAlphaToCountryName("mu"));
	mCountries.insert(269, kl->twoAlphaToCountryName("yt"));
	mCountries.insert(52, kl->twoAlphaToCountryName("mx"));
	mCountries.insert(691, kl->twoAlphaToCountryName("fm"));
	mCountries.insert(373, kl->twoAlphaToCountryName("md"));
	mCountries.insert(377, kl->twoAlphaToCountryName("mc"));
	mCountries.insert(976, kl->twoAlphaToCountryName("mn"));
	mCountries.insert(113, kl->twoAlphaToCountryName("ms"));
	mCountries.insert(212, kl->twoAlphaToCountryName("ma"));
	mCountries.insert(258, kl->twoAlphaToCountryName("mz"));
	mCountries.insert(95, kl->twoAlphaToCountryName("mm"));
	mCountries.insert(264, kl->twoAlphaToCountryName("na"));
	mCountries.insert(674, kl->twoAlphaToCountryName("nr"));
	mCountries.insert(977, kl->twoAlphaToCountryName("np"));
	mCountries.insert(599, kl->twoAlphaToCountryName("an"));
	mCountries.insert(31, kl->twoAlphaToCountryName("nl"));
	mCountries.insert(114, i18n("Nevis"));
	mCountries.insert(687, kl->twoAlphaToCountryName("nc"));
	mCountries.insert(64, kl->twoAlphaToCountryName("nz"));
	mCountries.insert(505, kl->twoAlphaToCountryName("ni"));
	mCountries.insert(227, kl->twoAlphaToCountryName("ne"));
	mCountries.insert(234, kl->twoAlphaToCountryName("ng"));
	mCountries.insert(683, kl->twoAlphaToCountryName("nu"));
	mCountries.insert(6722, kl->twoAlphaToCountryName("nf"));
	mCountries.insert(47, kl->twoAlphaToCountryName("no"));
	mCountries.insert(968, kl->twoAlphaToCountryName("om"));
	mCountries.insert(92, kl->twoAlphaToCountryName("pk"));
	mCountries.insert(680, kl->twoAlphaToCountryName("pw"));
	mCountries.insert(507, kl->twoAlphaToCountryName("pa"));
	mCountries.insert(675, kl->twoAlphaToCountryName("pg"));
	mCountries.insert(595, kl->twoAlphaToCountryName("py"));
	mCountries.insert(51, kl->twoAlphaToCountryName("pe"));
	mCountries.insert(63, kl->twoAlphaToCountryName("ph"));
	mCountries.insert(48, kl->twoAlphaToCountryName("pl"));
	mCountries.insert(351, kl->twoAlphaToCountryName("pt"));
	mCountries.insert(121, kl->twoAlphaToCountryName("pr"));
	mCountries.insert(974, kl->twoAlphaToCountryName("qa"));
	mCountries.insert(389, kl->twoAlphaToCountryName("mk"));
	mCountries.insert(262, i18n("Reunion Island"));
	mCountries.insert(40, kl->twoAlphaToCountryName("ro"));
	mCountries.insert(6701, i18n("Rota Island"));
	mCountries.insert(7, kl->twoAlphaToCountryName("ru"));
	mCountries.insert(250, kl->twoAlphaToCountryName("rw"));
	mCountries.insert(122, kl->twoAlphaToCountryName("lc"));
	mCountries.insert(670, i18n("Ivory Coast"));
	mCountries.insert(378, kl->twoAlphaToCountryName("sm"));
	mCountries.insert(239, kl->twoAlphaToCountryName("st"));
	mCountries.insert(966, kl->twoAlphaToCountryName("sa"));
	mCountries.insert(221, kl->twoAlphaToCountryName("sn"));
	mCountries.insert(248, kl->twoAlphaToCountryName("sc"));
	mCountries.insert(232, kl->twoAlphaToCountryName("sl"));
	mCountries.insert(65, kl->twoAlphaToCountryName("sg"));
	mCountries.insert(421, kl->twoAlphaToCountryName("sk"));
	mCountries.insert(386, kl->twoAlphaToCountryName("si"));
	mCountries.insert(677, kl->twoAlphaToCountryName("sb"));
	mCountries.insert(252, kl->twoAlphaToCountryName("so"));
	mCountries.insert(27, kl->twoAlphaToCountryName("za"));
	mCountries.insert(34, kl->twoAlphaToCountryName("es"));
	mCountries.insert(94, kl->twoAlphaToCountryName("lk"));
	mCountries.insert(290, kl->twoAlphaToCountryName("sh"));
	mCountries.insert(115, kl->twoAlphaToCountryName("kn"));
	mCountries.insert(508, kl->twoAlphaToCountryName("pm"));
	mCountries.insert(116, kl->twoAlphaToCountryName("vc"));
	mCountries.insert(249, kl->twoAlphaToCountryName("sd"));
	mCountries.insert(597, kl->twoAlphaToCountryName("sr"));
	mCountries.insert(268, kl->twoAlphaToCountryName("sz"));
	mCountries.insert(46, kl->twoAlphaToCountryName("se"));
	mCountries.insert(41, kl->twoAlphaToCountryName("ch"));
	mCountries.insert(963, kl->twoAlphaToCountryName("sy"));
	mCountries.insert(886, kl->twoAlphaToCountryName("tw"));
	mCountries.insert(708, kl->twoAlphaToCountryName("tj"));
	mCountries.insert(255, kl->twoAlphaToCountryName("tz"));
	mCountries.insert(66, kl->twoAlphaToCountryName("th"));
	mCountries.insert(6702, i18n("Tinian Island"));
	mCountries.insert(228, kl->twoAlphaToCountryName("tg")); // Togo
	mCountries.insert(690, kl->twoAlphaToCountryName("tk")); // Tokelau
	mCountries.insert(676, kl->twoAlphaToCountryName("to")); // Tonga
	mCountries.insert(117, kl->twoAlphaToCountryName("tt")); // Trinidad and Tobago
	mCountries.insert(216, kl->twoAlphaToCountryName("tn")); // Tunisia
	mCountries.insert(90, kl->twoAlphaToCountryName("tr"));
	mCountries.insert(709, kl->twoAlphaToCountryName("tm"));
	mCountries.insert(118, kl->twoAlphaToCountryName("tc")); // Turks and Caicos Island
	mCountries.insert(688, kl->twoAlphaToCountryName("tv")); // Tuvalu
	mCountries.insert(1, kl->twoAlphaToCountryName("us")); // United States of America
	mCountries.insert(256, kl->twoAlphaToCountryName("ug")); // Uganda
	mCountries.insert(380, kl->twoAlphaToCountryName("ua")); // Ukraine
	mCountries.insert(971, kl->twoAlphaToCountryName("ae")); // United Arab Emirates
	mCountries.insert(44, kl->twoAlphaToCountryName("gb")); // United Kingdom
	mCountries.insert(123, kl->twoAlphaToCountryName("vi")); // United States Virgin Islands
	mCountries.insert(598, kl->twoAlphaToCountryName("uy")); // Uruguay
	mCountries.insert(711, kl->twoAlphaToCountryName("uz")); // Uzbekistan
	mCountries.insert(678, kl->twoAlphaToCountryName("vu")); // Vanuatu
	mCountries.insert(379, kl->twoAlphaToCountryName("va")); // Vatican City
	mCountries.insert(58, kl->twoAlphaToCountryName("ve")); // Venezuela
	mCountries.insert(84, kl->twoAlphaToCountryName("vn")); // Vietnam
	mCountries.insert(681, kl->twoAlphaToCountryName("wf")); // Wallis and Futuna Islands
	mCountries.insert(685, kl->twoAlphaToCountryName("eh"));
	mCountries.insert(967, kl->twoAlphaToCountryName("ye"));
	mCountries.insert(381, kl->twoAlphaToCountryName("yu"));
	mCountries.insert(243, kl->twoAlphaToCountryName("zr"));
	mCountries.insert(260, kl->twoAlphaToCountryName("zm"));
	mCountries.insert(263, kl->twoAlphaToCountryName("zw"));
}

void ICQProtocol::initLang()
{

	KLocale *kl = KGlobal::locale(); //KLocale(QString::fromLatin1("kopete"));

	mLanguages.insert(0 , "");
	mLanguages.insert(1 , kl->twoAlphaToLanguageName("ar") /*i18n("Arabic")*/);
	mLanguages.insert(2 , i18n("Bhojpuri"));
	mLanguages.insert(3 , kl->twoAlphaToLanguageName("bg") /*i18n("Bulgarian")*/);
	mLanguages.insert(4 , kl->twoAlphaToLanguageName("my") /*i18n("Burmese")*/);
	mLanguages.insert(5 , i18n("Cantonese"));
	mLanguages.insert(6 , kl->twoAlphaToLanguageName("ca") /*i18n("Catalan")*/);
	mLanguages.insert(7 , kl->twoAlphaToLanguageName("zh") /*i18n("Chinese")*/);
	mLanguages.insert(8 , kl->twoAlphaToLanguageName("hr") /*i18n("Croatian")*/);
	mLanguages.insert(9 , kl->twoAlphaToLanguageName("cs") /*i18n("Czech")*/);
	mLanguages.insert(10, kl->twoAlphaToLanguageName("da") /*i18n("Danish")*/);
	mLanguages.insert(11, kl->twoAlphaToLanguageName("nl") /*i18n("Dutch")*/);
	mLanguages.insert(12, kl->twoAlphaToLanguageName("en") /*i18n("English")*/);
	mLanguages.insert(13, kl->twoAlphaToLanguageName("eo") /*i18n("Esperanto")*/);
	mLanguages.insert(14, kl->twoAlphaToLanguageName("et") /*i18n("Estonian")*/);
	mLanguages.insert(15, i18n("Farsi"));
	mLanguages.insert(16, kl->twoAlphaToLanguageName("fi") /*i18n("Finnish")*/);
	mLanguages.insert(17, kl->twoAlphaToLanguageName("fr") /*i18n("French")*/);
	mLanguages.insert(18, kl->twoAlphaToLanguageName("gd") /*i18n("Gaelic")*/);
	mLanguages.insert(19, kl->twoAlphaToLanguageName("de") /*i18n("German")*/);
	mLanguages.insert(20, kl->twoAlphaToLanguageName("el") /*i18n("Greek")*/);
	mLanguages.insert(21, kl->twoAlphaToLanguageName("he") /*i18n("Hebrew")*/);
	mLanguages.insert(22, kl->twoAlphaToLanguageName("hi") /*i18n("Hindi")*/);
	mLanguages.insert(23, kl->twoAlphaToLanguageName("hu") /*i18n("Hungarian")*/);
	mLanguages.insert(24, kl->twoAlphaToLanguageName("is") /*i18n("Icelandic")*/);
	mLanguages.insert(25, kl->twoAlphaToLanguageName("id") /*i18n("Indonesian")*/);
	mLanguages.insert(26, kl->twoAlphaToLanguageName("it") /*i18n("Italian")*/);
	mLanguages.insert(27, kl->twoAlphaToLanguageName("ja") /*i18n("Japanese")*/);
	mLanguages.insert(28, kl->twoAlphaToLanguageName("km") /*i18n("Khmer")*/);
	mLanguages.insert(29, kl->twoAlphaToLanguageName("ko") /*i18n("Korean")*/);
	mLanguages.insert(30, kl->twoAlphaToLanguageName("lo") /*i18n("Lao")*/);
	mLanguages.insert(31, kl->twoAlphaToLanguageName("lv") /*i18n("Latvian")*/);
	mLanguages.insert(32, kl->twoAlphaToLanguageName("lt") /*i18n("Lithuanian")*/);
	mLanguages.insert(33, kl->twoAlphaToLanguageName("ms") /*i18n("Malay")*/);
	mLanguages.insert(34, kl->twoAlphaToLanguageName("no") /*i18n("Norwegian")*/);
	mLanguages.insert(35, kl->twoAlphaToLanguageName("pl") /*i18n("Polish")*/);
	mLanguages.insert(36, kl->twoAlphaToLanguageName("pt") /*i18n("Portuguese")*/);
	mLanguages.insert(37, kl->twoAlphaToLanguageName("ro") /*i18n("Romanian")*/);
	mLanguages.insert(38, kl->twoAlphaToLanguageName("ru") /*i18n("Russian")*/);
	mLanguages.insert(39, kl->twoAlphaToLanguageName("sr") /*i18n("Serbian")*/);
	mLanguages.insert(40, kl->twoAlphaToLanguageName("sk") /*i18n("Slovak")*/);
	mLanguages.insert(41, kl->twoAlphaToLanguageName("sl") /*i18n("Slovenian")*/);
	mLanguages.insert(42, kl->twoAlphaToLanguageName("so") /*i18n("Somali")*/);
	mLanguages.insert(43, kl->twoAlphaToLanguageName("es") /*i18n("Spanish")*/);
	mLanguages.insert(44, kl->twoAlphaToLanguageName("sw") /*i18n("Swahili")*/);
	mLanguages.insert(45, kl->twoAlphaToLanguageName("sv") /*i18n("Swedish")*/);
	mLanguages.insert(46, kl->twoAlphaToLanguageName("tl") /*i18n("Tagalog")*/);
	mLanguages.insert(47, kl->twoAlphaToLanguageName("tt") /*i18n("Tatar")*/);
	mLanguages.insert(48, kl->twoAlphaToLanguageName("th") /*i18n("Thai")*/);
	mLanguages.insert(49, kl->twoAlphaToLanguageName("tr") /*i18n("Turkish")*/);
	mLanguages.insert(50, kl->twoAlphaToLanguageName("uk") /*i18n("Ukrainian")*/);
	mLanguages.insert(51, kl->twoAlphaToLanguageName("ur") /*i18n("Urdu")*/);
	mLanguages.insert(52, kl->twoAlphaToLanguageName("vi") /*i18n("Vietnamese")*/);
	mLanguages.insert(53, kl->twoAlphaToLanguageName("yi") /*i18n("Yiddish")*/);
	mLanguages.insert(54, kl->twoAlphaToLanguageName("yo") /*i18n("Yoruba")*/);
	mLanguages.insert(55, i18n("Taiwanese"));
	mLanguages.insert(56, kl->twoAlphaToLanguageName("af") /*i18n("Afrikaans")*/);
	mLanguages.insert(57, kl->twoAlphaToLanguageName("fa") /*i18n("Persian")*/);
	mLanguages.insert(58, kl->twoAlphaToLanguageName("sq") /*i18n("Albanian")*/);
	mLanguages.insert(59, kl->twoAlphaToLanguageName("hy") /*i18n("Armenian")*/);
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
	kdDebug(14200) << k_funcinfo << "tz=" << int(tz) << endl;
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
			widget->prsEmailLabel->setText(i18n("Unspecified"));
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
	kdDebug(14200) << k_funcinfo << "timezonecode=" <<
		c->generalInfo.timezoneCode << endl;
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

ICQProtocol *ICQProtocol::protocol()
{
	return protocolStatic_;
}

bool ICQProtocol::canSendOffline() const
{
	return true;
}

void ICQProtocol::deserializeContact(KopeteMetaContact *metaContact,
	const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> &/*addressBookData*/)
{
	QString accountId = serializedData["accountId"];
	QDict<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts(this);
	ICQAccount *account = static_cast<ICQAccount*>(accounts[accountId]);

	if(!account)
	{
		kdDebug(14200) << k_funcinfo <<
			"WARNING: Account for contact does not exist, skipping." << endl;
		return;
	}

	QString displayName=serializedData["displayName"];
	QString contactId=serializedData["contactId"];
	ICQContact *c = new ICQContact(contactId, displayName, account, metaContact);
	c->setGroupId(serializedData["groupID"].toInt());
	c->setEncoding(serializedData["Encoding"].toInt());
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
