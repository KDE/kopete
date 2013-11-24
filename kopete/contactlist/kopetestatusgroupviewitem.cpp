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

#include "kopetestatusgroupviewitem.h"
#include <kdebug.h>

KopeteStatusGroupViewItem::KopeteStatusGroupViewItem( Kopete::OnlineStatus::StatusType status_ , Q3ListView *parent)
		: Q3ListViewItem(parent)
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
		case Kopete::OnlineStatus::Online :
		return "A";
		break;
		case Kopete::OnlineStatus::Away :
		return "B";
		break;
		case Kopete::OnlineStatus::Offline :
		return "C";
		break;
		case Kopete::OnlineStatus::Unknown :
		default:
		return "D";
	}
}

