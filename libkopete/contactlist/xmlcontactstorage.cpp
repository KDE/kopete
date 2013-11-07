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
#include "xmlcontactstorage.h"
#ifdef Q_WS_X11
#include <fixx11h.h>
#endif

// Qt includes
#include <QtCore/QFile>
#include <QtCore/QUuid>
#include <QtCore/QRegExp>
#include <QtCore/QLatin1String>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

// KDE includes
#include <kdebug.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <ksavefile.h>

// Kopete includes
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopetegroup.h"
#include "kopetecontact.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
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
    : isBusy(false), isValid(false), version(0)
    {}

    bool isBusy;
    bool isValid;
    QString xmlFilename;
    QString errorMessage;
    uint version;

    /**
     * Current contact list version * 10 ( i.e. '10' is version '1.0' )
     */
    static const uint ContactListVersion = 12;
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

bool XmlContactStorage::isBusy() const
{
    return d->isBusy;
}

void XmlContactStorage::load()
{
    if ( isBusy() )
        return;

    d->isBusy = true;

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
        d->isValid = false;
        d->errorMessage = i18n("Could not find contactlist.xml in Kopete application data.");
        d->isBusy = false;
        return;
    }

    QDomDocument contactList( QString::fromLatin1( "kopete-contact-list" ) );

    QFile contactListFile( filename );
    contactListFile.open( QIODevice::ReadOnly );
    contactList.setContent( &contactListFile );

    QDomElement list = contactList.documentElement();

    d->version = readVersion( list );
    if( d->version < Private::ContactListVersion )
    {
        QFile::copy( filename, filename + QString::fromLatin1(".bak_v%1").arg( d->version ) );

        if ( d->version == 10 )
        {
            updateFrom10to11( list );
            d->version = readVersion( list );
        }
        if ( d->version == 11 )
        {
            updateFrom11to12( list );
            d->version = readVersion( list );
        }

        if ( d->version < Private::ContactListVersion )
        {
            contactListFile.close();
            kWarning(14010) << "The contact list on disk is older than expected or cannot be updated!"
                            << "No contact list will be loaded";
            d->isValid = false;
            d->isBusy = false;
            return;
        }
    }

    // First load all groups so we can assign them to metaContacts
    QDomElement element = list.firstChild().toElement();
    while( !element.isNull() )
    {
        if( element.tagName() == QString::fromLatin1("kopete-group") )
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
        element = element.nextSibling().toElement();
    }

    // above parseGroup will always fail for top level group, so add it here
    addGroup( Kopete::Group::topLevel() );

    // Load metaContacts
    element = list.firstChild().toElement();
    while( !element.isNull() )
    {
        if( element.tagName() == QString::fromLatin1("meta-contact") )
        {
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
        else if( element.tagName() != QString::fromLatin1("kopete-group") )
        {
            kWarning(14010) << "Unknown element '" << element.tagName() << "' in XML contact list storage!" << endl;
        }
        element = element.nextSibling().toElement();
    }

    checkGroupIds();

    contactListFile.close();
    d->isValid = true;
    d->isBusy = false;
}

void XmlContactStorage::save()
{
    if ( isBusy() )
        return;

    d->isBusy = true;

    QString filename;
    if( !d->xmlFilename.isEmpty() )
    {
        filename = d->xmlFilename;
    }
    else
    {
        filename = KStandardDirs::locateLocal( "appdata", QLatin1String( "contactlist.xml" ) );
    }

    KSaveFile contactListFile( filename );
    if( !contactListFile.open() )
    {
        d->isValid = false;
        d->errorMessage = i18n( "Could not open contact list file." );
        d->isBusy = false;
        return;
    }

    QDomDocument doc;
    doc.appendChild( doc.createElement( QLatin1String("kopete-contact-list") ) );
    doc.documentElement().setAttribute( QLatin1String("version"), QLatin1String("1.2"));

    // Save group information. ie: Open/Closed, pehaps later icons? Who knows.
    doc.documentElement().appendChild( doc.importNode( storeGroup( Kopete::Group::topLevel() ), true ) );

    Kopete::Group::List groupList = Kopete::ContactList::self()->groups();
    foreach( Kopete::Group *group, groupList )
    {
        QDomNode node = doc.importNode( storeGroup( group ), true );
        doc.documentElement().appendChild( node );
    }

    // Save metacontact information.
    Kopete::MetaContact::List metaContactList = Kopete::ContactList::self()->metaContacts();
    foreach( Kopete::MetaContact *metaContact, metaContactList )
    {
        if( !metaContact->isTemporary() )
        {
            QDomNode node = doc.importNode( storeMetaContact( metaContact ), true );
            doc.documentElement().appendChild( node );
        }
    }

    QTextStream stream ( &contactListFile );
    stream.setCodec(QTextCodec::codecForName("UTF-8"));
    doc.save( stream, 4 );

    if ( !contactListFile.finalize() )
    {
        d->isValid = false;
        d->errorMessage = i18n( "Could not write contact list to a file." );
        d->isBusy = false;
        return;
    }

    d->isValid = true;
    d->isBusy = false;
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
        metaContact->setMetaContactId( QUuid( strContactId ) );
    }
    else
        return false; // we can't handle metacontacts that on't have unique ids

    QString strKabcId = element.attribute( QString::fromUtf8( "kabcId" ) );
    if ( !strKabcId.isEmpty() )
        metaContact->setKabcId( strKabcId );

    QDomElement contactElement = element.firstChild().toElement();
    while( !contactElement.isNull() )
    {
        if( contactElement.tagName() == QString::fromUtf8( "display-name" ) )
        { // custom display name, used for the custom name source

            //the replace is there to workaround the Bug 95444
            metaContact->setDisplayName( contactElement.text().remove('\n') );

            if ( contactElement.hasAttribute(NSCID_ELEM) && contactElement.hasAttribute(NSPID_ELEM) && contactElement.hasAttribute(NSAID_ELEM))
            {
                oldNameTracking = true;
                //kDebug(14010) << "old name tracking";
                // retrieve deprecated data (now stored in property-sources)
                // save temporarely, we will find a Contact* with this later
                nameSourceCID = contactElement.attribute( NSCID_ELEM );
                nameSourcePID = contactElement.attribute( NSPID_ELEM );
                nameSourceAID = contactElement.attribute( NSAID_ELEM );
            }
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
//              kDebug(14010) << "old photo tracking";
                photoSourceCID = contactElement.attribute( PSCID_ELEM );
                photoSourcePID = contactElement.attribute( PSPID_ELEM );
                photoSourceAID = contactElement.attribute( PSAID_ELEM );
            }
//          else
//              kDebug(14010) << "no old photo tracking";
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
                    else //kopete 0.6 contact list
                        metaContact->addToGroup( this->findGroup( groupElement.text() ) );
                }
                else if( groupElement.tagName() == QString::fromUtf8( "top-level" ) ) //kopete 0.6 contactlist
                    metaContact->addToGroup( Kopete::Group::topLevel() );

                group = group.nextSibling();
            }
        }
        else if( contactElement.tagName() == QString::fromUtf8( "address-book-field" ) )
        {
            QString app = contactElement.attribute( QString::fromUtf8( "app" ), QString() );
            QString key = contactElement.attribute( QString::fromUtf8( "key" ), QString() );
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
//          kDebug(14010) << "Converting old name source";
            // even if the old tracking attributes exists, they could have been null, that means custom
            metaContact->setDisplayNameSource( Kopete::MetaContact::SourceContact );
        }
        else
        {
            // lets do the best conversion for the old name tracking
            // if the custom display name is the same as kabc name, set the source to kabc
            if ( !metaContact->kabcId().isEmpty() && ( metaContact->customDisplayName() == nameFromKABC( metaContact->kabcId() )) )
                metaContact->setDisplayNameSource( Kopete::MetaContact::SourceKABC );
            else
                metaContact->setDisplayNameSource( Kopete::MetaContact::SourceCustom );
        }
    }

    if ( oldPhotoTracking )
    {
//      kDebug(14010) << "Converting old photo source";
        if ( !photoSourceCID.isEmpty() )
        {
            metaContact->setPhotoSource( Kopete::MetaContact::SourceContact );
        }
        else
        {
            if ( !metaContact->kabcId().isEmpty() && !photoFromKABC( metaContact->kabcId() ).isNull() )
                metaContact->setPhotoSource( Kopete::MetaContact::SourceKABC );
            else
                metaContact->setPhotoSource( Kopete::MetaContact::SourceCustom );
        }
    }

    //FIXME if we ensure that XmlContactStorage is still in memory when signal allPluginsLoaded is emitted than
    // slotAllPluginsLoaded can be in this object with helper QStrings from MetaConctact ( nameSourcePID, nameSourceAID, nameSourceCID,
    // photoSourcePID, photoSourceAID, photoSourceCID ).
    metaContact->setPhotoSource( photoSourcePID, photoSourceAID, photoSourceCID );
    metaContact->setDisplayNameSource( nameSourcePID, nameSourceAID, nameSourceCID );

    // If a plugin is loaded, load data cached
    QObject::connect( Kopete::PluginManager::self(), SIGNAL(pluginLoaded(Kopete::Plugin*)),
                      metaContact, SLOT(slotPluginLoaded(Kopete::Plugin*)) );
    QObject::connect( Kopete::PluginManager::self(), SIGNAL(protocolLoaded(Kopete::Protocol*)),
                      metaContact, SLOT(slotProtocolLoaded(Kopete::Protocol*)) );

    // All plugins are already loaded, call manually the contact setting slot.
    if( Kopete::PluginManager::self()->isAllPluginsLoaded() )
        metaContact->slotAllPluginsLoaded();
    else
        // When all plugins are loaded, set the source contact.
        QObject::connect( Kopete::PluginManager::self(), SIGNAL(allPluginsLoaded()),
                          metaContact, SLOT(slotAllPluginsLoaded()) );

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
    if ( group->type() != Kopete::Group::Temporary && group->type() != Kopete::Group::TopLevel && group->type() != Kopete::Group::Offline )
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
        else if ( type == QLatin1String( "offline" ) )
        {
            if ( group->type() != Kopete::Group::Offline )
            {
                parseGroup( Kopete::Group::offline(), element );
                return false;
            }
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
            case Kopete::Group::Offline:
                group->setDisplayName( QLatin1String( "Offline" ) );
                break;
            default:
                group->setLoading( false );
                return false;
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
        QString pluginId = element.attribute( QLatin1String( "plugin-id" ), QString() );

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
    else if ( element.tagName() == QLatin1String( "plugin-contact-data" ) )
    {
        QMap<QString, QString> pluginData;
        QString pluginId = element.attribute( QLatin1String( "plugin-id" ), QString() );

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
        contactListElement->appendPluginContactData( pluginId, pluginData );
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
                QString stateStr = iconElement.attribute( QLatin1String( "state" ), QString() );
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

const QDomElement XmlContactStorage::storeMetaContact( Kopete::MetaContact *metaContact, bool minimal ) const
{
    // This causes each Kopete::Protocol subclass to serialise its contacts' data into the metacontact's plugin data and address book data
    metaContact->serialize();

    QDomDocument metaContactDoc;
    metaContactDoc.appendChild( metaContactDoc.createElement( QString::fromUtf8( "meta-contact" ) ) );
    metaContactDoc.documentElement().setAttribute( QString::fromUtf8( "contactId" ), metaContact->metaContactId() );
    metaContactDoc.documentElement().setAttribute( QString::fromUtf8( "kabcId" ), metaContact->kabcId() );

    // the custom display name, used for the custom name source
    QDomElement displayName = metaContactDoc.createElement( QString::fromUtf8("display-name" ) );
    displayName.appendChild( metaContactDoc.createTextNode( metaContact->customDisplayName() ) );
    metaContactDoc.documentElement().appendChild( displayName );

    QDomElement photo = metaContactDoc.createElement( QString::fromUtf8("photo" ) );
    photo.appendChild( metaContactDoc.createTextNode( metaContact->customPhoto().url() ) );
    metaContactDoc.documentElement().appendChild( photo );

    // Property sources
    QDomElement propertySources = metaContactDoc.createElement( QString::fromUtf8("property-sources" ) );
    QDomElement _nameSource = metaContactDoc.createElement( QString::fromUtf8("name") );
    QDomElement _photoSource = metaContactDoc.createElement( QString::fromUtf8("photo") );

    // set the contact source for display name
    _nameSource.setAttribute( QString::fromUtf8("source"), sourceToString( metaContact->displayNameSource() ) );

    // set contact source metadata
    if( metaContact->displayNameSourceContact() )
    {
        QDomElement contactNameSource = metaContactDoc.createElement( QString::fromUtf8("contact-source") );
        contactNameSource.setAttribute( NSCID_ELEM, metaContact->displayNameSourceContact()->contactId() );
        contactNameSource.setAttribute( NSPID_ELEM, metaContact->displayNameSourceContact()->protocol()->pluginId() );
        contactNameSource.setAttribute( NSAID_ELEM, metaContact->displayNameSourceContact()->account()->accountId() );
        _nameSource.appendChild( contactNameSource );
    }

    // set the contact source for photo
    _photoSource.setAttribute( QString::fromUtf8("source"), sourceToString( metaContact->photoSource() ) );

    if( !metaContact->kabcId().isEmpty()  )
        photo.setAttribute( QString::fromUtf8("syncWithKABC") , QString::fromUtf8( metaContact->isPhotoSyncedWithKABC() ? "true" : "false" ) );

    if( metaContact->photoSourceContact() )
    {
        //kDebug(14010) << "serializing photo source " << nameFromContact(photoSourceContact());
        // set contact source metadata for photo
        QDomElement contactPhotoSource = metaContactDoc.createElement( QString::fromUtf8("contact-source") );
        contactPhotoSource.setAttribute( NSCID_ELEM, metaContact->photoSourceContact()->contactId() );
        contactPhotoSource.setAttribute( NSPID_ELEM, metaContact->photoSourceContact()->protocol()->pluginId() );
        contactPhotoSource.setAttribute( NSAID_ELEM, metaContact->photoSourceContact()->account()->accountId() );
        _photoSource.appendChild( contactPhotoSource );
    }
    // apend name and photo sources to property sources
    propertySources.appendChild( _nameSource );
    propertySources.appendChild( _photoSource );

    metaContactDoc.documentElement().appendChild( propertySources );

    // Don't store these information in minimal mode.
    if(!minimal)
    {
        // Store groups
        if ( !metaContact->groups().isEmpty() )
        {
            QDomElement groups = metaContactDoc.createElement( QString::fromUtf8("groups") );
            foreach( Group *g, metaContact->groups() )
            {
                QDomElement group = metaContactDoc.createElement( QString::fromUtf8("group") );
                group.setAttribute( QString::fromUtf8("id"), g->groupId() );
                groups.appendChild( group );
            }
            metaContactDoc.documentElement().appendChild( groups );
        }

        // Store other plugin data
        const QList<QDomElement> pluginNodes = storeContactListElement( metaContact );
        foreach ( const QDomElement &it , pluginNodes )
            metaContactDoc.documentElement().appendChild( metaContactDoc.importNode( it, true ) );
    }
    return metaContactDoc.documentElement();
}

const QDomElement XmlContactStorage::storeGroup( Kopete::Group *group ) const
{
    QDomDocument groupDoc;
    groupDoc.appendChild( groupDoc.createElement( QLatin1String( "kopete-group" ) ) );
    groupDoc.documentElement().setAttribute( QLatin1String( "groupId" ), QString::number( group->groupId() ) );

    QString type;
    switch ( group->type() )
    {
        case Kopete::Group::Temporary:
            type = QLatin1String( "temporary" );
            break;
        case Kopete::Group::TopLevel:
            type = QLatin1String( "top-level" );
            break;
        case Kopete::Group::Offline:
            type = QLatin1String( "offline" );
            break;

        default:
            type = QLatin1String( "standard" ); // == Normal
            break;
    }

    groupDoc.documentElement().setAttribute( QLatin1String( "type" ), type );
    groupDoc.documentElement().setAttribute( QLatin1String( "view" ), QLatin1String( group->isExpanded() ? "expanded" : "collapsed" )  );

    QDomElement displayName = groupDoc.createElement( QLatin1String( "display-name" ) );
    displayName.appendChild( groupDoc.createTextNode( group->displayName() ) );
    groupDoc.documentElement().appendChild( displayName );

    // Store other plugin data
    const QList<QDomElement> pluginNodes = storeContactListElement( group );
    foreach ( const QDomElement &it , pluginNodes )
        groupDoc.documentElement().appendChild( groupDoc.importNode( it, true ) );


    return groupDoc.documentElement();

}

const QList<QDomElement> XmlContactStorage::storeContactListElement( Kopete::ContactListElement *contactListElement ) const
{
    QDomDocument pluginData;
    QList<QDomElement> pluginNodes;
    pluginData.appendChild( pluginData.createElement( QLatin1String( "plugin-data" ) ) );

    const Kopete::ContactListElement::PluginDataMap plugins = contactListElement->pluginData();
    if ( !plugins.isEmpty() )
    {
        Kopete::ContactListElement::PluginDataMap::ConstIterator pluginIt, pluginItEnd = plugins.end();
        for ( pluginIt = plugins.begin(); pluginIt != pluginItEnd; ++pluginIt )
        {
            QDomElement pluginElement = pluginData.createElement( QLatin1String( "plugin-data" ) );
            pluginElement.setAttribute( QLatin1String( "plugin-id" ), pluginIt.key()  );

            QMap<QString, QString>::ConstIterator it;
            for ( it = pluginIt.value().begin(); it != pluginIt.value().end(); ++it )
            {
                QDomElement pluginDataField = pluginData.createElement( QLatin1String( "plugin-data-field" ) );
                pluginDataField.setAttribute( QLatin1String( "key" ), it.key()  );
                pluginDataField.appendChild( pluginData.createTextNode(  it.value()  ) );
                pluginElement.appendChild( pluginDataField );
            }

            pluginData.documentElement().appendChild( pluginElement );
            pluginNodes.append( pluginElement );
        }
    }

    const QMap<QString, Kopete::ContactListElement::ContactDataList > pluginsContactData = contactListElement->pluginContactData();
    if ( !pluginsContactData.isEmpty() )
    {
        QMap<QString, Kopete::ContactListElement::ContactDataList >::ConstIterator pluginIt, pluginItEnd = pluginsContactData.end();
        for ( pluginIt = pluginsContactData.begin(); pluginIt != pluginItEnd; ++pluginIt )
        {
            foreach ( Kopete::ContactListElement::ContactData dataItem, pluginIt.value() )
            {
                QDomElement pluginElement = pluginData.createElement( QLatin1String( "plugin-contact-data" ) );
                pluginElement.setAttribute( QLatin1String( "plugin-id" ), pluginIt.key()  );

                QMap<QString, QString>::ConstIterator it;
                for ( it = dataItem.constBegin(); it != dataItem.constEnd(); ++it )
                {
                    QDomElement pluginDataField = pluginData.createElement( QLatin1String( "plugin-data-field" ) );
                    pluginDataField.setAttribute( QLatin1String( "key" ), it.key()  );
                    pluginDataField.appendChild( pluginData.createTextNode(  it.value()  ) );
                    pluginElement.appendChild( pluginDataField );
                }

                pluginData.documentElement().appendChild( pluginElement );
                pluginNodes.append( pluginElement );
            }
        }
    }

    const Kopete::ContactListElement::IconMap icons = contactListElement->icons();
    if ( !icons.isEmpty() )
    {
        QDomElement iconsElement = pluginData.createElement( QLatin1String( "custom-icons" ) );
        iconsElement.setAttribute( QLatin1String( "use" ), contactListElement->useCustomIcon() ?  QLatin1String( "1" ) : QLatin1String( "0" ) );

        Kopete::ContactListElement::IconMap::ConstIterator it, itEnd = icons.end();
        for ( it = icons.begin(); it != itEnd; ++it )
        {
            QDomElement iconElement = pluginData.createElement( QLatin1String( "icon" ) );
            QString stateStr;
            switch ( it.key() )
            {
                case ContactListElement::Open:
                    stateStr = QLatin1String( "open" );
                    break;
                case ContactListElement::Closed:
                    stateStr = QLatin1String( "closed" );
                    break;
                case ContactListElement::Online:
                    stateStr = QLatin1String( "online" );
                    break;
                case ContactListElement::Away:
                    stateStr = QLatin1String( "away" );
                    break;
                case ContactListElement::Offline:
                    stateStr = QLatin1String( "offline" );
                    break;
                case ContactListElement::Unknown:
                    stateStr = QLatin1String( "unknown" );
                    break;
                case ContactListElement::None:
                default:
                    stateStr = QLatin1String( "none" );
                    break;
            }
            iconElement.setAttribute( QLatin1String( "state" ), stateStr );
            iconElement.appendChild( pluginData.createTextNode( it.value() )  );
            iconsElement.appendChild( iconElement );
        }
        pluginData.documentElement().appendChild( iconsElement );
        pluginNodes.append( iconsElement );
    }
    return pluginNodes;
}

bool XmlContactStorage::updateFrom10to11( QDomElement &rootElement ) const
{
    QDomNodeList metaContactElements = rootElement.elementsByTagName( QLatin1String( "meta-contact" ) );
    for ( int i = 0; i < metaContactElements.count(); ++i )
    {
        QDomNode node = metaContactElements.at( i );
        QDomElement element = node.toElement();
        if ( element.hasAttribute( QLatin1String("contactId") ) )
        {
            QString kabcId;
            QString contactId = element.attribute( "contactId" );
            QUuid newContactId = QUuid::createUuid();

            if ( !contactId.contains( ':' ) )
                element.setAttribute( QLatin1String("kabcId"), contactId );

            element.setAttribute( QLatin1String("contactId"), newContactId.toString() );
        }
    }
    rootElement.setAttribute( QString("version"), "1.1" );
    return true;
}

bool XmlContactStorage::updateFrom11to12( QDomElement &rootElement ) const
{
    QDomNodeList metaContactElementList = rootElement.elementsByTagName( QLatin1String( "meta-contact" ) );
    for ( int i = 0; i < metaContactElementList.count(); ++i )
    {
        typedef QMap<QString, QString> PluginData;
        typedef QPair<QString, PluginData> ProtocolIdDataPair;

        QList<QDomElement> removeList;
        QList<ProtocolIdDataPair> newList;

        QDomElement metaContactElement = metaContactElementList.at( i ).toElement();
        QDomNodeList pluginElementList = metaContactElement.elementsByTagName( QLatin1String( "plugin-data" ) );
        for ( int j = 0; j < pluginElementList.count(); ++j )
        {
            QDomElement element = pluginElementList.at( j ).toElement();
            QString pluginId = element.attribute( QLatin1String( "plugin-id" ), QString() );
            if ( !pluginId.endsWith( "Protocol" ) )
                continue;

            QMap<QString, QStringList> serializedData;

            // Read data to serializedData
            QDomNode field = element.firstChild();
            while ( !field.isNull() )
            {
                QDomElement fieldElement = field.toElement();
                if ( fieldElement.tagName() == QLatin1String( "plugin-data-field" ) )
                {
                    QString key = fieldElement.attribute( QLatin1String( "key" ), QLatin1String( "undefined-key" ) );
                    serializedData[key] = fieldElement.text().split( QChar( 0xE000 ), QString::KeepEmptyParts );
                }
                field = field.nextSibling();
            }

            // Split serializedData by contact
            int count = serializedData[QLatin1String("contactId")].count();
            for ( int i = 0; i < count ; i++ )
            {
                QMap<QString, QString> sd;

                QMap<QString, QStringList>::Iterator it;
                QMap<QString, QStringList>::Iterator itEnd = serializedData.end();
                for ( it = serializedData.begin(); it != itEnd; ++it )
                {
                    QStringList sl = it.value();
                    if( sl.count() > i)
                        sd[it.key()] = sl.value( i );
                }
                newList.append( ProtocolIdDataPair( pluginId, sd ) );
            }

            removeList.append( element );
        }

        foreach( QDomElement e, removeList )
            metaContactElement.removeChild( e );

        foreach( ProtocolIdDataPair pdp, newList )
        {
            QDomElement pluginElement = metaContactElement.ownerDocument().createElement( QLatin1String( "plugin-contact-data" ) );
            pluginElement.setAttribute( QLatin1String( "plugin-id" ), pdp.first  );

            QMap<QString, QString>::ConstIterator it;
            for ( it = pdp.second.constBegin(); it != pdp.second.constEnd(); ++it )
            {
                QDomElement pluginDataField = metaContactElement.ownerDocument().createElement( QLatin1String( "plugin-data-field" ) );
                pluginDataField.setAttribute( QLatin1String( "key" ), it.key()  );
                pluginDataField.appendChild( metaContactElement.ownerDocument().createTextNode(  it.value()  ) );
                pluginElement.appendChild( pluginDataField );
            }

            metaContactElement.appendChild( pluginElement );
        }
    }
    rootElement.setAttribute( QString("version"), "1.2" );
    return true;
}

uint XmlContactStorage::readVersion( QDomElement &rootElement ) const
{
    QString versionString = rootElement.attribute( QString::fromLatin1( "version" ), QString() );
    if( QRegExp( QString::fromLatin1( "[0-9]+\\.[0-9]" ) ).exactMatch( versionString ) )
        return versionString.remove( QLatin1Char( '.' ) ).toUInt();
    else
        return 0;
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

void XmlContactStorage::checkGroupIds()
{
    // HACK: Check if we don't have duplicate groupIds => broken contactlist.xml,
    // if so reset all groupIds so there are unique.
    // Will break manual group sorting but we want consistent contactlist.

    QSet<uint> groupIdSet;

    bool idsUnique = true;
    bool haveTemporary = false;
    bool haveTopLevel = false;

    foreach( Kopete::Group * group, groups() )
    {
        if ( groupIdSet.contains( group->groupId() ) )
        {
            idsUnique = false;
            break;
        }
        groupIdSet.insert( group->groupId() );
        if ( group->type() == Kopete::Group::Temporary )
            haveTemporary = true;
        else if ( group->type() == Kopete::Group::TopLevel )
            haveTopLevel = true;
    }

    if ( !haveTemporary )
        groupIdSet.insert(Kopete::Group::topLevel()->groupId());

    if ( !haveTopLevel )
        groupIdSet.insert(Kopete::Group::topLevel()->groupId());

    if ( !idsUnique )
    {
        uint uniqueGroupId = 0;
        Kopete::Group::topLevel()->setGroupId( ++uniqueGroupId );
        Kopete::Group::temporary()->setGroupId( ++uniqueGroupId );

        foreach( Kopete::Group * group, groups() )
            group->setGroupId( ++uniqueGroupId );

        Kopete::Group::topLevel()->setUniqueGroupId( uniqueGroupId );
    }
}

}

//kate: indent-mode cstyle; indent-width 4; indent-spaces on; replace-tabs on;
