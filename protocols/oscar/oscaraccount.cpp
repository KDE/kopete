/*
  oscaraccount.cpp  -  Oscar Account Class

  Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
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
#include <qwidget.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>

#include "kopetecontact.h"
#include "kopetegroup.h"
#include "kopeteaway.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetestdaction.h"

#include "aim.h" // For tocNormalize()
#include "aimbuddylist.h"
#include "oscarsocket.h"
#include "oscarchangestatus.h"
#include "oscarcontact.h"
#include "oscaruserinfo.h"
#include "oscardebugdialog.h"

#include <klineeditdlg.h>

OscarAccount::OscarAccount(KopeteProtocol *parent, QString accountID, const char *name)
	: KopeteAccount(parent, accountID, name)
{
	mEngine = 0L;

	initActions(); // Initialize our actions
	initActionMenu(); // Create our action menu
	initEngine(); // Initialize the backend

	// Create the internal buddy list for this account
	mInternalBuddyList=new AIMBuddyList();

	// Init the myself contact
	mMyself=new OscarContact(accountId(), accountId(), this, 0L);

	// Instantiate the away dialog
	mAwayDialog=new OscarChangeStatus(getEngine());

	// The debug dialog
	mDebugDialog=new OscarDebugDialog();
	getEngine()->setDebugDialog(mDebugDialog);

	initSignals(); // Initialize the signals and slots

	// Set our random new numbers
	mRandomNewBuddyNum = 0;
	mRandomNewGroupNum = 0;
}

OscarAccount::~OscarAccount()
{
	kdDebug(14150) << k_funcinfo << "[" << accountId() << "] deleted, Disconnecting..." << endl;

	disconnect();

	// Delete the backend
	if (mEngine)
	{
		delete mEngine;
		mEngine = 0L;
	}

	delete mAwayDialog;
	delete mDebugDialog;
	delete mMyself;
}

void OscarAccount::connect()
{
	kdDebug(14150) << "[OscarAccount: " << accountId() << "] connect()" << endl;

	// Get the screen name for this account
	QString screenName = accountId();

	if ( screenName != i18n("(No ScreenName Set)") )
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
			kdDebug(14150) << "[OscarAccount: " << accountId() << "] Logging in as " << screenName << endl;

			// Get the server and port from the preferences
			QString server = pluginData(protocol(), "Server");
			QString port = pluginData(protocol(), "Port");

			// Connect, need to normalize the name first
			mEngine->doLogin( server, port.toInt(), tocNormalize(screenName), password );

			if(isICQ())
				myself()->setOnlineStatus(OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::ICQCONN));
			else
				myself()->setOnlineStatus(OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::AIMCONN));
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

	if (isICQ())
	{
		myself()->setOnlineStatus(
			OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::ICQOFFLINE));
	}
	else
	{
		myself()->setOnlineStatus(
			OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::AIMOFFLINE));
	}
}

void OscarAccount::setAway(bool away, const QString &awayReason)
{
	kdDebug(14150) << "[OscarAccount: " << accountId() << "] setAway()" << endl;

	if(away)
		mEngine->sendAway(true, awayReason);
	else
		mEngine->sendAway(false, QString::null);
}

KopeteContact* OscarAccount::myself() const
{
	return mMyself;
}

OscarSocket* OscarAccount::getEngine()
{
	// Return the oscar socket object we're using
	return mEngine;
}


bool OscarAccount::isICQ()
{
 	// FIXME: make this a private bool and determine account type on creation/editing
	bool isicq=(((accountId()[0]).isNumber()) && (accountId().length()>4));
//	kdDebug(14150) << k_funcinfo << "Returning " << isicq << endl;
	return isicq;
}

void OscarAccount::initActions()
{
	if(isICQ())
	{
		kdDebug(14150) << k_funcinfo << "for ICQ account '" << accountId() << "'" << endl;

		mActionGoOnline	= new KAction(i18n("Online"), ICON_ICQ_ON, 0, this, SLOT(slotGoOnline()), this, "mActionGoOnline");
		mActionGoOffline	= new KAction(i18n("Offline"), ICON_ICQ_OFF, 0, this, SLOT(slotGoOffline()), this, "mActionGoOffline");
		mActionGoAway		= new KAction(i18n("Away"), ICON_ICQ_AW, 0, this, SLOT(slotGoAway()), this, "mActionGoAway");
		// Now following: Enhanced ICQ states
		mActionGoNA			= new KAction(i18n("Not Available"), ICON_ICQ_NA, 0, this, SLOT(slotGoNA()), this, "mActionGoNA");
		mActionGoDND		= new KAction(i18n("Do Not Disturb"), ICON_ICQ_DND, 0, this, SLOT(slotGoDND()), this, "mActionGoDND");
		mActionGoOccupied	= new KAction(i18n("Occupied"), ICON_ICQ_OCC, 0, this, SLOT(slotGoOCC()), this, "mActionGoOccupied");
		mActionGoFFC		= new KAction(i18n("Free For Chat"), ICON_ICQ_FFC, 0, this, SLOT(slotGoFFC()), this, "mActionGoOccupied");
		mActionEditInfo = 0L; // TODO: can't send/retrieve info yet so no menuitem
	}
	else
	{
		kdDebug(14150) << k_funcinfo << "for AIM account '" << accountId() << "'" << endl;

		mActionGoOnline	= new KAction(i18n("Online"), ICON_AIM_ON, 0, this, SLOT(slotGoOnline()), this, "mActionGoOnline");
		mActionGoOffline	= new KAction(i18n("Offline"), ICON_AIM_OFF, 0, this, SLOT(slotGoOffline()), this, "mActionGoOffline");
 		mActionGoAway		= new KAction(i18n("Away"), ICON_AIM_AW, 0, this, SLOT(slotGoAway()), this, "mActionGoAway");
		mActionGoNA			= 0L;
		mActionGoDND		= 0L;
		mActionGoOccupied	= 0L;
		mActionGoFFC		= 0L;
		mActionEditInfo	= KopeteStdAction::contactInfo(this, SLOT(slotEditInfo()), this, "mActionEditInfo");
	}

	mActionShowDebug = new KAction( i18n("Show Debug"), "wizard", 0,
		this, SLOT(slotShowDebugDialog()),
		this, "actionShowDebug");

	mActionFastAddContact = new KAction(i18n("Fast add a Contact"), "", 0, this, SLOT(slotFastAddContact()), this, "actionFastAddContact" );
}

void OscarAccount::initActionMenu()
{
	QString icon;

	if(isICQ())
		icon="icq_online";
	else
		icon="oscar_online";

	mActionMenu=new KActionMenu(accountId(), icon, this, "OscarAccount::mActionMenu");
	mActionMenu->insert(mActionGoOnline); // always first

	if(isICQ())
	{
		mActionMenu->insert(mActionGoFFC);
		mActionMenu->insert(mActionGoAway);
		mActionMenu->insert(mActionGoNA);
		mActionMenu->insert(mActionGoDND);
		mActionMenu->insert(mActionGoOccupied);
	}
	else
	{
		mActionMenu->insert(mActionGoAway);
		mActionMenu->popupMenu()->insertSeparator();
		mActionMenu->insert(mActionEditInfo); // TODO: add feature to ICQ as well
	}

	mActionMenu->insert(mActionGoOffline); // always last
	mActionMenu->popupMenu()->insertSeparator();
	mActionMenu->insert(mActionFastAddContact);
	mActionMenu->insert(mActionShowDebug);
}

void OscarAccount::initEngine()
{
	kdDebug(14150) << k_funcinfo << "START; accountId="<< accountId() << endl;

	QByteArray cook;
	cook.duplicate("01234567",8); // what is that for? [mETz]
	mEngine = new OscarSocket(pluginData(protocol(), "server"), cook, this);

	kdDebug(14150) << k_funcinfo << "END; accountId=" << accountId() << endl;
}

void OscarAccount::initSignals()
{
	// Contact list signals for group management events
	QObject::connect(
		KopeteContactList::contactList(), SIGNAL( groupRenamed( KopeteGroup *, const QString & ) ),
		this, SLOT( slotKopeteGroupRenamed( KopeteGroup *, const QString & )));

	QObject::connect(
		KopeteContactList::contactList(), SIGNAL( groupRemoved( KopeteGroup * ) ),
		this, SLOT( slotKopeteGroupRemoved( KopeteGroup * ) ) );

	// This is for when the user decides to add a group in the contact list
	QObject::connect(
		KopeteContactList::contactList(), SIGNAL( groupAdded(KopeteGroup *) ),
		this, SLOT( slotGroupAdded(KopeteGroup *) ) );

	// Protocol error
	QObject::connect(
		getEngine(), SIGNAL(protocolError(QString, int)),
		this, SLOT(slotError(QString, int)));

	// Got IM
	QObject::connect( getEngine(),
		SIGNAL(gotIM(QString,QString,bool)),
		this, SLOT(slotGotIM(QString,QString,bool)));

	// Got Config (Buddy List)
	QObject::connect( getEngine(),
		SIGNAL(gotConfig(AIMBuddyList &)),
		this, SLOT(slotGotServerBuddyList(AIMBuddyList &)));

	// Got my user info
	QObject::connect( getEngine(),
		SIGNAL(gotMyUserInfo(UserInfo)),
		this, SLOT(slotGotMyUserInfo(UserInfo)));

	// Status changed (I think my own status)
	QObject::connect( getEngine(),
		SIGNAL( statusChanged( const KopeteOnlineStatus & ) ),
		SLOT( slotOurStatusChanged( const KopeteOnlineStatus & ) ) );

	// Got warning
	QObject::connect( getEngine(),
		SIGNAL(gotWarning(int,QString)),
		this, SLOT(slotGotWarning(int,QString)));

	// Got direct IM request
	QObject::connect( getEngine(),
		SIGNAL(gotDirectIMRequest(QString)),
		this, SLOT(slotGotDirectIMRequest(QString)));

	// We have officially become idle
	QObject::connect(&mIdleMgr,
		SIGNAL(timeout()),
		this, SLOT(slotIdleTimeout()));

	// We have officially become un-idle
	QObject::connect(
		&mIdleMgr, SIGNAL(activity()),
		this, SLOT(slotIdleActivity()));
}

void OscarAccount::setUserProfile( QString profile )
{
	// Tell the engine to set the profile
	getEngine()->setMyProfile( profile );
	setPluginData(protocol(), "Profile", profile); // Save the user profile
}

KActionMenu* OscarAccount::actionMenu()
{
	return mActionMenu;
}

void OscarAccount::slotFastAddContact()
{
	QString newUIN = KLineEditDlg::getText("Add ICQ/AIM Contact", "Add Contact");
	if (!newUIN.isEmpty())
		addContact( newUIN, QString::null, 0L, QString::null, true); //add a temporary contact
}

void OscarAccount::slotGoOnline()
{
//	kdDebug(14150) << k_funcinfo << "Called, AWAY= '"
//		<< OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::AWAY).description() << "'" << endl;

//	if (myself()->onlineStatus() == OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::AWAY))
	if(myself()->onlineStatus().status() == KopeteOnlineStatus::Away)
	{ // If we're away , set us available
		kdDebug(14150) << "[OscarAccount: " << accountId() << "] slotGoOnline: was away, marking back" << endl;
		setAway(false);
	}
//	else if(myself()->onlineStatus() == OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::AIMOFFLINE))
	else if(myself()->onlineStatus().status() == KopeteOnlineStatus::Offline)
	{ // If we're offline, connect
		kdDebug(14150) << "[OscarAccount: " << accountId() << "] slotGoOnline: was offline, now connecting" << endl;
		connect();
	}
	else
	{ // We're already online
		kdDebug(14150) << "[OscarAccount: " << accountId() << "] slotGoOnline: already online" << endl;
	}
}

void OscarAccount::slotGoOffline()
{
	// This will ask the server to log us off
	// and then when that is complete, engine
	// will notify us of a status change
	getEngine()->doLogoff();
}

void OscarAccount::slotGoAway()
{
	kdDebug(14150) << k_funcinfo << "Called" << endl;
	if(isICQ())
	{
		kdDebug(14150) << k_funcinfo << "TODO: Make OSCARS away dialog work with icq as well!!!" << endl;
		if(
			(myself()->onlineStatus().status() == KopeteOnlineStatus::Online) ||
			(myself()->onlineStatus().status() == KopeteOnlineStatus::Away) // Away could also be a different AWAY mode (like NA or OCC)
			)
		{
			mAwayDialog->show();
//			mEngine->sendStatus(ICQ_STATUS_AWAY);
		}
	}
	else
	{
//		if(myself()->onlineStatus() == OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::AIMONLINE))
		if(myself()->onlineStatus().status() == KopeteOnlineStatus::Online)
		{
			// Show the dialog, which takes care of the
			// setting of the away message
			mAwayDialog->show();
		}
	}
}


void OscarAccount::slotGoNA()
{
	kdDebug(14150) << k_funcinfo << "Called" << endl;
	// Away could also be a different AWAY mode (like NA or OCC)
	if(
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Online) ||
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Away)
		)
		mEngine->sendStatus(ICQ_STATUS_NA);
}

void OscarAccount::slotGoOCC()
{
	kdDebug(14150) << k_funcinfo << "Called" << endl;
	// Away could also be a different AWAY mode (like NA or OCC)
	if(
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Online) ||
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Away)
		)
		mEngine->sendStatus(ICQ_STATUS_OCC);
}

void OscarAccount::slotGoFFC()
{
	kdDebug(14150) << k_funcinfo << "Called" << endl;
	// Away could also be a different AWAY mode (like NA or OCC)
	if(
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Online) ||
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Away)
		)
		mEngine->sendStatus(ICQ_STATUS_FFC);
}

void OscarAccount::slotGoDND()
{
	kdDebug(14150) << k_funcinfo << "Called" << endl;
	// Away could also be a different AWAY mode (like NA or OCC)
	if(
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Online) ||
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Away)
		)
		mEngine->sendStatus(ICQ_STATUS_DND);
}

void OscarAccount::slotEditInfo()
{
	OscarUserInfo *oscaruserinfo;

	oscaruserinfo = new OscarUserInfo(
		accountId(), accountId(),
		this, getEngine()->getMyProfile());

	oscaruserinfo->exec(); // This is a modal dialog
}

void OscarAccount::slotShowDebugDialog()
{
	if(mDebugDialog)
		mDebugDialog->show();
}

void OscarAccount::slotError(QString errmsg, int errorCode)
{
	kdDebug(14150) << "[OscarAccount: " << accountId()
		<< "] slotError(), errmsg="
		<< errmsg << ", errorCode=" << errorCode << "." << endl;

	//TODO: somebody add a comment about these two error-types
	if (errorCode == 1 || errorCode == 5)
		slotDisconnected();

	KMessageBox::error(qApp->mainWidget(), errmsg);
}


// Called when we get disconnected
void OscarAccount::slotDisconnected()
{
	kdDebug(14150) << "[OscarAccount: " << accountId()
		<< "] slotDisconnected() and function info is: "
		<< k_funcinfo << endl;

	if(isICQ())
		myself()->setOnlineStatus(OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::ICQOFFLINE));
	else
		myself()->setOnlineStatus(OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::AIMOFFLINE));
}

// Called when a group is added by adding a contact
void OscarAccount::slotGroupAdded(KopeteGroup *group)
{
	kdDebug(14150) << k_funcinfo << "called" << endl;

	QString groupName = group->displayName();

	// See if we already have this group
	AIMGroup *aGroup = mInternalBuddyList->findGroup(groupName);
	if (!aGroup)
	{
		aGroup = mInternalBuddyList->addGroup(mRandomNewGroupNum, groupName);
		mRandomNewGroupNum++;
		kdDebug(14150) << "[OscarAccount: " << accountId() << "] addGroup() being called" << endl;
		if (isConnected())
		{
			getEngine()->sendAddGroup(groupName);
		}
	}
	else
	{ // The server already has it in it's list, don't worry about it
		kdDebug(14150) << "Group already existing" << endl;
	}
}

void OscarAccount::slotKopeteGroupRenamed( KopeteGroup *group,
										   const QString &oldName )
{
	kdDebug(14150) << k_funcinfo << "Sending group name change request" << endl;
	getEngine()->sendChangeGroupName(oldName, group->displayName());

}

void OscarAccount::slotKopeteGroupRemoved( KopeteGroup *group )
{
	// This method should be called after the contacts have been removed
	// We should then be able to remove the group from the server
	kdDebug(14150) << k_funcinfo
				   << "Telling the server to delete group "
				   << group->displayName() << endl;

	getEngine()->sendDelGroup(group->displayName());
}

// Called when we have gotten an IM
void OscarAccount::slotGotIM(QString /*message*/, QString sender, bool /*isAuto*/)
{
	kdDebug(14150) << k_funcinfo << "account='"<< accountId() <<
		"', sender='" << sender << "'" << endl;

	//basically, all this does is add the person to your buddy list
	// TODO: Right now this has no support for "temporary" buddies
	// because I could not think of a good way to do this with
	// AIM.

	if (!mInternalBuddyList->findBuddy(sender))
	{
		addContact(tocNormalize(sender), sender, 0L, QString::null, true); // last one should be true
	}
}

// Called when we retrieve the buddy list from the server
void OscarAccount::slotGotServerBuddyList(AIMBuddyList &buddyList)
{
	kdDebug(14150) << k_funcinfo << "account='" << accountId() << "'" << endl;

	//save server side contact list
	*mInternalBuddyList += buddyList;
	QValueList<AIMBuddy *> localList = buddyList.buddies().values();

	for (QValueList<AIMBuddy *>::Iterator it=localList.begin(); it!=localList.end(); ++it)
	{
		if ((*it))
			addServerContact((*it)); // Add the server contact to Kopete list
	}
}

void OscarAccount::addServerContact(AIMBuddy *buddy)
{
//	kdDebug(14150) << k_funcinfo << "Called for '" << buddy->screenname() << "'" << endl;

	// This gets the contact in the kopete contact list for our account
	// that has this name, need to normalize once again
	OscarContact *contact = static_cast<OscarContact*>(contacts()[tocNormalize(buddy->screenname())]);

	QString nick;
	if(!buddy->alias().isEmpty())
		nick=buddy->alias();
	else
		nick=buddy->screenname();

	if (contact)
	{
		// Contact existed in the list already, sync information
		// Set the status
		contact->setOnlineStatus( buddy->status() );
		if(contact->displayName()!=nick)
		{
//			kdDebug(14150) << k_funcinfo << "Contact has different nickname on server, renaming local contact" << endl;
			contact->rename(nick);
		}
		// TODO: write syncGroups in OscarContact
		contact->syncGroups();
	}
	else
	{
		kdDebug(14150) << k_funcinfo << "Adding new contact from Serverside List to Kopete" << endl;
		// Contact is new on server and not in the Kopete list yet
		// Create a new metacontact for it
		//KopeteMetaContact *metaContact = new KopeteMetaContact();

		// Create a new OscarContact
//	 	contact = new OscarContact( tocNormalize( buddy->screenname()), buddy->alias(), this, metaContact);

		// Set the contact's status
//		contact->setOnlineStatus( buddy->status() );

		// Get the group this buddy belongs to
		AIMGroup *aimGroup = mInternalBuddyList->findGroup(buddy->groupID());

		if (aimGroup)
		{
			// If the group exists in the internal list
			// Add it to the kopete contact list, with no metacontact
			// which creates a new one. This will also call the
			// addContactToMetaContact method in this class
			addContact(tocNormalize(buddy->screenname()), nick, 0L, aimGroup->name(), false);

			// Get the Kopete group
// 			KopeteGroup *group = KopeteContactList::contactList()->getGroup(aimGroup->name());

			// tell the metacontact that it's in that group
//			metaContact->addToGroup( group );
			// Tell the KopeteContactList about the contact
// 			KopeteContactList::contactList()->addMetaContact(metaContact);
		}
		else
		{
			// If the group doesn't exist on the server yet.
			// This is really strange if we have the contact
			// on the server but not the group it's in.
			// May have to do something here in the future
		}
	}
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
	* The third situation is when somebody new messages you
	*/

	/* We're not even online or connecting (when getting server contacts), so don't bother */
	if(
		(!myself()->isOnline()) &&
		(myself()->onlineStatus().status() != KopeteOnlineStatus::Connecting)
		)
	{
		kdDebug(14150) << k_funcinfo << "Can't add contact, we are offline!" << endl;
		return false;
	}

	// Next check our internal list to see if we have this buddy
	// already, findBuddy tocNormalizes the buddy name for us
	AIMBuddy *internalBuddy = mInternalBuddyList->findBuddy(contactId);

	if (internalBuddy) // We found the buddy internally
	{
		// Create an OscarContact for the metacontact
		OscarContact *newContact = new OscarContact(contactId, displayName, this, parentContact);
		// Set the oscar contact's status
		newContact->setOnlineStatus(internalBuddy->status());
		return (newContact != 0L);
	}
	else // It was not on our internal list yet
	{
		kdDebug(14150) << k_funcinfo << "New Contact '" << contactId
			<< "' wasn't in internal list. Creating new internal list entry" << endl;

		// Check to see if it's a temporary contact, ie. not on our list
		// but IMed us anyway
		if(!parentContact->isTemporary())
		{
			kdDebug(14150) << "real new contact, also going to add him to the serverside contactlist" << endl;
			QString groupName;
			KopeteGroupList kopeteGroups = parentContact->groups(); // Get a list of the groups it's in

			if(kopeteGroups.isEmpty())
			{
				kdDebug(14150) << k_funcinfo << "Contact with no group, adding to group 'Buddies'" << endl;
				groupName="Buddies"; // happens for temporary contacts
			}
			else
			{
				kdDebug(14150) << k_funcinfo << "Contact with group, grouplist count=" << kopeteGroups.count() << endl;
				// OSCAR doesn't support a contact in multiple groups, so we
				// just take the first one
				KopeteGroup *group = kopeteGroups.first();
				// Get the name of the group
				groupName = group->displayName();
				kdDebug(14150) << k_funcinfo << "groupName=" << groupName << "'" << endl;
			}

			if(groupName.isEmpty()) // emergency exit, should never occur
			{
				kdDebug(14150) << "could not add Contact because no groupname was given" << endl;
				return false;
			}

			// See if it exists in our internal group list already
			AIMGroup *internalGroup = mInternalBuddyList->findGroup(groupName);
			if (!internalGroup) // If the group didn't exist
			{
				internalGroup = mInternalBuddyList->addGroup(mRandomNewGroupNum, groupName);
				kdDebug(14150) << "created internal group for new contact" << endl;
				// Add the group on the server list
				getEngine()->sendAddGroup(internalGroup->name());
			}

			// Create a new internal buddy for this contact
			AIMBuddy *newBuddy = new AIMBuddy(mRandomNewBuddyNum, internalGroup->ID(), contactId);

			// Check if it has an alias
			if((displayName != QString::null) && (displayName != contactId))
				newBuddy->setAlias(displayName);

			// Add the buddy to the internal buddy list
			mInternalBuddyList->addBuddy( newBuddy );

			// Add the buddy to the server's list, with the group,
			// need to normalize the contact name
			getEngine()->sendAddBuddy(tocNormalize(contactId), internalGroup->name());

			// Increase these counters, I'm not sure what this does
			mRandomNewGroupNum++;
			mRandomNewBuddyNum++;

			// Create the actual OscarContact, which adds it to the metacontact
			/*OscarContact *newContact = */new OscarContact(contactId, displayName, this, parentContact);

			return true;
		}
		else
		{
			kdDebug(14150) << "Temporary new contact, only adding him to local list" << endl;
			// This is a temporary contact, so don't add it to the server list
			// Create the contact, which adds it to the parent contact
			OscarContact *newContact = new OscarContact(contactId, displayName, this, parentContact);

			// Add it to the kopete contact list, I think this is done in
			// KopeteAccount::addContact
			//KopeteContactList::contactList()->addMetaContact(parentContact);

			// Set it's initial status
			// This requests the buddy's info from the server
			// I'm not sure what it does if they're offline, but there
			// is code in oscarcontact to handle the signal from
			// the engine that this causes
			kdDebug(14150) << "[OscarAccount: " << accountId() << "] Requesting user info for " << contactId << endl;
			getEngine()->sendUserProfileRequest(tocNormalize(contactId));
			return (newContact != 0L);
		}
	} // END not internalBuddy

}

void OscarAccount::slotOurStatusChanged(const KopeteOnlineStatus &newStatus)
{
	kdDebug(14150) << k_funcinfo << "status=" << newStatus.description() <<
		"(" << newStatus.internalStatus() << ")" << endl;
	myself()->setOnlineStatus(newStatus); // update record of our status
}

// Called when we have been warned
void OscarAccount::slotGotWarning(int newlevel, QString warner)
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;

	//this is not a natural decrease in level
	if (mUserInfo.evil < newlevel)
	{
		QString warnMessage;
		if(warner.isNull())
			warnMessage = i18n("anonymously");
		else
			warnMessage = i18n("...warned by...", "by %1").arg(warner);
		QString message =
			i18n("You have been warned %1. Your new warning level is %2%.").arg(warnMessage).arg(newlevel);
		KMessageBox::sorry(0L,message);
	}
	mUserInfo.evil = newlevel;
}

void OscarAccount::slotGotDirectIMRequest(QString sn)
{
	QString title = i18n("Direct IM session request");
	QString message =
	i18n("%1 has requested a direct IM session with you. " \
		"Direct IM sessions allow the remote user to see your IP " \
		"address, which can lead to security problems if you don't " \
		"trust him/her. Do you want to establish a direct connection " \
		"to %2?").arg(sn).arg(sn);

	int result = KMessageBox::questionYesNo(qApp->mainWidget(), message, title);

	if (result == KMessageBox::Yes)
		getEngine()->sendDirectIMAccept(sn);
	else if (result == KMessageBox::No)
		getEngine()->sendDirectIMDeny(sn);
}

void OscarAccount::slotGotMyUserInfo(UserInfo newInfo)
{
	mUserInfo = newInfo;
}

void OscarAccount::slotIdleActivity()
{
//	kdDebug(14150) << k_funcinfo << "system is ACTIVE, setting idle time with server to 0" << endl;
	getEngine()->sendIdleTime(0);
}

// Called when there is no activity for a certain amount of time
void OscarAccount::slotIdleTimeout()
{
//	kdDebug(14150) << k_funcinfo << "system is IDLE, setting idle time with server" << endl;
	// idleTimeout() gives a value in minutes, engine wants seconds
	int idleTimeout = 0;
	idleTimeout = (pluginData(protocol(), "IdleTimeOut")).toInt();

	getEngine()->sendIdleTime(idleTimeout*60);
}

int OscarAccount::randomNewBuddyNum()
{
	return mRandomNewBuddyNum++;
}

int OscarAccount::randomNewGroupNum()
{
	return mRandomNewGroupNum++;
}

AIMBuddyList *OscarAccount::internalBuddyList()
{
	return mInternalBuddyList;
}

void OscarAccount::setServer( QString server )
{
	setPluginData(protocol(), "Server", server);
}

void OscarAccount::setPort( int port )
{
	if (port>0)// Do a little error checkin on it
		setPluginData(protocol(), "Port", QString::number( port ));
}

#include "oscaraccount.moc"
// vim: set noet ts=4 sts=4 sw=4:
