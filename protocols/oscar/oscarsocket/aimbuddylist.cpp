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

#include "aim.h"
#include "oscarsocket.h"

//#include <kdebug.h>

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
	// By default set it's status to offline
	mStatus = OSCAR_OFFLINE;
	mWaitAuth = false;
	mIsServerSide = false;
}

// vim: set noet ts=4 sts=4 sw=4:

