/*
    oscaraccount.cpp  -  Oscar Account Class

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
    Copyright (c) 2002 by Chris TenHarmsel <tenharmsel@staticmethod.net>
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

#include "oscaraccount.h"
#include <qapplication.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kopeteprotocol.h"
#include "kopeteaway.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopeteawaydialog.h"

// TODO: get rid of the next include
#include "aim.h" // For tocNormalize()

#include "oscarsocket.h"
#include "oscarchangestatus.h"


OscarAccount::OscarAccount(KopeteProtocol *parent, const QString &accountID, const char *name, bool isICQ)
	: KopeteAccount(parent, accountID, name)
{
	kdDebug(14150) << k_funcinfo << " accountID='" << accountID <<
		"', isICQ=" << isICQ << endl;

	mEngine = 0L;
	mMyself = 0L;
	// Set our random new numbers
	mRandomNewBuddyNum = 0;
	mRandomNewGroupNum = 0;
	// Set our idle to no
	mAreIdle = false;

	initEngine(isICQ); // Initialize the backend

	mAwayDialog = new OscarChangeStatus(engine());

	// Create the internal buddy list for this account
	// TODO: make this an internal list of KopeteGroup and Kopete-/OscarContact
	mInternalBuddyList = new AIMBuddyList(this, "mInternalBuddyList");

	initSignals();
}

OscarAccount::~OscarAccount()
{
	kdDebug(14150) << k_funcinfo << "'" << accountId() <<
		"' deleted, Disconnecting..." << endl;

	OscarAccount::disconnect();

	delete mAwayDialog; // has no parent

	// Delete the backend
	if (mEngine)
	{
		delete mEngine;
		mEngine = 0L;
	}
	else
	{
		kdDebug(14150) << k_funcinfo <<
			"ERROR, we don't have an OscarSocket anymore!" << endl;
	}
}

KopeteContact* OscarAccount::myself() const
{
	if(!mMyself)
		kdDebug(14150) << k_funcinfo << "returning NULL myself() contact!" << endl;
	return mMyself;
}

OscarSocket* OscarAccount::engine() const
{
	return mEngine;
}

void OscarAccount::connect()
{
	kdDebug(14150) << k_funcinfo <<
		"accountId='" << accountId() << "'" << endl;

	// Get the screen name for this account
	QString screenName = accountId();

	if (screenName != i18n("(No ScreenName Set)") )
	{	// If we have a screen name set
		// Get the password
		QString password = getPassword();
		if (password.isNull())
		{
			slotError(i18n("Kopete is unable to attempt to signon to the " \
				"AIM network because no password was specified in the " \
				"preferences."), 0);
		}
		else
		{
			kdDebug(14150) << k_funcinfo << accountId() <<
				": Logging in as " << screenName << endl;

			// Get the server and port from the preferences
			// TODO: check server and port are set
			QString server = pluginData(protocol(), "Server");
			QString port = pluginData(protocol(), "Port");

			if(server.isEmpty() || port.isEmpty())
			{
				kdDebug(14150) << k_funcinfo <<
					"Thank pluginData() for this one to break, no server and/or port set => can't connect." << endl;
			}

			// Connect, need to normalize the name first
			mEngine->doLogin(server, port.toInt(), tocNormalize(screenName), password);
		}
	}
	else
	{
		slotError(i18n("You have not specified your account name in the " \
			"account set up yet, please do so."), 0);
	}
}

void OscarAccount::disconnect()
{
	kdDebug(14150) << k_funcinfo << "accountID='" << accountId() << "'" << endl;
	mEngine->doLogoff();
}

void OscarAccount::initEngine(bool icq)
{
	kdDebug(14150) << k_funcinfo << "accountId="<< accountId() << endl;

	QByteArray cook;
	cook.duplicate("01234567",8);
	mEngine = new OscarSocket(pluginData(protocol(),"Server"), cook,
		this, this, "mEngine", icq);
}

void OscarAccount::initSignals()
{
	// Contact list signals for group management events
	QObject::connect(
		KopeteContactList::contactList(), SIGNAL(groupRenamed(KopeteGroup *, const QString &)),
		this, SLOT(slotKopeteGroupRenamed(KopeteGroup *, const QString &)));

	QObject::connect(
		KopeteContactList::contactList(), SIGNAL(groupRemoved(KopeteGroup *)),
		this, SLOT(slotKopeteGroupRemoved(KopeteGroup *)));

	// This is for when the user decides to add a group in the contact list
	QObject::connect(
		KopeteContactList::contactList(), SIGNAL(groupAdded(KopeteGroup *)),
		this, SLOT(slotGroupAdded(KopeteGroup *)));

	// own status changed
	QObject::connect(
		engine(), SIGNAL(statusChanged(const unsigned int)),
		this, SLOT(slotOurStatusChanged(const unsigned int)));

	// Protocol error
	QObject::connect(
		engine(), SIGNAL(protocolError(QString, int)),
		this, SLOT(slotError(QString, int)));

	// Got IM
	QObject::connect(
		engine(), SIGNAL(gotIM(QString,QString,bool)),
		this, SLOT(slotGotIM(QString,QString,bool)));

	// Got Config (Buddy List)
	QObject::connect(
		engine(), SIGNAL(gotConfig(AIMBuddyList &)),
		this, SLOT(slotGotServerBuddyList(AIMBuddyList &)));

	// Got direct IM request
	QObject::connect(
		engine(), SIGNAL(gotDirectIMRequest(QString)),
		this, SLOT(slotGotDirectIMRequest(QString)));

	// Set our idle time every 100 seconds to the server
	mIdleTimer = new QTimer(this, "OscarIdleTimer");

	QObject::connect(
		mIdleTimer, SIGNAL(timeout()),
		this, SLOT(slotIdleTimeout()));
//	mIdleTimer->start(100 * 1000);

	// We have officially become un-idle
	QObject::connect(
		KopeteAway::getInstance(), SIGNAL(activity()),
		this, SLOT(slotIdleActivity()));

	// Got a new server-side group, try and add any queued
	// buddies to our Kopete contact list
	QObject::connect(
		mInternalBuddyList, SIGNAL(groupAdded(AIMGroup *)),
		this, SLOT(slotReTryServerContacts()));
}

void OscarAccount::slotGoOnline()
{
	if(
		myself()->onlineStatus().status() == KopeteOnlineStatus::Away ||
		myself()->onlineStatus().internalStatus() == OSCAR_FFC)
	{ // If we're away , set us available
		kdDebug(14150) << k_funcinfo << accountId() <<
			": Was AWAY or FFC, marking back" << endl;

		setAway(false);
	}
	else if(myself()->onlineStatus().status() == KopeteOnlineStatus::Offline)
	{ // If we're offline, connect
		kdDebug(14150) << k_funcinfo << accountId() <<
			": Was OFFLINE, now connecting" << endl;

		OscarAccount::connect();
	}
	else
	{
		kdDebug(14150) << k_funcinfo << accountId() <<
			": Already ONLINE" << endl;
	}
}

void OscarAccount::slotGoOffline()
{
	// This will ask the server to log us off
	// and then when that is complete, engine
	// will notify us of a status change
	engine()->doLogoff();
}

void OscarAccount::slotGoAway()
{
	kdDebug(14150) << k_funcinfo << "Called" << endl;

	// Away could also be a different AWAY mode (like NA or OCC)
	if(
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Online) ||
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Away)
		)
	{
		mAwayDialog->show(OSCAR_AWAY);
	}
}

void OscarAccount::slotError(QString errmsg, int errorCode)
{
	kdDebug(14150) << k_funcinfo << "accountId()='" << accountId() <<
		"' errmsg=" << errmsg <<
		", errorCode=" << errorCode << "." << endl;

	// 1 = username unknown to server
	// 5 = wrong password
	if (errorCode == 1 || errorCode == 5)
	{
//		slotDisconnected();
	   OscarAccount::disconnect();
	}
	// FIXME: maybe goLogoff() instead to properly shutdown the connection

	KMessageBox::error(qApp->mainWidget(), errmsg);
}


// Called when we get disconnected
/*
void OscarAccount::slotDisconnected()
{
	kdDebug(14150) << k_funcinfo << "accountId='" << accountId() << "'" << endl;
}
*/

// Called when a group is added by adding a contact
void OscarAccount::slotGroupAdded(KopeteGroup *group)
{
	kdDebug(14150) << k_funcinfo <<
		"called, groupname='" << group->displayName() << "'" << endl;

	QString groupName = group->displayName();
	if(groupName.isEmpty())
	{
		kdDebug(14150) << k_funcinfo <<
			"FUCK KOPETE, I'm not adding groups with no names" << endl;
		return;
	}

	// See if we already have this group
	AIMGroup *aGroup = mInternalBuddyList->findGroup(groupName);
	if (!aGroup)
	{
		aGroup = mInternalBuddyList->addGroup(mRandomNewGroupNum, groupName);
		mRandomNewGroupNum++;
		kdDebug(14150) << k_funcinfo << accountId() << ":addGroup() being called" << endl;
		if (isConnected())
			engine()->sendAddGroup(groupName);
	}
	else
	{ // The server already has it in it's list, don't worry about it
		kdDebug(14150) << k_funcinfo << "Group already existing" << endl;
	}
}

void OscarAccount::slotKopeteGroupRenamed(KopeteGroup *group, const QString &oldName)
{
	kdDebug(14150) << k_funcinfo << "Sending 'group rename' to server" << endl;
	engine()->sendChangeGroupName(oldName, group->displayName());
}

void OscarAccount::slotKopeteGroupRemoved(KopeteGroup *group)
{
	// This method should be called after the contacts have been removed
	// We should then be able to remove the group from the server
	kdDebug(14150) << k_funcinfo <<
		"Sending 'delete group' to server, group='" <<
		group->displayName() << "'" << endl;

	QString groupName = group->displayName();
	if(groupName.isEmpty())
	{
		kdDebug(14150) << k_funcinfo <<
			"[EXPLETIVE REMOVED], Removing a group with no name!" << endl;
		return;
	}

	AIMGroup *aGroup = mInternalBuddyList->findGroup(groupName);
	if (aGroup)
	{
		engine()->sendDelGroup(groupName);
		// and remove the group from our AIMBuddyList
		mInternalBuddyList->removeGroup( aGroup->ID() );
	}
}

// Called when we have gotten an IM
void OscarAccount::slotGotIM(QString /*message*/, QString sender, bool /*isAuto*/)
{
//	kdDebug(14150) << k_funcinfo << "account='"<< accountId() <<
//		"', sender='" << sender << "'" << endl;

	//basically, all this does is add the person to your buddy list
	// TODO: Right now this has no support for "temporary" buddies
	// because I could not think of a good way to do this with
	// AIM.
	if(!mInternalBuddyList->findBuddy(sender))
	{
		kdDebug(14150) << k_funcinfo <<
			"Message from contact that is not on our contactlist, sender='" <<
			sender << "'" << endl;
		addContact(tocNormalize(sender), sender, 0L, QString::null, true); // last one should be true
	}
}

// Called when we retrieve the buddy list from the server
void OscarAccount::slotGotServerBuddyList(AIMBuddyList &buddyList)
{
	kdDebug(14150) << k_funcinfo << "account='" << accountId() << "'" << endl;

//	QValueList<AIMBuddy *> ll = mInternalBuddyList->buddies().values();
//	QValueList<AIMBuddy *> sl = buddyList.buddies().values();
//	for (QValueList<AIMBuddy *>::Iterator it=ll.begin(); it!=ll.end(); ++it)
//		qDebug("-- Local: %s, id: %i, groupId: %i", (*it)->screenname().latin1(), (*it)->ID(), (*it)->groupID());
//	for (QValueList<AIMBuddy *>::Iterator it=sl.begin(); it!=sl.end(); ++it)
//		qDebug("-- Server: %s, id: %i, groupId: %i", (*it)->screenname().latin1(), (*it)->ID(), (*it)->groupID());


	//save server side contact list
	*mInternalBuddyList += buddyList;
	QValueList<AIMBuddy *> localList = buddyList.buddies().values();

	for (QValueList<AIMBuddy *>::Iterator it=localList.begin(); it!=localList.end(); ++it)
	{
		if ((*it))
			addServerContact((*it)); // Add the server contact to Kopete list
	}

	syncLocalWithServerBuddyList( buddyList );
}

//
// Since the icq_new transition, I have lots of contacts, that are not in the serverside
// contact list. So we compare the two lists and add all local contacts that are not
// in the server side list.
//
void OscarAccount::syncLocalWithServerBuddyList( AIMBuddyList& serverList )
{
	const QDict<KopeteContact>& contactList = contacts();
	QDictIterator<KopeteContact> it( contactList );
	for ( ; it.current(); ++it )
	{
		QString contactId = static_cast<OscarContact*>( it.current() )->contactName();
		AIMBuddy *buddy = serverList.findBuddy( contactId );
		if ( !buddy && it.current() != mMyself )
		{
			kdDebug(14150) << "###### Serverside list doesn't contain local buddy: " << contactId << endl;
			kdDebug(14150) << "-> creating server side contact for it." << endl;
			// Add the buddy to the server's list
			const KopeteGroupList& groups = it.current()->metaContact()->groups();
			AIMGroup *group = findOrCreateGroup( groups.isEmpty() ? QString::null : groups.getFirst()->displayName(), serverList);
			if ( group )
				engine()->sendAddBuddy(tocNormalize(contactId), group->name());
		}
	}
}

// Looks for the group localGroup in the server-side list.
// If it doesn't find it there, creates and returns it.
//
AIMGroup * OscarAccount::findOrCreateGroup( const QString& localGroup, AIMBuddyList& serverList )
{
	QString groupName = localGroup.isEmpty() ? QString::fromLatin1( "Buddies"  ) : localGroup;

	// See if it exists in our internal group list already
	AIMGroup *internalGroup = serverList.findGroup( groupName );

	// If the group didn't exist, take it from the local list
	if (!internalGroup)
	{
		kdDebug(14150) << k_funcinfo << "Group doesn't exist on server list, create it:" << groupName << endl;
		internalGroup = mInternalBuddyList->findGroup( groupName );
		if ( !internalGroup ) { // or create it
			internalGroup = mInternalBuddyList->addGroup(mRandomNewGroupNum++, groupName);
		}

		// Add the group on the server list
		if ( internalGroup )
			engine()->sendAddGroup(internalGroup->name());
	}

	return internalGroup;
}

void OscarAccount::addServerContact(AIMBuddy *buddy)
{
	kdDebug(14150) << k_funcinfo << "Called for '" << buddy->screenname() << "'" << endl;

	//screennames are case insensitive and formatting which server
	//pushes might be different
	if(mMyself->contactId().contains(buddy->screenname(), false))
	{
		kdDebug(14150) << k_funcinfo <<
			"EEEK! Cannot have yourself on your own list! Aborting" << endl;
		return;
	}

	// Try and find an already existing Kopete side contact
	// with this buddy name
	OscarContact *contact = static_cast<OscarContact*>(contacts()[tocNormalize(buddy->screenname())]);

	QString nick;
	if(!buddy->alias().isEmpty())
		nick=buddy->alias();
	else
		nick=buddy->screenname();

	if (contact)
	{
		// Contact existed in the list already, sync information
		// FIXME: is this needed? won't work anymore as AIMBuddy doesn't return a KOS on status()!
//		contact->setOnlineStatus( buddy->status() );
// 		kdDebug(14150) << k_funcinfo <<
// 			"serverside contact already in kopete, his status=" << buddy->status() << endl;
		if(contact->displayName()!=nick)
			contact->rename(nick);
		contact->syncGroups();
	}
	else
	{
		kdDebug(14150) << k_funcinfo << "Adding new contact from Serverside List to Kopete" << endl;

		// Get the group this buddy belongs to
		AIMGroup *aimGroup = mInternalBuddyList->findGroup(buddy->groupID());
		if (aimGroup)
		{
			kdDebug(14150) << k_funcinfo << "Found its group on server, groupname=" <<
				aimGroup->name() << endl;

			// If the group exists in the internal list
			// Add contact to the kopete contact list, with no metacontact
			// which creates a new one. This will also call the
			// addContactToMetaContact method in this class
			addContact(tocNormalize(buddy->screenname()), nick, 0L, aimGroup->name(), false);
		}
		else
		{
			kdDebug(14150) << k_funcinfo << "DIDN'T find its group on server" << endl;
			// If the group doesn't exist on the server yet.
			// This is really strange if we have the contact
			// on the server but not the group it's in.
			// May have to do something here in the future
		}
	}
}

void OscarAccount::slotGotDirectIMRequest(QString sn)
{
	QString title = i18n("Direct IM session request");
	QString message =
	i18n("%1 has requested a direct IM session with you. " \
		"Direct IM sessions allow the remote user to see your IP " \
		"address, which can lead to security problems if you don't " \
		"trust him/her. Do you want to establish a direct connection " \
		"to %2?")
#if QT_VERSION < 0x030200
			.arg(sn).arg(sn);
#else
			.arg(sn,sn);
#endif



	int result = KMessageBox::questionYesNo(qApp->mainWidget(), message, title);

	if (result == KMessageBox::Yes)
		engine()->sendDirectIMAccept(sn);
	else if (result == KMessageBox::No)
		engine()->sendDirectIMDeny(sn);
}

void OscarAccount::slotIdleActivity()
{
	// Reset the idle timer to 10 minutes
	// TODO Make this configurable
	if (mIdleTimer != 0L)
	{
		mIdleTimer->stop();
//		mIdleTimer->start(10 * 60 * 1000);
	}

	if (mAreIdle)
	{ // If we _are_ idle, change it
		kdDebug(14150) << k_funcinfo
					   << "system change to ACTIVE, setting idle "
					   << "time with server to 0" << endl;
		engine()->sendIdleTime(0);
		mAreIdle = false;
	}
}

// Called when there is no activity for a certain amount of time
void OscarAccount::slotIdleTimeout()
{
	kdDebug(14150) << k_funcinfo
				   << "system is IDLE, setting idle time with "
				   << "server" << endl;
	engine()->sendIdleTime(
		KopeteAway::getInstance()->idleTime());
	// Restart the idle timer, so that we keep
	// updating idle times while we're idle
	// Every minute
	mIdleTimer->stop();
//	mIdleTimer->start(60 * 1000);
	// Set the idle flag
	mAreIdle = true;
}

int OscarAccount::randomNewBuddyNum()
{
	return mRandomNewBuddyNum++;
}

int OscarAccount::randomNewGroupNum()
{
	return mRandomNewGroupNum++;
}

AIMBuddyList *OscarAccount::internalBuddyList() const
{
	return mInternalBuddyList;
}

void OscarAccount::setServerAddress(const QString &server)
{
	kdDebug(14150) << k_funcinfo << "Called, server=" << server << endl;
	setPluginData(protocol(), "Server", server);
}

void OscarAccount::setServerPort(int port)
{
	kdDebug(14150) << k_funcinfo << "Called, port=" << port << endl;
	if (port > 0)// Do a little error checkin on it
		setPluginData(protocol(), "Port", QString::number(port));
}

bool OscarAccount::addContactToMetaContact(const QString &contactId,
	const QString &displayName, KopeteMetaContact *parentContact)
{
	/*
	 * This method is called in three situations.
	 * The first one is when the user, through the GUI
	 * adds a buddy and it happens to be from this account.
	 * In this situation, we need to create an internal buddy
	 * entry for the new contact, perhaps create a new group,
	 * and notify the server of both of these things.
	 *
	 * The second situation is where we are loading a server-
	 * side contact list through the method addServerContact
	 * which calls addContact, which in turn calls this method.
	 * To cope with this situation we need to first check if
	 * the contact already exists in the internal list.
	 *
	 * FIXME: it seems at the moment we are re-adding the
	 * server-side contact to the server-side list!
	 *
	 * The third situation is when somebody new messages you
	 */

	/* We're not even online or connecting
	 * (when getting server contacts), so don't bother
	 */
	if(
		(!myself()->isOnline()) &&
		(myself()->onlineStatus().status() != KopeteOnlineStatus::Connecting)
		)
	{
		kdDebug(14150) << k_funcinfo
					   << "Can't add contact, we are offline!" << endl;
		return false;
	}

	// Next check our internal list to see if we have this buddy
	// already, findBuddy tocNormalizes the buddy name for us
	AIMBuddy *internalBuddy = mInternalBuddyList->findBuddy(contactId);

	if (internalBuddy) // We found the buddy internally
	{
		kdDebug(14150) << k_funcinfo << "Found buddy internally, just make"
				<< " a new OscarContact subclass for it." << endl;
		// Create an OscarContact for the metacontact
		if(OscarContact* newContact = createNewContact(contactId, displayName, parentContact))
		{
			// Set the oscar contact's status
			newContact->setStatus(internalBuddy->status());
			return true;
		}
		else
			return false;
	}
	else // It was not on our internal list yet
	{
		kdDebug(14150) << k_funcinfo << "New Contact '" << contactId
			<< "' wasn't in internal list. Creating new "
			<< "internal list entry" << endl;

		// Check to see if it's a temporary contact, ie. not on our list
		// but IMed us anyway
		if(!parentContact->isTemporary())
		{
			kdDebug(14150) << k_funcinfo <<
				"real new contact, also going to add him to the " <<
				 "serverside contactlist" << endl;

			QString groupName;
			// Get a list of the groups it's in
			KopeteGroupList kopeteGroups = parentContact->groups();

			if(kopeteGroups.isEmpty())
			{
				kdDebug(14150) << k_funcinfo
					<< "Contact with no group, adding to group 'Buddies'" << endl;
				// happens for temporary contacts
				// TODO: support for temp contacts is screwed by design [mETz]
				groupName="Buddies";
			}
			else
			{
				kdDebug(14150) << k_funcinfo << "Contact with group, grouplist count="
					<< kopeteGroups.count() << endl;

				// OSCAR doesn't support a contact in multiple groups, so we
				// just take the first one
				KopeteGroup *group = kopeteGroups.first();
				// Get the name of the group
				groupName = group->displayName();

				kdDebug(14150) << k_funcinfo << "groupName='" << groupName << "'" << endl;
			}

			if(groupName.isEmpty())
			{ // emergency exit, should never occur
				kdDebug(14150) << k_funcinfo << "Could not add Contact because no " <<
					"groupname was given" << endl;
				return false;
			}

			// See if it exists in our internal group list already
			AIMGroup *internalGroup = mInternalBuddyList->findGroup(groupName);

			// If the group didn't exist
			if (!internalGroup)
			{
				internalGroup =
					mInternalBuddyList->addGroup(mRandomNewGroupNum, groupName);
				kdDebug(14150) << "created internal group for new contact" << endl;
				// Add the group on the server list
				engine()->sendAddGroup(internalGroup->name());
			}

			// Create a new internal buddy for this contact
			AIMBuddy *newBuddy =
				new AIMBuddy(mRandomNewBuddyNum,
							 internalGroup->ID(), contactId);

			// Check if it has an alias
			if((displayName != QString::null) && (displayName != contactId))
				newBuddy->setAlias(displayName);

			// Add the buddy to the internal buddy list
			mInternalBuddyList->addBuddy( newBuddy );

			// Add the buddy to the server's list, with the group,
			// need to normalize the contact name
			engine()->sendAddBuddy(tocNormalize(contactId),
									  internalGroup->name());

			// Increase these counters, I'm not sure what this does
			mRandomNewGroupNum++;
			mRandomNewBuddyNum++;

			// Create the actual contact, which adds it to the metacontact
			return ( createNewContact( contactId, displayName, parentContact ));
		}
		else
		{
			kdDebug(14150) << "Temporary new contact, only adding him to local list" << endl;
			// This is a temporary contact, so don't add it to the server list
			// Create the contact, which adds it to the parent contact
			if(createNewContact(contactId, displayName, parentContact))
			{
				// Set it's initial status
				// This requests the buddy's info from the server
				// I'm not sure what it does if they're offline, but there
				// is code in oscarcontact to handle the signal from
				// the engine that this causes
				kdDebug(14150) << k_funcinfo <<
					"Requesting user info for '" << contactId << "'" << endl;
				engine()->sendUserProfileRequest(tocNormalize(contactId));
				return true;
			}
			else
				return false;
		}
	} // END not internalBuddy

}

void OscarAccount::slotOurStatusChanged(const unsigned int newStatus)
{
	kdDebug(14150) << k_funcinfo << "Called; newStatus=" << newStatus << endl;
	mMyself->setStatus(newStatus);
}

void OscarAccount::slotReTryServerContacts()
{
	kdDebug(14150) << k_funcinfo << "Called, mGroupQueue.count()=" <<
		mGroupQueue.count() << endl;

	// Process the queue
	int i = 0;
	for (AIMBuddy *it = mGroupQueue.at(i); it != 0L; it = mGroupQueue.at( ++i ))
	{
		// Success, group now exists, add contact
		if (mInternalBuddyList->findGroup(it->groupID()))
		{
			mGroupQueue.remove(i);
			addOldContact(it);
		}
	}
}

// Adds a contact that we already know about to the list
void OscarAccount::addOldContact(AIMBuddy *bud,KopeteMetaContact *meta)
{
	bool temporary = false;

	AIMGroup *group = mInternalBuddyList->findGroup(bud->groupID());
	if (!group && bud)
	{
		kdDebug(14150) << k_funcinfo <<
			"Adding AIMBuddy '" << bud->ID() << "' to groupQueue " <<
			"to check later once we know about the group." << endl;
		mGroupQueue.append(bud);
		return;
	}

	mInternalBuddyList->addBuddy(bud);
	if(!mInternalBuddyList->findBuddy(bud->screenname()) ) return;

	if (group->name().isNull())
		temporary = true;

	kdDebug(14150) << k_funcinfo << "Called, groupName=" << group->name() << endl;

	KopeteMetaContact *m = KopeteContactList::contactList()->findContact(
		protocol()->pluginId(), accountId(), bud->screenname());

	kdDebug(14150) << k_funcinfo << "KopeteMetaContact m=" << m << endl;

	if(m)
	{
		// Existing contact, update data that is ALREADY in the LOCAL database for AIM
		if (m->isTemporary())
			m->setTemporary(false);
	}
	else
	{
		// New contact
		kdDebug(14150) << k_funcinfo << "Adding old contact '" <<
			bud->screenname() << "'" << endl;

		if(meta)
			m=meta;
		else
		{
			m=new KopeteMetaContact();
			if( !temporary )
				m->addToGroup( KopeteContactList::contactList()->getGroup(group->name()) );
		}

		if (temporary)
			m->setTemporary(true);

		QString nick;
		if( !bud->alias().isEmpty() )
			nick = bud->alias();
		else
			nick = bud->screenname();

		createNewContact(bud->screenname(), nick, m);

		if (!meta)
			KopeteContactList::contactList()->addMetaContact(m);
	}
}

#include "oscaraccount.moc"
// vim: set noet ts=4 sts=4 sw=4:
