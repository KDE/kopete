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

#include "xmlcontactstorage.h"

// Qt includes
#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QLatin1String>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

// KDE includes
#include <kdebug.h>
#include <kstandarddirs.h>

// Kopete includes
#include "kopetemetacontact.h"
#include "kopetegroup.h"
#include "kopetegeneralsettings.h"

namespace Kopete
{

class XmlContactStorage::Private
{
public:
	bool loaded;
};


XmlContactStorage::XmlContactStorage()
        : ContactListStorage(), d(new Private)
{
    d->loaded = false;
}


XmlContactStorage::~XmlContactStorage()
{
    delete d;
}


void XmlContactStorage::load()
{
    // don't save when we're in the middle of this...
    d->loaded = false;

    QString filename = KStandardDirs::locateLocal( "appdata", QLatin1String( "contactlist.xml" ) );
    if( filename.isEmpty() )
    {
        d->loaded=true;
        return ;
    }

    QDomDocument contactList( QString::fromLatin1( "kopete-contact-list" ) );

    QFile contactListFile( filename );
    contactListFile.open( QIODevice::ReadOnly );
    contactList.setContent( &contactListFile );

    QDomElement list = contactList.documentElement();
    //TODO: Check if we remove versioning from XML file.
#if 0
    QString versionString = list.attribute( QString::fromLatin1( "version" ), QString::null );
    uint version = 0;
    if( QRegExp( QString::fromLatin1( "[0-9]+\\.[0-9]" ) ).exactMatch( versionString ) )
        version = versionString.replace( QString::fromLatin1( "." ), QString::null ).toUInt();

    if( version < Private::ContactListVersion )
    {
        // The version string is invalid, or we're using an older version.
        // Convert first and reparse the file afterwards
        kDebug( 14010 ) << k_funcinfo << "Contact list version " << version
                << " is older than current version " <<  Private::ContactListVersion
                << ". Converting first." << endl;

        contactListFile.close();

        convertContactList( filename, version,  Private::ContactListVersion );
#endif
        contactList = QDomDocument ( QLatin1String( "kopete-contact-list" ) );

        contactListFile.open( QIODevice::ReadOnly );
        contactList.setContent( &contactListFile );

        list = contactList.documentElement();
#if 0
    }
#endif
 //TODO: Add to internal contactlist item list.
#if 0
    addGroup( Kopete::Group::topLevel() );
#endif

    QDomElement element = list.firstChild().toElement();
    while( !element.isNull() )
    {
        if( element.tagName() == QString::fromLatin1("meta-contact") )
        {
            //TODO: id isn't used
            //QString id = element.attribute( "id", QString::null );
            Kopete::MetaContact *metaContact = new Kopete::MetaContact();
            if ( !metaContact->fromXML( element ) )
            {
                delete metaContact;
                metaContact = 0;
            }
            else
            {
                 //TODO: Add to internal contactlist item list.
#if 0
                Kopete::ContactList::self()->addMetaContact(
                        metaContact );
#endif
            }
        }
        else if( element.tagName() == QString::fromLatin1("kopete-group") )
        {
            Kopete::Group *group = new Kopete::Group();
            if( !group->fromXML( element ) )
            {
                delete group;
                group = 0;
            }
            else
            {
                //TODO: Add to internal contactlist item list.
#if 0
                Kopete::ContactList::self()->addGroup( group );
#endif
            }
        }
        // Only load myself metacontact information when Global Identity is enabled.
        else if( element.tagName() == QString::fromLatin1("myself-meta-contact") && Kopete::GeneralSettings::self()->enableGlobalIdentity() )
        {
             //TODO: Add to internal contactlist item list.
#if 0
            if( !myself()->fromXML( element ) )
            {
                delete d->myself;
                d->myself = 0;
            }
#endif
        }
        else
        {
            kWarning(14010) << "Kopete::ContactList::loadXML: "
                    << "Unknown element '" << element.tagName()
                    << "' in contact list!" << endl;
        }
        element = element.nextSibling().toElement();
    }
    contactListFile.close();
    d->loaded=true;
}

void XmlContactStorage::save()
{

}

}

//kate: indent-mode cstyle; indent-width 4; indent-spaces on; replace-tabs on;

