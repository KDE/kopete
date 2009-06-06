/*
    gwcontactlist.h - Kopete GroupWise Protocol

    Copyright (c) 2006      Novell, Inc	 	 	 http://www.opensuse.org
    Copyright (c) 2005      SUSE Linux Products GmbH	 	 http://www.suse.com

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

// GROUPWISE SERVER SIDE CONTACT LIST MODEL

#include <qobject.h>

#ifndef GW_CONTACTLIST_H
#define GW_CONTACTLIST_H

class GWFolder;
class GWContactInstance;
class GWContactListItem;

typedef QList<GWContactInstance *> GWContactInstanceList;

	/**
	 *  These functions model the server side contact list structure enough to allow Kopete to manipulate it correctly
	 *  In GroupWise, a contact list is composed of folders, containing contacts.  But the contacts don't record which 
	 *  folders they are in.  Instead, each contact entry represents an instance of that contact within the list.  
	 *  In Kopete's model, this looks like duplicate contacts (illegal), so instead we have unique contacts, 
	 *  each (by way of its metacontact) knowing membership of potentially >1 KopeteGroups.  Contacts contain a list of the 
	 *  server side list instances.  Contact list management operations affect this list, which is updated during every
	 *  operation.  Having this list allows us to update the server side contact list and keep changes synchronized across
	 *  different clients.
	 *  The list is volatile - it is not stored in stable storage, but is purged on disconnect and recreated at login.
	 */
class GWContactList : public QObject
{
Q_OBJECT
public:
	GWContactList( QObject * parent );
	GWFolder * addFolder( unsigned int id, unsigned int sequence, const QString & displayName );
	GWContactInstance * addContactInstance( unsigned int id, unsigned int parent, unsigned int sequence, const QString & displayName, const QString & dn );
	GWFolder * findFolderById( unsigned int id );
	GWFolder * findFolderByName( const QString & name );
	GWContactInstanceList instancesWithDn( const QString & dn );
	void removeInstance( GWContactListItem * instance );
	void removeInstanceById( unsigned int id );
	int maxSequenceNumber();
	virtual void dump();
	void clear();
	GWFolder * rootFolder;
};

class GWContactListItem : public QObject
{
Q_OBJECT
public:
	GWContactListItem( QObject * parent, unsigned int theId, unsigned int theSequence, const QString & theDisplayName );

	unsigned int id;		// OBJECT_ID
	unsigned int sequence;	// SEQUENCE_NUMBER
	QString displayName;	// DISPLAY_NAME
};

class GWFolder : public GWContactListItem
{
Q_OBJECT
public:
	GWFolder( QObject * parent, unsigned int theId,  unsigned int theSequence, const QString & theDisplayName );
	virtual void dump( unsigned int depth );
};

class GWContactInstance : public GWContactListItem
{
Q_OBJECT
public:
	GWContactInstance( QObject * parent, unsigned int theId, unsigned int theSequence, const QString & theDisplayName, const QString & theDn );
	QString dn;				// DN
	virtual void dump( unsigned int depth );
};

// END OF SERVER SIDE CONTACT LIST MODEL

#endif
