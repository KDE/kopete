/*
    Kopete Contact List XML Storage Class

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
#include <klocale.h>

// Kopete includes
#include "kopetemetacontact.h"
#include "kopetegroup.h"
#include "kopetegeneralsettings.h"
#include "kopetepluginmanager.h"

namespace Kopete
{

    // this is just to save typing
    const QString NSCID_ELEM = QString::fromUtf8( "nameSourceContactId" );
    const QString NSPID_ELEM = QString::fromUtf8( "nameSourcePluginId" );
    const QString NSAID_ELEM = QString::fromUtf8( "nameSourceAccountId" );
    const QString PSCID_ELEM = QString::fromUtf8( "photoSourceContactId" );
    const QString PSPID_ELEM = QString::fromUtf8( "photoSourcePluginId" );
    const QString PSAID_ELEM = QString::fromUtf8( "photoSourceAccountId" );

class XmlContactStorage::Private
{
public:
    Private()
    : loaded(false), isValid(false)
    {}

    bool loaded;
    bool isValid;
    QString xmlFilename;
    QString errorMessage;
};


XmlContactStorage::XmlContactStorage()
        : ContactListStorage(), d(new Private)
{
}

XmlContactStorage::XmlContactStorage(const QString &fileName)
    : ContactListStorage(), d(new Private)
{
    d->xmlFilename = fileName;
}

XmlContactStorage::~XmlContactStorage()
{
    delete d;
}

bool XmlContactStorage::isValid() const
{
    return d->isValid;
}

QString XmlContactStorage::errorMessage() const
{
    return d->errorMessage;
}

void XmlContactStorage::load()
{
    // don't save when we're in the middle of this...
    d->loaded = false;

    QString filename;
    if( !d->xmlFilename.isEmpty() )
    {
        filename = d->xmlFilename;
    }
    else
    {
        filename = KStandardDirs::locateLocal( "appdata", QLatin1String( "contactlist.xml" ) );
    }

    if( filename.isEmpty() )
    {
        d->loaded=true;
        d->isValid = false;
        d->errorMessage = i18n("Could not find contactlist.xml in Kopete application data.");
        return;
    }

    QDomDocument contactList( QString::fromLatin1( "kopete-contact-list" ) );

    QFile contactListFile( filename );
    contactListFile.open( QIODevice::ReadOnly );
    contactList.setContent( &contactListFile );

    QDomElement list = contactList.documentElement();
    //TODO: Check if we remove versioning support from XML file.
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
        contactList = QDomDocument ( QLatin1String( "kopete-contact-list" ) );

        contactListFile.open( QIODevice::ReadOnly );
        contactList.setContent( &contactListFile );

        list = contactList.documentElement();
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
            if ( !parseMetaContact( metaContact, element ) )
            {
                delete metaContact;
                metaContact = 0;
            }
            else
            {
                addMetaContact( metaContact );
            }
        }
        else if( element.tagName() == QString::fromLatin1("kopete-group") )
        {
            Kopete::Group *group = new Kopete::Group();
            if( !parseGroup( group, element ) )
            {
                delete group;
                group = 0;
            }
            else
            {
                addGroup( group );
            }
        }
        // Only load myself metacontact information when Global Identity is enabled.
#if 0
        else if( element.tagName() == QString::fromLatin1("myself-meta-contact") && Kopete::GeneralSettings::self()->enableGlobalIdentity() )
        {
            //TODO: Add to internal contactlist item list.

            if( !myself()->fromXML( element ) )
            {
                delete d->myself;
                d->myself = 0;
            }

        }
#endif
        else
        {
            kWarning(14010) << k_funcinfo
                    << "Unknown element '" << element.tagName()
                    << "' in XML contact list storage!" << endl;
        }
        element = element.nextSibling().toElement();
    }
    contactListFile.close();
    d->isValid = true;
    d->loaded=true;
}

void XmlContactStorage::save()
{
    // TODO:
}

bool XmlContactStorage::parseMetaContact( Kopete::MetaContact *metaContact, const QDomElement &element )
{
    if( !element.hasChildNodes() )
        return false;

    bool oldPhotoTracking = false;
    bool oldNameTracking = false;
    QString nameSourcePID, nameSourceAID, nameSourceCID;
    QString photoSourcePID, photoSourceAID, photoSourceCID;

    metaContact->setLoading( true );

    QString strContactId = element.attribute( QString::fromUtf8("contactId") );
    if( !strContactId.isEmpty() )
    {
        metaContact->setMetaContactId( strContactId );
        // Set the KABC Picture
        //metaContact->slotUpdateAddressBookPicture(); moved to MetaContact::setMetaContactId
    }

    QDomElement contactElement = element.firstChild().toElement();
    while( !contactElement.isNull() )
    {
        if( contactElement.tagName() == QString::fromUtf8( "display-name" ) )
        { // custom display name, used for the custom name source

            // WTF, why were we not loading the metacontact if nickname was empty.
            //if ( contactElement.text().isEmpty() )
            //  return false;

            //the replace is there to workaround the Bug 95444
            //TODO check this, setDisplayName do more than just assignment
            metaContact->setDisplayName( contactElement.text().replace('\n',QString::fromUtf8("")) );

            if ( contactElement.hasAttribute(NSCID_ELEM) && contactElement.hasAttribute(NSPID_ELEM) && contactElement.hasAttribute(NSAID_ELEM))
            {
                oldNameTracking = true;
                //kDebug(14010) << k_funcinfo << "old name tracking" << endl;
                // retrieve deprecated data (now stored in property-sources)
                // save temporarely, we will find a Contact* with this later
                nameSourceCID = contactElement.attribute( NSCID_ELEM );
                nameSourcePID = contactElement.attribute( NSPID_ELEM );
                nameSourceAID = contactElement.attribute( NSAID_ELEM );
            }
//          else
//              kDebug(14010) << k_funcinfo << "no old name tracking" << endl;
        }
        else if( contactElement.tagName() == QString::fromUtf8( "photo" ) )
        {
            // custom photo, used for custom photo source
            metaContact->setPhoto( KUrl(contactElement.text()) );

            bool photoSyncedWithKABC = (contactElement.attribute(QString::fromUtf8("syncWithKABC")) == QString::fromUtf8("1")) || (contactElement.attribute(QString::fromUtf8("syncWithKABC")) == QString::fromUtf8("true"));
            metaContact->setPhotoSyncedWithKABC( photoSyncedWithKABC );

            // retrieve deprecated data (now stored in property-sources)
            // save temporarely, we will find a Contact* with this later
            if ( contactElement.hasAttribute(PSCID_ELEM) && contactElement.hasAttribute(PSPID_ELEM) && contactElement.hasAttribute(PSAID_ELEM))
            {
                oldPhotoTracking = true;
//              kDebug(14010) << k_funcinfo << "old photo tracking" << endl;
                photoSourceCID = contactElement.attribute( PSCID_ELEM );
                photoSourcePID = contactElement.attribute( PSPID_ELEM );
                photoSourceAID = contactElement.attribute( PSAID_ELEM );
            }
//          else
//              kDebug(14010) << k_funcinfo << "no old photo tracking" << endl;
        }
        else if( contactElement.tagName() == QString::fromUtf8( "property-sources" ) )
        {
            QDomNode property = contactElement.firstChild();
            while( !property.isNull() )
            {
                QDomElement propertyElement = property.toElement();

                if( propertyElement.tagName() == QString::fromUtf8( "name" ) )
                {
                    QString source = propertyElement.attribute( QString::fromUtf8("source") );
                    metaContact->setDisplayNameSource(stringToSource(source));
                    // find contact sources now.
                    QDomNode propertyParam = propertyElement.firstChild();
                    while( !propertyParam.isNull() )
                    {
                        QDomElement propertyParamElement = propertyParam.toElement();
                        if( propertyParamElement.tagName() == QString::fromUtf8( "contact-source" ) )
                        {
                            nameSourceCID = propertyParamElement.attribute( NSCID_ELEM );
                            nameSourcePID = propertyParamElement.attribute( NSPID_ELEM );
                            nameSourceAID = propertyParamElement.attribute( NSAID_ELEM );
                        }
                        propertyParam = propertyParam.nextSibling();
                    }
                }
                if( propertyElement.tagName() == QString::fromUtf8( "photo" ) )
                {
                    QString source = propertyElement.attribute( QString::fromUtf8("source") );
                    metaContact->setPhotoSource(stringToSource(source));
                    // find contact sources now.
                    QDomNode propertyParam = propertyElement.firstChild();
                    while( !propertyParam.isNull() )
                    {
                        QDomElement propertyParamElement = propertyParam.toElement();
                        if( propertyParamElement.tagName() == QString::fromUtf8( "contact-source" ) )
                        {
                            photoSourceCID = propertyParamElement.attribute( NSCID_ELEM );
                            photoSourcePID = propertyParamElement.attribute( NSPID_ELEM );
                            photoSourceAID = propertyParamElement.attribute( NSAID_ELEM );
                        }
                        propertyParam = propertyParam.nextSibling();
                    }
                }
                property = property.nextSibling();
            }
        }
        else if( contactElement.tagName() == QString::fromUtf8( "groups" ) )
        {
            QDomNode group = contactElement.firstChild();
            while( !group.isNull() )
            {
                QDomElement groupElement = group.toElement();

                if( groupElement.tagName() == QString::fromUtf8( "group" ) )
                {
                    QString strGroupId = groupElement.attribute( QString::fromUtf8("id") );
                    if( !strGroupId.isEmpty() )
                        metaContact->addToGroup( this->group( strGroupId.toUInt() ) );
                    else //kopete 0.6 contactlist
                        metaContact->addToGroup( this->findGroup( groupElement.text() ) );
                }
                else if( groupElement.tagName() == QString::fromUtf8( "top-level" ) ) //kopete 0.6 contactlist
                    metaContact->addToGroup( Kopete::Group::topLevel() );

                group = group.nextSibling();
            }
        }
        else if( contactElement.tagName() == QString::fromUtf8( "address-book-field" ) )
        {
            QString app = contactElement.attribute( QString::fromUtf8( "app" ), QString::null );
            QString key = contactElement.attribute( QString::fromUtf8( "key" ), QString::null );
            QString val = contactElement.text();
            metaContact->setAddressBookField( 0, app, key , val );
        }
        else //if( groupElement.tagName() == QString::fromUtf8( "plugin-data" ) || groupElement.tagName() == QString::fromUtf8("custom-icons" ))
        {
            parseContactListElement( metaContact, contactElement );
        }
        contactElement = contactElement.nextSibling().toElement();
    }

    if( oldNameTracking )
    {
        /* if (displayNameSourceContact() )  <- doesn't work because the contact is only set up when all plugin are loaded (BUG 111956) */
        if ( !nameSourceCID.isEmpty() )
        {
//          kDebug(14010) << k_funcinfo << "Converting old name source" << endl;
            // even if the old tracking attributes exists, they could have been null, that means custom
            metaContact->setDisplayNameSource( Kopete::MetaContact::SourceContact );
        }
        else
        {
            // lets do the best conversion for the old name tracking
            // if the custom display name is the same as kabc name, set the source to kabc
            if ( !metaContact->metaContactId().isEmpty() && ( metaContact->customDisplayName() == nameFromKABC( metaContact->metaContactId() )) )
                metaContact->setDisplayNameSource( Kopete::MetaContact::SourceKABC );
            else
                metaContact->setDisplayNameSource( Kopete::MetaContact::SourceCustom );
        }
    }

    if ( oldPhotoTracking )
    {
//      kDebug(14010) << k_funcinfo << "Converting old photo source" << endl;
        if ( !photoSourceCID.isEmpty() )
        {
            metaContact->setPhotoSource( Kopete::MetaContact::SourceContact );
        }
        else
        {
            if ( !metaContact->metaContactId().isEmpty() && !photoFromKABC( metaContact->metaContactId() ).isNull() )
                metaContact->setPhotoSource( Kopete::MetaContact::SourceKABC );
            else
                metaContact->setPhotoSource( Kopete::MetaContact::SourceCustom );
        }
    }

    //FIXME if we ensure that XmlContactStorage is still in memory when signal allPluginsLoaded is emited than
    // slotAllPluginsLoaded can be in this object with helper QStrings from MetaConctact ( nameSourcePID, nameSourceAID, nameSourceCID,
    // photoSourcePID, photoSourceAID, photoSourceCID ).
    metaContact->setPhotoSource( photoSourcePID, photoSourceAID, photoSourceCID );
    metaContact->setDisplayNameSource( nameSourcePID, nameSourceAID, nameSourceCID );

    // If a plugin is loaded, load data cached
    QObject::connect( Kopete::PluginManager::self(), SIGNAL( pluginLoaded(Kopete::Plugin*) ),
                      metaContact, SLOT( slotPluginLoaded(Kopete::Plugin*) ) );

    // All plugins are already loaded, call manually the contact setting slot.
    if( Kopete::PluginManager::self()->isAllPluginsLoaded() )
        metaContact->slotAllPluginsLoaded();
    else
        // When all plugins are loaded, set the source contact.
        QObject::connect( Kopete::PluginManager::self(), SIGNAL( allPluginsLoaded() ),
                          metaContact, SLOT( slotAllPluginsLoaded() ) );

// track changes only works if ONE Contact is inside the MetaContact
//  if (d->contacts.count() > 1) // Does NOT work as intended
//      d->trackChildNameChanges=false;

//  kDebug(14010) << k_funcinfo << "END" << endl;
    metaContact->setLoading( false );
    return true;
}

bool XmlContactStorage::parseGroup(Kopete::Group *group, const QDomElement &element)
{
    group->setLoading( true );

    QString strGroupId = element.attribute( QLatin1String( "groupId" ) );
    if ( !strGroupId.isEmpty() )
    {
        group->setGroupId( strGroupId.toUInt() );
        if ( group->groupId() > group->uniqueGroupId() )
            group->setUniqueGroupId( group->groupId() );
    }

    // Don't overwrite type for Temporary and TopLevel groups
    if ( group->type() != Kopete::Group::Temporary && group->type() != Kopete::Group::TopLevel )
    {
        QString type = element.attribute( QLatin1String( "type" ), QLatin1String( "standard" ) );
        if ( type == QLatin1String( "temporary" ) )
        {
            if ( group->type() != Kopete::Group::Temporary )
            {
                parseGroup( Kopete::Group::temporary(), element );
                return false;
            }
        }
        else if ( type == QLatin1String( "top-level" ) )
        {
            if ( group->type() != Kopete::Group::TopLevel )
            {
                parseGroup( Kopete::Group::topLevel(), element );
                return false;
            }
        }
        else
        {
            group->setType( Kopete::Group::Normal );
        }
    }

    QString view = element.attribute( QLatin1String( "view" ), QLatin1String( "expanded" ) );
    bool expanded = ( view != QLatin1String( "collapsed" ) );
    group->setExpanded( expanded );

    QDomNode groupData = element.firstChild();
    while ( !groupData.isNull() )
    {
        QDomElement groupElement = groupData.toElement();
        if ( groupElement.tagName() == QLatin1String( "display-name" ) )
        {
            // Don't set display name for temporary or top-level items
            if ( group->type() == Kopete::Group::Normal )
                group->setDisplayName( groupElement.text() );
        }
        else
        {
            parseContactListElement( group, groupElement );
        }

        groupData = groupData.nextSibling();
    }

    // Sanity checks. We must not have groups without a displayname.
    // FIXME or TODO: This sanity check should be done once, not by each ContactListStorage
    if ( group->displayName().isEmpty() )
    {
        switch ( group->type() )
        {
            case Kopete::Group::Temporary:
                group->setDisplayName( QLatin1String( "Temporary" ) );
                break;
            case Kopete::Group::TopLevel:
                group->setDisplayName( QLatin1String( "Top-Level" ) );
                break;
            default:
                group->setDisplayName( i18n( "(Unnamed Group)" ) );
                break;
        }
    }

    group->setLoading( false );
    
    //this allows to save data for the top-level group in the top-level group
    return ( group->type() == Kopete::Group::Normal );
}

