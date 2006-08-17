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

/* QT Includes */

/* KDE Includes */
#include <kdebug.h>
#include <kgenericfactory.h>
#include <ksimpleconfig.h>

/* Local Includes */
#include "yahooprotocol.h"
#include "yahooaccount.h"
#include "yahooaddcontact.h"
#include "yahooeditaccount.h"

/* Kopete Includes */
#include "kopeteaccountmanager.h"
#include "kopeteonlinestatusmanager.h"
#include "kopeteglobal.h"

typedef KGenericFactory<YahooProtocol> YahooProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_yahoo, YahooProtocolFactory( "kopete_yahoo" )  )

YahooProtocol::YahooProtocol( QObject *parent, const char *name, const QStringList & )
	: Kopete::Protocol( YahooProtocolFactory::instance(), parent, name ),
	Offline( Kopete::OnlineStatus::Offline,      0, this, 0x5a55aa56, QString::null,  i18n( "Offline" ),       i18n( "Offline" ),     Kopete::OnlineStatusManager::Offline ),
	Online( Kopete::OnlineStatus::Online,       25, this, 0, QString::null,           i18n( "Online" ),        i18n( "Online" ),      Kopete::OnlineStatusManager::Online, Kopete::OnlineStatusManager::HasAwayMessage  ),
	BeRightBack( Kopete::OnlineStatus::Away,    22, this, 1, "contact_away_overlay",  i18n( "Be right back" ), i18n( "Be right back" ) ),
	Busy( Kopete::OnlineStatus::Away,           20, this, 2, "contact_busy_overlay",  i18n( "Busy" ),          i18n( "Busy" ),        Kopete::OnlineStatusManager::Busy, Kopete::OnlineStatusManager::HasAwayMessage ),
	NotAtHome( Kopete::OnlineStatus::Away,      17, this, 3, "contact_xa_overlay",    i18n( "Not at home" ),   i18n( "Not at home" ), Kopete::OnlineStatusManager::ExtendedAway ),
	NotAtMyDesk( Kopete::OnlineStatus::Away,    18, this, 4, "contact_xa_overlay",    i18n( "Not at my desk"), i18n( "Not at my desk"), Kopete::OnlineStatusManager::Away ),
	NotInTheOffice( Kopete::OnlineStatus::Away, 16, this, 5, "contact_xa_overlay",    i18n( "Not in the office" ), i18n( "Not in the office" ) ),
	OnThePhone( Kopete::OnlineStatus::Away,     12, this, 6, "contact_phone_overlay", i18n( "On the phone" ), i18n( "On the phone" ) ),
	OnVacation( Kopete::OnlineStatus::Away,      3, this, 7, "contact_xa_overlay",    i18n( "On vacation" ),  i18n( "On vacation" ) ),
	OutToLunch( Kopete::OnlineStatus::Away,     10, this, 8, "contact_food_overlay",  i18n( "Out to lunch" ), i18n( "Out to lunch" ) ),
	SteppedOut( Kopete::OnlineStatus::Away,     14, this, 9, "contact_away_overlay",  i18n( "Stepped out" ),  i18n( "Stepped out" ) ),
	Invisible( Kopete::OnlineStatus::Invisible,  3, this, 12, "contact_invisible_overlay",  i18n( "Invisible" ), i18n( "Invisible" ), Kopete::OnlineStatusManager::Invisible ),
	Custom( Kopete::OnlineStatus::Away,         25, this, 99, "contact_busy_overlay", i18n( "Custom" ),       i18n( "Custom" ),	Kopete::OnlineStatusManager::HideFromMenu ),
	Idle( Kopete::OnlineStatus::Away,           15, this, 999, "yahoo_idle",          i18n( "Idle" ),         i18n( "Idle" ),         Kopete::OnlineStatusManager::Idle ),
	Connecting( Kopete::OnlineStatus::Connecting,2, this, 555, "yahoo_connecting",    i18n( "Connecting" ) ),
	awayMessage(Kopete::Global::Properties::self()->awayMessage()),
	iconCheckSum("iconCheckSum", i18n("Buddy Icon Checksum"), QString::null, true, false, true),
	iconExpire("iconExpire", i18n("Buddy Icon Expire"), QString::null, true, false, true),
	iconRemoteUrl("iconRemoteUrl", i18n("Buddy Icon Remote Url"), QString::null, true, false, true),
	propfirstName(Kopete::Global::Properties::self()->firstName()),
	propSecondName(),
	propLastName(Kopete::Global::Properties::self()->lastName()),
	propNickName(Kopete::Global::Properties::self()->nickName()),
	propTitle("YABTitle", i18n("Title"), QString::null, true, false),
	propPhoneMobile(Kopete::Global::Properties::self()->privateMobilePhone()),
	propEmail(Kopete::Global::Properties::self()->emailAddress()),
	propYABId("YABId", i18n("YAB Id"), QString::null, true, false, true),
	propPager("YABPager", i18n("Pager number"), QString::null, true, false),
	propFax("YABFax", i18n("Fax number"), QString::null, true, false),
	propAdditionalNumber("YABAdditionalNumber", i18n("Additional number"), QString::null, true, false),
	propAltEmail1("YABAlternativeEmail1", i18n("Alternative email 1"), QString::null, true, false),
	propAltEmail2("YABAlternativeEmail2", i18n("Alternative email 1"), QString::null, true, false),
	propImAIM("YABIMAIM", i18n("AIM"), QString::null, true, false),
	propImICQ("YABIMICQ", i18n("ICQ"), QString::null, true, false),
	propImMSN("YABIMMSN", i18n("MSN"), QString::null, true, false),
	propImGoogleTalk("YABIMGoogleTalk", i18n("GoogleTalk"), QString::null, true, false),
	propImSkype("YABIMSkype", i18n("Skype"), QString::null, true, false),
	propImIRC("YABIMIRC", i18n("IRC"), QString::null, true, false),
	propImQQ("YABIMQQ", i18n("QQ"), QString::null, true, false),
	propPrivateAddress("YABPrivateAddress", i18n("Private Address"), QString::null, true, false),
	propPrivateCity("YABPrivateCity", i18n("Private City"), QString::null, true, false),
	propPrivateState("YABPrivateState", i18n("Private State"), QString::null, true, false),
	propPrivateZIP("YABPrivateZIP", i18n("Private ZIP"), QString::null, true, false),
	propPrivateCountry("YABPrivateCountry", i18n("Private Country"), QString::null, true, false),
	propPrivatePhone(Kopete::Global::Properties::self()->privatePhone()),
	propPrivateURL("YABPrivateURL", i18n("Private URL"), QString::null, true, false),
	propCorporation("YABCorporation", i18n("Corporation"), QString::null, true, false),
	propWorkAddress("YABWorkAddress", i18n("Work Address"), QString::null, true, false),
	propWorkCity("YABWorkCity", i18n("Work City"), QString::null, true, false),
	propWorkState("YABWorkState", i18n("Work State"), QString::null, true, false),
	propWorkZIP("YABWorkZIP", i18n("Work ZIP"), QString::null, true, false),
	propWorkCountry("YABWorkCountry", i18n("Work Country"), QString::null, true, false),
	propWorkPhone(Kopete::Global::Properties::self()->workPhone()),
	propWorkURL("YABWorkURL", i18n("Work URL"), QString::null, true, false),
	propBirthday("YABBirthday", i18n("Birthday"), QString::null, true, false),
	propAnniversary("YABAnniversary", i18n("Anniversary"), QString::null, true, false),
	propNotes("YABNotes", i18n("Notes"), QString::null, true, false),
	propAdditional1("YABAdditional1", i18n("Additional 1"), QString::null, true, false),
	propAdditional2("YABAdditional2", i18n("Additional 2"), QString::null, true, false),
	propAdditional3("YABAdditional3", i18n("Additional 3"), QString::null, true, false),
	propAdditional4("YABAdditional4", i18n("Additional 4"), QString::null, true, false)
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;

	s_protocolStatic_ = this;
	setCapabilities( RichFgColor | RichFormatting | RichFont );
	addAddressBookField( "messaging/yahoo", Kopete::Plugin::MakeIndexField );
}


YahooProtocol::~YahooProtocol()
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;
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
	QString contactId = serializedData[ "contactId" ];
	QString accountId = serializedData[ "accountId" ];

	YahooAccount *theAccount = static_cast<YahooAccount*>(Kopete::AccountManager::self()->findAccount(protocol()->pluginId(), accountId));

	if(!theAccount)
	{	kdDebug( YAHOO_GEN_DEBUG ) << k_funcinfo << "Account " << accountId << " not found" << endl;
		return 0;
	}

	if(theAccount->contact(contactId))
	{	kdDebug( YAHOO_GEN_DEBUG ) << k_funcinfo << "User " << contactId << " already in contacts map" << endl;
		return 0;
	}

	theAccount->addContact(contactId,  metaContact, Kopete::Account::DontChangeKABC);
	return theAccount->contacts()[contactId];
}

AddContactPage *YahooProtocol::createAddContactWidget( QWidget * parent , Kopete::Account* )
{
	kdDebug(YAHOO_GEN_DEBUG) << "YahooProtocol::createAddContactWidget(<parent>)" << endl;
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

