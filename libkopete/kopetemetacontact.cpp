/*
    kopetemetacontact.cpp - Kopete Meta Contact

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

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
#include <qstylesheet.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knotifyclient.h>

#include "kopetecontactlist.h"
#include "kopeteonlinestatus.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "pluginloader.h"


#define EMAIL_WINDOW 0
#define CHAT_WINDOW 1

struct KopeteMetaContactPrivate
{
	QPtrList<KopeteContact> contacts;

	QString displayName;

	bool trackChildNameChanges;

	KopeteGroupList groups;

	QMap<QString, QMap<QString, QString> > addressBook;

	bool temporary;
	bool dirty;

	KopeteMetaContact::OnlineStatus onlineStatus;
	KopeteMetaContact::IdleState    idleState;
};

KopeteMetaContact::KopeteMetaContact()
: KopetePluginDataObject( KopeteContactList::contactList() )
{
	d = new KopeteMetaContactPrivate;

	d->trackChildNameChanges = true;
	d->temporary = false;
//	m_isTopLevel=false;

	d->onlineStatus = Unknown;
	d->idleState = Unspecified;
}

KopeteMetaContact::~KopeteMetaContact()
{
	delete d;
}

void KopeteMetaContact::addContact( KopeteContact *c )
{
	if( d->contacts.contains( c ) )
	{
		kdWarning(14010) << "Ignoring attempt to add duplicate contact " << c->contactId() << "!" << endl;
	}
	else
	{
		d->contacts.append( c );

		connect( c, SIGNAL( onlineStatusChanged( KopeteContact *, const KopeteOnlineStatus & ) ),
			SLOT( slotContactStatusChanged( KopeteContact *, const KopeteOnlineStatus & ) ) );

		connect( c, SIGNAL( displayNameChanged( const QString &,const QString & ) ),
			this, SLOT( slotContactNameChanged( const QString &,const QString & ) ) );

		connect( c, SIGNAL( contactDestroyed( KopeteContact * ) ),
			this, SLOT( slotContactDestroyed( KopeteContact * ) ) );

		connect( c, SIGNAL( idleStateChanged( KopeteContact *, KopeteContact::IdleState ) ),
			this, SLOT( slotContactIdleStateChanged( KopeteContact *, KopeteContact::IdleState ) ) );

		if( d->displayName.isNull() )
		{
			setDisplayName( c->displayName() );
			d->trackChildNameChanges = true;
		}

		if( d->contacts.count() > 1 )
		{
//			kdDebug(14010) << "[KopeteMetaContact] addContact(); disabling trackChildNameChanges,"
//			" more than ONE Contact in MetaContact" << endl;
			d->trackChildNameChanges = false;
		}

		/* for( QStringList::ConstIterator it = groups.begin(); it != groups.end(); ++it )
			addToGroup(*it); */
		emit contactAdded(c);
	}

	updateOnlineStatus();
}

void KopeteMetaContact::updateOnlineStatus()
{
	OnlineStatus newStatus = Unknown;

	QPtrListIterator<KopeteContact> it( d->contacts );
	for( ; it.current(); ++it )
	{
		KopeteOnlineStatus status = it.current()->onlineStatus();

		if( status.status() == KopeteOnlineStatus::Online )
		{
			newStatus = Online;
			break;
		}

		else if( ( status.status() == KopeteOnlineStatus::Away ) && ( newStatus != Online ) )
		{
			// Set status, but don't stop searching, since 'Online' overrules
			// 'Away'
			newStatus = Away;
		}
		else if( ( status.status() == KopeteOnlineStatus::Offline ) && ( newStatus != Away ) && ( newStatus != Online ) )
		{
			newStatus = Offline;
		}
	}

	if( newStatus != d->onlineStatus )
	{
		d->onlineStatus = newStatus;
		emit onlineStatusChanged( this, d->onlineStatus );
	}
}

void KopeteMetaContact::updateIdleState()
{
	IdleState newStatus = Unspecified;

	QPtrListIterator<KopeteContact> it( d->contacts );
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

	if( newStatus != d->idleState )
	{
		d->idleState = newStatus;
		emit idleStateChanged( this, d->idleState );
	}
}

void KopeteMetaContact::removeContact(KopeteContact *c, bool deleted)
{
	if( !d->contacts.contains( c ) )
	{
		kdDebug(14010) << "KopeteMetaContact::removeContact: Contact is not in this metaContact " << endl;
	}
	else
	{
		d->contacts.remove( c );

		if(!deleted)
		{  //If this function is tell by slotContactRemoved, c is maybe just a QObject
			disconnect( c, SIGNAL( onlineStatusChanged( KopeteContact *, const KopeteOnlineStatus & ) ),
				this, SLOT( slotContactStatusChanged( KopeteContact *, const KopeteOnlineStatus & ) ) );

			disconnect( c, SIGNAL( displayNameChanged( const QString &,const QString & ) ),
				this, SLOT( slotContactNameChanged( const QString &,const QString & ) ) );

			disconnect( c, SIGNAL( contactDestroyed( KopeteContact * ) ),
				this, SLOT( slotContactDestroyed( KopeteContact * ) ) );

			disconnect( c, SIGNAL( idleStateChanged( KopeteContact *, KopeteContact::IdleState ) ),
				this, SLOT( slotContactIdleStateChanged( KopeteContact *, KopeteContact::IdleState ) ) );

			kdDebug( 14010 ) << k_funcinfo << "Contact disconected" << endl;
		}
		emit contactRemoved( c );
	}
	updateOnlineStatus();
}

bool KopeteMetaContact::isTopLevel()
{
	if( d->groups.isEmpty() )
		d->groups.append( KopeteGroup::toplevel );
	return( d->groups.contains( KopeteGroup::toplevel ) );
}

void KopeteMetaContact::setTopLevel( bool b )
{
	if( b )
	{
		if( !isTopLevel() )
			d->groups.append( KopeteGroup::toplevel );
	}
	else
	{
		d->groups.remove( KopeteGroup::toplevel );
	}
}

KopeteContact *KopeteMetaContact::findContact( const QString &protocolId,
	const QString &accountId, const QString &contactId )
{
	//kdDebug(14010) << "*** Num contacts: " << d->contacts.count() << endl;
	QPtrListIterator<KopeteContact> it( d->contacts );
	for( ; it.current(); ++it )
	{
		//kdDebug(14010) << "*** Trying " << it.current()->contactId() << ", proto "
		//<< it.current()->protocol()->pluginId() << ", account " << it.current()->accountId() << endl;
		if( ( it.current()->contactId() == contactId ) && ( it.current()->protocol()->pluginId() == protocolId ) )
		{
			if ( accountId.isEmpty() )
				return it.current();
		
			if(it.current()->account())
			{
				if(it.current()->account()->accountId() == accountId)
					return it.current();
			}
		}
	}

	// Contact not found
	return 0L;
}

void KopeteMetaContact::sendMessage()
{
	KopeteContact *c = preferredContact();

	if( !c )
	{
		KMessageBox::error( qApp->mainWidget(),
			i18n( "This user is not reachable at the moment. Please"
				"try a protocol that supports offline sending, or wait "
				"until this user goes online." ),
			i18n( "User is not reachable - Kopete" ) );
	}
	else
	{
		c->sendMessage();
	}

}

void KopeteMetaContact::startChat()
{
	KopeteContact *c = preferredContact();

	if( !c )
	{
		KMessageBox::error( qApp->mainWidget(),
			i18n( "This user is not reachable at the moment. Please"
				"try a protocol that supports offline sending, or wait "
				"until this user goes online." ),
			i18n( "User is not reachable - Kopete" ) );
	}
	else
	{
		c->startChat();
	}
}

KopeteContact *KopeteMetaContact::preferredContact()
{
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

	KopeteContact *c = 0L;

	for( QPtrListIterator<KopeteContact> it( d->contacts ) ; it.current(); ++it )
	{
		if( ( *it )->isReachable() && ( !c || ( *it )->onlineStatus() > c->onlineStatus() ) )
			c = *it;
	}

	return c;
}

void KopeteMetaContact::execute()
{
	switch( KopetePrefs::prefs()->interfacePreference() )
	{
		case EMAIL_WINDOW:
			sendMessage();
			break;

		case CHAT_WINDOW:
		default:
			startChat();
			break;
	}
}

QString KopeteMetaContact::statusIcon() const
{
	switch( status() )
	{
		case Online:
			return QString::fromLatin1( "metacontact_online" );
		case Away:
			return QString::fromLatin1( "metacontact_away" );
		case Unknown:
			return QString::fromLatin1( "metacontact_unknown" );
		case Offline:
		default:
			return QString::fromLatin1( "metacontact_offline" );
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
	return d->onlineStatus;
}

bool KopeteMetaContact::isOnline() const
{
	QPtrListIterator<KopeteContact> it( d->contacts );
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

	QPtrListIterator<KopeteContact> it( d->contacts );
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

	QPtrListIterator<KopeteContact> it( d->contacts );
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
	if( d->contacts.isEmpty() || !canAcceptFiles() )
		return;

	//Find the highest ranked protocol that can accept files
	KopeteContact *c = d->contacts.first();
	for( QPtrListIterator<KopeteContact> it( d->contacts ) ; it.current(); ++it )
	{
		if( ( *it )->onlineStatus() > c->onlineStatus() && ( *it )->canAcceptFiles() )
			c = *it;
	}

	//Call the sendFile slot of this protocol
	c->sendFile( sourceURL, altFileName, fileSize );
}

void KopeteMetaContact::slotContactStatusChanged( KopeteContact * c, const KopeteOnlineStatus &status  )
{
	updateOnlineStatus();

	emit contactStatusChanged( c, status );
}

void KopeteMetaContact::setDisplayName( const QString &name )
{
//	kdDebug( 14000 ) << k_funcinfo << "Change displayName from " << d->displayName <<
//		" to " << name  << ", d->trackChildNameChanges=" << d->trackChildNameChanges << endl;

	if( name == d->displayName )
		return;

	emit displayNameChanged( d->displayName, name );

	d->displayName = name;

	//The name is setted by the user, disable tracking
	d->trackChildNameChanges = false;

	// Don't rename contacts on the server automagically as not everyone seems
	// to like it. We need a GUI for this first, but that's post-0.6 work.
	// After Kopete 0.6 is out we can think about the required options and/or
	// heuristics for when to sync to or from the server (and in what way)
	// - Martijn
#if 0
	for( KopeteContact *c = d->contacts.first(); c ; c = d->contacts.next() )
		c->rename( name );
#endif
}

QString KopeteMetaContact::displayName() const
{
	return d->displayName;
}

bool KopeteMetaContact::trackChildNameChanges() const
{
	return d->trackChildNameChanges;
}

void KopeteMetaContact::setTrackChildNameChanges( bool /* track */ )
{
/*
	if (track && (d->contacts.count() == 1))
	{
		kdDebug(14010) << "[KopeteMetaContact] setTrackChildNameChanges(); ENABLING TrackChildNameChanges" << endl;
		setDisplayName( (d->contacts.first())->displayName() );
		d->trackChildNameChanges = true;
	}
	else
	{
		kdDebug(14010) << "[KopeteMetaContact] setTrackChildNameChanges(); DISABLING TrackChildNameChanges" << endl;
		d->trackChildNameChanges = false;
	}
*/
}

void KopeteMetaContact::slotContactNameChanged( const QString &/*oldName*/, const QString &newName )
{
//	kdDebug(14010) << "[KopeteMetaContact] slotContactNameChanged(); name=" << name <<
//		", d->trackChildNameChanges=" << d->trackChildNameChanges << "." << endl;

	if( d->trackChildNameChanges || d->displayName.isEmpty() )
	{
		setDisplayName( newName );
		//because d->trackChildNameChanges is set to false in setDisplayName
		d->trackChildNameChanges = true;
	}
	else
	{
//		kdDebug( 14010 ) << k_funcinfo << "Tracking is off and we already have a display name, ignored KopeteContact name change" << endl;
	}
}

void KopeteMetaContact::moveToGroup( KopeteGroup *from, KopeteGroup *to )
{
	if( isTemporary() && to != KopeteGroup::temporary )
		return;

	if( !from || !d->groups.contains( from ) ||
		( !isTopLevel() && from == KopeteGroup::toplevel ) )
	{
		// We're adding, not moving, because 'from' is illegal
		addToGroup( to );
		return;
	}

	if( !to || d->groups.contains( to ) ||
		( isTopLevel() && to == KopeteGroup::toplevel ) )
	{
		// We're removing, not moving, because 'to' is illegal
		removeFromGroup( from );
		return;
	}

	kdDebug( 14010 ) << k_funcinfo << from->displayName() << " => " << to->displayName() << endl;

	d->groups.remove( from );
	d->groups.append( to );

	for( KopeteContact *c = d->contacts.first(); c ; c = d->contacts.next() )
		c->moveToGroup( from, to );

	emit movedToGroup( this, from, to );
}

void KopeteMetaContact::removeFromGroup( KopeteGroup *group )
{
	if( !group || !d->groups.contains( group ) ||
		( !isTopLevel() && group == KopeteGroup::toplevel ) ||
		( isTemporary() && group == KopeteGroup::temporary ) )
	{
		return;
	}

	d->groups.remove( group );

	for( KopeteContact *c = d->contacts.first(); c ; c = d->contacts.next() )
		c->removeFromGroup( group );

	emit removedFromGroup( this, group );
}

void KopeteMetaContact::addToGroup( KopeteGroup *to )
{
	if( d->temporary && to != KopeteGroup::temporary )
		return;

	if( !to || d->groups.contains( to ) || ( to == KopeteGroup::toplevel && isTopLevel() ) )
	{
		return;
	}

	d->groups.append( to );

	for( KopeteContact *c = d->contacts.first(); c ; c = d->contacts.next() )
	{
		c->addToGroup( to );
	}

	emit addedToGroup( this, to );
}

KopeteGroupList KopeteMetaContact::groups() const
{
/*	if(d->groups.isEmpty())
		d->groups.append(KopeteGroup::toplevel);*/
	return d->groups;
}

void KopeteMetaContact::slotContactDestroyed( KopeteContact *contact )
{
	removeContact(contact,true);
}

QString KopeteMetaContact::toXML()
{
	emit aboutToSave(this);

//	kdDebug(14010) << "[KopeteMetaContact] toXML(), d->trackChildNameChanges=" << d->trackChildNameChanges << "." << endl;

	QString xml = QString::fromLatin1( "  <meta-contact>\n    <display-name trackChildNameChanges=\"" ) +
		QString::number( static_cast<int>( d->trackChildNameChanges ) ) +
		QString::fromLatin1( "\">" ) + QStyleSheet::escape( d->displayName ) +
		QString::fromLatin1( "</display-name>\n" );

//	kdDebug(14010) << "[KopeteMetaContact] toXML(), xml=" << xml << "." << endl;

	// Store groups
	if ( !d->groups.isEmpty() )
	{
		xml += QString::fromLatin1( "    <groups>\n" );
		KopeteGroup *g;;
		for ( g = d->groups.first(); g; g = d->groups.next() )
		{
			QString group_s=g->displayName();
			if(!group_s.isNull())
			{
				if ( !group_s.isEmpty() )
					xml += QString::fromLatin1( "      <group>" ) + QStyleSheet::escape( group_s ) + QString::fromLatin1( "</group>\n" );
				else
					xml += QString::fromLatin1( "      <group>" ) + QStyleSheet::escape( i18n("Unknown" ) ) + QString::fromLatin1( "</group>\n" );
			}
		}

		// The contact is also at top-level
		if ( isTopLevel() )
		{
			xml += QString::fromLatin1( "      <top-level/>\n" );
		}

		xml += QString::fromLatin1( "    </groups>\n" );
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
		xml += QString::fromLatin1( "    <groups><top-level/></groups>\n" );
	}

	// Store address book fields
	QMap<QString, QMap<QString, QString> >::ConstIterator appIt = d->addressBook.begin();
	for( ; appIt != d->addressBook.end(); ++appIt )
	{
		QMap<QString, QString>::ConstIterator addrIt = appIt.data().begin();
		for( ; addrIt != appIt.data().end(); ++addrIt )
		{
			xml += QString::fromLatin1( "    <address-book-field app=\"" ) + QStyleSheet::escape( appIt.key() ) +
				QString::fromLatin1( "\" key=\"" ) + QStyleSheet::escape( addrIt.key() ) + QString::fromLatin1( "\">" ) +
				QStyleSheet::escape( addrIt.data() ) + QString::fromLatin1( "</address-book-field>\n" );
		}
	}

	// Store other plugin data
	xml += KopetePluginDataObject::toXML();

	xml += QString::fromLatin1( "  </meta-contact>\n" );

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
			if( contactElement.tagName() == QString::fromLatin1( "display-name" ) )
			{
				if ( contactElement.text().isEmpty() )
					return false;
				d->displayName = contactElement.text();

				//TODO: d->trackChildNameChanges is currently used only when contact creation
				//later, we will add a GUI to make it configurable

				/*d->trackChildNameChanges =
					( contactElement.attribute( QString::fromLatin1( "trackChildNameChanges" ),
					QString::fromLatin1( "1" ) ) == QString::fromLatin1( "1" ) );*/
				d->trackChildNameChanges = false;
			}
			else if( contactElement.tagName() == QString::fromLatin1( "groups" ) )
			{
				QDomNode group = contactElement.firstChild();
				while( !group.isNull() )
				{
					QDomElement groupElement = group.toElement();

					if( groupElement.tagName() == QString::fromLatin1( "group" ) )
						d->groups.append( KopeteContactList::contactList()->getGroup( groupElement.text() ) );
					else if( groupElement.tagName() == QString::fromLatin1( "top-level" ) )
						d->groups.append( KopeteGroup::toplevel );

					group = group.nextSibling();
				}
			}

			else if( contactElement.tagName() == QString::fromLatin1( "address-book-field" ) )
			{
				QString app = contactElement.attribute( QString::fromLatin1( "app" ), QString::null );
				QString key = contactElement.attribute( QString::fromLatin1( "key" ), QString::null );
				QString val = contactElement.text();
				d->addressBook[ app ][ key ] = val;
			}
			else if( contactElement.tagName() == QString::fromLatin1( "plugin-data" ) )
			{
				KopetePluginDataObject::fromXML(contactElement);
			}
		}
		contactNode = contactNode.nextSibling();
	}

	// If a plugin is loaded, load data cached
	connect( LibraryLoader::pluginLoader(), SIGNAL( pluginLoaded(KopetePlugin*) ),
		this, SLOT( slotPluginLoaded(KopetePlugin*) ) );

	// track changes only works if ONE Contact is inside the MetaContact
//	if (d->contacts.count() > 1) // Does NOT work as intended
		d->trackChildNameChanges=false;

//	kdDebug(14010) << "[KopeteMetaContact] END fromXML(), d->trackChildNameChanges=" << d->trackChildNameChanges << "." << endl;
	return true;
}

QString KopeteMetaContact::addressBookField( KopetePlugin * /* p */, const QString &app, const QString & key ) const
{
	return d->addressBook[ app ][ key ];
}

void KopeteMetaContact::setAddressBookField( KopetePlugin * /* p */, const QString &app, const QString &key, const QString &value )
{
	d->addressBook[ app ][ key ] = value;
}

bool KopeteMetaContact::isTemporary() const
{
	return d->temporary;
}

void KopeteMetaContact::setTemporary( bool isTemporary, KopeteGroup *group )
{
	d->temporary = isTemporary;
	KopeteGroup *temporaryGroup = KopeteGroup::temporary;
	if( d->temporary )
	{
		addToGroup (temporaryGroup);
		KopeteGroup *g;
		for( g = d->groups.first(); g; g = d->groups.next() )
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
	return d->dirty;
}

void KopeteMetaContact::setDirty( bool b  )
{
	d->dirty = b;
}

void KopeteMetaContact::slotPluginLoaded( KopetePlugin *p )
{
	if( !p )
		return;

	QMap<QString, QString> map= pluginData( p );
	if(!map.isEmpty())
	{
		p->deserialize(this,map);
	}
}

KopeteMetaContact::IdleState KopeteMetaContact::idleState() const
{
	return d->idleState;
}

void KopeteMetaContact::slotContactIdleStateChanged( KopeteContact *c, KopeteContact::IdleState s )
{
	emit contactIdleStateChanged(c,s);
	updateIdleState();
}

QPtrList<KopeteContact> KopeteMetaContact::contacts() const
{
	return d->contacts;
}

#include "kopetemetacontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

