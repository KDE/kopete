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

// ========================================================================================

SSI *SSIData::addContact(const QString &name, const QString &group, bool addingAuthBuddy)
{
	SSI *tmp = findGroup(group);
	if(!tmp) //the group does not exist
		return 0L;

	SSI *newitem = new SSI;
	newitem->name = name;
	newitem->gid = tmp->gid;
	newitem->waitingAuth = false;

	//find the largest bid (=contact id) in our group
	unsigned short maxbid = 0;
	for (SSI *i=first(); i; i = next())
	{
		if ((newitem->gid == i->gid) && (i->bid > maxbid))
			maxbid = i->bid;
	}

	newitem->bid = maxbid + 1;
	newitem->type = ROSTER_CONTACT;
	if (!addingAuthBuddy)
	{
		newitem->tlvlist = 0L;
		newitem->tlvlength = 0;
	}
	else
	{
		// TLV(0x0066) with no data
		newitem->tlvlist = "\x00\x66\x00\x00";
		newitem->tlvlength = 4;
		newitem->waitingAuth = true;
	}

	append(newitem);
	return newitem;
}

// Finds the contact with given name and group... returns NULL if not found
SSI *SSIData::findContact(const QString &name, const QString &group)
{
	SSI *gr = findGroup(group); //find the parent group
	if (gr)
	{
		kdDebug(14150) << k_funcinfo << "gr->name= " << gr->name <<
			", gr->gid= " << gr->gid <<
			", gr->bid= " << gr->bid <<
			", gr->type= " << gr->type << endl;
		for (SSI *i=first(); i; i = next())
		{
			//if the ssi item has the right name, is a contact, and has the right group
			/*kdDebug(14150) << k_funcinfo <<
				"i->gid is " << i->gid << ", gr->gid is " << gr->gid << endl;*/
			if ((i->name == name) && (i->type == ROSTER_CONTACT) && (i->gid == gr->gid))
			{
				//we have found our contact
				kdDebug(14150) << "Found contact " << name << " in SSI data" << endl;
				return i;
			}
		}
	}
	else
	{
		kdDebug(14150) << "Group " << group << " not found" << endl;
	}
	return 0L;
}

SSI *SSIData::findContact( const QString& name )
{
	for (SSI *i=first(); i; i = next())
	{
		//if the ssi item has the right name, is a contact, and has the right group
		/*kdDebug(14150) << k_funcinfo <<
			"i->gid is " << i->gid << ", gr->gid is " << gr->gid << endl;*/
		if ((i->name == name) && (i->type == ROSTER_CONTACT))
		{
			//we have found our contact
			kdDebug(14150) << "Found contact " << name << " in SSI data" << endl;
			return i;
		}
	}
	
	return 0L;
}

// ========================================================================================

SSI *SSIData::findGroup(const QString &name)
{
	for (SSI *i=first(); i; i = next())
	{
		if ((current()->name == name) && (current()->type == ROSTER_GROUP))
			return current();
	}
	return 0L;
}

SSI *SSIData::findGroup(const int groupId)
{
	for (SSI *i=first(); i; i = next())
	{
		if ((current()->bid == groupId) && (current()->type == ROSTER_GROUP))
			return current();
	}
	return 0L;
}

SSI *SSIData::addGroup(const QString &name)
{
	if(findGroup(name) != 0L)
		return 0L; // the group already exists

	SSI *newitem = new SSI;
	newitem->name = name;
	newitem->bid = 0;
	if(name.isEmpty()) // this is the master group
		newitem->gid = 0;
	else
		newitem->gid = maxGroupId() + 1;
	newitem->type = ROSTER_GROUP;
	newitem->tlvlength = 0;
	newitem->tlvlist = 0L;

	append(newitem);
	return newitem;
}

SSI *SSIData::renameGroup(const QString &currentName, const QString &newName)
{
	// Find the group
	SSI *group = findGroup(currentName);

	// No sense in trying to change the group's name if it doesn't exist
	if (group)
	{
		kdDebug(14150) << k_funcinfo << "Building group name change request" << endl;
		// Change the info in the SSI for the group name
		// Sending the OSCAR server this SNAC, where the
		// group ID is the same, but the name in the
		// SNAC has changed _should_ change the name of
		// the group on the server -Chris
		group->name = newName;
	}

	// Return the group, which will be null if this couldn't
	// find the group
	return group;
}

// ========================================================================================

SSI *SSIData::addInvis(const QString &name)
{
	kdDebug(14150) << k_funcinfo << "Called for contact '" << name << "'" << endl;

	SSI *newitem = new SSI;

	newitem->name = name;
	newitem->gid = 0;
	newitem->bid = maxContactId(newitem->gid) + 1;
	newitem->type = ROSTER_INVISIBLE; // the type here is deny
	newitem->tlvlist = 0L;
	newitem->tlvlength = 0;
	newitem->waitingAuth = false;

	append(newitem);

	return newitem;
}

bool SSIData::removeInvis(const QString &name)
{
	SSI *denyItem = findInvis(name);
	if(denyItem != 0L)
		remove(denyItem);

	return (denyItem!=0L);
}

SSI *SSIData::findInvis(const QString &name)
{
	kdDebug(14150) << k_funcinfo << "Called for contact '" << name << "'" << endl;
	for (SSI *i=first(); i; i = next())
	{
		if ((current()->name == name) && (current()->type == ROSTER_INVISIBLE))
			return current();
	}
	return 0L;
}

// ========================================================================================

SSI *SSIData::findVisibilitySetting()
{
	for (SSI *i=first(); i; i = next())
	{
		if ((current()->name.isEmpty()) && (current()->type == ROSTER_VISIBILITY))
			return current();
	}
	return 0L;
}

unsigned short SSIData::maxContactId(const int groupId)
{
	unsigned short maxId = 0;
	for (SSI *i=first(); i; i = next())
	{
		if ((groupId == i->gid) && (i->bid > maxId))
			maxId = i->bid;
	}
	return maxId;
}

unsigned short SSIData::maxGroupId()
{
	unsigned short maxId = 0;
	for (SSI *i=first(); i; i = next())
	{
		if (i->gid > maxId)
			maxId = i->gid;
	}
	return maxId;
}

void SSIData::setWaitingAuth( SSI* item, bool waiting )
{
	if ( item )
		item->waitingAuth = waiting;
}

bool SSIData::waitingAuth( SSI* item )
{
	if ( item )
		return item->waitingAuth;

	return 0L;
}


void SSIData::print()
{
	for (SSI *i=first(); i; i = next())
	{
		kdDebug(14150) << k_funcinfo << "name: " << i->name <<
			", gid: " << i->gid << ", bid: " << i->bid <<
			", type: " << i->type << ", tbslen: " << i->tlvlength << endl;
	}
}

// vim: set noet ts=4 sts=4 sw=4:
