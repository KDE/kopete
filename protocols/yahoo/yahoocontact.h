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
#include "yahoostatus.h"
/* Kopete Includes */
#include "kopetecontact.h"
#include "kopetemetacontact.h"

/* QT Includes */
#include <qtimer.h>
/* KDE Includes */

class YahooProtocol;

class YahooContact : public KopeteContact
{
	Q_OBJECT public:

	YahooContact(QString userId, QString fullName, YahooProtocol *protocol, KopeteMetaContact *metaContact);
	~YahooContact();

	virtual bool isOnline() const;
	virtual bool isReachable();
	virtual QString identityId() const;
	virtual KopeteContact::ContactStatus status() const;
	virtual QString statusText() const;
	virtual QString statusIcon() const;
	virtual int importance() const;
	virtual KActionCollection *customContextMenuActions();
	virtual KopeteMessageManager *manager();

	void setYahooStatus( YahooStatus::Status , const QString &, int  );

public slots:
	virtual void slotViewHistory();
	virtual void slotDeleteContact();
	virtual void slotUserInfo();
	virtual void slotSendFile();

private:
	QString m_fullName;
	YahooStatus m_status;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

