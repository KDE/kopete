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
#include "oscardebug.h"

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
	if (name.isNull() || group.isNull())
	{
		kdWarning(14151) << k_funcinfo <<
			"Passed NULL name or group string, aborting!" << endl;
		return 0;
	}

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
	if (name.isNull() || group.isNull())
	{
		kdWarning(14151) << k_funcinfo <<
			"Passed NULL name or group string, aborting!" << endl;
		return 0;
	}

	SSI *gr = findGroup(group); // find the parent group
	if (gr)
	{
#ifdef OSCAR_SSI_DEBUG
		kdDebug(14151) << k_funcinfo << "gr->name= " << gr->name <<
			", gr->gid= " << gr->gid <<
			", gr->bid= " << gr->bid <<
			", gr->type= " << gr->type << endl;
#endif
		for (QPtrListIterator<SSI> it (*this); it.current(); ++it)
		{
			//if the ssi item has the right name, is a contact, and has the right group
			/*kdDebug(14151) << k_funcinfo <<
				"i->gid is " << i->gid << ", gr->gid is " << gr->gid << endl;*/
			if ((it.current()->name == name) && 
			   (it.current()->type == ROSTER_CONTACT) && 
			   (it.current()->gid == gr->gid))
			{
				//we have found our contact
#ifdef OSCAR_SSI_DEBUG
				kdDebug(14151) << k_funcinfo <<
					"Found contact " << name << " in SSI data" << endl;
#endif
				return it.current();
			}
		}
	}
	else
	{
		kdDebug(14151) << k_funcinfo <<
			"ERROR: Group '" << group << "' not found!" << endl;
	}
	return 0L;
}

SSI *SSIData::findContact(const QString& name)
{
	if (name.isNull())
	{
		kdWarning(14151) << k_funcinfo <<
			"Passed NULL name string, aborting!" << endl;
		return 0;
	}

	for (QPtrListIterator<SSI> it (*this); it.current(); ++it)
	{
		//if the ssi item has the right name, is a contact, and has the right group
		/*kdDebug(14151) << k_funcinfo <<
			"i->gid is " << i->gid << ", gr->gid is " << gr->gid << endl;*/
		if ((it.current()->name.lower() == name.lower()) && (it.current()->type == ROSTER_CONTACT))
		{
			//we have found our contact
#ifdef OSCAR_SSI_DEBUG
			kdDebug(14151) << k_funcinfo <<
				"Found contact " << name << " in SSI data" << endl;
#endif
			return it.current();
		}
	}

	kdDebug(14151) << k_funcinfo <<
		"ERROR: contact '" << name << "' not found!" << endl;
	return 0L;
}

// ========================================================================================

SSI *SSIData::findGroup(const QString &name)
{
#ifdef OSCAR_SSI_DEBUG
	kdDebug(14151) << k_funcinfo <<
		"Looking for group named '" << name << "'" << endl;
#endif
	if (name.isNull())
	{
		kdWarning(14151) << k_funcinfo <<
			"Passed NULL groupname string, aborting!" << endl;
		return 0;
	}

	for (QPtrListIterator<SSI> it (*this); it.current(); ++it)
	{
		if ((it.current()->name == name) && (it.current()->type == ROSTER_GROUP))
			return it.current();
	}
	return 0;
}

SSI *SSIData::findGroup(const int groupId)
{
#ifdef OSCAR_SSI_DEBUG
	kdDebug(14151) << k_funcinfo <<
		"Looking for gid '" << groupId << "'" << endl;
#endif

	for (QPtrListIterator<SSI> it (*this); it.current(); ++it)
	{
		if ( it.current()->gid == groupId && it.current()->type == ROSTER_GROUP )
			return it.current();
	}
	return 0L;
}

SSI *SSIData::addGroup(const QString &name)
{
	if (name.isNull())
	{
		kdWarning(14151) << k_funcinfo <<
			"Passed NULL groupname string, aborting!" << endl;
		return 0;
	}

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
	if (newName.isNull())
	{
		kdWarning(14151) << k_funcinfo <<
			"Passed NULL groupname string for new groupname, aborting!" << endl;
		return 0;
	}

	// Find the group
	SSI *group = findGroup(currentName);

	// No sense in trying to change the group's name if it doesn't exist
	if (group)
	{
#ifdef OSCAR_SSI_DEBUG
		kdDebug(14151) << k_funcinfo << "Building group name change request" << endl;
#endif
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


SSI *SSIData::addVisible(const QString &name)
{
#ifdef OSCAR_SSI_DEBUG
	kdDebug(14151) << k_funcinfo << "Called for contact '" << name << "'" << endl;
#endif
	return addSpecial(name, ROSTER_VISIBLE);
}

SSI *SSIData::findVisible(const QString &name)
{
#ifdef OSCAR_SSI_DEBUG
	kdDebug(14151) << k_funcinfo << "Called for contact '" << name << "'" << endl;
#endif
	return findSpecial(name, ROSTER_VISIBLE);
}

// ================


SSI *SSIData::addInvisible(const QString &name)
{
#ifdef OSCAR_SSI_DEBUG
	kdDebug(14151) << k_funcinfo << "Called for contact '" << name << "'" << endl;
#endif
	return addSpecial(name, ROSTER_INVISIBLE);
}

SSI *SSIData::findInvisible(const QString &name)
{
#ifdef OSCAR_SSI_DEBUG
	kdDebug(14151) << k_funcinfo << "Called for contact '" << name << "'" << endl;
#endif
	return findSpecial(name, ROSTER_INVISIBLE);
}

// ================

SSI *SSIData::addIgnore(const QString &name)
{
#ifdef OSCAR_SSI_DEBUG
	kdDebug(14151) << k_funcinfo << "Called for contact '" << name << "'" << endl;
#endif
	return addSpecial(name, ROSTER_IGNORE);
}

SSI *SSIData::findIgnore(const QString &name)
{
#ifdef OSCAR_SSI_DEBUG
	kdDebug(14151) << k_funcinfo << "Called for contact '" << name << "'" << endl;
#endif
	return findSpecial(name, ROSTER_IGNORE);
}

// ================

SSI *SSIData::addSpecial(const QString &name, WORD type)
{
	SSI *newitem = new SSI;

	newitem->name = name;
	newitem->gid = 0;
	newitem->bid = maxContactId(newitem->gid) + 1;
	newitem->type = type;
	newitem->tlvlist = 0L;
	newitem->tlvlength = 0;
	newitem->waitingAuth = false;

	append(newitem);

	return newitem;
}

bool SSIData::removeSpecial(const QString &name, WORD type)
{
	SSI *denyItem = findSpecial(name, type);
	if(denyItem != 0L)
		remove(denyItem);
	return (denyItem!=0L);
}

SSI *SSIData::findSpecial(const QString &name, WORD type)
{
	for (QPtrListIterator<SSI> it (*this); it.current(); ++it)
	{
		if ((it.current()->name == name) && (it.current()->type == type))
			return it.current();
	}
	return 0L;
}


// =============================================================================


SSI *SSIData::findVisibilitySetting()
{
	for (QPtrListIterator<SSI> it (*this); it.current(); ++it)
	{
		if ((it.current()->name.isEmpty()) && (it.current()->type == ROSTER_VISIBILITY))
			return it.current();
	}
	return 0L;
}

unsigned short SSIData::maxContactId(const int groupId)
{
	unsigned short maxId = 0;
	for (QPtrListIterator<SSI> it (*this); it.current(); ++it)
	{
		if ((groupId == it.current()->gid) && (it.current()->bid > maxId))
			maxId = it.current()->bid;
	}
	return maxId;
}

unsigned short SSIData::maxGroupId()
{
	unsigned short maxId = 0;
	for (QPtrListIterator<SSI> it (*this); it.current(); ++it)
	{
		if (it.current()->gid > maxId)
			maxId = it.current()->gid;
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

	return false;
}


void SSIData::print()
{
	for (QPtrListIterator<SSI> it (*this); it.current(); ++it)
	{
		kdDebug(14151) << k_funcinfo << "name: " << it.current()->name <<
			", gid: " << it.current()->gid << ", bid: " << it.current()->bid <<
			", type: " << it.current()->type << ", tbslen: " << it.current()->tlvlength << endl;
	}
}

// vim: set noet ts=4 sts=4 sw=4:
