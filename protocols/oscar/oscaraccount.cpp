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
#include "aim.h"
#include "aimbuddy.h"
#include "aimgroup.h"

#include "kopeteprotocol.h"
#include "kopeteaway.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopeteawaydialog.h"
#include "kopetegroup.h"

#include <assert.h>

#include <qapplication.h>
#include <qregexp.h>
#include <qstylesheet.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

class OscarAccountPrivate
{
public:
    /**
	 * Server-side AIMBuddies that do not have KopeteContacts yet for the reason that
	 * their group has not yet been sent from the server
	 * See aimbuddylist.h under 'signals:' for an explanation of this.
	 * This is 'the queue'
	 */
	QPtrList<AIMBuddy> groupQueue;

	/*
	 * Our OSCAR socket object
	 */
	OscarSocket *engine;

	/*
	 * Random new group/contact number for the engine
	 */
	int randomNewGroupNum;
	int randomNewBuddyNum;

	/*
	 * This flag is used internally to keep track
	 * of if we're idle or not
	 */
	bool isIdle;

	/*
	 * Last idle time in seconds we sent to the server
	 */
	int lastIdleValue;

	/*
	 * anti SPAM feature :)
	 */
	bool ignoreUnknownContacts;

	/*
	 * This is our idle timer, it is used internally
	 * to represent idle times and report them to
	 * the server
	 */
	QTimer *idleTimer;

	QString awayMessage;

	// -- MERGED DATA FROM AIMBUDDYLIST ----------------------------------
	QMap<int, AIMGroup *> groupMap;
	QMap<QString, AIMBuddy *> buddyNameMap;
	QMap<QString, AIMGroup *> groupNameMap;
	// -- END MERGED DATA FROM AIMBUDDYLIST ------------------------------

	bool passwordWrong;
};

OscarAccount::OscarAccount(KopeteProtocol *parent, const QString &accountID, const char *name, bool isICQ)
: KopeteAccount( parent, accountID, name )
{
	kdDebug(14150) << k_funcinfo << " accountID='" << accountID <<
		"', isICQ=" << isICQ << endl;

	d = new OscarAccountPrivate;

	d->engine = 0L;
	// Set our random new numbers
	d->randomNewBuddyNum = 0;
	d->randomNewGroupNum = 0;
	d->ignoreUnknownContacts = false;
	d->isIdle = false;
	d->lastIdleValue = 0;
	d->awayMessage = "";
	d->passwordWrong = false;

	initEngine(isICQ); // Initialize the backend

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

	QObject::connect(
		engine(), SIGNAL(receivedMessage(const QString &, OscarMessage &, OscarSocket::OscarMessageType)),
		this, SLOT(slotReceivedMessage(const QString &, OscarMessage &, OscarSocket::OscarMessageType)));

	QObject::connect(
		engine(), SIGNAL(receivedAwayMessage(const QString &, const QString &)),
		this, SLOT(slotReceivedAwayMessage(const QString &, const QString &)));

	// Got Config (Buddy List)
	QObject::connect(
		engine(), SIGNAL(gotConfig()),
		this, SLOT(slotGotServerBuddyList()));

	// Got direct IM request
	QObject::connect(
		engine(), SIGNAL(gotDirectIMRequest(QString)),
		this, SLOT(slotGotDirectIMRequest(QString)));

	d->idleTimer = new QTimer(this, "OscarIdleTimer");
	QObject::connect(
		d->idleTimer, SIGNAL(timeout()),
		this, SLOT(slotIdleTimeout()));

	QObject::connect( d->engine, SIGNAL( loggedIn() ), this, SLOT( slotLoggedIn() ) );
}

OscarAccount::~OscarAccount()
{
	//kdDebug(14150) << k_funcinfo << "'" << accountId() << "'" << endl;

	OscarAccount::disconnect();

	// Delete the backend
	if (d->engine)
	{
		//kdDebug(14150) << k_funcinfo << "'" << accountId() << "'; delayed deleting of d->engine" << endl;
		d->engine->deleteLater();
	}

	delete d;
}

