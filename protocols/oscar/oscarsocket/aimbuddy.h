/*
    aimbuddy.h - The Less Crappy AIM Buddy List Implementation (TM)

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

#ifndef AIMBUDDY_H
#define AIMBUDDY_H

#include <qstring.h>

class AIMBuddy
{
public:
	// The buddy and group ID's are required to create a buddy
	AIMBuddy( const int buddyID, const int groupID, const QString &screenName );

	// Returns the buddy id
	int ID() const { return mBuddyID; }

	// Returns the group id
	int groupID() const { return mGroupID; }

	// Returns the screenname of this buddy
	QString screenname() const { return mScreenName; }

	// Returns the alias for this buddy
	QString alias() const { return mAlias; }

	// Returns the status of this buddy
	const int status() const {  return mStatus; }

	// Sets the alias for this buddy
	void setAlias( const QString &alias ) { mAlias = alias; }

	// Sets the online/offline/away status of a user
	void setStatus( const int status) { mStatus = status; }

	void setWaitAuth( bool b ) { mWaitAuth = b; }
	bool waitAuth() { return mWaitAuth; }

	bool isServerSide() { return mIsServerSide; }
	void setServerSide( bool b ) { mIsServerSide = b; }

private:
	friend class OscarAccount;

	void setGroupID( const int groupID ) { mGroupID = groupID; }

	QString mScreenName;
	QString mAlias;
	int mBuddyID;
	int mGroupID;
	uint mStatus;
	bool mWaitAuth;

	// When true the buddy was created upon login. This is used to merge
	// OscarAccount's two contact lists (loginContactList and internalBuddyList),
	// but should probably be removed later in favour of something better
	bool mIsServerSide;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

