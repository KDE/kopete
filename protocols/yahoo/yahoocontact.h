/***************************************************************************
                          yahoocontact.h  -  description
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Bruno Rodrigues
    email                : bruno.rodrigues@litux.org

    Based on code from   : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef YAHOOCONTACT_H
#define YAHOOCONTACT_H

// Local Includes
#include "yahooprotocol.h"

// Kopete Includes
#include "kopetecontact.h"

// QT Includes

// KDE Includes


class YahooProtocol;


class YahooContact : public KopeteContact 
{
	Q_OBJECT public:

		YahooContact(QString userID, QString fullName, QString group,
				YahooProtocol *protocol);	// Constructor
		~YahooContact();

		ContactStatus status() const;	// Return status 
		QString statusText() const;		// Return status text
		QString statusIcon() const;		// Return statusIcon 

	public slots:

	private slots:

	signals:

	private:
		enum YahooStatus { 
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
			Custom,			// 99
			CustomBusy,		// 99+Busy
			CustomMobile,	// 99+Mobile
			Idle			// 999
		};

		YahooProtocol *mProtocol;	// Parent Object

		QString mUserID;			// User ID
		QString mFullName;			// Full Name
		QString mGroup;				// Group
		YahooStatus mStatus;		// Status
		QString mStatusText;		// StatusText

};
	
#endif


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

