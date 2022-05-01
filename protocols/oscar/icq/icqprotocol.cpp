/*
  icqprotocol.cpp  -  ICQ Protocol Plugin

  Copyright (c) 2003      by Olivier Goffart        <ogoffart@kde.org>
  Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>

  Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

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

#include <qcombobox.h>
#include <kdialog.h>

#include <kpluginfactory.h>
#include <KLocalizedString>
#include <kconfig.h>
#include <kmessagebox.h>

#include "kopeteglobal.h"
#include "kopeteuiglobal.h"
#include "accountselector.h"
#include "kopeteaccountmanager.h"

#include "icqaccount.h"
#include "icqaddcontactpage.h"
#include "icqeditaccountwidget.h"

#include "icqstatusmanager.h"

K_PLUGIN_FACTORY( ICQProtocolFactory, registerPlugin<ICQProtocol>(); )

QString languageCodeToName(const char* code)
{
	return i18n(QLocale::languageToString(QLocale(code).language()).toLatin1());
}

QString countryCodeToName(const char* code)
{
	return i18n(QLocale::countryToString(QLocale(code).country()).toLatin1());
}

//BEGIN class ICQProtocolHandler

ICQProtocolHandler::ICQProtocolHandler() : Kopete::MimeTypeHandler(false)
{
	registerAsMimeHandler(QString::fromLatin1("application/x-icq"));
}

void ICQProtocolHandler::handleURL(const QString &mimeType, const QUrl & url) const
{
	if (mimeType != "application/x-icq")
		return;

	/**
	 * File Format usually looks like
	 *
	 * [ICQ User]
	 * UIN=123456789
	 * Email=
	 * NickName=
	 * FirstName=
	 * LastName=
	 */

	KConfig file(url.toLocalFile(), KConfig::SimpleConfig);
	QString group_name;

	if (file.hasGroup("ICQ User"))
		group_name= "ICQ User";
	else if (file.hasGroup("ICQ Message User"))
		group_name = "ICQ Message User";
	else
		return;

	KConfigGroup group = file.group(group_name);
	ICQProtocol *proto = ICQProtocol::protocol();

	QString uin = group.readEntry("UIN");
	if (uin.isEmpty())
		return;

	QString nick = group.readEntry("NickName");
	QString first = group.readEntry("FirstName");
	QString last = group.readEntry("LastName");
	QString email = group.readEntry("Email");

	Kopete::Account *account = 0;
	QList<Kopete::Account*> accounts = Kopete::AccountManager::self()->accounts(proto);
	// do not show chooser if we only have one account to "choose" from
	if (accounts.count() == 1)
	{
		account = accounts.first();
	}
	else
	{
		KDialog *chooser = new KDialog;
		chooser->setCaption( i18n("Choose Account") );
		chooser->setButtons( KDialog::Ok|KDialog::Cancel );
		chooser->setDefaultButton(KDialog::Ok);
		AccountSelector *accSelector = new AccountSelector(proto, chooser);
		accSelector->setObjectName( QLatin1String("accSelector") );
		chooser->setMainWidget(accSelector);

		int ret = chooser->exec();
		account = accSelector->selectedItem();

		delete chooser;
		if (ret == QDialog::Rejected || account == 0)
		{
			kDebug(14153) << "Cancelled";
			return;
		}
	}

	if (!account->isConnected())
	{
		kDebug(14153) << "Can't add contact, we are offline!";
		KMessageBox::sorry( Kopete::UI::Global::mainWidget(), i18n("You must be online to add a contact."), i18n("ICQ") );
		return;
	}

	QString nickuin = nick.isEmpty() ?
		i18n("'%1'", uin) :
		i18n("'%1' (%2)", nick, uin);

	if (KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget(),
	                               i18n("Do you want to add %1 to your contact list?", nickuin), QString(),
	                               KGuiItem( i18n("Add") ), KGuiItem( i18n("Do Not Add") ))
	    != KMessageBox::Yes)
	{
		kDebug(14153) << "Cancelled";
		return;
	}

	kDebug(14153) <<
		"Adding Contact; uin = " << uin << ", nick = '" << nick <<
		"', firstname = '" << first << "', lastname = '" << last <<"'" << endl;
	if (account->addContact(uin, nick, 0L, Kopete::Account::Temporary))
	{
		Kopete::Contact *contact = account->contacts().value( uin );
		if (!first.isEmpty())
			contact->setProperty(Kopete::Global::Properties::self()->firstName(), first);
		if (!last.isEmpty())
			contact->setProperty(Kopete::Global::Properties::self()->lastName(), last);
		if (!email.isEmpty())
			contact->setProperty(Kopete::Global::Properties::self()->emailAddress(), email);
	}
}

//END class ICQProtocolHandler

//BEGIN class ICQProtocol

ICQProtocol* ICQProtocol::protocolStatic_ = nullptr;

ICQProtocol::ICQProtocol(QObject *parent, const QVariantList&)
: OscarProtocol( parent ),
	firstName(Kopete::Global::Properties::self()->firstName()),
	lastName(Kopete::Global::Properties::self()->lastName()),
	emailAddress(Kopete::Global::Properties::self()->emailAddress()),
	ipAddress("ipAddress", i18n("IP Address") )
{
	if (protocolStatic_)
		kWarning(14153) << "ICQ plugin already initialized";
	else
		protocolStatic_ = this;

	// must be done after protocolStatic_ is set...
	statusManager_ = new ICQStatusManager;

	setCapabilities( Kopete::Protocol::FullRTF ); // setting capabilities
	kDebug(14153) << "capabilities set to FullRTF";
	addAddressBookField("messaging/icq", Kopete::Plugin::MakeIndexField);

	initGenders();
	initLang();
	initCountries();
	initEncodings();
	initMaritals();
	initInterests();
	initOccupations();
	initOrganizations();
	initAffiliations();
}

ICQProtocol::~ICQProtocol()
{
	delete statusManager_;
	protocolStatic_ =0L;
}

void ICQProtocol::initGenders()
{
	mGenders.insert(0, ""); // unspecified
	mGenders.insert(1, i18n("Female"));
	mGenders.insert(2, i18n("Male"));
}