OscarSocket* OscarAccount::engine() const
{
	assert(d->engine);
	return d->engine;
}

void OscarAccount::disconnect()
{
	kdDebug(14150) << k_funcinfo << "accountId='" << accountId() << "'" << endl;
	d->engine->doLogoff();
}

bool OscarAccount::passwordWasWrong()
{
	return d->passwordWrong;
}

void OscarAccount::initEngine(bool icq)
{
	//kdDebug(14150) << k_funcinfo << "accountId='" << accountId() << "'" << endl;

	QByteArray cook;
	cook.duplicate("01234567",8);
	d->engine = new OscarSocket(pluginData(protocol(),"Server"), cook,
		this, this, "d->engine", icq);
}

void OscarAccount::slotGoOffline()
{
	OscarAccount::disconnect();
}

void OscarAccount::slotError(QString errmsg, int errorCode)
{
	kdDebug(14150) << k_funcinfo << "accountId='" << accountId() <<
		"' errmsg=" << errmsg <<
		", errorCode=" << errorCode << "." << endl;

	// 1 = username unknown to server
	// 5 = wrong password
	if (errorCode == 1 || errorCode == 5 || errorCode == 24)
		OscarAccount::disconnect();

	KMessageBox::queuedMessageBox(0, KMessageBox::Error, errmsg,
		i18n("Connection Lost - ICQ Plugin"), KMessageBox::Notify);

	if (errorCode == 5)
	{
		d->passwordWrong = true;
		connect();
	}
}

void OscarAccount::slotReceivedMessage(const QString &sender, OscarMessage &incomingMessage, OscarSocket::OscarMessageType type)
{
	kdDebug(14150) << k_funcinfo << "account='" << accountId() <<
		"', type=" << static_cast<int>(type) << ", sender='" << sender << "'" << endl;

	OscarContact *contact = static_cast<OscarContact*>(contacts()[tocNormalize(sender)]);
	QString text = incomingMessage.text();

	if(!contact && !d->ignoreUnknownContacts)
	{
		//basically, all this does is add the person to your buddy list
		// TODO: Right now this has no support for "temporary" buddies
		// because I could not think of a good way to do this with
		// AIM.

		kdDebug(14150) << k_funcinfo <<
			"Message from contact that is not on our contactlist, sender='" <<
			sender << "'" << endl;

		if (addContact(tocNormalize(sender), sender, 0L, KopeteAccount::DontChangeKABC, QString::null, true))
			contact = static_cast<OscarContact*>(contacts()[tocNormalize(sender)]);
		else
			return; // adding contact failed for whatever reason!
	}

	if (contact)
	{
		switch(type)
		{
			case OscarSocket::Away:
				text = i18n("<b>[Away Message:]</b> %1").arg(text);
				break;

			case OscarSocket::URL:
				text.replace("þ", "<br />");
				text=i18n("<b>[URL Message:]</b> %1").arg(text);
				break;

			case OscarSocket::SMS:
				text=i18n("<b>[SMS Message:]</b> %1").arg(text);
				break;

			case OscarSocket::EMail:
				text=i18n("<b>[Email Message:]</b> %1").arg(text);
				break;

			case OscarSocket::WebPanel:
				text.replace(QString::fromLatin1("þþþ"), QString::fromLatin1("<br />"));
				text.replace(QString::fromLatin1("þ3þ"), QString::fromLatin1("<br />"));
				text=i18n("<b>[WebPanel Message:]</b> %1").arg(text);
				break;

			case OscarSocket::Normal:
				break;

			case OscarSocket::GrantedAuth:
				text=i18n("<b>[Granted authentication:]</b> %1").arg(text);
				break;

			case OscarSocket::DeclinedAuth:
				text=i18n("<b>[Declined authentication:]</b> %1").arg(text);
				break;
		}

		KopeteContactPtrList tmpList;
		tmpList.append(myself());

		KopeteMessage kmsg(
			incomingMessage.timestamp, contact, tmpList, text, KopeteMessage::Inbound,
			KopeteMessage::RichText);

		kmsg.setFg(incomingMessage.fgColor);
		kmsg.setBg(incomingMessage.bgColor);

		contact->receivedIM(kmsg);
	}
}

