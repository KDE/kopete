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

#include <kopeteaccount.h>
#include <kopetecontactlist.h>
#include <kopeteglobal.h>
#include <kopetegroup.h>
#include <kopetemessage.h>
#include <kopetemessagemanagerfactory.h>
#include <kopetemetacontact.h>

#include "tasks/createcontactinstancetask.h"
#include "tasks/deleteitemtask.h"
#include "tasks/movecontacttask.h"
#include "tasks/updatecontacttask.h"

#include "client.h"
#include "gwaccount.h"
#include "ui/gwcontactproperties.h"
#include "gwerror.h"
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
  m_sequence( sequence ), m_actionBlock( 0 ), m_archiving( false ), m_deleting( false )
{
	if ( dn.find( '=' ) != -1 )
	{
		m_dn = dn;
	}
	connect( static_cast< GroupWiseAccount *>( account ), SIGNAL( privacyChanged( const QString &, bool ) ),
			SLOT( receivePrivacyChanged( const QString &, bool ) ) );
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
	m_archiving = details.archive;
	//if ( !details.awayMessage.isNull() )
		//setProperty( protocol()->propAwayMessage, details.awayMessage );
	
	m_serverProperties = details.properties;
	
	if ( details.status != GroupWise::Invalid )
	{	
		KopeteOnlineStatus status = protocol()->gwStatusToKOS( details.status );
		//kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "setting initial status to " << status.description() << endl;
		setOnlineStatus( status );
	}
	//else 
		//kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "initial status is, not setting " << details.status << endl; 
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
	serializedData[ "DN" ] = m_dn;
}

KopeteMessageManager * GroupWiseContact::manager( bool canCreate )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "called, canCreate: " << canCreate << endl;

	KopeteContactPtrList chatMembers;
	chatMembers.append( this );

	return account()->messageManager( chatMembers, QString::null, canCreate );
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

void GroupWiseContact::purgeCLInstances()
{
	m_instances.clear();
}

void GroupWiseContact::slotDeleteContact()
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	// remove all the instances of this contact from the server's contact list
	QValueListConstIterator< ContactListInstance > it = m_instances.begin();
	const QValueListConstIterator< ContactListInstance > end = m_instances.end();
	m_deleting = true;
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
	if ( m_instances.count() == 0 && m_deleting )
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

		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " = CONTACT '" << property( Kopete::Global::Properties::self()->nickName() ).value().toString() << "' IS IN " << metaContact()->groups().count() << " MC GROUPS, AND HAS " << m_instances.count() << " CONTACT LIST INSTANCES." << endl;

		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " = LOOKING FOR NOOP GROUP MEMBERSHIPS" << endl;
		// 1) Seek matches between CLIs and MCGs and remove from the lists without taking any action. match on objectid, parentid
		// 2) Each remaining unmatched pair is a move, initiate and remove - need to take care to always use greatest unused sequence number - if we have to set the sequence number to the following sequence number within the folder, we may have a problem where after the first move, we have to wait for the state of the CLIs to be updated pending the completion of the first move - this would be difficult to cope with, because our current lists would be out of date, or we'd have to restart the sync - assuming the first move created a new matched CLI-MCG pair, we could do that with little cost.
		// 3) Any remaining entries in MCG list are adds, carry out
		// 4) Any remaining entries in CLI list are removes, carry out

		// start by discoverint the next free group sequence number in case we have to add any groups
		int nextFreeSequence = 0;
		QPtrList< KopeteGroup > groupList = KopeteContactList::contactList()->groups();
		QPtrListIterator< KopeteGroup > it( groupList );
		while( *it )
		{
			bool ok = true;
			int sequence = ( (*it)->pluginData( protocol(), account()->accountId() + " sequence" ) ).toInt( &ok );
			if ( sequence >= nextFreeSequence )
				nextFreeSequence = sequence + 1;
			++it;
		}
		// 1)
		// make a list of all the groups the metacontact is in
		groupList = metaContact()->groups();
		// make a list of all the groups this contact is in, according to its CLInstances
		CLInstanceList contactInstanceList = m_instances;
		// seek corresponding pairs in both lists and remove
		// ( for each group )
		QPtrListIterator< KopeteGroup > grpIt( groupList );
		while ( *grpIt )
		{
			QPtrListIterator< KopeteGroup > candidateGrp( groupList );
			candidateGrp = grpIt;
			++grpIt;
	
			QValueList< ContactListInstance >::Iterator instIt = contactInstanceList.begin();
			const QValueList< ContactListInstance >::Iterator instEnd = contactInstanceList.end();
			// ( see if a contactlist instance matches the group)
			while ( instIt != instEnd )
			{
				QValueList< ContactListInstance >::Iterator candidateInst = instIt;
				++instIt;
				
				kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " - Looking for a match, MC grp '" << ( *candidateGrp )->displayName() << "', objectId is '" << ( *candidateGrp )->pluginData( protocol(), account()->accountId() + " objectId" ) << "', CLInstance id is '" << QString::number( ( *candidateInst ).parentId ) << "'" << endl;
				
				if ( ( *candidateGrp )->pluginData( protocol(), account()->accountId() + " objectId" ).toInt() 
					== ( *candidateInst ).parentId )
				{
					// ( this pair matches, we can remove its members from both lists )
					kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "  - match! removing both entries" << endl;
					contactInstanceList.remove( candidateInst );
					groupList.remove( *candidateGrp );
					break;
				}
			}
		}
		
		// 2) seek unmatched pairs and move
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " = LOOKING FOR MOVES" << endl;
		grpIt.toFirst();
		// ( take the first pair and carry out a move )
		while ( *grpIt && !contactInstanceList.isEmpty() )
		{
			QPtrListIterator< KopeteGroup > candidateGrp( groupList );
			candidateGrp = grpIt;
			++grpIt;
			QValueList< ContactListInstance >::Iterator instIt = contactInstanceList.begin();

			kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "  - moving contact instance from group '" << (*instIt).parentId << "' to group '" << ( *candidateGrp )->pluginData( protocol(), account()->accountId() + " objectId" ) << "'" << endl;

			ContactItem instance;
			instance.id = ( *instIt ).objectId;
			instance.parentId = ( *instIt ).parentId;
			instance.sequence = ( *instIt ).sequence;
			instance.dn = m_dn;
			instance.displayName = property( Kopete::Global::Properties::self()->nickName() ).value().toString();

			MoveContactTask * mit = new MoveContactTask( account()->client()->rootTask() );
			mit->moveContact( instance, ( *candidateGrp )->pluginData( protocol(), account()->accountId() + " objectId" ).toInt() );
			connect( mit, SIGNAL( gotContactDeleted( const ContactItem & ) ), SLOT( receiveContactDeleted( const ContactItem & ) ) );
			mit->go();

			groupList.remove( candidateGrp );
			contactInstanceList.remove( instIt );
		}

		// 3) look for adds
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " = LOOKING FOR ADDS" << endl;
		grpIt.toFirst();
		while ( *grpIt )
		{
			QPtrListIterator< KopeteGroup > candidateGrp( groupList );
			candidateGrp = grpIt;
			++grpIt;
			kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "  - add a contact instance for group '" << ( *candidateGrp )->pluginData( protocol(), account()->accountId() + " objectId" ) << "'" << endl;

			CreateContactInstanceTask * ccit = new CreateContactInstanceTask( account()->client()->rootTask() );
			connect( ccit, SIGNAL( gotContactDeleted( const ContactItem & ) ), SLOT( receiveContactDeleted( const ContactItem & ) ) );

			// does this group exist on the server?  Create the contact appropriately
			bool ok = false;
			int parentId = ( *candidateGrp )->pluginData( protocol(), account()->accountId() + " objectId" ).toInt( &ok );
			if ( ok )
				ccit->contactFromUserId( m_dn, metaContact()->displayName(), parentId );
			else
			{
				// discover the next free sequence number and add the group using that
				ccit->contactFromUserIdAndFolder( m_dn, metaContact()->displayName(), nextFreeSequence++, ( *candidateGrp )->displayName() );
			}
			ccit->go( true );
			groupList.remove( candidateGrp );
		}

		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " = LOOKING FOR REMOVES" << endl;
		QValueList< ContactListInstance >::Iterator instIt = contactInstanceList.begin();
		const QValueList< ContactListInstance >::Iterator instEnd = contactInstanceList.end();
		// ( remove each remaining contactlist instance, because it doesn't exist locally any more )
		while ( instIt != instEnd )
		{
			QValueList< ContactListInstance >::Iterator candidateInst = instIt;
			++instIt;
			kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "  - remove contact instance '"<< ( *candidateInst ).objectId << "' in group '" << ( *candidateInst ).parentId << "'" << endl;

			DeleteItemTask * dit = new DeleteItemTask( account()->client()->rootTask() );
			dit->item( (*candidateInst).parentId, (*candidateInst).objectId );
			connect( dit, SIGNAL( gotContactDeleted( const ContactItem & ) ), SLOT( receiveContactDeleted( const ContactItem & ) ) );
			dit->go( true );

			contactInstanceList.remove( candidateInst );
		}

		// TODO: Don't forget to make sure the local CLInstance entries are updated, either here or by the results of the Tasks.

		// start an UpdateItem
		if ( metaContact()->displayName() != property( Kopete::Global::Properties::self()->nickName() ).value().toString() )
		{
			kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " resetting the contact's display name to " << metaContact()->displayName() << endl;
			// form a list of the contact's groups
			QValueList< ContactListInstance >::Iterator it = m_instances.begin();
			const QValueList< ContactListInstance >::Iterator end = m_instances.end();
			for ( ; it != end; ++it )
			{
				QValueList< ContactItem > instancesToChange;
				ContactItem instance;
				instance.id = (*it).objectId;
				instance.parentId = (*it).parentId;
				instance.sequence = (*it).sequence;
				instance.dn = m_dn;
				instance.displayName = property( Kopete::Global::Properties::self()->nickName() ).value().toString();
				instancesToChange.append( instance );
				UpdateContactTask * uct = new UpdateContactTask( account()->client()->rootTask() );
				uct->renameContact( metaContact()->displayName(), instancesToChange );
				connect ( uct, SIGNAL( finished() ), SLOT( slotRenamedOnServer() ) );
				uct->go( true );
			}
 		}
	}
}

