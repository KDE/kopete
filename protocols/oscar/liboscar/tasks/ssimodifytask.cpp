/*
	Kopete Oscar Protocol
	ssimodifytask.cpp - Handles all the ssi modification stuff

	Copyright (c) 2004 by Kopete Developers <kopete-devel@kde.org>

	Based on code Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
	Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

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


SSIModifyTask::SSIModifyTask( Task* parent, bool staticTask ) : Task( parent )
{
	m_ssiManager = parent->client()->ssiManager();
	m_static = staticTask;
	m_opType = NoType;
	m_opSubject = NoSubject;
	m_id = 0;
}


SSIModifyTask::~SSIModifyTask()
{
}

void SSIModifyTask::onGo()
{
	sendContactUpdate();
}

bool SSIModifyTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		SnacTransfer* st = dynamic_cast<SnacTransfer*>( transfer );
		if ( st )
		{
			setTransfer( transfer );

			if ( st->snacSubtype() == 0x0008 )
				handleContactAdd();
			else if ( st->snacSubtype() == 0x0009 )
				handleContactUpdate();
			else if ( st->snacSubtype() == 0x000A )
				handleContactRemove();
			else if ( st->snacSubtype() == 0x000E )
				handleContactAck();

			setTransfer( 0 );
		}
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

	OContact oldItem = m_ssiManager->findContact( newContact );
	OContact groupItem = m_ssiManager->findGroup( group );

	if ( !groupItem )
	{
		kDebug( OSCAR_RAW_DEBUG ) << "group " << group << " does not exist on SSI. Aborting";
		return false;
	}

	//create new Contact item and populate the TLV list
	QList<TLV> tlvList;
	if ( requiresAuth )
	{
		kDebug( OSCAR_RAW_DEBUG ) << "This contact requires auth. adding appropriate tlv";
		TLV t( 0x0066, 0, 0 );
		tlvList.append( t );
	}

	kDebug( OSCAR_RAW_DEBUG ) << "creating new SSI item for " << contact << " in group " << group;
	OContact newItem( newContact, groupItem.gid(), m_ssiManager->nextContactId(), ROSTER_CONTACT, tlvList );
	m_newItem = newItem;
	return true;
}

bool SSIModifyTask::removeContact( const QString& contact )
{
	m_opType = Remove;
	m_opSubject = Contact;
	m_oldItem = m_ssiManager->findContact( Oscar::normalize( contact ) );
	kDebug(OSCAR_RAW_DEBUG) << "Scheduling" << m_oldItem.name() << " for removal";
	return true;
}

bool SSIModifyTask::changeGroup( const QString& contact, const QString& newGroup )
{
	m_opType = Change;
	m_opSubject = Group;
	m_oldItem = m_ssiManager->findContact( Oscar::normalize( contact ) );
	OContact oldGroupItem;
	if ( m_oldItem.isValid() )
		oldGroupItem = m_ssiManager->findGroup( newGroup );
	else
		return false;

	if ( m_oldItem.gid() == oldGroupItem.gid() )
	{ //buddy already exists in this group
		kDebug( OSCAR_RAW_DEBUG ) << "contact " << contact << " already exists in group " << oldGroupItem.name() << ". Aborting.";
		return false;
	}

	m_groupItem = m_ssiManager->findGroup( newGroup );
	if ( !m_groupItem )
	{ //couldn't find group
		kDebug( OSCAR_RAW_DEBUG ) << "new group " << newGroup << " not found in SSI. Aborting";
		return false;
	}

	//create a new Contact item for the buddy in the new group
	OContact newItem( m_oldItem.name(), m_groupItem.gid(), m_oldItem.bid(), ROSTER_CONTACT, m_oldItem.tlvList() );
	m_newItem = newItem;
	kDebug(OSCAR_RAW_DEBUG) << "Moving '" << m_oldItem.name() << "' to group " << m_groupItem.name();
	return true;
}

bool SSIModifyTask::addGroup( const QString& groupName )
{
	m_opType = Add;
	m_opSubject = Group;
	m_newItem = m_ssiManager->findGroup( groupName );
	QList<TLV> dummy;
	OContact newItem( groupName, m_ssiManager->nextGroupId(), 0, ROSTER_GROUP, dummy );
	m_newItem = newItem;
	kDebug(OSCAR_RAW_DEBUG) << "Adding group '" << m_newItem.name() << "' to SSI";
	return true;
}

bool SSIModifyTask::removeGroup( const QString& groupName )
{
	m_opType = Remove;
	m_opSubject = Group;
	m_oldItem = m_ssiManager->findGroup( groupName );
	kDebug(OSCAR_RAW_DEBUG) << "Scheduling group '" << m_oldItem.name() << "' for SSI. ";
	return true;
}

bool SSIModifyTask::renameGroup( const QString& oldName, const QString & newName )
{
	m_opType = Rename;
	m_opSubject = Group;
	if ( oldName == newName )
		return false;

	m_oldItem = m_ssiManager->findGroup( oldName );
	OContact newItem( newName, m_oldItem.gid(), m_oldItem.bid(), ROSTER_GROUP, m_oldItem.tlvList() );
	m_newItem = newItem;
	return true;
}

bool SSIModifyTask::addItem( const OContact& item )
{
	m_opType = Add;
	m_opSubject = NoSubject;
	m_newItem = item;
	return true;
}

bool SSIModifyTask::removeItem( const OContact& item )
{
	m_opType = Remove;
	m_opSubject = NoSubject;
	m_oldItem = item;
	return true;
}

bool SSIModifyTask::modifyItem( const OContact& oldItem, const OContact& newItem )
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

bool SSIModifyTask::modifyContact( const OContact& oldItem, const OContact& newItem )
{
	if ( !modifyItem(oldItem, newItem) )
		return false;
	
	m_opSubject = Contact;
	return true;
}

bool SSIModifyTask::forMe( const Transfer * transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( !st )
		return false;

	if ( st->snacService() == 0x0013 )
	{
		Oscar::WORD subtype = st->snacSubtype();
		if ( m_static )
		{
			if ( subtype == 0x0008 || subtype == 0x0009 || subtype == 0x000A )
				return true;
		}
		else
		{
			if ( subtype == 0x000E && m_id == st->snacRequest() )
				return true;
		}
	}

	return false;
}

void SSIModifyTask::handleContactAck()
{
	Buffer* b = transfer()->buffer();
	int numItems = b->bytesAvailable() / 2;
	for( int i = 0; i < numItems; ++i )
	{
		Oscar::WORD ackCode = b->getWord();
		kDebug(OSCAR_RAW_DEBUG) << "Acknowledgement code is " << ackCode;
		
		if ( ackCode != 0x0000 )
			freeIdOnError();
		
		switch( ackCode )
		{
		case 0x0000:
			kDebug( OSCAR_RAW_DEBUG ) << "SSI Update successful";
			updateContactManager();
			break;
		case 0x0002:
			kWarning( OSCAR_RAW_DEBUG ) << "Item to modify not found in list";
			setSuccess( 0, QString() );
			break;
		case 0x0003:
			kWarning( OSCAR_RAW_DEBUG ) << "Item already exists in SSI";
			setSuccess( 0, QString() );
			break;
		case 0x000A:
			kWarning( OSCAR_RAW_DEBUG ) << "Error adding item ( invalid id, already in list, invalid data )";
			setSuccess( 0, QString() );
			break;
		case 0x000C:
			kWarning( OSCAR_RAW_DEBUG ) << "Can't add item. Limit exceeded.";
			setSuccess( 0, QString() );
			break;
		case 0x000D:
			kWarning( OSCAR_RAW_DEBUG ) << "Can't add ICQ item to AIM list ( and vice versa )";
			setSuccess( 0, QString() );
			break;
		case 0x000E:
			{
			kWarning( OSCAR_RAW_DEBUG ) << "Can't add item because contact requires authorization";
			OContact groupItem = m_ssiManager->findGroup( m_newItem.gid() );
			QString groupName = groupItem.name();
			addContact( m_newItem.name(), groupName, true );
			go();
			break;
			}
		default:
			kWarning( OSCAR_RAW_DEBUG ) << "Unknown acknowledgement code " << ackCode;
			setSuccess( 0, QString() );
			break;
		}
	};


}

void SSIModifyTask::sendContactUpdate()
{
	//what type of update are we sending?
	if ( m_opSubject == Group && m_opType == Change )
		changeGroupOnServer();

	//add an item to the ssi list
	if ( m_opType == Add )
	{
		kDebug( OSCAR_RAW_DEBUG ) << "Adding an item to the SSI list";
		sendEditStart();

		//add the item
		FLAP f1 = { 0x02, 0, 0 };
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
		kDebug(OSCAR_RAW_DEBUG) << "Removing " << m_oldItem.name() << " from SSI";
		sendEditStart();

		//remove the item
		FLAP f1 = { 0x02, 0, 0 };
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
		kDebug(OSCAR_RAW_DEBUG) << "Modifying the item: " << m_oldItem.toString();
		kDebug(OSCAR_RAW_DEBUG) << "changing it to: " << m_newItem.toString();
		sendEditStart();

		//change the group name
		FLAP f1 = { 0x02, 0, 0 };
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
	kDebug( OSCAR_RAW_DEBUG ) << "Moving a contact from one group to another";

	sendEditStart();

	//remove the old buddy from the list
	FLAP f1 = { 0x02, 0, 0 };
	SNAC s1 = { 0x0013,  0x000A, 0x0000, client()->snacSequence() };
	Buffer* b1 = new Buffer;
	b1->addBSTR( m_oldItem.name().toUtf8() );
	b1->addWord( m_oldItem.gid() );
	b1->addWord( m_oldItem.bid() );
	b1->addWord( m_oldItem.type() );
	b1->addWord( 0 );
	Transfer* t2 = createTransfer( f1, s1, b1 );
	send( t2 );

	//add the buddy to the list with a different group
	FLAP f2 = { 0x02, 0, 0 };
	m_id = client()->snacSequence(); //we don't care about the first ack
	SNAC s2 = { 0x0013, 0x0008, 0x0000, m_id };
	Buffer* b2 = new Buffer;
	addItemToBuffer( m_newItem, b2 );

	Transfer* t3 = createTransfer( f2, s2, b2 );
	send( t3 );

	//find the old group so we can change it's list of buddy ids
	//what a kludge
	OContact oldGroupItem = m_ssiManager->findGroup( m_oldItem.gid() );
	/* not checking the existence of oldGroupItem because if we got here
	   it has to exist */

	//Change the 0x00C8 TLV in the old group item to remove the bid we're
	//moving to a different group
	QList<TLV> list = oldGroupItem.tlvList();
	TLV oldIds = Oscar::findTLV( list, 0x00C8 );
	if ( oldIds.type == 0x00C8 )
	{
		Buffer newTLVData;
		Buffer tlvBuffer( oldIds.data, oldIds.length );
		while ( tlvBuffer.bytesAvailable() != 0 )
		{
			Oscar::WORD id = tlvBuffer.getWord();
			if ( id != m_oldItem.bid() )
				newTLVData.addWord( id );
		}

		TLV newGroupTLV( 0x00C8, newTLVData.length(), newTLVData.buffer() );

		list.removeAll( oldIds );
		list.append( newGroupTLV );
		oldGroupItem.setTLVList( list );
	}


	//Change the 0x00C8 TLV in the new group item to add the bid we're
	//adding to this group
	QList<TLV> list2 = m_groupItem.tlvList();
	TLV oldIds2 = Oscar::findTLV( list2, 0x00C8 );
	TLV newGroupTLV;
	if ( oldIds2.type == 0x00C8 )
	{
		Buffer tlvBuffer( oldIds2.data, oldIds2.length );
		tlvBuffer.addWord( m_newItem.bid() );

		TLV newGroupTLV( 0x00C8, tlvBuffer.length(), tlvBuffer.buffer() );
		list2.removeAll( oldIds );
		list2.append( newGroupTLV );
		m_groupItem.setTLVList( list2 );
	}

	//change the group properties
	FLAP f3 = { 0x02, 0, 0 };
	SNAC s3 = { 0x0013, 0x0009, 0x0000, client()->snacSequence() };
	Buffer* b3 = new Buffer;
	addItemToBuffer( oldGroupItem, b3 );
	addItemToBuffer( m_groupItem, b3 );

	Transfer* t4 = createTransfer( f3, s3, b3 ); //we get no ack from this packet
	send( t4 );

	sendEditEnd();
}

