#include "oscaraccount.h"

#include <qapplication.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>

#include "kopetecontact.h"
#include "kopetegroup.h"
#include "kopeteaway.h"
#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetestdaction.h"

#include "aim.h" // For tocNormalize()
#include "aimbuddylist.h"
#include "oscarsocket.h"
#include "oscarchangestatus.h"
#include "oscarcontact.h"
#include "oscaruserinfo.h"
#include "oscardebugdialog.h"

OscarAccount::OscarAccount( KopeteProtocol *parent,
			    QString accountID,
			    const char *name)
    : KopeteAccount(parent, accountID, name)
{
    m_engine = 0L;

    // Initialize our actions
    initActions();

    // Create our action menu
    initActionMenu();

    // Initialize the backend
    initEngine();

    // Create the internal buddy list for this account
    m_internalBuddyList = new AIMBuddyList();

    // Init the myself contact
    m_myself = new OscarContact( accountId(), accountId(), this, 0L);

    // Set us as offline to start with, just to be safe
    m_myself->setOnlineStatus(
	OscarProtocol::protocol()->getOnlineStatus(
	    OscarProtocol::OFFLINE ));

    // Instantiate the away dialog
    m_awayDialog = new OscarChangeStatus( getEngine() );

    // The debug dialog
    m_debugDialog = new OscarDebugDialog();
    getEngine()->setDebugDialog( m_debugDialog );

    // Initialize the signals and slots
    initSignals();

    // Set our random new numbers
    m_randomNewBuddyNum = 0;
    m_randomNewGroupNum = 0;
}

OscarAccount::~OscarAccount()
{
    kdDebug( 14150 ) << k_funcinfo << "[" << accountId()
		     << "] deleted...Disconnecting..." << endl;
    // Disconnect us
    disconnect();

    // Delete the backend
    if ( m_engine )
    {
	delete m_engine;
	m_engine = 0L;
    }

    delete m_awayDialog;
    delete m_debugDialog;
    delete m_myself;
    delete m_actionGoOnline;
    delete m_actionGoOffline;
    delete m_actionGoAway;
    delete m_actionEditInfo;
    delete m_actionShowDebug;
}

void OscarAccount::connect()
{
    kdDebug(14150) << "[OscarAccount: " << accountId()
		   << "] connect()" << endl;

    // Get the screen name for this account
    QString screenName = accountId();

    if ( screenName != i18n("(No ScreenName Set)") )
    { // If we have a screen name set
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
	    kdDebug(14150) <<  "[OscarAccount: " << accountId()
			   << "] Logging in as " << screenName << endl;

	    // Get the server and port from the preferences
	    QString server = pluginData(protocol(),  "Server");
	    QString port = pluginData(protocol(),  "Port");

	    // Connect, need to normalize the name first
	    m_engine->doLogin( server, port.toInt(), tocNormalize(screenName), password );

	    myself()->setOnlineStatus(
		OscarProtocol::protocol()->getOnlineStatus( OscarProtocol::CONNECTING ));
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
    kdDebug(14150) << "[OscarAccount: " << accountId()
		   << "] Disconnect() START" << endl;

    kdDebug(14150) << "[OscarAccount:" << accountId()
		   << "] Disconnect(); calling engine->doLogoff()..." << endl;
    m_engine->doLogoff();

    kdDebug(14150) << "[OscarAccount: " << accountId()
		   << "] Disconnect(); logged out." << endl;
    kdDebug(14150) << "[OscarAccount: " << accountId()
		   << "] Setting status to offline." << endl;
    myself()->setOnlineStatus(
	OscarProtocol::protocol()->getOnlineStatus( OscarProtocol::OFFLINE ));

    kdDebug(14150) << "[OscarAccount: " << accountId()
		   << "] Disconnect() END" << endl;
}

void OscarAccount::setAway( bool away )
{
    kdDebug(14150) << "[OscarAccount: " << accountId()
		   << "] setAway()" << endl;

    if( away )
    { // Set away
	QString msg;
	msg = KopeteAway::getInstance()->message();
	if (msg.isEmpty())
	    msg = " ";
	// FIXME: Chris check this
	m_engine->sendAway(true, msg);
    }
    else
    { // Set back from away
	m_engine->sendAway(false, "");
    }
}

KopeteContact* OscarAccount::myself() const
{
    // Return our contact
    return m_myself;
}

OscarSocket* OscarAccount::getEngine()
{
    // Return the oscar socket object we're using
    return m_engine;
}



void OscarAccount::initActions()
{
    kdDebug(14150) << "[OscarAccount:" << accountId()
		   << "] initActions() START" << endl;

    m_actionGoOnline = new KAction ( i18n("Online"), "oscar_online", 0,
				     this, SLOT(slotGoOnline()),
				     this, "actionOscarConnect" );

    m_actionGoOffline = new KAction ( i18n("Offline"), "oscar_offline", 0,
				      this, SLOT(slotGoOffline()),
				      this, "actionOscarConnect" );

    m_actionGoAway = new KAction ( i18n("Away"), "oscar_away", 0,
				   this, SLOT(slotGoAway()),
				   this, "actionOscarConnect" );

    m_actionEditInfo =
	KopeteStdAction::contactInfo(this,
				     SLOT(slotEditInfo()),
				     this, "actionInfo" );

    m_actionShowDebug = new KAction( i18n("Show Debug"), "wizard", 0,
				     this, SLOT(slotShowDebugDialog()),
				     this, "actionInfo");
}

void OscarAccount::initActionMenu()
{
    m_actionMenu = new KActionMenu( accountId(), this );
    m_actionMenu->insert( m_actionGoOnline );
    m_actionMenu->insert( m_actionGoOffline );
    m_actionMenu->insert( m_actionGoAway );
    m_actionMenu->popupMenu()->insertSeparator();
    m_actionMenu->insert( m_actionEditInfo );
    m_actionMenu->popupMenu()->insertSeparator();
    m_actionMenu->insert( m_actionShowDebug );
}

void OscarAccount::initEngine()
{
    kdDebug( 14150 ) << "[OscarAccount: " << accountId()
		     << "] initEngine() START" << endl;
    QByteArray cook;
    cook.duplicate("01234567",8);
    m_engine =
	new OscarSocket( pluginData(protocol(),  "server"),
			 cook, this );
    kdDebug( 14150 ) << "[OscarAccount: "
		     << "] initEngine() END" << endl;
}

void OscarAccount::initSignals()
{
    // Contact list signals for group management events
    QObject::connect( KopeteContactList::contactList(),
		      SIGNAL( groupRenamed( KopeteGroup *, const QString & ) ),
		      SLOT( slotKopeteGroupRenamed( KopeteGroup * )));

    QObject::connect( KopeteContactList::contactList(),
		      SIGNAL( groupRemoved( KopeteGroup * ) ),
		      SLOT( slotKopeteGroupRemoved( KopeteGroup * ) ) );

    // This is for when the user decides to add a group in the contact list
    QObject::connect( KopeteContactList::contactList(),
		      SIGNAL( groupAdded(KopeteGroup *) ),
		      SLOT( slotKopeteGroupAdded(KopeteGroup *) ) );

    // Protocol error
    QObject::connect( getEngine(),
		      SIGNAL(protocolError(QString, int)),
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
    QObject::connect(&m_idleMgr,
		     SIGNAL(timeout()),
		     this, SLOT(slotIdleTimeout()));

    // We have officially become un-idle
    QObject::connect(&m_idleMgr, SIGNAL(activity()),
		     this, SLOT(slotIdleActivity()));
}

void OscarAccount::setUserProfile( QString profile )
{
    // Tell the engine to set the profile
    getEngine()->setMyProfile( profile );
    // Save the user profile
    setPluginData( protocol(),  "Profile", profile);
}

KActionMenu* OscarAccount::actionMenu()
{
    return m_actionMenu;
}

void OscarAccount::slotGoOnline()
{
    kdDebug(14510) << "[OscarContact] slotGoOnline: AWAY = "
		   << OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::AWAY).description()
		   << endl;

    if ( myself()->onlineStatus() ==
	 OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::AWAY))
    { // If we're away , set us available
	kdDebug(14150) << "[OscarAccount: " << accountId()
		       << "] slotGoOnline: was away, marking back" << endl;
	setAway(false);
    }
    else if( myself()->onlineStatus() ==
	     OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::OFFLINE))
    { // If we're offline, connect
	kdDebug(14150) << "[OscarAccount: " << accountId()
		       << "] slotGoOnline: was offline, now connecting"
		       << endl;
	connect();
    }
    else
    { // We're already online
	kdDebug(14150) << "[OscarAccount: " << accountId()
		       << "] slotGoOnline: already online" << endl;
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
    kdDebug(14150) << "[OscarProtocol] slotGoAway()" << endl;
    if ( myself()->onlineStatus() ==
	 OscarProtocol::protocol()->getOnlineStatus( OscarProtocol::ONLINE))
    {
	// Show the dialog, which takes care of the
	// setting of the away message
	m_awayDialog->show();
    }
}

void OscarAccount::slotEditInfo()
{
    OscarUserInfo *oscaruserinfo =
	new OscarUserInfo(accountId(), accountId(),
			  this, getEngine()->getMyProfile());

    // This is a modal dialog
    oscaruserinfo->exec();
}

void OscarAccount::slotShowDebugDialog()
{ // If there is a debug dialog, show it
    if(m_debugDialog){
	m_debugDialog->show();
    }
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

    myself()->setOnlineStatus(
	OscarProtocol::protocol()->getOnlineStatus( OscarProtocol::OFFLINE ));
}

// Called when a group is added by adding a contact
void OscarAccount::slotGroupAdded(KopeteGroup *group)
{
    QString groupName = group->displayName();

    // See if we already have this group
    AIMGroup *aGroup = m_internalBuddyList->findGroup(groupName);
    if (!aGroup)
    {
	aGroup = m_internalBuddyList->addGroup(m_randomNewGroupNum, groupName);
	m_randomNewGroupNum++;
	kdDebug(14150) << "[OscarAccount: " << accountId()
		       << "] addGroup() being called" << endl;
	if (isConnected()) {
	    getEngine()->sendAddGroup(groupName);
	}
    }
    else
    { // The server already has it in it's list, don't worry about it
	return;
    }
}

void OscarAccount::slotGroupRenamed( const QString& groupName, uint groupNumber)
{
    // Nothing
}

void OscarAccount::slotGroupRemoved( const QString& groupName, uint groupNumber)
{
    // Nothing
}
void OscarAccount::slotKopeteGroupRenamed( KopeteGroup *group )
{
    // Nothing
}
void OscarAccount::slotKopeteGroupRemoved( KopeteGroup *group )
{
    // Nothing
}

// Called when we have gotten an IM
void OscarAccount::slotGotIM(QString message, QString sender, bool isAuto)
{
    kdDebug(14150) << "[OscarAccount: " << accountId()
		   << "] slotGotIM(); got a buddy for the list, sender="
		   << sender << endl;

    //basically, all this does is add the person to your buddy list
    // TODO: Right now this has no support for "temporary" buddies
    // because I could not think of a good way to do this with
    // AIM.  Someone think of one - Chris
    if (!m_internalBuddyList->findBuddy(sender)) {
	addContact(tocNormalize(sender), sender, 0L, QString::null, false);
    }
}

// Called when we retrieve the buddy list from the server
void OscarAccount::slotGotServerBuddyList(AIMBuddyList &buddyList)
{
    kdDebug(14150) << "[OscarAccount: " << accountId()
		   << "] slotGotServerBuddyList()" << endl;

    //save server side contact list
    *m_internalBuddyList += buddyList;
    QValueList<AIMBuddy * > localList = buddyList.buddies().values();
    for ( QValueList<AIMBuddy * >::Iterator it = localList.begin(); it != localList.end(); ++it)
    {
	if ((*it))
	{
	    kdDebug(14150) << "[OscarAccount: " << accountId()
			   << "] Calling addOldContact on "
			   << (*it)->screenname() << "." << endl;
	    // Add the server contact to Kopete list
	    addServerContact((*it));
	}
    }
}

