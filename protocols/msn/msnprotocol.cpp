/***************************************************************************
                          msnprotocol.cpp  -  MSN Plugin
                             -------------------
    Copyright (c) 2002   by Duncan Mac-Vicar P. <duncan@kde.org>
    Copyright (c) 2002   by Martijn Klingens    <klingens@kde.org>
    Copyright (c) 2002   by Olivier Goffart     <ogoffart@tiscalinet.be>

    Copyright (c) 2002  by the Kopete developers  <kopete-devel@kde.org>
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

#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
//#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>

#include "kopete.h"
#include "kopetecontactlist.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopetewindow.h"
#include "msnaddcontactpage.h"
#include "msncontact.h"
#include "msndebugrawcmddlg.h"
#include "msnidentity.h"
#include "msnnotifysocket.h"
#include "msnpreferences.h"
#include "msnprotocol.h"
#include "msnmessagemanager.h"
#include "newuserimpl.h"
#include "statusbaricon.h"
#include "systemtray.h"

K_EXPORT_COMPONENT_FACTORY( kopete_msn, KGenericFactory<MSNProtocol> );

MSNProtocol::MSNProtocol( QObject *parent, const char *name,
	const QStringList & /* args */ )
: KopeteProtocol( parent, name )
{
	QString protocolId = this->id();

	if( s_protocol )
		kdDebug() << "MSNProtocol::MSNProtocol: WARNING: s_protocol already defined!" << endl;
	else
		s_protocol = this;

	// Go in experimental mode: enable the new API :-)
	//enableStreaming( true );

	m_status = FLN;
	m_connectstatus=NLN;
	mIsConnected = false;
	m_notifySocket = 0L;
	m_myself=0L;

	m_identity = new MSNIdentity( this, "m_identity" );

	kdDebug() << "MSNProtocol::MSNProtocol: MSN Plugin Loading" << endl;

	initIcons();
	mPrefs= new MSNPreferences( "msn_protocol", this );
	connect( mPrefs, SIGNAL(saved()) , this , SLOT ( slotPreferencesSaved() ));
	slotPreferencesSaved();

	statusBarIcon = new StatusBarIcon();

	m_publicNameSyncMode = SyncFromServer;
	m_publicNameSyncNeeded = false;
	
	m_addWizard_metaContact=0L;
	m_connectstatus=NLN;

	initActions();

	QObject::connect(statusBarIcon, SIGNAL(rightClicked(const QPoint&)), this, SLOT(slotIconRightClicked(const QPoint&)));
	statusBarIcon->setPixmap( offlineIcon );

	// FIXME: I think we should add a global self metaContact (Olivier)
	m_myself = new MSNContact( m_msnId,m_publicName, "", 0L );

	if ( mPrefs->autoConnect() )
		Connect();             

	/*if( ( cfg->readEntry( "UserID", "" ).isEmpty() ) ||
		( cfg->readEntry( "Password", "" ).isEmpty() ) )
	{
		QString emptyText =
			i18n( "<qt>If you have an "
			"<a href=\"http://www.passport.com\">MSN account</a>, "
			"please configure it in the Kopete Settings.\n"
			"Get an MSN account <a href=\"http://login.hotmail.passport.com/"
			"cgi-bin/register/en/default.asp\">here</a>.</qt>" );
		QString emptyCaption = i18n( "MSN Not Configured Yet" );

		KMessageBox::information( kopeteapp->mainWindow(),
			emptyText, emptyCaption, QString::null,
			KMessageBox::AllowLink );
	}
	*/
}

MSNProtocol::~MSNProtocol()
{
	s_protocol = 0L;
}

/*
 * Plugin Class reimplementation
 */
void MSNProtocol::init()
{
}

bool MSNProtocol::unload()
{
	kdDebug() << "MSNProtocol::unload" << endl;

	Disconnect();
	
	if( kopeteapp->statusBar() )
	{
		kopeteapp->statusBar()->removeWidget(statusBarIcon);
		delete statusBarIcon;
	}

  	m_groupList.clear();
	m_allowList.clear();
	m_blockList.clear();

	if(m_notifySocket)
	{
		kdDebug() << "MSNProtocol::unload: WARNING NotifySocket was not deleted" <<endl;
		delete m_notifySocket;
	}
	
	return KopeteProtocol::unload();
}

/*
 * KopeteProtocol Class reimplementation
 */
void MSNProtocol::Connect()
{
	if( isConnected() )
	{
		kdDebug() << "MSN Plugin: Ignoring Connect request "
			<< "(Already Connected)" << endl;
		return;
	}

	if( m_msnId.isEmpty() )
	{
		int r = KMessageBox::warningContinueCancel(kopeteapp->mainWindow(),
			i18n("<qt>You have not yet specified a username for MSN. "
				"You can specify your MSN settings in the Kopete configuration dialog<br>"
				"Get an MSN account <a href=\"http://login.hotmail.passport.com/cgi-bin/register/en/default.asp\">here</a><br>"
				"Do you want to configure MSN now?</qt>" ),
			i18n( "MSN plugin not configured yet" ),
			KGuiItem( i18n( "C&onfigure..." ), "configure" ),  QString::null,
			KMessageBox::AllowLink );

		if( r != KMessageBox::Cancel )
		{
			mPrefs->activate();
		}
		return;
	}

	if(m_notifySocket)
	{
		kdDebug() << "MSNProtocol::connect: WARNING NotifySocket was not deleted"  <<endl;
		delete m_notifySocket;
	}

	kdDebug() << "MSNProtocol::connect: Connecting to MSN with Passport "
		<< m_msnId << endl;
	m_notifySocket = new MSNNotifySocket( m_msnId );

	connect( m_notifySocket, SIGNAL( groupAdded( QString, uint,uint ) ),
		this, SLOT( slotGroupAdded( QString, uint, uint ) ) );
	connect( m_notifySocket, SIGNAL( groupRenamed( QString, uint, uint ) ),
		this, SLOT( slotGroupRenamed( QString, uint, uint ) ) );
	connect( m_notifySocket, SIGNAL( groupName( QString, uint ) ),
		this, SLOT( slotGroupListed( QString, uint ) ) );
	connect( m_notifySocket, SIGNAL(groupRemoved( uint, uint ) ),
		this, SLOT( slotGroupRemoved( uint, uint ) ) );
//	connect( m_notifySocket, SIGNAL( statusChanged( QString ) ),
//		this, SLOT( slotStateChanged( QString ) ) );
	connect( m_notifySocket, SIGNAL( contactList( QString, QString, QString, QString ) ),
		this, SLOT( slotContactList( QString, QString, QString, QString ) ) );
	connect( m_notifySocket, SIGNAL( contactAdded( QString, QString, QString, uint, uint ) ),
		this,	SLOT( slotContactAdded( QString, QString, QString, uint, uint ) ) );
	connect( m_notifySocket, SIGNAL( contactRemoved( QString, QString, uint, uint ) ),
		this,	SLOT( slotContactRemoved( QString, QString, uint, uint ) ) );
	connect( m_notifySocket, SIGNAL( statusChanged( QString ) ),
		this, SLOT( slotStatusChanged( QString ) ) );
	connect( m_notifySocket, SIGNAL( onlineStatusChanged( MSNSocket::OnlineStatus ) ),
		this, SLOT( slotNotifySocketStatusChanged( MSNSocket::OnlineStatus ) ) );
	connect( m_notifySocket, SIGNAL( publicNameChanged( QString ) ),
		this, SLOT( slotPublicNameChanged( QString ) ) );
	connect( m_notifySocket, SIGNAL( invitedToChat( QString, QString, QString, QString, QString ) ),
		this, SLOT( slotCreateChat( QString, QString, QString, QString, QString ) ) );
	connect( m_notifySocket, SIGNAL( startChat( QString, QString ) ),
		this, SLOT( slotCreateChat( QString, QString ) ) );
	connect( m_notifySocket, SIGNAL( socketClosed( int ) ),
		this, SLOT( slotNotifySocketClosed( int ) ) );
	connect( m_notifySocket, SIGNAL( hotmailSeted( bool ) ),
		m_openInboxAction, SLOT( setEnabled( bool ) ) );

	m_notifySocket->setStatus( m_connectstatus );
	m_notifySocket->connect( m_password );
	statusBarIcon->setMovie( connectingIcon );
	m_openInboxAction->setEnabled(false);
}

void MSNProtocol::Disconnect()
{
	if( m_notifySocket )
	{
		m_notifySocket->disconnect();

		//delete m_notifySocket;
//		m_notifySocket->deleteLater();
//		m_notifySocket = 0L;
	}
}

bool MSNProtocol::isConnected() const
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

bool MSNProtocol::isAway(void) const
{
	switch( m_status )
	{
		case NLN:
			return false;
		case FLN:
		case BSY:
		case IDL:
		case AWY:
		case PHN:
		case BRB:
		case LUN:
			return true;
		default:
			return false;
	}
}

bool MSNProtocol::serialize( KopeteMetaContact *metaContact,
	QStringList &stream ) const
{
	//kdDebug() << "MSNProtocol::serialize " << metaContact->displayName()
	//<< endl;
	//MSNContact *c = m_metaContacts.find( metaContact );
	bool r=false;

	QPtrList<KopeteContact> contacts = metaContact->contacts();
	for( 	KopeteContact *c = contacts.first(); c ; c = contacts.next() )
	{
		if ( c->protocol()->id() != this->id() ) // not our contact, next one please
				continue;
		
		MSNContact *g = static_cast<MSNContact*>(c);
		
		if( g )
		{
			stream << g->id() << g->displayName() << g->groups().join( "," );
			r=true;
		}
	}
	return r;
}

void MSNProtocol::deserialize( KopeteMetaContact *metaContact,
	const QStringList &strList )
{
/*	kdDebug() << "MSNProtocol::deserialize: " << metaContact->displayName()
		<< ", [ " << strList.join( ", " ) << " ]" << endl;*/

	QString protocolId = this->id();

	uint idx = 0;
	while( idx < strList.size() )
	{
		QString passport    = strList[ idx ];
		QString displayName = strList[ idx + 1 ];
		QStringList groups  = QStringList::split( ",", strList[ idx + 2 ] );

/*		kdDebug() << "new MSNContact( " << protocolId << ", " << passport
			<< ", " << displayName << ", " << groups.first()
			<< ", " << metaContact->displayName() << endl;*/

		// Create MSN contact
		// FIXME: I think this should go in a single method, as it is
		// duplicated everywhere now - Martijn
		MSNContact *c = new MSNContact( passport, displayName,
			groups.first(), metaContact );

		c->setMsnStatus( MSNProtocol::FLN );

		connect( c, SIGNAL( contactDestroyed( KopeteContact * ) ),
			SLOT( slotContactDestroyed( KopeteContact * ) ) );
		//m_metaContacts.insert( metaContact, c );

		QStringList::Iterator it = groups.begin();
		if( it != groups.end() )
			++it; // Skip the first item as it was passed to the ctor already
		for( ; it != groups.end(); ++it )
			c->addedToGroup( groupName( (*it).toUInt() ) );

		metaContact->addContact( c);

		m_contacts.insert( c->msnId(), c );

		idx += 3;
	}
}

