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

	YahooContact(QString userID, QString fullName, QString group, YahooProtocol *protocol, KopeteMetaContact *metaContact);
	~YahooContact();
	
	const QString &group();

	virtual bool isOnline() const;
	virtual bool isReachable();
	virtual QString identityId() const;
	virtual ContactStatus status() const;
	virtual QString statusText() const;
	virtual QString statusIcon() const;
	//virtual QPixmap scaledStatusIcon(int size);
	virtual int importance() const;
	virtual QString id() const;
	virtual KActionCollection *customContextMenuActions();
	virtual void addThisTemporaryContact(KopeteGroup *group = 0L);
	
public slots:
	
	virtual void execute();
	virtual void slotViewHistory();
	virtual void slotDeleteContact();
	virtual void slotUserInfo();
	virtual void slotSendFile();	
		
	private slots:

	private:
		
		enum YahooStatus 
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
			Custom,			// 99
			CustomBusy,		// 99+Busy
			CustomMobile,	// 99+Mobile
			Idle			// 999
		};

	/* User id, full name, group, status code, and status description */
	QString mUserID;
	QString mFullName;
	QString mGroup;
	YahooStatus mStatus;
	QString mStatusText;

private slots: // Private slots
	void slotMovedToMetaContact();
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

