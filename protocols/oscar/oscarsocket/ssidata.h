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

/**
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
 * @author Tom Linsky (Main)
 * @author Chris TenHarmsel (Secondary)
 */

struct SSI
{
	QString name;
	int gid;
	int bid;
	int type;
	char *tlvlist;
	int tlvlength;
};

class SSIData : public QPtrList<SSI>
{
public:
	SSIData();
	~SSIData();

	/*
	 * Find the group named name, and returns a pointer to it
	 */
	SSI *findGroup(const QString &name);
	/*
	 * Same asa above but searches for group by using its groupID
	 */
	SSI *findGroup(const int groupId);

	/**
	 * Adds a buddy to the SSI data list
	 * you will need to actually send the info in the
	 * SSI returned by this method to change the Server
	 * Side data
	 */
	SSI *addBuddy(const QString &name, const QString &group);

	/**
	 * Adds a group to the local ssi data
	 * you will need to actually send the info in the
	 * SSI returned by this method to change the Server
	 * Side data
	 */
	SSI *addGroup(const QString &name);

	/**
	 * Changes the name of a group in the local SSI data
	 * You can use the SSI pointer returned by this
	 * method to pass to the server to actually change
	 * the name on the SSI data
	 */
	SSI *renameGroup(const QString &currentName, const QString &newName);

	/** Finds the buddy with given name and group... returns NULL if not found */
	SSI *findBuddy(const QString &name, const QString &group);

	/**
	 * Prints out the SSI data
	 */
	void print(void);

	/** Adds the given sn to the list of blocked sn's
	 * You can use the SSI pointer returned by this
	 * method to pass to the server to actually change
	 * the name on the SSI data
	 */
	SSI *addDeny(const QString &name);

	/** Finds the given buddy in the deny list... return NULL if not found */
	SSI *findDeny(const QString &name);

	SSI *findVisibilitySetting();
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
