/*
	Kopete Oscar Protocol
	ssimanager.h - SSI management

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

#ifndef SSIMANAGER_H
#define SSIMANAGER_H

#include <qobject.h>
#include <qvaluelist.h>

#include "oscartypes.h"
#include "oscartypeclasses.h"

using namespace Oscar;

class SSIManagerPrivate;

/**
SSI management

@author Gustavo Pichorim Boiko
@author Matt Rogers
*/
class KOPETE_EXPORT SSIManager : public QObject
{
        Q_OBJECT
public:
	SSIManager( QObject* parent = 0, const char* name = 0 );

	~SSIManager();
	
	/** Clear the internal SSI list */
	void clear();
	
	/** Get the next buddy id for an SSI item */
	WORD nextContactId();
	
	/** Get the next group id for an SSI item */
	WORD nextGroupId();

	/** Get the number of items in the SSI list. */
	WORD numberOfItems() const;

	/** Get the timestamp the list was last modified */
	DWORD lastModificationTime() const;

	/** Set the timestamp of the last modification time */
	void setLastModificationTime( DWORD lastTime );

	/** Set the parameters we should use for SSI */
	void setParameters( WORD maxContacts, WORD maxGroups, WORD maxVisible,
	                    WORD maxInvisible, WORD maxIgnore );

	/**
	 * Load an existing list from SSI objects.
	 * The current SSI list will be overwritten and it's contents
	 * replaced with the data from the new list
	 */
	void loadFromExisting( const QValueList<Oscar::SSI*>& newList );
	
	bool hasItem( const Oscar::SSI& item ) const;

	Oscar::SSI findGroup( const QString& group ) const;
	Oscar::SSI findGroup( int groupId ) const;
	

	Oscar::SSI findContact( const QString& contact, const QString& group ) const;
	Oscar::SSI findContact( const QString& contact ) const;
	Oscar::SSI findContact( int contactId ) const;
	
	Oscar::SSI findItemForIcon( QByteArray iconHash ) const;
	Oscar::SSI findItemForIconByRef( int ) const;
	
	Oscar::SSI findItem( const QString &contact, int type ) const;

	QValueList<Oscar::SSI> groupList() const;
	QValueList<Oscar::SSI> contactList() const;
	QValueList<Oscar::SSI> visibleList() const;
	QValueList<Oscar::SSI> invisibleList() const;
	QValueList<Oscar::SSI> contactsFromGroup( const QString& group ) const;
	QValueList<Oscar::SSI> contactsFromGroup( int groupId ) const;
	
	Oscar::SSI visibilityItem() const;

	void setListComplete( bool complete );
	bool listComplete() const;

public slots:
	bool newGroup( const Oscar::SSI& group );
	bool updateGroup( const Oscar::SSI& group );
	bool removeGroup( const Oscar::SSI& group );
	bool removeGroup( const QString& group );

	bool newContact( const Oscar::SSI& contact );
	bool updateContact( const Oscar::SSI& contact );
	bool removeContact( const Oscar::SSI& contact );
	bool removeContact( const QString& contact );
	
	bool newItem( const Oscar::SSI& item );
	bool updateItem( const Oscar::SSI& item );
	bool removeItem( const Oscar::SSI& item );

	void addID( const Oscar::SSI& item );
	void removeID( const Oscar::SSI& item );

signals:
	
	//! Emitted when we've added a new contact to the list
	void contactAdded( const Oscar::SSI& );
	
	//! Emitted when we've updated a contact in the list
	void contactUpdated( const Oscar::SSI& );
	
	//! Emitted when we've removed a contact from the list
	void contactRemoved( const QString& contactName );
	
	//! Emitted when we've added a new group to the list
	void groupAdded( const Oscar::SSI& );
	
	//! Emitted when we've updated a group in the list
	void groupUpdated( const Oscar::SSI& );
	
	//! Emitted when we've removed a group from the ssi list
	void groupRemoved( const QString& groupName );
	
	void modifyError( const QString& error );
	
private:
	WORD findFreeId( const QValueList<WORD>& idList, WORD fromId ) const;
		
	SSIManagerPrivate* d;
	Oscar::SSI m_dummyItem;
};

#endif

//kate: tab-width 4; indent-mode csands;