void OscarAccount::addServerContact(AIMBuddy *buddy)
{
    // This gets the contact in the kopete contact list for our account
    // that has this name, need to normalize once again
    OscarContact *contact =
	static_cast<OscarContact*>( contacts()[ tocNormalize(buddy->screenname()) ] );

    if (contact)
    { // Contact existed in the list already, sync information
	// Set the status
	contact->setOnlineStatus( buddy->status() );
	// TODO: write syncGroups in OscarContact
	contact->syncGroups();
    }
    else
    { // Contact is new on server and not in the Kopete list yet
	// Create a new metacontact for it
	//KopeteMetaContact *metaContact = new KopeteMetaContact();

	// Create a new OscarContact
// 	contact = new OscarContact( tocNormalize( buddy->screenname()),
// 				    buddy->alias(),
// 				    this, metaContact);

	// Set the contact's status
//	contact->setOnlineStatus( buddy->status() );

	// Get the group this buddy belongs to
	AIMGroup *aimGroup =
	    m_internalBuddyList->findGroup(buddy->groupID());

	if (aimGroup)
 	{ // If the group exists in the internal list
	    // Add it to the kopete contact list, with no metacontact
	    // which creates a new one.  This will also call the
	    // addContactToMetaContact method in this class
	    addContact( tocNormalize(buddy->screenname()),
			buddy->screenname(), 0L,
			aimGroup->name(),  false);

// 	    // Get the Kopete group
// 	    KopeteGroup *group =
// 		KopeteContactList::contactList()->getGroup(
// 		    aimGroup->name());

	    // tell the metacontact that it's in that group
// 	    metaContact->addToGroup( group );
	    // Tell the KopeteContactList about the contact
// 	    KopeteContactList::contactList()->addMetaContact(
// 		metaContact);
	}
	else
	{ // If the group doesn't exist on the server yet.
	    // This is really strange if we have the contact
	    // on the server but not the group it's in.
	    // May have to do something here in the future
	}

    }
}

bool OscarAccount::addContactToMetaContact( const QString &contactId,
					    const QString &displayName,
					    KopeteMetaContact *parentContact )
{
    /* This method is called in two situations, AFAIK.
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
     */

    if(myself()->isOnline())
    { // Check if we're online

	// Next check our internal list to see if we have this buddy
	// already, findBuddy tocNormalizes the buddy name for us
	AIMBuddy *internalBuddy =
	    m_internalBuddyList->findBuddy(contactId);

	if (internalBuddy)
	{ // We found the buddy internally
	    // Create an OscarContact for the metacontact
	    OscarContact *newContact =
		new OscarContact( contactId, displayName, this, parentContact );
	    // Set the oscar contact's status
	    newContact->setOnlineStatus( internalBuddy->status() );
	    return (newContact != 0L);
	}
	else
	{ // It was not on our internal list yet
	    kdDebug(14150) << k_funcinfo << " New Contact " << contactId
			   << " was not in internal list.  Creating new internal "
			   << "list entry" << endl;

	    // Check to see if it's a temporary contact, ie. not on our list
	    // but IMed us anyway
	    if( !parentContact->isTemporary() )
	    {  // Get a list of the groups it's in
		KopeteGroupList kopeteGroups = parentContact->groups();

		// AFAIK OSCAR doesn't support a contact in multiple groups, so we
		// just take the first one
		KopeteGroup *group = kopeteGroups.first();
		// Get the name of the group
		QString groupName = group->displayName();
		// See if it exists in our internal group list already
		AIMGroup *internalGroup = m_internalBuddyList->findGroup(groupName);

		if (!internalGroup)
		{  // If the group didn't exist
		    internalGroup =
			m_internalBuddyList->addGroup(m_randomNewGroupNum, groupName);
		    // Add the group on the server list
		    getEngine()->sendAddGroup(internalGroup->name());
		}

		// Create a new internal buddy for this contact
		AIMBuddy *newBuddy =
		    new AIMBuddy(m_randomNewBuddyNum, internalGroup->ID(), contactId);

		// Check if it has an alias
		if( displayName != QString::null && displayName != contactId )
		{
		    newBuddy->setAlias(displayName);
		}

		// Add the buddy to the internal buddy list
		m_internalBuddyList->addBuddy( newBuddy );

		// Add the buddy to the server's list, with the group,
		// need to normalize the contact name
		getEngine()->sendAddBuddy( tocNormalize(contactId), internalGroup->name());

		// Increase these counters, I'm not sure what this does
		m_randomNewGroupNum++;
		m_randomNewBuddyNum++;

		// Create the actual OscarContact, which adds it to the metacontact
		OscarContact *newContact =
		    new OscarContact( contactId, displayName, this, parentContact );
		newContact->setOnlineStatus(
		    OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::OFFLINE));

		// Add the metacontact to the KopeteContactList
		//KopeteContactList::contactList()->addMetaContact(parentContact);
		// Return true
		return true;
	    }
	    else
	    { // This is a temporary contact, so don't add it to the server list
		// Create the contact, which adds it to the parent contact
		OscarContact *newContact =
		    new OscarContact( contactId, displayName, this, parentContact );

		// Add it to the kopete contact list, I think this is done in
		// KopeteAccount::addContact
		//KopeteContactList::contactList()->addMetaContact(parentContact);

		// Set it's initial status
		// This requests the buddy's info from the server
		// I'm not sure what it does if they're offline, but there
		// is code in oscarcontact to handle the signal from
		// the engine that this causes
		kdDebug(14150) << "[OscarAccount: " << accountId()
			       << "] Requesting user info for " << contactId
			       << endl;
		getEngine()->sendUserProfileRequest( tocNormalize(contactId) );

		return (newContact != 0L);
	    }
	}
    }
    else
    { // We're not even online, so don't bother
	return false;
    }
}

void OscarAccount::slotOurStatusChanged( const KopeteOnlineStatus &newStatus )
{
    kdDebug( 14150 ) << k_funcinfo << "status=" << newStatus.internalStatus()
		     << endl;
    // update our own record of our status
    myself()->setOnlineStatus( newStatus );
}

// Called when we have been warned
void OscarAccount::slotGotWarning(int newlevel, QString warner)
{  //this is not a natural decrease in level
    if (m_userInfo.evil < newlevel)
    {
	QString warnMessage;
	if(warner.isNull())
	{
	    warnMessage = i18n("anonymously");
	}
	else
	{
	    warnMessage = i18n("...warned by...", "by %1").arg(warner);
	}

	QString message =
	    i18n("You have been warned %1. Your new warning level is %2%.").arg(
		warnMessage).arg(newlevel);
	KMessageBox::sorry(0L,message);
    }
    m_userInfo.evil = newlevel;
}


void OscarAccount::slotGotDirectIMRequest(QString sn)
{
    QString title = i18n("Direct IM session request");
    QString message =
	i18n("%1 has requested a direct IM session with you.  " \
	     "Direct IM sessions allow the remote user to see your IP " \
	     "address, which can lead to security problems if you don't " \
	     "trust him/her.  Do you want to establish a direct connection " \
	     "to %2?").arg(sn).arg(sn);

    int result =
	KMessageBox::questionYesNo(qApp->mainWidget(), message, title);

    if (result == KMessageBox::Yes)
    {
	getEngine()->sendDirectIMAccept(sn);
    }
    else if (result == KMessageBox::No)
    {
	getEngine()->sendDirectIMDeny(sn);
    }
}

void OscarAccount::slotGotMyUserInfo(UserInfo newInfo)
{
    m_userInfo = newInfo;
}

void OscarAccount::slotIdleActivity()
{
    kdDebug(14150) << k_funcinfo << "got some activity, setting idle time with server to 0" << endl;
    getEngine()->sendIdleTime( 0 );
}

/** Called when there is no activity for a certain amount of time  */
void OscarAccount::slotIdleTimeout()
{
    kdDebug(14150) << k_funcinfo << "got an idle timeout, setting idle time with server" << endl;
    //idleTimeout() gives a value in minutes, engine wants seconds
    int idleTimeout = 0;
    QString idleString = pluginData(protocol(),  "IdleTimeOut");
    idleTimeout = idleString.toInt();

    getEngine()->sendIdleTime( idleTimeout * 60 );
}

int OscarAccount::randomNewBuddyNum()
{
    return m_randomNewBuddyNum++;
}

int OscarAccount::randomNewGroupNum()
{
    return m_randomNewGroupNum++;
}

AIMBuddyList *OscarAccount::internalBuddyList()
{
    return m_internalBuddyList;
}

void OscarAccount::setServer( QString server )
{
    setPluginData(protocol(),  "Server", server);
}

void OscarAccount::setPort( int port )
{
    if ( port > 0 )
    { // Do a little error checkin on it
	setPluginData(protocol(), "Port", QString::number( port ));
    }
}
