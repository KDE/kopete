/*
	Kopete Oscar Protocol

	Copyright ( c ) 2004 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
	Copyright ( c ) 2004 Matt Rogers <mattr@kde.org>

	Kopete ( c ) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

	based on ssidata.h and ssidata.cpp ( c ) 2002 Tom Linsky <twl6@po.cwru.edu>

	*************************************************************************
	*                                                                       *
	* This library is free software; you can redistribute it and/or         *
	* modify it under the terms of the GNU Lesser General Public            *
	* License as published by the Free Software Foundation; either          *
	* version 2 of the License, or ( at your option ) any later version.    *
	*                                                                       *
	*************************************************************************
*/

#include "contactmanager.h"

#include <QtCore/QSet>

#include <kdebug.h>

#include "oscarutils.h"

// -------------------------------------------------------------------

class ContactManagerPrivate
{
public:
	QList<OContact> contactList;
	QSet<Oscar::WORD> itemIdSet;
	QSet<Oscar::WORD> groupIdSet;
	bool complete;
	Oscar::DWORD lastModTime;
	Oscar::WORD maxContacts;
	Oscar::WORD maxGroups;
	Oscar::WORD maxVisible;
	Oscar::WORD maxInvisible;
	Oscar::WORD maxIgnore;
	Oscar::WORD nextContactId;
	Oscar::WORD nextGroupId;
};

ContactManager::ContactManager( QObject *parent )
 : QObject(parent)
{
	d = new ContactManagerPrivate;
	d->complete = false;
	d->lastModTime = 0;
	d->nextContactId = 0;
	d->nextGroupId = 0;
	d->maxContacts = 999;
	d->maxGroups = 999;
	d->maxIgnore = 999;
	d->maxInvisible = 999;
	d->maxVisible = 999;
}


ContactManager::~ContactManager()
{
	clear();
	delete d;
}

void ContactManager::clear()
{
	//delete all Contacts from the list
	if ( d->contactList.count() > 0 )
	{
		kDebug(OSCAR_RAW_DEBUG) << "Clearing the SSI list";
		QList<OContact>::iterator it = d->contactList.begin();

		while ( it != d->contactList.end() && d->contactList.count() > 0 )
			it = d->contactList.erase( it );
	};

	d->itemIdSet.clear();
	d->groupIdSet.clear();
	d->complete = false;
	d->lastModTime = 0;
	d->nextContactId = 0;
	d->nextGroupId = 0;
}

Oscar::WORD ContactManager::nextContactId()
{
	if ( d->nextContactId == 0 )
		d->nextContactId++;

	d->nextContactId = findFreeId( d->itemIdSet, d->nextContactId );
	if ( d->nextContactId == 0xFFFF )
	{
		kWarning(OSCAR_RAW_DEBUG) << "No free id!";
		return 0xFFFF;
	}

	d->itemIdSet.insert( d->nextContactId );
	return d->nextContactId++;
}

Oscar::WORD ContactManager::nextGroupId()
{
	if ( d->nextGroupId == 0 )
		d->nextGroupId++;

	d->nextGroupId = findFreeId( d->groupIdSet, d->nextGroupId );
	if ( d->nextGroupId == 0xFFFF )
	{
		kWarning(OSCAR_RAW_DEBUG) << "No free group id!";
		return 0xFFFF;
	}

	d->groupIdSet.insert( d->nextGroupId );
	return d->nextGroupId++;
}

Oscar::WORD ContactManager::numberOfItems() const
{
	return d->contactList.count();
}

Oscar::DWORD ContactManager::lastModificationTime() const
{
	return d->lastModTime;
}

void ContactManager::setLastModificationTime( Oscar::DWORD lastTime )
{
	d->lastModTime = lastTime;
}

void ContactManager::setParameters( Oscar::WORD maxContacts, Oscar::WORD maxGroups, Oscar::WORD maxVisible, Oscar::WORD maxInvisible, Oscar::WORD maxIgnore )
{
	//I'm not using k_funcinfo for these debug statements because of
	//the function's long signature
	QString funcName = QString::fromLatin1( "[void ContactManager::setParameters] " );
	kDebug(OSCAR_RAW_DEBUG) << funcName << "Max number of contacts allowed in SSI: "
		<< maxContacts << endl;
	kDebug(OSCAR_RAW_DEBUG) << funcName << "Max number of groups allowed in SSI: "
		<< maxGroups << endl;
	kDebug(OSCAR_RAW_DEBUG) << funcName << "Max number of contacts allowed on visible list: "
		<< maxVisible << endl;
	kDebug(OSCAR_RAW_DEBUG) << funcName << "Max number of contacts allowed on invisible list: "
		<< maxInvisible << endl;
	kDebug(OSCAR_RAW_DEBUG) << funcName << "Max number of contacts allowed on ignore list: "
		<< maxIgnore << endl;

	d->maxContacts = maxContacts;
	d->maxGroups = maxGroups;
	d->maxInvisible = maxInvisible;
	d->maxVisible = maxVisible;
	d->maxIgnore = maxIgnore;
}

