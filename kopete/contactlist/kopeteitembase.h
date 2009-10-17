/*
    Kopete View Item Delegate

    Copyright (c) 2007 by Matt Rogers <mattr@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEITEMBASE_H
#define KOPETEITEMBASE_H

/** 
 * @file kopeteitembase.h
 * Contains definitions common between model items
 */ 

#define KOPETE_GROUP_DEFAULT_OPEN_ICON "folder-open"
#define KOPETE_GROUP_DEFAULT_CLOSED_ICON "folder"

namespace Kopete
{
	namespace Items
	{
		/** Qt Model Role Definitions */
		const int TypeRole = Qt::UserRole + 100;
		const int ElementRole = Qt::UserRole + 101;
		const int OnlineStatusRole = Qt::UserRole + 102;
		const int IdleTimeRole = Qt::UserRole + 103;
		const int UuidRole = Qt::UserRole + 104;
		const int TotalCountRole = Qt::UserRole + 105;
		const int ConnectedCountRole = Qt::UserRole + 106;
		const int IdRole = Qt::UserRole + 107;
				// the IdRole is used in cases where the identifier is not really a Uuid
				// for instance, Kopete::Group::groupId is the case justifying this role
		const int MetaContactImageRole = Qt::UserRole + 108;
				// the MetaContactImageRole can return QImage or QString. If it's QString
				// then it contains icon name.
		const int StatusTitleRole = Qt::UserRole + 109;
		const int StatusMessageRole = Qt::UserRole + 110;
		const int AccountIconsRole = Qt::UserRole + 111;
		const int ObjectRole = Qt::UserRole + 112;
		const int ExpandStateRole = Qt::UserRole + 113;
		const int HasNewMessageRole = Qt::UserRole + 114;
		const int MetaContactGroupRole = Qt::UserRole + 115;
		const int AlwaysVisible = Qt::UserRole + 116;

		/* Item type role values */
		enum Type { Group, MetaContact };
	}
}

#endif
