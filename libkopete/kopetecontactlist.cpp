/*
    kopetecontactlist.cpp - Kopete's Contact List backend

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@tiscalinet.be>
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

#include <kapplication.h>
#include <kabc/stdaddressbook.h>
#include <kdebug.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include "kopetemetacontact.h"
#include "kopetecontact.h"
//#include "kopetemessagemanager.h"
//#include "kopetemessage.h"
#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
//#include "kopetegroup.h"


namespace  Kopete
{

class ContactList::Private
{public:
	/** Flag:  do not save the contactlist until she is completely loaded */
	bool loaded ;

	QPtrList<MetaContact> contacts;
	QPtrList<Group> groups;
	QPtrList<MetaContact> selectedMetaContacts;
	QPtrList<Group> selectedGroups;

	QTimer *saveTimer;

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

	//no contactlist loaded yet, don't save them
	d->loaded=false;

	// automatically save on changes to the list
	d->saveTimer = new QTimer( this, "saveTimer" );
	connect( d->saveTimer, SIGNAL( timeout() ), SLOT ( save() ) );

	connect( this, SIGNAL( metaContactAdded( MetaContact * ) ), SLOT( slotSaveLater() ) );
	connect( this, SIGNAL( metaContactDeleted( MetaContact * ) ), SLOT( slotSaveLater() ) );
	connect( this, SIGNAL( groupAdded( Group * ) ), SLOT( slotSaveLater() ) );
	connect( this, SIGNAL( groupRemoved( Group * ) ), SLOT( slotSaveLater() ) );
	connect( this, SIGNAL( groupRenamed( Group *, const QString & ) ), SLOT( slotSaveLater() ) );

}

ContactList::~ContactList()
{
	delete d;
}


QPtrList<MetaContact> ContactList::metaContacts() const
{
	return d->contacts;
}


QPtrList<Group> ContactList::groups() const
{
	return d->groups;
}


MetaContact *ContactList::metaContact( const QString &metaContactId ) const
{
	QPtrListIterator<MetaContact> it( d->contacts );
	for( ; it.current(); ++it )
	{
		if( it.current()->metaContactId() == metaContactId )
			return it.current();
	}

	return 0L;
}



Group * ContactList::group(unsigned int groupId) const
{
	Group *groupIterator;
	for ( groupIterator = d->groups.first(); groupIterator; groupIterator = d->groups.next() ) 
	{
#if 0 //TODO
		if( groupIterator->groupId()==groupId )
			return groupIterator;
#endif
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
	QPtrListIterator<MetaContact> it( d->contacts );
	for( ; it.current(); ++it )
	{
//		kdDebug(14010) << "Display Name: " << it.current()->displayName() << "\n";
		if( it.current()->displayName() == displayName ) {
			return it.current();
		}
	}

	return 0L;
}

MetaContact* ContactList::findMetaContactByContactId( const QString &contactId ) const
{
	QPtrListIterator<MetaContact> it( d->contacts );
	//FIXME: This loop isn't very efficient
		// maybe it's more efficient to loop accounts and use the Account::Contact  QDict  -Olivier
	for ( ; it.current(); ++it )
	{
		QPtrList<Contact> cl = it.current()->contacts();
		QPtrListIterator<Contact> kcit ( cl );

		for ( ; kcit.current(); ++kcit )
		{
			if ( kcit.current()->contactId() == contactId )
				return kcit.current()->metaContact();
		}
	}
	return 0L;
}


Group * ContactList::findGroup(const QString& displayName, int type)
{
#if 0 //TODO
	if( type == Group::Temporary )
		return Group::temporary();

	Group *groupIterator;
	for ( groupIterator = d->groups.first(); groupIterator; groupIterator = d->groups.next() )
	{
		if( groupIterator->type() == type && groupIterator->displayName() == displayName )
			return groupIterator;
	}

	Group *newGroup = new Group( displayName, (Group::GroupType)type );
	addGroup( newGroup );
	return  newGroup;
#endif
}


QPtrList<MetaContact> ContactList::selectedMetaContacts() const
{
	return d->selectedMetaContacts;
}

QPtrList<Group> ContactList::selectedGroups() const
{
	return d->selectedGroups;
}


void ContactList::addMetaContact( MetaContact *mc )
{
	if ( d->contacts.contains( mc ) )
		return;

	d->contacts.append( mc );

	emit metaContactAdded( mc );
	connect( mc, SIGNAL( persistentDataChanged(  ) ), SLOT( slotSaveLater() ) );
	connect( mc, SIGNAL( addedToGroup( MetaContact *, Group * ) ), SIGNAL( metaContactAddedToGroup( MetaContact *, Group * ) ) );
	connect( mc, SIGNAL( removedFromGroup( MetaContact *, Group * ) ), SIGNAL( metaContactRemovedFromGroup( MetaContact *, Group * ) ) );
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
	QPtrList<Contact> cts=m->contacts();
	for( Contact *c = cts.first(); c; c = cts.next() )
	{
		c->deleteContact();
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
#if 0 // TODO
		connect( g , SIGNAL ( renamed(Group* , const QString & )) , this , SIGNAL ( groupRenamed(Group* , const QString & )) ) ;
#endif
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
#if 0 //TODO   (juste car group j'ai pas encore fait)
	g->deleteLater();
#endif
}


void ContactList::setSelectedItems(QPtrList<MetaContact> metaContacts , QPtrList<Group> groups)
{
	kdDebug( 14010 ) << k_funcinfo << metaContacts.count() << " metacontacts, " << groups.count() << " groups selected" << endl;
	d->selectedMetaContacts=metaContacts;
	d->selectedGroups=groups;

	emit metaContactSelected( groups.isEmpty() && metaContacts.count()==1 );
	emit selectionChanged();
}


#if 0
void ContactList::load()
{
	loadXML();
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
	contactListFile.open( IO_ReadOnly );
	contactList.setContent( &contactListFile );

	QDomElement list = contactList.documentElement();

	QString versionString = list.attribute( QString::fromLatin1( "version" ), QString::null );
	uint version = 0;
	if( QRegExp( QString::fromLatin1( "[0-9]+\\.[0-9]" ) ).exactMatch( versionString ) )
		version = versionString.replace( QString::fromLatin1( "." ), QString::null ).toUInt();

	if( version < KopeteContactListPrivate::ContactListVersion )
	{
		// The version string is invalid, or we're using an older version.
		// Convert first and reparse the file afterwards
		kdDebug( 14010 ) << k_funcinfo << "Contact list version " << version
			<< " is older than current version " <<  KopeteContactListPrivate::ContactListVersion
			<< ". Converting first." << endl;

		contactListFile.close();

		convertContactList( filename, version,  KopeteContactListPrivate::ContactListVersion );

		contactList = QDomDocument ( QString::fromLatin1( "kopete-contact-list" ) );

		contactListFile.open( IO_ReadOnly );
		contactList.setContent( &contactListFile );

		list = contactList.documentElement();
	}

	addGroup( KopeteGroup::topLevel() );

	QDomElement element = list.firstChild().toElement();
	while( !element.isNull() )
	{
		if( element.tagName() == QString::fromLatin1("meta-contact") )
		{
			//TODO: id isn't used
			//QString id = element.attribute( "id", QString::null );
			KopeteMetaContact *metaContact = new KopeteMetaContact();
			if ( !metaContact->fromXML( element ) )
			{
				delete metaContact;
				metaContact = 0;
			}
			else
			{
				KopeteContactList::contactList()->addMetaContact(
					metaContact );
			}
		}
		else if( element.tagName() == QString::fromLatin1("kopete-group") )
		{
			KopeteGroup *group = new KopeteGroup();
			if( !group->fromXML( element ) )
			{
				delete group;
				group = 0;
			}
			else
			{
				KopeteContactList::contactList()->addGroup( group );
			}
		}
		else
		{
			kdWarning(14010) << "KopeteContactList::loadXML: "
				<< "Unknown element '" << element.tagName()
				<< "' in contact list!" << endl;
		}
		element = element.nextSibling().toElement();
	}
	contactListFile.close();
	d->loaded=true;
}

void KopeteContactList::convertContactList( const QString &fileName, uint /* fromVersion */, uint /* toVersion */ )
{
	// For now, ignore fromVersion and toVersion. These are meant for future
	// changes to allow incremental (multi-pass) conversion so we don't have
	// to rewrite the whole conversion code for each change.

	QDomDocument contactList( QString::fromLatin1( "messaging-contact-list" ) );
	QFile contactListFile( fileName );
	contactListFile.open( IO_ReadOnly );
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
							val = data.replace( separator, QChar( 0xE000 ) );
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
								addressBookLabel = "msn";
							}
							else if( id == QString::fromLatin1("IRCProtocol") )
							{
								fieldCount = 3;
								addressBookLabel = "irc";
							}
							else if( id == QString::fromLatin1("OscarProtocol") )
							{
								fieldCount = 2;
								addressBookLabel = "aim";
							}
							else if( id == QString::fromLatin1("AIMProtocol") )
							{
								id = QString::fromLatin1("OscarProtocol");
								convertOldAim = true;
								addressBookLabel = "aim";
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
								while( idx < strList.size() )
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
										( idx + 2 < strList.size() ) && strList[ idx + 2 ] != QString::fromLatin1( "." ) )
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

	contactListFile.open( IO_WriteOnly );
	QTextStream stream( &contactListFile );
	stream.setEncoding( QTextStream::UnicodeUTF8 );
	stream << newList.toString( 2 );

	contactListFile.flush();
	contactListFile.close();
}

void KopeteContactList::save()
{
	saveXML();
}

void KopeteContactList::saveXML()
{
	if(!d->loaded)
	{
		kdDebug(14010) << "KopeteContactList::saveXML: contactlist not loaded, abort saving" << endl;
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
			kdDebug(14010) << "KopeteContactList::saveXML: failed to write contactlist, error code is: " << contactListFile.status() << endl;
		}
	}
	else
	{
		kdWarning(14010) << "KopeteContactList::saveXML: Couldn't open contact list file "
			<< contactListFileName << ". Contact list not saved." << endl;
	}

	// if we got here, saving the contact list failed. retry every minute until it works.
	d->saveTimer->start( 60000, true /* single-shot: will get restarted by us next time if it's still failing */ );
}

const QDomDocument KopeteContactList::toXML()
{
	QDomDocument doc;
	doc.appendChild( doc.createElement( QString::fromLatin1("kopete-contact-list") ) );
	doc.documentElement().setAttribute( QString::fromLatin1("version"), QString::fromLatin1("1.0"));

	// Save group information. ie: Open/Closed, pehaps later icons? Who knows.
	for( KopeteGroup *g = d->groups.first(); g; g = d->groups.next() )
		doc.documentElement().appendChild( doc.importNode( g->toXML(), true ) );

	// Save metacontact information.
	for( KopeteMetaContact *m = d->contacts.first(); m; m = d->contacts.next() )
		if( !m->isTemporary() )
			doc.documentElement().appendChild( doc.importNode( m->toXML(), true ) );

	return doc;
}

QStringList KopeteContactList::contacts() const
{
	QStringList contacts;
	QPtrListIterator<KopeteMetaContact> it( d->contacts );
	for( ; it.current(); ++it )
	{
		contacts.append( it.current()->displayName() );
	}
	return contacts;
}

QStringList KopeteContactList::contactStatuses() const
{
	QStringList meta_contacts;
	QPtrListIterator<KopeteMetaContact> it( d->contacts );
	for( ; it.current(); ++it )
	{
		meta_contacts.append( QString::fromLatin1( "%1 (%2)" ).
			arg( it.current()->displayName(), it.current()->statusString() ));
	}
	return meta_contacts;
}

QStringList KopeteContactList::reachableContacts() const
{
	QStringList contacts;
	QPtrListIterator<KopeteMetaContact> it( d->contacts );
	for( ; it.current(); ++it )
	{
		if ( it.current()->isReachable() )
			contacts.append( it.current()->displayName() );
	}
	return contacts;
}

QPtrList<KopeteContact> KopeteContactList::onlineContacts() const
{
	QPtrList<KopeteContact> result;
	QPtrListIterator<KopeteMetaContact> it( d->contacts );
	for( ; it.current(); ++it )
	{
		if ( it.current()->isOnline() )
		{
			QPtrList<KopeteContact> contacts = it.current()->contacts();
			QPtrListIterator<KopeteContact> cit( contacts );
			for( ; cit.current(); ++cit )
			{
				if ( cit.current()->isOnline() )
					result.append( cit.current() );
			}
		}
	}
	return result;
}

QPtrList<KopeteMetaContact> KopeteContactList::onlineMetaContacts() const
{
	QPtrList<KopeteMetaContact> result;
	QPtrListIterator<KopeteMetaContact> it( d->contacts );
	for( ; it.current(); ++it )
	{
		if ( it.current()->isOnline() )
			result.append( it.current() );
	}
	return result;
}

