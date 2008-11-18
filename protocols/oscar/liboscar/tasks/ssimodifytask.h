/*
	Kopete Oscar Protocol
	ssimodifytask.h - Handles all the ssi modification stuff

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
#ifndef SSIMODIFYTASK_H
#define SSIMODIFYTASK_H

#include "task.h"
#include "oscartypes.h"
#include "contactmanager.h"
#include "contact.h"


class Buffer;

/**
This class takes care of any SSI list modifications that need to be made. This includes:
@li adds
@li edits
@li removes
@li group changes
@li alias changes
@li authorization changes
etc.

This task implements the following SNACs from the SSI family (0x0013):
@li 0x0008
@li 0x0009
@li 0x000A
@li 0x000E
@li 0x0011
@li 0x0012

@author Matt Rogers
*/
class SSIModifyTask : public Task
{
public:
    explicit SSIModifyTask( Task* parent, bool staticTask = false );
    ~SSIModifyTask();

	virtual void onGo();
	virtual bool take( Transfer* transfer );

	/* Contact properties */
	enum OperationType { NoType = 0x00, Add = 0x10, Remove = 0x20, Rename = 0x40, Change = 0x80 };
	enum OperationSubject { NoSubject = 0x000, Contact = 0x100, Group = 0x200, Visibility = 0x400, Invisibility = 0x800 };
	
	//! Set up the stuff needed to add a contact.
	//! @return true if we can send the packet
	bool addContact( const QString& contact, const QString& group, bool requiresAuth = false );
	
	//! Set up the stuff needed to remove a contact.
	//! @return true if we can send the packet
	bool removeContact( const QString& contact );
	
	//! Set up the stuff needed to change groups
	//! @return true if we can send the packet
	bool changeGroup( const QString& contact, const QString& newGroup );
	
	/* Group properties */
	
	//! Add a new group to the SSI list
	//! @return true if we can send the packet
	bool addGroup( const QString& groupName );
	
	//! Remove a group from the SSI list
	//! @return true if we can send the packet
	bool removeGroup( const QString& groupName );
	
	//! Rename a group on the SSI list
	//! @return true if we can send the packet
	bool renameGroup( const QString& oldName, const QString& newName );
	
	//! Add an item to the SSI list
	//! Should be used for other items we don't have explicit functions for
	//! like icon hashs, privacy settings, non-icq contacts, etc.
	bool addItem( const OContact& item );
	
	//! Remove an item from the SSI list
	//! Should be used for other items we don't have explicit functions for
	//! like icon hashs, privacy settings, non-icq contacts, etc.
	bool removeItem( const OContact& item );
	
	//! Modify an item on the SSI list
	//! Should be used for other items we don't have explicit functions for
	//! like icon hashs, privacy settings, non-icq contacts, etc.
	bool modifyItem( const OContact& oldItem, const OContact& newItem );

	//! Modify an contact item on the SSI list
	bool modifyContact( const OContact& oldItem, const OContact& newItem );

protected:
	virtual bool forMe( const Transfer* transfer ) const;

private:
	//! Handle the acknowledgement from the server
	void handleContactAck();
	
	//! Construct and send the packet to send to the server
	void sendContactUpdate();
	
	//! Helper function to change the group on the server
	void changeGroupOnServer();
	
	//! Update the Contact Manager with the new data
	void updateContactManager();
	
	//! Helper function to free id on error
	void freeIdOnError();
		
	//! Send the SSI edit start packet
	void sendEditStart();
	
	//! Send the SSI edit end packet
	void sendEditEnd();
	
	void addItemToBuffer( OContact item, Buffer* buffer );
	OContact getItemFromBuffer( Buffer* buffer ) const;

	//! Handle server request to add data
	void handleContactAdd();

	//! Handle server request to update data
	void handleContactUpdate();

	//! Handle server request to remove data
	void handleContactRemove();

private:
	OContact m_oldItem;
	OContact m_newItem;
	OContact m_groupItem;
	OperationType m_opType;
	OperationSubject m_opSubject;
	Oscar::DWORD m_id;
	ContactManager* m_ssiManager;
	bool m_static;

};

#endif

//kate: tab-width 4; indent-mode csands
