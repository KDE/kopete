/*
	Kopete Oscar Protocol
	ssimanager.cpp - SSI management

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

#include "ssimanager.h"
#include <kdebug.h>
#include "oscarutils.h"

// -------------------------------------------------------------------

class SSIManagerPrivate
{
public:
	QValueList<Oscar::SSI> SSIList;
	QValueList<WORD> groupIdList;
	QValueList<WORD> itemIdList;
	bool complete;
	DWORD lastModTime;
	WORD maxContacts;
	WORD maxGroups;
	WORD maxVisible;
	WORD maxInvisible;
	WORD maxIgnore;
	WORD nextContactId;
	WORD nextGroupId;
};

SSIManager::SSIManager( QObject *parent, const char *name )
 : QObject( parent, name )
{
	d = new SSIManagerPrivate;
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


SSIManager::~SSIManager()
{
	clear();
	delete d;
}

void SSIManager::clear()
{
	//delete all SSIs from the list
	if ( d->SSIList.count() > 0 )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Clearing the SSI list" << endl;
		QValueList<Oscar::SSI>::iterator it = d->SSIList.begin();
		
		while ( it != d->SSIList.end() && d->SSIList.count() > 0 )
			it = d->SSIList.remove( it );
	};
	
	d->itemIdList.clear();
	d->groupIdList.clear();
	d->complete = false;
	d->lastModTime = 0;
	d->nextContactId = 0;
	d->nextGroupId = 0;
}

WORD SSIManager::nextContactId()
{
	if ( d->nextContactId == 0 )
		d->nextContactId++;
	
	d->nextContactId = findFreeId( d->itemIdList, d->nextContactId );
	
	if ( d->nextContactId == 0xFFFF )
	{
		kdWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "No free id!" << endl;
		return 0xFFFF;
	}
	
	if ( d->itemIdList.contains( d->nextContactId ) == 0 )
		d->itemIdList.append( d->nextContactId );
		
	return d->nextContactId++;
}

WORD SSIManager::nextGroupId()
{
	if ( d->nextGroupId == 0 )
		d->nextGroupId++;
	
	d->nextGroupId = findFreeId( d->groupIdList, d->nextGroupId );
	
	if ( d->nextGroupId == 0xFFFF )
	{
		kdWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "No free group id!" << endl;
		return 0xFFFF;
	}
	
	if ( d->groupIdList.contains( d->nextGroupId ) == 0 )
		d->groupIdList.append( d->nextGroupId );
	
	return d->nextGroupId++;
}

WORD SSIManager::numberOfItems() const
{
	return d->SSIList.count();
}

DWORD SSIManager::lastModificationTime() const
{
	return d->lastModTime;
}

void SSIManager::setLastModificationTime( DWORD lastTime )
{
	d->lastModTime = lastTime;
}

void SSIManager::setParameters( WORD maxContacts, WORD maxGroups, WORD maxVisible, WORD maxInvisible, WORD maxIgnore )
{
	//I'm not using k_funcinfo for these debug statements because of
	//the function's long signature
	QString funcName = QString::fromLatin1( "[void SSIManager::setParameters] " );
	kdDebug(OSCAR_RAW_DEBUG) << funcName << "Max number of contacts allowed in SSI: "
		<< maxContacts << endl;
	kdDebug(OSCAR_RAW_DEBUG) << funcName << "Max number of groups allowed in SSI: "
		<< maxGroups << endl;
	kdDebug(OSCAR_RAW_DEBUG) << funcName << "Max number of contacts allowed on visible list: "
		<< maxVisible << endl;
	kdDebug(OSCAR_RAW_DEBUG) << funcName << "Max number of contacts allowed on invisible list: "
		<< maxInvisible << endl;
	kdDebug(OSCAR_RAW_DEBUG) << funcName << "Max number of contacts allowed on ignore list: "
		<< maxIgnore << endl;
	
	d->maxContacts = maxContacts;
	d->maxGroups = maxGroups;
	d->maxInvisible = maxInvisible;
	d->maxVisible = maxVisible;
	d->maxIgnore = maxIgnore;
}

void SSIManager::loadFromExisting( const QValueList<Oscar::SSI*>& newList )
{
	Q_UNUSED( newList );
	//FIXME: NOT Implemented!
}

bool SSIManager::hasItem( const Oscar::SSI& item ) const
{
	QValueList<Oscar::SSI>::const_iterator it, listEnd = d->SSIList.end();
	
	for ( it = d->SSIList.begin(); it != listEnd; ++it )
	{
		Oscar::SSI s = ( *it );
		if ( s == item )
			return true;
	}
	
	return false;
}

Oscar::SSI SSIManager::findGroup( const QString &group ) const
{
	QValueList<Oscar::SSI>::const_iterator it, listEnd = d->SSIList.end();
	
	for ( it = d->SSIList.begin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_GROUP && (*it ).name().lower() == group.lower() )
			return ( *it );
	
	
	return m_dummyItem;
}

Oscar::SSI SSIManager::findGroup( int groupId ) const
{
	QValueList<Oscar::SSI>::const_iterator it, listEnd = d->SSIList.end();
	
	for ( it = d->SSIList.begin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_GROUP && (*it ).gid() == groupId )
			return ( *it );
	
	return m_dummyItem;
}

Oscar::SSI SSIManager::findContact( const QString &contact, const QString &group ) const
{
	
	if ( contact.isNull() || group.isNull() )
	{
		kdWarning(OSCAR_RAW_DEBUG) << k_funcinfo <<
			"Passed NULL name or group string, aborting!" << endl;
		
		return m_dummyItem;
	}
	
	Oscar::SSI gr = findGroup( group ); // find the parent group
	if ( gr.isValid() )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "gr->name= " << gr.name() <<
			", gr->gid= " << gr.gid() <<
			", gr->bid= " << gr.bid() <<
			", gr->type= " << gr.type() << endl;
	
		QValueList<Oscar::SSI>::const_iterator it, listEnd = d->SSIList.end();
		
		for ( it = d->SSIList.begin(); it != listEnd; ++it )
		{
			if ( ( *it ).type() == ROSTER_CONTACT && (*it ).name() == contact && (*it ).gid() == gr.gid() )
			{
				//we have found our contact
				kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo <<
					"Found contact " << contact << " in SSI data" << endl;
				 return ( *it );
			}
		}
	}
	else
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo <<
			"ERROR: Group '" << group << "' not found!" << endl;
	}
	return m_dummyItem;
}

Oscar::SSI SSIManager::findContact( const QString &contact ) const
{
	
	QValueList<Oscar::SSI>::const_iterator it, listEnd = d->SSIList.end();
	
	for ( it = d->SSIList.begin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_CONTACT && (*it ).name() == contact )
			return ( *it );
	
	return m_dummyItem;
}

Oscar::SSI SSIManager::findContact( int contactId ) const
{
	QValueList<Oscar::SSI>::const_iterator it,  listEnd = d->SSIList.end();
	
	for ( it = d->SSIList.begin(); it!= listEnd; ++it )
		if ( ( *it ).type() == ROSTER_CONTACT && ( *it ).bid() == contactId )
			return ( *it );
	
	return m_dummyItem;
}

Oscar::SSI SSIManager::findItemForIcon( QByteArray iconHash ) const
{
	QValueList<Oscar::SSI>::const_iterator it,  listEnd = d->SSIList.end();
	
	for ( it = d->SSIList.begin(); it!= listEnd; ++it )
	{
		if ( ( *it ).type() == ROSTER_BUDDYICONS )
		{
			TLV t = Oscar::findTLV( ( *it ).tlvList(), 0x00D5 );
			Buffer b(t.data);
			b.skipBytes(1); //don't care about flags
			BYTE iconSize = b.getByte();
			QByteArray hash( b.getBlock( iconSize ) );
			if ( hash == iconHash )
			{
				Oscar::SSI s = ( *it );
				return s;
			}
		}
	}
	return m_dummyItem;
}

Oscar::SSI SSIManager::findItemForIconByRef( int ref ) const
{
	QValueList<Oscar::SSI>::const_iterator it,  listEnd = d->SSIList.end();
	
	for ( it = d->SSIList.begin(); it!= listEnd; ++it )
	{
		if ( ( *it ).type() == ROSTER_BUDDYICONS )
		{
			if ( ( *it ).name().toInt() == ref )
			{
				Oscar::SSI s = ( *it );
				return s;
			}
		}
	}
	return m_dummyItem;	
}

Oscar::SSI SSIManager::findItem( const QString &contact, int type ) const
{
	QValueList<Oscar::SSI>::const_iterator it,  listEnd = d->SSIList.end();
	
	for ( it = d->SSIList.begin(); it!= listEnd; ++it )
		if ( ( *it ).type() == type && ( *it ).name() == contact )
			return ( *it );
	
	return m_dummyItem;
}

QValueList<Oscar::SSI> SSIManager::groupList() const
{
	QValueList<Oscar::SSI> list;
	
	QValueList<Oscar::SSI>::const_iterator it, listEnd = d->SSIList.end();
	for ( it = d->SSIList.begin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_GROUP  )
			list.append( ( *it ) );
	
	return list;
}

QValueList<Oscar::SSI> SSIManager::contactList() const
{
	QValueList<Oscar::SSI> list;
	
	QValueList<Oscar::SSI>::const_iterator it, listEnd = d->SSIList.end();
	for ( it = d->SSIList.begin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_CONTACT  )
			list.append( ( *it ) );
	
	return list;
}

QValueList<Oscar::SSI> SSIManager::visibleList() const
{
	QValueList<Oscar::SSI> list;
	
	QValueList<Oscar::SSI>::const_iterator it, listEnd = d->SSIList.end();
	for ( it = d->SSIList.begin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_VISIBLE  )
			list.append( ( *it ) );
	
	return list;
}

QValueList<Oscar::SSI> SSIManager::invisibleList() const
{
	QValueList<Oscar::SSI> list;
	
	QValueList<Oscar::SSI>::const_iterator it, listEnd = d->SSIList.end();
	for ( it = d->SSIList.begin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_INVISIBLE  )
			list.append( ( *it ) );
	
	return list;
}

QValueList<Oscar::SSI> SSIManager::contactsFromGroup( const QString &group ) const
{
	QValueList<Oscar::SSI> list;
	
	Oscar::SSI gr = findGroup( group );
	if ( gr.isValid() )
	{
		QValueList<Oscar::SSI>::const_iterator it, listEnd = d->SSIList.end();
		for ( it = d->SSIList.begin(); it != listEnd; ++it )
			if ( ( *it ).type() == ROSTER_CONTACT && (*it ).gid() == gr.gid() )
				list.append( ( *it ) );
	}
	return list;
}

QValueList<Oscar::SSI> SSIManager::contactsFromGroup( int groupId ) const
{
	QValueList<Oscar::SSI> list;
	
	QValueList<Oscar::SSI>::const_iterator it, listEnd = d->SSIList.end();
	for ( it = d->SSIList.begin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_CONTACT && (*it ).gid() == groupId  )
			list.append( ( *it ) );
	
	return list;
}

Oscar::SSI SSIManager::visibilityItem() const
{
	Oscar::SSI item = m_dummyItem;
	QValueList<Oscar::SSI>::const_iterator it, listEnd = d->SSIList.end();
	for ( it = d->SSIList.begin(); it != listEnd; ++it )
	{
		if ( ( *it ).type() == 0x0004 )
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Found visibility setting" << endl;
			item = ( *it );
			return item;
		}
	}
	
	return item;
}

void SSIManager::setListComplete( bool complete )
{
	d->complete = complete;
}

bool SSIManager::listComplete() const
{
	return d->complete;
}

bool SSIManager::newGroup( const Oscar::SSI& group )
{
	//trying to find the group by its ID
	QValueList<Oscar::SSI>::iterator it, listEnd = d->SSIList.end();
	if ( findGroup( group.name() ).isValid() )
		return false;

	if ( !group.name().isEmpty() ) //avoid the group with gid 0 and bid 0
	{	// the group is really new
		kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Adding group '" << group.name() << "' to SSI list" << endl;
		
		d->SSIList.append( group );
		addID( group );
		emit groupAdded( group );
		return true;
	}
	return false;
}

bool SSIManager::updateGroup( const Oscar::SSI& group )
{
	Oscar::SSI oldGroup = findGroup( group.name() );

	if ( oldGroup.isValid() )
	{
		removeID( oldGroup );
		d->SSIList.remove( oldGroup );
	}

	if ( d->SSIList.findIndex( group ) != -1 )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "New group is already in list." << endl;
		return false;
	}

	kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Updating group '" << group.name() << "' in SSI list" << endl;
	d->SSIList.append( group );
	addID( group );
	emit groupUpdated( group );

	return true;
}

bool SSIManager::removeGroup( const Oscar::SSI& group )
{
	QString groupName = group.name();
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Removing group " << group.name() << endl;
	int remcount = d->SSIList.remove( group );
	removeID( group );
	
	if ( remcount == 0 )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "No groups removed" << endl;
		return false;
	}
	
	emit groupRemoved( groupName );
	return true;
}

bool SSIManager::removeGroup( const QString &group )
{
	Oscar::SSI gr = findGroup( group );
	
	if ( gr.isValid() && removeGroup( gr )  )
	{
		return true;
	}
	else
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Group " << group << " not found." << endl;

	return false;
}

bool SSIManager::newContact( const Oscar::SSI& contact )
{
	if ( d->SSIList.findIndex( contact ) == -1 )
	{
		kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Adding contact '" << contact.name() << "' to SSI list" << endl;
		addID( contact );
		d->SSIList.append( contact );
		emit contactAdded( contact );
	}
	else
		return false;
	return true;
}

bool SSIManager::updateContact( const Oscar::SSI& contact )
{
	Oscar::SSI oldContact = findContact( contact.name() );

	if ( oldContact.isValid() )
	{
		removeID( oldContact );
		d->SSIList.remove( oldContact );
	}

	if ( d->SSIList.findIndex( contact ) != -1 )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "New contact is already in list." << endl;
		return false;
	}

	kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Updating contact '" << contact.name() << "' in SSI list" << endl;
	addID( contact );
	d->SSIList.append( contact );
	emit contactUpdated( contact );
	
	return true;
}

bool SSIManager::removeContact( const Oscar::SSI& contact )
{
	QString contactName = contact.name();
	int remcount = d->SSIList.remove( contact );
	removeID( contact );
	
	if ( remcount == 0 )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "No contacts were removed." << endl;
		return false;
	}
	
	emit contactRemoved( contactName );
	return true;
}

bool SSIManager::removeContact( const QString &contact )
{
	Oscar::SSI ct = findContact( contact );
	
	if ( ct.isValid() && removeContact( ct ) )
		return true;
	else
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Contact " << contact << " not found." << endl;
	
	return false;
}

bool SSIManager::newItem( const Oscar::SSI& item )
{
	if ( d->SSIList.findIndex( item ) != -1 )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Item is already in list." << endl;
		return false;
	}

	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Adding item " << item.toString() << endl;
	d->SSIList.append( item );
	addID( item );
	return true;
}

bool SSIManager::updateItem( const Oscar::SSI& item )
{
	Oscar::SSI oldItem = findItem( item.name(), item.type() );

	if ( oldItem.isValid() )
	{
		removeID( oldItem );
		d->SSIList.remove( oldItem );
	}

	if ( d->SSIList.findIndex( item ) != -1 )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "New item is already in list." << endl;
		return false;
	}

	kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Updating item in SSI list" << endl;
	addID( item );
	d->SSIList.append( item );
	return true;
}

bool SSIManager::removeItem( const Oscar::SSI& item )
{
	int remcount = d->SSIList.remove( item );
	removeID( item );

	if ( remcount == 0 )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "No items were removed." << endl;
		return false;
	}
	
	return true;
}

void SSIManager::addID( const Oscar::SSI& item )
{
	if ( item.type() == ROSTER_GROUP )
	{
		if ( d->groupIdList.contains( item.gid() ) == 0 )
			d->groupIdList.append( item.gid() );
	}
	else
	{
		if ( d->itemIdList.contains( item.bid() ) == 0 )
			d->itemIdList.append( item.bid() );
	}
}

void SSIManager::removeID( const Oscar::SSI& item )
{
	if ( item.type() == ROSTER_GROUP )
	{
		d->groupIdList.remove( item.gid() );
	
		if ( d->nextGroupId > item.gid() )
			d->nextGroupId = item.gid();
	}
	else
	{
		d->itemIdList.remove( item.bid() );
		
		if ( d->nextContactId > item.bid() )
			d->nextContactId = item.bid();
	}
}

WORD SSIManager::findFreeId( const QValueList<WORD>& idList, WORD fromId ) const
{
	for ( WORD id = fromId; id < 0x8000; id++ )
	{
		if ( idList.contains( id ) == 0 )
			return id;
	}
	
	return 0xFFFF;
}

#include "ssimanager.moc"

//kate: tab-width 4; indent-mode csands;
