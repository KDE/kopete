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

YahooContact::YahooContact(QString userID, QString fullName,
			     YahooProtocol *protocol, KopeteMetaContact *metaContact)
:  KopeteContact( protocol, userID, metaContact)
{
	kdDebug(14180) << "YahooContact::YahooContact("<< userID << ", " << fullName << ")" << endl;

	mUserID = userID;
	mFullName = fullName;

	mStatus.setStatus(YahooStatus::Offline);

	// Update ContactList
	setDisplayName(mFullName);
	emit statusChanged(this, status());

	// XXX initActions();
}

YahooContact::~YahooContact()
{
	kdDebug(14180) << "Yahoo::~YahooContact()" << endl;
}

// Return status
KopeteContact::ContactStatus YahooContact::status() const
{
	kdDebug(14180) << "YahooContact::status()" << endl;
	return mStatus.translate();
}

// Return status text
QString YahooContact::statusText() const
{
	kdDebug(14180) << "Yahoo::statusText()";
	return mStatus.text();
}

// Return status icon
QString YahooContact::statusIcon() const
{
	return mStatus.icon();
}

void YahooContact::setYahooStatus( YahooStatus::Status status_, const QString &msg, int away)
{
	mStatus.setStatus(status_);
	emit statusChanged( this, mStatus.translate() );
}

/* XXX
void YahooContact::slotUpdateStatus(QString status, QString statusText == NULL)
{
	DEBUG(YDMETHOD, "Yahoo::slotUpdateStatus(" << status << ")");

	DEBUG(YDINFO, "Buddy  - updating " << handle << " to "
			<< status << "." << endl;

	if (status != QString("")) {
		mStatus = status;
		kdDebug(14180) << "Yahoo plugin: Updating status." << endl;
	}
	mStatusText = statusText;
	emit statusChanged();
}
*/

bool YahooContact::isOnline() const
{
	kdDebug(14180) << "[YahooContact::isOnline()]" << endl;
	return status() != Offline && status() != Unknown;
}

bool YahooContact::isReachable()
{
	kdDebug(14180) << "[YahooContact::isReachable()]" << endl;
	return true;
}

QString YahooContact::identityId() const
{
	kdDebug(14180) << "[YahooContact::identityId()]" << endl;
	return QString::null;
}

/*
QPixmap YahooContact::scaledStatusIcon(int size)
{

}
*/

int YahooContact::importance() const
{
	kdDebug(14180) << "[YahooContact::importance()]" << endl;
	return 0;
}

KActionCollection *YahooContact::customContextMenuActions()
{
	kdDebug(14180) << "[YahooContact::customContextMenuActions()]" << endl;
	return 0L;
}

void YahooContact::execute()
{
	kdDebug(14180) << "[YahooContact::execute()]" << endl;
}

void YahooContact::slotViewHistory()
{
	kdDebug(14180) << "[YahooContact::slotViewHistory()]" << endl;
}

void YahooContact::slotDeleteContact()
{
	kdDebug(14180) << "[YahooContact::slotDeleteContact()]" << endl;
}

void YahooContact::slotUserInfo()
{
	kdDebug(14180) << "[YahooContact::slotUserInfo()]" << endl;
}

void YahooContact::slotSendFile()
{
	kdDebug(14180) << "[YahooContact::slotSendFile()]" << endl;
}

#include "yahoocontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

