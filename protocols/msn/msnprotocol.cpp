/***************************************************************************
                          msnprotocol.cpp  -  description
                             -------------------
    begin                : Wed Jan 2 2002
    copyright            : (C) 2002 by duncan
    email                : duncan@tarro
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kdebug.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <klocale.h>
#include "msnprotocol.h"
#include "msncontact.h"
#include <msnadd.h>
#include "kopete.h"
#include <systemtray.h>
#include <msnaddcontactpage.h>
#include <qcursor.h>


///////////////////////////////////////////////////
//           Constructor & Destructor
///////////////////////////////////////////////////

MSNProtocol::MSNProtocol(): QObject(0, "MSN"), IMProtocol()
{
	QString path;
	path = locateLocal("data","kopete/msn.contacts");
	mContactsFile=new KSimpleConfig(path);
	path = locateLocal("data","kopete/msn.groups");
	mGroupsFile=new KSimpleConfig(path);

	mIsConnected = false;
	kdDebug() << "\nMSN Plugin Loading\n";
	
	/* Load all ICQ icons from KDE standard dirs */
 	initIcons();
	
	kdDebug() << "MSN Protocol Plugin: Creating Status Bar icon\n";
	statusBarIcon = new StatusBarIcon();
	QObject::connect(statusBarIcon, SIGNAL(rightClicked(const QPoint)), this, SLOT(slotIconRightClicked(const QPoint)));
	
	/* We init the actions to plug them in the Kopete gui */
	initActions();
	actionStatusMenu->plug( kopeteapp->systemTray()->getContextMenu() );	

	kdDebug() << "MSN Protocol Plugin: Setting icon offline\n";
	statusBarIcon->setPixmap(offlineIcon);

	kdDebug() << "MSN Protocol Plugin: Creating Config Module\n";
	new MSNPreferences(protocolIcon, this);
	
	kdDebug() << "MSN Protocol Plugin: Creating MSN Engine\n";
	engine = new KMSNService;
	connect(engine, SIGNAL(connectingToService()), this, SLOT(slotConnecting()) );
	connect(engine, SIGNAL(connectedToService(bool)), this, SLOT(slotConnectedToMSN(bool)));
	connect(engine, SIGNAL(contactStatusChanged(QString, QString, int)), this, SIGNAL(userStateChange (QString, QString, int) ) );
	connect(engine, SIGNAL(statusChanged( uint)), this, SLOT(slotStateChanged ( uint) ) );

	connect(engine, SIGNAL( newContact(QString) ), this, SLOT(slotAuthenticate(QString) ) );

	connect(kopeteapp->contactList(), SIGNAL( groupAdded(QString) ), this, SLOT(slotGroupAdded(QString) ) );
	connect(kopeteapp->contactList(), SIGNAL( deletingGroup(QString) ), this, SLOT(slotDeletingGroup(QString) ) );
		
	KGlobal::config()->setGroup("MSN");

	/** Autoconnect if is selected in config */
	if ( KGlobal::config()->readBoolEntry("AutoConnect", "0") )
	{
		Connect();
	}
	
}

MSNProtocol::~MSNProtocol()
{

}

///////////////////////////////////////////////////
//           Plugin Class reimplementation
///////////////////////////////////////////////////

void MSNProtocol::init()
{

}

bool MSNProtocol::unload()
{
	kdDebug() << "MSN Protocol: Unloading...\n";
	kopeteapp->statusBar()->removeWidget(statusBarIcon);
	delete statusBarIcon;
	// heh!
	emit protocolUnloading();
	return 1;
}

///////////////////////////////////////////////////
//           IMProtocol Class reimplementation
///////////////////////////////////////////////////

void MSNProtocol::Connect()
{
	if ( !isConnected() )
	{
		KGlobal::config()->setGroup("MSN");
		kdDebug() << "Attempting to connect to MSN" << endl;
		kdDebug() << "Setting Monopoly mode..." << endl;
		kdDebug() << "Using Micro$oft UserID " << KGlobal::config()->readEntry("UserID", "0") << " with password " << KGlobal::config()->readEntry("Password", "") << endl;
		KGlobal::config()->setGroup("MSN");

		engine->setMyContactInfo( KGlobal::config()->readEntry("UserID", "")
								, KGlobal::config()->readEntry("Password", ""));
		//engine->setMyPublicName(KGlobal::config()->readEntry("Nick", ""));
		engine->connectToService();
	}
	else
	{
    	kdDebug() << "MSN Plugin: Ignoring Connect request (Already Connected)" << endl;
	}	
}

void MSNProtocol::Disconnect()
{
	if ( isConnected() )
	{
		engine->disconnect();
	}
	else
	{
    	kdDebug() << "MSN Plugin: Ignoring Disconnect request (Im not Connected)" << endl;
	}	
}


bool MSNProtocol::isConnected()
{
	return mIsConnected;	
}

/** This i used for al protocol selection dialogs */
QPixmap MSNProtocol::getProtocolIcon()
{
	return protocolIcon;
}

AddContactPage *MSNProtocol::getAddContactWidget(QWidget *parent)
{
	return (new MSNAddContactPage(this,parent));	
}

///////////////////////////////////////////////////
//           Internal functions implementation
///////////////////////////////////////////////////

/** No descriptions */
void MSNProtocol::initIcons()
{
	KIconLoader *loader = KGlobal::iconLoader();

	protocolIcon = QPixmap(loader->loadIcon("msn_protocol", KIcon::User));
	onlineIcon = QPixmap(loader->loadIcon("msn_online", KIcon::User));
	offlineIcon = QPixmap(loader->loadIcon("msn_offline", KIcon::User));
	awayIcon = QPixmap(loader->loadIcon("msn_away", KIcon::User));
	naIcon = QPixmap(loader->loadIcon("msn_na", KIcon::User));
	kdDebug() << "MSN Plugin: Loading animation " << loader->moviePath("msn_connecting", KIcon::User) << endl;
	connectingIcon = QMovie(loader->loadMovie("msn_connecting.mng", KIcon::User));
	//connectingIcon = QMovie("/opt/kde3/share/apps/kopete/pics/msn_connecting.mng");
}

void MSNProtocol::initActions()
{
	actionGoOnline = new KAction ( i18n("Go online"), "msn_online", 0, this, SLOT(slotGoOnline()), this, "actionMSNConnect" );
	actionGoOffline = new KAction ( i18n("Go Offline"), "msn_offline", 0, this, SLOT(slotGoOffline()), this, "actionMSNConnect" );
	actionGoAway = new KAction ( i18n("Go Away"), "msn_away", 0, this, SLOT(slotGoAway()), this, "actionMSNConnect" );
	actionStatusMenu = new KActionMenu("MSN",this);
	actionStatusMenu->insert( actionGoOnline );
	actionStatusMenu->insert( actionGoOffline );
	actionStatusMenu->insert( actionGoAway );	
}

void MSNProtocol::slotIconRightClicked(const QPoint point)
{
	KGlobal::config()->setGroup("MSN");
	QString handle = KGlobal::config()->readEntry("UserID", i18n("(User ID not set)"));

	popup = new KPopupMenu(statusBarIcon);
	popup->insertTitle(handle);
	actionGoOnline->plug( popup );
	actionGoOffline->plug( popup );
	actionGoAway->plug( popup );	
	popup->popup(QCursor::pos());
}

/* While trying to connect :-) */
void MSNProtocol::slotConnecting()
{
	//statusBarIcon->setMovie(connectingIcon);	
}

/** NOTE: CALL THIS ONLY BEING CONNECTED */
void MSNProtocol::slotSyncContactList()
{
	if ( ! mIsConnected )
	{
		return;
	}
	/* First, delete D marked contacts */
	QStringList localcontacts;
    /*
	contactsFile->setGroup("Default");

   	contactsFile->readListEntry("Contacts",localcontacts);
   	QString tmpUin;
   	tmpUin.sprintf("%d",uin);
   	tmp.append(tmpUin);
   	cnt=contactsFile->readNumEntry("Count",0);
    */
}

/** OK! We are connected , let's do some work */
void MSNProtocol::slotConnected()
{
 	mIsConnected = true;
	MSNContact *tmpcontact;
		
 	QStringList groups, contacts;
 	QString group, publicname, userid;
 	uint status;
 	// First, we change status bar icon
 	statusBarIcon->setPixmap(onlineIcon);
     // We get the group list
 	groups = engine->getGroups();
 	for ( QStringList::Iterator it = groups.begin(); it != groups.end(); ++it )
 	{
 		QListViewItem *groupItem;
		if ( kopeteapp->contactList()->getGroup( (*it).latin1() ) == NULL )
		{
			kdDebug() << "MSN Plugin: Group: [ " << (*it).latin1() << " ] exits in server but not locally!! CREATING!" <<endl;
 		    kopeteapp->contactList()->addGroup( (*it).latin1() );
		}
		/* We now get the widget for the group */
		groupItem = kopeteapp->contactList()->getGroup( (*it).latin1() );
		//item=  new QListViewItem(ListView,(*it).latin1() ,"","1");
 		//item->setPixmap(0,expandedPixmap);
 		//item->setOpen(true);
	    kdDebug() << "MSN Plugin: Searching contacts for group: [ " << (*it).latin1() << " ]" <<endl;
 		// We get the contacts for this group
 		contacts = engine->getContacts( (*it).latin1() );
 		for ( QStringList::Iterator it1 = contacts.begin(); it1 != contacts.end(); ++it1 )
 	 	{
 	 		userid = (*it1).latin1();
 			publicname = engine->getPublicName((*it1).latin1());
 			/* We check if the group was created ok, if not, just no group */
			if ( groupItem )
			{
				kdDebug() << "MSN Plugin: Group OK, exists in contact list" <<endl;
				tmpcontact = new MSNContact( groupItem, userid , publicname , this );
			}
			else
			{
				kdDebug() << "MSN Plugin: Ups! The group widget was null!" <<endl;
				tmpcontact = new MSNContact( userid , publicname , this );
 			}
			//item1= new QListViewItem(item, engine->getPublicName((*it1).latin1())  , (*it1).latin1() ,"1");
 	 		status = engine->getStatus( userid );
 	 		kdDebug() << "MSN Plugin: Created contact " << userid << " " << publicname << " with status " << status << endl;
			switch(status)
 			{
  				case NLN:
  				{
  					tmpcontact->setPixmap(0, onlineIcon);
  					break;
  				}
  				case FLN:
  				{
  					tmpcontact->setPixmap(0, offlineIcon);
  					break;
  				}
  				case BSY:
  				{
  					tmpcontact->setPixmap(0, onlineIcon);
  					break;
  				}
  				case IDL:
  				{
  					tmpcontact->setPixmap(0, onlineIcon);
  					break;
  				}
  				case AWY:
  				{
  					tmpcontact->setPixmap(0, awayIcon);
  					break;
  				}
  				case PHN:
  				{
  					tmpcontact->setPixmap(0, onlineIcon);
  					break;
  				}
  				case BRB:
  				{
  					tmpcontact->setPixmap(0, onlineIcon);
  					break;
  				}
  				case LUN:
  				{
  					tmpcontact->setPixmap(0, onlineIcon);
  					break;
  				}
 			}
 	 		if( engine->isBlocked( userid ) )
 	 		{
 	 			tmpcontact->setText(0,  publicname + i18n(" Blocked") );
 	 			tmpcontact->setPixmap(0, onlineIcon);
 	 		}

 	 	}
 	}
}

void MSNProtocol::slotDisconnected()
{
		mIsConnected = false;
		statusBarIcon->setPixmap(offlineIcon);
}


void MSNProtocol::slotGoOnline()
{
	kdDebug() << "MSN Plugin: Going Online" << endl;
	if (!isConnected() )
	{
		kdDebug() << "MSN Plugin: Ups! we have to connect before going online" << endl;
		Connect();
	}
	else
	{
		engine->changeStatus( NLN );		
	}
}
void MSNProtocol::slotGoOffline()
{
	kdDebug() << "MSN Plugin: Going Offline" << endl;
	if (isConnected() )
	{
		Disconnect();
	}
}
void MSNProtocol::slotGoAway()
{
	kdDebug() << "MSN Plugin: Going Away" << endl;	
	if (!isConnected() )
	{
		Connect();
	}
	engine->changeStatus( AWY );
}

void MSNProtocol::slotConnectedToMSN(bool c)
{
		mIsConnected = c;
		if (c)
		{
			slotConnected();
		}
		else
		{
			slotDisconnected();
		}
}

void MSNProtocol::slotUserStateChange (QString handle, QString nick, int newstatus)
{
	kdDebug() << "MSN Plugin: User State change " << handle << " " << nick << " " << newstatus <<"\n";
}

void MSNProtocol::slotStateChanged (uint newstate)
{
	kdDebug() << "MSN Plugin: My Status Changed to " << newstate <<"\n";
	switch(newstate)
 	{
  		case NLN:
		{
  			statusBarIcon->setPixmap(onlineIcon);
			break;
		}
		case FLN:
		{
  			statusBarIcon->setPixmap(offlineIcon);
			break;
		}
		case AWY:
		{
  			statusBarIcon->setPixmap(awayIcon);
			break;
		}
		case BSY:
		{
  			statusBarIcon->setPixmap(awayIcon);
			break;
		}
		case IDL:
		{
  			statusBarIcon->setPixmap(awayIcon);
			break;
		}
		case PHN:
		{
  			statusBarIcon->setPixmap(awayIcon);
			break;
		}
		case BRB:
		{
  			statusBarIcon->setPixmap(awayIcon);
			break;
		}
		case LUN:
		{
  			statusBarIcon->setPixmap(awayIcon);
			break;
		}
 	}
}

void MSNProtocol::slotInitContacts (QString status, QString userid, QString nick)
{
	kdDebug() << "MSN Plugin: User State change " << status << " " << userid << " " << nick <<"\n";
	if ( status == "NLN" )
	{
		MSNContact *newContact = new MSNContact(userid, nick, this);
		newContact->setPixmap(0,onlineIcon);
	}
}


void MSNProtocol::slotUserSetOffline (QString str)
{
	kdDebug() << "MSN Plugin: User Set Offline " << str << "\n";
		
}
// Dont use this for now
void MSNProtocol::slotNewUserFound (QString userid )
{
	QString tmpnick = engine->getPublicName(userid);
	kdDebug() << "MSN Plugin: User found " << userid << " " << tmpnick <<"\n";
	MSNContact *newContact = new MSNContact(userid, tmpnick, this);
	newContact->setPixmap(0,offlineIcon);		

}
// Dont use this for now
void MSNProtocol::slotNewUser (QString userid )
{
	QString tmpnick = engine->getPublicName(userid);
	kdDebug() << "MSN Plugin: User found " << userid << " " << tmpnick <<"\n";
	MSNContact *newContact = new MSNContact(userid, tmpnick, this);
	newContact->setPixmap(0,offlineIcon);		

}

void MSNProtocol::slotAuthenticate( QString handle )
{
	NewUserImpl *authDlg = new NewUserImpl(0);
	authDlg->setHandle(handle);
	connect( authDlg, SIGNAL(addUser( QString )), this, SLOT(slotAddContact( QString )));
	connect( authDlg, SIGNAL(blockUser( QString )), this, SLOT(slotBlockContact( QString )));
	authDlg->show();
}

void MSNProtocol::slotAddContact( QString handle )
{
	engine->contactAdd( handle );
}

void MSNProtocol::slotBlockContact( QString handle)
{
	engine->contactBlock( handle );
}		

void MSNProtocol::slotGroupAdded( QString handle)
{
		
}
void MSNProtocol::slotDeletingGroup( QString handle)
{

}		