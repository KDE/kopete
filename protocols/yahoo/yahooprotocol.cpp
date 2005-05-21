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
	Offline( Kopete::OnlineStatus::Offline, 0, this, 0x5a55aa56, QString::null,  i18n( "Offline" ), i18n( "Offline" ), Kopete::OnlineStatusManager::Offline ),
	Online( Kopete::OnlineStatus::Online, 25, this, 0, QString::null,  i18n( "Online" ), i18n( "Online" ), Kopete::OnlineStatusManager::Online ),
	BeRightBack( Kopete::OnlineStatus::Away, 10, this, 1, "contact_away_overlay",  i18n( "Be right back" ), i18n( "Be right back" ), Kopete::OnlineStatusManager::Away ),
	Busy( Kopete::OnlineStatus::Away, 10, this, 2, "contact_busy_overlay",  i18n( "Busy" ), i18n( "Busy" ), Kopete::OnlineStatusManager::Away ),
	NotAtHome( Kopete::OnlineStatus::Away, 10, this, 3, "contact_xa_overlay",  i18n( "Not at home" ), i18n( "Not at home" ), Kopete::OnlineStatusManager::Away ),
	NotAtMyDesk( Kopete::OnlineStatus::Away, 10, this, 4, "contact_xa_overlay", i18n( "Not at my desk"), i18n( "Not at my desk"), Kopete::OnlineStatusManager::Away ),
	NotInTheOffice( Kopete::OnlineStatus::Away, 10, this, 5, "contact_xa_overlay", i18n( "Not in the office" ), i18n( "Not in the office" ), Kopete::OnlineStatusManager::Away ),
	OnThePhone( Kopete::OnlineStatus::Away, 10, this, 6, "contact_phone_overlay",  i18n( "On the phone" ), i18n( "On the phone" ), Kopete::OnlineStatusManager::Away ),
	OnVacation( Kopete::OnlineStatus::Away, 10, this, 7, "contact_xa_overlay",  i18n( "On vacation" ), i18n( "On vacation" ), Kopete::OnlineStatusManager::Away ),
	OutToLunch( Kopete::OnlineStatus::Away, 10, this, 8, "contact_food_overlay",  i18n( "Out to lunch" ), i18n( "Out to lunch" ), Kopete::OnlineStatusManager::Away ),
	SteppedOut( Kopete::OnlineStatus::Away, 10, this, 9, "contact_away_overlay",  i18n( "Stepped out" ), i18n( "Stepped out" ), Kopete::OnlineStatusManager::Away ),
	Invisible( Kopete::OnlineStatus::Invisible, 0, this, 12, "contact_invisible_overlay",  i18n( "Invisible" ), i18n( "Invisible" ), Kopete::OnlineStatusManager::Away ),
	Custom( Kopete::OnlineStatus::Away, 20, this, 99, "contact_away_overlay", i18n( "Custom" ), i18n( "Custom" ), Kopete::OnlineStatusManager::Online ),
	Idle( Kopete::OnlineStatus::Away, 15, this, 999, "yahoo_idle",  i18n( "Idle" ), i18n( "Idle" ), Kopete::OnlineStatusManager::Busy ),
	Connecting( Kopete::OnlineStatus::Connecting, 2, this, 555, "yahoo_connecting", i18n( "Connecting" ) ),
	awayMessage(Kopete::Global::Properties::self()->awayMessage())

{
	kdDebug(14180) << k_funcinfo << endl;

	s_protocolStatic_ = this;
	setCapabilities( BaseFgColor | BaseFormatting | BaseFont );
	addAddressBookField( "messaging/yahoo", Kopete::Plugin::MakeIndexField );
}


YahooProtocol::~YahooProtocol()
{
	kdDebug(14180) << k_funcinfo << endl;
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
	{	kdDebug( 14180 ) << k_funcinfo << "Account " << accountId << " not found" << endl;
		return 0;
	}

	if(theAccount->contact(contactId))
	{	kdDebug( 14180 ) << k_funcinfo << "User " << contactId << " already in contacts map" << endl;
		return 0;
	}

	theAccount->addContact(contactId,  metaContact, Kopete::Account::DontChangeKABC);
	return theAccount->contacts()[contactId];
}

AddContactPage *YahooProtocol::createAddContactWidget( QWidget * parent , Kopete::Account* )
{
	kdDebug(14180) << "YahooProtocol::createAddContactWidget(<parent>)" << endl;
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

