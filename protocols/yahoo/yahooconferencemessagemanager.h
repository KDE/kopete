 /*
    yahooconferencemessagemanager.h

    Copyright (c) 2003 by Duncan Mac-Vicar Prett <duncan@kde.org>

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

#ifndef YAHOOCONFERENCEMESSAGEMANAGER_H
#define YAHOOCONFERENCEMESSAGEMANAGER_H

#include <qwidget.h>

#include "kopetecontact.h"
#include "kopetemessagemanager.h"

class YahooSession;

/**
  *@author Duncan Mac-Vicar P. <duncan@kde.org>
  */

class YahooConferenceMessageManager : public KopeteMessageManager
{
	Q_OBJECT
public:

	YahooConferenceMessageManager(const KopeteContact *user, KopeteContactPtrList others, const char *name = 0 );
	~YahooConferenceMessageManager();

private slots:
	void slotMessageSent(const KopeteMessage &message, KopeteMessageManager *);
private:
	YahooSession *m_session;
};

#endif
