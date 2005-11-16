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
#include <Q3PtrList>

#include <kapplication.h>
#include <kabc/stdaddressbook.h>
#include <kdebug.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <kopeteconfig.h>
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
	: QObject( kapp, "KopeteContactList" )
{
	d=new Private;

	//the myself metacontact can't be created now, because it will use
	//ContactList::self() as parent which will call this constructor -> infinite loop
	d->myself=0L;

	//no contactlist loaded yet, don't save them
	d->loaded=false;

	// automatically save on changes to the list
	d->saveTimer = new QTimer( this, "saveTimer" );
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
		kdDebug( 14010 ) << k_funcinfo << "Account not found" << endl;
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
//		kdDebug(14010) << "Display Name: " << it.current()->displayName() << "\n";
		if( mc->displayName() == displayName ) {
			return mc;
		}
	}

	return 0L;
}

MetaContact* ContactList::findMetaContactByContactId( const QString &contactId ) const
{
	Q3PtrList<Account> acts=AccountManager::self()->accounts();
	Q3PtrListIterator<Account> it( acts );
	for ( ; it.current(); ++it )
	{
		Contact *c=(*it)->contacts()[contactId];
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
		kdDebug(14010) << k_funcinfo << "Trying to remove a not listed MetaContact." << endl;
		return;
	}

	if ( d->selectedMetaContacts.contains( m ) )
	{
		d->selectedMetaContacts.remove( m );
		setSelectedItems( d->selectedMetaContacts, d->selectedGroups );
	}

	//removes subcontact from server here and now.
	QList<Contact *> cts = m->contacts();
	QListIterator<Contact *> it(cts);
	while( it.hasNext() )
	{
		it.next()->deleteContact();
	}

	d->contacts.remove( m );
	emit metaContactRemoved( m );
	m->deleteLater();
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
		d->selectedGroups.remove( g );
		setSelectedItems( d->selectedMetaContacts, d->selectedGroups );
	}

	d->groups.remove( g );
	emit groupRemoved( g );
	g->deleteLater();
}


void ContactList::setSelectedItems(QList<MetaContact *> metaContacts , QList<Group *> groups)
{
	kdDebug( 14010 ) << k_funcinfo << metaContacts.count() << " metacontacts, " << groups.count() << " groups selected" << endl;
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
	bool useGlobalIdentity;

	useGlobalIdentity = Kopete::Config::enableGlobalIdentity();

 	// Apply the global identity
	if(useGlobalIdentity)
 	{
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
}

void ContactList::slotDisplayNameChanged()
{
	kdDebug( 14010 ) << k_funcinfo << myself()->displayName() << endl;

	emit globalIdentityChanged(Kopete::Global::Properties::self()->nickName().key(), myself()->displayName());
}

void ContactList::slotPhotoChanged()
{
	QString photoURL;

	MetaContact::PropertySource photoSource = myself()->photoSource();

	// Save the image to ~./kde/share/apps/kopete/global-photo.png if the source is not custom.
	if(photoSource != MetaContact::SourceCustom)
	{
		QImage globalPhoto = myself()->photo();

		photoURL = "global-photo.png";
		photoURL = locateLocal("appdata", photoURL);

		if(!globalPhoto.save(photoURL, "PNG"))
		{
				kdDebug( 14010 ) << k_funcinfo << "Error while saving the global photo to file." << endl;
				return;
		}
	}
	else
	{
		photoURL = myself()->customPhoto().path();
	}

	kdDebug( 14010 ) << k_funcinfo << photoURL << endl;

	emit globalIdentityChanged(Kopete::Global::Properties::self()->photo().key(), photoURL);
}

///////////////////////////////////////////////////////////////////////////////////////////////
void ContactList::load()
{
	loadXML();
	// Apply the global identity when all the protocols plugins are loaded.
	connect(PluginManager::self(), SIGNAL(allPluginsLoaded()), this, SLOT(loadGlobalIdentity()));
}

void ContactList::loadXML()
{
	// don't save when we're in the middle of this...
	d->loaded = false;

	QString filename = locateLocal( "appdata", QString::fromLatin1( "contactlist.xml" ) );
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

	QString versionString = list.attribute( QString::fromLatin1( "version" ), QString::null );
	uint version = 0;
	if( QRegExp( QString::fromLatin1( "[0-9]+\\.[0-9]" ) ).exactMatch( versionString ) )
		version = versionString.replace( QString::fromLatin1( "." ), QString::null ).toUInt();

	if( version < Private::ContactListVersion )
	{
		// The version string is invalid, or we're using an older version.
		// Convert first and reparse the file afterwards
		kdDebug( 14010 ) << k_funcinfo << "Contact list version " << version
			<< " is older than current version " <<  Private::ContactListVersion
			<< ". Converting first." << endl;

		contactListFile.close();

		convertContactList( filename, version,  Private::ContactListVersion );

		contactList = QDomDocument ( QString::fromLatin1( "kopete-contact-list" ) );

		contactListFile.open( QIODevice::ReadOnly );
		contactList.setContent( &contactListFile );

		list = contactList.documentElement();
	}

	addGroup( Kopete::Group::topLevel() );

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
				Kopete::ContactList::self()->addMetaContact(
					metaContact );
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
				Kopete::ContactList::self()->addGroup( group );
			}
		}
		else if( element.tagName() == QString::fromLatin1("myself-meta-contact") )
		{
			if( !myself()->fromXML( element ) )
			{
				delete d->myself;
				d->myself = 0;
			}
		}
		else
		{
			kdWarning(14010) << "Kopete::ContactList::loadXML: "
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

	QDomDocument contactList( QString::fromLatin1( "messaging-contact-list" ) );
	QFile contactListFile( fileName );
	contactListFile.open( QIODevice::ReadOnly );
	contactList.setContent( &contactListFile );

	QDomElement oldList = contactList.documentElement();

	QDomDocument newList( QString::fromLatin1( "kopete-contact-list" ) );
	newList.appendChild( newList.createProcessingInstruction( QString::fromLatin1( "xml" ), QString::fromLatin1( "version=\"1.0\"" ) ) );

	QDomElement newRoot = newList.createElement( QString::fromLatin1( "kopete-contact-list" ) );
	newList.appendChild( newRoot );
	newRoot.setAttribute( QString::fromLatin1( "version" ), QString::fromLatin1( "1.0" ) );

	QDomNode oldNode = oldList.firstChild();
	while( !oldNode.isNull() )
	{
		QDomElement oldElement = oldNode.toElement();
		if( !oldElement.isNull() )
		{
			if( oldElement.tagName() == QString::fromLatin1("meta-contact") )
			{
				// Ignore ID, it is not used in the current list anyway
				QDomElement newMetaContact = newList.createElement( QString::fromLatin1( "meta-contact" ) );
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
					if( !oldContactElement.isNull() && oldContactElement.tagName() == QString::fromLatin1("address-book-field") )
					{
						// Convert address book fields.
						// Jabber will be called "xmpp", Aim/Toc and Aim/Oscar both will
						// be called "aim". MSN, AIM, IRC, Oscar and SMS don't use address
						// book fields yet; Gadu and ICQ can be converted as-is.
						// As Yahoo is unfinished we won't try to convert at all.
						QString id   = oldContactElement.attribute( QString::fromLatin1( "id" ), QString::null );
						QString data = oldContactElement.text();

						QString app, key, val;
						QString separator = QString::fromLatin1( "," );
						if( id == QString::fromLatin1( "messaging/gadu" ) )
							separator = QString::fromLatin1( "\n" );
						else if( id == QString::fromLatin1( "messaging/icq" ) )
							separator = QString::fromLatin1( ";" );
						else if( id == QString::fromLatin1( "messaging/jabber" ) )
							id = QString::fromLatin1( "messaging/xmpp" );

						if( id == QString::fromLatin1( "messaging/gadu" ) || id == QString::fromLatin1( "messaging/icq" ) ||
							id == QString::fromLatin1( "messaging/winpopup" ) || id == QString::fromLatin1( "messaging/xmpp" ) )
						{
							app = id;
							key = QString::fromLatin1( "All" );
							val = data.replace( separator, QString( QChar( 0xE000 ) ) );
						}

						if( !app.isEmpty() )
						{
							QDomElement addressBookField = newList.createElement( QString::fromLatin1( "address-book-field" ) );
							newMetaContact.appendChild( addressBookField );

							addressBookField.setAttribute( QString::fromLatin1( "app" ), app );
							addressBookField.setAttribute( QString::fromLatin1( "key" ), key );

							addressBookField.appendChild( newList.createTextNode( val ) );

							// ICQ didn't store the contactId locally, only in the address
							// book fields, so we need to be able to access it later
							if( id == QString::fromLatin1( "messaging/icq" ) )
								icqData = QStringList::split( QChar( 0xE000 ), val );
							else if( id == QString::fromLatin1("messaging/gadu") )
								gaduData = QStringList::split( QChar( 0xE000 ), val );
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
						if( oldContactElement.tagName() == QString::fromLatin1("display-name") )
						{
							QDomElement displayName = newList.createElement( QString::fromLatin1( "display-name" ) );
							displayName.appendChild( newList.createTextNode( oldContactElement.text() ) );
							newMetaContact.appendChild( displayName );
						}
						else if( oldContactElement.tagName() == QString::fromLatin1("groups") )
						{
							QDomElement groups = newList.createElement( QString::fromLatin1( "groups" ) );
							newMetaContact.appendChild( groups );

							QDomNode oldGroup = oldContactElement.firstChild();
							while( !oldGroup.isNull() )
							{
								QDomElement oldGroupElement = oldGroup.toElement();
								if ( oldGroupElement.tagName() == QString::fromLatin1("group") )
								{
									QDomElement group = newList.createElement( QString::fromLatin1( "group" ) );
									group.appendChild( newList.createTextNode( oldGroupElement.text() ) );
									groups.appendChild( group );
								}
								else if ( oldGroupElement.tagName() == QString::fromLatin1("top-level") )
								{
									QDomElement group = newList.createElement( QString::fromLatin1( "top-level" ) );
									groups.appendChild( group );
								}

								oldGroup = oldGroup.nextSibling();
							}
						}
						else if( oldContactElement.tagName() == QString::fromLatin1( "plugin-data" ) )
						{
							// Convert the plugin data
							QString id   = oldContactElement.attribute( QString::fromLatin1( "plugin-id" ), QString::null );
							QString data = oldContactElement.text();

							bool convertOldAim = false;
							uint fieldCount = 1;
							QString addressBookLabel;
							if( id == QString::fromLatin1("MSNProtocol") )
							{
								fieldCount = 3;
								addressBookLabel = QString::fromLatin1("msn");
							}
							else if( id == QString::fromLatin1("IRCProtocol") )
							{
								fieldCount = 3;
								addressBookLabel = QString::fromLatin1("irc");
							}
							else if( id == QString::fromLatin1("OscarProtocol") )
							{
								fieldCount = 2;
								addressBookLabel = QString::fromLatin1("aim");
							}
							else if( id == QString::fromLatin1("AIMProtocol") )
							{
								id = QString::fromLatin1("OscarProtocol");
								convertOldAim = true;
								addressBookLabel = QString::fromLatin1("aim");
							}
							else if( id == QString::fromLatin1("ICQProtocol") || id == QString::fromLatin1("WPProtocol") || id == QString::fromLatin1("GaduProtocol") )
							{
								fieldCount = 1;
							}
							else if( id == QString::fromLatin1("JabberProtocol") )
							{
								fieldCount = 4;
							}
							else if( id == QString::fromLatin1("SMSProtocol") )
							{
								// SMS used a variable serializing using a dot as delimiter.
								// The minimal count is three though (id, name, delimiter).
								fieldCount = 2;
								addressBookLabel = QString::fromLatin1("sms");
							}

							if( pluginData[ id ].isNull() )
							{
								pluginData[ id ] = newList.createElement( QString::fromLatin1( "plugin-data" ) );
								pluginData[ id ].setAttribute( QString::fromLatin1( "plugin-id" ), id );
								newMetaContact.appendChild( pluginData[ id ] );
							}

							// Do the actual conversion
							if( id == QString::fromLatin1( "MSNProtocol" ) || id == QString::fromLatin1( "OscarProtocol" ) ||
								id == QString::fromLatin1( "AIMProtocol" ) || id == QString::fromLatin1( "IRCProtocol" ) ||
								id == QString::fromLatin1( "ICQProtocol" ) || id == QString::fromLatin1( "JabberProtocol" ) ||
								id == QString::fromLatin1( "SMSProtocol" ) || id == QString::fromLatin1( "WPProtocol" ) ||
								id == QString::fromLatin1( "GaduProtocol" ) )
							{
								QStringList strList = QStringList::split( QString::fromLatin1( "||" ), data );

								// Unescape '||'
								for( QStringList::iterator it = strList.begin(); it != strList.end(); ++it )
								{
									( *it ).replace( QString::fromLatin1( "\\|;" ), QString::fromLatin1( "|" ) ).
									replace( QString::fromLatin1( "\\\\" ), QString::fromLatin1( "\\" ) );
								}

								uint idx = 0;
								while( idx < (unsigned int)strList.size() )
								{
									QDomElement dataField;

									dataField = newList.createElement( QString::fromLatin1( "plugin-data-field" ) );
									pluginData[ id ].appendChild( dataField );
									dataField.setAttribute( QString::fromLatin1( "key" ), QString::fromLatin1( "contactId" ) );
									if( id == QString::fromLatin1("ICQProtocol") )
										dataField.appendChild( newList.createTextNode( icqData[ idx ] ) );
									else if( id == QString::fromLatin1("GaduProtocol") )
										dataField.appendChild( newList.createTextNode( gaduData[ idx ] ) );
									else if( id == QString::fromLatin1("JabberProtocol") )
										dataField.appendChild( newList.createTextNode( strList[ idx + 1 ] ) );
									else
										dataField.appendChild( newList.createTextNode( strList[ idx ] ) );

									dataField = newList.createElement( QString::fromLatin1( "plugin-data-field" ) );
									pluginData[ id ].appendChild( dataField );
									dataField.setAttribute( QString::fromLatin1( "key" ), QString::fromLatin1( "displayName" ) );
									if( convertOldAim || id == QString::fromLatin1("ICQProtocol") || id == QString::fromLatin1("WPProtocol") || id == QString::fromLatin1("GaduProtocol") )
										dataField.appendChild( newList.createTextNode( strList[ idx ] ) );
									else if( id == QString::fromLatin1("JabberProtocol") )
										dataField.appendChild( newList.createTextNode( strList[ idx + 2 ] ) );
									else
										dataField.appendChild( newList.createTextNode( strList[ idx + 1 ] ) );

									if( id == QString::fromLatin1("MSNProtocol") )
									{
										dataField = newList.createElement( QString::fromLatin1( "plugin-data-field" ) );
										pluginData[ id ].appendChild( dataField );
										dataField.setAttribute( QString::fromLatin1( "key" ), QString::fromLatin1( "groups" ) );
										dataField.appendChild( newList.createTextNode( strList[ idx + 2 ] ) );
									}
									else if( id == QString::fromLatin1("IRCProtocol") )
									{
										dataField = newList.createElement( QString::fromLatin1( "plugin-data-field" ) );
										pluginData[ id ].appendChild( dataField );
										dataField.setAttribute( QString::fromLatin1( "key" ), QString::fromLatin1( "serverName" ) );
										dataField.appendChild( newList.createTextNode( strList[ idx + 2 ] ) );
									}
									else if( id == QString::fromLatin1("JabberProtocol") )
									{
										dataField = newList.createElement( QString::fromLatin1( "plugin-data-field" ) );
										pluginData[ id ].appendChild( dataField );
										dataField.setAttribute( QString::fromLatin1( "key" ), QString::fromLatin1( "accountId" ) );
										dataField.appendChild( newList.createTextNode( strList[ idx ] ) );

										dataField = newList.createElement( QString::fromLatin1( "plugin-data-field" ) );
										pluginData[ id ].appendChild( dataField );
										dataField.setAttribute( QString::fromLatin1( "key" ), QString::fromLatin1( "groups" ) );
										dataField.appendChild( newList.createTextNode( strList[ idx + 3 ] ) );
									}
									else if( id == QString::fromLatin1( "SMSProtocol" ) &&
										( idx + 2 < (uint)strList.size() ) && strList[ idx + 2 ] != QString::fromLatin1( "." ) )
									{
										dataField = newList.createElement( QString::fromLatin1( "plugin-data-field" ) );
										pluginData[ id ].appendChild( dataField );
										dataField.setAttribute( QString::fromLatin1( "key" ), QString::fromLatin1( "serviceName" ) );
										dataField.appendChild( newList.createTextNode( strList[ idx + 2 ] ) );

										dataField = newList.createElement( QString::fromLatin1( "plugin-data-field" ) );
										pluginData[ id ].appendChild( dataField );
										dataField.setAttribute( QString::fromLatin1( "key" ), QString::fromLatin1( "servicePrefs" ) );
										dataField.appendChild( newList.createTextNode( strList[ idx + 3 ] ) );

										// Add extra fields
										idx += 2;
									}

									// MSN, AIM, IRC, Oscar and SMS didn't store address book fields up
									// to now, so create one
									if( id != QString::fromLatin1("ICQProtocol") && id != QString::fromLatin1("JabberProtocol") && id != QString::fromLatin1("WPProtocol") && id != QString::fromLatin1("GaduProtocol") )
									{
										QDomElement addressBookField = newList.createElement( QString::fromLatin1( "address-book-field" ) );
										newMetaContact.appendChild( addressBookField );

										addressBookField.setAttribute( QString::fromLatin1( "app" ),
											QString::fromLatin1( "messaging/" ) + addressBookLabel );
										addressBookField.setAttribute( QString::fromLatin1( "key" ), QString::fromLatin1( "All" ) );
										addressBookField.appendChild( newList.createTextNode( strList[ idx ] ) );
									}

									idx += fieldCount;
								}
							}
							else if( id == QString::fromLatin1("ContactNotesPlugin") || id == QString::fromLatin1("CryptographyPlugin") || id == QString::fromLatin1("TranslatorPlugin") )
							{
								QDomElement dataField = newList.createElement( QString::fromLatin1( "plugin-data-field" ) );
								pluginData[ id ].appendChild( dataField );
								if( id == QString::fromLatin1("ContactNotesPlugin") )
									dataField.setAttribute( QString::fromLatin1( "key" ), QString::fromLatin1( "notes" ) );
								else if( id == QString::fromLatin1("CryptographyPlugin") )
									dataField.setAttribute( QString::fromLatin1( "key" ), QString::fromLatin1( "gpgKey" ) );
								else if( id == QString::fromLatin1("TranslatorPlugin") )
									dataField.setAttribute( QString::fromLatin1( "key" ), QString::fromLatin1( "languageKey" ) );

								dataField.appendChild( newList.createTextNode( data ) );
							}
						}
					}
					oldContactNode = oldContactNode.nextSibling();
				}
			}
			else if( oldElement.tagName() == QString::fromLatin1("kopete-group") )
			{
				QDomElement newGroup = newList.createElement( QString::fromLatin1( "kopete-group" ) );
				newRoot.appendChild( newGroup );

				QDomNode oldGroupNode = oldNode.firstChild();
				while( !oldGroupNode.isNull() )
				{
					QDomElement oldGroupElement = oldGroupNode.toElement();

					if( oldGroupElement.tagName() == QString::fromLatin1("display-name") )
					{
						QDomElement displayName = newList.createElement( QString::fromLatin1( "display-name" ) );
						displayName.appendChild( newList.createTextNode( oldGroupElement.text() ) );
						newGroup.appendChild( displayName );
					}
					if( oldGroupElement.tagName() == QString::fromLatin1("type") )
					{
						if( oldGroupElement.text() == QString::fromLatin1("Temporary") )
							newGroup.setAttribute( QString::fromLatin1( "type" ), QString::fromLatin1( "temporary" ) );
						else if( oldGroupElement.text() == QString::fromLatin1( "TopLevel" ) )
							newGroup.setAttribute( QString::fromLatin1( "type" ), QString::fromLatin1( "top-level" ) );
						else
							newGroup.setAttribute( QString::fromLatin1( "type" ), QString::fromLatin1( "standard" ) );
					}
					if( oldGroupElement.tagName() == QString::fromLatin1("view") )
					{
						if( oldGroupElement.text() == QString::fromLatin1("collapsed") )
							newGroup.setAttribute( QString::fromLatin1( "view" ), QString::fromLatin1( "collapsed" ) );
						else
							newGroup.setAttribute( QString::fromLatin1( "view" ), QString::fromLatin1( "expanded" ) );
					}
					else if( oldGroupElement.tagName() == QString::fromLatin1("plugin-data") )
					{
						// Per-group plugin data
						// FIXME: This needs updating too, ideally, convert this in a later
						//        contactlist.xml version
						QDomElement groupPluginData = newList.createElement( QString::fromLatin1( "plugin-data" ) );
						newGroup.appendChild( groupPluginData );

						groupPluginData.setAttribute( QString::fromLatin1( "plugin-id" ),
							oldGroupElement.attribute( QString::fromLatin1( "plugin-id" ), QString::null ) );
						groupPluginData.appendChild( newList.createTextNode( oldGroupElement.text() ) );
					}

					oldGroupNode = oldGroupNode.nextSibling();
				}
			}
			else
			{
				kdWarning( 14010 ) << k_funcinfo << "Unknown element '" << oldElement.tagName()
					<< "' in contact list!" << endl;
			}
		}
		oldNode = oldNode.nextSibling();
	}

	// Close the file, and save the new file
	contactListFile.close();

	QDir().rename( fileName, fileName + QString::fromLatin1( ".bak" ) );

	// kdDebug( 14010 ) << k_funcinfo << "XML output:\n" << newList.toString( 2 ) << endl;

	contactListFile.open( QIODevice::WriteOnly );
	QTextStream stream( &contactListFile );
	stream.setEncoding( QTextStream::UnicodeUTF8 );
	stream << newList.toString( 2 );

	contactListFile.flush();
	contactListFile.close();
}

void Kopete::ContactList::save()
{
	saveXML();
}

void Kopete::ContactList::saveXML()
{
	if(!d->loaded)
	{
		kdDebug(14010) << "Kopete::ContactList::saveXML: contactlist not loaded, abort saving" << endl;
		return;
	}

	QString contactListFileName = locateLocal( "appdata", QString::fromLatin1( "contactlist.xml" ) );
	KSaveFile contactListFile( contactListFileName );
	if( contactListFile.status() == 0 )
	{
		QTextStream *stream = contactListFile.textStream();
		stream->setEncoding( QTextStream::UnicodeUTF8 );
		toXML().save( *stream, 4 );

		if ( contactListFile.close() )
		{
			// cancel any scheduled saves
			d->saveTimer->stop();
			return;
		}
		else
		{
			kdDebug(14010) << "Kopete::ContactList::saveXML: failed to write contactlist, error code is: " << contactListFile.status() << endl;
		}
	}
	else
	{
		kdWarning(14010) << "Kopete::ContactList::saveXML: Couldn't open contact list file "
			<< contactListFileName << ". Contact list not saved." << endl;
	}

	// if we got here, saving the contact list failed. retry every minute until it works.
	d->saveTimer->start( 60000, true /* single-shot: will get restarted by us next time if it's still failing */ );
}

