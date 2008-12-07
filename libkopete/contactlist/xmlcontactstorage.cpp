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
#ifdef Q_WS_X11
#include <fixx11h.h>
#endif
#include "xmlcontactstorage.h"

// Qt includes
#include <QtCore/QFile>
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
    : isBusy(false), isValid(false)
    {}

    bool isBusy;
    bool isValid;
    QString xmlFilename;
    QString errorMessage;

    /**
     * Current contact list version * 10 ( i.e. '10' is version '1.0' )
     */
    static const uint ContactListVersion = 10;
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
    //TODO: Check if we remove versioning support from XML file.
#if 0
    QString versionString = list.attribute( QString::fromLatin1( "version" ), QString() );
    uint version = 0;
    if( QRegExp( QString::fromLatin1( "[0-9]+\\.[0-9]" ) ).exactMatch( versionString ) )
        version = versionString.replace( QString::fromLatin1( "." ), QString() ).toUInt();

    if( version < Private::ContactListVersion )
    {
        // The version string is invalid, or we're using an older version.
        // Convert first and reparse the file afterwards
        kDebug( 14010 ) << "Contact list version " << version
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
//TODO: Add to internal contact list item list.
#if 0
    addGroup( Kopete::Group::topLevel() );
#endif

    QDomElement element = list.firstChild().toElement();
    while( !element.isNull() )
    {
        if( element.tagName() == QString::fromLatin1("meta-contact") )
        {
            //TODO: id isn't used
            //QString id = element.attribute( "id", QString() );
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
        else
        {
            kWarning(14010) 
                    << "Unknown element '" << element.tagName()
                    << "' in XML contact list storage!" << endl;
        }
        element = element.nextSibling().toElement();
    }
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
    doc.documentElement().setAttribute( QLatin1String("version"), QLatin1String("1.0"));

    // Save group information. ie: Open/Closed, pehaps later icons? Who knows.
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
            metaContact->setDisplayName( contactElement.text().replace('\n',QString::fromUtf8("")) );

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
//          else
//              kDebug(14010) << "no old name tracking";
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
            if ( !metaContact->metaContactId().isEmpty() && ( metaContact->customDisplayName() == nameFromKABC( metaContact->metaContactId() )) )
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
            if ( !metaContact->metaContactId().isEmpty() && !photoFromKABC( metaContact->metaContactId() ).isNull() )
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

//  kDebug(14010) << "END";
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
        QString pluginId = element.attribute( QLatin1String( "plugin-id" ), QString() );

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
    metaContact->emitAboutToSave();

    QDomDocument metaContactDoc;
    metaContactDoc.appendChild( metaContactDoc.createElement( QString::fromUtf8( "meta-contact" ) ) );
    metaContactDoc.documentElement().setAttribute( QString::fromUtf8( "contactId" ), metaContact->metaContactId() );

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

    if( !metaContact->metaContactId().isEmpty()  )
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

void XmlContactStorage::convertContactList( const QString &fileName, uint /* fromVersion */, uint /* toVersion */ )
{
	Q_UNUSED(fileName)
    // For now, ignore fromVersion and toVersion. These are meant for future
    // changes to allow incremental (multi-pass) conversion so we don't have
    // to rewrite the whole conversion code for each change.
#if 0
    QDomDocument contactList( QLatin1String( "messaging-contact-list" ) );
    QFile contactListFile( fileName );
    contactListFile.open( QIODevice::ReadOnly );
    contactList.setContent( &contactListFile );

    QDomElement oldList = contactList.documentElement();

    QDomDocument newList( QLatin1String( "kopete-contact-list" ) );
    newList.appendChild( newList.createProcessingInstruction( QLatin1String( "xml" ), QLatin1String( "version=\"1.0\"" ) ) );

    QDomElement newRoot = newList.createElement( QLatin1String( "kopete-contact-list" ) );
    newList.appendChild( newRoot );
    newRoot.setAttribute( QLatin1String( "version" ), QLatin1String( "1.0" ) );

    QDomNode oldNode = oldList.firstChild();
    while( !oldNode.isNull() )
    {
        QDomElement oldElement = oldNode.toElement();
        if( !oldElement.isNull() )
        {
            if( oldElement.tagName() == QLatin1String("meta-contact") )
            {
                // Ignore ID, it is not used in the current list anyway
                QDomElement newMetaContact = newList.createElement( QLatin1String( "meta-contact" ) );
                newRoot.appendChild( newMetaContact );

                // Plugin data is stored completely different, and requires
                // some bookkeeping to convert properly
                QMap<QString, QDomElement> pluginData;
                QStringList icqData;
                QStringList gaduData;

                // ICQ and Gadu can only be converted properly if the address book fields
                // are already parsed. Therefore, scan for those first and add the rest
                // afterwards
                QDomNode oldContactNode = oldNode.firstChild();
                while( !oldContactNode.isNull() )
                {
                    QDomElement oldContactElement = oldContactNode.toElement();
                    if( !oldContactElement.isNull() && oldContactElement.tagName() == QLatin1String("address-book-field") )
                    {
                        // Convert address book fields.
                        // Jabber will be called "xmpp", Aim/Toc and Aim/Oscar both will
                        // be called "aim". MSN, AIM, IRC, Oscar and SMS don't use address
                        // book fields yet; Gadu and ICQ can be converted as-is.
                        // As Yahoo is unfinished we won't try to convert at all.
                        QString id   = oldContactElement.attribute( QLatin1String( "id" ), QString() );
                        QString data = oldContactElement.text();

                        QString app, key, val;
                        QString separator = QLatin1String( "," );
                        if( id == QLatin1String( "messaging/gadu" ) )
                            separator = QLatin1String( "\n" );
                        else if( id == QLatin1String( "messaging/icq" ) )
                            separator = QLatin1String( ";" );
                        else if( id == QLatin1String( "messaging/jabber" ) )
                            id = QLatin1String( "messaging/xmpp" );

                        if( id == QLatin1String( "messaging/gadu" ) || id == QLatin1String( "messaging/icq" ) ||
                            id == QLatin1String( "messaging/winpopup" ) || id == QLatin1String( "messaging/xmpp" ) )
                        {
                            app = id;
                            key = QLatin1String( "All" );
                            val = data.replace( separator, QString( QChar( 0xE000 ) ) );
                        }

                        if( !app.isEmpty() )
                        {
                            QDomElement addressBookField = newList.createElement( QLatin1String( "address-book-field" ) );
                            newMetaContact.appendChild( addressBookField );

                            addressBookField.setAttribute( QLatin1String( "app" ), app );
                            addressBookField.setAttribute( QLatin1String( "key" ), key );

                            addressBookField.appendChild( newList.createTextNode( val ) );

                            // ICQ didn't store the contactId locally, only in the address
                            // book fields, so we need to be able to access it later
                            if( id == QLatin1String( "messaging/icq" ) )
                                icqData = val.split( QChar( 0xE000 ), QString::SkipEmptyParts );
                            else if( id == QLatin1String("messaging/gadu") )
                                gaduData = val.split( QChar( 0xE000 ), QString::SkipEmptyParts );
                        }
                    }
                    oldContactNode = oldContactNode.nextSibling();
                }

                // Now, convert the other elements
                oldContactNode = oldNode.firstChild();
                while( !oldContactNode.isNull() )
                {
                    QDomElement oldContactElement = oldContactNode.toElement();
                    if( !oldContactElement.isNull() )
                    {
                        if( oldContactElement.tagName() == QLatin1String("display-name") )
                        {
                            QDomElement displayName = newList.createElement( QLatin1String( "display-name" ) );
                            displayName.appendChild( newList.createTextNode( oldContactElement.text() ) );
                            newMetaContact.appendChild( displayName );
                        }
                        else if( oldContactElement.tagName() == QLatin1String("groups") )
                        {
                            QDomElement groups = newList.createElement( QLatin1String( "groups" ) );
                            newMetaContact.appendChild( groups );

                            QDomNode oldGroup = oldContactElement.firstChild();
                            while( !oldGroup.isNull() )
                            {
                                QDomElement oldGroupElement = oldGroup.toElement();
                                if ( oldGroupElement.tagName() == QLatin1String("group") )
                                {
                                    QDomElement group = newList.createElement( QLatin1String( "group" ) );
                                    group.appendChild( newList.createTextNode( oldGroupElement.text() ) );
                                    groups.appendChild( group );
                                }
                                else if ( oldGroupElement.tagName() == QLatin1String("top-level") )
                                {
                                    QDomElement group = newList.createElement( QLatin1String( "top-level" ) );
                                    groups.appendChild( group );
                                }

                                oldGroup = oldGroup.nextSibling();
                            }
                        }
                        else if( oldContactElement.tagName() == QLatin1String( "plugin-data" ) )
                        {
                            // Convert the plugin data
                            QString id   = oldContactElement.attribute( QLatin1String( "plugin-id" ), QString() );
                            QString data = oldContactElement.text();

                            bool convertOldAim = false;
                            uint fieldCount = 1;
                            QString addressBookLabel;
                            if( id == QLatin1String("MSNProtocol") )
                            {
                                fieldCount = 3;
                                addressBookLabel = QLatin1String("msn");
                            }
                            else if( id == QLatin1String("IRCProtocol") )
                            {
                                fieldCount = 3;
                                addressBookLabel = QLatin1String("irc");
                            }
                            else if( id == QLatin1String("OscarProtocol") )
                            {
                                fieldCount = 2;
                                addressBookLabel = QLatin1String("aim");
                            }
                            else if( id == QLatin1String("AIMProtocol") )
                            {
                                id = QLatin1String("OscarProtocol");
                                convertOldAim = true;
                                addressBookLabel = QLatin1String("aim");
                            }
                            else if( id == QLatin1String("ICQProtocol") || id == QLatin1String("WPProtocol") || id == QLatin1String("GaduProtocol") )
                            {
                                fieldCount = 1;
                            }
                            else if( id == QLatin1String("JabberProtocol") )
                            {
                                fieldCount = 4;
                            }
                            else if( id == QLatin1String("SMSProtocol") )
                            {
                                // SMS used a variable serializing using a dot as delimiter.
                                // The minimal count is three though (id, name, delimiter).
                                fieldCount = 2;
                                addressBookLabel = QLatin1String("sms");
                            }

                            if( pluginData[ id ].isNull() )
                            {
                                pluginData[ id ] = newList.createElement( QLatin1String( "plugin-data" ) );
                                pluginData[ id ].setAttribute( QLatin1String( "plugin-id" ), id );
                                newMetaContact.appendChild( pluginData[ id ] );
                            }

                            // Do the actual conversion
                            if( id == QLatin1String( "MSNProtocol" ) || id == QLatin1String( "OscarProtocol" ) ||
                                id == QLatin1String( "AIMProtocol" ) || id == QLatin1String( "IRCProtocol" ) ||
                                id == QLatin1String( "ICQProtocol" ) || id == QLatin1String( "JabberProtocol" ) ||
                                id == QLatin1String( "SMSProtocol" ) || id == QLatin1String( "WPProtocol" ) ||
                                id == QLatin1String( "GaduProtocol" ) )
                            {
                                QStringList strList = data.split( QLatin1String( "||" ), QString::SkipEmptyParts );

                                // Unescape '||'
                                for( QStringList::iterator it = strList.begin(); it != strList.end(); ++it )
                                {
                                    ( *it ).replace( QLatin1String( "\\|;" ), QLatin1String( "|" ) ).
                                            replace( QLatin1String( "\\\\" ), QLatin1String( "\\" ) );
                                }

                                uint idx = 0;
                                while( idx < (unsigned int)strList.size() )
                                {
                                    QDomElement dataField;

                                    dataField = newList.createElement( QLatin1String( "plugin-data-field" ) );
                                    pluginData[ id ].appendChild( dataField );
                                    dataField.setAttribute( QLatin1String( "key" ), QLatin1String( "contactId" ) );
                                    if( id == QLatin1String("ICQProtocol") )
                                        dataField.appendChild( newList.createTextNode( icqData[ idx ] ) );
                                    else if( id == QLatin1String("GaduProtocol") )
                                        dataField.appendChild( newList.createTextNode( gaduData[ idx ] ) );
                                    else if( id == QLatin1String("JabberProtocol") )
                                        dataField.appendChild( newList.createTextNode( strList[ idx + 1 ] ) );
                                    else
                                        dataField.appendChild( newList.createTextNode( strList[ idx ] ) );

                                    dataField = newList.createElement( QLatin1String( "plugin-data-field" ) );
                                    pluginData[ id ].appendChild( dataField );
                                    dataField.setAttribute( QLatin1String( "key" ), QLatin1String( "displayName" ) );
                                    if( convertOldAim || id == QLatin1String("ICQProtocol") || id == QLatin1String("WPProtocol") || id == QLatin1String("GaduProtocol") )
                                        dataField.appendChild( newList.createTextNode( strList[ idx ] ) );
                                    else if( id == QLatin1String("JabberProtocol") )
                                        dataField.appendChild( newList.createTextNode( strList[ idx + 2 ] ) );
                                    else
                                        dataField.appendChild( newList.createTextNode( strList[ idx + 1 ] ) );

                                    if( id == QLatin1String("MSNProtocol") )
                                    {
                                        dataField = newList.createElement( QLatin1String( "plugin-data-field" ) );
                                        pluginData[ id ].appendChild( dataField );
                                        dataField.setAttribute( QLatin1String( "key" ), QLatin1String( "groups" ) );
                                        dataField.appendChild( newList.createTextNode( strList[ idx + 2 ] ) );
                                    }
                                    else if( id == QLatin1String("IRCProtocol") )
                                    {
                                        dataField = newList.createElement( QLatin1String( "plugin-data-field" ) );
                                        pluginData[ id ].appendChild( dataField );
                                        dataField.setAttribute( QLatin1String( "key" ), QLatin1String( "serverName" ) );
                                        dataField.appendChild( newList.createTextNode( strList[ idx + 2 ] ) );
                                    }
                                    else if( id == QLatin1String("JabberProtocol") )
                                    {
                                        dataField = newList.createElement( QLatin1String( "plugin-data-field" ) );
                                        pluginData[ id ].appendChild( dataField );
                                        dataField.setAttribute( QLatin1String( "key" ), QLatin1String( "accountId" ) );
                                        dataField.appendChild( newList.createTextNode( strList[ idx ] ) );

                                        dataField = newList.createElement( QLatin1String( "plugin-data-field" ) );
                                        pluginData[ id ].appendChild( dataField );
                                        dataField.setAttribute( QLatin1String( "key" ), QLatin1String( "groups" ) );
                                        dataField.appendChild( newList.createTextNode( strList[ idx + 3 ] ) );
                                    }
                                    else if( id == QLatin1String( "SMSProtocol" ) &&
                                             ( idx + 2 < (uint)strList.size() ) && strList[ idx + 2 ] != QLatin1String( "." ) )
                                    {
                                        dataField = newList.createElement( QLatin1String( "plugin-data-field" ) );
                                        pluginData[ id ].appendChild( dataField );
                                        dataField.setAttribute( QLatin1String( "key" ), QLatin1String( "serviceName" ) );
                                        dataField.appendChild( newList.createTextNode( strList[ idx + 2 ] ) );

                                        dataField = newList.createElement( QLatin1String( "plugin-data-field" ) );
                                        pluginData[ id ].appendChild( dataField );
                                        dataField.setAttribute( QLatin1String( "key" ), QLatin1String( "servicePrefs" ) );
                                        dataField.appendChild( newList.createTextNode( strList[ idx + 3 ] ) );

                                        // Add extra fields
                                        idx += 2;
                                    }

                                    // MSN, AIM, IRC, Oscar and SMS didn't store address book fields up
                                    // to now, so create one
                                    if( id != QLatin1String("ICQProtocol") && id != QLatin1String("JabberProtocol") && id != QLatin1String("WPProtocol") && id != QLatin1String("GaduProtocol") )
                                    {
                                        QDomElement addressBookField = newList.createElement( QLatin1String( "address-book-field" ) );
                                        newMetaContact.appendChild( addressBookField );

                                        addressBookField.setAttribute( QLatin1String( "app" ),
                                                QLatin1String( "messaging/" ) + addressBookLabel );
                                        addressBookField.setAttribute( QLatin1String( "key" ), QLatin1String( "All" ) );
                                        addressBookField.appendChild( newList.createTextNode( strList[ idx ] ) );
                                    }

                                    idx += fieldCount;
                                }
                            }
                            else if( id == QLatin1String("ContactNotesPlugin") || id == QLatin1String("CryptographyPlugin") || id == QLatin1String("TranslatorPlugin") )
                            {
                                QDomElement dataField = newList.createElement( QLatin1String( "plugin-data-field" ) );
                                pluginData[ id ].appendChild( dataField );
                                if( id == QLatin1String("ContactNotesPlugin") )
                                    dataField.setAttribute( QLatin1String( "key" ), QLatin1String( "notes" ) );
                                else if( id == QLatin1String("CryptographyPlugin") )
                                    dataField.setAttribute( QLatin1String( "key" ), QLatin1String( "gpgKey" ) );
                                else if( id == QLatin1String("TranslatorPlugin") )
                                    dataField.setAttribute( QLatin1String( "key" ), QLatin1String( "languageKey" ) );

                                dataField.appendChild( newList.createTextNode( data ) );
                            }
                        }
                    }
                    oldContactNode = oldContactNode.nextSibling();
                }
            }
            else if( oldElement.tagName() == QLatin1String("kopete-group") )
            {
                QDomElement newGroup = newList.createElement( QLatin1String( "kopete-group" ) );
                newRoot.appendChild( newGroup );

                QDomNode oldGroupNode = oldNode.firstChild();
                while( !oldGroupNode.isNull() )
                {
                    QDomElement oldGroupElement = oldGroupNode.toElement();

                    if( oldGroupElement.tagName() == QLatin1String("display-name") )
                    {
                        QDomElement displayName = newList.createElement( QLatin1String( "display-name" ) );
                        displayName.appendChild( newList.createTextNode( oldGroupElement.text() ) );
                        newGroup.appendChild( displayName );
                    }
                    if( oldGroupElement.tagName() == QLatin1String("type") )
                    {
                        if( oldGroupElement.text() == QLatin1String("Temporary") )
                            newGroup.setAttribute( QLatin1String( "type" ), QLatin1String( "temporary" ) );
                        else if( oldGroupElement.text() == QLatin1String( "TopLevel" ) )
                            newGroup.setAttribute( QLatin1String( "type" ), QLatin1String( "top-level" ) );
                        else
                            newGroup.setAttribute( QLatin1String( "type" ), QLatin1String( "standard" ) );
                    }
                    if( oldGroupElement.tagName() == QLatin1String("view") )
                    {
                        if( oldGroupElement.text() == QLatin1String("collapsed") )
                            newGroup.setAttribute( QLatin1String( "view" ), QLatin1String( "collapsed" ) );
                        else
                            newGroup.setAttribute( QLatin1String( "view" ), QLatin1String( "expanded" ) );
                    }
                    else if( oldGroupElement.tagName() == QLatin1String("plugin-data") )
                    {
                        // Per-group plugin data
                        // FIXME: This needs updating too, ideally, convert this in a later
                        //        contactlist.xml version
                        QDomElement groupPluginData = newList.createElement( QLatin1String( "plugin-data" ) );
                        newGroup.appendChild( groupPluginData );

                        groupPluginData.setAttribute( QLatin1String( "plugin-id" ),
                                oldGroupElement.attribute( QLatin1String( "plugin-id" ), QString() ) );
                        groupPluginData.appendChild( newList.createTextNode( oldGroupElement.text() ) );
                    }

                    oldGroupNode = oldGroupNode.nextSibling();
                }
            }
            else
            {
                kWarning( 14010 ) << "Unknown element '" << oldElement.tagName()
                        << "' in contact list!" << endl;
            }
        }
        oldNode = oldNode.nextSibling();
    }

    // Close the file, and save the new file
    contactListFile.close();

    QDir().rename( fileName, fileName + QLatin1String( ".bak" ) );

    // kDebug( 14010 ) << "XML output:\n" << newList.toString( 2 );

    contactListFile.open( QIODevice::WriteOnly );
    QTextStream stream( &contactListFile );
    stream.setCodec(QTextCodec::codecForName("UTF-8"));
    stream << newList.toString( 2 );

    contactListFile.flush();
    contactListFile.close();
#endif
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
