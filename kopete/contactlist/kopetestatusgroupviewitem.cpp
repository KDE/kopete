/*
    kopetestatusgroupviewitem.cpp

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

#include <kdebug.h>
#include "kopetestatusgroupviewitem.h"
 
KopeteStatusGroupViewItem::KopeteStatusGroupViewItem( KopeteMetaContact::OnlineStatus status_ , QListView *parent, const char *name )
		: QListViewItem(parent,name)
{
	m_status = status_;
}

KopeteStatusGroupViewItem::~KopeteStatusGroupViewItem()
{
}

QString KopeteStatusGroupViewItem::key( int, bool ) const
{
	switch (m_status)
	{
		case KopeteMetaContact::Online :
		return "A";
		break;
		case KopeteMetaContact::Away :
		return "B";
		break;
		case KopeteMetaContact::Offline :
		return "C";
		break;
		case KopeteMetaContact::Unknown :
		default:
		return "D";
	}	
}

