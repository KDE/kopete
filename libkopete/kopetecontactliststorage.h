/*
    Kopete Contact List Storage Base Class

    Copyright  2006      by Matt Rogers <mattr@kde.org>
    Copyright  2006      by Michaël Larouche <michael.larouche@kdemail.net>

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

#include "kopetegroup.h"
#include "kopetemetacontact.h"

namespace Kopete
{

/**
 * @brief Provide a storage for Kopete Contact List.
 *
 * @author Matt Rogers <mattr@kde.org>
 * @author Michaël Larouche <michael.larouche@kdemail.net>
 */
class ContactListStorage
{
public:
    ContactListStorage( );
    virtual ~ContactListStorage();

    /**
     * @brief Get the Group list for this storage.
     *
     * @return Group list.
     */
    Group::List groups() const;
    /**
     * @brief Get the MetaContact list for this storage.
     * 
     * @return MetaContact list.
     */
    MetaContact::List contacts() const;

public:
    /**
     * @brief Check if the current ContactListStorage.
     *
     * Call this method to verify if the loading of the storage
     * went without problems.
     * 
     * @code
     * Kopete::ContactListStorage *storage = new Kopete::XmlContactStorage;
     * storage->load();
     * 
     * if( !storage->isValid() )
     * {
     *     kDebug() << k_funcinfo << "Contact list storage failed. Reason: " << storage->errorMessage() << endl;
     * }
     * @endcode
     * 
     * Derived ContactListStorage must implement this method.
     * 
     * @return true if this ContactListStorage is valid.
     */
    virtual bool isValid() const = 0;
    
    /**
     * @brief Get a nice error message
     * 
     * Use this method to find out why an operation of the ContactListStorage
     * failed. The resulting error message is already translated.
     * 
     * Derived ContactListStorage must implement this method.
     * 
     * @return Translated error message
     */
    virtual QString errorMessage() const = 0;

    /**
     * @brief Load the contact list
     * 
     * Derived ContactListStorage must implement this method.
     */
    virtual void load() = 0;

    /**
     * @brief Save the contact list
     * 
     * Derived ContactListStorage must implement this method.
     */
    virtual void save() = 0;

protected:
    /**
     * @brief Add a MetaContact to internal list.
     * 
     * Derived ContactListStorage use this method to add new
     * MetaContact.
     * 
     * @param metaContact MetaContact to add.
     */
    void addMetaContact(Kopete::MetaContact *metaContact);
    /**
     * @brief Add a Group to internal listé
     * 
     * Derived ContactListStorage use this method to add new
     * Group.
     * 
     * @param group Group to add.
     */
    void addGroup(Kopete::Group *group);

private:
    class Private;
    Private *d;
};

}

#endif

//kate: indent-mode cstyle; indent-spaces on; indent-width 4; auto-insert-doxygen on; replace-tabs on