void OscarAccount::slotReceivedAwayMessage(const QString &sender, const QString &message)
{
	/*kdDebug(14150) << k_funcinfo << "account='" << accountId() <<
		", sender='" << sender << "'" << endl;*/

	OscarContact *contact = static_cast<OscarContact*>(contacts()[tocNormalize(sender)]);
	if(contact)
		contact->setAwayMessage(message);
}

// Called when a group is added by adding a contact
void OscarAccount::slotGroupAdded(KopeteGroup *group)
{
	kdDebug(14150) << k_funcinfo <<
		"called, groupname='" << group->displayName() << "'" << endl;

	QString groupName = group->displayName();
	if(groupName.isEmpty())
	{
		kdDebug(14150) << k_funcinfo <<
			"I'm not adding groups with no names!" << endl;
		return;
	}

	// See if we already have this group
	AIMGroup *aGroup = findGroup(groupName);
	if (!aGroup)
	{
		aGroup = addGroup(d->randomNewGroupNum, groupName);
		d->randomNewGroupNum++;
		kdDebug(14150) << k_funcinfo <<
			"'" << accountId() << "' addGroup() being called" << endl;
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
		kdDebug(14150) << k_funcinfo << "Removing a group with no name!" << endl;
		return;
	}

	AIMGroup *aGroup = findGroup(groupName);
	if (aGroup)
	{
		engine()->sendDelGroup(groupName);

		// and remove the group from our BuddyList
		AIMGroup *group = d->groupMap[ aGroup->ID() ];
		if ( !group )
			return;
		d->groupNameMap.remove( group->name() );
		d->groupMap.remove( aGroup->ID() );
		delete group; // also deletes the buddies in that group
	}
}

void OscarAccount::slotGotServerBuddyList()
{
	kdDebug( 14150 ) << k_funcinfo << "account='" << accountId() << "'" << endl;

	for ( QMap<QString, AIMBuddy *>::ConstIterator it = d->buddyNameMap.begin(); it != d->buddyNameMap.end(); ++it )
	{
		if ( it.data()->isServerSide() )
			addServerContact( it.data() ); // Add the server contact to Kopete list
	}

	// Old location
	//syncLocalWithServerBuddyList();
}

void OscarAccount::slotLoggedIn()
{
	kdDebug(14150) << k_funcinfo << "Called" << endl;

	d->passwordWrong = false;

	// Only call sync if we received a list on connect, does not happen on @mac AIM-accounts
	bool haveServerSideContacts = false;
	for ( QMap<QString, AIMBuddy *>::ConstIterator it = d->buddyNameMap.begin(); it != d->buddyNameMap.end(); ++it )
	{
		if ( it.data()->isServerSide() )
		{
			haveServerSideContacts = true;
			break;
		}
	}

	if ( haveServerSideContacts )
	{
		// FIXME: Use proper rate limiting rather than a fixed 2 second delay - Martijn
		QTimer::singleShot( 2000, this, SLOT( slotDelayedListSync() ) );
	}

	d->idleTimer->start(10 * 1000);
}

void OscarAccount::slotDelayedListSync()
{
	kdDebug(14150) << k_funcinfo << "Called" << endl;

	syncLocalWithServerBuddyList ();
}

//
// Since the icq_new transition, I have lots of contacts, that are not in the serverside
// contact list. So we compare the two lists and add all local contacts that are not
// in the server side list.
//

void OscarAccount::syncLocalWithServerBuddyList()
{
	kdDebug(14150) << k_funcinfo << "Called but DISABLED" << endl;
#if 0
	//FIXME: Does not work [mETz]
	const QDict<KopeteContact>& contactList = contacts();
	QDictIterator<KopeteContact> it( contactList );

	for(; it.current(); ++it)
	{
		QString contactId = static_cast<OscarContact*>( it.current() )->contactName();
		QString displayName = static_cast<OscarContact*>( it.current() )->displayName();

		AIMBuddy *buddy = d->loginContactlist->findBuddy( contactId );

		if(!buddy && it.current() != myself())
		{
			kdDebug(14150) << "###### Serverside list doesn't contain local buddy: " << contactId << endl;
			kdDebug(14150) << "-> creating server side contact for it." << endl;
			// Add the buddy to the server's list
			const KopeteGroupList& groups = it.current()->metaContact()->groups();

			AIMGroup *group = findOrCreateGroup( groups.isEmpty() ? QString::null : groups.getFirst()->displayName() );

			// Create a new internal buddy for this contact
			AIMBuddy *newBuddy =
				new AIMBuddy(d->randomNewBuddyNum++, group->ID(), tocNormalize(contactId));

			// Check if it has an alias
			if((displayName != QString::null) && (displayName != contactId))
				newBuddy->setAlias(displayName);

			// Add the buddy to the internal buddy list
			addBuddy( newBuddy );

			// Add the buddy to the server's list, with the group,
			// need to normalize the contact name
			engine()->sendAddBuddy(tocNormalize(contactId), group->name());
		}
	}
#endif
}

// Looks for the group localGroup in the server-side list.
// If it doesn't find it there, creates and returns it.
AIMGroup * OscarAccount::findOrCreateGroup( const QString& localGroup )
{
	QString groupName = localGroup.isEmpty() ? QString::fromLatin1("Buddies") : localGroup;
	// See if it exists in our internal group list already
	AIMGroup *internalGroup = findGroup( groupName );

	// If the group didn't exist, take it from the local list
	if (!internalGroup)
	{
		kdDebug(14150) << k_funcinfo << "Group doesn't exist on server list, create it:" << groupName << endl;
		internalGroup = findGroup( groupName );
		if(!internalGroup)
			internalGroup = addGroup(d->randomNewGroupNum++, groupName);

		// Add the group on the server list
		if(internalGroup)
			engine()->sendAddGroup(internalGroup->name());
	}
	return internalGroup;
}

void OscarAccount::addServerContact(AIMBuddy *buddy)
{
	kdDebug(14150) << k_funcinfo << "Called for '" << buddy->screenname() << "'" << endl;

	//screennames are case insensitive and formatting which server
	//pushes might be different
	if(myself()->contactId().contains(buddy->screenname(), false))
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

		if(buddy->waitAuth())
			kdDebug(14150) << k_funcinfo << "setting WAITAUTH on '" << contact->displayName() << "'" << endl;

		contact->setWaitAuth( buddy->waitAuth() );

		/* FIXME: sometimes renames contacts on server to the UIN/screenname
		if( contact->displayName() != nick )
			contact->rename(nick);
		 */

		contact->setGroupId( buddy->groupID() );
		contact->syncGroups();
	}
	else
	{
		kdDebug(14150) << k_funcinfo << "Adding new contact from Serverside List to Kopete" << endl;

		// Get the group this buddy belongs to
		AIMGroup *aimGroup = findGroup(buddy->groupID());
		if (aimGroup)
		{
			kdDebug(14150) << k_funcinfo << "Found its group on server, groupname=" <<
				aimGroup->name() << endl;

			// If the group exists in the internal list
			// Add contact to the kopete contact list, with no metacontact
			// which creates a new one. This will also call the
			// addContactToMetaContact method in this class
			addContact(tocNormalize(buddy->screenname()), nick, 0L, KopeteAccount::ChangeKABC, aimGroup->name(), false);
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
	//FIXME: This is a stupid question, ICQ always accepts
	// these as long as you allow direct connections in general

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

	int result = KMessageBox::questionYesNo(0, message, title);

	if (result == KMessageBox::Yes)
		engine()->sendDirectIMAccept(sn);
	else if (result == KMessageBox::No)
		engine()->sendDirectIMDeny(sn);
}

void OscarAccount::slotIdleTimeout()
{
	//kdDebug(14150) << k_funcinfo << "called" << endl;
	int idletime = KopeteAway::getInstance()->idleTime();
	// not doing anything for more than 5 mins and still not idle
	if(idletime >= 5*60)
	{
		if (idletime >= d->lastIdleValue + 60)
		{
			/*kdDebug(14150) << k_funcinfo <<
				"sending idle time to server, idletime=" << idletime << endl;*/
			engine()->sendIdleTime(idletime);
			d->lastIdleValue = idletime;

			// Set the idle flag
			d->isIdle = true;
		}
	}
	else
	{
		if (d->isIdle)
		{ // If we _are_ idle, change it
			/*kdDebug(14150) << k_funcinfo <<
				"system change to ACTIVE, setting idle time with server to 0" << endl;*/
			engine()->sendIdleTime(0);
			d->isIdle = false;
			d->lastIdleValue = 0;
		}
	}
}

int OscarAccount::randomNewBuddyNum()
{
	return d->randomNewBuddyNum++;
}

int OscarAccount::randomNewGroupNum()
{
	return d->randomNewGroupNum++;
}

void OscarAccount::setServerAddress(const QString &server)
{
	kdDebug(14150) << k_funcinfo << "Called, server=" << server << endl;
	setPluginData(protocol(), "Server", server);
}

void OscarAccount::setServerPort(int port)
{
	kdDebug(14150) << k_funcinfo << "Called, port=" << port << endl;
	if (port > 0)// Do a little error checking on it
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
	 * This is not true. No server-side contact list manipulation
	 * is done at this point in time. (Matt)
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
	AIMBuddy *internalBuddy = findBuddy(contactId);

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
			AIMGroup *internalGroup = findGroup(groupName);

			// If the group didn't exist
			if (!internalGroup)
			{
				internalGroup =
					addGroup(d->randomNewGroupNum, groupName);
				kdDebug(14150) << "created internal group for new contact" << endl;
				// Add the group on the server list
				engine()->sendAddGroup(internalGroup->name());
			}

			// Create a new internal buddy for this contact
			AIMBuddy *newBuddy =
				new AIMBuddy(d->randomNewBuddyNum,
							 internalGroup->ID(), contactId);

			// Check if it has an alias
			if((displayName != QString::null) && (displayName != contactId))
				newBuddy->setAlias(displayName);

			// Add the buddy to the internal buddy list
			addBuddy( newBuddy );

			// Add the buddy to the server's list, with the group,
			// need to normalize the contact name
			engine()->sendAddBuddy(tocNormalize(contactId), internalGroup->name(), false);

			// Increase these counters, I'm not sure what this does
			d->randomNewGroupNum++;
			d->randomNewBuddyNum++;

			// Create the actual contact, which adds it to the metacontact
			return(createNewContact(contactId, displayName, parentContact));
		}
		else
		{
			kdDebug(14150) << "Temporary new contact, only adding him to local list" << endl;
			// This is a temporary contact, so don't add it to the server list
			// Create the contact, which adds it to the parent contact
			if(!createNewContact(contactId, displayName, parentContact))
				return false;

			// Get user status through BLM if contact is temporary (ICQ only)
			if ( engine()->isICQ() )
				engine()->sendAddBuddylist(tocNormalize(contactId));

			// Set it's initial status
			// This requests the buddy's info from the server
			// I'm not sure what it does if they're offline, but there
			// is code in oscarcontact to handle the signal from
			// the engine that this causes
			//if(!d->engine->isICQ())
			{
				kdDebug(14150) << k_funcinfo <<
					"Requesting user info for '" << contactId << "'" << endl;
				engine()->sendUserLocationInfoRequest(tocNormalize(contactId), AIM_LOCINFO_SHORTINFO);
			}
			return true;
		}
	} // END not internalBuddy

}

void OscarAccount::slotOurStatusChanged(const unsigned int newStatus)
{
	kdDebug(14150) << k_funcinfo << "Called; newStatus=" << newStatus << endl;
	static_cast<OscarContact *>( myself() )->setStatus(newStatus);

	if(newStatus == OSCAR_OFFLINE)
		d->idleTimer->stop();
}

// Adds a contact that we already know about to the list
void OscarAccount::addOldContact(AIMBuddy *bud, KopeteMetaContact *meta)
{
	bool temporary = false;

	AIMGroup *group = findGroup(bud->groupID());
	if (!group && bud)
	{
		kdDebug(14150) << k_funcinfo <<
			"Adding AIMBuddy '" << bud->ID() << "' to groupQueue " <<
			"to check later once we know about the group." << endl;
		d->groupQueue.append(bud);
		return;
	}

	addBuddy(bud);
	if(!findBuddy(bud->screenname()) ) return;

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

void OscarAccount::setAwayMessage(const QString &msg)
{
	d->awayMessage = msg;
}

const QString &OscarAccount::awayMessage()
{
	return d->awayMessage;
}

bool OscarAccount::ignoreUnknownContacts() const
{
	return d->ignoreUnknownContacts;
}

void OscarAccount::setIgnoreUnknownContacts( bool b )
{
	d->ignoreUnknownContacts = b;
}

// -- MERGED CODE FROM AIMBUDDYLIST ----------------------------------
void OscarAccount::addBuddy(AIMBuddy *buddy)
{
	d->buddyNameMap.insert(tocNormalize(buddy->screenname()), buddy);
}

void OscarAccount::removeBuddy(AIMBuddy *buddy)
{
	d->buddyNameMap.remove(tocNormalize(buddy->screenname()));
	QMap<int, AIMGroup * >::Iterator group = d->groupMap.find(buddy->groupID());
	if (group == d->groupMap.end())
		return;
	(*group)->removeBuddy(buddy);
}

AIMBuddy *OscarAccount::findBuddy(const QString &name)
{
	QMap<QString, AIMBuddy * >::Iterator it = d->buddyNameMap.find(tocNormalize(name));
	if (it != d->buddyNameMap.end() && (*it))
		return (*it);
	return 0L;
}

AIMGroup *OscarAccount::addGroup( int id, const QString &name, OscarContactType type )
{
	AIMGroup *group = new AIMGroup( id );
	if ( type == ServerSideContacts )
		group->setServerSide( true );

	if (!name.isNull())
	{
		group->setName(name);
		d->groupNameMap.insert(name, group);
	}
	d->groupMap.insert(group->ID(), group);

	// Process the queue
	// The AOL server can and often does (on certain accounts) send
	// us contacts with a GID in which the group matching to that GID hasn't been
	// sent down from the server yet. This gets around that by having OscarProtocol
	// queue up contacts that have a GID which hasn't been created yet, then when
	// that group is greated with the corresponding GID to the contacts which
	// have been queued up, we add those contacts.
	kdDebug( 14150 ) << k_funcinfo << "d->groupQueue.count() = " << d->groupQueue.count() << endl;
	int i = 0;
	for ( AIMBuddy *buddy = d->groupQueue.at( i ); buddy; buddy = d->groupQueue.at( ++i ) )
	{
		// Success, group now exists, add contact
		if ( findGroup( buddy->groupID() ) )
		{
			d->groupQueue.remove( i );
			addOldContact( buddy );
		}
	}

	return group;
}

AIMGroup *OscarAccount::findGroup( int id, OscarContactType type )
{
	QMap<int, AIMGroup * >::Iterator it = d->groupMap.find(id);
	if ( it != d->groupMap.end() && ( *it ) && ( type == AllContacts || it.data()->isServerSide() ) )
		return (*it);
	return 0L;
}

AIMGroup *OscarAccount::findGroup(const QString &name)
{
	QMap<QString, AIMGroup * >::Iterator it = d->groupNameMap.find(name);
	if (it != d->groupNameMap.end() && (*it))
		return (*it);
	return 0L;
}
// -- END MERGED CODE FROM AIMBUDDYLIST ------------------------------

#include "oscaraccount.moc"

// vim: set noet ts=4 sts=4 sw=4:

