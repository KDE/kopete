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

#include <qapplication.h>
#include <qdom.h>
#include <qptrlist.h>
#include <qstylesheet.h>
#include <qregexp.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knotifyclient.h>

#include "kopetecontactlist.h"
#include "kopeteplugin.h"
#include "kopeteprotocol.h"
#include "kopeteprefs.h"
#include "pluginloader.h"

#if KDE_VERSION >= 306
#include <kpassivepopup.h>
#include "systemtray.h"
#endif

KopeteMetaContact::KopeteMetaContact()
: QObject( KopeteContactList::contactList() )
{
	m_trackChildNameChanges = false;
	m_temporary=false;
//	m_isTopLevel=false;

	m_onlineStatus = Unknown;
	m_idleState = Unspecified;
}

KopeteMetaContact::~KopeteMetaContact()
{
}

void KopeteMetaContact::addContact( KopeteContact *c )
{
	if( m_contacts.contains( c ) )
	{
		kdWarning(14010) << "Ignoring attempt to add duplicate contact " << c->contactId() << "!" << endl;
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

		connect( c, SIGNAL( contactDestroyed( KopeteContact * ) ),
			this, SLOT( slotContactDestroyed( KopeteContact * ) ) );

		connect( c, SIGNAL( idleStateChanged( KopeteContact *, KopeteContact::IdleState ) ),
			this, SLOT( slotContactIdleStateChanged( KopeteContact *, KopeteContact::IdleState ) ) );

		if( m_displayName.isNull() )
		{
			setDisplayName( c->displayName() );
			m_trackChildNameChanges = false;
		}

		if (m_contacts.count() > 1)
		{
			kdDebug(14010) << "[KopeteMetaContact] addContact(); disabling trackChildNameChanges,"
			" more than ONE Contact in MetaContact" << endl;
			m_trackChildNameChanges=false;
		}

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

void KopeteMetaContact::updateIdleState()
{
	IdleState newStatus = Unspecified;

	QPtrListIterator<KopeteContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		KopeteContact::IdleState s = it.current()->idleState();

		if ( s == KopeteContact::Active )
		{
			newStatus = Active;
			break;
		}
		else if ( s == KopeteContact::Idle )
		{
			// Set status, but don't stop searching, since 'Active' overrules
			// 'Idle'
			newStatus = Idle;
		}
	}

	if( newStatus != m_idleState )
	{
		m_idleState = newStatus;
		emit idleStateChanged( this, m_idleState );
	}
}

void KopeteMetaContact::removeContact(KopeteContact *c, bool deleted)
{
	if( !m_contacts.contains( c ) )
	{
		kdDebug(14010) << "KopeteMetaContact::removeContact: Contact is not in this metaContact " << endl;
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

			disconnect( c, SIGNAL( contactDestroyed( KopeteContact * ) ),
				this, SLOT( slotContactDestroyed( KopeteContact * ) ) );

			disconnect( c, SIGNAL( idleStateChanged( KopeteContact *, KopeteContact::IdleState ) ),
				this, SLOT( slotContactIdleStateChanged( KopeteContact *, KopeteContact::IdleState ) ) );

			kdDebug(14010) << "KopeteMetaContact::removeContact: Contact disconected" << endl;
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
	//kdDebug(14010) << "*** Num contacts: " << m_contacts.count() << endl;
	QPtrListIterator<KopeteContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		//kdDebug(14010) << "*** Trying " << it.current()->contactId() << ", proto " << it.current()->protocol()->pluginId() << ", identity " << it.current()->identityId() << endl;
		if( (it.current()->contactId() == contactId ) && (it.current()->protocol()->pluginId() == protocolId ) && (it.current()->identityId() == identityId))
			return it.current();
	}

	// Contact not found
	return 0L;
}

void KopeteMetaContact::sendMessage()
{
	kdDebug(14010) << "KopeteMetaContact::sendMessage() not implemented!" << endl;

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
	kdDebug(14010) << "KopeteMetaContact::startChat() not implemented!" << endl;

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

	KopeteContact *c = 0L;
	for( QPtrListIterator<KopeteContact> it( m_contacts ) ; it.current(); ++it )
	{
		if( ( !c || ( *it )->importance() > c->importance() ) &&
			( *it )->isReachable() )
		{
			c = *it;
		}
	}

	if( c )
	{
		c->execute();
	}
	else
	{
		KMessageBox::error( qApp->mainWidget(),
			i18n( "This user is not reachable at the moment. Please"
				"try a protocol that supports offline sending, or wait "
				"until this user goes online." ),
			i18n( "User is not reachable - Kopete" ) );
	}
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
			return i18n("Online");
		case Away:
			return i18n("Away");
		case Offline:
			return i18n("Offline");
		case Unknown:
		default:
			return i18n("Status not available");
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

//Determine if we are capable of accepting file transfers
bool KopeteMetaContact::canAcceptFiles() const
{
	if( !isOnline() )
		return false;

	QPtrListIterator<KopeteContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		if( it.current()->canAcceptFiles() )
			return true;
	}
	return false;
}

//Slot for sending files
void KopeteMetaContact::sendFile( const KURL &sourceURL, const QString &altFileName, unsigned long fileSize )
{
	//If we can't send any files then exit
	if( m_contacts.isEmpty() || !canAcceptFiles() )
		return;

	//Find the highest ranked protocol that can accept files
	KopeteContact *c=m_contacts.first();
	for(QPtrListIterator<KopeteContact> it( m_contacts ) ; it.current(); ++it )
	{
		if( ( (*it)->importance() > c->importance() ) && ( (*it)->canAcceptFiles() ) )
			c=*it;
	}

	//Call the sendFile slot of this protocol
	c->sendFile( sourceURL, altFileName, fileSize );
}

void KopeteMetaContact::slotContactStatusChanged( KopeteContact * c,
	KopeteContact::ContactStatus  s  )
{
	emit contactStatusChanged(c,s);
	OnlineStatus m = m_onlineStatus;
	updateOnlineStatus();
	if ( (m_onlineStatus != m) && (m_onlineStatus==Online) && (KopetePrefs::prefs()->soundNotify()) )
	{
		#if KDE_VERSION >= 306
		if ( KopetePrefs::prefs()->notifyOnline() )
		{
			KPassivePopup::message( i18n( "%2 is now %1!" ).arg(
				statusString() ).arg( displayName() ),
				KopeteSystemTray::systemTray() );
		}
		#endif

		KopeteProtocol* p = dynamic_cast<KopeteProtocol*>(c->protocol());
		if (!p)
		{
			kdDebug(14010) <<"[KopeteMetaContact] slotContactStatusChanged(); KopeteContact is not from a valid Protocol" <<endl;
			return;
		}
		if ( !p->isAway() || KopetePrefs::prefs()->soundIfAway() )
			KNotifyClient::event("kopete_online");
	}
}

void KopeteMetaContact::setDisplayName( const QString &name )
{
	kdDebug( 14000 ) << k_funcinfo << "Set new contact name to " << name  <<
		", m_trackChildNameChanges=" << m_trackChildNameChanges << endl;

	if( name == m_displayName )
		return;

	m_displayName = name;
	m_trackChildNameChanges = false;

	for( KopeteContact *c = m_contacts.first(); c ; c = m_contacts.next() )
		c->rename( name );

	emit displayNameChanged( this, name );
}

QString KopeteMetaContact::displayName() const
{
	return m_displayName;
}

bool KopeteMetaContact::trackChildNameChanges() const
{
	return m_trackChildNameChanges;
}

void KopeteMetaContact::setTrackChildNameChanges(bool track)
{
/*
	if (track && (m_contacts.count() == 1))
	{
		kdDebug(14010) << "[KopeteMetaContact] setTrackChildNameChanges(); ENABLING TrackChildNameChanges" << endl;
		setDisplayName( (m_contacts.first())->displayName() );
		m_trackChildNameChanges = true;
	}
	else
	{
		kdDebug(14010) << "[KopeteMetaContact] setTrackChildNameChanges(); DISABLING TrackChildNameChanges" << endl;
		m_trackChildNameChanges = false;
	}
*/
}

void KopeteMetaContact::slotContactNameChanged( const QString &name )
{
//	kdDebug(14010) << "[KopeteMetaContact] slotContactNameChanged(); name=" << name <<
//		", m_trackChildNameChanges=" << m_trackChildNameChanges << "." << endl;

	if( m_trackChildNameChanges )
	{
		setDisplayName( name );
		//because m_trackChildNameChanges is set to false in setDisplayName
//		m_trackChildNameChanges = true;
	}
	else
	{
		kdDebug(14010) << "[KopeteMetaContact] slotContactNameChanged(); IGNORED contact namechange. "<< endl;
	}
}

void KopeteMetaContact::moveToGroup( KopeteGroup *from, KopeteGroup *to )
{
	if( isTemporary() && to != KopeteGroup::temporary )
		return;

	if( !from || !m_groups.contains( from ) ||
		( !isTopLevel() && from == KopeteGroup::toplevel ) )
	{
		// We're adding, not moving, because 'from' is illegal
		addToGroup( to );
		return;
	}

	if( !to || m_groups.contains( to ) ||
		( isTopLevel() && to == KopeteGroup::toplevel ) )
	{
		// We're removing, not moving, because 'to' is illegal
		removeFromGroup( from );
		return;
	}

	kdDebug( 14010 ) << k_funcinfo << from->displayName() << " => " << to->displayName() << endl;

	m_groups.remove( from );
	m_groups.append( to );

	for( KopeteContact *c = m_contacts.first(); c ; c = m_contacts.next() )
		c->moveToGroup( from, to );

	emit movedToGroup( this, from, to );
}

void KopeteMetaContact::removeFromGroup( KopeteGroup *group )
{
	if( !group || !m_groups.contains( group ) ||
		( !isTopLevel() && group == KopeteGroup::toplevel ) ||
		( isTemporary() && group == KopeteGroup::temporary ) )
	{
		return;
	}

	m_groups.remove( group );

	for( KopeteContact *c = m_contacts.first(); c ; c = m_contacts.next() )
		c->removeFromGroup( group );

	emit removedFromGroup( this, group );
}

void KopeteMetaContact::addToGroup( KopeteGroup *to )
{
	if(m_temporary && to!=KopeteGroup::temporary)
		return;

	if(!to ||  m_groups.contains( to ) || (to==KopeteGroup::toplevel && isTopLevel()))
	{
		return;
	}

	m_groups.append( to );

	for( KopeteContact *c = m_contacts.first(); c ; c = m_contacts.next() )
	{
		c->addToGroup( to );
	}

	emit addedToGroup( this, to );
}

KopeteGroupList KopeteMetaContact::groups() const
{
/*	if(m_groups.isEmpty())
		m_groups.append(KopeteGroup::toplevel);*/
	return m_groups;
}

void KopeteMetaContact::slotContactDestroyed( KopeteContact *contact )
{
	removeContact(contact,true);
}

QString KopeteMetaContact::toXML()
{
	emit aboutToSave(this);

//	kdDebug(14010) << "[KopeteMetaContact] toXML(), m_trackChildNameChanges=" << m_trackChildNameChanges << "." << endl;

	QString xml = "  <meta-contact>\n"
		"    <display-name trackChildNameChanges=\"" +
		QString::number(static_cast<int>(m_trackChildNameChanges)) +
		"\">" +
		QStyleSheet::escape( m_displayName ) +
		"</display-name>\n";

//	kdDebug(14010) << "[KopeteMetaContact] toXML(), xml=" << xml << "." << endl;

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
		   hidden contact, also for toplevel contacts saved before
		   we added the <top-level> tag.
		*/
		xml += "    <groups><top-level/></groups>\n";
	}

	// Store address book fields
	QMap<QString, QMap<QString, QString> >::ConstIterator appIt = m_addressBook.begin();
	for( ; appIt != m_addressBook.end(); ++appIt )
	{
		QMap<QString, QString>::ConstIterator addrIt = appIt.data().begin();
		for( ; addrIt != appIt.data().end(); ++addrIt )
		{
			xml += "    <address-book-field app=\"" + QStyleSheet::escape( appIt.key() ) +
				"\" key=\"" + QStyleSheet::escape( addrIt.key() ) + "\">" +
				QStyleSheet::escape( addrIt.data() ) + "</address-book-field>\n";
		}
	}

	// Store other plugin data
	if( !m_pluginData.isEmpty() )
	{
		QMap<QString, QMap<QString, QString> >::ConstIterator pluginIt;
		for( pluginIt = m_pluginData.begin(); pluginIt != m_pluginData.end(); ++pluginIt )
		{
			xml += "    <plugin-data plugin-id=\"" + QStyleSheet::escape( pluginIt.key() ) + "\">\n";

			QMap<QString, QString>::ConstIterator it;
			for( it = pluginIt.data().begin(); it != pluginIt.data().end(); ++it )
			{
				xml += "      <plugin-data-field key=\"" + QStyleSheet::escape( it.key() ) + "\">"
						+ QStyleSheet::escape( it.data() ) + "</plugin-data-field>\n";
			}

			xml += "    </plugin-data>\n";
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

				if( contactElement.attribute("trackChildNameChanges","1") == "1")
					m_trackChildNameChanges = true;
				else
					m_trackChildNameChanges = false;

//				m_trackChildNameChanges = false;
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
				QString app = contactElement.attribute( "app", QString::null );
				QString key = contactElement.attribute( "key", QString::null );
				QString val = contactElement.text();
				m_addressBook[ app ][ key ] = val;
			}
			else if( contactElement.tagName() == "plugin-data" )
			{
				QMap<QString, QString> pluginData;
				QString pluginId = contactElement.attribute( "plugin-id", QString::null );

				QDomNode field = contactElement.firstChild();
				while( !field.isNull() )
				{
					QDomElement fieldElement = field.toElement();
					if( fieldElement.tagName() == "plugin-data-field" )
						pluginData.insert( fieldElement.attribute( "key", "undefined-key" ), fieldElement.text() );

					field = field.nextSibling();
				}

				m_pluginData.insert( pluginId, pluginData );
			}
		}
		contactNode = contactNode.nextSibling();
	}

	// If a plugin is loaded, load data cached
	connect( LibraryLoader::pluginLoader(), SIGNAL( pluginLoaded(KopetePlugin*) ),
		this, SLOT( slotPluginLoaded(KopetePlugin*) ) );

	// track changes only works if ONE Contact is inside the MetaContact
//	if (m_contacts.count() > 1) // Does NOT work as intended
		m_trackChildNameChanges=false;

//	kdDebug(14010) << "[KopeteMetaContact] END fromXML(), m_trackChildNameChanges=" << m_trackChildNameChanges << "." << endl;
	return true;
}

QString KopeteMetaContact::addressBookField( KopetePlugin * /* p */, const QString &app, const QString & key ) const
{
	return m_addressBook[ app ][ key ];
}

void KopeteMetaContact::setAddressBookField( KopetePlugin * /* p */, const QString &app, const QString &key, const QString &value )
{
	m_addressBook[ app ][ key ] = value;
}

bool KopeteMetaContact::isTemporary() const
{
	return m_temporary;
}

void KopeteMetaContact::setTemporary( bool isTemporary, KopeteGroup *group )
{
	m_temporary = isTemporary;
	KopeteGroup *temporaryGroup = KopeteGroup::temporary;
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
		moveToGroup(temporaryGroup, group);
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

	QMap<QString, QMap<QString, QString> >::ConstIterator it;
	for( it = m_pluginData.begin(); it != m_pluginData.end(); ++it )
	{
		//kdDebug( 14010 ) << "key: " << it.key() << ", plugin id: " << p->pluginId() << endl;
		if( p->pluginId() == it.key() )
			p->deserialize( this, it.data() );
	}
}

void KopeteMetaContact::setPluginData( KopetePlugin *p, const QMap<QString, QString> &pluginData )
{
	if( pluginData.isEmpty() )
	{
		m_pluginData.remove( p->pluginId() );
		return;
	}

	m_pluginData[ p->pluginId() ] = pluginData;
}

void KopeteMetaContact::setPluginData( KopetePlugin *p, const QString &key, const QString &value )
{
	m_pluginData[ p->pluginId() ][ key ] = value;
}

QMap<QString, QString> KopeteMetaContact::pluginData( KopetePlugin *p ) const
{
	if( !m_pluginData.contains( p->pluginId() ) )
		return QMap<QString, QString>();

	return m_pluginData[ p->pluginId() ];
}

QString KopeteMetaContact::pluginData( KopetePlugin *p, const QString &key ) const
{
	if( !m_pluginData.contains( p->pluginId() ) || !m_pluginData[ p->pluginId() ].contains( key ) )
		return QString::null;

	return m_pluginData[ p->pluginId() ][ key ];
}

KopeteMetaContact::IdleState KopeteMetaContact::idleState() const
{
	return m_idleState;
}

void KopeteMetaContact::slotContactIdleStateChanged( KopeteContact *c, KopeteContact::IdleState s )
{
	emit contactIdleStateChanged(c,s);
	updateIdleState();
}

#include "kopetemetacontact.moc"

// vim: set noet ts=4 sts=4 sw=4:


