/*
    gwcontact.cpp - Kopete GroupWise Protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed   
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    
	Blocking status taken from MSN
    Copyright (c) 2003      by Will Stephenson		  <will@stevello.free-online.co.uk>
    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Ryan Cumming           <bodnar42@phalynx.dhs.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@tiscalinet.be>
    
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

#include "tasks/deleteitemtask.h"
#include "tasks/updatecontacttask.h"

#include "client.h"
#include "gwaccount.h"
#include "ui/gwcontactproperties.h"
#include "gwerror.h"
#include "gwfakeserver.h"
#include "gwmessagemanager.h"
#include "gwprotocol.h"
#include "privacymanager.h"
#include "userdetailsmanager.h"

#include "gwcontact.h"

using namespace GroupWise;

GroupWiseContact::GroupWiseContact( KopeteAccount* account, const QString &dn, 
			KopeteMetaContact *parent, 
			const int objectId, const int parentId, const int sequence )
: KopeteContact( account, GroupWiseProtocol::dnToDotted( dn ), parent ), m_objectId( objectId ), m_parentId( parentId ),
  m_sequence( sequence ), m_actionBlock( 0 )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " id supplied: " << dn << endl;
	if ( dn.find( '=' ) != -1 )
	{
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "dn: " << dn << " m_dn before: " << m_dn << endl;
		m_dn = dn;
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "dn: " << dn << " m_dn after: " << m_dn << endl;
	}
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

QString GroupWiseContact::dn() const
{
	return m_dn;
}

void GroupWiseContact::updateDetails( const ContactDetails & details )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	if ( !details.cn.isNull() )
		setProperty( protocol()->propCN, details.cn );
	if ( !details.dn.isNull() )
		m_dn = details.dn;
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

	// fallback, something went wrong
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

QPtrList<KAction> *GroupWiseContact::customContextMenuActions() 
{
	QPtrList<KAction> *m_actionCollection = new QPtrList<KAction>;

	// Block/unblock Contact
	QString label = account()->isContactBlocked( m_dn ) ? i18n( "Unblock User" ) : i18n( "Block User" );
	if( !m_actionBlock )
	{
		m_actionBlock = new KAction( label, "msn_blocked",0, this, SLOT( slotBlock() ),
			this, "actionBlock" );
	}
	else
		m_actionBlock->setText( label );
	m_actionBlock->setEnabled( account()->isConnected() );
	
	m_actionCollection->append( m_actionBlock );

	return m_actionCollection;
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
		QString autoReplyPrefix = i18n("Prefix used for automatically generated auto-reply"
									   " messages when the contact is Away, contains contact's name",
				"Auto reply from %1: " ).arg( metaContact()->displayName() );
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
	//TODO: Implement leaveConference.
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
		DeleteItemTask * dit = new DeleteItemTask( account()->client()->rootTask() );
		dit->item( (*it).parentId, (*it).objectId );
		connect( dit, SIGNAL( gotContactDeleted( const ContactItem & ) ), SLOT( receiveContactDeleted( const ContactItem & ) ) );
		dit->go( true );
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

void GroupWiseContact::syncGroups()
{
	if ( account()->myself() != this )
	{
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
		if ( !account()->isConnected() )
		{
			kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "not connected, can't sync display name or group membership" << endl;
			return;
		}
	
		// if this is a temporary contact, don't bother
		if ( metaContact()->isTemporary() )
			return;
			
		// determine which groups we have joined
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " assuming we haven't JOINED any groups" << endl;
		// get the difference between the current set of contact list instances and the metacontact's current group membership
		// assume all MC groups are new wrt server side list
			// initialise a duplicate newGroupMemberships MC group list initially containing all the MC's groups
		// assume we haven't left any groups.
		// cycle through each group in the contact's CLInstances
			// if there is a corresponding MC group
				// this group exists on the server and we have not left it, definitely don't add it
				// remove that MC group from newGroupMemberships
				// add it to unchangedMemberships 
			// if it is not
				// we have left this group
				// create a contactlistitem and add to groupsLeft
	
		// determine which groups we have left
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " assuming we haven't LEFT any groups" << endl;
		// start an UpdateItem
/*		if ( metaContact()->displayName() != m_displayName )
		{*/
			kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " resetting the contact's display name to " << metaContact()->displayName() << endl;
			// form a list of the contact's groups
			QValueList< ContactItem > instancesToChange;
			QValueList< ContactListInstance >::Iterator it = m_instances.begin();
			const QValueList< ContactListInstance >::Iterator end = m_instances.end();
			for ( ; it != end; ++it )
			{
				ContactItem instance;
				instance.id = (*it).objectId;
				instance.parentId = (*it).parentId;
				instance.sequence = (*it).sequence;
				instance.dn = m_dn;
				instance.displayName = m_displayName;
				instancesToChange.append( instance );
			}
			UpdateContactTask * uct = new UpdateContactTask( account()->client()->rootTask() );
			uct->renameContact( metaContact()->displayName(), instancesToChange );
			uct->go( true );
//			setProperty( Kopete::Global::Properties::self()->nickName(), metaContact()->displayName() );
// 		}
	}
}

void GroupWiseContact::slotBlock()
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	if ( account()->isConnected() )
	{
		connect( account()->client()->privacyManager(), SIGNAL( privacyChanged( const QString &, bool ) ),
				SLOT( receivePrivacyChanged( const QString &, bool ) ) );
		if ( account()->isContactBlocked( m_dn ) )
			account()->client()->privacyManager()->setAllow( m_dn );
		else
			account()->client()->privacyManager()->setDeny( m_dn );
	}
}

void GroupWiseContact::receivePrivacyChanged( const QString & dn, bool allow )
{
	Q_UNUSED( allow );
	if ( dn == m_dn )
	{
		disconnect( account()->client()->privacyManager(), SIGNAL( privacyChanged( const QString &, bool ) ),
					this, SLOT( receivePrivacyChanged( const QString &, bool ) ) );
		// set the online status back to itself. this will reset the blocking state
		setOnlineStatus( this->onlineStatus() );
	}
}

void GroupWiseContact::setOnlineStatus( const KopeteOnlineStatus& status )
{
	if ( account()->isContactBlocked( m_dn ) && status.internalStatus() < 15 )
	{
		KopeteContact::setOnlineStatus(KopeteOnlineStatus(status.status() , (status.weight()==0) ? 0 : (status.weight() -1)  ,
			protocol() , status.internalStatus()+15 , QString::fromLatin1("msn_blocked"),
			status.caption() ,  i18n("%1|Blocked").arg( status.description() ) ) );
	}
	else
	{
		if(status.internalStatus() >= 15)
		{	//the user is not blocked, but the status is blocked
			switch(status.internalStatus()-15)
			{
				case 0:
					KopeteContact::setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseUnknown );
					break;
				case 1:
					KopeteContact::setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseOffline );
					break;
				case 2:
					KopeteContact::setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseAvailable );
					break;
				case 3:
					KopeteContact::setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseBusy );
					break;
				case 4:
					KopeteContact::setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseAway );
					break;
				case 5:
					KopeteContact::setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseAwayIdle );
					break;
				default:
					KopeteContact::setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseUnknown );
					break;
			}
		}
		else
			KopeteContact::setOnlineStatus(status);
	}
}

#include "gwcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

