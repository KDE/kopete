/*
    gwcontact.cpp - Kopete GroupWise Protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed   
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kopeteaccount.h"
#include "kopeteglobal.h"
#include "kopetemessage.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"

#include "tasks/deletecontacttask.h"

#include "client.h"
#include "gwaccount.h"
#include "ui/gwcontactproperties.h"
#include "gwerror.h"
#include "gwfakeserver.h"
#include "gwmessagemanager.h"
#include "gwprotocol.h"
#include "userdetailsmanager.h"

#include "gwcontact.h"

using namespace GroupWise;

GroupWiseContact::GroupWiseContact( KopeteAccount* account, const QString &dn, 
			KopeteMetaContact *parent, 
			const int objectId, const int parentId, const int sequence )
: KopeteContact( account, dn, parent ), m_objectId( objectId ), m_parentId( parentId ),
  m_sequence( sequence )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " dn: " << dn << endl;
	setOnlineStatus( ( parent && parent->isTemporary() ) ? protocol()->groupwiseUnknown : protocol()->groupwiseOffline );
}

GroupWiseContact::~GroupWiseContact()
{
	// This is necessary because otherwise the userDetailsManager 
	// would not fetch details for this contact if they contact you
	// again from off-contact-list.
	if ( metaContact()->isTemporary() )
		account()->client()->userDetailsManager()->removeContact( contactId() );
}

void GroupWiseContact::updateDetails( const ContactDetails & details )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	if ( !details.cn.isNull() )
		setProperty( protocol()->propCN, details.cn );
	if ( !details.givenName.isNull() )
		setProperty( protocol()->propGivenName, details.givenName );
	if ( !details.surname.isNull() )
		setProperty( protocol()->propLastName, details.surname );
	if ( !details.fullName.isNull() )
		setProperty( protocol()->propFullName, details.fullName );
	//if ( !details.awayMessage.isNull() )
		//setProperty( protocol()->propAwayMessage, details.awayMessage );
	
	m_serverProperties = details.properties;
	
	if ( details.status != GroupWise::Invalid )
	{	
		KopeteOnlineStatus status = protocol()->gwStatusToKOS( details.status );
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "setting initial status to " << status.description() << endl;
		setOnlineStatus( status );
	}
	else 
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "initial status is, not setting " << details.status << endl; 
}

GroupWiseProtocol *GroupWiseContact::protocol()
{
	return static_cast<GroupWiseProtocol *>( KopeteContact::protocol() );
}

GroupWiseAccount *GroupWiseContact::account()
{
	return static_cast<GroupWiseAccount *>( KopeteContact::account() );
}

bool GroupWiseContact::isReachable()
{
	// When we are invisible we can't start a chat with others, but we don't make isReachable return false, because then we
	// don't get any notification when we click on someone in the contact list.  Instead we warn the user when they try to send a message,
	// in GWMessageManager
	// (This is a GroupWise rule, not a problem in Kopete)

	if ( account()->isConnected() && isOnline()/* && account()->myself()->onlineStatus() != protocol()->groupwiseAppearOffline */)
		return true;
	if ( !account()->isConnected()/* || account()->myself()->onlineStatus() == protocol()->groupwiseAppearOffline*/ )
		return false;
}

void GroupWiseContact::serialize( QMap< QString, QString > &serializedData, QMap< QString, QString > & /* addressBookData */ )
{
/*    QString value;
	switch ( m_type )
	{
	case Null:
		value = "null";
	case Echo:
		value = "echo";
	}
	serializedData[ "contactType" ] = value;*/
}

KopeteMessageManager * GroupWiseContact::manager( bool canCreate )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "called, canCreate: " << canCreate << endl;

	KopeteContactPtrList chatMembers;
	chatMembers.append( this );

	return manager( chatMembers, canCreate );

/*	if ( m_msgManager )
	{
		return m_msgManager;
	}
	else
	{
		QPtrList<KopeteContact> contacts;
		contacts.append(this);
		m_msgManager = KopeteMessageManagerFactory::factory()->create(account()->myself(), contacts, protocol());
		connect(m_msgManager, SIGNAL(messageSent(KopeteMessage&, KopeteMessageManager*)),
				this, SLOT( sendMessage( KopeteMessage& ) ) );
		connect(m_msgManager, SIGNAL(destroyed()), this, SLOT(slotMessageManagerDestroyed()));
		return m_msgManager;
	}*/
}

GroupWiseMessageManager * GroupWiseContact::manager( KopeteContactPtrList chatMembers, bool canCreate )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	
	KopeteMessageManager *_manager = KopeteMessageManagerFactory::factory()->findKopeteMessageManager ( account()->myself(), chatMembers, protocol() );
	GroupWiseMessageManager *mgr = dynamic_cast<GroupWiseMessageManager*>( _manager );

	/*
	 * If we didn't find a message manager for this contact,
	 * instantiate a new one if we are allowed to. (otherwise return 0)
	 */
	if ( !mgr && canCreate )
	{
		mgr = account()->messageManager( account()->myself(), chatMembers, protocol(), QString::null );
		connect( mgr, SIGNAL( destroyed ( QObject * ) ), SLOT( slotMessageManagerDeleted ( QObject * ) ) );
		connect( mgr, SIGNAL( conferenceCreated() ), SLOT( slotConferenceCreated() ) );
		//m_pendingManagers.append( mgr );
	
	}

	return mgr;
}

GroupWiseMessageManager * GroupWiseContact::manager( const QString & guid, bool canCreate )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "called for guid: " << guid << ", canCreate: " << canCreate << endl;
	if ( !guid.isNull() )
	{
		GroupWiseMessageManager * mgr = m_msgManagers[ guid ];
		if ( !mgr )
		{
			// we've received the first message from a new conference.
			// create a message manager with the given guid
			kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " - this GroupWiseMessageManager is new to us. " << endl;
			KopeteContactPtrList chatMembers;
			chatMembers.append ( this );
			mgr = account()->messageManager( account()->myself(), chatMembers, protocol(), guid );
			connect( mgr, SIGNAL( destroyed( QObject * ) ), this, SLOT( slotMessageManagerDeleted ( QObject * ) ) );
			m_msgManagers.insert( guid, mgr );
		}
		else
			kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " - found an existing GroupWiseMessageManager for this guid. " << endl;
		
		return mgr;
	}
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "GUID is empty, grabbing first available manager." << endl;
	/*
	 * The guid supplied is empty, so just return the first available manager.
	 */
	return dynamic_cast<GroupWiseMessageManager *>( manager( canCreate ) );
}

QPtrList<KAction> *GroupWiseContact::customContextMenuActions() //OBSOLETE
{
	//FIXME!!!  this function is obsolete, we should use XMLGUI instead
	/*m_actionCollection = new KActionCollection( this, "userColl" );
	m_actionPrefs = new KAction(i18n( "&Contact Settings" ), 0, this,
			SLOT( showContactSettings( )), m_actionCollection, "contactSettings" );

	return m_actionCollection;*/
	return 0L;
}

void GroupWiseContact::slotUserInfo()
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	new GroupWiseContactProperties( this, account(), "gwcontactproperties" );
}

QMap< QString, QString > GroupWiseContact::serverProperties()
{
	return m_serverProperties;
}

void GroupWiseContact::sendMessage( KopeteMessage &message )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	// convert to the what the server wants
	// For this 'protocol', there's nothing to do
	// send it
/*	static_cast<GroupWiseAccount *>( account() )->server()->sendMessage(
			message.to().first()->contactId(),
			message.plainBody() );*/
	// give it back to the manager to display
	manager()->appendMessage( message );
	// tell the manager it was sent successfully
	manager()->messageSucceeded();
}

