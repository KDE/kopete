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
namespace Kopete
{
    namespace Items
    {
        /** Qt Model Role Definitions */
        const int TypeRole = Qt::UserRole + 1;
        
        /* Item type role values */
        enum Type { Group, MetaContact };
        
    } /* Items */
    
} /* Kopete */

#endif
