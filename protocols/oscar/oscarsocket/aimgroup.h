/*
    aimgroup.h - The Less Crappy AIM Buddy List Implementation (TM)

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

/* Q_UINT32, int, and Q_UINT8 are used in replacements of the normal
    integer variables because the Qt (above)-ones are 64-bit safe.
*/

#ifndef AIMGROUP_H
#define AIMGROUP_H

#include <qptrlist.h>
#include <qstring.h>

class AIMBuddy;

class AIMGroup
{
public:
	// Group ID is required when creating
	AIMGroup( const int );

	// Returns the group's ID
	int ID() const { return mGroupID; }

	// Returns this group's name, if it has been assigned one yet
	QString name() const { return mName; }

	// Returns a list of buddies belonging to this group
	QPtrList<AIMBuddy> buddies() const { return mBuddies; }

	bool isServerSide() { return mIsServerSide; }
	void setServerSide( bool b ) { mIsServerSide = b; }

protected:
	friend class OscarAccount;
	void removeBuddy( AIMBuddy *buddy );
	bool addBuddy( AIMBuddy *buddy );
	void setName( const QString &name ) { mName = name; }

private:
	int mGroupID;
	QString mName;
	QPtrList<AIMBuddy> mBuddies;

	// When true the group was created upon login. This is used to merge
	// OscarAccount's two contact lists (loginContactList and internalBuddyList),
	// but should probably be removed later in favour of something better
	bool mIsServerSide;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

