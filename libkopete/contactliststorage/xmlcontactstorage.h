/*
    Kopete Contact List XML Storage Class

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

#ifndef KOPETEXMLCONTACTSTORAGE_H
#define KOPETEXMLCONTACTSTORAGE_H

#include <kopetecontactliststorage.h>

namespace Kopete
{

/**
 * @brief XML storage for contact list elements.
 *
 * This contactlist storage is provided for backwards compability
 * with the "old" contact list which was saved as a XML document.
 *
 * @author Matt Rogers <mattr@kde.org>
 */
class XmlContactStorage : public ContactListStorage
{
public:
    XmlContactStorage();
    /**
     * @brief Create a new XML storage using the given filename.
     *
     * This contructor mostly used for unittests.
     * @param fileName XML filename to load.
     */
    XmlContactStorage(const QString &fileName);
    ~XmlContactStorage();

    virtual bool isValid() const;
    virtual QString errorMessage() const;
    virtual void load();
    virtual void save();

private:
    class Private;
    Private* d;
};

}

#endif

//kate: indent-mode cstyle; indent-width 4; indent-spaces on; replace-tabs on;