QPtrList<KopeteMetaContact> KopeteContactList::onlineMetaContacts( const QString &protocolId ) const
{
	QPtrList<KopeteMetaContact> result;
	QPtrListIterator<KopeteMetaContact> it( d->contacts );
	for( ; it.current(); ++it )
	{
		// FIXME: This loop is not very efficient :(
		if ( it.current()->isOnline() )
		{
			QPtrList<KopeteContact> contacts = it.current()->contacts();
			QPtrListIterator<KopeteContact> cit( contacts );
			for( ; cit.current(); ++cit )
			{
				if( cit.current()->isOnline() && cit.current()->protocol()->pluginId() == protocolId )
					result.append( it.current() );
			}
		}
	}
	return result;
}

QPtrList<KopeteContact> KopeteContactList::onlineContacts( const QString &protocolId ) const
{
	QPtrList<KopeteContact> result;
	QPtrListIterator<KopeteMetaContact> it( d->contacts );
	for( ; it.current(); ++it )
	{
		// FIXME: This loop is not very efficient :(
		if ( it.current()->isOnline() )
		{
			QPtrList<KopeteContact> contacts = it.current()->contacts();
			QPtrListIterator<KopeteContact> cit( contacts );
			for( ; cit.current(); ++cit )
			{
				if( cit.current()->isOnline() && cit.current()->protocol()->pluginId() == protocolId )
					result.append( cit.current() );
			}
		}
	}
	return result;
}

QStringList KopeteContactList::fileTransferContacts() const
{
	QStringList contacts;
	QPtrListIterator<KopeteMetaContact> it( d->contacts );
	for( ; it.current(); ++it )
	{
		if ( it.current()->canAcceptFiles() )
			contacts.append( it.current()->displayName() );
	}
	return contacts;
}

void KopeteContactList::sendFile( const QString &displayName, const KURL &sourceURL,
	const QString &altFileName, const long unsigned int fileSize)
{
//	kdDebug(14010) << "Send To Display Name: " << displayName << "\n";

	KopeteMetaContact *c = findContactByDisplayName( displayName );
	if( c )
		c->sendFile( sourceURL, altFileName, fileSize );
}

void KopeteContactList::messageContact( const QString &contactId, const QString &messageText )
{
	KopeteMetaContact *mc = findMetaContactByContactId( contactId );
	if (!mc) return;

	KopeteContact *c = mc->execute(); //We need to know which contact was chosen as the preferred in order to message it
	if (!c) return;

	KopeteMessage msg(c->account()->myself(), c, messageText, KopeteMessage::Outbound);
	c->manager(true)->sendMessage(msg);

}


QStringList KopeteContactList::contactFileProtocols(const QString &displayName)
{
//	kdDebug(14010) << "Get contacts for: " << displayName << "\n";
	QStringList protocols;

	KopeteMetaContact *c = findContactByDisplayName( displayName );
	if( c )
	{
		QPtrList<KopeteContact> mContacts = c->contacts();
		kdDebug(14010) << mContacts.count() << endl;
		QPtrListIterator<KopeteContact> jt( mContacts );
		for ( ; jt.current(); ++jt )
		{
			kdDebug(14010) << "1" << jt.current()->protocol()->pluginId() << endl;
			if( jt.current()->canAcceptFiles() ) {
				kdDebug(14010) << jt.current()->protocol()->pluginId() << endl;
				protocols.append ( jt.current()->protocol()->pluginId() );
			}
		}
		return protocols;
	}
	return QStringList();
}



#endif


void ContactList::slotSaveLater()
{
	// if we already have a save scheduled, it will be cancelled. either way,
	// start a timer to save the contact list a bit later.
	d->saveTimer->start( 17100 /* 17,1 seconds */, true /* single-shot */ );
}

void ContactList::slotKABCChanged()
{
	// call syncWithKABC on each metacontact to check if its associated kabc entry has changed.
	for ( MetaContact * mc = d->contacts.first(); mc; mc = d->contacts.next() )
	{
#if 0//TODO
		mc->syncWithKABC();
 #endif
	}
}

} //END namespace Kopete

#include "kopetecontactlist.moc"

// vim: set noet ts=4 sts=4 sw=4:

