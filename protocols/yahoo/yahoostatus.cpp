/*
    yahoostatus.cpp - Yahoo Plugin for Kopete

    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Copyright (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qstring.h>
#include <kdebug.h>
#include "yahoostatus.h"
#include "yahooprotocol.h"
#include "kopeteonlinestatus.h"

YahooStatus::YahooStatus( Status status_ )
{
	m_status = status_;
}

YahooStatus::YahooStatus()
{
	m_status = Offline;
}

KopeteOnlineStatus YahooStatus::translate() const
{
	if(m_status == Available)
		return KopeteOnlineStatus(KopeteOnlineStatus::Online,  25, YahooProtocol::protocol(), m_status, "yahoo_protocol",  i18n(YSTAvailable), i18n(YSTAvailable));
	else if(m_status == Mobile)
		return KopeteOnlineStatus(KopeteOnlineStatus::Away,     5, YahooProtocol::protocol(), m_status, "yahoo_mobile",  i18n("On the mobile"), i18n("On the mobile"));
	else if(m_status == Invisible)
		return KopeteOnlineStatus(KopeteOnlineStatus::Offline, 0, YahooProtocol::protocol(), m_status, "", i18n(YSTInvisible), i18n(YSTInvisible));
	else if(m_status == Idle)
		return KopeteOnlineStatus(KopeteOnlineStatus::Away,    15, YahooProtocol::protocol(), m_status, "yahoo_idle",    i18n("Idle"), i18n("Idle"));
	else if(m_status == Custom)
		return KopeteOnlineStatus(KopeteOnlineStatus::Away,    20, YahooProtocol::protocol(), m_status, "yahoo_away",    m_statusText, m_statusText);
	else if(m_status == CustomBusy)
		return KopeteOnlineStatus(KopeteOnlineStatus::Away,    20, YahooProtocol::protocol(), m_status, "yahoo_busy",    m_statusText, m_statusText);
	else if(m_status == CustomMobile)
		return KopeteOnlineStatus(KopeteOnlineStatus::Away,    20, YahooProtocol::protocol(), m_status, "yahoo_mobile",  m_statusText, m_statusText);
	else if(m_status == BeRightBack)
		return KopeteOnlineStatus(KopeteOnlineStatus::Away,    10, YahooProtocol::protocol(), m_status, "yahoo_away",    i18n(YSTBeRightBack), i18n(YSTBeRightBack));
	else if(m_status == Busy)
		return KopeteOnlineStatus(KopeteOnlineStatus::Away,    10, YahooProtocol::protocol(), m_status, "yahoo_busy",    i18n(YSTBusy), i18n(YSTBusy));
	else if(m_status == NotAtHome)
		return KopeteOnlineStatus(KopeteOnlineStatus::Away,    10, YahooProtocol::protocol(), m_status, "yahoo_away",    i18n(YSTNotAtHome), i18n(YSTNotAtHome));
	else if(m_status == NotAtMyDesk)
		return KopeteOnlineStatus(KopeteOnlineStatus::Away,    10, YahooProtocol::protocol(), m_status, "yahoo_away",    i18n(YSTNotAtMyDesk), i18n(YSTNotAtMyDesk));
	else if(m_status == NotInTheOffice)
		return KopeteOnlineStatus(KopeteOnlineStatus::Away,    10, YahooProtocol::protocol(), m_status, "yahoo_away",    i18n(YSTNotInTheOffice), i18n(YSTNotInTheOffice));
	else if(m_status == OnThePhone)
		return KopeteOnlineStatus(KopeteOnlineStatus::Away,    10, YahooProtocol::protocol(), m_status, "yahoo_mobile",  i18n(YSTOnThePhone), i18n(YSTOnThePhone));
	else if(m_status == OnVacation)
		return KopeteOnlineStatus(KopeteOnlineStatus::Away,    10, YahooProtocol::protocol(), m_status, "yahoo_away",    i18n(YSTOnVacation), i18n(YSTOnVacation));
	else if(m_status == OutToLunch)
		return KopeteOnlineStatus(KopeteOnlineStatus::Away,    10, YahooProtocol::protocol(), m_status, "yahoo_tea",    i18n(YSTOutToLunch), i18n(YSTOutToLunch));
	else if(m_status == SteppedOut)
		return KopeteOnlineStatus(KopeteOnlineStatus::Away,    10, YahooProtocol::protocol(), m_status, "yahoo_away",    i18n(YSTSteppedOut), i18n(YSTSteppedOut));
	else	// must be offline
		return KopeteOnlineStatus(KopeteOnlineStatus::Offline,  0, YahooProtocol::protocol(), m_status, "", i18n(YSTOffline), i18n(YSTOffline));
}

void YahooStatus::setStatus( Status status_, const QString &statusText_ )
{
	m_status = status_;
	m_statusText = statusText_;
}

YahooStatus::Status YahooStatus::fromLibYahoo2( int status_ )
{
	switch (status_)
	{
		case YAHOO_STATUS_AVAILABLE :
			return Available;
		case YAHOO_STATUS_BRB :
			return BeRightBack;
		case YAHOO_STATUS_BUSY :
			return Busy;
		case YAHOO_STATUS_NOTATHOME :
			return NotAtHome;
		case YAHOO_STATUS_NOTATDESK :
			return NotAtMyDesk;
		case YAHOO_STATUS_NOTINOFFICE :
			return NotInTheOffice;
		case YAHOO_STATUS_ONPHONE :
			return OnThePhone;
		case YAHOO_STATUS_ONVACATION :
			return OnVacation;
		case YAHOO_STATUS_OUTTOLUNCH :
			return OutToLunch;
		case YAHOO_STATUS_STEPPEDOUT :
			return SteppedOut;
		case YAHOO_STATUS_INVISIBLE :
			return Invisible;
		case YAHOO_STATUS_CUSTOM :
			return Custom;
		case YAHOO_STATUS_IDLE :
			return Idle;
		case YAHOO_STATUS_OFFLINE :
			return Offline;
		case YAHOO_STATUS_TYPING :
			return Typing;
	}

	return Offline;
}