void GroupWiseContact::dumpManagers()
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " for: " << contactId() << endl;
	QDictIterator< GroupWiseMessageManager > it( m_msgManagers );
	for ( ; it.current(); ++it )
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "guid: " << it.currentKey() << endl;
}

void GroupWiseContact::slotConferenceCreated()
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	const GroupWiseMessageManager * mgr = (GroupWiseMessageManager *)sender();
	m_msgManagers.insert( mgr->guid(), mgr );
	dumpManagers();
}

void GroupWiseContact::slotMessageManagerDeleted( QObject *sender )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "Message manager deleted, collecting the pieces..." << endl;

	GroupWiseMessageManager *manager = static_cast<GroupWiseMessageManager *>(sender);

	m_msgManagers.remove( manager->guid() );
}

void GroupWiseContact::handleIncomingMessage( const ConferenceEvent & message, bool autoReply )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "Got a " << ( autoReply ? "auto-reply" : "message" ) << " for conference: " << message.guid << ", message: " << message.message << endl;
	KopeteContactPtrList contactList;
	contactList.append ( account()->myself () );
	GroupWiseMessageManager *mgr = manager( message.guid, true );

	// add an auto-reply indicator if needed
	QString messageMunged = message.message;
	if ( autoReply )
	{
		QString autoReplyPrefix = i18n("Prefix used for automatically generated auto-reply messages when the contact is Away, contains contact's name",
									   "Auto reply from %1: " ).arg( property( Kopete::Global::Properties::self()->nickName() ).value().toString() );
		messageMunged = autoReplyPrefix + message.message;
	}
	KopeteMessage * newMessage = new KopeteMessage ( message.timeStamp, this, contactList, messageMunged, 
									KopeteMessage::Inbound,
									autoReply ? KopeteMessage::PlainText : KopeteMessage::RichText );
	Q_ASSERT( mgr );
	mgr->appendMessage( *newMessage );
	delete newMessage;
}

void GroupWiseContact::joinConference( const QString & guid )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	manager( guid, false );
}

void GroupWiseContact::leaveConference( const QString & guid )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "NOT IMPLEMENTED" << endl;
}

void GroupWiseContact::addCLInstance( const ContactListInstance & instance )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	m_instances.append( instance );
}

void GroupWiseContact::removeCLInstance( const int objectId )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	QValueListIterator< ContactListInstance > it = m_instances.begin();
	const QValueListIterator< ContactListInstance > end = m_instances.end();
	for ( ; it != end; ++it )
	{
		if ( (*it).objectId == objectId )
		{
			kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "removed objectId: " << objectId << endl;
			m_instances.remove( it );
			break;
		}
	}
}

bool GroupWiseContact::hasCLObjectId( const int objectId ) const
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	QValueListConstIterator< ContactListInstance > it = m_instances.begin();
	const QValueListConstIterator< ContactListInstance > end = m_instances.end();
	for ( ; it != end; ++it )
	{
		if ( (*it).objectId == objectId )
		{
			return true;
		}
	}
	return false;
}

CLInstanceList GroupWiseContact::instances() const
{
	return m_instances;
}

void GroupWiseContact::slotDeleteContact()
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	// remove all the instances of this contact from the server's contact list
	QValueListConstIterator< ContactListInstance > it = m_instances.begin();
	const QValueListConstIterator< ContactListInstance > end = m_instances.end();
	for ( ; it != end; ++it )
	{
		DeleteContactTask * dct = new DeleteContactTask( account()->client()->rootTask() );
		dct->contact( (*it).parentId, (*it).objectId );
		connect( dct, SIGNAL( gotContactDeleted( const ContactItem & ) ), SLOT( receiveContactDeleted( const ContactItem & ) ) );
		dct->go( true );
	}
}

void GroupWiseContact::receiveContactDeleted( const ContactItem & instance )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	removeCLInstance( instance.id );
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << contactId() << " now has " << m_instances.count() << " instances remaining." << endl;
	if ( m_instances.count() == 0 )
		deleteLater();
}

#include "gwcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

