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
#include <kdebug.h>
#include "oscarutils.h"

// -------------------------------------------------------------------

class ContactManagerPrivate
{
public:
	QList<OContact> contactList;
	WORD lastModTime;
	WORD maxContacts;
	WORD maxGroups;
	WORD maxVisible;
	WORD maxInvisible;
	WORD maxIgnore;
	WORD nextContactId;
	WORD nextGroupId;
};

ContactManager::ContactManager( QObject *parent )
 : QObject(parent)
{
	d = new ContactManagerPrivate;
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
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Clearing the SSI list" << endl;
		QList<OContact>::iterator it = d->contactList.begin();

		while ( it != d->contactList.end() && d->contactList.count() > 0 )
			it = d->contactList.erase( it );
	};
}

WORD ContactManager::nextContactId()
{
	d->nextContactId++;
	return d->nextContactId;
}

WORD ContactManager::nextGroupId()
{
	d->nextGroupId++;
	return d->nextGroupId;
}

WORD ContactManager::numberOfItems() const
{
	return d->contactList.count();
}

DWORD ContactManager::lastModificationTime() const
{
	return d->lastModTime;
}

void ContactManager::setLastModificationTime( DWORD lastTime )
{
	d->lastModTime = lastTime;
}

void ContactManager::setParameters( WORD maxContacts, WORD maxGroups, WORD maxVisible, WORD maxInvisible, WORD maxIgnore )
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
	QList<OContact>::const_iterator it, listEnd = d->contactList.end();

	for ( it = d->contactList.begin(); it != listEnd; ++it )
	{
		OContact s = ( *it );
		if ( s == item )
			return true;
	}

	return false;
}

OContact ContactManager::findGroup( const QString &group ) const
{
	QList<OContact>::const_iterator it, listEnd = d->contactList.end();

	for ( it = d->contactList.begin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_GROUP && (*it ).name().toLower() == group.toLower() )
			return ( *it );


	return m_dummyItem;
}

OContact ContactManager::findGroup( int groupId ) const
{
	QList<OContact>::const_iterator it, listEnd = d->contactList.end();

	for ( it = d->contactList.begin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_GROUP && (*it ).gid() == groupId )
			return ( *it );

	return m_dummyItem;
}

OContact ContactManager::findContact( const QString &contact, const QString &group ) const
{

	if ( contact.isNull() || group.isNull() )
	{
		kWarning(OSCAR_RAW_DEBUG) << k_funcinfo <<
			"Passed NULL name or group string, aborting!" << endl;

		return m_dummyItem;
	}

	OContact gr = findGroup( group ); // find the parent group
	if ( gr.isValid() )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "gr->name= " << gr.name() <<
			", gr->gid= " << gr.gid() <<
			", gr->bid= " << gr.bid() <<
			", gr->type= " << gr.type() << endl;

		QList<OContact>::const_iterator it, listEnd = d->contactList.end();

		for ( it = d->contactList.begin(); it != listEnd; ++it )
		{
			if ( ( *it ).type() == ROSTER_CONTACT && (*it ).name() == contact && (*it ).gid() == gr.gid() )
			{
				//we have found our contact
				kDebug(OSCAR_RAW_DEBUG) << k_funcinfo <<
					"Found contact " << contact << " in SSI data" << endl;
				 return ( *it );
			}
		}
	}
	else
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo <<
			"ERROR: Group '" << group << "' not found!" << endl;
	}
	return m_dummyItem;
}

OContact ContactManager::findContact( const QString &contact ) const
{

	QList<OContact>::const_iterator it, listEnd = d->contactList.end();

	for ( it = d->contactList.begin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_CONTACT && (*it ).name() == contact )
			return ( *it );

	return m_dummyItem;
}

OContact ContactManager::findContact( int contactId ) const
{
	QList<OContact>::const_iterator it,  listEnd = d->contactList.end();

	for ( it = d->contactList.begin(); it!= listEnd; ++it )
		if ( ( *it ).type() == ROSTER_CONTACT && ( *it ).bid() == contactId )
			return ( *it );

	return m_dummyItem;
}

OContact ContactManager::findItemForIcon( QByteArray iconHash ) const
{
	QList<OContact>::const_iterator it,  listEnd = d->contactList.end();

	for ( it = d->contactList.begin(); it!= listEnd; ++it )
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
				OContact s = ( *it );
				return s;
			}
		}
	}
	return m_dummyItem;
}

OContact ContactManager::findItemForIconByRef( int ref ) const
{
	QList<OContact>::const_iterator it,  listEnd = d->contactList.end();

	for ( it = d->contactList.begin(); it!= listEnd; ++it )
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
	QList<OContact>::const_iterator it,  listEnd = d->contactList.end();

	for ( it = d->contactList.begin(); it!= listEnd; ++it )
		if ( ( *it ).type() == type && ( *it ).name() == contact )
			return ( *it );

	return m_dummyItem;
}

QList<OContact> ContactManager::groupList() const
{
	QList<OContact> list;

	QList<OContact>::const_iterator it, listEnd = d->contactList.end();
	for ( it = d->contactList.begin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_GROUP  )
			list.append( ( *it ) );

	return list;
}

QList<OContact> ContactManager::contactList() const
{
	QList<OContact> list;

	QList<OContact>::const_iterator it, listEnd = d->contactList.end();
	for ( it = d->contactList.begin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_CONTACT  )
			list.append( ( *it ) );

	return list;
}

QList<OContact> ContactManager::visibleList() const
{
	QList<OContact> list;

	QList<OContact>::const_iterator it, listEnd = d->contactList.end();
	for ( it = d->contactList.begin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_VISIBLE  )
			list.append( ( *it ) );

	return list;
}

QList<OContact> ContactManager::invisibleList() const
{
	QList<OContact> list;

	QList<OContact>::const_iterator it, listEnd = d->contactList.end();
	for ( it = d->contactList.begin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_INVISIBLE  )
			list.append( ( *it ) );

	return list;
}

QList<OContact> ContactManager::contactsFromGroup( const QString &group ) const
{
	QList<OContact> list;

	OContact gr = findGroup( group );
	if ( gr.isValid() )
	{
		QList<OContact>::const_iterator it, listEnd = d->contactList.end();
		for ( it = d->contactList.begin(); it != listEnd; ++it )
			if ( ( *it ).type() == ROSTER_CONTACT && (*it ).gid() == gr.gid() )
				list.append( ( *it ) );
	}
	return list;
}

QList<OContact> ContactManager::contactsFromGroup( int groupId ) const
{
	QList<OContact> list;

	QList<OContact>::const_iterator it, listEnd = d->contactList.end();
	for ( it = d->contactList.begin(); it != listEnd; ++it )
		if ( ( *it ).type() == ROSTER_CONTACT && (*it ).gid() == groupId  )
			list.append( ( *it ) );

	return list;
}

OContact ContactManager::visibilityItem() const
{
	OContact item = m_dummyItem;
	QList<OContact>::const_iterator it, listEnd = d->contactList.end();
	for ( it = d->contactList.begin(); it != listEnd; ++it )
	{
		if ( ( *it ).type() == 0x0004 )
		{
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Found visibility setting" << endl;
			item = ( *it );
			return item;
		}
	}

	return item;
}

bool ContactManager::listComplete() const
{
	//if last modification time is zero, we're not done
	//getting the list
	return d->lastModTime != 0;
}

bool ContactManager::newGroup( const OContact& group )
{
	//trying to find the group by its ID
	QList<OContact>::iterator it, listEnd = d->contactList.end();
	if ( findGroup( group.name() ).isValid() )
		return false;

	if ( !group.name().isEmpty() ) //avoid the group with gid 0 and bid 0
	{	// the group is really new
		kDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Adding group '" << group.name() << "' to SSI list" << endl;
		if ( group.gid() > d->nextGroupId )
			d->nextGroupId = group.gid();

		d->contactList.append( group );
		emit groupAdded( group );
		return true;
	}
	return false;
}

bool ContactManager::updateGroup( const OContact& oldGroup, const OContact& newGroup )
{
	if ( d->contactList.removeAll( oldGroup ) == 0 )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "No group were removed." << endl;
		return false;
	}
	
	if ( d->contactList.contains( newGroup ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "New group is already in list." << endl;
		return false;
	}
	
	kDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Updating group '" << newGroup.name() << "' in SSI list" << endl;
	d->contactList.append( newGroup );
	emit groupUpdated( newGroup );
	
	return true;
}

bool ContactManager::removeGroup( const OContact& group )
{
	QString groupName = group.name();
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Removing group " << group.name() << endl;
	int remcount = d->contactList.removeAll( group );
	if ( remcount == 0 )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "No groups removed" << endl;
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
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Group " << group << " not found." << endl;

	return false;
}

bool ContactManager::newContact( const OContact& contact )
{
	//what to validate?
	if ( contact.bid() > d->nextContactId )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Setting next contact ID to " << contact.bid() << endl;
		d->nextContactId = contact.bid();
	}

	if ( d->contactList.contains( contact ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "New contact is already in list." << endl;
		return false;
	}
		
	kDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Adding contact '" << contact.name() << "' to SSI list" << endl;
	d->contactList.append( contact );
	emit contactAdded( contact );
	return true;
}

bool ContactManager::updateContact( const OContact& oldContact, const OContact& newContact )
{
	if ( d->contactList.removeAll( oldContact ) == 0 )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "No contacts were removed." << endl;
		return false;
	}
	
	if ( d->contactList.contains( newContact ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "New contact is already in list." << endl;
		return false;
	}
	
	kDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Updating contact '" << newContact.name() << "' in SSI list" << endl;
	d->contactList.append( newContact );
	emit contactUpdated( newContact );
	
	return true;
}

bool ContactManager::removeContact( const OContact& contact )
{
	QString contactName = contact.name();
	int remcount = d->contactList.removeAll( contact );

	if ( remcount == 0 )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "No contacts were removed." << endl;
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
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Contact " << contact << " not found." << endl;

	return false;
}

bool ContactManager::newItem( const OContact& item )
{
	if ( item.bid() > d->nextContactId )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Setting next contact ID to " << item.bid() << endl;
		d->nextContactId = item.bid();
	}

	//no error checking for now
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Adding item " << item.toString() << endl;
	d->contactList.append( item );
	return true;
}

bool ContactManager::removeItem( const OContact& item )
{
	d->contactList.removeAll( item );
	return true;
}

#include "contactmanager.moc"

//kate: tab-width 4; indent-mode csands;
