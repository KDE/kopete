/*
  icqprotocol.cpp  -  ICQ Protocol Plugin

  Copyright (c) 2003      by Olivier Goffart        <ogoffart @ kde.org>
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

#include "config.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <netinet/in.h> // for ntohl()

#include <qcombobox.h>
/*
#include <qhostaddress.h>
#include <qlistbox.h>
#include <qspinbox.h>
#include <qtextedit.h>

#include <kdatewidget.h>*/
#include <qvaluelist.h>
#include <kdialogbase.h>
/*
#include <klineedit.h>
#include <kurllabel.h>
*/
#include <kgenericfactory.h>
#include <kdebug.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>

#include "kopeteglobal.h"
#include "kopeteuiglobal.h"
#include "accountselector.h"
#include "kopeteaccountmanager.h"

#include "oscartypeclasses.h"

#include "icqaccount.h"
#include "icqcontact.h"
#include "icqaddcontactpage.h"
#include "icqeditaccountwidget.h"
//#include "icquserinfowidget.h"


#include "icqprotocol.h"

typedef KGenericFactory<ICQProtocol> ICQProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_icq, ICQProtocolFactory( "kopete_icq" ) )

//BEGIN class ICQProtocolHandler

ICQProtocolHandler::ICQProtocolHandler() : Kopete::MimeTypeHandler(false)
{
	registerAsMimeHandler(QString::fromLatin1("application/x-icq"));
}

void ICQProtocolHandler::handleURL(const QString &mimeType, const KURL & url) const
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

	KSimpleConfig file(url.path(), true);

	if (file.hasGroup("ICQ User"))
		file.setGroup("ICQ User");
	else if (file.hasGroup("ICQ Message User"))
		file.setGroup("ICQ Message User");
	else
		return;

	ICQProtocol *proto = ICQProtocol::protocol();

	QString uin = file.readEntry("UIN");
	if (uin.isEmpty())
		return;

	QString nick = file.readEntry("NickName");
	QString first = file.readEntry("FirstName");
	QString last = file.readEntry("LastName");
	QString email = file.readEntry("Email");

	Kopete::Account *account = 0;
	QDict<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts(proto);
	// do not show chooser if we only have one account to "choose" from
	if (accounts.count() == 1)
	{
		QDictIterator<Kopete::Account> it(accounts);
		account = it.current();
	}
	else
	{
		KDialogBase *chooser = new KDialogBase(0, "chooser", true,
			i18n("Choose Account"), KDialogBase::Ok|KDialogBase::Cancel,
			KDialogBase::Ok, false);
		AccountSelector *accSelector = new AccountSelector(proto, chooser,
			"accSelector");
		chooser->setMainWidget(accSelector);

		int ret = chooser->exec();
		account = accSelector->selectedItem();

		delete chooser;
		if (ret == QDialog::Rejected || account == 0)
		{
			kdDebug(14153) << k_funcinfo << "Cancelled" << endl;
			return;
		}
	}

	if (!account->isConnected())
	{
		kdDebug(14153) << k_funcinfo << "Can't add contact, we are offline!" << endl;
		KMessageBox::sorry( Kopete::UI::Global::mainWidget(), i18n("You must be online to add a contact."), i18n("ICQ") );
		return;
	}

	QString nickuin = nick.isEmpty() ?
		i18n("'%1'").arg(uin) :
		i18n("'%1' (%2)").arg(nick, uin);

	if (KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget(),
	                               i18n("Do you want to add %1 to your contact list?").arg(nickuin), QString::null, i18n("Add"), i18n("Do Not Add"))
	    != KMessageBox::Yes)
	{
		kdDebug(14153) << k_funcinfo << "Cancelled" << endl;
		return;
	}

	kdDebug(14153) << k_funcinfo <<
		"Adding Contact; uin = " << uin << ", nick = '" << nick <<
		"', firstname = '" << first << "', lastname = '" << last <<"'" << endl;
	if (account->addContact(uin, nick, 0L, Kopete::Account::Temporary))
	{
		Kopete::Contact *contact = account->contacts()[uin];
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

ICQProtocol::ICQProtocol(QObject *parent, const char *name, const QStringList&)
: Kopete::Protocol( ICQProtocolFactory::instance(), parent, name ),
	firstName(Kopete::Global::Properties::self()->firstName()),
	lastName(Kopete::Global::Properties::self()->lastName()),
	awayMessage(Kopete::Global::Properties::self()->awayMessage()),
	emailAddress(Kopete::Global::Properties::self()->emailAddress()),
	ipAddress("ipAddress", i18n("IP Address") ),
	clientFeatures("clientFeatures", i18n("Client Features"), 0, false),
	buddyIconHash("iconHash", i18n("Buddy Icon MD5 Hash"), QString::null, true, false, true),
    contactEncoding( "contactEncoding", i18n( "Contact Encoding" ), QString::null, true, false, true )

{
	if (protocolStatic_)
		kdWarning(14153) << k_funcinfo << "ICQ plugin already initialized" << endl;
	else
		protocolStatic_ = this;

	// must be done after protocolStatic_ is set...
	statusManager_ = new ICQ::OnlineStatusManager;

	addAddressBookField("messaging/icq", Kopete::Plugin::MakeIndexField);

	initGenders();
	initLang();
	initCountries();
	initEncodings();
	initMaritals();
	initInterests();
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
	mCountries.insert(42, kl->twoAlphaToCountryName("cz"));
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
	mCountries.insert(371, kl->twoAlphaToCountryName("lv"));
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
	mCountries.insert(4201, kl->twoAlphaToCountryName("sk"));
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

	mEncodings.insert(106, i18n("UTF-8 Unicode"));
	mEncodings.insert(1015, i18n("UTF-16 Unicode"));

/*
Missing ones (copied from qtextcodec doc):
TSCII -- Tamil
utf8 -- Unicode, 8-bit
utf16 -- Unicode
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
	mInterests.insert(102, i18n("Celebrities"));
	mInterests.insert(103, i18n("Collections"));
	mInterests.insert(104, i18n("Computers"));
	mInterests.insert(105, i18n("Culture"));
	mInterests.insert(106, i18n("Fitness"));
	mInterests.insert(107, i18n("Games"));
	mInterests.insert(108, i18n("Hobbies"));
	mInterests.insert(109, i18n("ICQ - Help"));
	mInterests.insert(110, i18n("Internet"));
	mInterests.insert(111, i18n("Lifestyle"));
	mInterests.insert(112, i18n("Movies"));
	mInterests.insert(113, i18n("Music"));
	mInterests.insert(114, i18n("Outdoors"));
	mInterests.insert(115, i18n("Parenting"));
	mInterests.insert(116, i18n("Pets and animals"));
	mInterests.insert(117, i18n("Religion"));
	mInterests.insert(118, i18n("Science"));
	mInterests.insert(119, i18n("Skills"));
	mInterests.insert(120, i18n("Sports"));
	mInterests.insert(121, i18n("Web design"));
	mInterests.insert(122, i18n("Ecology"));
	mInterests.insert(123, i18n("News and media"));
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
	mInterests.insert(136, i18n("40's"));
	mInterests.insert(137, i18n("50's"));
	mInterests.insert(138, i18n("Finance and corporate"));
	mInterests.insert(139, i18n("Entertainment"));
	mInterests.insert(140, i18n("Consumer electronics"));
	mInterests.insert(141, i18n("Retail stores"));
	mInterests.insert(142, i18n("Health and beauty"));
	mInterests.insert(143, i18n("Media"));
	mInterests.insert(144, i18n("Household products"));
	mInterests.insert(145, i18n("Mail order catalog"));
	mInterests.insert(146, i18n("Business services"));
	mInterests.insert(147, i18n("Audio and visual"));
	mInterests.insert(148, i18n("Sporting and athletic"));
	mInterests.insert(149, i18n("Publishing"));
	mInterests.insert(150, i18n("Home automation"));

}

void ICQProtocol::fillComboFromTable(QComboBox *box, const QMap<int, QString> &map)
{
//	kdDebug(14153) << k_funcinfo << "Called." << endl;

	QStringList list = map.values();
	list.sort();
	box->insertStringList(list);
}

void ICQProtocol::setComboFromTable(QComboBox *box, const QMap<int, QString> &map, int value)
{
//	kdDebug(14153) << k_funcinfo << "Called." << endl;
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
	kdDebug(14153) << k_funcinfo << "tz=" << int(tz) << endl;
	if ((tz < -24) || (tz > 24))
		combo->setCurrentItem(24); // GMT+0:00 as default
	else
		combo->setCurrentItem(24 + tz);
}

char ICQProtocol::getTZComboValue(QComboBox *combo)
{
	char ret =  combo->currentItem() - 24;
// 	kdDebug(14153) << k_funcinfo << "return value=" << int(ret) << endl;
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

Kopete::Contact *ICQProtocol::deserializeContact( Kopete::MetaContact *metaContact,
                                                  const QMap<QString, QString> &serializedData,
                                                  const QMap<QString, QString> &/*addressBookData*/ )
{
	QString accountId = serializedData["accountId"];
	QDict<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts(this);
	ICQAccount *account = static_cast<ICQAccount*>(accounts[accountId]);

	if(!account)
	{
		kdWarning(14153) << k_funcinfo <<
			"WARNING: Account for contact does not exist, skipping " << accountId << endl;
		return 0;
	}

	QString contactId=serializedData["contactId"];
	uint ssiGid = 0, ssiBid = 0, ssiType = 0xFFFF;
	QString ssiName;
	bool ssiWaitingAuth = false;
    if ( serializedData.contains( "ssi_name" ) )
	ssiName = serializedData["ssi_name"];

    if ( serializedData.contains( "ssi_waitingAuth" ) )
    {
	QString authStatus = serializedData["ssi_waitingAuth"];
	if ( authStatus == "true" )
		ssiWaitingAuth = true;
    }

    if ( serializedData.contains( "ssi_gid" ) )
	ssiGid = serializedData["ssi_gid"].toUInt();
    if ( serializedData.contains( "ssi_bid" ) )
	ssiBid = serializedData["ssi_bid"].toUInt();
    if ( serializedData.contains( "ssi_type" ) )
	ssiType = serializedData["ssi_type"].toUInt();

	Oscar::SSI item( ssiName, ssiGid, ssiBid, ssiType, QValueList<TLV>(), 0 );
	item.setWaitingAuth( ssiWaitingAuth );
	ICQContact *c = new ICQContact( account, contactId, metaContact, QString::null, item );
	return c;
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

ICQ::OnlineStatusManager *ICQProtocol::statusManager()
{
	return statusManager_;
}

//END class ICQProtocol

#include "icqprotocol.moc"
// kate: indent-mode csands;
// vim: set noet ts=4 sts=4 sw=4:
