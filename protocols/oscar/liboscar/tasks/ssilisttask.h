/*
  Kopete Oscar Protocol
  ssilisttask.h - handles all operations dealing with the whole Contact list

  Copyright (c) 2004 Matt Rogers <mattr@kde.org>

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
#ifndef SSILISTTASK_H
#define SSILISTTASK_H

#include <task.h>

class OContact;
class ContactManager;

/**
 * This task handles all the operations dealing with the whole Contact list
 * All SNACs handled by this task are in family 13. Subtypes 5, 6, and 7
 * are handled by individual functions. Subtype F is handled in the take()
 * function. We don't use subtype 4 because the same thing can be accomplished
 * using subtypes 5 and 6 together.
 *
 * @author Matt Rogers
*/
class SSIListTask : public Task
{
Q_OBJECT
public:
	SSIListTask( Task* parent );
	~SSIListTask();

	virtual bool take( Transfer* transfer );

protected:
	virtual bool forMe( const Transfer* transfer ) const;
	virtual void onGo();

signals:
	/** We have a new group */
	void newGroup( const OContact& );

	/** We have a new contact */
	void newContact( const OContact& );
	
	/**
	 * We have a new contact and they're on the visible list
	 */
	void newVisibleItem( const OContact& );
	
	/**
	 * We have a new contact and they're on the invisible list
	 */
	void newInvisibleItem( const OContact& );
	
	/**
	 * We have a new item 
	 * Used for items we don't explicitly handle yet
	 */
	void newItem( const OContact& );

private:

	/**
	 * Handle the list we get from the server
	 * This is SNAC( 0x13, 0x06 )
	 */
	void handleContactListReply();

	/**
	 * Check the timestamp of the local Contact copy
	 * If it's up to date, we'll get SNAC( 13, 06 )
	 * If it's out of date, we'll get SNAC( 13, 0F )
	 * This is SNAC( 0x13, 0x05 )
	 */
	void checkContactTimestamp();

	/**
	 * The timestamp of the Contact is up to date
	 * This is SNAC( 0x13, 0x0F )
	 */
	void handleSSIUpToDate();

	
private:
	/**
	 * Pointer to the Contact manager so we don't have to keep
	 * calling client()->ssiManager(). It's guaranteed to
	 * exist.
	 */
	ContactManager* m_ssiManager;

};

#endif

// kate: tab-width 4; indent-mode csands;
