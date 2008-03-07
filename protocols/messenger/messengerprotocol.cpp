/*
 * messengerprotocol.cpp - Windows Live Messenger Kopete protocol definition.
 *
 * Copyright (c) 2007 by Zhang Panyong <pyzhang@gmail.com>
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 * 
 * Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */

#include "messengerprotocol.h"

// Qt Includes
#include <QComboBox>
#include <QMap>

// KDE includes
#include <kgenericfactory.h>

// Kopete includes
#include <kopeteaccount.h>
#include <kopetemetacontact.h>

// Messenger includes
#include "messengeraccount.h"
//#include "messengercontact.h"
#include "messengeraddcontactpage.h"
#include "messengereditaccountwidget.h"
#include "ui/messengeraddcontactpage.h"

K_PLUGIN_FACTORY( MessengerProtocolFactory, registerPlugin<MessengerProtocol>(); )
K_EXPORT_PLUGIN( MessengerProtocolFactory( "kopete_messenger" ) )
MessengerProtocol *MessengerProtocol::protocolInstance = 0;

MessengerProtocol::MessengerProtocol(QObject *parent, const QVariantList &/*args*/)
 : Kopete::Protocol(MessengerProtocolFactory::componentData(), parent),

	NLN( Kopete::OnlineStatus::Online, 25, this, 1, QStringList(QString()),
			i18n( "Online" ), i18n( "O&nline" ), Kopete::OnlineStatusManager::Online, 
			Kopete::OnlineStatusManager::HasStatusMessage ),
	
	BSY( Kopete::OnlineStatus::Away, 20, this, 2, QStringList(QString::fromLatin1("messenger_busy")),
			i18n( "Busy" ), i18n( "&Busy" ), Kopete::OnlineStatusManager::Busy,  
			Kopete::OnlineStatusManager::HasStatusMessage ),
	
	BRB( Kopete::OnlineStatus::Away, 22, this, 3, QStringList(QString::fromLatin1("messenger_brb")),
			i18n( "Be Right Back" ), i18n( "Be &Right Back" ), 0,
			Kopete::OnlineStatusManager::HasStatusMessage ),
	
	AWY( Kopete::OnlineStatus::Away, 18, this, 4, QStringList(QString::fromLatin1("contact_away_overlay")),
			i18n( "Away From Computer" ), i18n( "&Away" ), Kopete::OnlineStatusManager::Away,  
			Kopete::OnlineStatusManager::HasStatusMessage ),
	
	PHN( Kopete::OnlineStatus::Away, 12, this, 5, QStringList(QString::fromLatin1("contact_phone_overlay")),
			i18n( "On the Phone" ), i18n( "On The &Phone" ), 0, Kopete::OnlineStatusManager::HasStatusMessage ),
	
	LUN( Kopete::OnlineStatus::Away, 15, this, 6, QStringList(QString::fromLatin1("contact_food_overlay")),   
			i18n( "Out to Lunch" ), i18n( "Out To &Lunch" ), 0, Kopete::OnlineStatusManager::HasStatusMessage ),
	
	FLN( Kopete::OnlineStatus::Offline, 0, this, 7, QStringList(QString()),
			i18n( "Offline" ), i18n( "&Offline" ), Kopete::OnlineStatusManager::Offline,
			Kopete::OnlineStatusManager::DisabledIfOffline ),
	
	HDN( Kopete::OnlineStatus::Invisible, 3, this, 8, QStringList(QString::fromLatin1("contact_invisible_overlay")), 
			i18n( "Invisible" ), i18n( "&Invisible" ), Kopete::OnlineStatusManager::Invisible ), 
	
	IDL( Kopete::OnlineStatus::Away, 10, this, 9, QStringList(QString::fromLatin1("contact_away_overlay")),      
			i18n( "Idle" ), i18n( "&Idle" ), Kopete::OnlineStatusManager::Idle , Kopete::OnlineStatusManager::HideFromMenu ),
	
	UNK( Kopete::OnlineStatus::Unknown, 25, this, 0, QStringList(QString::fromLatin1("status_unknown")), 
			i18n( "Status not available" ) ),
	
	CNT( Kopete::OnlineStatus::Connecting, 2, this, 10, QStringList(QString::fromLatin1("messenger_connecting")), 
			i18n( "Connecting" ) ),
	
	propGuid("guid", i18n("Contact GUID"), 0, Kopete::PropertyTmpl::PersistentProperty),

	propEmail(Kopete::Global::Properties::self()->emailAddress()),
	propContactType("MessengerContactType", i18n("Contact Type"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propFirstName("MessengerFirstName", i18n("First Name"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propLastName("MessengerLastName", i18n("Last Name"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propComment("MessengerComment", i18n("Comment"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propAnniversary("MessengerAnniversary", i18n("Anniversary"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propBirthday("MessengerBirthday", i18n("Birthday"), QString(), Kopete::PropertyTmpl::PersistentProperty),

	//Annotation
	propABJobTitle("MessengerABJobTitle", i18n("ABJobTitle"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propABNickName("MessengerABNickName", i18n("ABNickName"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propABJobSpouse("MessengerABJobSpouse", i18n("ABJobSpouse"), QString(), Kopete::PropertyTmpl::PersistentProperty),

	//Email
	propContactEmailBusiness("MessengerContactEmailBusiness", i18n("ContactEmailBusiness"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propContactEmailMessenger("MessengerContactEmailMessenger", i18n("ContactEmailMessenger"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propContactEmailOther("MessengerContactEmailOther", i18n("ContactEmailOther"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propContactEmailPersonal("MessengerContactEmailPersonal", i18n("ContactEmailPersonal"), QString(), Kopete::PropertyTmpl::PersistentProperty),

	//Phone
//	propContactPhoneBusiness("MessengerContactPhoneBusiness", i18n("ContactPhoneBusiness"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propContactPhoneBusiness(Kopete::Global::Properties::self()->workPhone()),
	propContactPhoneFax("MessengerContactPhoneFax", i18n("ContactPhoneFax"), QString(), Kopete::PropertyTmpl::PersistentProperty),
//	propContactPhoneMobile("MessengerContactPhoneMobile", i18n("ContactPhoneMobile"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propContactPhoneMobile(Kopete::Global::Properties::self()->privateMobilePhone()),
	propContactPhoneOther("MessengerContactPhoneOther", i18n("ContactPhoneOther"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propContactPhonePager("MessengerContactPhonePager", i18n("ContactPhonePager"), QString(), Kopete::PropertyTmpl::PersistentProperty),
//	propContactPhonePersonal("MessengerContactPhonePersonal", i18n("ContactPhonePersonal"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propContactPhonePersonal(Kopete::Global::Properties::self()->privatePhone()),

	//Business Location
	propBusinessName("MessengerBusinessName", i18n("BusinessName"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propBusinessStreet("MessengerBusinessStreet", i18n("BusinessStreet"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propBusinessCity("MessengerBusinessCity", i18n("BusinessCity"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propBusinessState("MessengerBusinessState", i18n("BusinessState"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propBusinessCountry("MessengerBusinessCountry", i18n("BusinessCountry"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propBusinessPostalCode("MessengerBusinessPostalCode", i18n("BusinessPostalCode"), QString(), Kopete::PropertyTmpl::PersistentProperty),

	//Personal Location
	propPersonalName("MessengerPersonalName", i18n("PersonalName"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propPersonalStreet("MessengerPersonalStreet", i18n("PersonalStreet"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propPersonalCity("MessengerPersonalCity", i18n("PersonalCity"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propPersonalState("MessengerPersonalState", i18n("PersonalState"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propPersonalCountry("MessengerPersonalCountry", i18n("PersonalCountry"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propPersonalPostalCode("MessengerPersonalPostalCode", i18n("PersonalPostalCode"), QString(), Kopete::PropertyTmpl::PersistentProperty),

	//Website
	propContactWebSiteBusiness("MessengerContactWebSiteBusiness", i18n("ContactWebSiteBusiness"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propContactWebSitePersonal("MessengerContactWebSitePersonal", i18n("ContactWebSitePersonal"), QString(), Kopete::PropertyTmpl::PersistentProperty),

	propPhoneHome(Kopete::Global::Properties::self()->privatePhone()),
	propPhoneWork(Kopete::Global::Properties::self()->workPhone()),
	propPhoneMobile(Kopete::Global::Properties::self()->privateMobilePhone()),
	propClient("client", i18n("Remote Client"), 0),
	propPersonalMessage(Kopete::Global::Properties::self()->statusMessage())
{
	protocolInstance = this;

	addAddressBookField( "messaging/messenger", Kopete::Plugin::MakeIndexField );

	//setting capabilities
	setCapabilities( Kopete::Protocol::BaseFgColor | Kopete::Protocol::BaseFont | Kopete::Protocol::BaseFormatting );
//	setCapabilities( Kopete::Protocol::FullRTF );

	initCountries();
	initMonths();
	initDays();
}

MessengerProtocol::~MessengerProtocol()
{
    delete protocolInstance;
}
MessengerProtocol *MessengerProtocol::protocol()
{
	return protocolInstance;
}

bool MessengerProtocol::validContactId(const QString& userid)
{
	return( userid.count("@") ==1 && userid.count(".") >=1 /*&& userid.count(QChar(' ')) == 1*/ );
}

Kopete::Account *MessengerProtocol::createNewAccount(const QString &accountId)
{
	// TODO
	return new MessengerAccount(this, accountId);
}

AddContactPage *MessengerProtocol::createAddContactWidget(QWidget *parent, Kopete::Account *account)
{
	MessengerAccount *messengerAccount=dynamic_cast<MessengerAccount*>(account);
	return (new MessengerAddContactPage(messengerAccount,parent));
}

KopeteEditAccountWidget *MessengerProtocol::createEditAccountWidget(Kopete::Account *account, QWidget *parent)
{
	/*create the Edit Account UI*/
	MessengerAccount *messengerAccount=dynamic_cast<MessengerAccount*>(account);
	return new MessengerEditAccountWidget(this,messengerAccount,parent);
}

/* check the ContactId is a valid Windows Live ID
   FIXME no definition in messengerprotocol.h
bool MessengerProtocol::validContactId(const QString& userid)
{
	return( userid.count("@") ==1 && userid.count(".") >=1); // && userid.count(QChar(' ')) == 1
} */

Kopete::Contact *MessengerProtocol::deserializeContact( Kopete::MetaContact *metaContact, 
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData )
{
	return 0;
}

/*scale picture to size [96,96] */
/* FIXME no definition in messengerprotocol.h 
QImage MessengerProtocol::scalePicture(const QImage &picture)
{
	QImage img(picture);
	img = img.scaled( 96, 96, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation );
	// crop image if not square
	if(img.width() < img.height())
	{
		img = img.copy((img.width()-img.height())/2, 0, 96, 96);
	}
	else if(img.width() > img.height())
	{
		img = img.copy(0, (img.height()-img.width())/2, 96, 96);
	}

	return img;
} */

void MessengerProtocol::initCountries()
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

void MessengerProtocol::initMonths()
{
	mMonths.insert(0,"--");
	mMonths.insert(1, i18nc("Long month name", "January"));
	mMonths.insert(2, i18nc("Long month name", "February")); 
	mMonths.insert(3, i18nc("Long month name", "March"));
	mMonths.insert(4, i18nc("Long month name", "April"));
	mMonths.insert(5, i18nc("Long month name", "May"));
	mMonths.insert(6, i18nc("Long month name", "June"));
	mMonths.insert(7, i18nc("Long month name", "July"));
	mMonths.insert(8, i18nc("Long month name", "August"));
	mMonths.insert(9, i18nc("Long month name", "September"));
	mMonths.insert(10, i18nc("Long month name", "October"));
	mMonths.insert(11, i18nc("Long month name", "November"));
	mMonths.insert(12, i18nc("Long month name", "December"));
}

void MessengerProtocol::initDays()
{
	mDays.insert(0,"--");
	for(qint32 i =1; i<=31; i++){
		mDays.insert(i,QString::number(i,10));
	}
}

void MessengerProtocol::fillComboFromTable(QComboBox *box, const QMap<int, QString> &map)
{
	kDebug(14153) << k_funcinfo << "Called." << endl;

#if 0
	QStringList list = map.values();
	list.sort();
	box->addItems(list);
#else
	QMap<QString, int> sortedMap( reverseMap( countries() ) );
	QMapIterator<QString, int> it( sortedMap );

	while ( it.hasNext() )
	{
		it.next();
		box->addItem( it.key(), it.value() );
	}
#endif
}

void MessengerProtocol::setComboFromTable(QComboBox *box, const QMap<int, QString> &map, int value)
{
//	kDebug(14153) << k_funcinfo << "Called." << endl;
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

int MessengerProtocol::getCodeForCombo(QComboBox *cmb, const QMap<int, QString> &map)
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

QMap<QString, int> MessengerProtocol::reverseMap( const QMap<int, QString>& map ) const
{
	QMap<QString, int> revMap;
	QMapIterator<int, QString> it( map );

	while ( it.hasNext() )
	{
		it.next();
		revMap.insert( it.value(), it.key() );
	}

	return revMap;
}