void GroupWiseContact::slotBlock()
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	if ( account()->isConnected() )
	{
		if ( account()->isContactBlocked( m_dn ) )
			account()->client()->privacyManager()->setAllow( m_dn );
		else
			account()->client()->privacyManager()->setDeny( m_dn );
	}
}

void GroupWiseContact::receivePrivacyChanged( const QString & dn, bool allow )
{
	Q_UNUSED( allow );
	if ( dn == m_dn )	// set the online status back to itself. this will set the blocking state
		setOnlineStatus( this->onlineStatus() );
}

void GroupWiseContact::setOnlineStatus( const KopeteOnlineStatus& status )
{
	bool idleChanged = false;
	if ( status == protocol()->groupwiseAwayIdle && status != onlineStatus() )
	{
		idleChanged = true;
		setIdleTime( 1 );
	}
	else if ( onlineStatus() == protocol()->groupwiseAwayIdle && status != onlineStatus() )
	{
		idleChanged = true;
		setIdleTime( 0 );
	}

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
	if ( idleChanged )
		emit idleStateChanged( this );
}

bool GroupWiseContact::archiving()
{
	return m_archiving;
}

void GroupWiseContact::slotRenamedOnServer()
{
	UpdateContactTask * uct = ( UpdateContactTask * )sender();
	if ( uct->success() )
	{
		if( uct->displayName() != 
				property( Kopete::Global::Properties::self()->nickName() ).value().toString() )
			setProperty( Kopete::Global::Properties::self()->nickName(), uct->displayName() );
	}
	else
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "rename failed, return code: " << uct->statusCode() << endl;
}
#include "gwcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

