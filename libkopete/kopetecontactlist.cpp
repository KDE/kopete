/*
    kopetecontactlist.cpp - Kopete's Contact List backend

    Copyright (c) 2005      by Michael Larouche       <michael.larouche@kdemail.net>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart @ kde.org>
    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>

    Copyright (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetecontactlist.h"

#include <qdir.h>
#include <qregexp.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QTextStream>

#include <kapplication.h>
#include <kabc/stdaddressbook.h>
#include <kdebug.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopetechatsession.h"
//#include "kopetemessage.h"
#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetegroup.h"
#include "kopetepicture.h"
#include "kopetegeneralsettings.h"
#include "xmlcontactstorage.h"

namespace  Kopete
{

class ContactList::Private
{public:
	/** Flag:  do not save the contactlist until she is completely loaded */
	bool loaded ;

	QList<MetaContact *> contacts;
	QList<Group *> groups;
	QList<MetaContact *> selectedMetaContacts;
	QList<Group *> selectedGroups;

	QTimer *saveTimer;

	MetaContact *myself;

	/** Flag: does the user uses the global identity */
	bool useGlobalIdentity;

	/**
	 * Current contact list version * 10 ( i.e. '10' is version '1.0' )
	 */
	static const uint ContactListVersion = 10;
};

ContactList *ContactList::s_self = 0L;

ContactList *ContactList::self()
{
	if( !s_self )
		s_self = new ContactList;

	return s_self;
}

ContactList::ContactList()
	: QObject( kapp )
{
	setObjectName( "KopeteContactList" );
	d=new Private;

	//the myself metacontact can't be created now, because it will use
	//ContactList::self() as parent which will call this constructor -> infinite loop
	d->myself=0L;

	//no contactlist loaded yet, don't save them
	d->loaded=false;

	// automatically save on changes to the list
	d->saveTimer = new QTimer( this );
	d->saveTimer->setObjectName( "saveTimer" );
	d->saveTimer->setSingleShot( true );
	connect( d->saveTimer, SIGNAL( timeout() ), SLOT ( save() ) );

	connect( this, SIGNAL( metaContactAdded( Kopete::MetaContact * ) ), SLOT( slotSaveLater() ) );
	connect( this, SIGNAL( metaContactRemoved( Kopete::MetaContact * ) ), SLOT( slotSaveLater() ) );
	connect( this, SIGNAL( groupAdded( Kopete::Group * ) ), SLOT( slotSaveLater() ) );
	connect( this, SIGNAL( groupRemoved( Kopete::Group * ) ), SLOT( slotSaveLater() ) );
	connect( this, SIGNAL( groupRenamed( Kopete::Group *, const QString & ) ), SLOT( slotSaveLater() ) );
}

ContactList::~ContactList()
{
	delete d->myself;
	delete d;
}

QList<MetaContact *> ContactList::metaContacts() const
{
	return d->contacts;
}


QList<Group *> ContactList::groups() const
{
	return d->groups;
}


MetaContact *ContactList::metaContact( const QString &metaContactId ) const
{
	QListIterator<MetaContact *> it( d->contacts );

	while ( it.hasNext() )
	{
		MetaContact *mc = it.next();
		if( mc->metaContactId() == metaContactId )
			return mc;
	}

	return 0L;
}


Group * ContactList::group(unsigned int groupId) const
{
	QListIterator<Group *> it(d->groups);
	
	while ( it.hasNext() )
	{
		Group *curr = it.next();
		if( curr->groupId()==groupId )
			return curr;
	}
	return 0L;
}


Contact *ContactList::findContact( const QString &protocolId,
	const QString &accountId, const QString &contactId ) const
{
	//Browsing metacontacts is too slow, better to uses the Dict of the account.
	Account *i=AccountManager::self()->findAccount(protocolId,accountId);
	if(!i)
	{
		kDebug( 14010 ) << k_funcinfo << "Account not found" << endl;
		return 0L;
	}
	return i->contacts()[contactId];
}


