/***************************************************************************
                          msnprotocol.cpp  -  MSN Plugin
                             -------------------
    begin                : Wed Jan 2 2002
    copyright            : (C) 2002 by Duncan mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qcursor.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include "kmsnservice.h"
#include "kopete.h"
#include "msnaddcontactpage.h"
#include "msncontact.h"
#include "msnmessagedialog.h"
#include "msnpreferences.h"
#include "msnprotocol.h"
#include "msnuser.h"
#include "newuserimpl.h"
#include "statusbaricon.h"

MSNProtocol::MSNProtocol(): QObject(0, "MSNProtocol"), KopeteProtocol()
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

	kdDebug() << "MSN Protocol Plugin: Setting icon offline\n";
	statusBarIcon->setPixmap(offlineIcon);

	kdDebug() << "MSN Protocol Plugin: Creating Config Module\n";
	mPrefs = new MSNPreferences("msn_protocol", this);
	connect(mPrefs, SIGNAL(saved(void)), this, SIGNAL(settingsChanged(void)) );

	kdDebug() << "MSN Protocol Plugin: Creating MSN Engine\n";
	m_msnService = new KMSNService;

	connect( m_msnService, SIGNAL( connectingToService() ),
				this, SLOT( slotConnecting() ) );
	connect( m_msnService, SIGNAL( connectedToService( bool ) ),
				this, SLOT( slotConnectedToMSN( bool ) ) );
	connect( m_msnService, SIGNAL( statusChanged( uint ) ),
				this, SLOT( slotStateChanged( uint) ) );
	connect( m_msnService, SIGNAL( contactAdded( QString, QString, QString ) ),
				this, SLOT( slotContactAdded( QString, QString, QString ) ) );
	connect( m_msnService, SIGNAL( startChat( KMSNChatService *, QString ) ),
				this, SLOT( slotIncomingChat( KMSNChatService *, QString ) ) );
	connect( m_msnService, SIGNAL( newContact( QString ) ),
				this, SLOT( slotAuthenticate( QString ) ) );

	// Propagate signals from the MSN Service
	connect( m_msnService, SIGNAL( updateContact( QString, uint ) ),
				this, SIGNAL( updateContact( QString, uint ) ) );
	connect( m_msnService, SIGNAL( contactRemoved( QString, QString ) ),
				this, SIGNAL( contactRemoved( QString, QString ) ) );

	KGlobal::config()->setGroup("MSN");

	if ( (KGlobal::config()->readEntry("UserID", "") == "" ) || (KGlobal::config()->readEntry("Password", "") == "" ) )
	{
		QString emptyText = i18n( "<qt>If you have a <a href=\"http://www.passport.com\">MSN account</a>, please configure it in the Kopete Settings. Get a MSN account <a href=\"http://login.hotmail.passport.com/cgi-bin/register/en/default.asp\">here</a>.</qt>" );
		QString emptyCaption = i18n( "No MSN Configuration found!" );

		KMessageBox::error(kopeteapp->mainWindow(), emptyText,emptyCaption );
	}
	/** Autoconnect if is selected in config */
	if ( KGlobal::config()->readBoolEntry("AutoConnect", "0") )
	{
		Connect();
	}
}

MSNProtocol::~MSNProtocol()
{
}

/*
 * Plugin Class reimplementation
 */
void MSNProtocol::init()
{
}

bool MSNProtocol::unload()
{
	kdDebug() << "MSN Protocol: Unloading...\n";
	if( kopeteapp->statusBar() )
	{
		kopeteapp->statusBar()->removeWidget(statusBarIcon);
		delete statusBarIcon;
	}

	emit protocolUnloading();
	return true;
}

/*
 * KopeteProtocol Class reimplementation
 */
void MSNProtocol::Connect()
{
	if ( !isConnected() )
	{
		KGlobal::config()->setGroup("MSN");
		kdDebug() << "Attempting to connect to MSN" << endl;
		kdDebug() << "Setting Monopoly mode..." << endl;
		kdDebug() << "Using Microsoft UserID " << KGlobal::config()->readEntry("UserID", "0") << " with password (hidden)" << endl;
		KGlobal::config()->setGroup("MSN");

		m_msnService->setMyContactInfo( KGlobal::config()->readEntry("UserID", "")
								, KGlobal::config()->readEntry("Password", ""));
		//m_msnService->setMyPublicName(KGlobal::config()->readEntry("Nick", ""));
		m_msnService->connectToService();
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
		m_msnService->disconnect();
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


void MSNProtocol::setAway(void)
{
	slotGoAway();
}

void MSNProtocol::setAvailable(void)
{
	slotGoOnline();
}

bool MSNProtocol::isAway(void)
{
	uint status;
	status = m_msnService->getStatus();
	switch(status)
	{
		case NLN:
		{
			return false;
			break;
		}
		case FLN:
		case BSY:
		case IDL:
		case AWY:
		case PHN:
		case BRB:
		case LUN:
		{
	    	return true;
			break;
		}
	}
	return false;
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

/*
 * Internal functions implementation
 */
void MSNProtocol::initIcons()
{
	KIconLoader *loader = KGlobal::iconLoader();
    KStandardDirs dir;

	protocolIcon = QPixmap(loader->loadIcon("msn_protocol", KIcon::User));
	onlineIcon = QPixmap(loader->loadIcon("msn_online", KIcon::User));
	offlineIcon = QPixmap(loader->loadIcon("msn_offline", KIcon::User));
	awayIcon = QPixmap(loader->loadIcon("msn_away", KIcon::User));
	naIcon = QPixmap(loader->loadIcon("msn_na", KIcon::User));
	kdDebug() << "MSN Plugin: Loading animation " << loader->moviePath("msn_connecting", KIcon::User) << endl;
	connectingIcon = QMovie(dir.findResource("data","kopete/pics/msn_connecting.mng"));
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

	actionStatusMenu->plug( kopeteapp->systemTray()->getContextMenu(), 1 );
}

void MSNProtocol::slotIconRightClicked( const QPoint /* point */ )
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
	statusBarIcon->setMovie(connectingIcon);
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
	groups = m_msnService->getGroups();
	for ( QStringList::Iterator it = groups.begin(); it != groups.end(); ++it )
	{
		kdDebug() << "MSN Plugin: Searching contacts for group: [ " << (*it).latin1() << " ]" <<endl;

		// We get the contacts for this group
		contacts = m_msnService->getContacts( (*it).latin1() );
		for ( QStringList::Iterator it1 = contacts.begin(); it1 != contacts.end(); ++it1 )
		{
			userid = (*it1).latin1();
			publicname = m_msnService->getPublicName((*it1).latin1());

			kdDebug() << "MSN Plugin: Group OK, exists in contact list" <<endl;
			tmpcontact = new MSNContact( userid , publicname, (*it).latin1(), this );
			kopeteapp->contactList()->addContact(tmpcontact, (*it).latin1() );

			kdDebug() << "MSN Plugin: Created contact " << userid << " " << publicname << " with status " << status << endl;

			if( m_msnService->isBlocked( userid ) )
			{
				tmpcontact->setName(publicname + i18n(" Blocked") );
			}
		}
	}
	// FIXME: is there any way to do a faster sync of msn groups?
	/* Now we sync local groups that dont exist in server */
	QStringList localgroups = (kopeteapp->contactList()->groups()) ;
	QStringList servergroups = m_msnService->getGroups();
	QString localgroup;
	QString remotegroup;
	int exists;

	KGlobal::config()->setGroup("MSN");
	if ( KGlobal::config()->readBoolEntry("ExportGroups", true) )
	{
		for ( QStringList::Iterator it1 = localgroups.begin(); it1 != localgroups.end(); ++it1 )
		{
			exists = 0;
			localgroup = (*it1).latin1();
			for ( QStringList::Iterator it2 = servergroups.begin(); it2 != servergroups.end(); ++it2 )
			{
				remotegroup = (*it2).latin1();
				if ( localgroup == remotegroup )
				{
					exists++;
				}
			}

			/* Groups doesnt match any server group */
			if ( exists == 0 )
			{
				kdDebug() << "MSN Plugin: Sync: Local group " << localgroup << " dont exists in server!" << endl;
				/*
				QString notexistsMsg = i18n(
					"the group %1 doesn't exist in MSN server group list, if you want to move" \
					" a MSN contact to this group you need to add it to MSN server, do you want" \
					" to add this group to the server group list?" ).arg(localgroup);
				useranswer = KMessageBox::warningYesNo (kopeteapp->mainWindow(), notexistsMsg , i18n("New local group found...") );
				*/
				m_msnService->groupAdd( localgroup );
			}
		}
	}
}

void MSNProtocol::slotIncomingChat(KMSNChatService *newboard, QString reqUserID)
{
	MSNMessageDialog *messageDialog;

	// Maybe we have a copy of us in another group
	for( messageDialog = mChatWindows.first() ; messageDialog; messageDialog = mChatWindows.next() )
	{
		if ( messageDialog->user()->userID() == reqUserID )
			break;
	}

	if( messageDialog && messageDialog->isVisible() )
	{
		kdDebug() << "MSN Plugin: Incoming chat but Window already opened for " << reqUserID <<"\n";
		messageDialog->setBoard( newboard );
		connect(newboard,SIGNAL(msgReceived(QString,QString,QString, QFont, QColor)),messageDialog,SLOT(slotMessageReceived(QString,QString,QString, QFont, QColor)));
		messageDialog->raise();
	}
	else
	{
		kdDebug() << "MSN Plugin: Incoming chat, no window, creating window for " << reqUserID <<"\n";
		QString nick = m_msnService->getPublicName( reqUserID );

		// FIXME: MSN message dialog needs status

		// FIXME: We leak this object!
		MSNUser *user = new MSNUser( reqUserID, nick, MSNUser::Online );
		messageDialog = new MSNMessageDialog( user, newboard, this );
//		connect( this, SIGNAL(userStateChanged(QString)), messageDialog, SLOT(slotUserStateChanged(QString)) );
		connect( messageDialog, SIGNAL(closing(QString)), this, SLOT(slotMessageDialogClosing(QString)) );

		mChatWindows.append( messageDialog );
		messageDialog->show();
	}
}

void MSNProtocol::slotMessageDialogClosing(QString handle)
{
	mChatWindows.setAutoDelete(true);
	MSNMessageDialog *messageDialog = mChatWindows.first();
	for ( ; messageDialog; messageDialog = mChatWindows.next() )
	{
		if ( messageDialog->user()->userID() == handle )
		{
			mChatWindows.remove(messageDialog);
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
		Connect();
	else
		m_msnService->changeStatus( NLN );
}
void MSNProtocol::slotGoOffline()
{
	kdDebug() << "MSN Plugin: Going Offline" << endl;
	if (isConnected() )
		Disconnect();
	else // disconnect while trying to connect. Anyone know a better way? (remenic)
	{
		m_msnService->cancelConnect();
		statusBarIcon->setPixmap(offlineIcon);
	}
}
void MSNProtocol::slotGoAway()
{
	kdDebug() << "MSN Plugin: Going Away" << endl;
	if (!isConnected() )
		Connect();
	m_msnService->changeStatus( AWY );
}

void MSNProtocol::slotConnectedToMSN(bool c)
{
	mIsConnected = c;
	if ( c )
		slotConnected();
	else
		slotDisconnected();
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
		kopeteapp->contactList()->addContact(new MSNContact(userid, nick, i18n("Unknown"), this), i18n("Unknown"));
	}
}


void MSNProtocol::slotUserSetOffline (QString str)
{
	kdDebug() << "MSN Plugin: User Set Offline " << str << "\n";
}

void MSNProtocol::slotContactAdded( QString handle, QString nick, QString group)
{
	kdDebug() << "MSN Plugin: Contact Added in group " << group << " ... creating contact" << endl;
	kopeteapp->contactList()->addContact(new MSNContact( handle, nick, group, this ), group);
}

// Dont use this for now
void MSNProtocol::slotNewUserFound (QString userid )
{
	QString tmpnick = m_msnService->getPublicName(userid);
	kdDebug() << "MSN Plugin: User found " << userid << " " << tmpnick <<"\n";

	kopeteapp->contactList()->addContact(new MSNContact(userid, tmpnick, i18n("Unknown"), this), i18n("Unknown"));
}

// Dont use this for now
void MSNProtocol::slotNewUser (QString userid )
{
	QString tmpnick = m_msnService->getPublicName(userid);
	kdDebug() << "MSN Plugin: User found " << userid << " " << tmpnick <<"\n";

	kopeteapp->contactList()->addContact(new MSNContact(userid, tmpnick, i18n("Unknown"),this), i18n("Unknown"));
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
	m_msnService->contactAdd( handle );
}

void MSNProtocol::slotBlockContact( QString handle)
{
	m_msnService->contactBlock( handle );
}

void MSNProtocol::slotGoURL( QString url)
{
	kapp->invokeBrowser( url );
}

KMSNService* MSNProtocol::msnService() const
{
	return m_msnService;
}

void MSNProtocol::removeContact( const QString &userID )
{
	m_msnService->contactDelete( userID );
}

void MSNProtocol::removeFromGroup( const QString &userID, const QString &group )
{
	m_msnService->contactRemove( userID, group );
}

void MSNProtocol::moveContact( const QString &userID, const QString &oldGroup,
								const QString &newGroup )
{
	m_msnService->contactMove( userID, oldGroup, newGroup);
}

void MSNProtocol::copyContact( const QString &userID, const QString &newGroup )
{
	m_msnService->contactCopy( userID, newGroup);
}

QStringList MSNProtocol::groups()
{
	return m_msnService->getGroups();
}

// vim: set ts=4 sts=4 sw=4 noet:

#include "msnprotocol.moc"
