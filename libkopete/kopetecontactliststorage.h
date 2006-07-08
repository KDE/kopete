/*
    Kopete Contact List Storage Base Class

    Copyright  2006      by Matt Rogers <mattr@kde.org>
    Kopete     2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETECONTACTLISTSTORAGE_H
#define KOPETECONTACTLISTSTORAGE_H

namespace Kopete
{

/**
 * @author Matt Rogers <mattr@kde.org>
 */
class ContactListStorage
{
public:
    ContactListStorage( );
    virtual ~ContactListStorage();

public:
    /**
     * Load the contact list
     */
    virtual void load() = 0;

    /**
     * Save the contact list
     */
    virtual void save() = 0;

};

}

#endif

//kate: indent-mode cstyle; indent-spaces on; indent-width 4; auto-insert-doxygen on; replace-tabs on
