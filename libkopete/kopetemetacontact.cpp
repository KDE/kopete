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

#include "kopete.h"
#include "kopetecontactlist.h"
#include "kopetemetacontactlvi.h"

#include <klocale.h>
#include <kdebug.h>

// FIXME: Add parent!!
KopeteMetaContact::KopeteMetaContact()
: QObject( KopeteContactList::contactList() )
{
	m_trackChildNameChanges = true;
}

KopeteMetaContact::~KopeteMetaContact()
{
}

void KopeteMetaContact::addContact( KopeteContact *c, const QStringList &groups )
{
	bool isUnknown = false;

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

		connect( c, SIGNAL( nameChanged( const QString & ) ),
			this, SLOT( slotContactNameChanged( const QString & ) ) );

		connect( c, SIGNAL( destroyed( QObject * ) ),
			this, SLOT( slotMetaContactDestroyed( QObject * ) ) );

		 if (displayName() == "")
			 setDisplayName( c->name() );

		// FIXME: Group handling!!!!
		// Generally, if the groups are explicitly set by the user they
		// should not be overridden. Until then manage them automatically.
		// For now just assume an empty group list means override
		if( m_groups.isEmpty() )
		{
			m_groups = groups;
			for( QStringList::ConstIterator it = groups.begin();
				it != groups.end(); ++it )
			{
				QString group = *it;

				if( group.isEmpty() )
				{
					// already added to the unknown group
					if( isUnknown )
						continue;
					group = i18n( "Unknown" );
					isUnknown = true;
				}
				kdDebug() << "KopeteMetaContact::addContact: adding " << c->id()
					<< " to group " << group << endl;

				QListViewItem *groupLVI =
					kopeteapp->contactList()->getGroup( group );

				// If the group doesn't exist: create it first
				if( !groupLVI )
				{
					kopeteapp->contactList()->addGroup( group );
					groupLVI = kopeteapp->contactList()->getGroup( group );
				}

				kopeteapp->contactList()->addContact(
					new KopeteMetaContactLVI( this, groupLVI ) );
			}
		}
	}
}

KopeteContact *KopeteMetaContact::findContact( const QString &contactId )
{
	QPtrListIterator<KopeteContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		if( it.current()->id() == contactId )
			return it.current();
	}

	// Contact not found
	return 0L;
}

KopeteContact *KopeteMetaContact::findContact( const QString &protocolId, const QString &contactId )
{
	QPtrListIterator<KopeteContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		if( (it.current()->id() == contactId ) && (it.current()->protocol() == protocolId ))
			return it.current();
	}

	// Contact not found
	return 0L;
}

void KopeteMetaContact::addData( QString &pluginId, QString &key, QString &value )
{
	//m_metadata.insert( QPair( pluginId, key) , value);
	m_metadata[ qMakePair( pluginId, key) ] = value;
}

QString KopeteMetaContact::data( QString &pluginId, QString &key)
{
	return m_metadata[ qMakePair( pluginId, key) ];
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
	if( !m_contacts.isEmpty() )
		m_contacts.first()->execute();
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
	bool awayFound = false;

	QPtrListIterator<KopeteContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		KopeteContact::ContactStatus s = it.current()->status();

		if ( s == KopeteContact::Online )
			return Online;
		else if ( s == KopeteContact::Away )
			awayFound = true;
	}
/*
	it.toFirst();
	for( ; it.current(); ++it )
	{
		if( it.current()->status() == KopeteContact::Away )
			return Away;
	}
*/
	if ( awayFound )
		return Away;
	else
		return Offline;
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
	kdDebug() << "KopeteMetaContact::slotContactStatusChanged" << endl;
	emit onlineStatusChanged( this, status() );
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
	if( m_groups.contains( to ) )
		return;

	m_groups.remove( from );
	m_groups.append( to );

	QPtrListIterator<KopeteContact> it( m_contacts );
	for( ; it.current(); ++it )
		it.current()->moveToGroup( from, to );

	emit movedToGroup( from, to );
}

void KopeteMetaContact::addToGroup( const QString &to )
{
	if( m_groups.contains( to ) )
		return;

	m_groups.append( to );

    /*
	QPtrListIterator<KopeteContact> it( m_contacts );
	for( ; it.current(); ++it )
		it.current()->addToGroup( to );
    */

	emit addedToGroup( to );
}

QStringList KopeteMetaContact::groups() const
{
	return m_groups;
}

void KopeteMetaContact::slotMetaContactDestroyed( QObject *obj )
{
	// Try removing the item. The contact might be removed already, but
	// that's safely handled inside QPtrList::remove()
	m_contacts.remove( dynamic_cast<KopeteContact *>( obj ) );
}

QString KopeteMetaContact::toXML()
{
	QString xml;

	xml = "<person";

	if ( ! m_displayName.isNull() )
	{
		xml = xml + " name=\"" + m_displayName + "\"";
    }

	xml = xml + ">\n";

	/* We include all contacts in the XML */
	QPtrListIterator<KopeteContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		    xml = xml + "\t" + (it.current())->toXML() + "\n";
	}

	// FIXME: KopeteMetaContact::toXML() Metadata is ignored

	/* We include all groups in the XML */
	for( QStringList::ConstIterator it = m_groups.begin(); it != m_groups.end(); ++it )
	{
		QString group = *it;
		xml = xml + "\t<group name=\"" + group + "\"/>\n";                          
	}

	xml = xml + "</person>";
	return xml;
}


#include "kopetemetacontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

