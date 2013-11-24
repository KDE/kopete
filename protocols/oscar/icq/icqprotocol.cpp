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

#include <kgenericfactory.h>
#include <klocale.h>
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
K_EXPORT_PLUGIN( ICQProtocolFactory( "kopete_icq" ) )

//BEGIN class ICQProtocolHandler

ICQProtocolHandler::ICQProtocolHandler() : Kopete::MimeTypeHandler(false)
{
	registerAsMimeHandler(QString::fromLatin1("application/x-icq"));
}

void ICQProtocolHandler::handleURL(const QString &mimeType, const KUrl & url) const
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

ICQProtocol* ICQProtocol::protocolStatic_ = 0L;

ICQProtocol::ICQProtocol(QObject *parent, const QVariantList&)
: OscarProtocol( ICQProtocolFactory::componentData(), parent ),
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
	KLocale *kl = KGlobal::locale(); //KLocale(QString::fromLatin1("kopete"));

	mCountries.insert(93, kl->countryCodeToName("af"));
	mCountries.insert(355, kl->countryCodeToName("al"));
	mCountries.insert(213, kl->countryCodeToName("dz"));
	mCountries.insert(684, kl->countryCodeToName("as"));
	mCountries.insert(376, kl->countryCodeToName("ad"));
	mCountries.insert(244, kl->countryCodeToName("ao"));
	mCountries.insert(101, kl->countryCodeToName("ai"));
	mCountries.insert(102, i18n("Antigua"));
	mCountries.insert(1021, kl->countryCodeToName("ag"));
	mCountries.insert(54, kl->countryCodeToName("ar"));
	mCountries.insert(374, kl->countryCodeToName("am"));
	mCountries.insert(297, kl->countryCodeToName("aw"));
	mCountries.insert(247, i18n("Ascension Island"));
	mCountries.insert(61, kl->countryCodeToName("au"));
	mCountries.insert(43, kl->countryCodeToName("at"));
	mCountries.insert(994, kl->countryCodeToName("az"));
	mCountries.insert(103, kl->countryCodeToName("bs"));
	mCountries.insert(973, kl->countryCodeToName("bh"));
	mCountries.insert(880, kl->countryCodeToName("bd"));
	mCountries.insert(104, kl->countryCodeToName("bb"));
	mCountries.insert(120, i18n("Barbuda"));
	mCountries.insert(375, kl->countryCodeToName("by"));
	mCountries.insert(32, kl->countryCodeToName("be"));
	mCountries.insert(501, kl->countryCodeToName("bz"));
	mCountries.insert(229, kl->countryCodeToName("bj"));
	mCountries.insert(105, kl->countryCodeToName("bm"));
	mCountries.insert(975, kl->countryCodeToName("bt"));
	mCountries.insert(591, kl->countryCodeToName("bo"));
	mCountries.insert(387, kl->countryCodeToName("ba"));
	mCountries.insert(267, kl->countryCodeToName("bw"));
	mCountries.insert(55, kl->countryCodeToName("br"));
	mCountries.insert(106, i18n("British Virgin Islands"));
	mCountries.insert(673, kl->countryCodeToName("bn"));
	mCountries.insert(359, kl->countryCodeToName("bg"));
	mCountries.insert(226, kl->countryCodeToName("bf"));
	mCountries.insert(257, kl->countryCodeToName("bi"));
	mCountries.insert(855, kl->countryCodeToName("kh"));
	mCountries.insert(237, kl->countryCodeToName("cm"));
	mCountries.insert(107, kl->countryCodeToName("ca"));
	mCountries.insert(238, kl->countryCodeToName("cv"));
	mCountries.insert(108, kl->countryCodeToName("ky"));
	mCountries.insert(236, kl->countryCodeToName("cf"));
	mCountries.insert(235, kl->countryCodeToName("td"));
	mCountries.insert(178, i18n("Canary Islands"));
	mCountries.insert(56, kl->countryCodeToName("cl"));
	mCountries.insert(86, kl->countryCodeToName("cn"));
	mCountries.insert(672, kl->countryCodeToName("cx"));
	mCountries.insert(6101, i18n("Cocos-Keeling Islands"));
	mCountries.insert(6102, i18n("Cocos (Keeling) Islands"));
	mCountries.insert(57, kl->countryCodeToName("co"));
	mCountries.insert(2691, kl->countryCodeToName("km"));
	mCountries.insert(242, kl->countryCodeToName("cg"));
	mCountries.insert(682, kl->countryCodeToName("ck"));
	mCountries.insert(506, kl->countryCodeToName("cr"));
	mCountries.insert(385, kl->countryCodeToName("hr"));
	mCountries.insert(53, kl->countryCodeToName("cu"));
	mCountries.insert(357, kl->countryCodeToName("cy"));
	mCountries.insert(42, kl->countryCodeToName("cz"));
	mCountries.insert(45, kl->countryCodeToName("dk"));
	mCountries.insert(246, i18n("Diego Garcia"));
	mCountries.insert(253, kl->countryCodeToName("dj"));
	mCountries.insert(109, kl->countryCodeToName("dm"));
	mCountries.insert(110, kl->countryCodeToName("do"));
	mCountries.insert(593, kl->countryCodeToName("ec"));
	mCountries.insert(20, kl->countryCodeToName("eg"));
	mCountries.insert(503, kl->countryCodeToName("sv"));
	mCountries.insert(240, kl->countryCodeToName("gq"));
	mCountries.insert(291, kl->countryCodeToName("er"));
	mCountries.insert(372, kl->countryCodeToName("ee"));
	mCountries.insert(251, kl->countryCodeToName("et"));
	mCountries.insert(298, kl->countryCodeToName("fo"));
	mCountries.insert(500, kl->countryCodeToName("fk"));
	mCountries.insert(679, kl->countryCodeToName("fj"));
	mCountries.insert(358, kl->countryCodeToName("fi"));
	mCountries.insert(33, kl->countryCodeToName("fr"));
	mCountries.insert(5901, i18n("French Antilles"));
	mCountries.insert(5902, i18n("Antilles"));
	mCountries.insert(594, i18n("French Guiana"));
	mCountries.insert(689, kl->countryCodeToName("pf"));
	mCountries.insert(241, kl->countryCodeToName("ga"));
	mCountries.insert(220, kl->countryCodeToName("gm"));
	mCountries.insert(995, kl->countryCodeToName("ge"));
	mCountries.insert(49, kl->countryCodeToName("de"));
	mCountries.insert(233, kl->countryCodeToName("gh"));
	mCountries.insert(350, kl->countryCodeToName("gi"));
	mCountries.insert(30, kl->countryCodeToName("gr"));
	mCountries.insert(299, kl->countryCodeToName("gl"));
	mCountries.insert(111, kl->countryCodeToName("gd"));
	mCountries.insert(590, kl->countryCodeToName("gp"));
	mCountries.insert(671, kl->countryCodeToName("gu"));
	mCountries.insert(502, kl->countryCodeToName("gt"));
	mCountries.insert(224, kl->countryCodeToName("gn"));
	mCountries.insert(245, kl->countryCodeToName("gw"));
	mCountries.insert(592, kl->countryCodeToName("gy"));
	mCountries.insert(509, kl->countryCodeToName("ht"));
	mCountries.insert(504, kl->countryCodeToName("hn"));
	mCountries.insert(852, kl->countryCodeToName("hk"));
	mCountries.insert(36, kl->countryCodeToName("hu"));
	mCountries.insert(354, kl->countryCodeToName("is"));
	mCountries.insert(91, kl->countryCodeToName("in"));
	mCountries.insert(62, kl->countryCodeToName("id"));
	mCountries.insert(98, kl->countryCodeToName("ir"));
	mCountries.insert(964, kl->countryCodeToName("iq"));
	mCountries.insert(353, kl->countryCodeToName("ie"));
	mCountries.insert(972, kl->countryCodeToName("il"));
	mCountries.insert(39, kl->countryCodeToName("it"));
	mCountries.insert(225, kl->countryCodeToName("ci"));
	mCountries.insert(112, kl->countryCodeToName("jm"));
	mCountries.insert(81, kl->countryCodeToName("jp"));
	mCountries.insert(962, kl->countryCodeToName("jo"));
	mCountries.insert(705, kl->countryCodeToName("kz"));
	mCountries.insert(254, kl->countryCodeToName("ke"));
	mCountries.insert(686, kl->countryCodeToName("ki"));
	mCountries.insert(850, kl->countryCodeToName("kp"));
	mCountries.insert(82, kl->countryCodeToName("kr"));
	mCountries.insert(965, kl->countryCodeToName("kw"));
	mCountries.insert(706, kl->countryCodeToName("kg"));
	mCountries.insert(856, kl->countryCodeToName("la"));
	mCountries.insert(371, kl->countryCodeToName("lv"));
	mCountries.insert(961, kl->countryCodeToName("lb"));
	mCountries.insert(266, kl->countryCodeToName("ls"));
	mCountries.insert(231, kl->countryCodeToName("lr"));
	mCountries.insert(218, kl->countryCodeToName("ly"));
	mCountries.insert(4101, kl->countryCodeToName("li"));
	mCountries.insert(370, kl->countryCodeToName("lt"));
	mCountries.insert(352, kl->countryCodeToName("lu"));
	mCountries.insert(853, kl->countryCodeToName("mo"));
	mCountries.insert(261, kl->countryCodeToName("mg"));
	mCountries.insert(265, kl->countryCodeToName("mw"));
	mCountries.insert(60, kl->countryCodeToName("my"));
	mCountries.insert(960, kl->countryCodeToName("mv"));
	mCountries.insert(223, kl->countryCodeToName("ml"));
	mCountries.insert(356, kl->countryCodeToName("mt"));
	mCountries.insert(692, kl->countryCodeToName("mh"));
	mCountries.insert(596, kl->countryCodeToName("mq"));
	mCountries.insert(222, kl->countryCodeToName("mr"));
	mCountries.insert(230, kl->countryCodeToName("mu"));
	mCountries.insert(269, i18n("Mayotte Island"));
	mCountries.insert(52, kl->countryCodeToName("mx"));
	mCountries.insert(691, kl->countryCodeToName("fm"));
	mCountries.insert(373, kl->countryCodeToName("md"));
	mCountries.insert(377, kl->countryCodeToName("mc"));
	mCountries.insert(976, kl->countryCodeToName("mn"));
	mCountries.insert(113, kl->countryCodeToName("ms"));
	mCountries.insert(212, kl->countryCodeToName("ma"));
	mCountries.insert(258, kl->countryCodeToName("mz"));
	mCountries.insert(95, kl->countryCodeToName("mm"));
	mCountries.insert(264, kl->countryCodeToName("na"));
	mCountries.insert(674, kl->countryCodeToName("nr"));
	mCountries.insert(977, kl->countryCodeToName("np"));
	mCountries.insert(599, kl->countryCodeToName("an"));
	mCountries.insert(31, kl->countryCodeToName("nl"));
	mCountries.insert(114, i18n("Nevis"));
	mCountries.insert(687, kl->countryCodeToName("nc"));
	mCountries.insert(64, kl->countryCodeToName("nz"));
	mCountries.insert(505, kl->countryCodeToName("ni"));
	mCountries.insert(227, kl->countryCodeToName("ne"));
	mCountries.insert(234, kl->countryCodeToName("ng"));
	mCountries.insert(683, kl->countryCodeToName("nu"));
	mCountries.insert(6722, kl->countryCodeToName("nf"));
	mCountries.insert(47, kl->countryCodeToName("no"));
	mCountries.insert(968, kl->countryCodeToName("om"));
	mCountries.insert(92, kl->countryCodeToName("pk"));
	mCountries.insert(680, kl->countryCodeToName("pw"));
	mCountries.insert(507, kl->countryCodeToName("pa"));
	mCountries.insert(675, kl->countryCodeToName("pg"));
	mCountries.insert(595, kl->countryCodeToName("py"));
	mCountries.insert(51, kl->countryCodeToName("pe"));
	mCountries.insert(63, kl->countryCodeToName("ph"));
	mCountries.insert(48, kl->countryCodeToName("pl"));
	mCountries.insert(351, kl->countryCodeToName("pt"));
	mCountries.insert(121, kl->countryCodeToName("pr"));
	mCountries.insert(974, kl->countryCodeToName("qa"));
	mCountries.insert(389, kl->countryCodeToName("mk"));
	mCountries.insert(262, i18n("Reunion Island"));
	mCountries.insert(40, kl->countryCodeToName("ro"));
	mCountries.insert(6701, i18n("Rota Island"));
	mCountries.insert(7, kl->countryCodeToName("ru"));
	mCountries.insert(250, kl->countryCodeToName("rw"));
	mCountries.insert(122, kl->countryCodeToName("lc"));
	mCountries.insert(670, i18n("Saipan Island"));
	mCountries.insert(378, kl->countryCodeToName("sm"));
	mCountries.insert(239, kl->countryCodeToName("st"));
	mCountries.insert(966, kl->countryCodeToName("sa"));
	mCountries.insert(221, kl->countryCodeToName("sn"));
	mCountries.insert(248, kl->countryCodeToName("sc"));
	mCountries.insert(232, i18n("Sierra Leone"));
	mCountries.insert(65, kl->countryCodeToName("sg"));
	mCountries.insert(4201, kl->countryCodeToName("sk"));
	mCountries.insert(386, kl->countryCodeToName("si"));
	mCountries.insert(677, kl->countryCodeToName("sb"));
	mCountries.insert(252, kl->countryCodeToName("so"));
	mCountries.insert(27, kl->countryCodeToName("za"));
	mCountries.insert(34, kl->countryCodeToName("es"));
	mCountries.insert(94, kl->countryCodeToName("lk"));
	mCountries.insert(290, kl->countryCodeToName("sh"));
	mCountries.insert(115, i18n("St. Kitts"));
	mCountries.insert(1141, kl->countryCodeToName("kn"));
	mCountries.insert(508, kl->countryCodeToName("pm"));
	mCountries.insert(116, kl->countryCodeToName("vc"));
	mCountries.insert(249, kl->countryCodeToName("sd"));
	mCountries.insert(597, kl->countryCodeToName("sr"));
	mCountries.insert(268, kl->countryCodeToName("sz"));
	mCountries.insert(46, kl->countryCodeToName("se"));
	mCountries.insert(41, kl->countryCodeToName("ch"));
	mCountries.insert(963, kl->countryCodeToName("sy"));
	mCountries.insert(886, kl->countryCodeToName("tw"));
	mCountries.insert(708, kl->countryCodeToName("tj"));
	mCountries.insert(255, kl->countryCodeToName("tz"));
	mCountries.insert(66, kl->countryCodeToName("th"));
	mCountries.insert(6702, i18n("Tinian Island"));
	mCountries.insert(228, kl->countryCodeToName("tg")); // Togo
	mCountries.insert(690, kl->countryCodeToName("tk")); // Tokelau
	mCountries.insert(676, kl->countryCodeToName("to")); // Tonga
	mCountries.insert(117, kl->countryCodeToName("tt")); // Trinidad and Tobago
	mCountries.insert(216, kl->countryCodeToName("tn")); // Tunisia
	mCountries.insert(90, kl->countryCodeToName("tr"));
	mCountries.insert(709, kl->countryCodeToName("tm"));
	mCountries.insert(118, kl->countryCodeToName("tc")); // Turks and Caicos Island
	mCountries.insert(688, kl->countryCodeToName("tv")); // Tuvalu
	mCountries.insert(1, kl->countryCodeToName("us")); // United States of America
	mCountries.insert(256, kl->countryCodeToName("ug")); // Uganda
	mCountries.insert(380, kl->countryCodeToName("ua")); // Ukraine
	mCountries.insert(971, kl->countryCodeToName("ae")); // United Arab Emirates
	mCountries.insert(44, kl->countryCodeToName("gb")); // United Kingdom
	mCountries.insert(441, i18n("Wales"));
	mCountries.insert(442, i18n("Scotland"));
	mCountries.insert(123, kl->countryCodeToName("vi")); // United States Virgin Islands
	mCountries.insert(598, kl->countryCodeToName("uy")); // Uruguay
	mCountries.insert(711, kl->countryCodeToName("uz")); // Uzbekistan
	mCountries.insert(678, kl->countryCodeToName("vu")); // Vanuatu
	mCountries.insert(379, kl->countryCodeToName("va")); // Vatican City
	mCountries.insert(58, kl->countryCodeToName("ve")); // Venezuela
	mCountries.insert(84, kl->countryCodeToName("vn")); // Vietnam
	mCountries.insert(681, kl->countryCodeToName("wf")); // Wallis and Futuna Islands
	mCountries.insert(685, kl->countryCodeToName("ws"));
	mCountries.insert(967, kl->countryCodeToName("ye"));
	mCountries.insert(3811, i18n("Yugoslavia - Serbia"));
	mCountries.insert(382, i18n("Yugoslavia - Montenegro"));
	mCountries.insert(381, i18n("Yugoslavia"));
	mCountries.insert(243, i18n("Congo, Democratic Republic of (Zaire)"));
	mCountries.insert(260, kl->countryCodeToName("zm"));
	mCountries.insert(263, kl->countryCodeToName("zw"));
	mCountries.insert(9999, i18n("Unknown"));

}

void ICQProtocol::initLang()
{

	KLocale *kl = KGlobal::locale(); //KLocale(QString::fromLatin1("kopete"));

	mLanguages.insert(0 , "");
	mLanguages.insert(1 , kl->languageCodeToName("ar") /*i18n("Arabic")*/);
	mLanguages.insert(2 , i18n("Bhojpuri"));
	mLanguages.insert(3 , kl->languageCodeToName("bg") /*i18n("Bulgarian")*/);
	mLanguages.insert(4 , kl->languageCodeToName("my") /*i18n("Burmese")*/);
	mLanguages.insert(5 , i18n("Cantonese"));
	mLanguages.insert(6 , kl->languageCodeToName("ca") /*i18n("Catalan")*/);
	mLanguages.insert(7 , kl->languageCodeToName("zh") /*i18n("Chinese")*/);
	mLanguages.insert(8 , kl->languageCodeToName("hr") /*i18n("Croatian")*/);
	mLanguages.insert(9 , kl->languageCodeToName("cs") /*i18n("Czech")*/);
	mLanguages.insert(10, kl->languageCodeToName("da") /*i18n("Danish")*/);
	mLanguages.insert(11, kl->languageCodeToName("nl") /*i18n("Dutch")*/);
	mLanguages.insert(12, kl->languageCodeToName("en") /*i18n("English")*/);
	mLanguages.insert(13, kl->languageCodeToName("eo") /*i18n("Esperanto")*/);
	mLanguages.insert(14, kl->languageCodeToName("et") /*i18n("Estonian")*/);
	mLanguages.insert(15, i18n("Farsi"));
	mLanguages.insert(16, kl->languageCodeToName("fi") /*i18n("Finnish")*/);
	mLanguages.insert(17, kl->languageCodeToName("fr") /*i18n("French")*/);
	mLanguages.insert(18, kl->languageCodeToName("gd") /*i18n("Gaelic")*/);
	mLanguages.insert(19, kl->languageCodeToName("de") /*i18n("German")*/);
	mLanguages.insert(20, kl->languageCodeToName("el") /*i18n("Greek")*/);
	mLanguages.insert(21, kl->languageCodeToName("he") /*i18n("Hebrew")*/);
	mLanguages.insert(22, kl->languageCodeToName("hi") /*i18n("Hindi")*/);
	mLanguages.insert(23, kl->languageCodeToName("hu") /*i18n("Hungarian")*/);
	mLanguages.insert(24, kl->languageCodeToName("is") /*i18n("Icelandic")*/);
	mLanguages.insert(25, kl->languageCodeToName("id") /*i18n("Indonesian")*/);
	mLanguages.insert(26, kl->languageCodeToName("it") /*i18n("Italian")*/);
	mLanguages.insert(27, kl->languageCodeToName("ja") /*i18n("Japanese")*/);
	mLanguages.insert(28, kl->languageCodeToName("km") /*i18n("Khmer")*/);
	mLanguages.insert(29, kl->languageCodeToName("ko") /*i18n("Korean")*/);
	mLanguages.insert(30, kl->languageCodeToName("lo") /*i18n("Lao")*/);
	mLanguages.insert(31, kl->languageCodeToName("lv") /*i18n("Latvian")*/);
	mLanguages.insert(32, kl->languageCodeToName("lt") /*i18n("Lithuanian")*/);
	mLanguages.insert(33, kl->languageCodeToName("ms") /*i18n("Malay")*/);
	mLanguages.insert(34, kl->languageCodeToName("no") /*i18n("Norwegian")*/);
	mLanguages.insert(35, kl->languageCodeToName("pl") /*i18n("Polish")*/);
	mLanguages.insert(36, kl->languageCodeToName("pt") /*i18n("Portuguese")*/);
	mLanguages.insert(37, kl->languageCodeToName("ro") /*i18n("Romanian")*/);
	mLanguages.insert(38, kl->languageCodeToName("ru") /*i18n("Russian")*/);
	mLanguages.insert(39, kl->languageCodeToName("sr") /*i18n("Serbian")*/);
	mLanguages.insert(40, kl->languageCodeToName("sk") /*i18n("Slovak")*/);
	mLanguages.insert(41, kl->languageCodeToName("sl") /*i18n("Slovenian")*/);
	mLanguages.insert(42, kl->languageCodeToName("so") /*i18n("Somali")*/);
	mLanguages.insert(43, kl->languageCodeToName("es") /*i18n("Spanish")*/);
	mLanguages.insert(44, kl->languageCodeToName("sw") /*i18n("Swahili")*/);
	mLanguages.insert(45, kl->languageCodeToName("sv") /*i18n("Swedish")*/);
	mLanguages.insert(46, kl->languageCodeToName("tl") /*i18n("Tagalog")*/);
	mLanguages.insert(47, kl->languageCodeToName("tt") /*i18n("Tatar")*/);
	mLanguages.insert(48, kl->languageCodeToName("th") /*i18n("Thai")*/);
	mLanguages.insert(49, kl->languageCodeToName("tr") /*i18n("Turkish")*/);
	mLanguages.insert(50, kl->languageCodeToName("uk") /*i18n("Ukrainian")*/);
	mLanguages.insert(51, kl->languageCodeToName("ur") /*i18n("Urdu")*/);
	mLanguages.insert(52, kl->languageCodeToName("vi") /*i18n("Vietnamese")*/);
	mLanguages.insert(53, kl->languageCodeToName("yi") /*i18n("Yiddish")*/);
	mLanguages.insert(54, kl->languageCodeToName("yo") /*i18n("Yoruba")*/);
	mLanguages.insert(55, kl->languageCodeToName("af") /*i18n("Afrikaans")*/);
	mLanguages.insert(56, kl->languageCodeToName("bs") /*i18n("Bosnian")*/);
	mLanguages.insert(57, kl->languageCodeToName("fa") /*i18n("Persian")*/);
	mLanguages.insert(58, kl->languageCodeToName("sq") /*i18n("Albanian")*/);
	mLanguages.insert(59, kl->languageCodeToName("hy") /*i18n("Armenian")*/);
	mLanguages.insert(60, i18n("Punjabi"));
	mLanguages.insert(61, kl->languageCodeToName("ch") /*i18n("Chamorro")*/);
	mLanguages.insert(62, kl->languageCodeToName("mn") /*i18n("Mongolian")*/);
	mLanguages.insert(63, i18n("Mandarin"));
	mLanguages.insert(64, i18n("Taiwanese"));
	mLanguages.insert(65, kl->languageCodeToName("mk") /*i18n("Macedonian")*/);
	mLanguages.insert(66, kl->languageCodeToName("sd") /*i18n("Sindhi")*/);
	mLanguages.insert(67, kl->languageCodeToName("cy") /*i18n("Welsh")*/);
	mLanguages.insert(68, kl->languageCodeToName("az") /*i18n("Azerbaijani")*/);
	mLanguages.insert(69, kl->languageCodeToName("ku") /*i18n("Kurdish")*/);
	mLanguages.insert(70, kl->languageCodeToName("gu") /*i18n("Gujarati")*/);
	mLanguages.insert(71, kl->languageCodeToName("ta") /*i18n("Tamil")*/);
	mLanguages.insert(72, i18n("Belorussian"));
	mLanguages.insert(255, i18n("Unknown"));

}

void ICQProtocol::initEncodings()
{
	QSet<int> availableMibs = QSet<int>::fromList( QTextCodec::availableMibs() );

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
// vim: set noet ts=4 sts=4 sw=4:
