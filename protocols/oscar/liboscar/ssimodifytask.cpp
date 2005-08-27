/*
	Kopete Oscar Protocol
	ssimodifytask.cpp - Handles all the ssi modification stuff

	Copyright (c) 2004 by Kopete Developers <kopete-devel@kde.org>

	Based on code Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
	Based on Iris, Copyright (C) 2003  Justin Karneges

	Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

	*************************************************************************
	*                                                                       *
	* This library is free software; you can redistribute it and/or         *
	* modify it under the terms of the GNU Lesser General Public            *
	* License as published by the Free Software Foundation; either          *
	* version 2 of the License, or (at your option) any later version.      *
	*                                                                       *
	*************************************************************************
*/
#include "ssimodifytask.h"

#include <kdebug.h>
#include <klocale.h>
#include <qstring.h>
#include "connection.h"
#include "oscarutils.h"
#include "transfer.h"


SSIModifyTask::SSIModifyTask( Task* parent ) : Task( parent )
{
	m_ssiManager = parent->client()->ssiManager();
	m_opType = NoType;
	m_opSubject = NoSubject;
	m_id = 0;
}


SSIModifyTask::~SSIModifyTask()
{
}

void SSIModifyTask::onGo()
{
	sendSSIUpdate();
}

bool SSIModifyTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		setTransfer( transfer );
		handleSSIAck();
		setTransfer( 0 );
		return true;
	}
	else
		return false;
}

bool SSIModifyTask::addContact( const QString& contact, const QString& group, bool requiresAuth )
{
	m_opType = Add;
	m_opSubject = Contact;
	
	QString newContact = Oscar::normalize( contact );
	
	Oscar::SSI oldItem = m_ssiManager->findContact( newContact );
	Oscar::SSI groupItem = m_ssiManager->findGroup( group );
	
	if ( !groupItem )
	{
		kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "group " << group << " does not exist on SSI. Aborting" << endl;
		return false;
	}
	
	//create new SSI item and populate the TLV list
	QValueList<TLV> tlvList;
	if ( requiresAuth )
	{
		kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "This contact requires auth. adding appropriate tlv" << endl;
		TLV t( 0x0066, 0, 0 );
		tlvList.append( t );
	}
	
	kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "creating new SSI item for " << contact << " in group " << group << endl;
	Oscar::SSI newItem( newContact, groupItem.gid(), m_ssiManager->nextContactId(), ROSTER_CONTACT, tlvList );
	m_newItem = newItem;
	return true;
}

bool SSIModifyTask::removeContact( const QString& contact )
{
	m_opType = Remove;
	m_opSubject = Contact;
	m_oldItem = m_ssiManager->findContact( Oscar::normalize( contact ) );
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Scheduling" << m_oldItem.name() << " for removal" << endl;
	return true;
}

bool SSIModifyTask::changeGroup( const QString& contact, const QString& newGroup )
{
	m_opType = Change;
	m_opSubject = Group;
	m_oldItem = m_ssiManager->findContact( Oscar::normalize( contact ) );
	Oscar::SSI oldGroupItem;
	if ( m_oldItem.isValid() )
		oldGroupItem = m_ssiManager->findGroup( newGroup );
	else
		return false;
	
	if ( m_oldItem.gid() == oldGroupItem.gid() )
	{ //buddy already exists in this group
		kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "contact " << contact << " already exists in group " << oldGroupItem.name() << ". Aborting." << endl;
		return false;
	}
	
	m_groupItem = m_ssiManager->findGroup( newGroup );
	if ( !m_groupItem )
	{ //couldn't find group
		kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "new group " << newGroup << " not found in SSI. Aborting" << endl;
		return false;
	}
	
	//create a new SSI item for the buddy in the new group
	Oscar::SSI newItem( m_oldItem.name(), m_groupItem.gid(), m_oldItem.bid(), ROSTER_CONTACT, m_oldItem.tlvList() ); 
	m_newItem = newItem;
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Moving '" << m_oldItem.name() << "' to group " << m_groupItem.name() << endl;
	return true;
}

bool SSIModifyTask::addGroup( const QString& groupName )
{
	m_opType = Add;
	m_opSubject = Group;
	m_newItem = m_ssiManager->findGroup( groupName );
	QValueList<TLV> dummy;
	Oscar::SSI newItem( groupName, m_ssiManager->nextGroupId(), 0, ROSTER_GROUP, dummy );
	m_newItem = newItem;
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Adding group '" << m_newItem.name() << "' to SSI" << endl;
	return true;
}

bool SSIModifyTask::removeGroup( const QString& groupName )
{
	m_opType = Remove;
	m_opSubject = Group;
	m_oldItem = m_ssiManager->findGroup( groupName );
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Scheduling group '" << m_oldItem.name() << "' for removal. " << endl;
	return true;
}

bool SSIModifyTask::renameGroup( const QString& oldName, const QString & newName )
{
	m_opType = Rename;
	m_opSubject = Group;
	if ( oldName == newName )
		return false;
	
	m_oldItem = m_ssiManager->findGroup( oldName );
	Oscar::SSI newItem( newName, m_oldItem.gid(), m_oldItem.bid(), ROSTER_GROUP, m_oldItem.tlvList() );
	m_newItem = newItem;
	return true;
}

bool SSIModifyTask::addItem( const SSI& item )
{
	m_opType = Add;
	m_opSubject = NoSubject;
	m_newItem = item;
	return true;
}

bool SSIModifyTask::removeItem( const SSI& item )
{
	m_opType = Remove;
	m_opSubject = NoSubject;
	m_oldItem = item;
	return false;
}

bool SSIModifyTask::modifyItem( const SSI& oldItem, const SSI& newItem )
{
	if ( !m_ssiManager->hasItem( oldItem ) )
		return false;
	
	//make sure there are some common things between the two items
	if ( oldItem.type() != newItem.type() )
		return false;
	
	m_oldItem = oldItem;
	m_newItem = newItem;
	m_opType = Change;
	m_opSubject = NoSubject;
	return true;
}

bool SSIModifyTask::forMe( const Transfer * transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( !st )
		return false;
	
	if ( st->snacService() == 0x0013 && st->snacSubtype() == 0x000E && m_id == st->snac().id )
		return true;
	else
		return false;
}

void SSIModifyTask::handleSSIAck()
{
	Buffer* b = transfer()->buffer();
	int numItems = b->length() / 2;
	for( int i = 0; i < numItems; ++i )
	{
		WORD ackCode = b->getWord();
		kdDebug(OSCAR_RAW_DEBUG) << "Acknowledgement code is " << ackCode << endl;
		switch( ackCode )
		{
		case 0x0000:
			kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "SSI Update successful" << endl;
			updateSSIManager();
			break;
		case 0x0002:
			kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Item to modify not found in list" << endl;
			setSuccess( 0, QString::null );
			break;
		case 0x0003:
			kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Item already exists in SSI" << endl;
			setSuccess( 0, QString::null );
			break;
		case 0x000A:
			kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Error adding item ( invalid id, already in list, invalid data )" << endl;
			setSuccess( 0, QString::null );
			break;
		case 0x000C:
			kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Can't add item. Limit exceeded." << endl;
			setSuccess( 0, QString::null );
			break;
		case 0x000D:
			kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Can't add ICQ item to AIM list ( and vice versa )" << endl;
			setSuccess( 0, QString::null );
			break;
		case 0x000E:
			{
			kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Can't add item because contact requires authorization" << endl;
			Oscar::SSI groupItem = m_ssiManager->findGroup( m_newItem.gid() );
			QString groupName = groupItem.name();
			addContact( m_newItem.name(), groupName, true );
			go();
			break;
			}
		default:
			kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Unknown acknowledgement code" << endl;
			setSuccess( 0, QString::null );
			break;
		}
	};
	
	
}

void SSIModifyTask::sendSSIUpdate()
{
	//what type of update are we sending?
	if ( m_opSubject == Group && m_opType == Change )
		changeGroupOnServer();
	
	//add an item to the ssi list
	if ( m_opType == Add )
	{
		kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Adding an item to the SSI list" << endl;
		sendEditStart();
		
		//add the item
		FLAP f1 = { 0x02, client()->flapSequence(), 0 };
		m_id = client()->snacSequence();
		SNAC s1 = { 0x0013, 0x0008, 0x0000, m_id };
		Buffer* ssiBuffer = new Buffer;
		ssiBuffer->addString( m_newItem );
		Transfer* t2 = createTransfer( f1, s1, ssiBuffer );
		send( t2 );
		
		sendEditEnd();
	}
	
	//remove an item
	if ( m_opType == Remove )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Removing " << m_oldItem.name() << " from SSI" << endl;
		sendEditStart();
		
		//remove the item
		FLAP f1 = { 0x02, client()->flapSequence(), 0 };
		m_id = client()->snacSequence();
		SNAC s1 = { 0x0013, 0x000A, 0x0000, m_id };
		Buffer* ssiBuffer = new Buffer;
		ssiBuffer->addString( m_oldItem );
		Transfer* t2 = createTransfer( f1, s1, ssiBuffer );
		send( t2 );
		
		sendEditEnd();
	}

	//modify an item
	//we use rename for group and change for other items
	if ( m_opType == Rename || ( m_opType == Change && m_opSubject != Group ) )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Modifying the item: " << m_oldItem.toString() << endl;
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "changing it to: " << m_newItem.toString() << endl;
		sendEditStart();
		
		//change the group name
		FLAP f1 = { 0x02, client()->flapSequence(), 0 };
		m_id = client()->snacSequence();
		SNAC s1 = { 0x0013, 0x0009, 0x0000, m_id };
		Buffer* ssiBuffer = new Buffer;
		ssiBuffer->addString( m_newItem );
		Transfer* t2 = createTransfer( f1, s1, ssiBuffer );
		send( t2 );
		
		sendEditEnd();
	}
	
}

void SSIModifyTask::changeGroupOnServer()
{
	kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Moving a contact from one group to another" << endl;

	sendEditStart();
	
	//remove the old buddy from the list 
	FLAP f1 = { 0x02, client()->flapSequence(), 0 };
	SNAC s1 = { 0x0013,  0x000A, 0x0000, client()->snacSequence() };
	Buffer* b1 = new Buffer;
	b1->addBSTR( m_oldItem.name().latin1() );
	b1->addWord( m_oldItem.gid() );
	b1->addWord( m_oldItem.bid() );
	b1->addWord( m_oldItem.type() );
	b1->addWord( 0 );
	Transfer* t2 = createTransfer( f1, s1, b1 );
	send( t2 );
	
	//add the buddy to the list with a different group
	FLAP f2 = { 0x02, client()->flapSequence(), 0 };
	m_id = client()->snacSequence(); //we don't care about the first ack
	SNAC s2 = { 0x0013, 0x0008, 0x0000, m_id };
	Buffer* b2 = new Buffer;
	addItemToBuffer( m_newItem, b2 );
	
	Transfer* t3 = createTransfer( f2, s2, b2 );
	send( t3 );
	
	//find the old group so we can change it's list of buddy ids
	//what a kludge
	Oscar::SSI oldGroupItem = m_ssiManager->findGroup( m_oldItem.gid() );
	/* not checking the existance of oldGroupItem because if we got here
	   it has to exist */
		
	//Change the 0x00C8 TLV in the old group item to remove the bid we're
	//moving to a different group
	QValueList<TLV> list = oldGroupItem.tlvList();
	TLV oldIds = Oscar::findTLV( list, 0x00C8 );
	if ( oldIds.type == 0x00C8 )
	{
		Buffer newTLVData;
		Buffer tlvBuffer( oldIds.data, oldIds.length );
		while ( tlvBuffer.length() != 0 )
		{
			WORD id = tlvBuffer.getWord();
			if ( id != m_oldItem.bid() )
				newTLVData.addWord( id );
		}
		
		TLV newGroupTLV( 0x00C8, newTLVData.length(), newTLVData.buffer() );
		
		list.remove( oldIds );
		list.append( newGroupTLV );
		oldGroupItem.setTLVList( list );
	}
	

	//Change the 0x00C8 TLV in the new group item to add the bid we're
	//adding to this group
	QValueList<TLV> list2 = m_groupItem.tlvList();
	TLV oldIds2 = Oscar::findTLV( list2, 0x00C8 );
	TLV newGroupTLV;
	if ( oldIds2.type == 0x00C8 )
	{
		Buffer tlvBuffer( oldIds2.data, oldIds2.length );
		tlvBuffer.addWord( m_newItem.bid() );
		
		TLV newGroupTLV( 0x00C8, tlvBuffer.length(), tlvBuffer.buffer() );
		list2.remove( oldIds );
		list2.append( newGroupTLV );
		m_groupItem.setTLVList( list2 );
	}
	
	//change the group properties
	FLAP f3 = { 0x02, client()->flapSequence(), 0 };
	SNAC s3 = { 0x0013, 0x0009, 0x0000, client()->snacSequence() };
	Buffer* b3 = new Buffer;
	addItemToBuffer( oldGroupItem, b3 );
	addItemToBuffer( m_groupItem, b3 );

	Transfer* t4 = createTransfer( f3, s3, b3 ); //we get no ack from this packet
	send( t4 );
	
	sendEditEnd();
}

void SSIModifyTask::updateSSIManager()
{
	if ( m_oldItem.isValid() && m_newItem.isValid() )
	{ //changing groups on the server, or renaming. the group item will have already been updated
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Removing " << m_oldItem.name() << endl;
		m_ssiManager->removeContact( m_oldItem.name() );
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "and adding " << m_newItem.name() << " to SSI manager" << endl;
		m_ssiManager->newContact( m_newItem );
		setSuccess( 0, QString::null );
		return;
	}
	
	if ( m_oldItem.isValid() && !m_newItem )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Removing " << m_oldItem.name() << " from SSI manager" << endl;
		if ( m_opSubject == Group )
			m_ssiManager->removeGroup( m_oldItem.name() );
		if ( m_opSubject == Contact )
			m_ssiManager->removeContact( m_oldItem.name() );
		setSuccess( 0, QString::null );
		return;
	}

	if ( m_newItem.isValid() && !m_oldItem )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Adding " << m_newItem.name() << " to SSI manager" << endl;
		if ( m_opSubject == Group )
			m_ssiManager->newGroup( m_newItem );
		if ( m_opSubject == Contact )
			m_ssiManager->newContact( m_newItem );
		setSuccess( 0, QString::null );
		return;
	}
	
	setSuccess( 0, QString::null );
}

void SSIModifyTask::sendEditStart()
{
	SNAC editStartSnac = { 0x0013, 0x0011, 0x0000, client()->snacSequence() };
	FLAP editStart = { 0x02, client()->flapSequence(), 10 };
	Buffer* emptyBuffer = new Buffer;
	Transfer* t1 = createTransfer( editStart, editStartSnac, emptyBuffer );
	send( t1 );
}

void SSIModifyTask::sendEditEnd()
{
	SNAC editEndSnac = { 0x0013, 0x0012, 0x0000, client()->snacSequence() };
	FLAP editEnd = { 0x02, client()->flapSequence(), 10 } ;
	Buffer* emptyBuffer = new Buffer;
	Transfer *t5 = createTransfer( editEnd, editEndSnac, emptyBuffer );
	send( t5 );
}

void SSIModifyTask::addItemToBuffer( Oscar::SSI item, Buffer* buffer )
{
	buffer->addBSTR( item.name().latin1() );
	buffer->addWord( item.gid() );
	buffer->addWord( item.bid() );
	buffer->addWord( item.type() );
	buffer->addWord( item.tlvListLength() );
	
	QValueList<TLV>::const_iterator it =  item.tlvList().begin();
	QValueList<TLV>::const_iterator listEnd = item.tlvList().end();
	for( ; it != listEnd; ++it )
		buffer->addTLV( ( *it ) );
}

//kate: tab-width 4; indent-mode csands;