bool XmlContactStorage::parseContactListElement( Kopete::ContactListElement *contactListElement, const QDomElement &element )
{
    if ( element.tagName() == QLatin1String( "plugin-data" ) )
    {
        QMap<QString, QString> pluginData;
        QString pluginId = element.attribute( QLatin1String( "plugin-id" ), QString::null );

        //in kopete 0.6 the AIM protocol was called OSCAR
        if ( pluginId == QLatin1String( "OscarProtocol" ) )
            pluginId = QLatin1String( "AIMProtocol" );

        QDomNode field = element.firstChild();
        while( !field.isNull() )
        {
            QDomElement fieldElement = field.toElement();
            if ( fieldElement.tagName() == QLatin1String( "plugin-data-field" ) )
            {
                pluginData.insert( fieldElement.attribute( QLatin1String( "key" ),
                                   QLatin1String( "undefined-key" ) ), fieldElement.text() );
            }
            field = field.nextSibling();
        }
        contactListElement->setPluginData( pluginId, pluginData );
    }
    else if ( element.tagName() == QLatin1String( "custom-icons" ) )
    {
        contactListElement->setUseCustomIcon( element.attribute( QLatin1String( "use" ), QLatin1String( "1" ) ) == QLatin1String( "1" ) );
        QDomNode ic = element.firstChild();
        while( !ic.isNull() )
        {
            QDomElement iconElement = ic.toElement();
            if ( iconElement.tagName() == QLatin1String( "icon" ) )
            {
                QString stateStr = iconElement.attribute( QLatin1String( "state" ), QString::null );
                QString icon = iconElement.text();
                ContactListElement::IconState state = ContactListElement::None;

                if ( stateStr == QLatin1String( "open" ) )
                    state = ContactListElement::Open;
                if ( stateStr == QLatin1String( "closed" ) )
                    state = ContactListElement::Closed;
                if ( stateStr == QLatin1String( "online" ) )
                    state = ContactListElement::Online;
                if ( stateStr == QLatin1String( "offline" ) )
                    state = ContactListElement::Offline;
                if ( stateStr == QLatin1String( "away" ) )
                    state = ContactListElement::Away;
                if ( stateStr == QLatin1String( "unknown" ) )
                    state = ContactListElement::Unknown;

                contactListElement->setIcon( icon, state );
            }
            ic = ic.nextSibling();
        }
    }
    else
    {
        return false;
    }

    return true;
}

QString XmlContactStorage::sourceToString( Kopete::MetaContact::PropertySource source ) const
{
    if ( source == Kopete::MetaContact::SourceCustom )
        return QString::fromUtf8("custom");
    else if ( source == Kopete::MetaContact::SourceKABC )
        return QString::fromUtf8("addressbook");
    else if ( source == Kopete::MetaContact::SourceContact )
        return QString::fromUtf8("contact");
    else // recovery
        return sourceToString( Kopete::MetaContact::SourceCustom );
}

Kopete::MetaContact::PropertySource XmlContactStorage::stringToSource( const QString &name ) const
{
    if ( name == QString::fromUtf8("custom") )
        return Kopete::MetaContact::SourceCustom;
    else if ( name == QString::fromUtf8("addressbook") )
        return Kopete::MetaContact::SourceKABC;
    else if ( name == QString::fromUtf8("contact") )
        return Kopete::MetaContact::SourceContact;
    else // recovery
        return Kopete::MetaContact::SourceCustom;
}

}

//kate: indent-mode cstyle; indent-width 4; indent-spaces on; replace-tabs on;