MetaContact *ContactList::findMetaContactByDisplayName( const QString &displayName ) const
{
	QListIterator<MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		MetaContact *mc = it.next();
//		kDebug(14010) << "Display Name: " << it.current()->displayName() << "\n";
		if( mc->displayName() == displayName ) {
			return mc;
		}
	}

	return 0L;
}

MetaContact* ContactList::findMetaContactByContactId( const QString &contactId ) const
{
	QListIterator<Kopete::Account *> it( Kopete::AccountManager::self()->accounts() );
	Kopete::Account *a;
	while ( it.hasNext() )
	{
		a = it.next();
		Contact *c=a->contacts()[contactId];
		if(c && c->metaContact())
			return c->metaContact();
	}
	return 0L;
}

Group * ContactList::findGroup(const QString& displayName, int type)
{
	if( type == Group::Temporary )
		return Group::temporary();

	QListIterator<Group *> it(d->groups);
	while ( it.hasNext() )
	{
		Group *curr = it.next();
		if( curr->type() == type && curr->displayName() == displayName )
			return curr;
	}

	Group *newGroup = new Group( displayName, (Group::GroupType)type );
	addGroup( newGroup );
	return  newGroup;
}


QList<MetaContact *> ContactList::selectedMetaContacts() const
{
	return d->selectedMetaContacts;
}

QList<Group *> ContactList::selectedGroups() const
{
	return d->selectedGroups;
}

void ContactList::addMetaContacts( QList<MetaContact *> metaContacts )
{
	foreach( MetaContact* mc, metaContacts )
		addMetaContact( mc );
}

void ContactList::addMetaContact( MetaContact *mc )
{
	if ( d->contacts.contains( mc ) )
		return;

	d->contacts.append( mc );

	emit metaContactAdded( mc );
	connect( mc, SIGNAL( persistentDataChanged( ) ), SLOT( slotSaveLater() ) );
	connect( mc, SIGNAL( addedToGroup( Kopete::MetaContact *, Kopete::Group * ) ), SIGNAL( metaContactAddedToGroup( Kopete::MetaContact *, Kopete::Group * ) ) );
	connect( mc, SIGNAL( removedFromGroup( Kopete::MetaContact *, Kopete::Group * ) ), SIGNAL( metaContactRemovedFromGroup( Kopete::MetaContact *, Kopete::Group * ) ) );
}


void ContactList::removeMetaContact(MetaContact *m)
{
	if ( !d->contacts.contains(m) )
	{
		kDebug(14010) << k_funcinfo << "Trying to remove a not listed MetaContact." << endl;
		return;
	}

	if ( d->selectedMetaContacts.contains( m ) )
	{
		d->selectedMetaContacts.removeAll( m );
		setSelectedItems( d->selectedMetaContacts, d->selectedGroups );
	}

	//removes subcontact from server here and now.
	QList<Contact *> cts = m->contacts();
	QListIterator<Contact *> it(cts);
	while( it.hasNext() )
	{
		it.next()->deleteContact();
	}

	d->contacts.removeAll( m );
	emit metaContactRemoved( m );
	m->deleteLater();
}

void ContactList::addGroups( QList<Group *> groups )
{
	foreach( Group* g, groups )
		addGroup( g );
}

void ContactList::addGroup( Group * g )
{
	if(!d->groups.contains(g) )
	{
		d->groups.append( g );
		emit groupAdded( g );
		connect( g , SIGNAL ( displayNameChanged(Kopete::Group* , const QString & )) , this , SIGNAL ( groupRenamed(Kopete::Group* , const QString & )) ) ;
	}
}

void ContactList::removeGroup( Group *g )
{
	if ( d->selectedGroups.contains( g ) )
	{
		d->selectedGroups.removeAll( g );
		setSelectedItems( d->selectedMetaContacts, d->selectedGroups );
	}

	d->groups.removeAll( g );
	emit groupRemoved( g );
	g->deleteLater();
}


void ContactList::setSelectedItems(QList<MetaContact *> metaContacts , QList<Group *> groups)
{
	kDebug( 14010 ) << k_funcinfo << metaContacts.count() << " metacontacts, " << groups.count() << " groups selected" << endl;
	d->selectedMetaContacts=metaContacts;
	d->selectedGroups=groups;

	emit metaContactSelected( groups.isEmpty() && metaContacts.count()==1 );
	emit selectionChanged();
}

