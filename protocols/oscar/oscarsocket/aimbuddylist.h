/*
    aimbuddylist.h - The Less Crappy AIM Buddy List Implementation (TM)

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

#ifndef AIMBUDDYLIST_H
#define AIMBUDDYLIST_H

// Qt includes
#include <qptrlist.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <qobject.h>

//#include "kopeteonlinestatus.h"

class AIMBuddyCaps
{
public:
	AIMBuddyCaps();
	bool buddyicon;
	bool voice;
	bool imimage;
	bool chat;
	bool getfile;
	bool sendfile;
	bool games;
	bool savestocks;
	bool sendbuddylist;
	bool games2;
	bool icq;
	bool apinfo;
	bool icqrtf;
	bool empty;
	bool icqserverrelay;
	bool icqunknown;
	bool trilliancrypt;
	bool last;
};

class AIMBuddy
{
	public:
		// The buddy and group ID's are required to create a buddy
		AIMBuddy(const int buddyID, const int groupID, const QString &screenName);

		// Returns the buddy id
		int ID() { return mBuddyID; }

		// Returns the group id
		int groupID() { return mGroupID; }

		// Returns the screenname of this buddy
		const QString &screenname() { return mScreenName; }

		// Returns the alias for this buddy
		const QString &alias() { return mAlias; }

		// Returns the capabilities of this buddy
		const AIMBuddyCaps &capabilities() { return mCapabilities; }

		// Returns the status of this buddy
	//	KopeteOnlineStatus status() { return mStatus; }
		const int status() {  return mStatus; }

		// Returns the user class of this buddy
		int userClass() { return mUserClass; }

		// Returns the last known sign on time
		long signOnTime() { return mSignOnTime; }

		// Returns the idle time
//		Q_UINT32 idleTime() { return mIdleTime; }

		// Returns how long this user has been a member since
		long memberSince() { return mMemberSince; }

		// Returns the users evil (warning) level
		Q_UINT32 evil() { return mEvil; }

		// Returns the users last sign on time
		long lastSignOnTime() { return mLastOn; }

		// Returns the string version of the last sign on time
		const QString &stringLastOn() { return mStringLastOn; }

		// Sets the alias for this buddy
		void setAlias(const QString &alias) { mAlias = alias; }

		// Sets the capabilities of this buddy.
		void setCapabilities(const AIMBuddyCaps &cap) { mCapabilities = cap; }

		// Sets the online/offline/away status of a user
	//	void setStatus( const KopeteOnlineStatus &status ) { mStatus = status; }
		void setStatus(const int status) { mStatus = status; }


		// Sets the users user class
		void setUserClass(int userClass) { mUserClass = userClass; }

		// Sets the last known sign on time
		void setSignOnTime(long signOnTime) { mSignOnTime = signOnTime; }

		// Sets the idle time
//		void setIdleTime(Q_UINT32 idleTime) { mIdleTime = idleTime; }

		// Sets how long this user has been a member since
		void setMemberSince(long memberSince) { mMemberSince = memberSince; }

		// Sets their 'evil' rating. Usually refered to as their 'warning level'
		void setEvil(Q_UINT32 evil) { mEvil = evil; }

		// Sets the last time this member was signed on
		void setLastSignOnTime(long lastOn) { mLastOn = lastOn; }

		// Sets the string version of the last time the member was on
		void setStringLastOn(const QString &stringLastOn) { mStringLastOn = stringLastOn; }

	protected:
		friend class AIMBuddyList;
		void setGroupID(const int groupID) { mGroupID = groupID; }

	private:
		QString mScreenName;
		QString mAlias;
		int mBuddyID;
		int mGroupID;
		AIMBuddyCaps mCapabilities;
	//	KopeteOnlineStatus mStatus;
		unsigned int mStatus;
		int mUserClass;
		long mSignOnTime;
//		Q_UINT32 mIdleTime;
		long mMemberSince;
		Q_UINT32 mEvil;
		long mLastOn;
		QString mStringLastOn;
};

class AIMGroup
{
	public:
		// Group ID is required when creating
		AIMGroup(const int);

		// Returns the group's ID
		int ID() { return mGroupID; }

		// Returns this group's name, if it has been assigned one yet
		const QString &name() { return mName; }

		// Returns a list of buddies belonging to this group
		const QPtrList<AIMBuddy> &buddies() { return mBuddies; }

	protected:
		friend class AIMBuddyList;
		void removeBuddy(AIMBuddy *);
		bool addBuddy(AIMBuddy *);
		void setName(const QString &name) { mName = name; }

	private:
		int mGroupID;
		QString mName;
		QPtrList<AIMBuddy> mBuddies;
};

class AIMBuddyList : public QObject
{
	Q_OBJECT

	public:
		AIMBuddyList(QObject *parent=0, const char *name=0);

		// Merges two lists together
		AIMBuddyList *operator+= (AIMBuddyList &);

		// Adds a buddy to the buddy list
		void addBuddy(AIMBuddy *);

		// Removes a buddy from the buddy list
		void removeBuddy(AIMBuddy *);

		/* Moves a buddy 'buddy' from group 'from' to group 'to'. Both groups
			need to have been created already */
		void moveBuddy(AIMBuddy *buddy, AIMGroup *from, AIMGroup *to);

		// Finds a buddy in the buddy list. Returns 0L if none found.
		AIMBuddy *findBuddy(const int, const int gid);

		// Finds a buddy in the buddy list. Returns 0L if none found.
		AIMBuddy *findBuddy(const QString &);

		// Adds a group to the contact list and returns the new group
		AIMGroup *addGroup(const int id, const QString &name = QString::null);

		// Removes an entire group (including its children buddies!)
		void removeGroup(const int id);

		// Finds a group and returns it. Uses GID
		AIMGroup *findGroup(const int);

		// Finds a group by name
		AIMGroup *findGroup(const QString &name);

		// Sets the name of a group. If this returns false, that means another group has this same name
		bool setGroupName(AIMGroup *, const QString &name);

		// Adds a buddy to the permit list
		void addBuddyPermit(AIMBuddy *);

		// Removes a buddy from the permit list
		void removeBuddyPermit(AIMBuddy *);

		// Adds a buddy to the deny list
		void addBuddyDeny(AIMBuddy *);

		// Removes a buddy from the deny list
		void removeBuddyDeny(AIMBuddy *);

		// Returns a list of all of the buddies
		const QMap<QString, AIMBuddy * > &buddies() { return mBuddyNameMap; }

		// Returns a list of all the buddies in the deny list
		const QPtrList<AIMBuddy> &denyBuddies() { return mBuddiesDeny; }

		// Returns a list of all the buddies in the permit list
		const QPtrList<AIMBuddy> &permitBuddies() { return mBuddiesPermit; }

		// Revision of this list
		int revision;

		// Timestamp of the list
		int timestamp;

	signals:
		/* "Why?" you ask, do I have a signal that lets us know such a useless thing?
			Well, basically the AOL server can and often does (on certain accounts) send
			us contacts with a GID in which the group matching to that GID hasn't been
			sent down from the server yet. This gets around that by having OscarProtocol
			queue up contacts that have a GID which hasn't been created yet, then when
			that group is greated with the cooresponding GID to the contacts which
			have been queued up, we add those contacts. That's why we have this signal,
			to let OscarProtocol know to try to add certain contacts. */
		void groupAdded(AIMGroup *);

	private:
		QPtrList<AIMBuddy> mBuddiesDeny;
		QPtrList<AIMBuddy> mBuddiesPermit;
		QMap<int, AIMGroup * > mGroupMap;
		QMap<QString, AIMBuddy * > mBuddyNameMap;
		QMap<QString, AIMGroup * > mGroupNameMap;
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