const QDomDocument ContactList::toXML()
{
	QDomDocument doc;
	doc.appendChild( doc.createElement( QString::fromLatin1("kopete-contact-list") ) );
	doc.documentElement().setAttribute( QString::fromLatin1("version"), QString::fromLatin1("1.0"));

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
	QDomElement myselfElement = myself()->toXML(true); // Save minimal information.
	myselfElement.setTagName( QString::fromLatin1("myself-meta-contact") );
	doc.documentElement().appendChild( doc.importNode( myselfElement, true ) );

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

void Kopete::ContactList::sendFile( const QString &displayName, const KURL &sourceURL,
	const QString &altFileName, const long unsigned int fileSize)
{
//	kdDebug(14010) << "Send To Display Name: " << displayName << "\n";

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
//	kdDebug(14010) << "Get contacts for: " << displayName << "\n";
	QStringList protocols;

	Kopete::MetaContact *c = findMetaContactByDisplayName( displayName );
	if( c )
	{
		QList<Kopete::Contact *> mContacts = c->contacts();
		kdDebug(14010) << mContacts.count() << endl;
		QListIterator<Kopete::Contact *> jt( mContacts );
		while ( jt.hasNext() )
		{
			Kopete::Contact *c = jt.next();
			kdDebug(14010) << "1" << c->protocol()->pluginId() << endl;
			if( c->canAcceptFiles() ) {
				kdDebug(14010) << c->protocol()->pluginId() << endl;
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
	d->saveTimer->start( 17100 /* 17,1 seconds */, true /* single-shot */ );
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

