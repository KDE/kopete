/*
    yahoouserinfo.h - hold and display buddy information

    Copyright (c) 2005 by Andre Duffeck <andre@duffeck.de>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef YAHOOUSERINFO_H
#define YAHOOUSERINFO_H

// QT Includes
#include <qstring.h>

// KDE Includes
#include <kdialogbase.h>

// Local Includes
#include "yahoouserinfobase.h"

struct YahooUserInfo {
	QString	userID;
	QString	abID;
	QString	firstName;
	QString	lastName;
	QString	nickName;
	QString	email;
	QString	phoneHome;
	QString	phoneWork;
	QString	phoneMobile;
};

class YahooSession;

class YahooUserInfoDialog : public KDialogBase
{
	Q_OBJECT

public:
	YahooUserInfoDialog( QWidget* parent = 0, const char* name = 0 );
	~YahooUserInfoDialog();

	void setSession( YahooSession * );
	void setUserInfo( const YahooUserInfo & );
public slots:

protected slots:
	virtual void slotClose();
	virtual void slotApply();
	virtual void slotUser1();

private:
	YahooUserInfoWidget *mMainWidget;
	YahooSession *m_theSession;
	YahooUserInfo m_userInfo;
};

#endif