KopeteContact* MSNProtocol::myself() const
{
	return m_myself;
}

/** This i used for al protocol selection dialogs */
QString MSNProtocol::protocolIcon() const
{
	return "msn_protocol";
}

AddContactPage *MSNProtocol::createAddContactWidget(QWidget *parent)
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

	onlineIcon = QPixmap(loader->loadIcon("msn_online", KIcon::User));
	offlineIcon = QPixmap(loader->loadIcon("msn_offline", KIcon::User));
	awayIcon = QPixmap(loader->loadIcon("msn_away", KIcon::User));
	naIcon = QPixmap(loader->loadIcon("msn_na", KIcon::User));
	kdDebug() << "MSN Plugin: Loading animation " << loader->moviePath("msn_connecting", KIcon::User) << endl;
	connectingIcon = QMovie(dir.findResource("data","kopete/pics/msn_connecting.mng"));
}

void MSNProtocol::slotOpenInbox()
{
	if(m_notifySocket)
		m_notifySocket->slotOpenInbox();
}

void MSNProtocol::initActions()
{
	actionGoOnline = new KAction ( i18n("Go O&nline"), "msn_online", 0, this, SLOT(slotGoOnline()), this, "actionMSNConnect" );
	actionGoOffline = new KAction ( i18n("Go &Offline"), "msn_offline", 0, this, SLOT(slotGoOffline()), this, "actionMSNConnect" );
	actionGoAway = new KAction ( i18n("Set &Away"), "msn_away", 0, this, SLOT(slotGoAway()), this, "actionMSNConnect" );
	actionGoBusy = new KAction ( i18n("Set &Busy"), "msn_na", 0, this, SLOT(slotGoBusy()), this, "actionMSNConnect" );
	actionGoBeRightBack = new KAction ( i18n("Set Be &right back"), "msn_away", 0, this, SLOT(slotGoBeRightBack()), this, "actionMSNConnect" );
	actionGoOnThePhone = new KAction ( i18n("Set On the &phone"), "msn_na", 0, this, SLOT(slotGoOnThePhone()), this, "actionMSNConnect" );
	actionGoOutToLunch = new KAction ( i18n("Set Out to &Lunch"), "msn_away", 0, this, SLOT(slotGoOutToLunch()), this, "actionMSNConnect" );
	actionGoInvisible = new KAction ( i18n("Set &Invisible"), "msn_offline", 0, this, SLOT(slotGoInvisible()), this, "actionMSNConnect" );

	m_renameAction = new KAction ( i18n( "&Change Nickname..." ),
		QString::null, 0, this, SLOT( slotChangePublicName() ),
		this, "m_renameAction" );
	actionStatusMenu = new KActionMenu( "MSN", this );

	m_startChatAction = new KAction ( i18n( "&Start chat..." ), "mail_generic", 0, this, SLOT( slotStartChat() ),
		this, "m_renameAction" );

	m_openInboxAction = new KAction ( i18n( "Open inbo&x" ), "mail_generic", 0, this, SLOT( slotOpenInbox() ), this, "m_openInboxAction" );
	m_openInboxAction->setEnabled(false);

	m_debugMenu = new KActionMenu( "Debug", this );
	m_debugRawCommand = new KAction( i18n( "Send Raw C&ommand..." ), 0,
		this, SLOT( slotDebugRawCommand() ), this, "m_debugRawCommand" );

	m_menuTitleId = actionStatusMenu->popupMenu()->insertTitle(
		*( statusBarIcon->pixmap() ),
		i18n( "%1 (%2)" ).arg( m_publicName ).arg( m_msnId ) );
		
	actionStatusMenu->insert( actionGoOnline );
	actionStatusMenu->insert( actionGoAway );
	actionStatusMenu->insert( actionGoBusy );
	actionStatusMenu->insert( actionGoBeRightBack );
	actionStatusMenu->insert( actionGoOnThePhone );
	actionStatusMenu->insert( actionGoOutToLunch );
	actionStatusMenu->insert( actionGoInvisible );
	actionStatusMenu->insert( actionGoOffline );

	actionStatusMenu->popupMenu()->insertSeparator();
	actionStatusMenu->insert( m_renameAction );
	actionStatusMenu->insert( m_startChatAction );
	actionStatusMenu->popupMenu()->insertSeparator();
	actionStatusMenu->insert( m_openInboxAction );

#if !defined NDEBUG
	actionStatusMenu->popupMenu()->insertSeparator();
	actionStatusMenu->insert( m_debugMenu );

	m_debugMenu->insert( m_debugRawCommand );
#endif

	actionStatusMenu->plug( kopeteapp->systemTray()->contextMenu(), 1 );
}

void MSNProtocol::slotIconRightClicked( const QPoint& /* point */ )
{
	actionStatusMenu->popup( QCursor::pos() );
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

void MSNProtocol::slotGoOnline()
{
	m_connectstatus=NLN;
	kdDebug() << "MSN Plugin: Going Online" << endl;
	if (!isConnected() )
		Connect();
	else
		setStatus( NLN );
}

void MSNProtocol::slotGoOffline()
{
	Disconnect();
	m_connectstatus=NLN;
}

void MSNProtocol::slotGoAway()
{
	setStatus( AWY );
}
void MSNProtocol::slotGoBusy()
{
	setStatus( BSY );
}

void MSNProtocol::slotGoBeRightBack()
{
	setStatus( BRB );
}
void MSNProtocol::slotGoOnThePhone()
{
	setStatus( PHN );
}
void MSNProtocol::slotGoOutToLunch()
{
	setStatus( LUN );
}                     
void MSNProtocol::slotGoInvisible()
{
	setStatus( HDN );
}

        
void MSNProtocol::setStatus(Status s)
{
	if (isConnected() )
	{
		m_notifySocket->setStatus( s );
	}
	else
	{
		m_connectstatus=s;
		Connect();
	}
}

void MSNProtocol::slotStartChat()
{
	if ( !isConnected() )
	{
		KMessageBox::error( 0l,
			i18n( "<qt>Please go online before you start a chat</qt>" ),
			i18n( "MSN Plugin" ));
		return;
	}

	bool ok;
	QString handle = KLineEditDlg::getText(
		i18n( "Start chat - MSN Plugin" ),
		i18n( "Please enter the email address of the person with whom you want to chat" ),
		QString::null, &ok );
	if( ok )
	{
		if( handle.contains('@') ==1 && handle.contains('.') >=1)
		{
			m_msgHandle = handle;
			// don't crash when we were disconnected before we got the address
			if ( m_notifySocket ) m_notifySocket->createChatSession();
		}
		else
		{
			KMessageBox::error(0l, i18n("<qt>You must enter a valide e-mail adress</qt>"), i18n("MSN Plugin"));
		}
	}
}

void MSNProtocol::slotNotifySocketStatusChanged( MSNSocket::OnlineStatus status )
{
	kdDebug() << "MSNProtocol::slotOnlineStatusChanged: " << status <<endl;
	mIsConnected = (status == MSNSocket::Connected);
	if ( mIsConnected )
	{
		//kopeteapp->sessionFactory()->cleanSessions(this);
		// Sync public name when needed
		if( m_publicNameSyncNeeded )
		{
			kdDebug() << "MSNProtocol::slotOnlineStatusChanged: Syncing public name to "
				<< m_publicName << endl;
			setPublicName( m_publicName );
			m_publicNameSyncNeeded = false;
		}
		else
		{
			kdDebug() << "MSNProtocol::slotOnlineStatusChanged: Leaving public name as "
				<< m_publicName << endl;
		}

		mIsConnected = true;

		// Now pending changes are updated we want to sync both ways
		m_publicNameSyncMode = SyncBoth;

		QStringList contacts;
		QString group, publicname, userid;

		statusBarIcon->setPixmap( onlineIcon );

		// FIXME: is there any way to do a faster sync of msn groups?
		/* Now we sync local groups that don't exist on server */
		QStringList localgroups = KopeteContactList::contactList()->groups().toStringList() ;
		QStringList servergroups = groups();
		QString localgroup;
		QString remotegroup;
		int exists;

		if ( mPrefs->exportGroups() )
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
					kdDebug() << "MSNProtocol::slotOnlineStatusChanged: Sync: Local group " << localgroup << " doesn't exist on server!" << endl;
					/*
					QString notexistsMsg = i18n(
						"the group %1 doesn't exist in MSN server group list, if you want to move" \
						" a MSN contact to this group you need to add it to MSN server, do you want" \
						" to add this group to the server group list?" ).arg(localgroup);
					useranswer = KMessageBox::warningYesNo (kopeteapp->mainWindow(), notexistsMsg , i18n("New Local Group Found") );
					*/
					addGroup( localgroup );
				}
			}
		}
	}
	else if( status == MSNSocket::Disconnected )
	{
		KopeteMessageManagerDict sessions =
			kopeteapp->sessionFactory()->protocolSessions( this );
		QIntDictIterator<KopeteMessageManager> kmmIt( sessions );
		for( ; kmmIt.current() ; ++kmmIt )
		{
			// Disconnect all active chats (but don't actually remove the
			// chat windows, the user might still want to view them!)
			kmmIt.current()->slotSendEnabled( false );
			MSNMessageManager *msnMM =
				dynamic_cast<MSNMessageManager *>( kmmIt.current() );
			if( msnMM )
			{
				kdDebug() << "MSNProtocol::slotOnlineStatusChanged: "
					<< "Closed MSNMessageManager because the protocol socket "
					<< "closed." << endl;
				msnMM->slotCloseSession();
			}
/*			else
			{
				kdDebug() << "MSNProtocol::slotOnlineStatusChanged: "
					<< "KMM is not an MSN message manager, not closing "
					<< "connection." << endl;
			}*/
		}

		QMap<QString, MSNContact*>::Iterator it;
		for ( it = m_contacts.begin(); it != m_contacts.end() ; ++it)
		{
			(*it)->setMsnStatus( MSNProtocol::FLN );
		}

		m_allowList.clear();
		m_blockList.clear();
		m_groupList.clear();

		mIsConnected = false;
		statusBarIcon->setPixmap(offlineIcon);
		m_openInboxAction->setEnabled(false);

		m_status = FLN;

		// Reset flags. They can't be set in the connect method, because
		// offline changes might have been made before. Instead the c'tor
		// sets the defaults, and the disconnect slot resets those defaults
		// FIXME: Can't we share this code?
		m_publicNameSyncMode = SyncFromServer;
	}
	else if (status==MSNSocket::Connecting)
	{
		QMap<QString, MSNContact*>::Iterator it;
		for ( it = m_contacts.begin(); it != m_contacts.end() ; ++it)
		{
			(*it)->setMsnStatus( MSNProtocol::UNK );
		}
	}
}

void MSNProtocol::slotStatusChanged( QString status )
{
	m_status = convertStatus( status );
	m_myself->setMsnStatus(m_status);

	kdDebug() << "MSN Plugin: My Status Changed to " << m_status <<
		" (" << status <<")\n";

	switch( m_status )
	{
		case NLN:
			statusBarIcon->setPixmap(onlineIcon);
			break;
		case AWY:
			statusBarIcon->setPixmap(awayIcon);
			break;
		case BSY:
			statusBarIcon->setPixmap(naIcon);
			break;
		case IDL:
			statusBarIcon->setPixmap(awayIcon);
			break;
		case PHN:
			statusBarIcon->setPixmap(naIcon);
			break;
		case BRB:
			statusBarIcon->setPixmap(awayIcon);
			break;
		case LUN:
			statusBarIcon->setPixmap(awayIcon);
			break;
		case FLN:
		default:
			statusBarIcon->setPixmap(offlineIcon);
			break;
	}
}


void MSNProtocol::slotAddContact( QString handle )
{
	addContact( handle );
}

void MSNProtocol::slotBlockContact( QString handle ) const
{
	if(m_allowList.contains(handle))
		m_notifySocket->removeContact( handle, 0, AL);
	else if(!m_blockList.contains(handle))
		m_notifySocket->addContact( handle, handle, 0, BL );
}

void MSNProtocol::addContact( const QString &userID , KopeteMetaContact *m, const QString &group) 
{
	if( isConnected() )
	{
		m_addWizard_metaContact=m;

		if(m && !m->groups().isEmpty())
		{
			QStringList gprs=m->groups().toStringList();
			for( QStringList::ConstIterator it = gprs.begin(); it != gprs.end(); ++it )
			{
				int g = groupNumber( *it );
				if(g!=-1) 
				{
					m_notifySocket->addContact( userID, userID, g, FL );
				}
				else
				{
					tmp_addToNewGroup << QPair<QString,QString>(userID,*it);
					addGroup(*it);
				}
			}
		}
		else
		{
			int g = groupNumber( group );
			if(g==-1) {
				tmp_addToNewGroup << QPair<QString,QString>(userID,group);
				addGroup(group);
			}
			else {
				m_notifySocket->addContact( userID, userID, g, FL ); 
			}
		}
	}
}



QStringList MSNProtocol::groups() const
{
	QStringList result;
	QMap<uint, QString>::ConstIterator it;
	for( it = m_groupList.begin(); it != m_groupList.end(); ++it )
		result.append( *it );

//	kdDebug() << "MSNProtocol::groups(): " << result.join(", " ) << endl;
	return result;
}

MSNProtocol *MSNProtocol::s_protocol = 0L;

MSNProtocol* MSNProtocol::protocol()
{
	return s_protocol;
}

int MSNProtocol::groupNumber( const QString &group) const
{
	//if group is null, add them to the first available group
	if(group==QString::null)
		return m_groupList.begin().key();

	QMap<uint, QString>::ConstIterator it;
	for( it = m_groupList.begin(); it != m_groupList.end(); ++it )
	{
		if( *it == group )
			return it.key();
	}
	return -1;
}

QString MSNProtocol::groupName( uint num ) const
{
	if( m_groupList.contains( num ) )
		return m_groupList[ num ];
	else
		return QString::null;
}

void MSNProtocol::slotGroupListed( QString groupName, uint group )
{
	if( !m_groupList.contains( group ) )
	{
		//kdDebug() << "MSNProtocol::slotGroupListed: Appending group " << group
		//	<< ", with name " << groupName << endl;
		m_groupList.insert( group, groupName );
	}
}

void MSNProtocol::slotGroupAdded( QString groupName, uint /* serial */, uint group )
{
	if( !m_groupList.contains( group ) )
	{
		//kdDebug() << "MSNProtocol::slotGroupAdded: Appending group " << group
		//	<< ", with name " << groupName << endl;
		m_groupList.insert( group, groupName );
	}
	if(tmp_addToNewGroup.count()>0)
	{
		for ( QValueList<QPair<QString,QString> >::Iterator it = tmp_addToNewGroup.begin(); it != tmp_addToNewGroup.end(); ++it)
		{
			if((*it).second==groupName)
			{
				kdDebug() << "MSNProtocol::slotGroupAdded : Adding to new group: " << (*it).first <<  endl;
				m_notifySocket->addContact( (*it).first, (*it).first, group, FL );
			}
		}
		tmp_addToNewGroup.clear();
	}
}

void MSNProtocol::slotGroupRenamed( QString groupName, uint /* serial */,
	uint group )
{
	if( m_groupList.contains( group ) )
	{
		// each contact has a groupList, so change it
		QMap<QString, MSNContact*>::ConstIterator it;

		for( it = m_contacts.begin(); it != m_contacts.end(); ++it )
		{
			if( ( *it )->groups().contains( m_groupList[ group ] ) )
			{
				( *it )->addedToGroup( groupName );
				( *it )->removedFromGroup( m_groupList[ group ] );
			}
		}

		m_groupList[ group ] = groupName;
	}
}

void MSNProtocol::slotGroupRemoved( uint /* serial */, uint group )
{
	if( m_groupList.contains( group ) )
		m_groupList.remove( group );
}

void MSNProtocol::addGroup( const QString &groupName , const QString& contactToAdd )
{
	if(!contactToAdd.isNull())
		tmp_addToNewGroup << QPair<QString,QString>(contactToAdd,groupName);

	if( !( groups().contains( groupName ) ) )
		m_notifySocket->addGroup( groupName );
}

void MSNProtocol::renameGroup( const QString &oldGroup,
	const QString &newGroup )
{
	int g = groupNumber( oldGroup );
	if( g != -1 )
		m_notifySocket->renameGroup( newGroup, g );
}

void MSNProtocol::removeGroup( const QString &name )
{
	int g = groupNumber( name );
	if( g != -1 )
		m_notifySocket->removeGroup( g );
}

MSNProtocol::Status MSNProtocol::status() const
{
	return m_status;
}

MSNProtocol::Status MSNProtocol::convertStatus( QString status )
{
	if( status == "NLN" )
		return NLN;
	else if( status == "FLN" )
		return FLN;
	else if( status == "HDN" )
		return HDN;
	else if( status == "PHN" )
		return PHN;
	else if( status == "LUN" )
		return LUN;
	else if( status == "BRB" )
		return BRB;
	else if( status == "AWY" )
		return AWY;
	else if( status == "BSY" )
		return BSY;
	else if( status == "IDL" )
		return IDL;
	else
		return FLN;
}

void MSNProtocol::slotContactList( QString handle, QString publicName,
	QString group, QString list )
{
	// On empty lists handle might be empty, ignore that
	if( handle.isEmpty() )
		return;

	QStringList groups;
	groups = QStringList::split(",", group, false );
	if( list == "FL" )
	{
		KopeteMetaContact *m = KopeteContactList::contactList()->findContact( id(), QString::null, handle );

		if( m )
		{
			//Contact exists, update data.
			//Merging difference between server contact list and KopeteContact's contact list into MetaContact's contact-list
			MSNContact *c = static_cast<MSNContact*>(m->findContact( id(), QString::null, handle ));
			c->setMsnStatus(FLN);
			c->setDisplayName(publicName);
			QStringList contactGroups=c->groups();

			QStringList serverGroups;
			for( QStringList::Iterator it = groups.begin();
				it != groups.end(); ++it )
			{
				serverGroups.append( groupName( (*it).toUInt() ) );
			}

			for( QStringList::ConstIterator it = serverGroups.begin(); it != serverGroups.end(); ++it )
			{
				QString serverGroup=*it;
				if(!contactGroups.contains(serverGroup))
				{
					c->addedToGroup(serverGroup);
					m->addToGroup(KopeteContactList::contactList()->getGroup(serverGroup));
				}
			}
			for( QStringList::ConstIterator it = contactGroups.begin(); it != contactGroups.end(); ++it )
			{
				QString cGroup=*it;
				if(!serverGroups.contains(cGroup))
				{
					c->removedFromGroup(cGroup);
					m->removeFromGroup(KopeteContactList::contactList()->getGroup(cGroup));
				}
			}
		}
		else
		{
			m=new KopeteMetaContact();

			MSNContact *msnContact = new MSNContact( handle,
				publicName, QString::null, m );
			connect( msnContact, SIGNAL( contactDestroyed( KopeteContact * ) ),
				SLOT( slotContactDestroyed( KopeteContact * ) ) );
//			m_metaContacts.insert( m, msnContact );

			msnContact->setMsnStatus(FLN);

			for( QStringList::Iterator it = groups.begin();
				it != groups.end(); ++it )
			{
				msnContact->addedToGroup( groupName( (*it).toUInt() ) );
				m->addToGroup(KopeteContactList::contactList()->getGroup(groupName( (*it).toUInt() ) ) );
			}
			m->addContact( msnContact );
			KopeteContactList::contactList()->addMetaContact(m);

			m_contacts.insert( msnContact->msnId(), msnContact );
			
		}
	}
	else if( list == "BL" )
	{
		if( !m_blockList.contains( handle ) )
			m_blockList.append( handle );
		if( m_contacts.contains( handle ) )
			m_contacts[ handle ]->setBlocked( true );
	}
	else if( list == "AL" )
	{
		if( !m_allowList.contains( handle ) )
			m_allowList.append( handle );
		if( m_contacts.contains( handle ) )
			m_contacts[ handle ]->setAllowed( true );

	}
	else if( list == "RL" )
	{
		// search for new Contacts
		// FIXME: Users in the allow list or block list now never trigger the
		// 'new user' dialog, which makes it impossible to add those here.
		// Not necessarily bad, but the usability effects need more thought
		// before I declare it good :-)
		if( !m_allowList.contains( handle ) && !m_blockList.contains( handle ) )
		{
			kdDebug() << "MSNProtocol: Contact not found in list!" << endl;

			NewUserImpl *authDlg = new NewUserImpl(0);
			authDlg->setHandle(handle, publicName);
			connect( authDlg, SIGNAL(addUser( QString )), this, SLOT(slotAddContact( QString )));
			connect( authDlg, SIGNAL(blockUser( QString )), this, SLOT(slotBlockContact( QString )));
			authDlg->show();
		}

		if(m_contacts.contains( handle ))
			m_contacts[ handle ]->setReversed( true );
	}
}

void MSNProtocol::slotContactRemoved( QString handle, QString list,
	uint /* serial */, uint group )
{
	QString gn = groupName( group );

	if( list == "BL" )
	{
		m_blockList.remove(handle);
		if(!m_allowList.contains(handle))
			m_notifySocket->addContact( handle, handle, 0, AL );

	}
	if( list == "AL" )
	{
		m_allowList.remove(handle);
		if(!m_blockList.contains(handle))
			m_notifySocket->addContact( handle, handle, 0, BL );

	}

	MSNContact *c=contact(handle);
	if( c )
	{
		if( list == "RL" )
		{
			// Contact is removed from the reverse list
			// only MSN can do this, so this is currently not supported
			c->setReversed( false );
/*
			InfoWidget *info = new InfoWidget(0);
			info->title->setText("<b>" + i18n( "Contact removed!" ) +"</b>" );
			QString dummy;
			dummy = "<center><b>" + imContact->getPublicName() + "(" +imContact->getHandle()  +")</b></center><br>";
			dummy += i18n("has removed you from his contact list!") + "<br>";
			dummy += i18n("This contact is now removed from your contact list");
			info->infoText->setText(dummy);
			info->setCaption("KMerlin - Info");
			info->show();
*/
			
		}
		else if( list == "FL" )
		{
			// Contact is removed from the FL list, remove it from the group
			c->removedFromGroup( gn );
		}
		else	if( list == "BL" )
		{
			c->setBlocked(false);
		}
		else	if( list == "AL" )
		{
			c->setAllowed(false);
		}

		if( c->groups().isEmpty()  && !c->isMoving())
		{
			kdDebug() << "MSNProtocol::slotContactRemoved : contact removed from each group, delete contact" << endl;
			delete c;
			m_contacts.remove( handle );
		} 
	}
}

void MSNProtocol::slotContactAdded( QString handle, QString publicName,
	QString list, uint /* serial */, uint group )
{
	QString gn = groupName( group );

	if( list == "FL" )
	{
		bool new_contact=false;
		if(!m_contacts.contains( handle ))
		{
			KopeteMetaContact *m = KopeteContactList::contactList()->findContact( this->id(), QString::null, handle );
			if(m)
			{
				MSNContact *c = static_cast<MSNContact*>(m->findContact( this->id(), QString::null, handle ));
				m_contacts.insert( c->msnId(), c );
			}
			else
			{
				new_contact=true;

				if(m_addWizard_metaContact)
					m=m_addWizard_metaContact;
				else
					m=new KopeteMetaContact();

				MSNContact *c = new MSNContact( handle, publicName, gn, m );
				connect( c, SIGNAL( contactDestroyed( KopeteContact * ) ),
					SLOT( slotContactDestroyed( KopeteContact * ) ) );
				//m_metaContacts.insert( m, c );

				if(!m_addWizard_metaContact)
				{
					m->addToGroup(KopeteContactList::contactList()->getGroup(gn));
					KopeteContactList::contactList()->addMetaContact(m);
				}
				m->addContact( c);
				c->setMsnStatus(FLN);

				m_addWizard_metaContact=0L;

				m_contacts.insert( c->msnId(), c );
			}
		}
		if(!new_contact)
		{
			MSNContact *c=m_contacts[ handle ];
			if(c->msnStatus()==UNK)
				c->setMsnStatus(FLN);

			c->addedToGroup( gn );
			if(c->metaContact()->isTemporary())
				c->metaContact()->setTemporary(false,KopeteContactList::contactList()->getGroup(gn));
		}
		
		if(!m_allowList.contains(handle) && !m_blockList.contains(handle))
			m_notifySocket->addContact( handle, handle, 0, AL );
	}
	if( list == "BL" )
	{
		if( m_contacts.contains( handle ) )
			m_contacts[ handle ]->setBlocked( true );
		if( !m_blockList.contains( handle ) )
			m_blockList.append( handle );
	}
	if( list == "AL" )
	{
		if( m_contacts.contains( handle ) )
			m_contacts[ handle ]->setAllowed( true );
		if( !m_allowList.contains( handle ) )
			m_allowList.append( handle );
	}
	if( list == "RL" )
	{
		if(!m_contacts.contains( handle ))
		{
			NewUserImpl *authDlg = new NewUserImpl(0);
			authDlg->setHandle(handle, publicName);
			connect( authDlg, SIGNAL(addUser( QString )), this, SLOT(slotAddContact( QString )));
			connect( authDlg, SIGNAL(blockUser( QString )), this, SLOT(slotBlockContact( QString )));
			authDlg->show();
		}
		else
		{
			m_contacts[ handle ]->setReversed( true );
		}
	}
}

void MSNProtocol::slotPublicNameChanged( QString publicName)
{
	if( publicName != m_publicName )
	{
		if( m_publicNameSyncMode & SyncFromServer )
		{
			m_publicName = publicName;
			m_publicNameSyncMode = SyncBoth;

			m_myself->setDisplayName(publicName);

			actionStatusMenu->popupMenu()->changeTitle( m_menuTitleId,
				*( statusBarIcon->pixmap() ), QString( m_publicName+" ("+ m_msnId +")" ));

			// Also sync the config file
			mPrefs->setPublicName(m_publicName);
		}
		else
		{
			// Check if name differs, and schedule sync if needed
			if( m_publicNameSyncMode & SyncToServer )
				m_publicNameSyncNeeded = true;
			else
				m_publicNameSyncNeeded = false;
		}
	}
} 

void MSNProtocol::setPublicName( const QString &publicName )
{
	kdDebug() << "MSNProtocol::setPublicName: Setting name to "
		<< publicName << "..." << endl;

	if(m_notifySocket)
	{
		m_notifySocket->changePublicName( publicName );
	}
}

void MSNProtocol::slotCreateChat( QString address, QString auth)
{
	slotCreateChat( 0L, address, auth, m_msgHandle, m_msgHandle );
}

void MSNProtocol::slotCreateChat( QString ID, QString address, QString auth,
	QString handle, QString  publicName  )
{
	handle = handle.lower();

	kdDebug() << "MSNProtocol::slotCreateChat: Creating chat for " <<
		handle << endl;

	if( !m_contacts.contains( handle ) )
	{
		KopeteMetaContact *m = KopeteContactList::contactList()->findContact( id(), QString::null, handle );
		if(m)
		{
			KopeteContact *c=m->findContact( id(), QString::null, handle );
			MSNContact *msnContact=static_cast<MSNContact*>(c);
			m_contacts.insert( handle, msnContact );
		}
		else
		{
			m=new KopeteMetaContact();
			m->setTemporary(true);
			QString protocolid = this->id();

			MSNContact *msnContact = new MSNContact( handle, publicName, QString::null, m );
			connect( msnContact, SIGNAL( contactDestroyed( KopeteContact * ) ),
				SLOT( slotContactDestroyed( KopeteContact * ) ) );
			//m_metaContacts.insert( m, msnContact );

			m->addContact( msnContact);
			KopeteContactList::contactList()->addMetaContact(m);

			m_contacts.insert( msnContact->msnId(), msnContact );
		}
	}

	KopeteContact *c = contact( handle );
	if ( c && m_myself )
	{
		KopeteContactPtrList chatmembers;
		chatmembers.append(c);

		KopeteMessageManager *_manager =
			kopeteapp->sessionFactory()->findKopeteMessageManager( m_myself,
				chatmembers, this  );
		MSNMessageManager *manager =
			dynamic_cast<MSNMessageManager*>( _manager );

		if( !manager )
		{
			manager = new MSNMessageManager( m_myself, chatmembers,
				QString( "msn_logs/" + handle + ".log" ) );
		}
		manager->createChat( handle, address, auth, ID );

		//		m_switchBoardSockets.insert( manager, chatService );

		if( mPrefs->openWindow() || !ID )
		{
			manager->readMessages();
		}
	}
}

void MSNProtocol::slotStartChatSession( QString handle )
{
	// First create a message manager, because we might get an existing
	// manager back, in which case we likely also have an active switchboard
	// connection to reuse...
	KopeteContact *c = contact( handle );
	if( isConnected() && c && m_myself && handle != m_msnId )
	{
		KopeteContactPtrList chatmembers;
		chatmembers.append(c);

		KopeteMessageManager *_manager = kopeteapp->sessionFactory()->findKopeteMessageManager( m_myself, chatmembers, this  );
		MSNMessageManager *manager= dynamic_cast<MSNMessageManager*>(_manager);
		if(!manager)
		{
			manager=new MSNMessageManager(m_myself,chatmembers, QString( "msn_logs/" + handle + ".log" ));
		}

		if(manager->service())
		{
			kdDebug() << "MSNProtocol::slotStartChatSession: "
				<< "Reusing existing switchboard connection" << endl;

			manager->readMessages();
		}
		else
		{
			kdDebug() << "MSNProtocol::slotStartChatSession: "
				<< "Creating new switchboard connection" << endl;

       	//FIXME: what's happend when the user try to open two socket in the same time????  can the m_msgHandle be altered??
			m_msgHandle = handle;
			m_notifySocket->createChatSession();
		}
	}
}

void MSNProtocol::slotChangePublicName()
{
	bool ok;
	QString name = KLineEditDlg::getText(
		i18n( "Change Nickname - MSN Plugin - Kopete" ),
		i18n( "Enter the new public name by which you want to be "
			"visible to your friends on MSN." ),
		m_publicName, &ok );

	if( ok )
	{
		// For some stupid reasons the public name is not allowed to contain
		// the text 'msn'. It would result in an error 209 from the server.
		if( name.contains( "msn", false ) )
		{
			KMessageBox::error( 0L,
				i18n( "Your display name is "
					"not allowed to contain the text 'MSN'.\n"
					"Your display name has not been changed." ),
				i18n( "Change Nickname - MSN Plugin - Kopete" ) );
			return;
		}

		if( isConnected() )
			setPublicName( name );
		else
		{
			// Bypass the protocol, it doesn't work, call the slot
			// directly. Upon connect the name will be synced.
			// FIXME: Use a single code path instead!
			slotPublicNameChanged( name );
			m_publicNameSyncMode = SyncToServer;
		}
	}
}

void MSNProtocol::slotDebugRawCommand()
{
	if ( !isConnected() )
	{
		return;
	}

	MSNDebugRawCmdDlg *dlg = new MSNDebugRawCmdDlg( 0L );
	int result = dlg->exec();
	if( result == QDialog::Accepted && m_notifySocket )
	{
		m_notifySocket->sendCommand( dlg->command(), dlg->params(),
					     dlg->addId() );
	}
	delete dlg;
}

void MSNProtocol::slotNotifySocketClosed( int /*state*/ )
{
	kdDebug() << "MSNProtocol::slotNotifySocketClosed" << endl;
	//FIXME: Kopete crash when i show this message box...
/*	if ( state == 0x10 ) // connection died unexpectedly
	{
		KMessageBox::error( kopeteapp->mainWindow(), i18n( "Connection with the MSN server was lost unexpectedly.\nIf you are unable to reconnect, please try again later." ), i18n( "Connection lost - MSN Plugin - Kopete" ) );
	}*/
	//m_notifySocket->deleteLater();
	delete m_notifySocket;
	m_notifySocket=0l;
	mIsConnected = false;
	statusBarIcon->setPixmap(offlineIcon);
	m_openInboxAction->setEnabled(false);
	kdDebug() << "MSNProtocol::slotNotifySocketClosed - done" << endl;
}

void MSNProtocol::slotContactDestroyed( KopeteContact *c )
{
	//kdDebug() << "MSNProtocol::slotContactDestroyed " << endl;
	for ( QMap<QString,MSNContact*>::iterator it = m_contacts.begin(); it!=m_contacts.end(); ++it  )
	{
		if(*it == c)
			m_contacts.remove(it);
	}
}

KActionCollection * MSNProtocol::customChatActions(KopeteMessageManager * manager)
{
	MSNMessageManager *msnMM= dynamic_cast<MSNMessageManager*>(manager);
	if(!msnMM)
		return 0L;

	return msnMM->chatActions();
}

MSNContact *MSNProtocol::contact( const QString &handle )
{
	if(m_contacts.contains(handle))
	{
		return m_contacts[handle];
	}
	return 0L;
}

void MSNProtocol::slotPreferencesSaved()
{
	m_password   = mPrefs->password();
//	m_publicName = mPrefs->publicName();

	if(m_msnId != mPrefs->msnId())
	{
		m_msnId  = mPrefs->msnId();
		if(m_myself)
			m_myself->setMsnId(m_msnId);
	}
}


#include "msnprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

