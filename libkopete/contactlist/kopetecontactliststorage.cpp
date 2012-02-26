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
#include "kopetecontactliststorage.h"

// Qt includes

// KDE includes

// Kopete includes
namespace Kopete
{

class ContactListStorage::Private
{
public:
    Private()
    {}

    Group::List groupList;
    MetaContact::List metaContactList;
};

ContactListStorage::ContactListStorage()
 : d(new Private)
{
}


ContactListStorage::~ContactListStorage()
{
    delete d;
}

Group::List ContactListStorage::groups() const
{
    return d->groupList;
}

MetaContact::List ContactListStorage::contacts() const
{
    return d->metaContactList;
}

void ContactListStorage::addMetaContact(Kopete::MetaContact *contact)
{
    d->metaContactList.append( contact );
}

void ContactListStorage::addGroup(Kopete::Group *group)
{
    d->groupList.append( group );
}

Kopete::Group * ContactListStorage::group( unsigned int groupId ) const
{
    foreach( Kopete::Group * group, d->groupList )
    {
        if( group->groupId() == groupId )
            return group;
    }
    return 0L;
}

Kopete::Group * ContactListStorage::findGroup( const QString& displayName, int type )
{
    if( type == Kopete::Group::Temporary )
        return Kopete::Group::temporary();
    if( type == Kopete::Group::TopLevel )
        return Kopete::Group::topLevel();
    if( type == Kopete::Group::Offline )
        return Kopete::Group::offline();

    foreach( Kopete::Group * group, d->groupList )
    {
        if( group->type() == type && group->displayName() == displayName )
            return group;
    }

    Kopete::Group *newGroup = new Kopete::Group( displayName );
    addGroup( newGroup );
    return  newGroup;
}

}

//kate: indent-mode cstyle; indent-spaces on; indent-width 4; auto-insert-doxygen on; replace-tabs on
