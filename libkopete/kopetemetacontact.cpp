/*
    kopetemetacontact.cpp - Kopete Meta Contact

    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetemetacontact.h"

#include <qdom.h>
#include <qptrlist.h>
#include <qstylesheet.h>

#include <kdebug.h>
#include <klocale.h>

#include "kopete.h"
#include "kopetecontactlist.h"
#include "kopetecontactlistview.h"
#include "kopetemetacontactlvi.h"
#include "kopeteplugin.h"
#include "pluginloader.h"

KopeteMetaContact::KopeteMetaContact()
: QObject( KopeteContactList::contactList() )
{
	//TODO: implement m_trackChildNameChanges
	m_trackChildNameChanges = false;
	m_temporary=false;
	m_isTopLevel=false;

	m_onlineStatus = Offline;
}

KopeteMetaContact::~KopeteMetaContact()
{
}

void KopeteMetaContact::addContact( KopeteContact *c )
{
	// If the meta contact isn't in any group, make them member of the
	// new contact's groups, otherwise don't care about the groups
	// (FIXME: Why is this? - Martijn)
	if( m_groups.isEmpty() )
		addContact( c, c->groups() );
	else
		addContact( c, QStringList() );
}

void KopeteMetaContact::addContact( KopeteContact *c,
	const QStringList &groups )
{
	if( m_contacts.contains( c ) )
	{
		kdDebug() << "KopeteMetaContact::addContact: WARNING: "
			<< "Ignoring attempt to add duplicate contact " << c->id()
			<< "!" << endl;
	}
	else
	{
		m_contacts.append( c );

		connect( c, SIGNAL( statusChanged( KopeteContact *,
			KopeteContact::ContactStatus ) ),
			this, SLOT( slotContactStatusChanged( KopeteContact *,
			KopeteContact::ContactStatus ) ) );

		connect( c, SIGNAL( displayNameChanged( const QString & ) ),
			this, SLOT( slotContactNameChanged( const QString & ) ) );

		connect( c, SIGNAL( destroyed( QObject * ) ),
			this, SLOT( slotContactDestroyed( QObject * ) ) );

		 if (m_displayName == QString::null)
			 setDisplayName( c->displayName() );

		for( QStringList::ConstIterator it = groups.begin(); it != groups.end(); ++it )
		{
			addToGroup(*it);
		}
		emit contactAdded(c);
	}

	updateOnlineStatus();
}

void KopeteMetaContact::updateOnlineStatus()
{
	OnlineStatus newStatus = Offline;

	QPtrListIterator<KopeteContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		KopeteContact::ContactStatus s = it.current()->status();

		if ( s == KopeteContact::Online )
		{
			newStatus = Online;
			break;
		}
		else if ( s == KopeteContact::Away )
		{
			// Set status, but don't stop searching, since 'Online' overrules
			// 'Away'
			newStatus = Away;
		}
	}

	if( newStatus != m_onlineStatus )
	{
		m_onlineStatus = newStatus;
		emit onlineStatusChanged( this, m_onlineStatus );
	}
}

void KopeteMetaContact::removeContact(KopeteContact *c, bool deleted)
{
	if( !m_contacts.contains( c ) )
	{
		kdDebug() << "KopeteMetaContact::removeContact: Contact is not in this metaContact " << endl;
	}
	else
	{
		m_contacts.remove( c );

		if(!deleted)
		{  //If this function is tell by slotContactRemoved, c is maybe just a QObject
			disconnect( c, SIGNAL( statusChanged( KopeteContact *,
				KopeteContact::ContactStatus ) ),
				this, SLOT( slotContactStatusChanged( KopeteContact *,
				KopeteContact::ContactStatus ) ) );

			disconnect( c, SIGNAL( displayNameChanged( const QString & ) ),
				this, SLOT( slotContactNameChanged( const QString & ) ) );
	
			disconnect( c, SIGNAL( destroyed( QObject * ) ),
				this, SLOT( slotContactDestroyed( QObject * ) ) );

			kdDebug() << "KopeteMetaContact::removeContact: Contact disconected" << endl;
		}
		emit contactRemoved(c);
	}

	updateOnlineStatus();
}


bool KopeteMetaContact::isTopLevel()
{
	if(groups().isEmpty())
		m_isTopLevel=true;
	return m_isTopLevel;
}

void KopeteMetaContact::setTopLevel( bool b )
{
	if(m_isTopLevel!=b)
	{
		m_isTopLevel = b;
		emit topLevel(this, b);
	}
}
 
KopeteContact *KopeteMetaContact::findContact( const QString &protocolId, const QString &identityId, const QString &contactId )
{
	//kdDebug() << "*** Num contacts: " << m_contacts.count() << endl;
	QPtrListIterator<KopeteContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		//kdDebug() << "*** Trying " << it.current()->id() << ", proto " << it.current()->protocol() << endl;
		if( (it.current()->id() == contactId ) && (it.current()->protocol() == protocolId ) && (it.current()->identityId() == identityId))
			return it.current();
	}

	// Contact not found
	return 0L;
}

void KopeteMetaContact::sendMessage()
{
	kdDebug() << "KopeteMetaContact::sendMessage() not implemented!" << endl;

	/*
		Algorithm:
		1. Determine the protocol to use
		   a. In the configuration the user can specify the order in which
		      protocols are tried, so if multiple protocols are available, the
		      preferred one is used.
		   b. Iterate over the preference order to see if a contact exists
		      and is online for this protocol and if the same protocol is
		      connected. Offline contacts are ignored, as are contacts for
		      which the protocol is currently disconnected.
		   c. If the protocol is not found in step b, repeat, but now looking
		      for protocols that support offline messages and send an offline
		      message.
		   d. If no combination of a connected protocol and a reachable user
		      ( either with online or offline messages ) is found, display
		      an error and return.
		2. Show dialog for entering the message
		3. In the background, try to open a chat session. This is a no-op
		   for message-based protocols like ICQ
		4. Send the message
		5. Close the chat session, if any, for the connection-based protocols
	*/
}

void KopeteMetaContact::startChat()
{
	kdDebug() << "KopeteMetaContact::startChat() not implemented!" << endl;

	/*
		Algorithm:
		1. Determine the protocol as described in sendMessage(), with a
		   small difference:
		   If a contact can *only* be reached by using offline messages,
		   notify the user and switch to sendMessage() mode. Interactive
		   chats with offline users are a bit pointless after all...
		2. Show the chat dialog
		3. Open the session in the background
		4. Send messages, until the dialog is closed
		5. Close the chat session

		Caveats:
		- while connecting in protocols like MSN, messages might get 'sent'
		  by the user before the connection is open. Until the connection
		  signals a 'ready' condition all messages should be queued. This
		  should be done here to avoid code duplication
		- while sending messages you have to wait for the server confirmation
		  with most protocols. In MSN this latency is small enough to not
		  notice and MSN also requires a builtin queue for protocol-specific
		  reasons, but in general the queuing should be done here. Until the
		  chat session signals a 'message sent' condition any subsequent
		  messages should be put on hold and queued. Kopete should also show
		  a simple spinning hour glass animation or something similar to
		  indicate that messages are still pending.
	*/

	// FIXME: Implement the above!
	//now just select the highter status importance
	if( m_contacts.isEmpty() )
		return;

	KopeteContact *c=m_contacts.first();
	for(QPtrListIterator<KopeteContact> it( m_contacts ) ; it.current(); ++it )
	{
		if( (*it)->importance() > c->importance())
			c=*it;
	}
	
	c->execute();
}

void KopeteMetaContact::execute()
{
	// FIXME: Implement, don't hardcode startChat()!
	startChat();
}

QString KopeteMetaContact::statusIcon() const
{
	switch( status() )
	{
		case Online:
			return "metacontact_online";
		case Away:
			return "metacontact_away";
		case Offline:
		default:
			return "metacontact_offline";
	}
}

QString KopeteMetaContact::statusString() const
{
	switch( status() )
	{
		case Online:
			return "Online";
		case Away:
			return "Away";
		case Offline:
		default:
			return "Offline";
	}
}

KopeteMetaContact::OnlineStatus KopeteMetaContact::status() const
{
	return m_onlineStatus;
}

bool KopeteMetaContact::isOnline() const
{
	QPtrListIterator<KopeteContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		if( it.current()->isOnline() )
			return true;
	}
	return false;
}

bool KopeteMetaContact::isReachable() const
{
	if( isOnline() )
		return true;

	QPtrListIterator<KopeteContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		// FIXME: implement KopeteContact::protocol()!!!
		//if( it.current()->protocol()->canSendOffline() )
		//	return true;
		continue;
	}
	return false;
}

void KopeteMetaContact::slotContactStatusChanged( KopeteContact * /* c */,
	KopeteContact::ContactStatus /* s */ )
{
	updateOnlineStatus();
}

void KopeteMetaContact::setDisplayName( const QString &name )
{
	m_displayName = name;
	emit displayNameChanged( this, name );
}

QString KopeteMetaContact::displayName() const
{
	return m_displayName;
}

void KopeteMetaContact::slotContactNameChanged( const QString &name )
{
	if( m_trackChildNameChanges )
		setDisplayName( name );
}

void KopeteMetaContact::moveToGroup( const QString &from, const QString &to )
{
	if(m_temporary && to!="temporaryGroup")  
		return;
	
	if(  m_groups.contains( to ) || (to.isNull() && isTopLevel()) ||  (!m_groups.contains( from ) && !from.isNull() ) || (from.isNull() && !isTopLevel()))
		return;
	
	if ( from.isNull() && to.isNull() )
		return;

	kdDebug() << "KopeteMetaContact::moveToGroup: "<<from <<" => "<<to << endl;


	if (from.isNull())
	{    // from top-level to a group
		m_isTopLevel=false;
	}
	else
		m_groups.remove( from );

	if (to.isNull())
	{    // from group to top-level	
      m_isTopLevel=true; 
	}
	else
		m_groups.append( to );	

	for( 	KopeteContact *c = m_contacts.first(); c ; c = m_contacts.next() )
	{
		c->moveToGroup(from,to);
	}

	emit movedToGroup(this, from, to );
}

void KopeteMetaContact::removeFromGroup( const QString &from)
{
	if(m_temporary && from=="temporaryGroup")
		return ;

	if( from.isNull() && groups().isEmpty())
	{
		kdDebug() << "KopeteMetaContact::removeFromGroup: This contact is removed from all groups, deleting contact" <<endl;
		KopeteContactList::contactList()->removeMetaContact(this);
	   return ;
	}
	
	if ( from.isNull() )
	{
		if(!isTopLevel())
			return;
		m_isTopLevel=false;
	}
	else
	{
		if( !m_groups.contains( from ) )
			return;

		m_groups.remove( from );
	}

	for( 	KopeteContact *c = m_contacts.first(); c ; c = m_contacts.next() )
	{
		c->removeFromGroup(from);
	}

	emit removedFromGroup( this, from);

}

void KopeteMetaContact::addToGroup( const QString &to )
{
	if(m_temporary && to!="temporaryGroup")
		return;

	if ( to.isNull() )
	{
		if(isTopLevel())
			return;
			
		m_isTopLevel=true;
	}
	else
	{
		if( m_groups.contains( to ) )
			return ;

		m_groups.append( to );
   }
	for( 	KopeteContact *c = m_contacts.first(); c ; c = m_contacts.next() )
	{
		c->addToGroup(to);
	}

	emit addedToGroup( this, to );

}

QStringList KopeteMetaContact::groups() const
{
	return m_groups;
}

void KopeteMetaContact::slotContactDestroyed( QObject *obj )
{

	KopeteContact *contact = static_cast<KopeteContact *>( obj );

	removeContact(contact,true);
	
}

QString KopeteMetaContact::toXML()
{
	QString xml = "  <meta-contact id=\"TODO: KABC ID\">\n"
		"    <display-name>" +
		QStyleSheet::escape( m_displayName ) +
		"</display-name>\n";

	// Store groups
	if ( !m_groups.isEmpty() )
	{
		xml += "    <groups>\n";
		for( QStringList::ConstIterator it = m_groups.begin(); it != m_groups.end(); ++it )
		{
			if ( !(*it).isEmpty() )
				xml += "      <group>" + QStyleSheet::escape( *it ) +
				       "</group>\n";
			else
				xml += "      <group>" + QStyleSheet::escape( i18n("Unknown") ) +
				       "</group>\n";
		}

		// The contact is also at top-level
		if ( m_isTopLevel )
		{
			xml += "      <top-level/>\n";
		}
		
		xml += "    </groups>\n";
	}
	else
	{
		/*
		   Rare case to prevent bug, if contact has no groups
		   and it is not at top level it should have been deleted.
		   But we didn't, so we put it in toplevel to prevent a
		   hided contact, also for toplevel contacts saved before
		   we added the <top-level> tag.
		*/
		if ( ! m_isTopLevel )
			xml += "    <groups><top-level/></groups>\n";
	}

	QPtrList<KopetePlugin> ps = kopeteapp->libraryLoader()->plugins();
	for( KopetePlugin *p = ps.first() ; p != 0L; p = ps.next() )
	{
		//++pluginIt;
		QStringList strList;
		if ( p->serialize( this, strList ) && !strList.empty() )
		{
			QString data = QStyleSheet::escape( strList.join( "||" ) );
			kdDebug() << "KopeteMetaContact::toXML: plugin-data = " << data <<endl;
			xml += "    <plugin-data plugin-id=\"" +
				QString( p->id() ) + "\">" + data  + "</plugin-data>\n";
		}
	}

	// Store address book fields
	AddressBookFields::Iterator addrIt = m_addressBook.begin();
	for( ; addrIt != m_addressBook.end(); ++addrIt )
	{
		kdDebug() << "KopeteMetaContact::toXML: Storing address book field "
			<< addrIt.key() << " with value '" << addrIt.data() << "'" << endl;
		xml += "    <address-book-field id=\"" + addrIt.key() + "\">" +
					addrIt.data() + "</address-book-field>\n";
	}

	// We may have more 'cached' plugin data from plugins that are not
	// loaded at the moment. Assume our copy is still valid and store it
	QMap<QString, QString>::ConstIterator it;
	for( it = m_pluginData.begin(); it != m_pluginData.end(); ++it )
	{
		KopetePlugin *plugin = kopeteapp->libraryLoader()->searchByID(
			it.key() );

		if( !plugin )
		{
			xml += "    <plugin-data plugin-id=\"" + it.key() + "\">"
				+ it.data() + "</plugin-data>\n";
		}
	}

	xml += "  </meta-contact>\n";

	return xml;
}

bool KopeteMetaContact::fromXML( const QDomNode& cnode )
{
	m_isTopLevel = false;

	QDomNode contactNode = cnode;
	while( !contactNode.isNull() )
	{
		QDomElement contactElement = contactNode.toElement();
		if( !contactElement.isNull() )
		{
			if( contactElement.tagName() == "display-name" )
			{
				if ( contactElement.text().isEmpty() )
					return false;
				m_displayName = contactElement.text();
			}
			else if( contactElement.tagName() == "groups" )
			{
				QDomNode group = contactElement.firstChild();
				while( !group.isNull() )
				{
                    QDomElement groupElement = group.toElement();
					if ( groupElement.tagName() == "group" )
					{
						m_groups << groupElement.text();
					}
					else if ( groupElement.tagName() == "top-level" )
					{
						m_isTopLevel = true;
					}
	
					group = group.nextSibling();
				}
			}

			else if( contactElement.tagName() == "address-book-field" )
			{
				QString id = contactElement.attribute( "id", QString::null );
				QString val = contactElement.text();
				m_addressBook.insert( id, val );

			}
			else if( contactElement.tagName() == "plugin-data" )
			{
				QString pluginId = contactElement.attribute(
					"plugin-id", QString::null );
				m_pluginData.insert( pluginId, contactElement.text() );
			}

		}
		contactNode = contactNode.nextSibling();
	}

	// Deserialize when the *whole* meta contact has been read!
	QMap<QString, QString>::ConstIterator it;
	for( it = m_pluginData.begin(); it != m_pluginData.end(); ++it )
	{
		QStringList strList = QStringList::split( "||", it.data() );
		KopetePlugin *plugin = kopeteapp->libraryLoader()->searchByID(
			it.key() );

		if( plugin )
			plugin->deserialize( this, strList );
	}

	//If a plugin is loaded, load data cached
	connect( kopeteapp->libraryLoader(), SIGNAL( pluginLoaded(KopetePlugin*) ),
			this, SLOT( slotPluginLoaded(KopetePlugin*) ) );


	if ( m_groups.isEmpty() && ! m_isTopLevel )
		m_isTopLevel = true;

	return true;
}

QString KopeteMetaContact::addressBookField( KopetePlugin * p,
	const QString & key ) const
{
	if ( p && p->addressBookFields().contains( key ) ) {
		if ( m_addressBook.contains( key ) ) {
			return m_addressBook[ key ];
		} else
			return QString::null;
	} else
		return QString::null;
}

void KopeteMetaContact::setAddressBookField( KopetePlugin * p ,
	const QString & key, const QString & value )
{
	if ( p && p->addressBookFields().contains( key ) )
		m_addressBook.insert( key, value );
	else
		kdDebug() << "[KopeteMetaContact::setAddressBookField] Sorry, plugin "
			  << p->id() << " doesn't have field "
			  << key << " registered" << endl;
}

KopeteMetaContact::AddressBookFields KopeteMetaContact::addressBookFields() const
{
	return m_addressBook;
}

bool KopeteMetaContact::isTemporary() const
{
	return m_temporary;
}
void KopeteMetaContact::setTemporary( bool b  )
{
	m_temporary=b;

	if(m_temporary)
	{
		addToGroup ("temporaryGroup");
		for( QStringList::ConstIterator it = m_groups.begin(); it != m_groups.end(); ++it )
		{
			if(*it != "temporaryGroup")
				removeFromGroup("temporaryGroup");
		}
	}
	else
		moveToGroup("temporaryGroup",QString::null);  //move to top-level
}

void KopeteMetaContact::slotPluginLoaded(KopetePlugin *p)
{
	if(!p)
		return;
	QMap<QString, QString>::ConstIterator it;
	for( it = m_pluginData.begin(); it != m_pluginData.end(); ++it )
	{
		KopetePlugin *plugin = kopeteapp->libraryLoader()->searchByID(it.key());
		if(plugin==p)
		{
			QStringList strList = QStringList::split( "||", it.data() );
			p->deserialize( this, strList );
		}
	}
}

#include "kopetemetacontact.moc"

// vim: set noet ts=4 sts=4 sw=4:


