/*
    aimbuddylist.cpp - The Less Crappy AIM Buddy List Implementation (TM)

    Copyright (c) 2003 by Nick Betcher <nbetcher@kde.org>
    with copyrighted ideas stolen from Neil Stevens

    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "aimbuddylist.h"

// Project includes
#include "aim.h"

// KDE Includes
#include <kdebug.h>

AIMBuddyList::AIMBuddyList()
	: QObject()
{

}

AIMBuddyList *AIMBuddyList::operator+= (AIMBuddyList &original)
{
	for (int i = 0; i != original.mBuddiesDeny.count(); ++i)
		mBuddiesDeny.append(original.mBuddiesDeny.at(i));

	for (int i = 0; i != original.mBuddiesPermit.count(); ++i)
		mBuddiesPermit.append(original.mBuddiesPermit.at(i));

	for (QMap<int, AIMBuddy * >::Iterator it = original.mBuddyMap.begin(); it != original.mBuddyMap.end(); ++it)
	{
		if ((*it))
			if (mBuddyMap.find((*it)->ID()) != mBuddyMap.end())
				continue; //already have this in our list

		mBuddyMap.insert((*it)->ID(), (*it));
		mBuddyNameMap.insert((*it)->screenname(), (*it));
	}

	for (QMap<int, AIMGroup * >::Iterator it = original.mGroupMap.begin(); it != original.mGroupMap.end(); ++it)
	{
		if ((*it))
			if (mGroupMap.find((*it)->ID()) != mGroupMap.end())
				continue; //already have this in our list

		mGroupMap.insert((*it)->ID(), (*it));
		if (!(*it)->name().isNull())
			mGroupNameMap.insert((*it)->name(), (*it));
	}
	return this;
}

void AIMBuddyList::addBuddy(AIMBuddy *buddy)
{
	mBuddyMap.insert(buddy->ID(), buddy);
	mBuddyNameMap.insert(tocNormalize(buddy->screenname()), buddy);
}

void AIMBuddyList::removeBuddy(AIMBuddy *buddy)
{
	mBuddyNameMap.remove(buddy->screenname());
	mBuddyMap.remove(buddy->ID());
	QMap<int, AIMGroup * >::Iterator group = mGroupMap.find(buddy->groupID());
	if (group == mGroupMap.end())
		return;
	(*group)->removeBuddy(buddy);
}

void AIMBuddyList::moveBuddy(AIMBuddy *buddy, AIMGroup *from, AIMGroup *to)
{
	from->removeBuddy(buddy);
	buddy->setGroupID(to->ID());
	to->addBuddy(buddy);
}

AIMBuddy *AIMBuddyList::findBuddy(const int id)
{
	QMap<int, AIMBuddy * >::Iterator it = mBuddyMap.find(id);
	if (it != mBuddyMap.end() && (*it))
		return (*it);
	return 0L;
}

AIMBuddy *AIMBuddyList::findBuddy(const QString &name)
{
	QMap<QString, AIMBuddy * >::Iterator it = mBuddyNameMap.find(tocNormalize(name));
	if (it != mBuddyNameMap.end() && (*it))
		return (*it);
	return 0L;
}

AIMGroup *AIMBuddyList::addGroup(const int id, const QString &name)
{
	AIMGroup *group = new AIMGroup(id);
	if (name != QString::null)
	{
		group->setName(name);
		mGroupNameMap.insert(name, group);
	}
	mGroupMap.insert(group->ID(), group);
	emit groupAdded(group);
	return group;
}

void AIMBuddyList::removeGroup(const int id)
{
	AIMGroup *group = mGroupMap[id];
	if (!group) return;
	mGroupNameMap.remove(group->name());
	delete group; // also deletes the buddies in that group too
	mGroupMap.remove(id);
}

AIMGroup *AIMBuddyList::findGroup(const int id)
{
	QMap<int, AIMGroup * >::Iterator it = mGroupMap.find(id);
	if (it != mGroupMap.end() && (*it))
		return (*it);
	return 0L;
}

AIMGroup *AIMBuddyList::findGroup(const QString &name)
{
	QMap<QString, AIMGroup * >::Iterator it = mGroupNameMap.find(name);
	if (it != mGroupNameMap.end() && (*it))
		return (*it);
	return 0L;
}

bool AIMBuddyList::setGroupName(AIMGroup *group, const QString &name)
{
	QMap<QString, AIMGroup * >::Iterator oldgroup = mGroupNameMap.find(name);
	if (oldgroup == mGroupNameMap.end())
	{
		group->setName(name);
		return true;
	}
	return false;
}

void AIMBuddyList::addBuddyPermit(AIMBuddy *buddy)
{
	mBuddiesPermit.insert(buddy->ID(), buddy);
}

void AIMBuddyList::removeBuddyPermit(AIMBuddy *buddy)
{
	mBuddiesPermit.remove(buddy->ID());
}

void AIMBuddyList::addBuddyDeny(AIMBuddy *buddy)
{
	mBuddiesDeny.insert(buddy->ID(), buddy);
}

void AIMBuddyList::removeBuddyDeny(AIMBuddy *buddy)
{
	mBuddiesDeny.remove(buddy->ID());
}

AIMGroup::AIMGroup(const int id)
{
	mGroupID = id;
}

void AIMGroup::removeBuddy(AIMBuddy *buddy)
{
	mBuddies.remove(buddy);
}

bool AIMGroup::addBuddy(AIMBuddy *buddy)
{
	if (buddy->groupID() == mGroupID)
	{
		mBuddies.insert(mGroupID, buddy);
		return true;
	}
	return false;
}

AIMBuddy::AIMBuddy(const int buddyID, const int groupID, const QString &screenName)
{
	mBuddyID = buddyID;
	mGroupID = groupID;
	mScreenName = screenName;
}

AIMBuddyCaps::AIMBuddyCaps()
{
	buddyicon = false;
	voice = false;
	imimage = false;
	chat = false;
	getfile = false;
	sendfile = false;
	games = false;
	savestocks = false;
	sendbuddylist = false;
	games2 = false;
	icq = false;
	apinfo = false;
	icqrtf = false;
	empty = false;
	icqserverrelay = false;
	icqunknown = false;
	trilliancrypt = false;
	last = false;
}

#include "aimbuddylist.moc"
