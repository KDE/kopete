/*
    YahooInviteListImpl - conference invitation dialog
    
    Copyright (c) 2004 by Duncan Mac-Vicar P.    <duncan@kde.org>
    
    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef YAHOO_INVITE_LIST_IMPL
#define YAHOO_INVITE_LIST_IMPL

#include <qwidget.h>

#include "yahooinvitelistbase.h"

class YahooInviteListImpl : public YahooInviteListBase
{
	Q_OBJECT
public: 
	YahooInviteListImpl(QWidget *parent=0, const char *name=0);
	~YahooInviteListImpl();	
private:
	
signals:
	
protected slots:

};

#endif

