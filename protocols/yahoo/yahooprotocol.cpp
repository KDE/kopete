/*
    yahooprotocol.cpp - Yahoo Plugin for Kopete

    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2003-2004 by Matt Rogers <matt@matt.rogers.name>

    Copyright (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
/* Own Header */
#include "yahooprotocol.h"

/* QT Includes */

/* KDE Includes */
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kconfig.h>

/* Local Includes */
#include "yahooaccount.h"
#include "yahooaddcontact.h"
#include "yahooeditaccount.h"

/* Kopete Includes */
#include "kopeteaccountmanager.h"
#include "kopeteonlinestatusmanager.h"
#include "kopeteglobal.h"
#include "kopeteproperty.h"
#include "kopetecontact.h"

K_PLUGIN_FACTORY( YahooProtocolFactory, registerPlugin<YahooProtocol>(); )
K_EXPORT_PLUGIN( YahooProtocolFactory( "kopete_yahoo" ) )

YahooProtocol::YahooProtocol( QObject *parent, const QVariantList & )
    : Kopete::Protocol( parent ),
	Offline( Kopete::OnlineStatus::Offline,      0, this, 0x5a55aa56, QStringList(),  i18n( "Offline" ),       i18n( "Offline" ),     Kopete::OnlineStatusManager::Offline ),
	Online( Kopete::OnlineStatus::Online,       25, this, 0, QStringList(),           i18n( "Online" ),        i18n( "Online" ),      Kopete::OnlineStatusManager::Online, Kopete::OnlineStatusManager::HasStatusMessage  ),
	BeRightBack( Kopete::OnlineStatus::Away,    22, this, 1, QStringList(QLatin1String("contact_away_overlay")),  i18n( "Be right back" ), i18n( "Be right back" ) ),
	Busy( Kopete::OnlineStatus::Busy,           20, this, 2, QStringList(QLatin1String("contact_busy_overlay")),  i18n( "Busy" ),          i18n( "Busy" ),        Kopete::OnlineStatusManager::Busy, Kopete::OnlineStatusManager::HasStatusMessage ),
	NotAtHome( Kopete::OnlineStatus::Away,      17, this, 3, QStringList(QLatin1String("contact_xa_overlay")),    i18n( "Not at home" ),   i18n( "Not at home" ), Kopete::OnlineStatusManager::ExtendedAway ),
	NotAtMyDesk( Kopete::OnlineStatus::Away,    18, this, 4, QStringList(QLatin1String("contact_xa_overlay")),    i18n( "Not at my desk"), i18n( "Not at my desk"), Kopete::OnlineStatusManager::Away ),
	NotInTheOffice( Kopete::OnlineStatus::Away, 16, this, 5, QStringList(QLatin1String("contact_xa_overlay")),    i18n( "Not in the office" ), i18n( "Not in the office" ) ),
	OnThePhone( Kopete::OnlineStatus::Busy,     12, this, 6, QStringList(QLatin1String("contact_phone_overlay")), i18n( "On the phone" ), i18n( "On the phone" ) ),
	OnVacation( Kopete::OnlineStatus::Busy,      3, this, 7, QStringList(QLatin1String("contact_xa_overlay")),    i18n( "On vacation" ),  i18n( "On vacation" ) ),
	OutToLunch( Kopete::OnlineStatus::Away,     10, this, 8, QStringList(QLatin1String("contact_food_overlay")),  i18n( "Out to lunch" ), i18n( "Out to lunch" ) ),
	SteppedOut( Kopete::OnlineStatus::Away,     14, this, 9, QStringList(QLatin1String("contact_away_overlay")),  i18n( "Stepped out" ),  i18n( "Stepped out" ) ),
	OnSMS( Kopete::OnlineStatus::Busy,     20, this, 10, QStringList(QLatin1String("contact_phone_overlay")),  i18n( "I'm On SMS" ),  i18n( "I'm On SMS" ) ),
	Invisible( Kopete::OnlineStatus::Invisible,  3, this, 12, QStringList(QLatin1String("contact_invisible_overlay")),  i18n( "Invisible" ), i18n( "Invisible" ), Kopete::OnlineStatusManager::Invisible ),
	Custom( Kopete::OnlineStatus::Away,         25, this, 99, QStringList(QLatin1String("contact_busy_overlay")), i18n( "Custom" ),       i18n( "Custom" ),	0, Kopete::OnlineStatusManager::HideFromMenu ),
	Idle( Kopete::OnlineStatus::Away,           15, this, 999, QStringList(QLatin1String("yahoo_idle")),          i18n( "Idle" ),         i18n( "Idle" ),         Kopete::OnlineStatusManager::Idle ),
	Connecting( Kopete::OnlineStatus::Connecting,2, this, 555, QStringList(QLatin1String("yahoo_connecting")),    i18n( "Connecting" ), i18n("Connecting"), 0, Kopete::OnlineStatusManager::HideFromMenu ),
	iconCheckSum(QStringLiteral("iconCheckSum"), i18n("Buddy Icon Checksum"), QString(), Kopete::PropertyTmpl::PersistentProperty | Kopete::PropertyTmpl::PrivateProperty),
	iconExpire(QStringLiteral("iconExpire"), i18n("Buddy Icon Expires"), QString(), Kopete::PropertyTmpl::PersistentProperty | Kopete::PropertyTmpl::PrivateProperty),
	iconRemoteUrl(QStringLiteral("iconRemoteUrl"), i18n("Buddy Icon Remote URL"), QString(), Kopete::PropertyTmpl::PersistentProperty | Kopete::PropertyTmpl::PrivateProperty),
	propfirstName(Kopete::Global::Properties::self()->firstName()),
	propSecondName(),
	propLastName(Kopete::Global::Properties::self()->lastName()),
	propNickName(Kopete::Global::Properties::self()->nickName()),
	propTitle(QStringLiteral("YABTitle"), i18n("Title"), QString(), Kopete::PropertyTmpl::PersistentProperty ),
	propPhoneMobile(Kopete::Global::Properties::self()->privateMobilePhone()),
	propEmail(Kopete::Global::Properties::self()->emailAddress()),
	propYABId(QStringLiteral("YABId"), i18n("YAB Id"), QString(), Kopete::PropertyTmpl::PersistentProperty | Kopete::PropertyTmpl::PrivateProperty),
	propPager(QStringLiteral("YABPager"), i18n("Pager number"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propFax(QStringLiteral("YABFax"), i18n("Fax number"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propAdditionalNumber(QStringLiteral("YABAdditionalNumber"), i18n("Additional number"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propAltEmail1(QStringLiteral("YABAlternativeEmail1"), i18n("Alternative email 1"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propAltEmail2(QStringLiteral("YABAlternativeEmail2"), i18n("Alternative email 1"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propImAIM(QStringLiteral("YABIMAIM"), i18n("AIM"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propImICQ(QStringLiteral("YABIMICQ"), i18n("ICQ"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propImMSN(QStringLiteral("YABIMMSN"), i18n("MSN"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propImGoogleTalk(QStringLiteral("YABIMGoogleTalk"), i18n("GoogleTalk"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propImSkype(QStringLiteral("YABIMSkype"), i18n("Skype"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propImIRC(QStringLiteral("YABIMIRC"), i18n("IRC"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propImQQ(QStringLiteral("YABIMQQ"), i18n("QQ"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propPrivateAddress(QStringLiteral("YABPrivateAddress"), i18n("Private Address"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propPrivateCity(QStringLiteral("YABPrivateCity"), i18n("Private City"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propPrivateState(QStringLiteral("YABPrivateState"), i18n("Private State"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propPrivateZIP(QStringLiteral("YABPrivateZIP"), i18n("Private ZIP"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propPrivateCountry(QStringLiteral("YABPrivateCountry"), i18n("Private Country"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propPrivatePhone(Kopete::Global::Properties::self()->privatePhone()),
	propPrivateURL(QStringLiteral("YABPrivateURL"), i18n("Private URL"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propCorporation(QStringLiteral("YABCorporation"), i18n("Corporation"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propWorkAddress(QStringLiteral("YABWorkAddress"), i18n("Work Address"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propWorkCity(QStringLiteral("YABWorkCity"), i18n("Work City"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propWorkState(QStringLiteral("YABWorkState"), i18n("Work State"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propWorkZIP(QStringLiteral("YABWorkZIP"), i18n("Work ZIP"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propWorkCountry(QStringLiteral("YABWorkCountry"), i18n("Work Country"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propWorkPhone(Kopete::Global::Properties::self()->workPhone()),
	propWorkURL(QStringLiteral("YABWorkURL"), i18n("Work URL"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propBirthday(QStringLiteral("YABBirthday"), i18n("Birthday"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propAnniversary(QStringLiteral("YABAnniversary"), i18n("Anniversary"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propNotes(QStringLiteral("YABNotes"), i18n("Notes"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propAdditional1(QStringLiteral("YABAdditional1"), i18n("Additional 1"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propAdditional2(QStringLiteral("YABAdditional2"), i18n("Additional 2"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propAdditional3(QStringLiteral("YABAdditional3"), i18n("Additional 3"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propAdditional4(QStringLiteral("YABAdditional4"), i18n("Additional 4"), QString(), Kopete::PropertyTmpl::PersistentProperty)
{
	kDebug(YAHOO_GEN_DEBUG) ;

	s_protocolStatic_ = this;
	setCapabilities( RichFgColor | RichFormatting | RichFont );
	addAddressBookField( QStringLiteral("messaging/yahoo"), Kopete::Plugin::MakeIndexField );
}


YahooProtocol::~YahooProtocol()
{
	kDebug(YAHOO_GEN_DEBUG) ;
	s_protocolStatic_ = 0L;
}

YahooProtocol* YahooProtocol::s_protocolStatic_ = 0L;

Kopete::OnlineStatus YahooProtocol::statusFromYahoo( int status )
{
	switch ( status )
	{
		case 0 :
			return Online;
		case 1:
			return BeRightBack;
		case 2:
			return Busy;
		case 3:
			return NotAtHome;
		case 4:
			return NotAtMyDesk;
		case 5:
			return NotInTheOffice;
		case 6:
			return OnThePhone;
		case 7:
			return OnVacation;
		case 8:
			return OutToLunch;
		case 9:
			return SteppedOut;
		case 10:
			return OnSMS;
		case 12:
			return Invisible;
		case 99:
			return Custom;
		case 999:
			return Idle;
		case 0x5a55aa56:
			return Offline;
	}

	return Offline;
}

/***************************************************************************
 *                                                                         *
 *   Re-implementation of Plugin class methods                             *
 *                                                                         *
 ***************************************************************************/

YahooProtocol *YahooProtocol::protocol()
{
	return s_protocolStatic_;
}

Kopete::Contact *YahooProtocol::deserializeContact( Kopete::MetaContact *metaContact,
	const QMap<QString, QString> &serializedData, const QMap<QString, QString> & /* addressBookData */ )
{
	QString contactId = serializedData[ QStringLiteral("contactId") ];
	QString accountId = serializedData[ QStringLiteral("accountId") ];
	Kopete::Contact::NameType nameType = Kopete::Contact::nameTypeFromString(serializedData[ QStringLiteral("preferredNameType") ]);

	YahooAccount *theAccount = static_cast<YahooAccount*>(Kopete::AccountManager::self()->findAccount(protocol()->pluginId(), accountId));

	if(!theAccount)
	{	kDebug( YAHOO_GEN_DEBUG ) << "Account " << accountId << " not found";
		return 0;
	}

	if(theAccount->contact(contactId))
	{	kDebug( YAHOO_GEN_DEBUG ) << "User " << contactId << " already in contacts map";
		return 0;
	}

	theAccount->addContact(contactId,  metaContact, Kopete::Account::DontChangeKABC);

	Kopete::Contact *c = theAccount->contacts().value(contactId);
	if (!c)
		return 0;

	c->setPreferredNameType(nameType);
	return c;
}

AddContactPage *YahooProtocol::createAddContactWidget( QWidget * parent , Kopete::Account* )
{
	kDebug(YAHOO_GEN_DEBUG) << "YahooProtocol::createAddContactWidget(<parent>)";
	return new YahooAddContact(this, parent);
}

KopeteEditAccountWidget *YahooProtocol::createEditAccountWidget(Kopete::Account *account, QWidget *parent)
{
	return new YahooEditAccount(this, account, parent);
}

Kopete::Account *YahooProtocol::createNewAccount(const QString &accountId)
{
	return new YahooAccount(this, accountId);
}

#include "yahooprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

