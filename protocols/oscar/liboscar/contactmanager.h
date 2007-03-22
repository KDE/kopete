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
	* version 2 of the License, or ( at your option ) any later version.      *
	*                                                                       *
	*************************************************************************
*/

#ifndef CONTACTMANAGER_H
#define CONTACTMANAGER_H

#include <qobject.h>
#include <QList>

#include "oscartypes.h"
#include "oscartypeclasses.h"
#include "contact.h"

using namespace Oscar;

class ContactManagerPrivate;

/**
Contact management

@author Gustavo Pichorim Boiko
@author Matt Rogers
*/
class LIBOSCAR_EXPORT ContactManager : public QObject
{
        Q_OBJECT
public:
	ContactManager( QObject* parent = 0 );

	~ContactManager();
	
	/** Clear the internal Contact list */
	void clear();
	
	/** Get the next buddy id for an Contact item */
	Oscar::WORD nextContactId();
	
	/** Get the next group id for an Contact item */
	Oscar::WORD nextGroupId();

	/** Get the number of items in the Contact list. */
	Oscar::WORD numberOfItems() const;

	/** Get the timestamp the list was last modified */
	Oscar::DWORD lastModificationTime() const;

	/** Set the timestamp of the last modification time */
	void setLastModificationTime( Oscar::DWORD lastTime );

	/** Set the parameters we should use for Contact */
	void setParameters( Oscar::WORD maxContacts, Oscar::WORD maxGroups, Oscar::WORD maxVisible,
	                    Oscar::WORD maxInvisible, Oscar::WORD maxIgnore );

	/**
	 * Load an existing list from Contact objects.
	 * The current Contact list will be overwritten and it's contents
	 * replaced with the data from the new list
	 */
	void loadFromExisting( const QList<OContact*>& newList );
	
	bool hasItem( const OContact& item ) const;

	OContact findGroup( const QString& group ) const;
	OContact findGroup( int groupId ) const;
	

	OContact findContact( const QString& contact, const QString& group ) const;
	OContact findContact( const QString& contact ) const;
	OContact findContact( int contactId ) const;
	
	OContact findItemForIcon( QByteArray iconHash ) const;
	OContact findItemForIconByRef( int ) const;
	
	OContact findItem( const QString &contact, int type ) const;

	QList<OContact> groupList() const;
	QList<OContact> contactList() const;
	QList<OContact> visibleList() const;
	QList<OContact> invisibleList() const;
	QList<OContact> ignoreList() const;
	QList<OContact> contactsFromGroup( const QString& group ) const;
	QList<OContact> contactsFromGroup( int groupId ) const;
	
	OContact visibilityItem() const;

	void setListComplete( bool complete );
	bool listComplete() const;

public slots:
	bool newGroup( const OContact& group );
	bool updateGroup( const OContact& group );
	bool removeGroup( const OContact& group );
	bool removeGroup( const QString& group );

	bool newContact( const OContact& contact );
	bool updateContact( const OContact& contact );
	bool removeContact( const OContact& contact );
	bool removeContact( const QString& contact );
	
	bool newItem( const OContact& item );
	bool updateItem( const OContact& item );
	bool removeItem( const OContact& item );

	void addID( const OContact& item );
	void removeID( const OContact& item );

signals:
	
	//! Emitted when we've added a new contact to the list
	void contactAdded( const OContact& );
	
	//! Emitted when we've updated a contact in the list
	void contactUpdated( const OContact& );
	
	//! Emitted when we've removed a contact from the list
	void contactRemoved( const QString& contactName );
	
	//! Emitted when we've added a new group to the list
	void groupAdded( const OContact& );
	
	//! Emitted when we've updated a group in the list
	void groupUpdated( const OContact& );
	
	//! Emitted when we've removed a group from the ssi list
	void groupRemoved( const QString& groupName );
	
	void modifyError( const QString& error );
	
private:
	Oscar::WORD findFreeId( const QSet<Oscar::WORD>& idSet, Oscar::WORD fromId ) const;
	
	ContactManagerPrivate* d;
	OContact m_dummyItem;
};

#endif

//kate: tab-width 4; indent-mode csands;