void SSIModifyTask::updateContactManager()
{
	if ( m_oldItem.isValid() && m_newItem.isValid() )
	{
		if ( m_opSubject == Contact )
		{
			kDebug(OSCAR_RAW_DEBUG) << "Removing " << m_oldItem.name();
			m_ssiManager->removeContact( m_oldItem.name() );
			kDebug(OSCAR_RAW_DEBUG) << "and adding " << m_newItem.name() << " to contact manager";
			m_ssiManager->newContact( m_newItem );
		}
		else if ( m_opSubject == Group )
		{
			if ( m_opType == Rename )
				m_ssiManager->updateGroup( m_newItem );
			else if ( m_opType == Change )
				m_ssiManager->updateContact( m_newItem );
		}
		else if ( m_opSubject == NoSubject )
		{
			kDebug(OSCAR_RAW_DEBUG) << "Removing " << m_oldItem.name();
			m_ssiManager->removeItem( m_oldItem );
			kDebug(OSCAR_RAW_DEBUG) << "and adding " << m_newItem.name() << " to contact manager";
			m_ssiManager->newItem( m_newItem );
		}
		setSuccess( 0, QString() );
		return;
	}

	if ( m_oldItem.isValid() && !m_newItem )
	{
		kDebug(OSCAR_RAW_DEBUG) << "Removing " << m_oldItem.name() << " from contact manager";
		if ( m_opSubject == Group )
			m_ssiManager->removeGroup( m_oldItem.name() );
		else if ( m_opSubject == Contact )
			m_ssiManager->removeContact( m_oldItem.name() );
		else if ( m_opSubject == NoSubject )
			m_ssiManager->removeItem( m_oldItem );
		setSuccess( 0, QString() );
		return;
	}

	if ( m_newItem.isValid() && !m_oldItem )
	{
		kDebug(OSCAR_RAW_DEBUG) << "Adding " << m_newItem.name() << " to contact manager";
		if ( m_opSubject == Group )
			m_ssiManager->newGroup( m_newItem );
		else if ( m_opSubject == Contact )
			m_ssiManager->newContact( m_newItem );
		else if ( m_opSubject == NoSubject )
			m_ssiManager->newItem( m_newItem );
		setSuccess( 0, QString() );
		return;
	}

	setSuccess( 0, QString() );
}

void SSIModifyTask::freeIdOnError()
{
	if ( m_oldItem.isValid() && m_newItem.isValid() )
	{
		if ( m_opSubject == Contact || m_opSubject == NoSubject )
		{
			if ( m_oldItem.bid() != m_newItem.bid() )
				m_ssiManager->removeID( m_newItem );
		}
		else if ( m_opSubject == Group )
		{
			if ( m_oldItem.gid() != m_newItem.gid() )
				m_ssiManager->removeID( m_newItem );
		}
	}
	else if ( m_newItem.isValid() && !m_oldItem )
	{
		if ( m_opSubject == Group || m_opSubject == Contact ||
		     m_opSubject == NoSubject )
		{
			m_ssiManager->removeID( m_newItem );
		}
	}
}

void SSIModifyTask::sendEditStart()
{
	SNAC editStartSnac = { 0x0013, 0x0011, 0x0000, client()->snacSequence() };
	FLAP editStart = { 0x02, 0, 10 };
	Buffer* emptyBuffer = new Buffer;
	Transfer* t1 = createTransfer( editStart, editStartSnac, emptyBuffer );
	send( t1 );
}

void SSIModifyTask::sendEditEnd()
{
	SNAC editEndSnac = { 0x0013, 0x0012, 0x0000, client()->snacSequence() };
	FLAP editEnd = { 0x02, 0, 10 } ;
	Buffer* emptyBuffer = new Buffer;
	Transfer *t5 = createTransfer( editEnd, editEndSnac, emptyBuffer );
	send( t5 );
}

