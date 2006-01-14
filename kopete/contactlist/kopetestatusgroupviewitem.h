/*
    kopetestatusgroupviewitem.h

    Class to show a status folder

    Copyright (c) 2001-2003 by Duncan Mac-Vicar Prett   <duncan@kde.org>

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

#ifndef KOPETESTATUSGROUPVIEWITEM_H
#define KOPETESTATUSGROUPVIEWITEM_H

#include <klistview.h>
#include "kopetemetacontact.h"

/**
  *@author Duncan Mac-Vicar Prett <duncan@kde.org>
  */

 class KopeteStatusGroupViewItem : public QListViewItem
{
public: 
	KopeteStatusGroupViewItem( Kopete::OnlineStatus::StatusType status_ , QListView *parent, const char *name=0);
	~KopeteStatusGroupViewItem();

private:

	Kopete::OnlineStatus::StatusType m_status;
	QString key( int column, bool ascending ) const;

};

#endif
