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

const WORD ROSTER_CONTACT	= 0x0000; // a normal contact
const WORD ROSTER_GROUP		= 0x0001; // a group of contacts
const WORD ROSTER_VISIBLE	= 0x0002; // a contact on the visible list
const WORD ROSTER_INVISIBLE	= 0x0003; // a contact on the invisible list
const WORD ROSTER_VISIBILITY	= 0x0004; // this entry contains visibility setting TLV(0xca)=TLV(202)
const WORD ROSTER_PRESENCE	= 0x0005; // Presence info (if others can see your idle status, etc)
const WORD ROSTER_ICQSHORTCUT	= 0x0009; // Unknown or ICQ2k shortcut bar items
const WORD ROSTER_IGNORE	= 0x000e; // a contact on the ignore list
const WORD ROSTER_LASTUPDATE	= 0x000F; // Last update date (name: "LastUpdateDate")
const WORD ROSTER_NONICQ	= 0x0010; // a non-icq contact, no UIN, used to send SMS
const WORD ROSTER_IMPORTTIME	= 0x0013; // roster import time (name: "Import time")
const WORD ROSTER_BUDDYICONS	= 0x0014; // Buddy icon info. (names: from "0" and incrementing by one)

const WORD SSIACK_OK			= 0x0000; // SSI change succeeded
const WORD SSIACK_NOTFOUND		= 0x0002; // Modified item not found on server
const WORD SSIACK_ALREADYONSERVER	= 0x0003; // Added item already on server
const WORD SSIACK_ADDERR		= 0x000A; // Error adding item (invalid id, already in list, invalid data)
const WORD SSIACK_LIMITEXD		= 0x000C; // Cannot add item, item limit exceeded
const WORD SSIACK_ICQTOAIM		= 0x000D; // Cannot add ICQ contact to AIM list
const WORD SSIACK_NEEDAUTH		= 0x000E; // Cannot add contact because he needs AUTH

struct SSI
{
	/** \brief The name of this SSI item.
	 *
	 * This is usually the screenname, ICQ number, or group name.
	 */
	QString name;

	/** \brief The group id of the SSI item */
	int gid;

	/** \brief The buddy id of the SSI item */
	int bid;

	/** \brief The type of the SSI Item.
	 *
	 * see ROSTER_* defines
	 *
	 */
	int type;
	/** \brief The TLV list for this item. */
	char *tlvlist;
	/** \brief The length of the TLV list */
	int tlvlength;
	/** \brief Indicates we're awaiting authorization from this item
	 *
	 * This item isn't a part of the normal SSI data structure
	 * (as defined by the OSCAR protocol). It's for internal purposes
	 */
	bool waitingAuth;
};

/**
 * @author Tom Linsky (Main)
 * @author Chris TenHarmsel (Secondary)
 *
 * \brief Manages SSI data from the server
 *
 * You can use the SSI pointers returned from many of these methods to get
 * the data for the actual request to the server. None of these methods send
 * any data to the server. These fall under the Oscar Protocol Specification:
 * Family 0x0013, Subtype 0x0008 (Add item) or the Oscar Protocol
 * Specification: Family 0x0013, Subtype 0x0009 (Modify Item), but do not
 * contain information for whether it's an add or modify request, you will
 * have to construct that part of the SNAC yourself, and can then append this
 * data to the "series of items" list
 *
 * See http://kingant.net/oscar/?family=0x0013&subtype=0x0008
 * for more info
*/

class SSIData : public QPtrList<SSI>
{
	public:
		SSIData();
		~SSIData();

		/**
		 * \brief Adds a contact to the SSI data list
		 *
		 * you will need to actually send the info in the
		 * SSI returned by this method to change the Server
		 * Side data
		 */
		SSI *addContact(const QString &name, const QString &group, bool addingAuthBuddy);

		/**
		 * \brief Finds the contact with given name and group...
		 * \return NULL if not found
		 */
		SSI *findContact(const QString &name, const QString &group);

		/**
		 * \brief Find the contact with the given name.
		 *
		 * The group the contacts is in doesn't matter. This is a
		 * convienence function and behaves much like the above function
		 * \return NULL if not found
		 */
		SSI *findContact(const QString &name);

		// ===============================================================================

		/**
		 * \brief Adds a group to the local SSI data
		 * \return An SSI pointer to the data or NULL if no matching
		 * group is found.
		 */
		SSI *addGroup(const QString &name);

		/**
		 * \brief Find the group named name
		 * \return An SSI pointer to the data or NULL if no matching
		 * group is found.
		 */
		SSI *findGroup(const QString &name);

		/**
		 * \brief Find the group by it's ID
		 * \return An SSI pointer to the data or NULL if no matching
		 * group is found.
		 */
		SSI *findGroup(const int groupId);

		/**
		 * \brief Change the name of a group in the local SSI data
		 *
		 * \return An SSI pointer to the data or NULL if no matching
		 * group is found. The pointer will contain the new data.
		 */
		SSI *renameGroup(const QString &currentName, const QString &newName);

		// ===============================================================================

		/**
		 * \brief Add a screenname to the list of blocked screennames
		 *
		 * Creates a new SSI item to be added to the invisible list.
		 * Existing items are not changed.
		 * \return An SSI pointer to the data.
		 */
		SSI *addInvis(const QString &name);

		/**
		 * \brief Remove an SSI item from the list of blocked
		 * screennames
		 *
		 * Deletes the SSI item created by addInvis from the SSI list
		 * The SSI item should be retrieved using findInvis before
		 * removing the item using this function.
		 * \return true if the deletion was successful, false otherwise
		 */
		bool removeInvis(const QString &name);

		/**
		 * \brief Finds the given contact in the list of blocked
		 * screennames
		 *
		 * \return An SSI pointer to the data or NULL is no matching
		 * screenname is found.
		 */
		SSI *findInvis(const QString &name);

		// ===============================================================================

		/**
		 * \brief Set that we're waiting auth for a SSI Item
		 *
		 * This does not indicate that the appropriate TLV will be
		 * added or is currently in the TLV list for this item
		 */
		void setWaitingAuth( SSI* item, bool waiting );

		/**
		 * \brief Get whether or not we're waiting auth.
		 * This does not indicate that the appropriate TLV exists in
		 * or has been added to the TLV list for this item
		 */
		bool waitingAuth( SSI* item );

		/**
		 * \internal
		 * Prints out the SSI data
		 */
		void print();

		/**
		 * \brief Find the visibility setting for our account
		 * \return An SSI pointer containing the visibility setting
		 * or NULL if the appropriate item cannot be found
		 */
		SSI *findVisibilitySetting();

	private:
		/**
		 * \internal
		 * \return the maximum contact id used in the SSI list
		 */
		unsigned short maxContactId(const int);
		/**
		 * \internal
		 * \returm the maximum group id used in the SSI list
		 */
		unsigned short maxGroupId();
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
