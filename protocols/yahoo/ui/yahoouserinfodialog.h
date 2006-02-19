/*
 Kopete Yahoo Protocol
 yahoouserinfodialog.h - Display Yahoo user info

 Copyright (c) 2005 Matt Rogers <mattr@kde.org>
 Copyright (c) 2006 Andre Duffeck <mattr@kde.org>

 Kopete (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

 *************************************************************************
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU Lesser General Public            *
 * License as published by the Free Software Foundation; either          *
 * version 2 of the License, or (at your option) any later version.      *
 *                                                                       *
 *************************************************************************
*/

#ifndef YAHOOUSERINFODIALOG_H_
#define YAHOOUSERINFODIALOG_H_

#include <kdialogbase.h>
#include "../libkyahoo/yabentry.h"

class KJanusWidget;
class YahooWorkInfoWidget;
class YahooGeneralInfoWidget;
class YahooOtherInfoWidget;
class YahooContact;

class YahooUserInfoDialog : public KDialogBase
{
Q_OBJECT
public:
	YahooUserInfoDialog( YahooContact *c, QWidget* parent = 0, const char* name = 0 );
	void setAccountConnected( bool isOnline );
signals:
	void saveYABEntry( YABEntry & );
public slots:
	void setData( const YABEntry &yab );
private slots:
	void slotSaveAndCloseClicked();
	void slotUser2();
private:
	YahooGeneralInfoWidget* m_genInfoWidget;
	YahooWorkInfoWidget* m_workInfoWidget;
	YahooOtherInfoWidget* m_otherInfoWidget;
	
	YABEntry m_yab;
	YahooContact *m_contact;
};

#endif

//kate: indent-mode csands; tab-width 4; space-indent off; replace-tabs off;
