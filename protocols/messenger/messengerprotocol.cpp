/*
 * messengerprotocol.cpp - Windows Live Messenger Kopete protocol definition.
 *
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

// KDE includes
#include <kgenericfactory.h>

// Kopete includes
#include <kopeteaccount.h>
#include <kopetemetacontact.h>

typedef KGenericFactory<MessengerProtocol> MessengerProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_messenger, MessengerProtocolFactory("kopete_messenger") )

MessengerProtocol *MessengerProtocol::s_self = 0;

MessengerProtocol::MessengerProtocol(QObject *parent, const QStringList &/*args*/)
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
	
	propGuid("guid", i18n("Contact GUID"), 0, Kopete::ContactPropertyTmpl::PersistentProperty),

	propEmail(Kopete::Global::Properties::self()->emailAddress()),
	propContactType("MessengerContactType", i18n("Contact Type"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propFirstName("MessengerFirstName", i18n("First Name"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propLastName("MessengerLastName", i18n("Last Name"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propComment("MessengerComment", i18n("Comment"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propAnniversary("MessengerAnniversary", i18n("Anniversary"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propBirthday("MessengerBirthday", i18n("Birthday"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),

	//Annotation
	propABJobTitle("MessengerABJobTitle", i18n("ABJobTitle"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propABNickName("MessengerABNickName", i18n("ABNickName"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propABJobSpouse("MessengerABJobSpouse", i18n("ABJobSpouse"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),

	//Email
	propContactEmailBusiness("MessengerContactEmailBusiness", i18n("ContactEmailBusiness"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propContactEmailMessenger("MessengerContactEmailMessenger", i18n("ContactEmailMessenger"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propContactEmailOther("MessengerContactEmailOther", i18n("ContactEmailOther"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propContactEmailPersonal("MessengerContactEmailPersonal", i18n("ContactEmailPersonal"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),

	//Phone
//	propContactPhoneBusiness("MessengerContactPhoneBusiness", i18n("ContactPhoneBusiness"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propContactPhoneBusiness(Kopete::Global::Properties::self()->workPhone()),
	propContactPhoneFax("MessengerContactPhoneFax", i18n("ContactPhoneFax"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
//	propContactPhoneMobile("MessengerContactPhoneMobile", i18n("ContactPhoneMobile"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propContactPhoneMobile(Kopete::Global::Properties::self()->privateMobilePhone()),
	propContactPhoneOther("MessengerContactPhoneOther", i18n("ContactPhoneOther"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propContactPhonePager("MessengerContactPhonePager", i18n("ContactPhonePager"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
//	propContactPhonePersonal("MessengerContactPhonePersonal", i18n("ContactPhonePersonal"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propContactPhonePersonal(Kopete::Global::Properties::self()->privatePhone()),

	//Business Location
	propBusinessName("MessengerBusinessName", i18n("BusinessName"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propBusinessStreet("MessengerBusinessStreet", i18n("BusinessStreet"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propBusinessCity("MessengerBusinessCity", i18n("BusinessCity"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propBusinessState("MessengerBusinessState", i18n("BusinessState"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propBusinessCountry("MessengerBusinessCountry", i18n("BusinessCountry"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propBusinessPostalCode("MessengerBusinessPostalCode", i18n("BusinessPostalCode"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),

	//Personal Location
	propPersonalName("MessengerPersonalName", i18n("PersonalName"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propPersonalStreet("MessengerPersonalStreet", i18n("PersonalStreet"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propPersonalCity("MessengerPersonalCity", i18n("PersonalCity"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propPersonalState("MessengerPersonalState", i18n("PersonalState"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propPersonalCountry("MessengerPersonalCountry", i18n("PersonalCountry"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propPersonalPostalCode("MessengerPersonalPostalCode", i18n("PersonalPostalCode"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),

	//Website
	propContactWebSiteBusiness("MessengerContactWebSiteBusiness", i18n("ContactWebSiteBusiness"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),
	propContactWebSitePersonal("MessengerContactWebSitePersonal", i18n("ContactWebSitePersonal"), QString(), Kopete::ContactPropertyTmpl::PersistentProperty),

	propPhoneHome(Kopete::Global::Properties::self()->privatePhone()),
	propPhoneWork(Kopete::Global::Properties::self()->workPhone()),
	propPhoneMobile(Kopete::Global::Properties::self()->privateMobilePhone()),
	propClient("client", i18n("Remote Client"), 0),
	propPersonalMessage(Kopete::Global::Properties::self()->statusMessage())
{
	s_self = this;

	addAddressBookField( "messaging/messenger", Kopete::Plugin::MakeIndexField );

	//setting capabilities
	setCapabilities( Kopete::Protocol::BaseFgColor | Kopete::Protocol::BaseFont | Kopete::Protocol::BaseFormatting );
//	setCapabilities( Kopete::Protocol::FullRTF );

	initCountries();
	initMonths();
	initDays();
}

MessengerProtocol *MessengerProtocol::protocol()
{
	return s_self;
}

Kopete::Account *MessengerProtocol::createNewAccount(const QString &accountId)
{
	return new MessengerAccount(this, accountId);
}

AddContactPage *MessengerProtocol::createAddContactWidget(QWidget *parent, Kopete::Account *account)
{
	return new MessengerAddContactPage( static_cast<MessengerAccount*>account , parent );
}

KopeteEditAccountWidget *MessengerProtocol::createEditAccountWidget(Kopete::Account *account, QWidget *parent)
{
	/*create the Edit Account UI*/
	return new MessengerEditAccountWidget(this,account,parent);
}

/*check the ContactId is a valid Windows Live ID*/
bool MessengerProtocol::validContactId(const QString& userid)
{
	return( userid.count("@") ==1 && userid.count(".") >=1 /*&& userid.count(QChar(' ')) == 1*/ );
}

Kopete::Contact *MessengerProtocol::deserializeContact( Kopete::MetaContact *metaContact, 
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData )
{
	return 0;
}

/*scale picture to size [96,96] */
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
}

void MessengerProtocol::initCountries()
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
	mCountries.insert(102, i18n("Antigua"));
	mCountries.insert(1021, kl->twoAlphaToCountryName("ag"));
	mCountries.insert(54, kl->twoAlphaToCountryName("ar"));
	mCountries.insert(374, kl->twoAlphaToCountryName("am"));
	mCountries.insert(297, kl->twoAlphaToCountryName("aw"));
	mCountries.insert(247, i18n("Ascension Island"));
	mCountries.insert(61, kl->twoAlphaToCountryName("au"));
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
	mCountries.insert(178, i18n("Canary Islands"));
	mCountries.insert(56, kl->twoAlphaToCountryName("cl"));
	mCountries.insert(86, kl->twoAlphaToCountryName("cn"));
	mCountries.insert(672, kl->twoAlphaToCountryName("cx"));
	mCountries.insert(6101, i18n("Cocos-Keeling Islands"));
	mCountries.insert(6102, i18n("Cocos (Keeling) Islands"));
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
	mCountries.insert(5902, i18n("Antilles"));
	mCountries.insert(594, i18n("French Guiana"));
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
	mCountries.insert(502, kl->twoAlphaToCountryName("gt"));
	mCountries.insert(224, kl->twoAlphaToCountryName("gn"));
	mCountries.insert(245, kl->twoAlphaToCountryName("gw"));
	mCountries.insert(592, kl->twoAlphaToCountryName("gy"));
	mCountries.insert(509, kl->twoAlphaToCountryName("ht"));
	mCountries.insert(504, kl->twoAlphaToCountryName("hn"));
	mCountries.insert(852, kl->twoAlphaToCountryName("hk"));
	mCountries.insert(36, kl->twoAlphaToCountryName("hu"));
	mCountries.insert(354, kl->twoAlphaToCountryName("is"));
	mCountries.insert(91, kl->twoAlphaToCountryName("in"));
	mCountries.insert(62, kl->twoAlphaToCountryName("id"));
	mCountries.insert(98, kl->twoAlphaToCountryName("ir"));
	mCountries.insert(964, kl->twoAlphaToCountryName("iq"));
	mCountries.insert(353, kl->twoAlphaToCountryName("ie"));
	mCountries.insert(972, kl->twoAlphaToCountryName("il"));
	mCountries.insert(39, kl->twoAlphaToCountryName("it"));
	mCountries.insert(225, kl->twoAlphaToCountryName("ci"));
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
	mCountries.insert(961, kl->twoAlphaToCountryName("lb"));
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
	mCountries.insert(269, i18n("Mayotte Island"));
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
	mCountries.insert(670, i18n("Saipan Island"));
	mCountries.insert(378, kl->twoAlphaToCountryName("sm"));
	mCountries.insert(239, kl->twoAlphaToCountryName("st"));
	mCountries.insert(966, kl->twoAlphaToCountryName("sa"));
	mCountries.insert(221, kl->twoAlphaToCountryName("sn"));
	mCountries.insert(248, kl->twoAlphaToCountryName("sc"));
	mCountries.insert(232, i18n("Sierra Leone"));
	mCountries.insert(65, kl->twoAlphaToCountryName("sg"));
	mCountries.insert(4201, kl->twoAlphaToCountryName("sk"));
	mCountries.insert(386, kl->twoAlphaToCountryName("si"));
	mCountries.insert(677, kl->twoAlphaToCountryName("sb"));
	mCountries.insert(252, kl->twoAlphaToCountryName("so"));
	mCountries.insert(27, kl->twoAlphaToCountryName("za"));
	mCountries.insert(34, kl->twoAlphaToCountryName("es"));
	mCountries.insert(94, kl->twoAlphaToCountryName("lk"));
	mCountries.insert(290, kl->twoAlphaToCountryName("sh"));
	mCountries.insert(115, i18n("St. Kitts"));
	mCountries.insert(1141, kl->twoAlphaToCountryName("kn"));
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
	mCountries.insert(441, i18n("Wales"));
	mCountries.insert(442, i18n("Scotland"));
	mCountries.insert(123, kl->twoAlphaToCountryName("vi")); // United States Virgin Islands
	mCountries.insert(598, kl->twoAlphaToCountryName("uy")); // Uruguay
	mCountries.insert(711, kl->twoAlphaToCountryName("uz")); // Uzbekistan
	mCountries.insert(678, kl->twoAlphaToCountryName("vu")); // Vanuatu
	mCountries.insert(379, kl->twoAlphaToCountryName("va")); // Vatican City
	mCountries.insert(58, kl->twoAlphaToCountryName("ve")); // Venezuela
	mCountries.insert(84, kl->twoAlphaToCountryName("vn")); // Vietnam
	mCountries.insert(681, kl->twoAlphaToCountryName("wf")); // Wallis and Futuna Islands
	mCountries.insert(685, kl->twoAlphaToCountryName("ws"));
	mCountries.insert(967, kl->twoAlphaToCountryName("ye"));
	mCountries.insert(3811, i18n("Yugoslavia - Serbia"));
	mCountries.insert(382, i18n("Yugoslavia - Montenegro"));
	mCountries.insert(381, i18n("Yugoslavia"));
	mCountries.insert(243, i18n("Congo, Democratic Republic of (Zaire)"));
	mCountries.insert(260, kl->twoAlphaToCountryName("zm"));
	mCountries.insert(263, kl->twoAlphaToCountryName("zw"));
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
	QMap<QString, int> sortedMap( reverseMap( icqProtocol->countries() ) );
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