void ICQProtocol::initCountries()
{
	mCountries.insert(0, ""); // unspecified
	mCountries.insert(93, countryCodeToName("af"));
	mCountries.insert(355, countryCodeToName("al"));
	mCountries.insert(213, countryCodeToName("dz"));
	mCountries.insert(684, countryCodeToName("as"));
	mCountries.insert(376, countryCodeToName("ad"));
	mCountries.insert(244, countryCodeToName("ao"));
	mCountries.insert(101, countryCodeToName("ai"));
	mCountries.insert(102, i18n("Antigua"));
	mCountries.insert(1021, countryCodeToName("ag"));
	mCountries.insert(54, countryCodeToName("ar"));
	mCountries.insert(374, countryCodeToName("am"));
	mCountries.insert(297, countryCodeToName("aw"));
	mCountries.insert(247, i18n("Ascension Island"));
	mCountries.insert(61, countryCodeToName("au"));
	mCountries.insert(43, countryCodeToName("at"));
	mCountries.insert(994, countryCodeToName("az"));
	mCountries.insert(103, countryCodeToName("bs"));
	mCountries.insert(973, countryCodeToName("bh"));
	mCountries.insert(880, countryCodeToName("bd"));
	mCountries.insert(104, countryCodeToName("bb"));
	mCountries.insert(120, i18n("Barbuda"));
	mCountries.insert(375, countryCodeToName("by"));
	mCountries.insert(32, countryCodeToName("be"));
	mCountries.insert(501, countryCodeToName("bz"));
	mCountries.insert(229, countryCodeToName("bj"));
	mCountries.insert(105, countryCodeToName("bm"));
	mCountries.insert(975, countryCodeToName("bt"));
	mCountries.insert(591, countryCodeToName("bo"));
	mCountries.insert(387, countryCodeToName("ba"));
	mCountries.insert(267, countryCodeToName("bw"));
	mCountries.insert(55, countryCodeToName("br"));
	mCountries.insert(106, i18n("British Virgin Islands"));
	mCountries.insert(673, countryCodeToName("bn"));
	mCountries.insert(359, countryCodeToName("bg"));
	mCountries.insert(226, countryCodeToName("bf"));
	mCountries.insert(257, countryCodeToName("bi"));
	mCountries.insert(855, countryCodeToName("kh"));
	mCountries.insert(237, countryCodeToName("cm"));
	mCountries.insert(107, countryCodeToName("ca"));
	mCountries.insert(238, countryCodeToName("cv"));
	mCountries.insert(108, countryCodeToName("ky"));
	mCountries.insert(236, countryCodeToName("cf"));
	mCountries.insert(235, countryCodeToName("td"));
	mCountries.insert(178, i18n("Canary Islands"));
	mCountries.insert(56, countryCodeToName("cl"));
	mCountries.insert(86, countryCodeToName("cn"));
	mCountries.insert(672, countryCodeToName("cx"));
	mCountries.insert(6101, i18n("Cocos-Keeling Islands"));
	mCountries.insert(6102, i18n("Cocos (Keeling) Islands"));
	mCountries.insert(57, countryCodeToName("co"));
	mCountries.insert(2691, countryCodeToName("km"));
	mCountries.insert(242, countryCodeToName("cg"));
	mCountries.insert(682, countryCodeToName("ck"));
	mCountries.insert(506, countryCodeToName("cr"));
	mCountries.insert(385, countryCodeToName("hr"));
	mCountries.insert(53, countryCodeToName("cu"));
	mCountries.insert(357, countryCodeToName("cy"));
	mCountries.insert(42, countryCodeToName("cz"));
	mCountries.insert(45, countryCodeToName("dk"));
	mCountries.insert(246, i18n("Diego Garcia"));
	mCountries.insert(253, countryCodeToName("dj"));
	mCountries.insert(109, countryCodeToName("dm"));
	mCountries.insert(110, countryCodeToName("do"));
	mCountries.insert(593, countryCodeToName("ec"));
	mCountries.insert(20, countryCodeToName("eg"));
	mCountries.insert(503, countryCodeToName("sv"));
	mCountries.insert(240, countryCodeToName("gq"));
	mCountries.insert(291, countryCodeToName("er"));
	mCountries.insert(372, countryCodeToName("ee"));
	mCountries.insert(251, countryCodeToName("et"));
	mCountries.insert(298, countryCodeToName("fo"));
	mCountries.insert(500, countryCodeToName("fk"));
	mCountries.insert(679, countryCodeToName("fj"));
	mCountries.insert(358, countryCodeToName("fi"));
	mCountries.insert(33, countryCodeToName("fr"));
	mCountries.insert(5901, i18n("French Antilles"));
	mCountries.insert(5902, i18n("Antilles"));
	mCountries.insert(594, i18n("French Guiana"));
	mCountries.insert(689, countryCodeToName("pf"));
	mCountries.insert(241, countryCodeToName("ga"));
	mCountries.insert(220, countryCodeToName("gm"));
	mCountries.insert(995, countryCodeToName("ge"));
	mCountries.insert(49, countryCodeToName("de"));
	mCountries.insert(233, countryCodeToName("gh"));
	mCountries.insert(350, countryCodeToName("gi"));
	mCountries.insert(30, countryCodeToName("gr"));
	mCountries.insert(299, countryCodeToName("gl"));
	mCountries.insert(111, countryCodeToName("gd"));
	mCountries.insert(590, countryCodeToName("gp"));
	mCountries.insert(671, countryCodeToName("gu"));
	mCountries.insert(502, countryCodeToName("gt"));
	mCountries.insert(224, countryCodeToName("gn"));
	mCountries.insert(245, countryCodeToName("gw"));
	mCountries.insert(592, countryCodeToName("gy"));
	mCountries.insert(509, countryCodeToName("ht"));
	mCountries.insert(504, countryCodeToName("hn"));
	mCountries.insert(852, countryCodeToName("hk"));
	mCountries.insert(36, countryCodeToName("hu"));
	mCountries.insert(354, countryCodeToName("is"));
	mCountries.insert(91, countryCodeToName("in"));
	mCountries.insert(62, countryCodeToName("id"));
	mCountries.insert(98, countryCodeToName("ir"));
	mCountries.insert(964, countryCodeToName("iq"));
	mCountries.insert(353, countryCodeToName("ie"));
	mCountries.insert(972, countryCodeToName("il"));
	mCountries.insert(39, countryCodeToName("it"));
	mCountries.insert(225, countryCodeToName("ci"));
	mCountries.insert(112, countryCodeToName("jm"));
	mCountries.insert(81, countryCodeToName("jp"));
	mCountries.insert(962, countryCodeToName("jo"));
	mCountries.insert(705, countryCodeToName("kz"));
	mCountries.insert(254, countryCodeToName("ke"));
	mCountries.insert(686, countryCodeToName("ki"));
	mCountries.insert(850, countryCodeToName("kp"));
	mCountries.insert(82, countryCodeToName("kr"));
	mCountries.insert(965, countryCodeToName("kw"));
	mCountries.insert(706, countryCodeToName("kg"));
	mCountries.insert(856, countryCodeToName("la"));
	mCountries.insert(371, countryCodeToName("lv"));
	mCountries.insert(961, countryCodeToName("lb"));
	mCountries.insert(266, countryCodeToName("ls"));
	mCountries.insert(231, countryCodeToName("lr"));
	mCountries.insert(218, countryCodeToName("ly"));
	mCountries.insert(4101, countryCodeToName("li"));
	mCountries.insert(370, countryCodeToName("lt"));
	mCountries.insert(352, countryCodeToName("lu"));
	mCountries.insert(853, countryCodeToName("mo"));
	mCountries.insert(261, countryCodeToName("mg"));
	mCountries.insert(265, countryCodeToName("mw"));
	mCountries.insert(60, countryCodeToName("my"));
	mCountries.insert(960, countryCodeToName("mv"));
	mCountries.insert(223, countryCodeToName("ml"));
	mCountries.insert(356, countryCodeToName("mt"));
	mCountries.insert(692, countryCodeToName("mh"));
	mCountries.insert(596, countryCodeToName("mq"));
	mCountries.insert(222, countryCodeToName("mr"));
	mCountries.insert(230, countryCodeToName("mu"));
	mCountries.insert(269, i18n("Mayotte Island"));
	mCountries.insert(52, countryCodeToName("mx"));
	mCountries.insert(691, countryCodeToName("fm"));
	mCountries.insert(373, countryCodeToName("md"));
	mCountries.insert(377, countryCodeToName("mc"));
	mCountries.insert(976, countryCodeToName("mn"));
	mCountries.insert(113, countryCodeToName("ms"));
	mCountries.insert(212, countryCodeToName("ma"));
	mCountries.insert(258, countryCodeToName("mz"));
	mCountries.insert(95, countryCodeToName("mm"));
	mCountries.insert(264, countryCodeToName("na"));
	mCountries.insert(674, countryCodeToName("nr"));
	mCountries.insert(977, countryCodeToName("np"));
	mCountries.insert(599, countryCodeToName("an"));
	mCountries.insert(31, countryCodeToName("nl"));
	mCountries.insert(114, i18n("Nevis"));
	mCountries.insert(687, countryCodeToName("nc"));
	mCountries.insert(64, countryCodeToName("nz"));
	mCountries.insert(505, countryCodeToName("ni"));
	mCountries.insert(227, countryCodeToName("ne"));
	mCountries.insert(234, countryCodeToName("ng"));
	mCountries.insert(683, countryCodeToName("nu"));
	mCountries.insert(6722, countryCodeToName("nf"));
	mCountries.insert(47, countryCodeToName("no"));
	mCountries.insert(968, countryCodeToName("om"));
	mCountries.insert(92, countryCodeToName("pk"));
	mCountries.insert(680, countryCodeToName("pw"));
	mCountries.insert(507, countryCodeToName("pa"));
	mCountries.insert(675, countryCodeToName("pg"));
	mCountries.insert(595, countryCodeToName("py"));
	mCountries.insert(51, countryCodeToName("pe"));
	mCountries.insert(63, countryCodeToName("ph"));
	mCountries.insert(48, countryCodeToName("pl"));
	mCountries.insert(351, countryCodeToName("pt"));
	mCountries.insert(121, countryCodeToName("pr"));
	mCountries.insert(974, countryCodeToName("qa"));
	mCountries.insert(389, countryCodeToName("mk"));
	mCountries.insert(262, i18n("Reunion Island"));
	mCountries.insert(40, countryCodeToName("ro"));
	mCountries.insert(6701, i18n("Rota Island"));
	mCountries.insert(7, countryCodeToName("ru"));
	mCountries.insert(250, countryCodeToName("rw"));
	mCountries.insert(122, countryCodeToName("lc"));
	mCountries.insert(670, i18n("Saipan Island"));
	mCountries.insert(378, countryCodeToName("sm"));
	mCountries.insert(239, countryCodeToName("st"));
	mCountries.insert(966, countryCodeToName("sa"));
	mCountries.insert(221, countryCodeToName("sn"));
	mCountries.insert(248, countryCodeToName("sc"));
	mCountries.insert(232, i18n("Sierra Leone"));
	mCountries.insert(65, countryCodeToName("sg"));
	mCountries.insert(4201, countryCodeToName("sk"));
	mCountries.insert(386, countryCodeToName("si"));
	mCountries.insert(677, countryCodeToName("sb"));
	mCountries.insert(252, countryCodeToName("so"));
	mCountries.insert(27, countryCodeToName("za"));
	mCountries.insert(34, countryCodeToName("es"));
	mCountries.insert(94, countryCodeToName("lk"));
	mCountries.insert(290, countryCodeToName("sh"));
	mCountries.insert(115, i18n("St. Kitts"));
	mCountries.insert(1141, countryCodeToName("kn"));
	mCountries.insert(508, countryCodeToName("pm"));
	mCountries.insert(116, countryCodeToName("vc"));
	mCountries.insert(249, countryCodeToName("sd"));
	mCountries.insert(597, countryCodeToName("sr"));
	mCountries.insert(268, countryCodeToName("sz"));
	mCountries.insert(46, countryCodeToName("se"));
	mCountries.insert(41, countryCodeToName("ch"));
	mCountries.insert(963, countryCodeToName("sy"));
	mCountries.insert(886, countryCodeToName("tw"));
	mCountries.insert(708, countryCodeToName("tj"));
	mCountries.insert(255, countryCodeToName("tz"));
	mCountries.insert(66, countryCodeToName("th"));
	mCountries.insert(6702, i18n("Tinian Island"));
	mCountries.insert(228, countryCodeToName("tg")); // Togo
	mCountries.insert(690, countryCodeToName("tk")); // Tokelau
	mCountries.insert(676, countryCodeToName("to")); // Tonga
	mCountries.insert(117, countryCodeToName("tt")); // Trinidad and Tobago
	mCountries.insert(216, countryCodeToName("tn")); // Tunisia
	mCountries.insert(90, countryCodeToName("tr"));
	mCountries.insert(709, countryCodeToName("tm"));
	mCountries.insert(118, countryCodeToName("tc")); // Turks and Caicos Island
	mCountries.insert(688, countryCodeToName("tv")); // Tuvalu
	mCountries.insert(1, countryCodeToName("us")); // United States of America
	mCountries.insert(256, countryCodeToName("ug")); // Uganda
	mCountries.insert(380, countryCodeToName("ua")); // Ukraine
	mCountries.insert(971, countryCodeToName("ae")); // United Arab Emirates
	mCountries.insert(44, countryCodeToName("gb")); // United Kingdom
	mCountries.insert(441, i18n("Wales"));
	mCountries.insert(442, i18n("Scotland"));
	mCountries.insert(123, countryCodeToName("vi")); // United States Virgin Islands
	mCountries.insert(598, countryCodeToName("uy")); // Uruguay
	mCountries.insert(711, countryCodeToName("uz")); // Uzbekistan
	mCountries.insert(678, countryCodeToName("vu")); // Vanuatu
	mCountries.insert(379, countryCodeToName("va")); // Vatican City
	mCountries.insert(58, countryCodeToName("ve")); // Venezuela
	mCountries.insert(84, countryCodeToName("vn")); // Vietnam
	mCountries.insert(681, countryCodeToName("wf")); // Wallis and Futuna Islands
	mCountries.insert(685, countryCodeToName("ws"));
	mCountries.insert(967, countryCodeToName("ye"));
	mCountries.insert(3811, i18n("Yugoslavia - Serbia"));
	mCountries.insert(382, i18n("Yugoslavia - Montenegro"));
	mCountries.insert(381, i18n("Yugoslavia"));
	mCountries.insert(243, i18n("Congo, Democratic Republic of (Zaire)"));
	mCountries.insert(260, countryCodeToName("zm"));
	mCountries.insert(263, countryCodeToName("zw"));
	mCountries.insert(9999, i18n("Unknown"));

}

void ICQProtocol::initLang()
{
	mLanguages.insert(0 , "");
	mLanguages.insert(1 , languageCodeToName("ar") /*i18n("Arabic")*/);
	mLanguages.insert(2 , i18n("Bhojpuri"));
	mLanguages.insert(3 , languageCodeToName("bg") /*i18n("Bulgarian")*/);
	mLanguages.insert(4 , languageCodeToName("my") /*i18n("Burmese")*/);
	mLanguages.insert(5 , i18n("Cantonese"));
	mLanguages.insert(6 , languageCodeToName("ca") /*i18n("Catalan")*/);
	mLanguages.insert(7 , languageCodeToName("zh") /*i18n("Chinese")*/);
	mLanguages.insert(8 , languageCodeToName("hr") /*i18n("Croatian")*/);
	mLanguages.insert(9 , languageCodeToName("cs") /*i18n("Czech")*/);
	mLanguages.insert(10, languageCodeToName("da") /*i18n("Danish")*/);
	mLanguages.insert(11, languageCodeToName("nl") /*i18n("Dutch")*/);
	mLanguages.insert(12, languageCodeToName("en") /*i18n("English")*/);
	mLanguages.insert(13, languageCodeToName("eo") /*i18n("Esperanto")*/);
	mLanguages.insert(14, languageCodeToName("et") /*i18n("Estonian")*/);
	mLanguages.insert(15, i18n("Farsi"));
	mLanguages.insert(16, languageCodeToName("fi") /*i18n("Finnish")*/);
	mLanguages.insert(17, languageCodeToName("fr") /*i18n("French")*/);
	mLanguages.insert(18, languageCodeToName("gd") /*i18n("Gaelic")*/);
	mLanguages.insert(19, languageCodeToName("de") /*i18n("German")*/);
	mLanguages.insert(20, languageCodeToName("el") /*i18n("Greek")*/);
	mLanguages.insert(21, languageCodeToName("he") /*i18n("Hebrew")*/);
	mLanguages.insert(22, languageCodeToName("hi") /*i18n("Hindi")*/);
	mLanguages.insert(23, languageCodeToName("hu") /*i18n("Hungarian")*/);
	mLanguages.insert(24, languageCodeToName("is") /*i18n("Icelandic")*/);
	mLanguages.insert(25, languageCodeToName("id") /*i18n("Indonesian")*/);
	mLanguages.insert(26, languageCodeToName("it") /*i18n("Italian")*/);
	mLanguages.insert(27, languageCodeToName("ja") /*i18n("Japanese")*/);
	mLanguages.insert(28, languageCodeToName("km") /*i18n("Khmer")*/);
	mLanguages.insert(29, languageCodeToName("ko") /*i18n("Korean")*/);
	mLanguages.insert(30, languageCodeToName("lo") /*i18n("Lao")*/);
	mLanguages.insert(31, languageCodeToName("lv") /*i18n("Latvian")*/);
	mLanguages.insert(32, languageCodeToName("lt") /*i18n("Lithuanian")*/);
	mLanguages.insert(33, languageCodeToName("ms") /*i18n("Malay")*/);
	mLanguages.insert(34, languageCodeToName("no") /*i18n("Norwegian")*/);
	mLanguages.insert(35, languageCodeToName("pl") /*i18n("Polish")*/);
	mLanguages.insert(36, languageCodeToName("pt") /*i18n("Portuguese")*/);
	mLanguages.insert(37, languageCodeToName("ro") /*i18n("Romanian")*/);
	mLanguages.insert(38, languageCodeToName("ru") /*i18n("Russian")*/);
	mLanguages.insert(39, languageCodeToName("sr") /*i18n("Serbian")*/);
	mLanguages.insert(40, languageCodeToName("sk") /*i18n("Slovak")*/);
	mLanguages.insert(41, languageCodeToName("sl") /*i18n("Slovenian")*/);
	mLanguages.insert(42, languageCodeToName("so") /*i18n("Somali")*/);
	mLanguages.insert(43, languageCodeToName("es") /*i18n("Spanish")*/);
	mLanguages.insert(44, languageCodeToName("sw") /*i18n("Swahili")*/);
	mLanguages.insert(45, languageCodeToName("sv") /*i18n("Swedish")*/);
	mLanguages.insert(46, languageCodeToName("tl") /*i18n("Tagalog")*/);
	mLanguages.insert(47, languageCodeToName("tt") /*i18n("Tatar")*/);
	mLanguages.insert(48, languageCodeToName("th") /*i18n("Thai")*/);
	mLanguages.insert(49, languageCodeToName("tr") /*i18n("Turkish")*/);
	mLanguages.insert(50, languageCodeToName("uk") /*i18n("Ukrainian")*/);
	mLanguages.insert(51, languageCodeToName("ur") /*i18n("Urdu")*/);
	mLanguages.insert(52, languageCodeToName("vi") /*i18n("Vietnamese")*/);
	mLanguages.insert(53, languageCodeToName("yi") /*i18n("Yiddish")*/);
	mLanguages.insert(54, languageCodeToName("yo") /*i18n("Yoruba")*/);
	mLanguages.insert(55, languageCodeToName("af") /*i18n("Afrikaans")*/);
	mLanguages.insert(56, languageCodeToName("bs") /*i18n("Bosnian")*/);
	mLanguages.insert(57, languageCodeToName("fa") /*i18n("Persian")*/);
	mLanguages.insert(58, languageCodeToName("sq") /*i18n("Albanian")*/);
	mLanguages.insert(59, languageCodeToName("hy") /*i18n("Armenian")*/);
	mLanguages.insert(60, i18n("Punjabi"));
	mLanguages.insert(61, languageCodeToName("ch") /*i18n("Chamorro")*/);
	mLanguages.insert(62, languageCodeToName("mn") /*i18n("Mongolian")*/);
	mLanguages.insert(63, i18n("Mandarin"));
	mLanguages.insert(64, i18n("Taiwanese"));
	mLanguages.insert(65, languageCodeToName("mk") /*i18n("Macedonian")*/);
	mLanguages.insert(66, languageCodeToName("sd") /*i18n("Sindhi")*/);
	mLanguages.insert(67, languageCodeToName("cy") /*i18n("Welsh")*/);
	mLanguages.insert(68, languageCodeToName("az") /*i18n("Azerbaijani")*/);
	mLanguages.insert(69, languageCodeToName("ku") /*i18n("Kurdish")*/);
	mLanguages.insert(70, languageCodeToName("gu") /*i18n("Gujarati")*/);
	mLanguages.insert(71, languageCodeToName("ta") /*i18n("Tamil")*/);
	mLanguages.insert(72, i18n("Belorussian"));
	mLanguages.insert(255, i18n("Unknown"));

}

void ICQProtocol::initEncodings()
{
	const QList<int> mibs = QTextCodec::availableMibs();
	QSet<int> availableMibs( mibs.begin(), mibs.end() );

	addEncoding( availableMibs, 2026, i18n("Big5") );
	addEncoding( availableMibs, 2101, i18n("Big5-HKSCS") );
	addEncoding( availableMibs, 18, i18n("euc-JP Japanese") );
	addEncoding( availableMibs, 38, i18n("euc-KR Korean") );
	addEncoding( availableMibs, 57, i18n("GB-2312 Chinese") );
	addEncoding( availableMibs, 113, i18n("GBK Chinese") );
	addEncoding( availableMibs, 114, i18n("GB18030 Chinese") );

	addEncoding( availableMibs, 16, i18n("JIS Japanese") );
	addEncoding( availableMibs, 17, i18n("Shift-JIS Japanese") );

	addEncoding( availableMibs, 2084, i18n("KOI8-R Russian") );
	addEncoding( availableMibs, 2088, i18n("KOI8-U Ukrainian") );

	addEncoding( availableMibs, 4, i18n("ISO-8859-1 Western") );
	addEncoding( availableMibs, 5, i18n("ISO-8859-2 Central European") );
	addEncoding( availableMibs, 6, i18n("ISO-8859-3 Central European") );
	addEncoding( availableMibs, 7, i18n("ISO-8859-4 Baltic") );
	addEncoding( availableMibs, 8, i18n("ISO-8859-5 Cyrillic") );
	addEncoding( availableMibs, 9, i18n("ISO-8859-6 Arabic") );
	addEncoding( availableMibs, 10, i18n("ISO-8859-7 Greek") );
	addEncoding( availableMibs, 11, i18n("ISO-8859-8 Hebrew, visually ordered") );
	addEncoding( availableMibs, 85, i18n("ISO-8859-8-I Hebrew, logically ordered") );
	addEncoding( availableMibs, 12, i18n("ISO-8859-9 Turkish") );
	addEncoding( availableMibs, 13, i18n("ISO-8859-10") );
	addEncoding( availableMibs, 109, i18n("ISO-8859-13") );
	addEncoding( availableMibs, 110, i18n("ISO-8859-14") );
	addEncoding( availableMibs, 111, i18n("ISO-8859-15 Western") );

	addEncoding( availableMibs, CP1250, i18n("Windows-1250 Central European") );
	addEncoding( availableMibs, CP1251, i18n("Windows-1251 Cyrillic") );
	addEncoding( availableMibs, CP1252, i18n("Windows-1252 Western") );
	addEncoding( availableMibs, CP1253, i18n("Windows-1253 Greek") );
	addEncoding( availableMibs, CP1254, i18n("Windows-1254 Turkish") );
	addEncoding( availableMibs, CP1255, i18n("Windows-1255 Hebrew") );
	addEncoding( availableMibs, CP1256, i18n("Windows-1256 Arabic") );
	addEncoding( availableMibs, CP1257, i18n("Windows-1257 Baltic") );
	addEncoding( availableMibs, CP1258, i18n("Windows-1258 Viet Nam") );

	addEncoding( availableMibs, 2009, i18n("IBM 850") );
	addEncoding( availableMibs, 2085, i18n("IBM 866") );

	addEncoding( availableMibs, 2259, i18n("TIS-620 Thai") );

	addEncoding( availableMibs, 106, i18n("UTF-8 Unicode") );
	addEncoding( availableMibs, 1015, i18n("UTF-16 Unicode") );

/*
Missing ones (copied from qtextcodec doc):
TSCII -- Tamil
CP874
Apple Roman
*/
}
void ICQProtocol::initMaritals()
{
	mMarital.insert(0 , "");
	mMarital.insert(10 , i18n("Single"));
	mMarital.insert(11 , i18n("Long term relationship"));
	mMarital.insert(12 , i18n("Engaged"));
	mMarital.insert(20 , i18n("Married"));
	mMarital.insert(30 , i18n("Divorced"));
	mMarital.insert(31 , i18n("Separated"));
	mMarital.insert(40 , i18n("Widowed"));

}

void ICQProtocol::initInterests()
{
	mInterests.insert(0 , "");
	mInterests.insert(100, i18n("Art"));
	mInterests.insert(101, i18n("Cars"));
	mInterests.insert(102, i18n("Celebrity Fans"));
	mInterests.insert(103, i18n("Collections"));
	mInterests.insert(104, i18n("Computers"));
	mInterests.insert(105, i18n("Culture"));
	mInterests.insert(106, i18n("Fitness"));
	mInterests.insert(107, i18n("Games"));
	mInterests.insert(108, i18n("Hobbies"));
	mInterests.insert(109, i18n("ICQ - Help"));
	mInterests.insert(110, i18n("Internet"));
	mInterests.insert(111, i18n("Lifestyle"));
	mInterests.insert(112, i18n("Movies and TV"));
	mInterests.insert(113, i18n("Music"));
	mInterests.insert(114, i18n("Outdoors"));
	mInterests.insert(115, i18n("Parenting"));
	mInterests.insert(116, i18n("Pets and Animals"));
	mInterests.insert(117, i18n("Religion"));
	mInterests.insert(118, i18n("Science"));
	mInterests.insert(119, i18n("Skills"));
	mInterests.insert(120, i18n("Sports"));
	mInterests.insert(121, i18n("Web Design"));
	mInterests.insert(122, i18n("Ecology"));
	mInterests.insert(123, i18n("News and Media"));
	mInterests.insert(124, i18n("Government"));
	mInterests.insert(125, i18n("Business"));
	mInterests.insert(126, i18n("Mystics"));
	mInterests.insert(127, i18n("Travel"));
	mInterests.insert(128, i18n("Astronomy"));
	mInterests.insert(129, i18n("Space"));
	mInterests.insert(130, i18n("Clothing"));
	mInterests.insert(131, i18n("Parties"));
	mInterests.insert(132, i18n("Women"));
	mInterests.insert(133, i18n("Social science"));
	mInterests.insert(134, i18n("60's"));
	mInterests.insert(135, i18n("70's"));
	mInterests.insert(136, i18n("80's"));
	mInterests.insert(137, i18n("50's"));
	mInterests.insert(138, i18n("Finance and Corporate"));
	mInterests.insert(139, i18n("Entertainment"));
	mInterests.insert(140, i18n("Consumer Electronics"));
	mInterests.insert(141, i18n("Retail Stores"));
	mInterests.insert(142, i18n("Health and Beauty"));
	mInterests.insert(143, i18n("Media"));
	mInterests.insert(144, i18n("Household Products"));
	mInterests.insert(145, i18n("Mail Order Catalog"));
	mInterests.insert(146, i18n("Business Services"));
	mInterests.insert(147, i18n("Audio and Visual"));
	mInterests.insert(148, i18n("Sporting and Athletic"));
	mInterests.insert(149, i18n("Publishing"));
	mInterests.insert(150, i18n("Home Automation"));

}

void ICQProtocol::initOccupations()
{
	mOccupations.insert(0 , "");
	mOccupations.insert(1 , i18n("Academic"));
	mOccupations.insert(2 , i18n("Administrative"));
	mOccupations.insert(3 , i18n("Art/Entertainment"));
	mOccupations.insert(4 , i18n("College Student"));
	mOccupations.insert(5 , i18n("Computers"));
	mOccupations.insert(6 , i18n("Community & Social"));
	mOccupations.insert(7 , i18n("Education"));
	mOccupations.insert(8 , i18n("Engineering"));
	mOccupations.insert(9 , i18n("Financial Services"));
	mOccupations.insert(10 , i18n("Government"));
	mOccupations.insert(11 , i18n("High School Student"));
	mOccupations.insert(12 , i18n("Home"));
	mOccupations.insert(13 , i18n("ICQ - Providing Help"));
	mOccupations.insert(14 , i18n("Law"));
	mOccupations.insert(15 , i18n("Managerial"));
	mOccupations.insert(16 , i18n("Manufacturing"));
	mOccupations.insert(17 , i18n("Medical/Health"));
	mOccupations.insert(18 , i18n("Military"));
	mOccupations.insert(19 , i18n("Non-Government Organization"));
	mOccupations.insert(99 , i18n("Other Services"));
	mOccupations.insert(20 , i18n("Professional"));
	mOccupations.insert(21 , i18n("Retail"));
	mOccupations.insert(22 , i18n("Retired"));
	mOccupations.insert(23 , i18n("Science & Research"));
	mOccupations.insert(24 , i18n("Sports"));
	mOccupations.insert(25 , i18n("Technical"));
	mOccupations.insert(26 , i18n("University Student"));
	mOccupations.insert(27 , i18n("Web Building"));

}

void ICQProtocol::initOrganizations()
{
	mOrganizations.insert(0 , "");
	mOrganizations.insert(200 , i18n("Alumni Org."));
	mOrganizations.insert(201 , i18n("Charity Org."));
	mOrganizations.insert(202 , i18n("Club/Social Org."));
	mOrganizations.insert(203 , i18n("Community Org."));
	mOrganizations.insert(204 , i18n("Cultural Org."));
	mOrganizations.insert(205 , i18n("Fan Clubs"));
	mOrganizations.insert(206 , i18n("Fraternity/Sorority"));
	mOrganizations.insert(207 , i18n("Hobbyists Org."));
	mOrganizations.insert(208 , i18n("International Org."));
	mOrganizations.insert(209 , i18n("Nature and Environment Org."));
	mOrganizations.insert(299 , i18n("Other"));
	mOrganizations.insert(210 , i18n("Professional Org."));
	mOrganizations.insert(211 , i18n("Scientific/Technical Org."));
	mOrganizations.insert(212 , i18n("Self Improvement Group"));
	mOrganizations.insert(213 , i18n("Spiritual/Religious Org."));
	mOrganizations.insert(214 , i18n("Sports Org."));
	mOrganizations.insert(215 , i18n("Support Org."));
	mOrganizations.insert(216 , i18n("Trade and Business Org."));
	mOrganizations.insert(217 , i18n("Union"));
	mOrganizations.insert(218 , i18n("Voluntary Org."));

}

void ICQProtocol::initAffiliations()
{
	mAffiliations.insert(0 , "");
	mAffiliations.insert(300 , i18n("Elementary School"));
	mAffiliations.insert(301 , i18n("High School"));
	mAffiliations.insert(302 , i18n("College"));
	mAffiliations.insert(303 , i18n("University"));
	mAffiliations.insert(304 , i18n("Military"));
	mAffiliations.insert(305 , i18n("Past Work Place"));
	mAffiliations.insert(306 , i18n("Past Organization"));
	mAffiliations.insert(399 , i18n("Other"));

}

void ICQProtocol::fillComboFromTable(QComboBox *box, const QMap<int, QString> &map)
{
//	kDebug(14153) << "Called.";

	QStringList list = map.values();
	list.sort();
	box->addItems(list);
}

void ICQProtocol::setComboFromTable(QComboBox *box, const QMap<int, QString> &map, int value)
{
//	kDebug(14153) << "Called.";
	QMap<int, QString>::ConstIterator it;
	it = map.find(value);
	if ( it == map.end() )
		return;

	for(int i=0; i<box->count(); i++)
	{
		if((*it) == box->itemText(i))
		{
			box->setCurrentIndex(i);
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
		if(it.value() == curText)
			return it.key();
	}
	return 0; // unspecified is always first 0
}

void ICQProtocol::addEncoding( const QSet<int> &availableMibs, int mib, const QString &name )
{
	if ( availableMibs.contains( mib ) )
		mEncodings.insert( mib, name );
}

#if 0

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
	kDebug(14153) << "tz=" << int(tz);
	if ((tz < -24) || (tz > 24))
		combo->setCurrentItem(24); // GMT+0:00 as default
	else
		combo->setCurrentItem(24 + tz);
}

char ICQProtocol::getTZComboValue(QComboBox *combo)
{
	char ret =  combo->currentItem() - 24;
// 	kDebug(14153) << "return value=" << int(ret);
	return ret;
}

#endif
ICQProtocol *ICQProtocol::protocol()
{
	return protocolStatic_;
}

bool ICQProtocol::canSendOffline() const
{
	return true;
}

AddContactPage *ICQProtocol::createAddContactWidget(QWidget *parent, Kopete::Account *account)
{
	return new ICQAddContactPage( static_cast<ICQAccount*>( account ), parent);
}

KopeteEditAccountWidget *ICQProtocol::createEditAccountWidget(Kopete::Account *account, QWidget *parent)
{
	return new ICQEditAccountWidget(this, account, parent);
}

Kopete::Account *ICQProtocol::createNewAccount(const QString &accountId)
{
	return new ICQAccount(this, accountId);
}

OscarStatusManager *ICQProtocol::statusManager() const
{
	return statusManager_;
}

//END class ICQProtocol

#include "icqprotocol.moc"
// kate: indent-mode csands;