void SSIModifyTask::addItemToBuffer( OContact item, Buffer* buffer )
{
	buffer->addBSTR( item.name().toUtf8() );
	buffer->addWord( item.gid() );
	buffer->addWord( item.bid() );
	buffer->addWord( item.type() );
	buffer->addWord( item.tlvListLength() );

	QList<TLV>::const_iterator it =  item.tlvList().begin();
	QList<TLV>::const_iterator listEnd = item.tlvList().end();
	for( ; it != listEnd; ++it )
		buffer->addTLV( ( *it ) );
}

OContact SSIModifyTask::getItemFromBuffer( Buffer* buffer ) const
{
	QList<TLV> tlvList;

	QString itemName = QString::fromUtf8( buffer->getBSTR() );
	Oscar::WORD groupId = buffer->getWord();
	Oscar::WORD itemId = buffer->getWord();
	Oscar::WORD itemType = buffer->getWord();
	Oscar::WORD tlvLength = buffer->getWord();
	for ( int i = 0; i < tlvLength; )
	{
		TLV t = buffer->getTLV();
		i += 4;
		i += t.length;
		tlvList.append( t );
	}

	if ( itemType == ROSTER_CONTACT )
		itemName = Oscar::normalize( itemName );

	return OContact( itemName, groupId, itemId, itemType, tlvList );
}

void SSIModifyTask::handleContactAdd()
{
	Buffer* b = transfer()->buffer();

	while ( b->bytesAvailable() > 0 )
	{
		OContact item = getItemFromBuffer( b );
		kDebug(OSCAR_RAW_DEBUG) << "Adding " << item.name() << " to SSI manager";

		if ( item.type() == ROSTER_GROUP )
			m_ssiManager->newGroup( item );
		else if ( item.type() == ROSTER_CONTACT )
			m_ssiManager->newContact( item );
		else
			m_ssiManager->newItem( item );
	}
}

void SSIModifyTask::handleContactUpdate()
{
	Buffer* b = transfer()->buffer();

	while ( b->bytesAvailable() > 0 )
	{
		OContact item = getItemFromBuffer( b );
		kDebug(OSCAR_RAW_DEBUG) << "Updating " << item.name() << " in SSI manager";

		if ( item.type() == ROSTER_GROUP )
			m_ssiManager->updateGroup( item );
		else if ( item.type() == ROSTER_CONTACT )
			m_ssiManager->updateContact( item );
		else
			m_ssiManager->updateItem( item );
	}
}

void SSIModifyTask::handleContactRemove()
{
	Buffer* b = transfer()->buffer();

	while ( b->bytesAvailable() > 0 )
	{
		OContact item = getItemFromBuffer( b );
		kDebug(OSCAR_RAW_DEBUG) << "Removing " << item.name() << " from SSI manager";

		if ( item.type() == ROSTER_GROUP )
			m_ssiManager->removeGroup( item );
		else if ( item.type() == ROSTER_CONTACT )
			m_ssiManager->removeContact( item );
		else
			m_ssiManager->removeItem( item );
	}
}

//kate: tab-width 4; indent-mode csands;