MetaContact* ContactList::myself()
{
	if(!d->myself)
		d->myself=new MetaContact();
	return d->myself;
}

void ContactList::loadGlobalIdentity()
{
 	// Apply the global identity
	if(Kopete::GeneralSettings::self()->enableGlobalIdentity())
 	{
		// Disconnect to make sure it will not cause duplicate calls.
		disconnect(myself(), SIGNAL(displayNameChanged(const QString&, const QString&)), this, SLOT(slotDisplayNameChanged()));
		disconnect(myself(), SIGNAL(photoChanged()), this, SLOT(slotPhotoChanged()));

		connect(myself(), SIGNAL(displayNameChanged(const QString&, const QString&)), this, SLOT(slotDisplayNameChanged()));
		connect(myself(), SIGNAL(photoChanged()), this, SLOT(slotPhotoChanged()));

		// Ensure that the myself metaContactId is always the KABC whoAmI
		KABC::Addressee a = KABC::StdAddressBook::self()->whoAmI();
		if(!a.isEmpty() && a.uid() != myself()->metaContactId())
		{
			myself()->setMetaContactId(a.uid());
		}

		// Apply the global identity
		// Maybe one of the myself contact from a account has a different displayName/photo at startup.
		slotDisplayNameChanged();
		slotPhotoChanged();
 	}
	else
	{
		disconnect(myself(), SIGNAL(displayNameChanged(const QString&, const QString&)), this, SLOT(slotDisplayNameChanged()));
		disconnect(myself(), SIGNAL(photoChanged()), this, SLOT(slotPhotoChanged()));
	}
}

void ContactList::slotDisplayNameChanged()
{
	static bool mutex=false;
	if(mutex)
	{
		kDebug (14010) << k_funcinfo << " mutex blocked" << endl ;
		return;
	}
	mutex=true;

	kDebug( 14010 ) << k_funcinfo << myself()->displayName() << endl;

	emit globalIdentityChanged(Kopete::Global::Properties::self()->nickName().key(), myself()->displayName());
	mutex=false;
}

void ContactList::slotPhotoChanged()
{
	static bool mutex=false;
	if(mutex)
	{
		kDebug (14010) << k_funcinfo << " mutex blocked" << endl ;
		return;
	}
	mutex=true;
	kDebug( 14010 ) << k_funcinfo << myself()->picture().path() << endl;

	emit globalIdentityChanged(Kopete::Global::Properties::self()->photo().key(), myself()->picture().path());
	mutex=false;
	/* The mutex is useful to don't have such as stack overflow 
	Kopete::ContactList::slotPhotoChanged  ->  Kopete::ContactList::globalIdentityChanged  
	MSNAccount::slotGlobalIdentityChanged  ->  Kopete::Contact::propertyChanged 
	Kopete::MetaContact::slotPropertyChanged -> Kopete::MetaContact::photoChanged -> Kopete::ContactList::slotPhotoChanged 
	*/
}

///////////////////////////////////////////////////////////////////////////////////////////////
void ContactList::load()
{
	// don't save when we're in the middle of this...
	d->loaded = false;
	
	Kopete::ContactListStorage *storage = new Kopete::XmlContactStorage();
	storage->load();
	if( !storage->isValid() )
	{
		kDebug(14010) << k_funcinfo << "Contact list storage failed. Reason: " << storage->errorMessage() << endl;
		d->loaded = true;
		delete storage;
		return;
	}

	addGroups( storage->groups() );
	addMetaContacts( storage->contacts() );

	// Apply the global identity when all the protocols plugins are loaded.
	connect(PluginManager::self(), SIGNAL(allPluginsLoaded()), this, SLOT(loadGlobalIdentity()));
	
	d->loaded = true;
	delete storage;
}

void ContactList::loadXML()
{
	// don't save when we're in the middle of this...
	d->loaded = false;

	QString filename = KStandardDirs::locateLocal( "appdata", QLatin1String( "contactlist.xml" ) );
	if( filename.isEmpty() )
	{
		d->loaded=true;
		return ;
	}

	QDomDocument contactList( QLatin1String( "kopete-contact-list" ) );

	QFile contactListFile( filename );
	contactListFile.open( QIODevice::ReadOnly );
	contactList.setContent( &contactListFile );

	QDomElement list = contactList.documentElement();

	QString versionString = list.attribute( QLatin1String( "version" ), QString::null );
	uint version = 0;
	if( QRegExp( QLatin1String( "[0-9]+\\.[0-9]" ) ).exactMatch( versionString ) )
		version = versionString.replace( QLatin1String( "." ), QString::null ).toUInt();

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

	addGroup( Kopete::Group::topLevel() );

	QDomElement element = list.firstChild().toElement();
	while( !element.isNull() )
	{
		if( element.tagName() == QLatin1String("meta-contact") )
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
				Kopete::ContactList::self()->addMetaContact(
					metaContact );
			}
		}
		else if( element.tagName() == QLatin1String("kopete-group") )
		{
			Kopete::Group *group = new Kopete::Group();
			if( !group->fromXML( element ) )
			{
				delete group;
				group = 0;
			}
			else
			{
				Kopete::ContactList::self()->addGroup( group );
			}
		}
		// Only load myself metacontact information when Global Identity is enabled.
		else if( element.tagName() == QLatin1String("myself-meta-contact") && Kopete::GeneralSettings::self()->enableGlobalIdentity() )
		{
			if( !myself()->fromXML( element ) )
			{
				delete d->myself;
				d->myself = 0;
			}
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

void ContactList::convertContactList( const QString &fileName, uint /* fromVersion */, uint /* toVersion */ )
{
	// For now, ignore fromVersion and toVersion. These are meant for future
	// changes to allow incremental (multi-pass) conversion so we don't have
	// to rewrite the whole conversion code for each change.

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
						QString id   = oldContactElement.attribute( QLatin1String( "id" ), QString::null );
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
							QString id   = oldContactElement.attribute( QLatin1String( "plugin-id" ), QString::null );
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
							oldGroupElement.attribute( QLatin1String( "plugin-id" ), QString::null ) );
						groupPluginData.appendChild( newList.createTextNode( oldGroupElement.text() ) );
					}

					oldGroupNode = oldGroupNode.nextSibling();
				}
			}
			else
			{
				kWarning( 14010 ) << k_funcinfo << "Unknown element '" << oldElement.tagName()
					<< "' in contact list!" << endl;
			}
		}
		oldNode = oldNode.nextSibling();
	}

	// Close the file, and save the new file
	contactListFile.close();

	QDir().rename( fileName, fileName + QLatin1String( ".bak" ) );

	// kDebug( 14010 ) << k_funcinfo << "XML output:\n" << newList.toString( 2 ) << endl;

	contactListFile.open( QIODevice::WriteOnly );
	QTextStream stream( &contactListFile );
	stream.setCodec(QTextCodec::codecForName("UTF-8"));
	stream << newList.toString( 2 );

	contactListFile.flush();
	contactListFile.close();
}

void Kopete::ContactList::save()
{
	if( !d->loaded )
	{
		kDebug(14010) << "Contact list not loaded, abort saving" << endl;
		return;
	}

	Kopete::ContactListStorage *storage = new Kopete::XmlContactStorage();
	storage->save();
	if( !storage->isValid() )
	{
		kDebug(14010) << k_funcinfo << "Contact list storage failed. Reason: " << storage->errorMessage() << endl;

		// Saving the contact list failed. retry every minute until it works.
		// single-shot: will get restarted by us next time if it's still failing
		d->saveTimer->setSingleShot( true );
		d->saveTimer->start( 60000 );
		delete storage;
		return;
	}

	// cancel any scheduled saves
	d->saveTimer->stop();
	delete storage;
}

void Kopete::ContactList::saveXML()
{
	if(!d->loaded)
	{
		kDebug(14010) << "Kopete::ContactList::saveXML: contactlist not loaded, abort saving" << endl;
		return;
	}

	QString contactListFileName = KStandardDirs::locateLocal( "appdata", QLatin1String( "contactlist.xml" ) );
	KSaveFile contactListFile( contactListFileName );
	if( contactListFile.status() == 0 )
	{
		QTextStream *stream = contactListFile.textStream();
		stream->setCodec(QTextCodec::codecForName("UTF-8"));
		toXML().save( *stream, 4 );

		if ( contactListFile.close() )
		{
			// cancel any scheduled saves
			d->saveTimer->stop();
			return;
		}
		else
		{
			kDebug(14010) << "Kopete::ContactList::saveXML: failed to write contactlist, error code is: " << contactListFile.status() << endl;
		}
	}
	else
	{
		kWarning(14010) << "Kopete::ContactList::saveXML: Couldn't open contact list file "
			<< contactListFileName << ". Contact list not saved." << endl;
	}

	// if we got here, saving the contact list failed. retry every minute until it works.
	// single-shot: will get restarted by us next time if it's still failing
	d->saveTimer->setSingleShot( true );
	d->saveTimer->start( 60000 );
}

const QDomDocument ContactList::toXML()
{
	QDomDocument doc;
	doc.appendChild( doc.createElement( QLatin1String("kopete-contact-list") ) );
	doc.documentElement().setAttribute( QLatin1String("version"), QLatin1String("1.0"));

	// Save group information. ie: Open/Closed, pehaps later icons? Who knows.
	QListIterator<Group *> it(d->groups);
	while ( it.hasNext() )
		doc.documentElement().appendChild( doc.importNode( (it.next())->toXML(), true ) );

	// Save metacontact information.
	QListIterator<MetaContact *> mcit(d->contacts);
	while ( mcit.hasNext() )
	{
		MetaContact *mc = mcit.next();
		if( !mc->isTemporary() )
			doc.documentElement().appendChild( doc.importNode( mc->toXML(), true ) );
	}
	// Save myself metacontact information
	if( Kopete::GeneralSettings::self()->enableGlobalIdentity() )
	{
		QDomElement myselfElement = myself()->toXML(true); // Save minimal information.
		myselfElement.setTagName( QLatin1String("myself-meta-contact") );
		doc.documentElement().appendChild( doc.importNode( myselfElement, true ) );
	}

	return doc;
}

QStringList ContactList::contacts() const
{
	QStringList contacts;
	QListIterator<Kopete::MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		contacts.append( it.next()->displayName() );
	}
	return contacts;
}

QStringList ContactList::contactStatuses() const
{
	QStringList meta_contacts;
	QListIterator<Kopete::MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Kopete::MetaContact * mc = it.next();
		meta_contacts.append( QString::fromLatin1( "%1 (%2)" ).
			arg( mc->displayName(), mc->statusString() ));
	}
	return meta_contacts;
}

QStringList ContactList::reachableContacts() const
{
	QStringList contacts;
	QListIterator<Kopete::MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Kopete::MetaContact * mc = it.next();
		if ( mc->isReachable() )
			contacts.append( mc->displayName() );
	}
	return contacts;
}

QList<Contact *> ContactList::onlineContacts() const
{
	QList<Kopete::Contact *> result;
	QListIterator<Kopete::MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Kopete::MetaContact *mc = it.next();
		if ( mc->isOnline() )
		{
			QList<Kopete::Contact *> contacts = mc->contacts();
			QListIterator<Kopete::Contact *> cit( contacts );
			while ( cit.hasNext() )
			{
				Kopete::Contact *c = cit.next();
				if ( c->isOnline() )
					result.append( c );
			}
		}
	}
	return result;
}

QList<Kopete::MetaContact *> Kopete::ContactList::onlineMetaContacts() const
{
	QList<Kopete::MetaContact *> result;
	QListIterator<Kopete::MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Kopete::MetaContact * mc = it.next();
		if ( mc->isOnline() )
			result.append( mc );
	}
	return result;
}

QList<Kopete::MetaContact *> Kopete::ContactList::onlineMetaContacts( const QString &protocolId ) const
{
	QList<Kopete::MetaContact *> result;
	QListIterator<Kopete::MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Kopete::MetaContact * mc = it.next();
		// FIXME: This loop is not very efficient :(
		if ( mc->isOnline() )
		{
			QList<Kopete::Contact *> contacts = mc->contacts();
			QListIterator<Kopete::Contact *> cit( contacts );
			while ( cit.hasNext() )
			{
				Kopete::Contact *c = cit.next();
				if( c->isOnline() && c->protocol()->pluginId() == protocolId )
					result.append( mc );
			}
		}
	}
	return result;
}

QList<Kopete::Contact *> Kopete::ContactList::onlineContacts( const QString &protocolId ) const
{
	QList<Kopete::Contact *> result;
	QListIterator<Kopete::MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Kopete::MetaContact * mc = it.next();
		// FIXME: This loop is not very efficient :(
		if ( mc->isOnline() )
		{
			QList<Kopete::Contact *> contacts = mc->contacts();
			QListIterator<Kopete::Contact *> cit( contacts );
			while ( cit.hasNext() )
			{
				Kopete::Contact *c = cit.next();
				if( c->isOnline() && c->protocol()->pluginId() == protocolId )
					result.append( c );
			}
		}
	}
	return result;
}

QStringList Kopete::ContactList::fileTransferContacts() const
{
	QStringList contacts;
	QList<Kopete::MetaContact *> result;
	QListIterator<Kopete::MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Kopete::MetaContact * mc = it.next();
		if ( mc->canAcceptFiles() )
			contacts.append( mc->displayName() );
	}
	return contacts;
}

void Kopete::ContactList::sendFile( const QString &displayName, const KUrl &sourceURL,
	const QString &altFileName, const long unsigned int fileSize)
{
//	kDebug(14010) << "Send To Display Name: " << displayName << "\n";

	Kopete::MetaContact *c = findMetaContactByDisplayName( displayName );
	if( c )
		c->sendFile( sourceURL, altFileName, fileSize );
}

void Kopete::ContactList::messageContact( const QString &contactId, const QString &messageText )
{
	Kopete::MetaContact *mc = findMetaContactByContactId( contactId );
	if (!mc) return;

	Kopete::Contact *c = mc->execute(); //We need to know which contact was chosen as the preferred in order to message it
	if (!c) return;

	Kopete::Message msg(c->account()->myself(), c, messageText, Kopete::Message::Outbound);
	c->manager(Contact::CanCreate)->sendMessage(msg);

}


QStringList Kopete::ContactList::contactFileProtocols(const QString &displayName)
{
//	kDebug(14010) << "Get contacts for: " << displayName << "\n";
	QStringList protocols;

	Kopete::MetaContact *c = findMetaContactByDisplayName( displayName );
	if( c )
	{
		QList<Kopete::Contact *> mContacts = c->contacts();
		kDebug(14010) << mContacts.count() << endl;
		QListIterator<Kopete::Contact *> jt( mContacts );
		while ( jt.hasNext() )
		{
			Kopete::Contact *c = jt.next();
			kDebug(14010) << "1" << c->protocol()->pluginId() << endl;
			if( c->canAcceptFiles() ) {
				kDebug(14010) << c->protocol()->pluginId() << endl;
				protocols.append ( c->protocol()->pluginId() );
			}
		}
		return protocols;
	}
	return QStringList();
}


void ContactList::slotSaveLater()
{
	// if we already have a save scheduled, it will be cancelled. either way,
	// start a timer to save the contact list a bit later.
	d->saveTimer->start( 17100 /* 17,1 seconds */ );
}

void ContactList::slotKABCChanged()
{
	// TODO: react to changes in KABC, replacing this function, post 3.4 (Will)
	// call syncWithKABC on each metacontact to check if its associated kabc entry has changed.
/*	for ( MetaContact * mc = d->contacts.first(); mc; mc = d->contacts.next() )

		mc->syncWithKABC();*/
}


} //END namespace Kopete

#include "kopetecontactlist.moc"

// vim: set noet ts=4 sts=4 sw=4:

