/*
    yahoocontact.cpp - Yahoo Contact

    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Portions based on code by Bruno Rodrigues <bruno.rodrigues@litux.org>

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

// Local Includes
#include "yahoodebug.h"
#include "yahoostatus.h"
#include "yahoocontact.h"

// QT Includes
#include <qstring.h>

// KDE Includes
#include <kdebug.h>
#include <kmessagebox.h>

YahooContact::YahooContact(QString userID, QString fullName, QString group,
			     YahooProtocol *protocol, KopeteMetaContact *metaContact)
:  KopeteContact( protocol, metaContact)
{
	kdDebug() << "YahooContact::YahooContact("<< userID << ", " << fullName <<
			", " << group << ", <protocol>)" ;

	mUserID = userID;
	mFullName = fullName;
	mGroup = group;

	mStatus = Offline;
	mStatusText = "";

	// Update ContactList
	setDisplayName(mFullName);
	emit statusChanged(this, status());

	// XXX initActions();

	connect (this , SIGNAL( moved(KopeteMetaContact*,KopeteContact*) ), this, SLOT (slotMovedToMetaContact() ));
	connect (metaContact , SIGNAL( aboutToSave(KopeteMetaContact*) ), protocol, SLOT (serialize(KopeteMetaContact*) ));

}

YahooContact::~YahooContact()
{
	kdDebug() << "Yahoo::~YahooContact()" << endl;
}

// Return status
YahooContact::ContactStatus YahooContact::status() const
{
	kdDebug() << "Yahoo::ContactStatus()" << endl;

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

const QString& YahooContact::group()
{
	return mGroup;
}


// Return status text
QString YahooContact::statusText() const
{
	kdDebug() << "Yahoo::statusText()";

	if(mStatus == Offline || mStatus == Available || mStatus == Mobile ||
			mStatus == Invisible)
		return "";
	else if(mStatus == Idle)
		return i18n("Idle");
	else if(mStatus == Custom || mStatus == CustomBusy ||
			mStatus == CustomMobile)
		return mStatusText;
	else if(mStatus == BeRightBack)
		return i18n(YSTBeRightBack);
	else if(mStatus == Busy)
		return i18n(YSTBusy);
	else if(mStatus == NotAtHome)
		return i18n(YSTNotAtHome);
	else if(mStatus == NotAtMyDesk)
		return i18n(YSTNotAtMyDesk);
	else if(mStatus == NotInTheOffice)
		return i18n(YSTNotInTheOffice);
	else if(mStatus == OnThePhone)
		return i18n(YSTOnThePhone);
	else if(mStatus == OnVacation)
		return i18n(YSTOnVacation);
	else if(mStatus == OutToLunch)
		return i18n(YSTOutToLunch);
	else if(mStatus == SteppedOut)
		return i18n(YSTSteppedOut);
	else
	{
		kdDebug() << "Invalid status" << endl;
		return "?";
	}
}

// Return status icon
QString YahooContact::statusIcon() const
{
	kdDebug() << "Yahoo::statusIcon()" << endl;

	if(mStatus == Offline || mStatus == Invisible)
	{
		return "yahoo_offline";
	}
	else if(mStatus == Idle)
	{
		return "yahoo_idle";
	}
	else if(mStatus == Mobile || mStatus == CustomMobile)
	{
		return "yahoo_mobile";
	}
	else if(mStatus == Available || mStatus == Custom)
	{
		return "yahoo_online";
	}
	else if(mStatus == BeRightBack || mStatus == Busy || mStatus == NotAtHome
			|| mStatus == NotAtMyDesk || mStatus == NotInTheOffice
			|| mStatus == OnThePhone || mStatus == OnVacation
			|| mStatus == OutToLunch || mStatus == SteppedOut
			|| mStatus == CustomBusy)
	{
		return "yahoo_busy";
	}
	else
	{
		kdDebug() << "Unknown status" << endl;
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

bool YahooContact::isOnline() const
{
	kdDebug() << "[YahooContact::isOnline()]" << endl;
	return status() != Offline && status() != Unknown;
}

bool YahooContact::isReachable()
{
	kdDebug() << "[YahooContact::isReachable()]" << endl;
	return true;
}

QString YahooContact::identityId() const
{
	kdDebug() << "[YahooContact::identityId()]" << endl;
	return QString::null;
}

/*
QPixmap YahooContact::scaledStatusIcon(int size)
{

}
*/

int YahooContact::importance() const
{
	kdDebug() << "[YahooContact::importance()]" << endl;
	return 0;
}

QString YahooContact::contactId() const
{
	kdDebug() << "[YahooContact::contactId()]" << endl;
	return mUserID;
}

KActionCollection *YahooContact::customContextMenuActions()
{
	kdDebug() << "[YahooContact::customContextMenuActions()]" << endl;
	return 0L;
}

void YahooContact::addThisTemporaryContact(KopeteGroup *group)
{
	kdDebug() << "[addThisTemporaryContact]" << endl;
}

void YahooContact::execute()
{
	kdDebug() << "[YahooContact::execute()]" << endl;
}

void YahooContact::slotViewHistory()
{
	kdDebug() << "[YahooContact::slotViewHistory()]" << endl;
}

void YahooContact::slotDeleteContact()
{
	kdDebug() << "[YahooContact::slotDeleteContact()]" << endl;
}

void YahooContact::slotUserInfo()
{
	kdDebug() << "[YahooContact::slotUserInfo()]" << endl;
}

void YahooContact::slotSendFile()
{
	kdDebug() << "[YahooContact::slotSendFile()]" << endl;
}

void YahooContact::slotMovedToMetaContact()
{
	connect (metaContact() , SIGNAL( aboutToSave(KopeteMetaContact*) ), protocol(), SLOT (serialize(KopeteMetaContact*) ));
}

#include "yahoocontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

