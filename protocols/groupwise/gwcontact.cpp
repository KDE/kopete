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
#include "kopetemessage.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"

#include "gwaccount.h"
#include "gwerror.h"
#include "gwfakeserver.h"
#include "gwmessagemanager.h"
#include "gwprotocol.h"

#include "gwcontact.h"

using namespace GroupWise;

GroupWiseContact::GroupWiseContact( KopeteAccount* account, const QString &dn, 
			KopeteMetaContact *parent, 
			const QString &displayName, const int objectId, const int parentId, const int sequence )
: KopeteContact( account, dn, parent ), m_objectId( objectId ), m_parentId( parentId ),
  m_sequence( sequence )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " dn: " << dn << endl;
	rename( displayName );
}

GroupWiseContact::~GroupWiseContact()
{
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

bool GroupWiseContact::isReachable()
{
	//TODO: use status to determine reachability
    return true;
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
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "NOT IMPLEMENTED!" << endl;
	
	KopeteMessageManager *_manager = KopeteMessageManagerFactory::factory()->findKopeteMessageManager ( account()->myself(), chatMembers, protocol() );
	GroupWiseMessageManager *mgr = dynamic_cast<GroupWiseMessageManager*>( _manager );

	/*
	 * If we didn't find a message manager for this contact,
	 * instantiate a new one if we are allowed to. (otherwise return 0)
	 */
	if ( !mgr && canCreate )
	{
		mgr = new GroupWiseMessageManager( static_cast<GroupWiseContact *>( account()->myself() ), chatMembers, protocol(), QString::null );
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
			kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " - creating a new GroupWiseMessageManager for this guid. " << endl;
			KopeteContactPtrList chatMembers;
			chatMembers.append ( this );
			mgr = new GroupWiseMessageManager( this, chatMembers, protocol(), guid );
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

void GroupWiseContact::showContactSettings()
{
	//GroupWiseContactSettings* p = new GroupWiseContactSettings( this );
	//p->show();
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

void GroupWiseContact::handleIncomingMessage( const ConferenceEvent & message )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "Got a message for conference: " << message.guid << ", message: " << message.message << endl; 
	KopeteContactPtrList contactList;
	contactList.append ( account()->myself () );
	GroupWiseMessageManager *mgr = manager( message.guid, true );
	// strip RTF using protocol's utility function
	KopeteMessage * newMessage = new KopeteMessage ( message.timeStamp, this, contactList, message.message,
									KopeteMessage::Inbound,
									KopeteMessage::PlainText );;
	Q_ASSERT( mgr );
	mgr->appendMessage( *newMessage );
	delete newMessage;
}
#include "gwcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