void ContactManager::loadFromExisting( const QList<OContact*>& newList )
{
	Q_UNUSED( newList );
	//FIXME: NOT Implemented!
}

bool ContactManager::hasItem( const OContact& item ) const
{
	QList<OContact>::const_iterator it, listEnd = d->contactList.constEnd();

	for ( it = d->contactList.constBegin(); it != listEnd; ++it )
	{
		OContact s = ( *it );
		if ( s == item )
			return true;
	}

	return false;
}

OContact ContactManager::findGroup( const QString &group ) const
{
	QList<OContact>::const_iterator it, listEnd = d->contactList.constEnd();

	for ( it = d->contactList.constBegin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_GROUP && (*it ).name().toLower() == group.toLower() )
			return ( *it );


	return m_dummyItem;
}

OContact ContactManager::findGroup( int groupId ) const
{
	QList<OContact>::const_iterator it, listEnd = d->contactList.constEnd();

	for ( it = d->contactList.constBegin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_GROUP && (*it ).gid() == groupId )
			return ( *it );

	return m_dummyItem;
}

OContact ContactManager::findContact( const QString &contact, const QString &group ) const
{

	if ( contact.isNull() || group.isNull() )
	{
		kWarning(OSCAR_RAW_DEBUG) <<
			"Passed NULL name or group string, aborting!" << endl;

		return m_dummyItem;
	}

	OContact gr = findGroup( group ); // find the parent group
	if ( gr.isValid() )
	{
		kDebug(OSCAR_RAW_DEBUG) << "gr->name= " << gr.name() <<
			", gr->gid= " << gr.gid() <<
			", gr->bid= " << gr.bid() <<
			", gr->type= " << gr.type() << endl;

		QList<OContact>::const_iterator it, listEnd = d->contactList.constEnd();

		for ( it = d->contactList.constBegin(); it != listEnd; ++it )
		{
			if ( ( *it ).type() == ROSTER_CONTACT && (*it ).name() == contact && (*it ).gid() == gr.gid() )
			{
				//we have found our contact
				kDebug(OSCAR_RAW_DEBUG) <<
					"Found contact " << contact << " in SSI data" << endl;
				 return ( *it );
			}
		}
	}
	else
	{
		kDebug(OSCAR_RAW_DEBUG) <<
			"ERROR: Group '" << group << "' not found!" << endl;
	}
	return m_dummyItem;
}

OContact ContactManager::findContact( const QString &contact ) const
{

	QList<OContact>::const_iterator it, listEnd = d->contactList.constEnd();

	for ( it = d->contactList.constBegin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_CONTACT && (*it ).name() == contact )
			return ( *it );

	return m_dummyItem;
}

OContact ContactManager::findContact( int contactId ) const
{
	QList<OContact>::const_iterator it,  listEnd = d->contactList.constEnd();

	for ( it = d->contactList.constBegin(); it!= listEnd; ++it )
		if ( ( *it ).type() == ROSTER_CONTACT && ( *it ).bid() == contactId )
			return ( *it );

	return m_dummyItem;
}

OContact ContactManager::findItemForIcon( QByteArray iconHash ) const
{
	QList<OContact>::const_iterator it,  listEnd = d->contactList.constEnd();

	for ( it = d->contactList.constBegin(); it!= listEnd; ++it )
	{
		if ( ( *it ).type() == ROSTER_BUDDYICONS )
		{
			TLV t = Oscar::findTLV( ( *it ).tlvList(), 0x00D5 );
			Buffer b(t.data);
			b.skipBytes(1); //don't care about flags
			Oscar::BYTE iconSize = b.getByte();
			QByteArray hash( b.getBlock( iconSize ) );
			if ( hash == iconHash )
			{
				OContact s = ( *it );
				return s;
			}
		}
	}
	return m_dummyItem;
}

