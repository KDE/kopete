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
#include <qregexp.h>

#include <kdebug.h>
#include <klocale.h>
#include <knotifyclient.h>

#include "kopete.h"
#include "kopetecontactlist.h"
#include "kopetecontactlistview.h"
#include "kopetemetacontactlvi.h"
#include "kopeteplugin.h"
#include "kopeteprotocol.h"
#include "kopeteprefs.h"
#include "pluginloader.h"

#if KDE_VERSION >= 305
#include <kpassivepopup.h>
#include "systemtray.h"
#endif

KopeteMetaContact::KopeteMetaContact()
: QObject( KopeteContactList::contactList() )
{
	//TODO: implement m_trackChildNameChanges
	m_trackChildNameChanges = false;
	m_temporary=false;
//	m_isTopLevel=false;

	m_onlineStatus = Unknown;
}

KopeteMetaContact::~KopeteMetaContact()
{
}

void KopeteMetaContact::addContact( KopeteContact *c )
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

	/*	for( QStringList::ConstIterator it = groups.begin(); it != groups.end(); ++it )
		{
			addToGroup(*it);
		}*/
		emit contactAdded(c);
	}

	updateOnlineStatus();
}

void KopeteMetaContact::updateOnlineStatus()
{
	OnlineStatus newStatus = Unknown;

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
		else if (s == KopeteContact::Offline && newStatus!=Away)
		{
			newStatus = Offline;
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
	if(m_groups.isEmpty())
		m_groups.append(KopeteGroup::toplevel);	
	return (m_groups.contains(KopeteGroup::toplevel));
}

void KopeteMetaContact::setTopLevel( bool b )
{
	if(b)
	{
		if(!isTopLevel())
			m_groups.append(KopeteGroup::toplevel);
	}
	else
	{
			m_groups.remove(KopeteGroup::toplevel);
	}
}

KopeteContact *KopeteMetaContact::findContact( const QString &protocolId,
	const QString &identityId, const QString &contactId )
{
	//kdDebug() << "*** Num contacts: " << m_contacts.count() << endl;
	QPtrListIterator<KopeteContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		//kdDebug() << "*** Trying " << it.current()->id() << ", proto " << it.current()->protocol() << endl;
		if( (it.current()->id() == contactId ) && (it.current()->protocol()->id() == protocolId ) && (it.current()->identityId() == identityId))
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
		case Unknown:
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
			return "Offline";
		case Unknown:
		default:
			return "Status not avaliable";
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

void KopeteMetaContact::slotContactStatusChanged( KopeteContact * c,
	KopeteContact::ContactStatus /* s */ )
{
	OnlineStatus m = m_onlineStatus;
	updateOnlineStatus();
	if ( (m_onlineStatus != m) && (m_onlineStatus==Online) && (KopetePrefs::prefs()->soundNotify()) )
	{
		#if KDE_VERSION >= 305
		if ( KopetePrefs::prefs()->notifyOnline() )
			KPassivePopup::message(i18n("%2 is now %1!").arg(statusString()).arg(displayName()), kopeteapp->systemTray());
		#endif

		KopeteProtocol* p = dynamic_cast<KopeteProtocol*>(c->protocol());
		if (!p)
		{
			kdDebug() <<"KopeteMetaContact::slotContactStatusChanged: KopeteContact is not from a valid Protocol" <<endl;
			return;
		}
		if ( !p->isAway() || KopetePrefs::prefs()->soundIfAway() )
			KNotifyClient::event("kopete_online");
	}
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

void KopeteMetaContact::moveToGroup(  KopeteGroup *from,  KopeteGroup *to )
{
	if(m_temporary && to!=KopeteGroup::temporary)
		return;

	if (!from || !m_groups.contains( from ) || (from==KopeteGroup::toplevel && !isTopLevel()))
	{
		addToGroup(to);
		return;
	}
	
	if(!to ||  m_groups.contains( to ) || (to==KopeteGroup::toplevel && isTopLevel()) )
	{
		removeFromGroup(from);
		return;
	}

//	kdDebug() << "KopeteMetaContact::moveToGroup: "<<from.string() <<" => "<<to.string() << endl;

	m_groups.remove( from );

	m_groups.append( to );	

/*	for( 	KopeteContact *c = m_contacts.first(); c ; c = m_contacts.next() )
	{
		c->moveToGroup(from,to);
	}*/

	emit movedToGroup(from, to, this );
}

void KopeteMetaContact::removeFromGroup(  KopeteGroup *from)
{
	if(m_temporary && from==KopeteGroup::temporary)
		return ;

	if (!from || !m_groups.contains( from ) || (from==KopeteGroup::toplevel && !isTopLevel()))
	{
		/*kdDebug() << "KopeteMetaContact::removeFromGroup: This contact is removed from all groups, deleting contact" <<endl;
		KopeteContactList::contactList()->removeMetaContact(this);*/
		return;
	}

	m_groups.remove( from );


	/*for( 	KopeteContact *c = m_contacts.first(); c ; c = m_contacts.next() )
	{
		c->removeFromGroup(from);
	} */

	emit removedFromGroup(  from, this);
}

void KopeteMetaContact::addToGroup(  KopeteGroup *to )
{
	if(m_temporary && to!=KopeteGroup::temporary)
		return;

	if(!to ||  m_groups.contains( to ) || (to==KopeteGroup::toplevel && isTopLevel()))
	{
		return;
	}

	m_groups.append( to );

	/*for( 	KopeteContact *c = m_contacts.first(); c ; c = m_contacts.next() )
	{
		c->addToGroup(to);
	}*/

	emit addedToGroup( to ,this );
}

KopeteGroupList KopeteMetaContact::groups() const
{
/*	if(m_groups.isEmpty())
		m_groups.append(KopeteGroup::toplevel);*/
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
		KopeteGroup *g;;
		for ( g = m_groups.first(); g; g = m_groups.next() )
		{
			QString group_s=g->displayName();
			if(!group_s.isNull())
			{
				if ( !group_s.isEmpty() )
					xml += "      <group>" + QStyleSheet::escape( group_s) + "</group>\n";
				else
				  xml += "      <group>" + QStyleSheet::escape( i18n("Unknown") ) + "</group>\n";
			}
		}

		// The contact is also at top-level
		if ( isTopLevel() )
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
		xml += "    <groups><top-level/></groups>\n";
	}

	QPtrList<KopetePlugin> ps = kopeteapp->libraryLoader()->plugins();
	for( KopetePlugin *p = ps.first() ; p != 0L; p = ps.next() )
	{
		QStringList strList;
		if ( p->serialize( this, strList ) && !strList.empty() )
		{
			for ( QStringList::iterator it = strList.begin(); it != strList.end(); ++it )
			{
				//escape '||' I don't like this but it is needed
				(*it)=(*it).replace(QRegExp("\\\\"),"\\\\").replace(QRegExp("\\|"),"\\|;");
			}
			QString data = QStyleSheet::escape( strList.join( "||" ) );
//			kdDebug() << "KopeteMetaContact::toXML: plugin-data = " << data <<endl;
			xml += "    <plugin-data plugin-id=\"" +
				QStyleSheet::escape( p->id())  + "\">" + data  + "</plugin-data>\n";
		}
	}

	// Store address book fields
	AddressBookFields::Iterator addrIt = m_addressBook.begin();
	for( ; addrIt != m_addressBook.end(); ++addrIt )
	{
//		kdDebug() << "KopeteMetaContact::toXML: Storing address book field "
//			<< addrIt.key() << " with value '" << addrIt.data() << "'" << endl;
		xml += "    <address-book-field id=\"" + QStyleSheet::escape(addrIt.key()) + "\">" +
					QStyleSheet::escape(addrIt.data()) + "</address-book-field>\n";
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
			xml += "    <plugin-data plugin-id=\"" + QStyleSheet::escape(it.key()) + "\">"
				+ QStyleSheet::escape(it.data()) + "</plugin-data>\n";
		}
	}

	xml += "  </meta-contact>\n";

	return xml;
}

bool KopeteMetaContact::fromXML( const QDomNode& cnode )
{
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
						m_groups.append(KopeteContactList::contactList()->getGroup(groupElement.text()));
					}
					else if ( groupElement.tagName() == "top-level" )
					{
						m_groups.append(KopeteGroup::toplevel);
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
		KopetePlugin *plugin = kopeteapp->libraryLoader()->searchByID( it.key() );
		if( plugin )
		{
			QStringList strList = QStringList::split( "||", it.data() );
	
			for ( QStringList::iterator it2 = strList.begin(); it2 != strList.end(); ++it2 )
			{
				//unescape '||' 
				(*it2)=(*it2).replace(QRegExp("\\\\\\|;"),"|").replace(QRegExp("\\\\\\\\"),"\\");
			}

			plugin->deserialize( this, strList );
		}
	}

	// If a plugin is loaded, load data cached
	connect( kopeteapp->libraryLoader(), SIGNAL( pluginLoaded(KopetePlugin*) ),
		this, SLOT( slotPluginLoaded(KopetePlugin*) ) );

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
void KopeteMetaContact::setTemporary( bool b , KopeteGroup *group )
{
	m_temporary=b;
	KopeteGroup *temporaryGroup=KopeteGroup::temporary;

	if(m_temporary)
	{
		addToGroup (temporaryGroup);
		KopeteGroup *g;
		for ( g = m_groups.first(); g; g = m_groups.next() )
		{
			if(g != temporaryGroup)
				removeFromGroup(g);
		}
	}
	else
		moveToGroup(temporaryGroup,group);
}

bool KopeteMetaContact::isDirty() const
{
	return m_dirty;
}

void KopeteMetaContact::setDirty( bool b  )
{
	m_dirty = b;
}

void KopeteMetaContact::slotPluginLoaded( KopetePlugin *p )
{
	if( !p )
		return;

	QMap<QString, QString>::ConstIterator it;
	for( it = m_pluginData.begin(); it != m_pluginData.end(); ++it )
	{
		if( p->id() == it.key() )
		{
			QStringList strList = QStringList::split( "||", it.data() );
			for ( QStringList::iterator it2 = strList.begin(); it2 != strList.end(); ++it2 )
			{
				//unescape '||'
				(*it2)=(*it2).replace(QRegExp("\\\\\\|;"),"|").replace(QRegExp("\\\\\\\\"),"\\");
			}

			p->deserialize( this, strList );
		}
	}
}

#include "kopetemetacontact.moc"

// vim: set noet ts=4 sts=4 sw=4:


