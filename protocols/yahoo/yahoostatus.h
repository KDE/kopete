/*
    yahoostatus.cpp - Yahoo Plugin for Kopete

    Copyright (c) 2003 by Matt Rogers <mattrogers@sbcglobal.net>
    Copyright (c) 2002 by Bruno Rodrigues <bruno.rodrigues@litux.org>

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


 #ifndef YAHOOSTATUS__H
 #define YAHOOSTATUS__H

// KDE Includes
#include <klocale.h>
#include "kopetecontact.h"
#include "libyahoo2/yahoo2_types.h"

#define YSTOffline			I18N_NOOP("Offline")
#define YSTInvisible		I18N_NOOP("Invisible")
#define YSTAvailable		I18N_NOOP("Available")
#define YSTBeRightBack		I18N_NOOP("Be Right Back")
#define YSTBusy				I18N_NOOP("Busy")
#define YSTNotAtHome		I18N_NOOP("Not at Home")
#define YSTNotAtMyDesk		I18N_NOOP("Not at my Desk")
#define YSTNotInTheOffice	I18N_NOOP("Not in the Office")
#define YSTOnThePhone		I18N_NOOP("On the Phone")
#define YSTOnVacation		I18N_NOOP("On Vacation")
#define YSTOutToLunch		I18N_NOOP("Out to Lunch")
#define YSTSteppedOut		I18N_NOOP("Stepped Out")
#define YSTIdle				I18N_NOOP("Idle")

class YahooStatus
{
public:
	enum Status
	{
		Offline,		// Disconnected
		Available,		// 0
		Mobile,			// 0+Mobile
		BeRightBack,	// 1+Busy
		Busy,			// 2+Busy
		NotAtHome,		// 3+Busy
		NotAtMyDesk,	// 4+Busy
		NotInTheOffice,	// 5+Busy
		OnThePhone,		// 6+Busy
		OnVacation,		// 7+Busy
		OutToLunch,		// 8+Busy
		SteppedOut,		// 9+Busy
		Invisible,		// 12
		Custom,			// 13
		CustomBusy,		// 99+Busy
		CustomMobile,	// 99+Mobile
		Idle		// 999
	};

	YahooStatus( Status );
	YahooStatus();

	/* Translates YahooStatus to Kopete standard ones */
	KopeteOnlineStatus ys2kos() const;
	/* Set current status */
	void setStatus( Status, const QString & = QString::null );
	static Status fromLibYahoo2( int );
private:
	Status m_status;
	QString m_statusText;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

