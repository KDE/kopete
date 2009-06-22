/*
    Kopete Contact List XML Storage Class

    Copyright  2006      by Matt Rogers <mattr@kde.org>
    Copyright  2006      by Michaël Larouche <larouche@kde.org>
    Copyright  2006      by Roman Jarosz <kedgedev@centrum.cz>

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
#include <kopete_export.h>
class QDomElement;

namespace Kopete
{

/**
 * @brief XML storage for contact list elements.
 *
 * This contact list storage is provided for backwards compatibility
 * with the "old" contact list which was saved as a XML document.
 *
 * @author Matt Rogers <mattr@kde.org>
 * @author Michaël Larouche <larouche@kde.org>
 */
class KOPETE_EXPORT XmlContactStorage : public ContactListStorage
{
public:
    XmlContactStorage();
    /**
     * @brief Create a new XML storage using the given filename.
     *
     * This contructor mostly used for unittests.
     * @param fileName XML filename to load.
     */
    explicit XmlContactStorage(const QString &fileName);
    ~XmlContactStorage();

    virtual bool isValid() const;
    virtual QString errorMessage() const;
    virtual bool isBusy() const;
    virtual void load();
    virtual void save();

protected:
    bool parseMetaContact( Kopete::MetaContact *metaContact, const QDomElement &element );
    bool parseGroup( Kopete::Group *group, const QDomElement &element );
    bool parseContactListElement( Kopete::ContactListElement *contactListElement, const QDomElement &element );

    const QDomElement storeMetaContact( Kopete::MetaContact *metaContact, bool minimal = false ) const;
    const QDomElement storeGroup( Kopete::Group *group ) const;
    const QList<QDomElement> storeContactListElement( Kopete::ContactListElement *contactListElement ) const;

    bool updateFrom10to11( QDomElement &rootElement ) const;
    bool updateFrom11to12( QDomElement &rootElement ) const;

    uint readVersion( QDomElement &rootElement ) const;

private:
    /**
     * Convert the contact list from an older version
     */
    void convertContactList( const QString &fileName, uint fromVersion, uint toVersion );

    QString sourceToString( Kopete::MetaContact::PropertySource source ) const;
    Kopete::MetaContact::PropertySource stringToSource( const QString &name ) const;

    void checkGroupIds();

    class Private;
    Private* d;
};

}

#endif

//kate: indent-mode cstyle; indent-width 4; indent-spaces on; replace-tabs on;
