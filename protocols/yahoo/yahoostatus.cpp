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

YahooStatus::YahooStatus( Status status_ )
{
	m_status = status_;
}

YahooStatus::YahooStatus()
{
	m_status = Offline;
}

KopeteContact::ContactStatus YahooStatus::translate() const
{
	if(m_status == Offline )
		return KopeteContact::Offline;
	else if(m_status == Available )
		return KopeteContact::Online;
	else if(m_status == Mobile )
		return KopeteContact::Away;
	else if(m_status == Invisible )
		return KopeteContact::Offline;
	else if(m_status == Idle)
		return KopeteContact::Away;
	else if(m_status == Custom || m_status == CustomBusy || m_status == CustomMobile)
		return KopeteContact::Away;
	else if(m_status == BeRightBack)
		return KopeteContact::Away;
	else if(m_status == Busy)
		return KopeteContact::Away;
	else if(m_status == NotAtHome)
		return KopeteContact::Away;
	else if(m_status == NotAtMyDesk)
		return KopeteContact::Away;
	else if(m_status == NotInTheOffice)
		return KopeteContact::Away;
	else if(m_status == OnThePhone)
		return KopeteContact::Away;
	else if(m_status == OnVacation)
		return KopeteContact::Away;
	else if(m_status == OutToLunch)
		return KopeteContact::Away;
	else if(m_status == SteppedOut)
		return KopeteContact::Away;
	else
		return KopeteContact::Offline;
}

QString YahooStatus::text() const
{
	kdDebug() << "YahooStatus::text()";

	if(m_status == Offline || m_status == Available || m_status == Mobile || m_status == Invisible)
		return "";
	else if(m_status == Idle)
		return i18n("Idle");
	else if(m_status == Custom || m_status == CustomBusy || m_status == CustomMobile)
		return m_statusText;
	else if(m_status == BeRightBack)
		return i18n(YSTBeRightBack);
	else if(m_status == Busy)
		return i18n(YSTBusy);
	else if(m_status == NotAtHome)
		return i18n(YSTNotAtHome);
	else if(m_status == NotAtMyDesk)
		return i18n(YSTNotAtMyDesk);
	else if(m_status == NotInTheOffice)
		return i18n(YSTNotInTheOffice);
	else if(m_status == OnThePhone)
		return i18n(YSTOnThePhone);
	else if(m_status == OnVacation)
		return i18n(YSTOnVacation);
	else if(m_status == OutToLunch)
		return i18n(YSTOutToLunch);
	else if(m_status == SteppedOut)
		return i18n(YSTSteppedOut);
	else
	{
		kdDebug() << "Invalid status" << endl;
		return "?";
	}
}

QString YahooStatus::icon() const
{
	kdDebug() << "YahooStatus::icon()" << endl;

	if(m_status == Offline || m_status == Invisible)
	{
		return "yahoo_offline";
	}
	else if(m_status == Idle)
	{
		return "yahoo_idle";
	}
	else if(m_status == Mobile || m_status == CustomMobile)
	{
		return "yahoo_mobile";
	}
	else if(m_status == Available || m_status == Custom)
	{
		return "yahoo_online";
	}
	else if(m_status == BeRightBack || m_status == Busy || m_status == NotAtHome
			|| m_status == NotAtMyDesk || m_status == NotInTheOffice
			|| m_status == OnThePhone || m_status == OnVacation
			|| m_status == OutToLunch || m_status == SteppedOut
			|| m_status == CustomBusy)
	{
		return "yahoo_busy";
	}
	else
	{
		kdDebug() << "Unknown status" << endl;
		return "yahoo_unknown";
	}
}

void YahooStatus::setStatus( Status status_ )
{
	m_status = status_;
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
}