/***************************************************************************
                          jabberprotocol.cpp  -  Jabber Plugin
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Daniel Stone
    email                : dstone@kde.org
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *                                                                         *
 *   Kudos to my lappy for letting me hack on the way to Sunbury, and CBV  *
 *   for lending it to me. :)                                              *
 *                                                                         *
 *   All this code has been written on a 120-odd character terminal, and   *
 *   thus the wrapping is changed accordingly; in fact, there is none.     *
 *                                                                         *
 *   Deal.                                                                 *
 *                                                                         *
 ***************************************************************************/

#include <qcursor.h>
#include <qmap.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kgenericfactory.h>
#include <kstatusbar.h>
#include <kaction.h>
#include <kpopupmenu.h>

#include "kopete.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetewindow.h"
#include "systemtray.h"

#include "jabberprotocol.h"
#include "jabcommon.h"
#include "statusbaricon.h"
#include "dlgjabberstatus.h"
#include "dlgjabbersendraw.h"
#include "jabio.h"
#include "jabtasks.h"

const JabberProtocol * JabberProtocol::sProtocol = 0;

K_EXPORT_COMPONENT_FACTORY(kopete_jabber, KGenericFactory<JabberProtocol>);

/*
 * JabberProtocol constructor
 */
JabberProtocol::JabberProtocol(QObject *parent, QString name, QStringList) : KopeteProtocol(parent, name)
{
	kdDebug() << "[JabberProtocol] Loading ..." << endl;

	// this is mean to be a singleton, so we will check if we have been loaded before
	if (sProtocol)
		kdDebug() << "[JabberProtocol] Warning: sProtocol already exists! Not redefining." << endl;
	else
		sProtocol = this;

	// these dialogs will be re-instantiated when needed and deleted after
	// they have served their purpose
	reasonDialog = 0L;
	sendRawDialog = 0L;
	
	myContact = 0L;

	mPrefs = new JabberPreferences("jabber_protocol_32", this);
	connect (mPrefs, SIGNAL(saved()), this, SLOT(slotSettingsChanged()));

	// read the Jabber ID from Kopete's configuration
	KGlobal::config()->setGroup("Jabber");
	if ((KGlobal::config()->readEntry("UserID", "") == "") || (KGlobal::config()->readEntry("Password", "") == ""))
		KMessageBox::error(kopeteapp->mainWindow(), i18n("<qt>If you have a Jabber account, please configure it in the Kopete Settings dialog. If you don't, you can register from there as well.</qt>"),	i18n("No Jabber Configuration Found!"));

	// set initial state to disconnected, don't create the current contact
	mIsConnected = false;
	doRegister = false;
	

	// setup icons and actions
	initIcons();
	initActions();

	// read remaining settings from configuration file
	slotSettingsChanged();
	
	// initialize icon that sits in Kopete's status bar
	statusBarIcon = new StatusBarIcon();
	QObject::connect(statusBarIcon, SIGNAL(rightClicked(const QPoint&)), this, SLOT(slotIconRightClicked(const QPoint&)));
	statusBarIcon->setPixmap(offlineIcon);

	// if we need to connect on startup, do it now
	if (KGlobal::config()->readBoolEntry("AutoConnect", "0"))
		Connect();
}

/*
 * JabberProtocol destructor
 */
JabberProtocol::~JabberProtocol()
{

	// we should at least disconnect here
	Disconnect();
	
	for(JabberContactList::iterator it = contactList.begin(); it != contactList.end(); it++)
	{
		//delete it.data();
		it.data() = 0;
	}
	
	// clear the contact list
	contactList.clear();
	metaContactMap.clear();

}

void JabberProtocol::init()
{

	// nothing to be done here so far
	
}

bool JabberProtocol::unload()
{
	
	kdDebug() << "[JabberProtocol] Unload..." << endl;
	
	// remove the statusbar indicator
	if (kopeteapp->statusBar())
	{
		kopeteapp->statusBar()->removeWidget(statusBarIcon);
		delete statusBarIcon;
	}

	emit protocolUnloading();
	return true;

}

/*
 * connect to the Jabber server
 */
void JabberProtocol::Connect()
{
	
	kdDebug() << "[JabberProtocol] Connect()" << endl;
	
	if (!isConnected())
	{
		// if the status is set to offline, set it to online,
		// otherwise leave it as previously indicated
		if (!mStatus)
			mStatus = STATUS_ONLINE;

		// instantiate new Psi backend class for handling the protocol and setup slots and signals
		mProtocol = new Jabber;
		connect(mProtocol, SIGNAL(connected()), this, SLOT(slotConnected()));
		connect(mProtocol, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
		connect(mProtocol, SIGNAL(contactNew(JabRosterEntry *)), this, SLOT(slotNewContact(JabRosterEntry *)));
		connect(mProtocol, SIGNAL(contactChanged(JabRosterEntry *)), this, SLOT(slotContactUpdated(JabRosterEntry *)));
		connect(mProtocol, SIGNAL(resourceAvailable(const Jid &, const JabResource &)), this, SLOT(slotResourceAvailable(const Jid &, const JabResource &)));
		connect(mProtocol, SIGNAL(resourceUnavailable(const Jid &)), this, SLOT(slotResourceUnavailable(const Jid &)));
		connect(mProtocol, SIGNAL(authRequest(const Jid &)), this, SLOT(slotUserWantsAuth(const Jid &)));
		connect(mProtocol, SIGNAL(authRemove(const Jid &)), this, SLOT(slotUserDeletedAuth(const Jid &)));
		connect(mProtocol, SIGNAL(messageReceived(const JabMessage &)), this, SLOT(slotNewMessage(const JabMessage &)));
		connect(mProtocol, SIGNAL(error(JabError *)), this, SLOT(slotError(JabError *)));

		kdDebug() << "[JabberProtocol] Connecting to Jabber server " << mServer << ":" << mPort << endl;
		kdDebug() << "                 with UserID " << mUsername << endl;

		// pass account details on to protocol class
		KGlobal::config()->setGroup("Jabber");
		mProtocol->setHost(mServer, mPort);
		mProtocol->setAccount(mUsername, mPassword, mResource);
		mProtocol->setSSLEnabled(KGlobal::config()->readBoolEntry("UseSSL", "0"));

		// if we are to create a new account on the server, do it here
		if (doRegister)
		{
			kdDebug() << "[JabberProtocol] Registering user" << endl;
			mProtocol->accRegister();
		}
		else
		{
			kdDebug() << "[JabberProtocol] Already registered, performing normal login." << endl;
			mProtocol->login(mStatus, "", 1, true);
		}
		
		// call slot for connecting, this will update the status bar
		slotConnecting();
	}
	else
		if (isAway())
			// the user is connected but currently away, update status
			slotGoOnline();
		else
			// user is connected and not away, ignore
			kdDebug() << "[JabberProtocol] Ignoring connect request (already connected)." << endl;
			
}

/*
 * Returns current protocol instance
 */
const JabberProtocol *JabberProtocol::protocol()
{
	
	// return current instance
	return sProtocol;
	
}

/*
 * Disconnect from the server
 */
void JabberProtocol::Disconnect()
{
	
	if (isConnected())
	{
		// tell backend class to disconnect
		mProtocol->disc();
		delete mProtocol;
		
		kdDebug() << "[JabberProtocol] Disconnected." << endl;
		
		mIsConnected = false;
		statusBarIcon->setPixmap(offlineIcon);
	}
	else
	{
		// we are already disconnected, ignore
		kdDebug() << "[JabberProtocol] Ignoring disconnect request (not connected)." << endl;
	}

	mStatus = STATUS_OFFLINE;

}

/*
 * Slot to connect to the server
 */
void JabberProtocol::slotConnect()
{
	
	Connect();
	
}

/*
 * Slot to disconnect from the server
 */
void JabberProtocol::slotDisconnect()
{

	Disconnect();

}

/*
 * Slot announcing a connect in progress
 */
void JabberProtocol::slotConnecting()
{

	statusBarIcon->setMovie(connectingIcon);

}

/*
 * Slot called upon successful connection (called by Psi backend)
 */
void JabberProtocol::slotConnected()
{
	
	kdDebug() << "[JabberProtocol] Connected to Jabber server." << endl;
	
	mIsConnected = true;
	
	// determine status and update icon appropriately
	switch(mStatus)
	{
		case STATUS_AWAY:
		case STATUS_XA:
					statusBarIcon->setPixmap(awayIcon);
					break;
		case STATUS_DND:
					statusBarIcon->setPixmap(naIcon);
					break;
		default:
					statusBarIcon->setPixmap(onlineIcon);
					break;
	}

}

/*
 * Slot called upon successful disconnection (called by Psi backend)
 */
void JabberProtocol::slotDisconnected()
{
	
	kdDebug() << "[JabberProtocol] Disconnected from Jabber server." << endl;

	mIsConnected = false;
	statusBarIcon->setPixmap(offlineIcon);
	
	delete myContact;
	myContact = 0L;
	
}

/*
 * Slot called if there was a protocol error (called by Psi backend)
 */
void JabberProtocol::slotError(JabError *error)
{
	
	// determine type of error
	switch(error->type)
	{
		case JABERR_CONNECT:
			KMessageBox::error(kopeteapp->mainWindow(), i18n("There was an error connecting to the Jabber server (%1).").arg(error->msg, 1),
					   i18n("Error connecting to Jabber Server"));
			break;
		case JABERR_AUTH:
			KMessageBox::error(kopeteapp->mainWindow(), i18n("Login failed. Please check your Jabber ID and password."),
					   i18n("Error connecting to Jabber Server"));
			break;
		case JABERR_CREATE:
			KMessageBox::error(kopeteapp->mainWindow(), i18n("The Jabber account could not be created. Maybe it exists already?"),
					   i18n("Error creating Jabber Account"));
			break;
		default:
			KMessageBox::error(kopeteapp->mainWindow(), i18n("You were disconnected for an unspecified reason (%1).").arg(error->type, 1),
								    i18n("Disconnected From Jabber Server"));
			break;
	}
	
	/* basically all errors mean disconnection, so disconnect for real here */
	Disconnect();
	
}

/*
 * Return connection status
 */
bool JabberProtocol::isConnected() const
{

	return mIsConnected;

}

/*
 * Set status to "Away"
 */
void JabberProtocol::setAway(void)
{
    
	setPresence(STATUS_AWAY, "Automatically set away due to being idle.");

}

/*
 * Set status to "Online"/"Available"
 */
void JabberProtocol::setAvailable(void)
{
    
	slotGoOnline();

}

/*
 * Return a KopeteContact pointer of self
 */
KopeteContact* JabberProtocol::myself() const
{

	return myContact;

}

/*
 * Determine away status
 */
bool JabberProtocol::isAway(void) const
{
	
	if (isConnected())
		return (mProtocol->status() == STATUS_AWAY || mProtocol->status() == STATUS_XA || mProtocol->status() == STATUS_DND);
	else
		// we are not connected, so default to FALSE
		return false;

}

/*
 * Return the protocol icon
 */
QString JabberProtocol::protocolIcon() const
{
    
	return "jabber_protocol_32";

}

/*
 * Serialize contact data
 */
bool JabberProtocol::serialize(KopeteMetaContact *contact, QStringList &data) const
{

	kdDebug() << "[JabberProtocol] Serializing data for metacontact " << contact->displayName() << endl;

	JabberContact *c = metaContactMap.find(contact);
	
	if(c)
	{
		data << c->identityId() << c->userID() << c->displayName() << c->group();
		
		return true;
	}
	else
	{
		kdDebug() << "[JabberProtocol] Contact " << contact->displayName() << " not found in the map! Can't serialize." << endl;
		
		return false;
	}
	
}

/*
 * Deserialize contact data
 */
void JabberProtocol::deserialize(KopeteMetaContact *contact, const QStringList &data)
{

	kdDebug() << "[JabberProtocol] Deserializing data for metacontact " << contact->displayName() << endl;
	
	if(data[0] != myContact->userID())
	{
		// trying to deserialize a contact that does not belong to our roster, skip it.
		// this usually happens if the contact list was serialized from a certain account
		// and the user changed his account details (and thus receives a new roster)
		kdDebug() << "[JabberProtocol] Contact " << data[2] << " belongs to " << data[0] << ", however, we are " << myContact->userID() << ". Skipping." << endl;
		return;
	}
	
	JabberContact *c = new JabberContact(data[1], data[2], data[3], this, contact, data[0]);
	
	metaContactMap.insert(contact, c);
	contactList[c->userID()] = c;
	
	connect(c, SIGNAL(contactDestroyed(KopeteContact *)), this, SLOT(slotContactDestroyed(KopeteContact*)));
	
	contact->addContact(c, c->group());
	
}

/*
 * Cleans up when a contact is destroyed
 */
void JabberProtocol::slotContactDestroyed(KopeteContact *contact)
{

	metaContactMap.remove(contact->metaContact());

}
 
/*
 * Return a list of address book fields we are interested in monitoring
 */
/*
QStringList JabberProtocol::addressBookFields() const
{

}
*/

/*
 * Notification slot in case one of the monitored address book fields changed
 */
/*
void JabberProtocol::addressBookFieldChanged(KopeteMetaContact *contact, const QString &key)
{

}
*/

/*
 * Create and return "add contact" wizard
 */
AddContactPage *JabberProtocol::createAddContactWidget(QWidget *parent)
{

	return new JabberAddContactPage(this, parent);

}

/*
 * Load and create icons
 */
void JabberProtocol::initIcons()
{
	KIconLoader *loader = KGlobal::iconLoader();
	KStandardDirs dir;

	onlineIcon = QPixmap(loader->loadIcon("jabber_online", KIcon::User));
	offlineIcon = QPixmap(loader->loadIcon("jabber_offline", KIcon::User));
	awayIcon = QPixmap(loader->loadIcon("jabber_away", KIcon::User));
	naIcon = QPixmap(loader->loadIcon("jabber_na", KIcon::User));
	connectingIcon = QMovie(dir.findResource("data", "kopete/pics/jabber_connecting.mng"));

}

/*
 * Create actions
 */
void JabberProtocol::initActions()
{
	
	actionGoOnline = new KAction(i18n("Online"), "jabber_online", 0, this, SLOT(slotConnect()), this, "actionJabberConnect");
	actionGoAway = new KAction(i18n("Away"), "jabber_away", 0, this, SLOT(slotSetAway()), this, "actionJabberway");
	actionGoXA = new KAction(i18n("Extended Away"), "jabber_away", 0, this, SLOT(slotSetXA()), this, "actionJabberXA");
	actionGoDND = new KAction(i18n("Do Not Disturb"), "jabber_na", 0, this, SLOT(slotSetDND()), this, "actionJabberDND");
	actionGoOffline = new KAction(i18n("Offline"), "jabber_offline", 0, this, SLOT(slotDisconnect()), this, "actionJabberDisconnect");
	actionSendRaw = new KAction(i18n("Send raw packet..."), "filenew", 0, this, SLOT(slotSendRaw()), this, "actionJabberSendRaw");
	
	actionStatusMenu = new KActionMenu("Jabber", this);
	
	// will be overwritten in slotSettingsChanged, maybe there is a better way (gogo)
	m_menuTitleId = actionStatusMenu->popupMenu()->insertTitle(""); 	
	
	actionStatusMenu->insert(actionGoOnline);
	actionStatusMenu->insert(actionGoAway);
	actionStatusMenu->insert(actionGoXA);
	actionStatusMenu->insert(actionGoDND);
	actionStatusMenu->insert(actionGoOffline);
	
	actionStatusMenu->popupMenu()->insertSeparator();
	actionStatusMenu->insert(actionSendRaw);

	actionStatusMenu->plug(kopeteapp->systemTray()->contextMenu(), 1);
}

/*
 * Slot for going online
 */
void JabberProtocol::slotGoOnline()
{

	kdDebug() << "[JabberProtocol] Going online!" << endl;

	if (!isConnected())
	{
		// we are not connected yet, so connect now
		mStatus = STATUS_ONLINE;
		Connect();
	}
	else
		// we are connected already, update status
		setPresence(STATUS_ONLINE, "");

	statusBarIcon->setPixmap(onlineIcon);

}

/*
 * Slot for going offline
 */
void JabberProtocol::slotGoOffline()
{
    
	kdDebug() << "[JabberProtocol] Going offline." << endl;
    
	Disconnect();
	
	statusBarIcon->setPixmap(offlineIcon);

}

/*
 * Slot for going "away"
 */
void JabberProtocol::slotSetAway()
{
	
	kdDebug() << "[JabberProtocol] Setting away mode." << endl;
	
	// kill old reason dialog if it's still in memory
	if (reasonDialog != 0L)
		delete reasonDialog;
	
	reasonDialog = new dlgJabberStatus(this, STATUS_AWAY, kopeteapp->mainWindow());
	
}

/*
 * Slot for going "not available"
 */
void JabberProtocol::slotSetXA()
{

	kdDebug() << "Jabber plugin: Setting extended away mode." << endl;

	// kill old reason dialog if it's still in memory
	if (reasonDialog != 0L)
		delete reasonDialog;
	
	reasonDialog = new dlgJabberStatus(this, STATUS_XA, kopeteapp->mainWindow());

}

/*
 * Slot for going "do not disturb"
 */
void JabberProtocol::slotSetDND()
{
    
	kdDebug() << "Jabber plugin: Setting do not disturb mode." << endl;

	// kill old reason dialog if it's still in memory
	if (reasonDialog != 0L)
		delete reasonDialog;

	reasonDialog = new dlgJabberStatus(this, STATUS_DND, kopeteapp->mainWindow());

}

/*
 * Slot for sending a raw message to the server
 */
void JabberProtocol::slotSendRaw()
{

	// kill old dialog if it's still in memory
	if (sendRawDialog != 0L)
		delete sendRawDialog;

	sendRawDialog = new dlgJabberSendRaw(this, kopeteapp->mainWindow());

}

/*
 * Send a raw message to the server (usually called by the dialog widget)
 */
void JabberProtocol::sendRawMessage(const QString &packet)
{

	kdDebug() << "--- Sending raw message ---" << endl << packet << "---------------------------" << endl ;

	mProtocol->insertXml(packet);

}

void JabberProtocol::sendPresenceToNode(const int &status,const QString &toUser )
{
	QString status_str;

	switch(status)
	{
		case 0: status_str = "chat"; break;
		case 1: status_str = "away" ; break;
		case 2: status_str = "xa"; break;
		case 3: status_str = "dnd"; break;
	}
	
	mProtocol->insertXml( QString("<presence to=\"%1\" from=\"%2\">\n<priority>1</priority>\n<status></status>\n<show>%3</show>\n</presence>").arg(toUser).arg(mUsername+"@"+mServer).arg(status_str));
	
	}

/*
 * Set presence (usually called by dialog widget)
 */
void JabberProtocol::setPresence(int status, QString reason, int priority)
{
	
	if (mIsConnected)
	{
		mProtocol->setPresence(status, reason, priority);
		
		switch(status)
		{
			case STATUS_AWAY:
			case STATUS_XA:
						statusBarIcon->setPixmap(awayIcon);
						mProtocol->setPresence(status, reason, priority);
						break;
			case STATUS_DND:
						statusBarIcon->setPixmap(naIcon);
						mProtocol->setPresence(status, reason, priority);
						break;
			case STATUS_ONLINE:
						statusBarIcon->setPixmap(onlineIcon);
						mProtocol->setPresence(status, reason, priority);
						break;
			default:
						kdDebug() << "[JabberProtocol] Unknown presence status, ignoring (status == " << status << ")" << endl;
						break;
		}
	}

}

/*
 * Create popup menu for right clicks on status icon
 */
void JabberProtocol::slotIconRightClicked(const QPoint&)
{

	actionStatusMenu->popup(QCursor::pos());

}

/*
 * Remove the user with the given ID from the contact list
 */
void JabberProtocol::removeUser(QString userID)
{
	
	if (mIsConnected)
	{
		kdDebug() << "[JabberProtocol] Protocol removing user " << userID << endl;

		// revoke previously granted subscription to other end
		mProtocol->unsubscribed(userID);
		
		// now unsubscribe from this user's presence
		mProtocol->unsubscribe(userID);
		
		// remove account from local list
		//delete contactList[userID];
		contactList.remove(userID);
	}
	
}

/*
 * Rename a contact
 */
void JabberProtocol::renameContact(QString userID, QString name, QString group)
{
    
	if (mIsConnected)
	{
		kdDebug() << "[JabberProtocol] Protocol renaming user " << userID << " to " << name << endl;

		mProtocol->setRoster(userID, name, group);
		
		contactList[userID]->setDisplayName(name);
	}

}

/*
 * Move a contact between groups
 */
void JabberProtocol::moveUser(QString userID, QString group, QString name, JabberContact *contact)
{
	
	if (mIsConnected)
	{
		QString localGroup;
		
		if ( group.isEmpty() )
		{
			kdDebug() << "[JabberProtocol] Protocol moving user " << userID << " out of a group." << endl;
			localGroup = i18n("Unknown");
		}
		else
		{
			kdDebug() << "[JabberProtocol] Protocol moving user " << userID << " to " << group << endl;
			localGroup = group;
		}
		
		mProtocol->setRoster(userID, name, group);

	}

}

/*
 * Add a contact to the list
 */

void JabberProtocol::addContact(QString userID)
{
    
	if (mIsConnected)
	{
		kdDebug() << "[JabberProtocol] Protocol adding user " << userID << endl;
		
		// this will send a subscription request to the other user,
		// which will cause slotNewContact() to be called if subscription was successful
		mProtocol->subscribe(userID);
	}

}

/*
 * A user requests authentication (call from Psi backend)
 */
void JabberProtocol::slotUserWantsAuth(const Jid &jid)
{
	QString userID = QString("%1@%2").arg(jid.user(), 1).arg(jid.host(), 2);
	
	kdDebug() << "[JabberProtocol] " << userID << " wants auth!" << endl;
	
	if (KMessageBox::questionYesNo(kopeteapp->mainWindow(),
	    i18n("The Jabber user %1 wants to add you to their contact list. Do you want to authorize them?").arg(userID, 1), i18n("Authorize Jabber User?")) == 3)
	{
		mProtocol->subscribed(userID);
	
		// ask if we are to subscribe to this user's presence in return,
		// in case it is not in our contact list yet
		if (!contactList.contains(userID))
			if (KMessageBox::questionYesNo(kopeteapp->mainWindow(),
			    i18n("Do you want to add %1 to your contact list in return?").arg(userID, 1), i18n("Add Jabber User?")) == 3)
				mProtocol->subscribe(userID);
	}

}

/*
 * A user deleted you from his contact list (call from Psi backend) 
 */

void JabberProtocol::slotUserDeletedAuth(const Jid &jid)
{

	QString jidString = QString("%1@%2").arg(jid.user(), 1).arg(jid.host(), 2);
	
	kdDebug() << "[JabberProtocol] " << jidString << " deleted auth!" << endl;
	
	// set user offline, but how to mark we are unsubscrbed? move contact to 
	// a dir "Need Authorization" like in psi? ....
	// btw it's just a notification, so can also send a unsubscribe message
	// without to "unsubscribed user"! ,)
	// FIXME: this needs a checkbox to determine if we delete the contact forever
	contactList[jidString]->slotUpdateContact("", STATUS_OFFLINE, "");

	KMessageBox::information(0L, i18n("%1 unsubscribed you!").arg(jidString), i18n("Notification"));

}

/*
 * Update a contact's details
 */
void JabberProtocol::slotContactUpdated(JabRosterEntry *contact)
{
	
	// sanity check
	if(contactList.contains(contact->jid.latin1()))
		return;

	// update the contact data
	contactList[contact->jid.latin1()]->slotUpdateContact(contact->nick.latin1(), contact->localStatus(), contact->unavailableStatusString.latin1());
	
}

/*
 * Add new contact to the roster
 */
void JabberProtocol::slotNewContact(JabRosterEntry *contact)
{
	
	if (contactList.contains(contact->jid))
	{
		kdDebug() << "[JabberProtocol] Entry already exists for " << contact->jid << endl;
		return;
	}
	
	QString group = *(contact->groups.begin());

	KopeteContactList *l = KopeteContactList::contactList();
	KopeteMetaContact *m = l->findContact(this->id(), QString::null, contact->jid);
	KopeteContact *c = m->findContact(this->id(), QString::null,contact->jid);

	if (c)
	{
		// existing contact, update data
		kdDebug() << "[JabberProtocol] Contact " << contact->jid << " already exists, updating" << endl;
		
		QString tmpGroup = (!group.isNull() ? group : QString("") );
		((JabberContact *)c)->initContact(contact->jid, contact->nick, tmpGroup);
	}
	else
	{
		kdDebug() << "[JabberProtocol] Adding contact " << contact->jid << " ..." << endl;
		
		JabberContact *jabContact = new JabberContact(contact->jid, contact->nick, group ? group : QString(""), this, 0L, myContact->userID());
		contactList[contact->jid] = jabContact;

		connect(jabContact, SIGNAL(contactDestroyed(KopeteContact *)), this, SLOT(slotContactDestroyed(KopeteContact *)));
		
		kdDebug() << "[JabberProtocol] Contact has been added to contactList[]" << endl;
		kdDebug() << "[JabberProtocol] New Contact's userID is " << contactList[contact->jid]->userID() << endl;
		
		m->addContact(jabContact, group ? QStringList(group) : QStringList("Unknown"));
		
		metaContactMap.insert(m, jabContact);
	}

	slotContactUpdated(contact); /* More kludges! Ugh. */

}

/*
 * Create a new contact
 * FIXME: old API, needs to be removed!
 */
KopeteContact *JabberProtocol::createContact(KopeteMetaContact *parent, const QString &data)
{
	
	// assumption: data is just the JID; this could change at some stage
	addContact(data);
	
	JabberContact *contact = new JabberContact(data, "", QString(""), this, parent, myContact->userID());
	
	connect(contact, SIGNAL(contactDestroyed(KopeteContact *)), this, SLOT(slotContactDestroyed(KopeteContact *)));
	
	contactList[data] = contact;
	metaContactMap.insert(parent, contact);
	
	return contact;

}

/*
 * Slot to update the configuration data
 */
void JabberProtocol::slotSettingsChanged()
{
	QString currentId = QString::null;
	
	// read all configuration data from the configuration file
	KGlobal::config()->setGroup("Jabber");
	mUsername = KGlobal::config()->readEntry("UserID", "");
	mPassword = KGlobal::config()->readEntry("Password", "");
	mResource = KGlobal::config()->readEntry("Resource", "Kopete");
	mServer = KGlobal::config()->readEntry("Server", "jabber.org");
	mPort = KGlobal::config()->readNumEntry("Port", 5222);

	if(myContact)
	{
		currentId = myContact->userID();
		delete myContact;
	}
		
	// create a contact instance for self
	myContact = new JabberContact(QString("%1@%2").arg(mUsername, 1).arg(mServer, 2), mUsername, i18n("Unknown"), this, 0L, QString::null);

	if(currentId != myContact->userID())
	{
		for(JabberContactList::iterator it = contactList.begin(); it != contactList.end(); it++)
			delete it.data();

		// the account details were changed,
		// flush contact list
		contactList.clear();
		metaContactMap.clear();
	}
		
	// set the title according to the new changes
	actionStatusMenu->popupMenu()->changeTitle( m_menuTitleId , mUsername + "@" + mServer );

}

/*
 * Slot for sending a message
 */
void JabberProtocol::slotSendMsg(JabMessage message)
{
	
	// just pass the message on to the Psi backend if we are connected
	if (isConnected())
		mProtocol->sendMessage(message);

}

/*
 * Slot for displaying a new message
 */
void JabberProtocol::slotNewMessage(const JabMessage &message)
{
	QString jid = QString("%1@%2").arg(message.from.user(), 1).arg(message.from.host(), 2);

	if (message.from.user().isEmpty())
	{
		// if the sender field is empty, it is a server message
		kdDebug() << "[JabberProtocol] New server message for us!" << endl;
		
		KMessageBox::information(kopeteapp->mainWindow(), message.body, i18n("Jabber: Server message"));
	}
	else
	{
		kdDebug() << "[JabberProtocol] New message from '" << jid << "'" << endl;

		// see if the contact is actually in our roster
		if (!contactList.contains(jid))
		{
			// this should never happen - but _could_ happen if we are getting a message from
			// someone via a transport who hasn't previously subscribed with us (FIXME?)
			kdDebug() << "[JabberProtocol] Message received from an unknown contact, ignoring." << endl;
			return;
		}
		
		// pass the message on to the contact
		contactList[jid]->slotNewMessage(message);

	}

}

/*
 * Slot for notifying the availability of another resource for a contact
 * (called from Psi backend)
 */
void JabberProtocol::slotResourceAvailable(const Jid &jid, const JabResource &resource)
{
	QString jidString;
	
	jidString = QString("%1@%2").arg(jid.user(), 1).arg(jid.host(), 2);
	
	kdDebug() << "[JabberProtocol] New resource available for " << jidString << endl;
	
	if(!contactList.contains(jidString))
	{
		kdDebug() << "[JabberProtocol] Trying to add a resource, but couldn't find an entry for " << jidString << endl;
		return;
	}
	
	contactList[jidString]->slotResourceAvailable(jid, resource);

}

/*
 * Slot for notifying the removal of a certain resource for a contact
 * (called from Psi backend)
 */
void JabberProtocol::slotResourceUnavailable(const Jid &jid)
{
	QString jidString;
	
	jidString = QString("%1@%2").arg(jid.user(), 1).arg(jid.host(), 2);
	
	kdDebug() << "[JabberProtocol] Resource now unavailable for " << jidString << endl;

	if(!contactList.contains(jidString))
	{
		kdDebug() << "[JabberProtocol] Trying to remove a resource, but couldn't find an entry for " << jidString << endl;
		return;
	}
	
	contactList[jidString]->slotResourceUnavailable(jid);

}

/*
 * Slot for retrieving a vCard
 */
void JabberProtocol::slotSnarfVCard(QString &userID)
{
	JabTask *psiIOTask = mProtocol->ioUser();
	JT_VCard *tmpVCard = new JT_VCard(psiIOTask);
	
	// signal to ourselves when the vCard data arrived
	connect(tmpVCard, SIGNAL(finished(JabTask *)), this, SLOT(slotGotVCard(JabTask *)));
	
	tmpVCard->get(userID);
	tmpVCard->go();
	
}

/*
 * Slot being called when a vCard arrives
 */
void JabberProtocol::slotGotVCard(JabTask *task)
{
	JT_VCard *vCard = (JT_VCard *) task;
	
	if (!(vCard->success() && !vCard->vcard.isIncomplete()))
		// unsuccessful, or incomplete
		return;

	if(!contactList.contains(vCard->jid))
	{
		kdDebug() << "[JabberProtocol] slotGotVCard received a vCard - but couldn't find JID " << vCard->jid << " in the list!" << endl;
		return;
	}
	
	contactList[vCard->jid]->slotGotVCard(vCard);

	// FIXME: this should be done here, have to check dialog first for copying the data correctly though
//	delete vCard;
	
}

/*
 * Create a new user account on the server
 */
void JabberProtocol::registerUser()
{

	// a bit clumsy: hardwire registering to "on", connect and reset the flag
	mPrefs->save();
	doRegister = true;
	Connect();
	doRegister = false;

}

#include "jabberprotocol.moc"


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

