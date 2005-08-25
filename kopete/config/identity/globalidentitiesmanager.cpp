/*
    globalidentitiesmanager.h  -  Kopete Global identities manager.

    Copyright (c) 2005      by Michaël Larouche       <michael.larouche@kdemail.net>

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

GlobalIdentitiesManager::GlobalIdentitiesManager(QObject *parent, const char *name)
        : QObject(parent, name)
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
	QMapIterator<QString, Kopete::MetaContact*> it;
	QMapIterator<QString, Kopete::MetaContact*> end = d->identitiesList.end();

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
	kdDebug() << k_funcinfo << "Loading global identities list from XML." << endl;

	QString filename = locateLocal( "appdata", QString::fromUtf8("global-identities.xml") );
	if( filename.isEmpty() )
	{
		return;
	}

	QDomDocument globalIdentitiesList( QString::fromUtf8( "kopete-global-identities-list" ) );
	
	QFile globalIdentitiesListFile( filename );
	globalIdentitiesListFile.open( IO_ReadOnly );
	globalIdentitiesList.setContent( &globalIdentitiesListFile );

	QDomElement list = globalIdentitiesList.documentElement();
	QDomElement element = list.firstChild().toElement();
	while( !element.isNull() )
	{
		if( element.tagName() == QString::fromUtf8("identity") )
		{
			Kopete::MetaContact *metaContact = createNewMetaContact();
			QString identityName = element.attribute(QString::fromUtf8("name"));

			if(!metaContact->fromXML(element))
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
	kdDebug() << k_funcinfo << "Saving global identities list to XML." << endl;

	QString globalIdentitiesListFileName = locateLocal( "appdata", QString::fromUtf8("global-identities.xml") );
	KSaveFile globalIdentitiesListFile(globalIdentitiesListFileName);
	if( globalIdentitiesListFile.status() == 0 )
	{
		QTextStream *stream = globalIdentitiesListFile.textStream();
		stream->setEncoding( QTextStream::UnicodeUTF8 );
		toXML().save( *stream, 4 );

		if ( globalIdentitiesListFile.close() )
		{
			return;
		}
		else
		{
			kdDebug(14000) << k_funcinfo << "Failed to write global identities list, error code is: " << globalIdentitiesListFile.status() << endl;
		}
	}
	else
	{
		kdWarning(14000) << k_funcinfo << "Couldn't open global identities list file " << globalIdentitiesListFileName
			<< ". Global Identities list not saved." << endl;
	}
}

const QDomDocument GlobalIdentitiesManager::toXML()
{
	QDomDocument doc;
	
	doc.appendChild(doc.createElement(QString::fromUtf8("kopete-global-identities-list")));
	
	QMapIterator<QString, Kopete::MetaContact*> it;
	QMapIterator<QString, Kopete::MetaContact*> end = d->identitiesList.end();
	for(it = d->identitiesList.begin(); it != end; ++it)
	{
		kdDebug(14000) << k_funcinfo << "Saving " << it.key() << endl;
		QDomElement identityMetaContactElement = it.data()->toXML(true); // Save minimal information.
		identityMetaContactElement.setTagName(QString::fromUtf8("identity"));
		identityMetaContactElement.setAttribute(QString::fromUtf8("name"), it.key());
		doc.documentElement().appendChild(doc.importNode(identityMetaContactElement, true));
	}

	return doc;
}

Kopete::MetaContact *GlobalIdentitiesManager::createNewMetaContact()
{
	Kopete::MetaContact *newMetaContact = new Kopete::MetaContact();
	QPtrList<Kopete::Contact> contactList = Kopete::ContactList::self()->myself()->contacts();

	// Copy the contacts list to the new metacontact, so Kopete::Contact for SourceContact
	// will not be null.
	QPtrListIterator<Kopete::Contact> it( contactList);
	for ( ; it.current(); ++it )
	{
		newMetaContact->addContact(it.current());
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

#include "globalidentitiesmanager.moc"
