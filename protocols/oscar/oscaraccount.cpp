#include "oscaraccount.h"

#include <kaction.h>

#include "kopetecontact.h"
#include "kopetegroup.h"

#include "aimbuddylist.h"
#include "oscarsocket.h"
#include "oscarprotocol.h"
#include "oscarawaydialog.h"

OscarAccount::OscarAccount( KopeteProtocol *parent,
							const QString& accountID,
							const char *name=0L)
	: KopeteAccount(parent, accountID, name),
{
	m_engine = 0L;
	
	// Initialize our actions
	initActions();
	
	// Create our action menu
	initActionMenu();
	
	// Initialize the backend
	initEngine();
	
	// Init the myself contact
	m_myself = new OscarContact( this, accountId(), 0L);
	
	// Set us as offline to start with, just to be safe
	m_myself->setOnlineStatus(
		protocol()->getOnlineStatus( OscarProtocol::OFFLINE ));

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
	kdDebug( 14150 ) << k_funcinfo << "Disconnecting..." << endl;
	// Disconnect us
	disconnect();

	// Delete the backend
	if ( m_engine )
	{
		delete engine;
		engine = 0L;
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
			QString server = pluginData(this, "server");
			QString port = pluginData(this, "port");
			
			// Connect
			m_engine->doLogin( server, port, screenName, password );
			
			myself()->setOnlineStatus( 
				protocol()->getOnlineStatus( OscarProtocol::CONNECTING ));
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
		protocol()->getOnlineStatus( OscarProtocol::OFFLINE ));
	
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
		m_engine->sendAaway(false, "");
	}
}

KopeteContact* OscarAccount::myself()
{
	// Return our contact
	return m_myself;
}

OscarSocket* OscarAccount::getEngine()
{
	// Return the oscar socket object we're using
	return m_engine;
}

bool OscarAccount::addContactToMetaContact( const QString &contactId,
											const QString &displayName,
											KopeteMetaContact *parentContact )
{
	// First check to see if we're connected
	if(myself()->isOnline())
	{  // First task is to add it to the internal list and update the server
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
					m_internalBuddyList->addGroup(randomNewGroupNum, groupName);
				// Add the group on the server list
				getEngine()->sendAddGroup(internalGroup->name());
			}
			
			// Create a new internal buddy for this contact
			AIMBuddy *newBuddy = 
				new AIMBuddy(randomNewBuddyNum, internalGroup->ID(), contactId);
				
			// Check if it has an alias
			if( displayName != QString::null && displayName != contactId )
			{
				newBuddy->setAlias(displayName);
			}
			
			// Add the buddy to the internal buddy list
			m_internalBuddyList->addBuddy( newBuddy );
			
			// Add the buddy to the server's list, with the group
			getEngine()->sendAddBuddy( contactId, internalGroup->name());
			
			// Increase these counters, I'm not sure what this does
			randomNewGroupNum++;
			randomNewBuddyNum++;
			
			// Create the actual OscarContact, which adds it to the metacontact
			OscarContact *newContact = 
				new OscarContact( contactId, displayName, this, parentContact );
				
			// Return true
			return true;
		}
		else 
		{ // This is a temporary contact, so don't add it to the server list
			OscarContact *newContact =
				new OscarContact( contactId, displayName, this, parentContact );
			return (newContact != 0L);
		}
	}
	else 
	{ // We're not even online, so don't bother
		return false;
	}
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
									SLOT(slotEditOwnInfo()), 
									this, "actionInfo" );

	m_actionShowDebug = new KAction( i18n("Show Debug"), "wizard", 0,
									 this, SLOT(slotShowDebugDialog()), 
									 this, "actionInfo");
}

void OscarAccount::initActionMenu()
{
	m_actionMenu = new KActionMenu( accountID(), this );
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
	engine = new OscarSocket( pluginData(this, "server", cook );
	kdDebug( 14150 ) << "[OscarAccount: " 
					<< "] initEngine() END" << endl;
}

void OscarAccount::initSignals()
{
	// Contact list signals for group management events
	QObject::connect( KopeteContactList::contactList(),
		SIGNAL( groupRenamed( KopeteGroup *, const QString & ) ),
		SLOT( slotKopeteGroupRenamed( KopeteGroup * ) ) );

	QObject::connect( KopeteContactList::contactList(),
		SIGNAL( groupRemoved( KopeteGroup * ) ),
		SLOT( slotKopeteGroupRemoved( KopeteGroup * ) ) );
		
	// This is for when the user decides to add a group in the contact list
	QObject::connect(
		KopeteContactList::contactList(),
		SIGNAL( groupAdded(KopeteGroup *) ),
		SLOT( slotKopeteGroupAdded(KopeteGroup *) ) );
		
	// Protocol error
	QObject::connect( getEngine(),SIGNAL(protocolError(QString, int)),
		this, SLOT(slotError(QString, int)));
		
	// Got IM
	QObject::connect( getEngine(), SIGNAL(gotIM(QString,QString,bool)),
		this, SLOT(slotGotIM(QString,QString,bool)));
	
	// Got Config (Buddy List)
	QObject::connect( getEngine(), SIGNAL(gotConfig(AIMBuddyList &)),
		this, SLOT(slotGotServerBuddyList(AIMBuddyList &)));
		
	// Got my user info
	QObject::connect( getEngine(), SIGNAL(gotMyUserInfo(UserInfo)),
		this, SLOT(slotGotMyUserInfo(UserInfo)));
	
	// Status changed (I think my own status)
	QObject::connect( getEngine(), SIGNAL( statusChanged( const KopeteOnlineStatus & ) ),
		SLOT( slotStatusChanged( const KopeteOnlineStatus & ) ) );

	// Got warning
	QObject::connect( getEngine(), SIGNAL(gotWarning(int,QString)),
		this, SLOT(slotGotWarning(int,QString)));
		
	// Got direct IM request
	QObject::connect( getEngine(), SIGNAL(gotDirectIMRequest(QString)),
		this, SLOT(slotGotDirectIMRequest(QString)));
		
	// We have officially become idle
	QObject::connect(&mIdleMgr, SIGNAL(timeout()),
		this, SLOT(slotIdleTimeout()));
		
	// We have officially become un-idle
	QObject::connect(&mIdleMgr, SIGNAL(activity()),
		this, SLOT(slotIdleActivity()));
}

void OscarAccount::setUserProfile( QString profile )
{
	// Tell the engine to set the profile
	engine->setMyProfile( prof );
	// Save the user profile
	setPluginData( protocol(), accountId() + "Profile", prof);
}
KActionMenu* OscarAccount::actionMenu()
{
	return m_actionMenu;
}

			
void OscarAccount::slotGoOnline()
{
	if ( m_status == OscarAccount::AWAY )
	{ // If we're away , set us available
		kdDebug(14150) << "[OscarAccount: " << accountId
					   << "] slotGoOnline: was away, marking back" << endl;
		setAvailable();
	}
	else if( m_status == OscarAccount::OFFLINE )
	{ // If we're offline, connect
		kdDebug(14150) << "[OscarAccount: " << accountId
					   << "] slotGoOnline: was offline, now connecting" 
					   << endl;
		connect();
	}
	else
	{ // We're already online
		kdDebug(14150) << "[OscarAccount: " << accountId
					   << "] slotGoOnline: already online" << endl;
	}
}

void OscarAccount::slotGoOffline()
{
	// TODO: Write slotGoOffline
}

void OscarAccount::slotGoAway()
{
	kdDebug(14150) << "[OscarProtocol] slotGoAway()" << endl;
	if ( m_status == OscarAccount::ONLINE )
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
			this, engine->getMyProfile());
			
	// This is a modal dialog
	oscaruserinfo->exec(); 
}

void OscarAccount::slotShowDebug()
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
			protocol()->getOnlineStatus( OscarProtocol::OFFLINE ));
}

// Called when a group is added by adding a contact
void OscarAccount::slotGroupAdded(KopeteGroup *group)
{
	QString groupName = group->displayName();
	// See if we already have this group
	
	AIMGroup *aGroup = mBuddyList->findGroup(groupName);
	if (!aGroup)
	{
		aGroup = mBuddyList->addGroup(randomNewGroupNum, groupName);
		randomNewGroupNum++;
	} else return;
	kdDebug(14150) << "[OscarAccount: " << accountId() 
					<< "] addGroup() being called" << endl;
	if (m_connected)
		engine->sendAddGroup(groupName);
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
	if (!mBuddyList->findBuddy(sender))
		addNewContact(sender, QString(), false, i18n("Unknown"));
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
							<< "] Calling addOldContact on " << (*it)->screenname() << "." << endl;
			// TODO: Rename addOldContact to addServerContact
			addServerContact((*it));
		}
	}
}
