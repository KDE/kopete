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

        /* Item type role values */
        enum Type { Group, MetaContact };
    }
}

#endif
