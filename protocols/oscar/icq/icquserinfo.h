 /*
    icquserinfo.h  -  ICQ Protocol Plugin

    Copyright (c) 2002 by Nick Betcher <nbetcher@kde.org>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef ICQUSERINFO_H
#define ICQUSERINFO_H

#include <kdebug.h>
#include <qhbox.h>
#include <kdialogbase.h>

class ICQProtocol;
class ICQAccount;
class ICQContact;
class ICQUserInfoWidget;

class ICQUserInfo : public KDialogBase
{
	Q_OBJECT

	public:
		ICQUserInfo(ICQContact *, QWidget *parent = 0, const char* name = "ICQUserInfo");

	private:
		void setReadonly();

	private slots:
		void slotSaveClicked();
		void slotCloseClicked();
		void slotHomePageClicked(const QString &);
		void slotEmailClicked(const QString &);
		void slotFetchInfo(); // initiate fetching info from server
		void slotReadInfo(); // read in results from fetch
		void slotUserInfoRequestFailed();

	signals:
//		void updateNickname(const QString);
		void closing();

	private:
		ICQProtocol *p;
		ICQAccount *mAccount;
		ICQContact *mContact;
		ICQUserInfoWidget *mMainWidget;
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
