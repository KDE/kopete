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
class AIMBuddy;

class AIMUserInfo : public AIMUserInfoBase
{
	Q_OBJECT
	public:
	/**
	* This constructor is used to show the user info
	* for a contact
	*/
		AIMUserInfo(const QString name, const QString nick,
				OscarAccount *account, AIMBuddy &buddy);
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
		void slotSearchFound(UserInfo, QString);

	signals:
		void updateNickname(const QString);
};

#endif

