/***************************************************************************
                          ssidata.h  -  description
                             -------------------
    begin                : Wed Aug 14 2002

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef SSIDATA_H
#define SSIDATA_H

#include <qstring.h>
#include <qptrlist.h>

#include "oscartypes.h"

const WORD ROSTER_CONTACT = 0x0000;		// a normal contact
const WORD ROSTER_GROUP = 0x0001;		// a group of contacts
const WORD ROSTER_VISIBLE = 0x0002;		// a contact on the visible list
const WORD ROSTER_INVISIBLE = 0x0003;	// a contact on the invisible list
const WORD ROSTER_VISIBILITY = 0x0004;	// this entry contains visibility setting TLV(0xca)=TLV(202)
const WORD ROSTER_IGNORE = 0x000e;		// a contact on the ignore list

const WORD SSIACK_OK			= 0x0000; // SSI change succeeded
const WORD SSIACK_NOTFOUND		= 0x0002; // Modified item not found on server
const WORD SSIACK_ALREADYONSERVER	= 0x0003; // Added item already on server
const WORD SSIACK_ADDERR		= 0x000A; // Error adding item (invalid id, already in list, invalid data)
const WORD SSIACK_LIMITEXD		= 0x000C; // Cannot add item, item limit exceeded
const WORD SSIACK_ICQTOAIM		= 0x000D; // Cannot add ICQ contact to AIM list
const WORD SSIACK_NEEDAUTH		= 0x000E; // Cannot add contact because he needs AUTH

struct SSI
{
	QString name;
	int gid;
	int bid;
	int type;
	char *tlvlist;
	int tlvlength;
	bool waitingAuth;
};

/*
 * @author Tom Linsky (Main)
 * @author Chris TenHarmsel (Secondary)
 *
 * Manages SSI data from the server
 * You can use the SSI pointers returned
 * from many of these methods to send
 * the actual request to the server.
 * These fall under the
 * Oscar Protocol Specification: Family 0x0013, Subtype 0x0008
 * (Add item)
 * or the Oscar Protocol Specification: Family 0x0013, Subtype 0x0009
 * (Modify Item), but do not contain information for
 * whether it's an add or modify request, you will have to
 * construct that part of the SNAC yourself, and can then
 * append this data to the "series of items" list
 * See: http://kingant.net/oscar/?family=0x0013&subtype=0x0008
 * for more info
 */

class SSIData : public QPtrList<SSI>
{
	public:
		SSIData();
		~SSIData();

		/*
		 * Adds a contact to the SSI data list
		 * you will need to actually send the info in the
		 * SSI returned by this method to change the Server
		 * Side data
		 */
		SSI *addContact(const QString &name, const QString &group, bool addingAuthBuddy);
		SSI *addContact(const int groupId, const int contactId, bool addingAuthBuddy);

		/*
		 * Finds the contact with given name and group...
		 * returns NULL if not found
		 */
		SSI *findContact(const QString &name, const QString &group);

		/**
		 * Find the contact with the given name. Group here doesn't matter.
		 * This is a convienence function and behaves much like the above function
		 * \return NULL if not found
		 */
		SSI *findContact(const QString &name);

		// ===============================================================================

		/*
		 * Adds a group to the local ssi data
		 * you will need to actually send the info in the
		 * SSI returned by this method to change the Server
		 * Side data
		 */
		SSI *addGroup(const QString &name);

		/*
		 * Find the group named name, and returns a pointer to it
		 */
		SSI *findGroup(const QString &name);
		/*
		 * Same asa above but searches for group by using its groupID
		 */
		SSI *findGroup(const int groupId);

		/*
		 * Changes the name of a group in the local SSI data
		 * You can use the SSI pointer returned by this
		 * method to pass to the server to actually change
		 * the name on the SSI data
		 */
		SSI *renameGroup(const QString &currentName, const QString &newName);

		// ===============================================================================

		/* Adds the given sn to the list of blocked sn's
		 * You can use the SSI pointer returned by this
		 * method to pass to the server to actually change
		 * the name on the SSI data
		 */
		SSI *addInvis(const QString &name);

		bool removeInvis(const QString &name);

		/*
		 * Finds the given contact in the deny list...
		 * returns NULL if not found
		 */
		SSI *findInvis(const QString &name);

		// ===============================================================================

		void setWaitingAuth( SSI* item, bool waiting );

		bool waitingAuth( SSI* item );
		/*
		 * Prints out the SSI data
		 */
		void print();

		SSI *findVisibilitySetting();

	private:
		unsigned short maxContactId(const int);
		unsigned short maxGroupId();
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
