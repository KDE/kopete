/***************************************************************************
                          yahoocontact.cpp  -  description
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Bruno Rodrigues
    email                : bruno.rodrigues@litux.org

    Based on code from   : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


// Local Includes
#include "yahoodebug.h"
#include "yahoostatus.h"
#include "yahoocontact.h"

// Kopete Includes
#include "kopetewindow.h"
#include "kopete.h"

// QT Includes

// KDE Includes
#include <kdebug.h>
#include <kmessagebox.h>


YahooContact::YahooContact(QString userID, QString fullName, QString group,
			     YahooProtocol *protocol, KopeteMetaContact *metaContact)
:  KopeteContact( protocol, metaContact)
{
	DEBUG(YDMETHOD, "YahooContact::YahooContact("<< userID << ", " << fullName << 
			", " << group << ", <protocol>)");

    mUserID = userID;
    mFullName = fullName;
    mGroup = group;
    mProtocol = protocol;

	mStatus = Offline;
	mStatusText = "";

	// Update ContactList
    setDisplayName(mFullName);
	emit statusChanged(this, status());

    // XXX initActions();
}


YahooContact::~YahooContact()
{
	DEBUG(YDMETHOD, "Yahoo::~YahooContact()");
}


// Return status
YahooContact::ContactStatus YahooContact::status() const
{
	DEBUG(YDMETHOD, "Yahoo::ContactStatus()");

	if(mStatus == Offline || mStatus == Invisible) { 
		return KopeteContact::Offline;
	} 
	else if(mStatus == Idle) {
		return Away;
	}
	else {
		return Online;
	}
}


// Return status text
QString YahooContact::statusText() const
{
	DEBUG(YDMETHOD, "Yahoo::statusText()");

	DEBUG(YDMETHOD, "Yahoo::setStatus()");
	if(mStatus == Offline || mStatus == Available || mStatus == Mobile ||
			mStatus == Invisible) 
		return "";
	else if(mStatus == Idle)
		return "Idle";
	else if(mStatus == Custom || mStatus == CustomBusy || 
			mStatus == CustomMobile)
		return mStatusText;
	else if(mStatus == BeRightBack)
		return YSTBeRightBack;
	else if(mStatus == Busy)
		return YSTBusy;
	else if(mStatus == NotAtHome)
		return YSTNotAtHome;
	else if(mStatus == NotAtMyDesk)
		return YSTNotAtMyDesk;
	else if(mStatus == NotInTheOffice)
		return YSTNotInTheOffice;
	else if(mStatus == OnThePhone)
		return YSTOnThePhone;
	else if(mStatus == OnVacation)
		return YSTOnVacation;
	else if(mStatus == OutToLunch)
		return YSTOutToLunch;
	else if(mStatus == SteppedOut)
		return YSTSteppedOut;
	else {
		DEBUG(YDERROR, "Invalid status");
		return "?";
	}
}

// Return status icon
QString YahooContact::statusIcon() const
{
	DEBUG(YDMETHOD, "Yahoo::statusIcon()");

	if(mStatus == Offline || mStatus == Invisible)
		return "yahoo_offline";
	else if(mStatus == Idle) 
		return "yahoo_idle";
	else if(mStatus == Mobile || mStatus == CustomMobile) 
		return "yahoo_mobile";
	else if(mStatus == Available || mStatus == Custom)
		return "yahoo_online";
	else if(mStatus == BeRightBack || mStatus == Busy || mStatus == NotAtHome 
			|| mStatus == NotAtMyDesk || mStatus == NotInTheOffice
			|| mStatus == OnThePhone || mStatus == OnVacation 
			|| mStatus == OutToLunch || mStatus == SteppedOut
			|| mStatus == CustomBusy)
		return "yahoo_busy";
	else {
		DEBUG(YDERROR, "Unknown status");
		return "yahoo_unknown";
	}
}


/* XXX
void YahooContact::slotUpdateStatus(QString status, QString statusText == NULL)
{
	DEBUG(YDMETHOD, "Yahoo::slotUpdateStatus(" << status << ")");

    DEBUG(YDINFO, "Buddy  - updating " << handle << " to "
			<< status << "." << endl;

    if (status != QString("")) {
		mStatus = status;
		kdDebug() << "Yahoo plugin: Updating status." << endl;
    }
	mStatusText = statusText;
    emit statusChanged();
}
*/

#include "yahoocontact.moc"


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

