/*
    oscaruserinfo.h  -  Oscar Protocol Plugin

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef AIMUSERINFO_H
#define AIMUSERINFO_H

#include "aiminfobase.h"
#include "oscarsocket.h"

class KTextBrowser;
class OscarAccount;
class AIMContact;

class AIMUserInfo : public AIMUserInfoBase
{
	Q_OBJECT
public:
	/**
	 * This constructor is used to show the user info
	 * for a contact.  This constructor will tell the
	 * engine to send the User Profile Request, and will
	 * update it's view with the info when it is returned
	 * by the AIM server
	 */
	AIMUserInfo(const QString name, const QString nick,
				OscarAccount *account, AIMContact *contact);
	/**
	 * This constructor is called when we want to edit our
	 * own profile
	 */
	AIMUserInfo(const QString name, const QString nick,
				OscarAccount *account, const QString &profile);

private:
	QString m_nick;
	OscarAccount *m_account;
	QString m_name;

private slots:
	void slotSaveClicked();
	void slotCloseClicked();
	void slotSearchFound(const UserInfo &/*u*/, const QString /*profile*/);

signals:
	void updateNickname(const QString);
};

#endif