OContact ContactManager::findItemForIconByRef( int ref ) const
{
	QList<OContact>::const_iterator it,  listEnd = d->contactList.constEnd();

	for ( it = d->contactList.constBegin(); it!= listEnd; ++it )
	{
		if ( ( *it ).type() == ROSTER_BUDDYICONS )
		{
			if ( ( *it ).name().toInt() == ref )
			{
				OContact s = ( *it );
				return s;
			}
		}
	}
	return m_dummyItem;
}

OContact ContactManager::findItem( const QString &contact, int type ) const
{
	QList<OContact>::const_iterator it,  listEnd = d->contactList.constEnd();

	for ( it = d->contactList.constBegin(); it!= listEnd; ++it )
		if ( ( *it ).type() == type && ( *it ).name() == contact )
			return ( *it );

	return m_dummyItem;
}

QList<OContact> ContactManager::groupList() const
{
	QList<OContact> list;

	QList<OContact>::const_iterator it, listEnd = d->contactList.constEnd();
	for ( it = d->contactList.constBegin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_GROUP  )
			list.append( ( *it ) );

	return list;
}

QList<OContact> ContactManager::contactList() const
{
	QList<OContact> list;

	QList<OContact>::const_iterator it, listEnd = d->contactList.constEnd();
	for ( it = d->contactList.constBegin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_CONTACT  )
			list.append( ( *it ) );

	return list;
}

QList<OContact> ContactManager::visibleList() const
{
	QList<OContact> list;

	QList<OContact>::const_iterator it, listEnd = d->contactList.constEnd();
	for ( it = d->contactList.constBegin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_VISIBLE  )
			list.append( ( *it ) );

	return list;
}

QList<OContact> ContactManager::invisibleList() const
{
	QList<OContact> list;

	QList<OContact>::const_iterator it, listEnd = d->contactList.constEnd();
	for ( it = d->contactList.constBegin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_INVISIBLE  )
			list.append( ( *it ) );

	return list;
}

QList<OContact> ContactManager::ignoreList() const
{
	QList<OContact> list;
	
	QList<OContact>::const_iterator it, listEnd = d->contactList.constEnd();
	for ( it = d->contactList.constBegin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_IGNORE  )
			list.append( ( *it ) );
	
	return list;
}

QList<OContact> ContactManager::contactsFromGroup( const QString &group ) const
{
	QList<OContact> list;

	OContact gr = findGroup( group );
	if ( gr.isValid() )
	{
		QList<OContact>::const_iterator it, listEnd = d->contactList.constEnd();
		for ( it = d->contactList.constBegin(); it != listEnd; ++it )
			if ( ( *it ).type() == ROSTER_CONTACT && (*it ).gid() == gr.gid() )
				list.append( ( *it ) );
	}
	return list;
}

QList<OContact> ContactManager::contactsFromGroup( int groupId ) const
{
	QList<OContact> list;

	QList<OContact>::const_iterator it, listEnd = d->contactList.constEnd();
	for ( it = d->contactList.constBegin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_CONTACT && (*it ).gid() == groupId  )
			list.append( ( *it ) );

	return list;
}

OContact ContactManager::visibilityItem() const
{
	OContact item = m_dummyItem;
	QList<OContact>::const_iterator it, listEnd = d->contactList.constEnd();
	for ( it = d->contactList.constBegin(); it != listEnd; ++it )
	{
		if ( ( *it ).type() == 0x0004 && ( *it ).name().isEmpty() )
		{
			kDebug(OSCAR_RAW_DEBUG) << "Found visibility setting";
			item = ( *it );
			return item;
		}
	}

	return item;
}

void ContactManager::setListComplete( bool complete )
{
	d->complete = complete;
}

bool ContactManager::listComplete() const
{
	return d->complete;
}

bool ContactManager::newGroup( const OContact& group )
{
	//trying to find the group by its ID
	if ( findGroup( group.name() ).isValid() )
		return false;

	if ( !group.name().isEmpty() ) //avoid the group with gid 0 and bid 0
	{	// the group is really new
		kDebug( OSCAR_RAW_DEBUG ) << "Adding group '" << group.name() << "' to SSI list";

		addID( group );
		d->contactList.append( group );
		emit groupAdded( group );
		return true;
	}
	return false;
}

bool ContactManager::updateGroup( const OContact& group )
{
	OContact oldGroup = findGroup( group.name() );

	if ( oldGroup.isValid() )
	{
		removeID( oldGroup );
		d->contactList.removeAll( oldGroup );
	}

	if ( d->contactList.contains( group ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << "New group is already in list.";
		return false;
	}

	kDebug( OSCAR_RAW_DEBUG ) << "Updating group '" << group.name() << "' in SSI list";
	addID( group );
	d->contactList.append( group );
	emit groupUpdated( group );
	return true;
}

bool ContactManager::removeGroup( const OContact& group )
{
	QString groupName = group.name();
	kDebug(OSCAR_RAW_DEBUG) << "Removing group " << group.name();
	removeID( group );
	int remcount = d->contactList.removeAll( group );
	if ( remcount == 0 )
	{
		kDebug(OSCAR_RAW_DEBUG) << "No groups removed";
		return false;
	}

	emit groupRemoved( groupName );
	return true;
}

bool ContactManager::removeGroup( const QString &group )
{
	OContact gr = findGroup( group );

	if ( gr.isValid() && removeGroup( gr )  )
	{
		return true;
	}
	else
		kDebug(OSCAR_RAW_DEBUG) << "Group " << group << " not found.";

	return false;
}

bool ContactManager::newContact( const OContact& contact )
{
	if ( d->contactList.contains( contact ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << "New contact is already in list.";
		return false;
	}
		
	kDebug( OSCAR_RAW_DEBUG ) << "Adding contact '" << contact.name() << "' to SSI list";
	addID( contact );
	d->contactList.append( contact );
	emit contactAdded( contact );
	return true;
}

bool ContactManager::updateContact( const OContact& contact )
{
	OContact oldContact = findContact( contact.name() );

	if ( oldContact.isValid() )
	{
		removeID( oldContact );
		d->contactList.removeAll( oldContact );
	}

	if ( d->contactList.contains( contact ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << "New contact is already in list.";
		return false;
	}

	kDebug( OSCAR_RAW_DEBUG ) << "Updating contact '" << contact.name() << "' in SSI list";
	addID( contact );
	d->contactList.append( contact );
	emit contactUpdated( contact );
	return true;
}

bool ContactManager::removeContact( const OContact& contact )
{
	QString contactName = contact.name();
	removeID( contact );
	int remcount = d->contactList.removeAll( contact );

	if ( remcount == 0 )
	{
		kDebug(OSCAR_RAW_DEBUG) << "No contacts were removed.";
		return false;
	}

	emit contactRemoved( contactName );
	return true;
}

bool ContactManager::removeContact( const QString &contact )
{
	OContact ct = findContact( contact );

	if ( ct.isValid() && removeContact( ct ) )
		return true;
	else
		kDebug(OSCAR_RAW_DEBUG) << "Contact " << contact << " not found.";

	return false;
}

bool ContactManager::newItem( const OContact& item )
{
	if ( d->contactList.contains( item ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << "Item is already in list.";
		return false;
	}

	kDebug(OSCAR_RAW_DEBUG) << "Adding item " << item.toString();
	addID( item );
	d->contactList.append( item );
	return true;
}

bool ContactManager::updateItem( const OContact& item )
{
	OContact oldItem = findItem( item.name(), item.type() );

	if ( oldItem.isValid() )
	{
		removeID( oldItem );
		d->contactList.removeAll( oldItem );
	}

	if ( d->contactList.contains( item ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << "New item is already in list.";
		return false;
	}

	kDebug( OSCAR_RAW_DEBUG ) << "Updating item in SSI list";
	addID( item );
	d->contactList.append( item );
	return true;
}

bool ContactManager::removeItem( const OContact& item )
{
	removeID( item );
	int remcount = d->contactList.removeAll( item );

	if ( remcount == 0 )
	{
		kDebug(OSCAR_RAW_DEBUG) << "No items were removed.";
		return false;
	}

	return true;
}

void ContactManager::addID( const OContact& item )
{
	if ( item.type() == ROSTER_GROUP )
		d->groupIdSet.insert( item.gid() );
	else
		d->itemIdSet.insert( item.bid() );
}

void ContactManager::removeID( const OContact& item )
{
	if ( item.type() == ROSTER_GROUP )
	{
		d->groupIdSet.remove( item.gid() );

		if ( d->nextGroupId > item.gid() )
			d->nextGroupId = item.gid();
	}
	else
	{
		d->itemIdSet.remove( item.bid() );

		if ( d->nextContactId > item.bid() )
			d->nextContactId = item.bid();
	}
}

Oscar::WORD ContactManager::findFreeId( const QSet<Oscar::WORD>& idSet, Oscar::WORD fromId ) const
{
	for ( Oscar::WORD id = fromId; id < 0x8000; id++ )
	{
		if ( !idSet.contains( id ) )
			return id;
	}

	return 0xFFFF;
}

#include "contactmanager.moc"

//kate: tab-width 4; indent-mode csands;
