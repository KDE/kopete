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

/** Adds a buddy to the SSI data list */
SSI * SSIData::addBuddy(const QString &name, const QString &group)
{
 	SSI *newitem = new SSI;
	newitem->name = name;
	SSI *tmp = findGroup(group);
	if (!tmp) //the group does not exist
		return NULL;
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
			if (i->name == name) //the group already exists
				return NULL;
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
	newitem->tlvlist = NULL;
	append(newitem);
	return newitem;
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
		kdDebug(14150) << "[OSCAR][SSIData] name: " << i->name << ", gid: " << i->gid << ", bid: " << i->bid << ", type: "
			  << i->type << ", tbslen: " << i->tlvlength << endl;
	}
}

/** Adds the given sn to the list of blocked sn's */
SSI *SSIData::addBlock(const QString &name)
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

/** Finds the given buddy in the deny list... return NULL if not found */
SSI *SSIData::findDeny(const QString &name)
{
	for (SSI *i=first(); i; i = next())
	{
		if ((current()->name == name) && (current()->type == 0x0003))
			return current();
	}
	return 0L;
}

// vim: set noet ts=4 sts=4 sw=4:
