/***************************************************************************
                          ssidata.cpp  -  description
                             -------------------
    begin                : Wed Aug 14 2002
    copyright            : (C) 2002 by Tom Linsky
    email                : twl6@po.cwru.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <kdebug.h>
#include "ssidata.h"

SSIData::SSIData()
{
	setAutoDelete(true);
}

SSIData::~SSIData()
{
	clear();
}

SSI * SSIData::addBuddy(const QString &name, const QString &group)
{
 	SSI *newitem = new SSI;
	newitem->name = name;
	SSI *tmp = findGroup(group);

	if (!tmp) { //the group does not exist
		delete newitem;
		return 0;
	}

	newitem->gid = tmp->gid;
	//find the largest bid (=buddy id) in our group
	unsigned short maxbid = 0;
	for (SSI *i=first(); i; i = next())
	{
		if ((newitem->gid == i->gid) && (i->bid > maxbid))
			maxbid = i->bid;
	}
	newitem->bid = maxbid + 1;
	newitem->type = 0x0000; // the type here is buddy
	newitem->tlvlist = NULL;
	newitem->tlvlength = 0;
	append(newitem);
	return newitem;
}

/** Find the group named name, and returns a pointer to it */
SSI * SSIData::findGroup(const QString &name)
{
	for (SSI *i=first(); i; i = next())
	{
		if ((current()->name == name) && (current()->type == 0x0001))
			return current();
	}
	return 0L;
}

/** Adds a group to the local ssi data */
SSI * SSIData::addGroup(const QString &name)
{
	SSI *newitem = new SSI;
	newitem->name = name;
	if (!name.isEmpty())
	{
		unsigned short maxgid = 0;
		//find the highest gid, add 1 to it, and assign it to this group
		for (SSI *i=first(); i; i = next())
		{
			if (i->name == name) {//the group already exists
				delete newitem;
				return 0;
			}
			if (i->gid > maxgid)
				maxgid = i->gid;
		}
		newitem->gid = maxgid + 1;
	}
	else  //this is the master group
	{
		newitem->gid = 0;
	}
	newitem->type = 0x0001;  //group
	newitem->bid = 0;
	newitem->tlvlength = 0;
	newitem->tlvlist = 0;
	append(newitem);
	return newitem;
}

// Changes a groups name
SSI *SSIData::changeGroup(const QString &currentName, const QString &newName)
{
	// Find the group
	SSI *group = findGroup(currentName);
	// No sense in trying to change the group's name if it doesn't exist
	if (group != 0L)
	{ // Printing some debugging info
		kdDebug(14150) << k_funcinfo <<  "Building group name change request"
					   << endl;
		// Change the info in the SSI for the group name
		// Sending the OSCAR serverthis SNAC, where the
		// group ID is the same, but the name in the
		// SNAC has changed _should_ change the name of
		// the group on the server -Chris
		group->name = newName;
	}
	// Return the group, which will be null if this couldn't
	// find the group
	return group;
}

// Finds the buddy with given name and group... returns NULL if not found
SSI *SSIData::findBuddy(const QString &name, const QString &group)
{
	SSI *gr = findGroup(group); //find the parent group
	if (gr)
	{
		printf("g->name is %s, g->gid is %x, g->bid is %x, g->type is %x\n",gr->name.latin1(),gr->gid,gr->bid,gr->type);
		for (SSI *i=first(); i; i = next())
		{
			//if the ssi item has the right name, is a buddy, and has the right group
			kdDebug(14150) << "i->gid is " << i->gid << ", gr->gid is " << gr->gid << endl;
			if ((i->name == name) && (i->type == 0x0000) && (i->gid == gr->gid))
			{
				//we have found our buddy
				kdDebug(14150) << "Found buddy " << name << " in SSI data" << endl;
				return i;
			}
		}
	}
	else
		kdDebug(14150) << "Group " << group << " not found" << endl;
	return 0L;
}

void SSIData::print(void)
{
	for (SSI *i=first(); i; i = next())
	{
		kdDebug(14150) << k_funcinfo << "name: " << i->name <<
			", gid: " << i->gid << ", bid: " << i->bid <<
			", type: " << i->type << ", tbslen: " << i->tlvlength << endl;
	}
}

SSI *SSIData::addDeny(const QString &name)
{
	SSI *newitem = new SSI;
	newitem->name = name;
	newitem->gid = 0;
	//find the largest bid (=buddy id) in our group
	unsigned short maxbid = 0;
	for (SSI *i=first(); i; i = next())
	{
		if ((newitem->gid == i->gid) && (i->bid > maxbid))
			maxbid = i->bid;
	}
	newitem->bid = maxbid + 1;
	newitem->type = 0x0003; // the type here is deny
	newitem->tlvlist = NULL;
	newitem->tlvlength = 0;
	append(newitem);
	return newitem;
}

SSI *SSIData::findDeny(const QString &name)
{
	for (SSI *i=first(); i; i = next())
	{
		if ((current()->name == name) && (current()->type == 0x0003))
			return current();
	}
	return 0L;
}

SSI *SSIData::findVisibilitySetting()
{
	for (SSI *i=first(); i; i = next())
	{
		if ((current()->name.isEmpty()) && (current()->type == 0x0004))
			return current();
	}
	return 0L;
}
// vim: set noet ts=4 sts=4 sw=4:
