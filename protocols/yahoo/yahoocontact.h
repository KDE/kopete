/*
    yahoocontact.h - Yahoo Contact

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

#ifndef YAHOOCONTACT_H
#define YAHOOCONTACT_H

/* Local Includes */
#include "yahooprotocol.h"

/* Kopete Includes */
#include "kopetecontact.h"
#include "kopetemetacontact.h"

/* QT Includes */

/* KDE Includes */

class YahooProtocol;

class YahooContact : public KopeteContact
{
	Q_OBJECT public:

	YahooContact(QString userID, QString fullName, YahooProtocol *protocol, KopeteMetaContact *metaContact);
	~YahooContact();

	virtual bool isOnline() const;
	virtual bool isReachable();
	virtual QString identityId() const;
	virtual KopeteContact::ContactStatus status() const;
	virtual QString statusText() const;
	virtual QString statusIcon() const;
	virtual int importance() const;
	virtual KActionCollection *customContextMenuActions();

	void setYahooStatus( YahooStatus::Status , const QString &, int  );

public slots:
	virtual void execute();
	virtual void slotViewHistory();
	virtual void slotDeleteContact();
	virtual void slotUserInfo();
	virtual void slotSendFile();

	private slots:

	private:

	/* User id, full name, group, status code, and status description */
	QString mUserID;
	QString mFullName;

	YahooStatus mStatus;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

