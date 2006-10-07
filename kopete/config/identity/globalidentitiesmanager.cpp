/*
    globalidentitiesmanager.h  -  Kopete Global identities manager.

    Copyright (c) 2005      by Michaël Larouche       <larouche@kde.org>

    Kopete    (c) 2003-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "globalidentitiesmanager.h"

// Qt includes
#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>

// KDE includes
#include <kdebug.h>
#include <ksavefile.h>
#include <klocale.h>
#include <kurl.h>
#include <kstandarddirs.h>

// Kopete includes
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetepluginmanager.h"
#include "kopeteaccount.h"
#include "kopeteprotocol.h"

// this is just to save typing
const QString NSCID_ELEM = QString::fromUtf8( "nameSourceContactId" );
const QString NSPID_ELEM = QString::fromUtf8( "nameSourcePluginId" );
const QString NSAID_ELEM = QString::fromUtf8( "nameSourceAccountId" );
const QString PSCID_ELEM = QString::fromUtf8( "photoSourceContactId" );
const QString PSPID_ELEM = QString::fromUtf8( "photoSourcePluginId" );
const QString PSAID_ELEM = QString::fromUtf8( "photoSourceAccountId" );

class GlobalIdentitiesManager::Private
{
public:
	QMap<QString, Kopete::MetaContact*> identitiesList;
};

GlobalIdentitiesManager *GlobalIdentitiesManager::s_self = 0L;
GlobalIdentitiesManager *GlobalIdentitiesManager::self()
{
	if ( !s_self )
		s_self = new GlobalIdentitiesManager;

	return s_self;
}

GlobalIdentitiesManager::GlobalIdentitiesManager(QObject *parent)
        : QObject(parent)
{
	d = new Private;
}

GlobalIdentitiesManager::~GlobalIdentitiesManager()
{
	s_self = 0L;

	delete d;
}

void GlobalIdentitiesManager::createNewIdentity(const QString &identityName)
{
	// Create new identity metacontact based on myself to get the sub-contacts.
	Kopete::MetaContact *newIdentity = createNewMetaContact();
	
	// Add to internal list.
	d->identitiesList.insert(identityName, newIdentity);
}

void GlobalIdentitiesManager::copyIdentity(const QString &copyIdentityName, const QString &sourceIdentity)
{
	Kopete::MetaContact *copyIdentity = createCopyMetaContact(d->identitiesList[sourceIdentity]);
	
	d->identitiesList.insert(copyIdentityName, copyIdentity);
}

void GlobalIdentitiesManager::renameIdentity(const QString &oldName, const QString &newName)
{
	Kopete::MetaContact *renamedIdentity = d->identitiesList[oldName];
	d->identitiesList.remove(oldName);
	d->identitiesList.insert(newName, renamedIdentity);
}

void GlobalIdentitiesManager::removeIdentity(const QString &removedIdentity)
{
	// Clear from memory the identity metacontact.
	delete d->identitiesList[removedIdentity];
	// Remove from the list.
	d->identitiesList.remove(removedIdentity);
}

void GlobalIdentitiesManager::updateIdentity(const QString &updatedIdentity, Kopete::MetaContact *sourceMetaContact)
{
	copyMetaContact(d->identitiesList[updatedIdentity], sourceMetaContact);
}

bool GlobalIdentitiesManager::isIdentityPresent(const QString &identityName)
{
	QMap<QString, Kopete::MetaContact*>::iterator it;
	QMap<QString, Kopete::MetaContact*>::iterator end = d->identitiesList.end();

	for(it = d->identitiesList.begin(); it != end; ++it)
	{
		if(it.key() == identityName)
		{
			// A entry with the same name was found.
			return true;
		}
	}
	return false;
}

Kopete::MetaContact *GlobalIdentitiesManager::getIdentity(const QString &identityName)
{
	// Check if the identity is present.
	return isIdentityPresent(identityName) ? d->identitiesList[identityName] : 0;
}

void GlobalIdentitiesManager::loadXML()
{
	kDebug() << k_funcinfo << "Loading global identities list from XML." << endl;

	QString filename = KStandardDirs::locateLocal( "appdata", QString::fromUtf8("global-identities.xml") );
	if( filename.isEmpty() )
	{
		return;
	}

	QDomDocument globalIdentitiesList( QString::fromUtf8( "kopete-global-identities-list" ) );
	
	QFile globalIdentitiesListFile( filename );
	globalIdentitiesListFile.open( QIODevice::ReadOnly );
	globalIdentitiesList.setContent( &globalIdentitiesListFile );

	QDomElement list = globalIdentitiesList.documentElement();
	QDomElement element = list.firstChild().toElement();
	while( !element.isNull() )
	{
		if( element.tagName() == QString::fromUtf8("identity") )
		{
			Kopete::MetaContact *metaContact = createNewMetaContact();
			QString identityName = element.attribute(QString::fromUtf8("name"));

			if( !parseMetaContact( metaContact, element ) )
			{
				delete metaContact;
				metaContact = 0L;
			}
			else
			{
				d->identitiesList.insert(identityName, metaContact);
			}
		}
		element = element.nextSibling().toElement();
	}

	// If no identity are loaded, create a default identity MetaContact.
	if(d->identitiesList.empty())
	{
		createNewIdentity(i18n("Default Identity"));
	}
}

void GlobalIdentitiesManager::saveXML()
{
	kDebug() << k_funcinfo << "Saving global identities list to XML." << endl;

	QString globalIdentitiesListFileName = KStandardDirs::locateLocal( "appdata", QString::fromUtf8("global-identities.xml") );
	KSaveFile globalIdentitiesListFile(globalIdentitiesListFileName);
	if( globalIdentitiesListFile.status() == 0 )
	{
		QTextStream *stream = globalIdentitiesListFile.textStream();
		stream->setCodec(QTextCodec::codecForName("UTF-8"));
		toXML().save( *stream, 4 );

		if ( globalIdentitiesListFile.close() )
		{
			return;
		}
		else
		{
			kDebug(14000) << k_funcinfo << "Failed to write global identities list, error code is: " << globalIdentitiesListFile.status() << endl;
		}
	}
	else
	{
		kWarning(14000) << k_funcinfo << "Couldn't open global identities list file " << globalIdentitiesListFileName
			<< ". Global Identities list not saved." << endl;
	}
}

const QDomDocument GlobalIdentitiesManager::toXML()
{
	QDomDocument doc;
	
	doc.appendChild(doc.createElement(QString::fromUtf8("kopete-global-identities-list")));
	
	QMap<QString, Kopete::MetaContact*>::iterator it;
	QMap<QString, Kopete::MetaContact*>::iterator end = d->identitiesList.end();
	for(it = d->identitiesList.begin(); it != end; ++it)
	{
		kDebug(14000) << k_funcinfo << "Saving " << it.key() << endl;
		QDomElement identityMetaContactElement = storeMetaContact( it.value() ); // Save minimal information.
		identityMetaContactElement.setTagName(QString::fromUtf8("identity"));
		identityMetaContactElement.setAttribute(QString::fromUtf8("name"), it.key());
		doc.documentElement().appendChild(doc.importNode(identityMetaContactElement, true));
	}

	return doc;
}

Kopete::MetaContact *GlobalIdentitiesManager::createNewMetaContact()
{
	Kopete::MetaContact *newMetaContact = new Kopete::MetaContact();
	QList<Kopete::Contact*> contactList = Kopete::ContactList::self()->myself()->contacts();

	// Copy the contacts list to the new metacontact, so Kopete::Contact for SourceContact
	// will not be null.
	QList<Kopete::Contact*>::iterator it;
	for ( it = contactList.begin(); it != contactList.end(); ++it )
	{
		newMetaContact->addContact( (*it) );
	}

	newMetaContact->setDisplayNameSource(Kopete::MetaContact::SourceCustom);
	newMetaContact->setPhotoSource(Kopete::MetaContact::SourceCustom);

	return newMetaContact;
}

Kopete::MetaContact *GlobalIdentitiesManager::createCopyMetaContact(Kopete::MetaContact *source)
{
	Kopete::MetaContact *copyMetaContactObject = createNewMetaContact();

	copyMetaContact(copyMetaContactObject, source);
	
	return copyMetaContactObject;
}

void GlobalIdentitiesManager::copyMetaContact(Kopete::MetaContact *destination, Kopete::MetaContact *source)
{
	destination->setDisplayName(source->customDisplayName());
	destination->setDisplayNameSource(source->displayNameSource());
	destination->setDisplayNameSourceContact(source->displayNameSourceContact());
	
	destination->setPhoto(source->customPhoto());
	destination->setPhotoSource(source->photoSource());
	destination->setPhotoSourceContact(source->photoSourceContact());
}

QMap<QString, Kopete::MetaContact*> GlobalIdentitiesManager::getGlobalIdentitiesList()
{
	return d->identitiesList;
}

// Load minimal information only.
bool GlobalIdentitiesManager::parseMetaContact( Kopete::MetaContact *metaContact, const QDomElement &element )
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
		else if( contactElement.tagName() == QString::fromUtf8( "address-book-field" ) )
		{
			QString app = contactElement.attribute( QString::fromUtf8( "app" ), QString::null );
			QString key = contactElement.attribute( QString::fromUtf8( "key" ), QString::null );
			QString val = contactElement.text();
			metaContact->setAddressBookField( 0, app, key , val );
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
			if ( !metaContact->metaContactId().isEmpty() && ( metaContact->customDisplayName() == Kopete::nameFromKABC( metaContact->metaContactId() )) )
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
			if ( !metaContact->metaContactId().isEmpty() && !Kopete::photoFromKABC( metaContact->metaContactId() ).isNull() )
				metaContact->setPhotoSource( Kopete::MetaContact::SourceKABC );
			else
				metaContact->setPhotoSource( Kopete::MetaContact::SourceCustom );
		}
	}
	
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

// Save minimal information only.
const QDomElement GlobalIdentitiesManager::storeMetaContact( Kopete::MetaContact *metaContact ) const
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
        //kDebug(14010) << k_funcinfo << "serializing photo source " << nameFromContact(photoSourceContact()) << endl;
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
	return metaContactDoc.documentElement();
}

QString GlobalIdentitiesManager::sourceToString( Kopete::MetaContact::PropertySource source ) const
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

Kopete::MetaContact::PropertySource GlobalIdentitiesManager::stringToSource( const QString &name ) const
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

#include "globalidentitiesmanager.moc"
