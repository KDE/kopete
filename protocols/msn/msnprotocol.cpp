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

#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <ksimpleconfig.h>
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
#include "msnswitchboardsocket.h"
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
	mIsConnected = false;
	m_notifySocket = 0L;

	m_identity = new MSNIdentity( this, "m_identity" );

	kdDebug() << "MSNProtocol::MSNProtocol: MSN Plugin Loading" << endl;

	initIcons();
	new MSNPreferences( "msn_protocol", this );

	kdDebug() << "MSN Protocol Plugin: Creating Status Bar icon\n";
	statusBarIcon = new StatusBarIcon();

	// FIXME: Duplicated, needs to get proper code!
	m_msnId      = KGlobal::config()->readEntry( "UserID", "" );
	m_password   = KGlobal::config()->readEntry( "Password", "" );
	m_publicName = KGlobal::config()->readEntry( "Nick", "Kopete User" );
	m_publicNameSyncMode = SyncFromServer;
	m_publicNameSyncNeeded = false;
  m_msgQueued=0L;

	initActions();

	QObject::connect(statusBarIcon, SIGNAL(rightClicked(const QPoint&)), this, SLOT(slotIconRightClicked(const QPoint&)));
	statusBarIcon->setPixmap( offlineIcon );

	KConfig *cfg = KGlobal::config();
	cfg->setGroup( "MSN" );

	if( ( cfg->readEntry( "UserID", "" ).isEmpty() ) ||
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
			emptyText, emptyCaption );
	}

	// FIXME: Is 'self' supposed to be a KopeteMetaContact? I guess so.
	// Fix that. - Martijn
	m_myself = new MSNContact( protocolId, cfg->readEntry( "UserID", "" ),
		cfg->readEntry( "Nick", "" ), "", 0L );

	if ( cfg->readBoolEntry( "AutoConnect", "0" ) )
		Connect();
}

MSNProtocol::~MSNProtocol()
{
	m_groupList.clear();
	m_allowList.clear();
	m_blockList.clear();

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
	kdDebug() << "MSN Protocol: Unloading...\n";

	Disconnect();

	// Delete all MSNContacts from the meta contacts. Not doing this will
	// cause all kinds of memory overrun crashes ;-)
	while( m_metaContacts.count() )
		delete ( QPtrDictIterator<MSNContact>( m_metaContacts ) ).current();

	m_metaContacts.clear();

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
	if( isConnected() )
	{
		kdDebug() << "MSN Plugin: Ignoring Connect request "
			<< "(Already Connected)" << endl;
		return;
	}

	KGlobal::config()->setGroup( "MSN" );
	m_msnId      = KGlobal::config()->readEntry( "UserID", "" );
	m_password   = KGlobal::config()->readEntry( "Password", "" );

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
	connect( m_notifySocket, SIGNAL( statusChanged( QString ) ),
				this, SLOT( slotStateChanged( QString ) ) );
	connect( m_notifySocket,
		SIGNAL( contactStatusChanged( const QString &, const QString &, MSNProtocol::Status ) ),
		this,
		SLOT( slotContactStatusChanged( const QString &, const QString &, MSNProtocol::Status ) ) );
	connect( m_notifySocket,
		SIGNAL( contactList( QString, QString, QString, QString ) ),
		this, SLOT( slotContactList( QString, QString, QString, QString ) ) );
	connect( m_notifySocket,
		SIGNAL( contactAdded( QString, QString, QString, uint, uint ) ),
		this,
		SLOT( slotContactAdded( QString, QString, QString, uint, uint ) ) );
	connect( m_notifySocket,
		SIGNAL( contactRemoved( QString, QString, uint, uint ) ),
		this,
		SLOT( slotContactRemoved( QString, QString, uint, uint ) ) );
	connect( m_notifySocket, SIGNAL( statusChanged( QString ) ),
		this, SLOT( slotStatusChanged( QString ) ) );
	connect( m_notifySocket,
		SIGNAL( contactStatus( QString, QString, QString ) ),
		this, SLOT( slotContactStatus( QString, QString, QString ) ) );
	connect( m_notifySocket,
		SIGNAL( onlineStatusChanged( MSNSocket::OnlineStatus ) ),
		this, SLOT( slotOnlineStatusChanged( MSNSocket::OnlineStatus ) ) );
	connect( m_notifySocket, SIGNAL( publicNameChanged( QString, QString ) ),
		this, SLOT( slotPublicNameChanged( QString, QString ) ) );
	connect( m_notifySocket,
		SIGNAL( invitedToChat( QString, QString, QString, QString, QString ) ),
		this,
		SLOT( slotCreateChat( QString, QString, QString, QString, QString ) ) );
	connect( m_notifySocket, SIGNAL( startChat( QString, QString ) ),
		this, SLOT( slotCreateChat( QString, QString ) ) );

	connect( m_notifySocket, SIGNAL( socketClosed( int ) ),
		this, SLOT( slotNotifySocketClosed( int ) ) );

	m_notifySocket->connect( m_password );
	statusBarIcon->setMovie( connectingIcon );
}

void MSNProtocol::Disconnect()
{
	if (m_notifySocket)
		m_notifySocket->disconnect();

	//delete m_notifySocket;
	m_notifySocket->deleteLater();
	m_notifySocket = 0L;

	m_switchBoardSockets.setAutoDelete( true );
	m_switchBoardSockets.clear();
	m_switchBoardSockets.setAutoDelete( false );
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
	MSNContact *c = m_metaContacts.find( metaContact );

	if( c )
	{
		stream << c->id() << c->displayName() << c->groups().join( "," );
		return true;
	}
	else
	{
		return false;
	}
}

void MSNProtocol::deserialize( KopeteMetaContact *metaContact,
	const QStringList &strList )
{
	kdDebug() << "MSNProtocol::deserialize: " << metaContact->displayName()
		<< ", [ " << strList.join( ", " ) << " ]" << endl;

	QString protocolId = this->id();

	uint idx = 0;
	while( idx < strList.size() )
	{
		QString passport    = strList[ idx ];
		QString displayName = strList[ idx + 1 ];
		QStringList groups  = QStringList::split( ",", strList[ idx + 2 ] );

		kdDebug() << "new MSNContact( " << protocolId << ", " << passport
			<< ", " << displayName << ", " << groups.first()
			<< ", " << metaContact->displayName() << endl;

		// Create MSN contact
		// FIXME: I think this should go in a single method, as it is
		// duplicated everywhere now - Martijn
		MSNContact *c = new MSNContact( protocolId, passport, displayName,
			groups.first(), metaContact );
		connect( c, SIGNAL( contactDestroyed( KopeteContact * ) ),
			SLOT( slotContactDestroyed( KopeteContact * ) ) );
		m_metaContacts.insert( metaContact, c );

		QStringList::Iterator it = groups.begin();
		if( it != groups.end() )
			++it; // Skip the first item as it was passed to the ctor already
		for( ; it != groups.end(); ++it )
			c->addToGroup( groupName( (*it).toUInt() ) );

		metaContact->addContact( c, c->groups() );

		// FIXME: Try to remove this
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

void MSNProtocol::initActions()
{
	actionGoOnline = new KAction ( i18n("Go O&nline"), "msn_online", 0, this, SLOT(slotGoOnline()), this, "actionMSNConnect" );
	actionGoOffline = new KAction ( i18n("Go &Offline"), "msn_offline", 0, this, SLOT(slotGoOffline()), this, "actionMSNConnect" );
	actionGoAway = new KAction ( i18n("Set &Away"), "msn_away", 0, this, SLOT(slotGoAway()), this, "actionMSNConnect" );
	m_renameAction = new KAction ( i18n( "&Change Nickname..." ),
		QString::null, 0, this, SLOT( slotChangePublicName() ),
		this, "m_renameAction" );
	actionStatusMenu = new KActionMenu( "MSN", this );

	m_debugMenu = new KActionMenu( "Debug", this );
	m_debugRawCommand = new KAction( i18n( "Send &Raw Command..." ), 0,
		this, SLOT( slotDebugRawCommand() ), this, "m_debugRawCommand" );

	m_menuTitleId = actionStatusMenu->popupMenu()->insertTitle(
		*( statusBarIcon->pixmap() ),
		i18n( "%1 (%2)" ).arg( m_publicName ).arg( m_msnId ) );
	actionStatusMenu->insert( actionGoOnline );
	actionStatusMenu->insert( actionGoOffline );
	actionStatusMenu->insert( actionGoAway );
	actionStatusMenu->popupMenu()->insertSeparator();
	actionStatusMenu->insert( m_renameAction );

	actionStatusMenu->popupMenu()->insertSeparator();
	actionStatusMenu->insert( m_debugMenu );

	m_debugMenu->insert( m_debugRawCommand );

	actionStatusMenu->plug( kopeteapp->systemTray()->contextMenu(), 1 );
}

void MSNProtocol::slotIconRightClicked( const QPoint& /* point */ )
{
	KGlobal::config()->setGroup("MSN");
	QString handle = KGlobal::config()->readEntry("UserID", i18n("(User ID not set)"));

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
	kdDebug() << "MSN Plugin: Going Online" << endl;
	if (!isConnected() )
		Connect();
	else
		m_notifySocket->setStatus( NLN );
}

void MSNProtocol::slotGoOffline()
{
	Disconnect();
}

void MSNProtocol::slotGoAway()
{
	kdDebug() << "MSN Plugin: Going Away" << endl;
	if (!isConnected() )
		Connect();
	m_notifySocket->setStatus( AWY );
}

void MSNProtocol::slotOnlineStatusChanged( MSNSocket::OnlineStatus status )
{
	mIsConnected = status == MSNSocket::Connected;
	if ( mIsConnected )
	{
		kopeteapp->sessionFactory()->cleanSessions(this);
		// Sync public name when needed
		if( m_publicNameSyncNeeded )
		{
			kdDebug() << "MSNProtocol::slotConnected: Syncing public name to "
				<< m_publicName << endl;
			setPublicName( m_publicName );
			m_publicNameSyncNeeded = false;
		}
		else
		{
			kdDebug() << "MSNProtocol::slotConnected: Leaving public name as "
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
		QStringList localgroups = (kopeteapp->contactList()->groups()) ;
		QStringList servergroups = groups();
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
					kdDebug() << "MSN Plugin: Sync: Local group " << localgroup << " doesn't exist on server!" << endl;
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

		QIntDictIterator<KopeteMessageManager> kmmIt( kopeteapp->sessionFactory()->protocolSessions( this ) );
		for ( ; kmmIt.current() ; ++kmmIt )
		{
			kmmIt.current()->slotSendEnabled(false);
		}

		// FIXME!!!!!!!
		QMap<QString, MSNContact*>::Iterator it;
		for ( it = m_contacts.begin(); it != m_contacts.end() ; ++it)
		{
			//delete *it;
			(*it)->setMsnStatus( MSNProtocol::FLN );
			//m_contacts.remove( it );
		}

		m_allowList.clear();
		m_blockList.clear();
		m_groupList.clear();

		mIsConnected = false;
		statusBarIcon->setPixmap(offlineIcon);

		m_status = FLN;

		// Reset flags. They can't be set in the connect method, because
		// offline changes might have been made before. Instead the c'tor
		// sets the defaults, and the disconnect slot resets those defaults
		// FIXME: Can't we share this code?
		m_publicNameSyncMode = SyncFromServer;
	}
}

void MSNProtocol::slotStateChanged( QString status )
{
	m_status = convertStatus( status );

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
			statusBarIcon->setPixmap(awayIcon);
			break;
		case IDL:
			statusBarIcon->setPixmap(awayIcon);
			break;
		case PHN:
			statusBarIcon->setPixmap(awayIcon);
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

void MSNProtocol::addToContactList( MSNContact *c, const QString &group )
{
	kdDebug() << "MSNProtocol::addToContactList: adding " << c->msnId()
		<< " to group " << group << endl;
	kopeteapp->contactList()->addContact( c, group );
	m_contacts.insert( c->msnId(), c );
}

void MSNProtocol::slotAddContact( QString handle )
{
	addContact( handle );
}

void MSNProtocol::slotBlockContact( QString handle ) const
{
	blockContact( handle );
}

void MSNProtocol::blockContact( QString handle ) const
{
	m_notifySocket->removeContact( handle, 0, AL);
	m_notifySocket->addContact( handle, handle, 0, BL );
}

void MSNProtocol::addContact( const QString &userID )
{
	if( isConnected() )
	{
		m_notifySocket->addContact( userID, userID, 0, AL );
		m_notifySocket->addContact( userID, userID, 0, FL );
	}
}

void MSNProtocol::removeContact( const MSNContact *c ) const
{
	QStringList list;
	const QString id = c->msnId();
	if( m_contacts.contains( id ) )
		list = m_contacts[ id ]->groups();
	else
		return;

	for( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
	{
		m_notifySocket->removeContact( id, groupNumber( (*it).latin1() ),
			FL );
	}

	if( m_contacts[ id ]->isBlocked() )
		m_notifySocket->removeContact( id, 0, BL );
}

void MSNProtocol::removeFromGroup( const MSNContact *c,
	const QString &group ) const
{
	int g = groupNumber( group );
	if( g != -1 )
		m_notifySocket->removeContact( c->msnId(), g, FL );
}

void MSNProtocol::moveContact( const MSNContact *c,
	const QString &oldGroup, const QString &newGroup ) const
{
	int og = groupNumber( oldGroup );
	int ng = groupNumber( newGroup );

	kdDebug() << "MSNProtocol::moveContact: Moving contact "
		<< c->msnId() << " from group #" << og << " to #" << ng << endl;

	if( og != -1 && ng != -1 )
	{
		m_notifySocket->addContact( c->msnId(), c->displayName(), ng,
			FL);
		m_notifySocket->removeContact( c->msnId(), og, FL );
	}
}

void MSNProtocol::copyContact( const MSNContact *c,
	const QString &newGroup ) const
{
	int g = groupNumber( newGroup );
	if( g != -1 )
	{
		m_notifySocket->addContact( c->msnId(), c->displayName(), g,
			FL);
	}
}

QStringList MSNProtocol::groups() const
{
	QStringList result;
	QMap<uint, QString>::ConstIterator it;
	for( it = m_groupList.begin(); it != m_groupList.end(); ++it )
		result.append( *it );

	kdDebug() << "MSNProtocol::groups(): " << result.join(", " ) << endl;
	return result;
}

const MSNProtocol* MSNProtocol::s_protocol = 0L;

const MSNProtocol* MSNProtocol::protocol()
{
	return s_protocol;
}

int MSNProtocol::groupNumber( const QString &group ) const
{
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
		kdDebug() << "MSNProtocol::slotGroupListed: Appending group " << group
			<< ", with name " << groupName << endl;
		m_groupList.insert( group, groupName );
	}
}

void MSNProtocol::slotGroupAdded( QString groupName, uint /* serial */,
	uint group )
{
	if( !m_groupList.contains( group ) )
	{
		kdDebug() << "MSNProtocol::slotGroupAdded: Appending group " << group
			<< ", with name " << groupName << endl;
		m_groupList.insert( group, groupName );
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
				( *it )->removeFromGroup( m_groupList[ group ] );
				( *it )->addToGroup( groupName );
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

void MSNProtocol::addGroup( const QString &groupName )
{
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

void MSNProtocol::slotContactStatus( QString handle, QString publicName,
	QString status )
{
	kdDebug() << "MSNProtocol::slotContactStatus: " << handle << " (" <<
		publicName << ") has status " << status << endl;

	if( m_contacts.contains( handle ) )
	{
		m_contacts[ handle ]->setMsnStatus( convertStatus( status ) );
		m_contacts[ handle ]->setDisplayName( publicName );
	}
}

void MSNProtocol::slotContactStatusChanged( const QString &handle,
	const QString &publicName, MSNProtocol::Status status )
{
	kdDebug() << "MSNProtocol::slotContactStatusChanged: " << handle << " (" <<
		publicName << ") has status " << status << endl;

	if( m_contacts.contains( handle ) )
	{
		m_contacts[ handle ]->setMsnStatus( status );
		m_contacts[ handle ]->setDisplayName( publicName );

		if( status == FLN )
		{
			bool done;
			do
			{
				done = true;
				QPtrDictIterator<MSNSwitchBoardSocket> it( m_switchBoardSockets );
				for( ; m_switchBoardSockets.count() && it.current(); ++it )
				{
					if( ( *it ).chatMembers().contains( handle ) )
					{
						kdDebug() << "MSNProtocol::slotContactStatusChanged: "
							<< "Removing stale switchboard from offline user "
							<< handle << endl;
					(*it).userLeftChat(handle);
						done = false;
						break;
					}
				}
			} while( !done );
		}
	}
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
		KopeteContactList *l = KopeteContactList::contactList();
		KopeteMetaContact *m = l->findContact( id(), QString::null, handle );
		KopeteContact *c = m->findContact( id(), QString::null, handle );

		if( c )
		{
			// Existing contact, update data
			// FIXME: TODO!
			kdDebug() << "MSNProtocol::slotContactList: Not implemented: "
				<< "Meta contact already contains contact " << handle
				<< "???" << endl;
		}
		else
		{
			QString protocolid = this->id();

			MSNContact *msnContact = new MSNContact( protocolid, handle,
				publicName, QString::null, m );
			connect( msnContact, SIGNAL( contactDestroyed( KopeteContact * ) ),
				SLOT( slotContactDestroyed( KopeteContact * ) ) );
			m_metaContacts.insert( m, msnContact );

			for( QStringList::Iterator it = groups.begin();
				it != groups.end(); ++it )
			{
				msnContact->addToGroup( groupName( (*it).toUInt() ) );
			}
			c = msnContact;
			m->addContact( c, msnContact->groups() );

			// FIXME: Try to remove this
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
			authDlg->setHandle(handle);
			connect( authDlg, SIGNAL(addUser( QString )), this, SLOT(slotAddContact( QString )));
			connect( authDlg, SIGNAL(blockUser( QString )), this, SLOT(slotBlockContact( QString )));
			authDlg->show();
		}
	}
}

void MSNProtocol::slotContactRemoved( QString handle, QString list,
	uint /* serial */, uint group )
{
	QString gn = groupName( group );
	if( gn.isNull() )
		gn = i18n( "Unknown" );

	if( m_contacts.contains( handle ) )
	{
		if( group )
			m_contacts[ handle ]->removeFromGroup( gn );

		else if( list == "RL" )
		{
/*
			// Contact is removed from the reverse list
			// only MSN can do this, so this is currently not supported

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
			m_contacts[ handle ]->removeFromGroup( gn );
		}

		if( m_contacts[ handle ]->groups().isEmpty() )
		{
			delete m_contacts[ handle ];
			m_contacts.remove( handle );
		}
	}
}

void MSNProtocol::slotContactAdded( QString handle, QString publicName,
	QString list, uint /* serial */, uint group )
{
	QString gn = groupName( group );
	if( gn.isNull() )
		gn = "Unknown";

	if( m_contacts.contains( handle ) )
	{
		if( group )
			m_contacts[ handle ]->addToGroup( gn );

		if( list == "BL" )
			m_contacts[ handle ]->setBlocked( true );
		else if( list == "FL" )
			m_contacts[ handle ]->addToGroup( gn );
		else if( list == "AL" )
		{
			// Unblocking a blocked contact
			m_contacts[ handle ]->setBlocked( false );
		}
	}
	else
	{
		// contact not found, create new one
		if( list == "FL" )
		{
			KopeteContactList *l = KopeteContactList::contactList();
			KopeteMetaContact *m = l->findContact( this->id(), QString::null, handle );
			KopeteContact *c = m->findContact( this->id(), QString::null, handle );

			if( c )
			{
				// Existing contact, update data
				// FIXME: TODO!
				kdDebug() << "MSNProtocol::slotContactAdded: Not implemented: "
					<< "Meta contact already contains contact " << handle
					<< "???" << endl;
			}
			else
			{
                QString protocol = this->id();

				MSNContact *c = new MSNContact( protocol, handle, publicName,
					gn, m );
				connect( c, SIGNAL( contactDestroyed( KopeteContact * ) ),
					SLOT( slotContactDestroyed( KopeteContact * ) ) );
				m_metaContacts.insert( m, c );
				m->addContact( c, gn );

				// FIXME: Try to remove this
				m_contacts.insert( c->msnId(), c );
			}
		}
	}

	if( list == "AL" )
	{
		if( !m_allowList.contains( handle ) )
			m_allowList.append( handle );
	}
	else if( list == "BL" )
	{
		if( !m_blockList.contains( handle ) )
			m_blockList.append( handle );
	}
}

void MSNProtocol::slotStatusChanged( QString status )
{
	m_status = convertStatus( status );
}

void MSNProtocol::slotPublicNameChanged(QString handle, QString publicName)
{
	if( handle == m_msnId && publicName != m_publicName )
	{
		if( m_publicNameSyncMode & SyncFromServer )
		{
			m_publicName = publicName;
			m_publicNameSyncMode = SyncBoth;

      m_myself->setDisplayName(publicName);

			actionStatusMenu->popupMenu()->changeTitle( m_menuTitleId,
				*( statusBarIcon->pixmap() ),
				i18n( "%1 (%2)" ).arg( m_publicName ).arg( m_msnId ) );

			// Also sync the config file
			KConfig *config=KGlobal::config();
			config->setGroup( "MSN" );
			config->writeEntry( "Nick", m_publicName );
			config->sync();
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

	m_notifySocket->changePublicName( publicName );
}

void MSNProtocol::slotCreateChat( QString address, QString auth)
{
	slotCreateChat( 0L, address, auth, m_msgHandle, publicName() );
}

void MSNProtocol::slotMessageSent( const KopeteMessage& msg, KopeteMessageManager *manager )
{
	kdDebug() << "MSNProtocol::slotMessageSent: Message sent to " <<
		msg.to().first()->displayName() << endl;

	MSNSwitchBoardSocket *service = m_switchBoardSockets[ manager ];
	if( service )
		service->slotSendMsg( msg );
	else // There's no switchboard available, so we must create a new one!
	{
		MSNContact *contact = dynamic_cast<MSNContact*>( msg.to().first() );
		if ( !contact )
		{
			kdDebug() <<
				"MSNProtocol::slotMessageSent:Unable to create a " <<
				"new switchboard!" << endl;
			return;
		}
		kdDebug() << "MSNProtocol::slotMessageSent: Creating new "
			<< "SwitchBoardSocket for " << contact->msnId() << "!" << endl;

     
		slotStartChatSession( contact->msnId() );
    m_msgQueued=new KopeteMessage(msg);
	}
}

void MSNProtocol::slotCreateChat( QString ID, QString address, QString auth,
	QString handle, QString /* publicName */ )
{
	kdDebug() << "MSNProtocol::slotCreateChat: Creating chat for " <<
		handle << endl;

	KopeteContact *c = m_contacts[ handle ];
	if ( c && m_myself )
	{
		KopeteContactPtrList chatmembers;
		chatmembers.append(c);

		KopeteMessageManager *manager = kopeteapp->sessionFactory()->create(
			m_myself, chatmembers, this, QString( "msn_logs/" + ID + ".log" ) );

		// FIXME: Don't we leak this ?
		MSNSwitchBoardSocket *chatService = new MSNSwitchBoardSocket();
		chatService->setHandle( m_msnId );
		chatService->setMsgHandle( handle );
		chatService->connectToSwitchBoard( ID, address, auth );
		m_switchBoardSockets.insert( manager, chatService );

		connect( chatService, SIGNAL( updateChatMember(QString,bool,QString)),
			this, SLOT( slotUpdateChatMember(QString,bool,QString) ) );


		connect( chatService, SIGNAL( msgReceived( const KopeteMessage & ) ),
			manager, SLOT( appendMessage( const KopeteMessage & ) ) );
		connect( chatService,
			SIGNAL( switchBoardClosed(MSNSwitchBoardSocket *) ),
			this, SLOT( slotSwitchBoardClosed(MSNSwitchBoardSocket * ) ) );

		// We may have a new KMM here, but it could just as well be an
		// existing instance. To avoid connecting multiple times, try to
		// disconnect the existing connection first
		disconnect( manager, SIGNAL( messageSent( const KopeteMessage&, KopeteMessageManager* ) ),
			this, SLOT( slotMessageSent( const KopeteMessage& , KopeteMessageManager*) ) );
		connect( manager, SIGNAL( messageSent( const KopeteMessage&, KopeteMessageManager* ) ),
			this, SLOT( slotMessageSent( const KopeteMessage& , KopeteMessageManager*) ) );
		manager->readMessages();


    if(m_msgQueued)
    {
        chatService->slotSendMsg( *m_msgQueued );
        delete m_msgQueued;
        m_msgQueued=0L;
    }

	}
}

void MSNProtocol::slotStartChatSession( QString handle )
{
	// First create a message manager, because we might get an existing
	// manager back, in which case we likely also have an active switchboard
	// connection to reuse...
	KopeteContact *c = m_contacts[ handle ];
	if( isConnected() && c && m_myself && handle != m_msnId )
	{
		KopeteContactPtrList chatmembers;
		chatmembers.append(c);

		KopeteMessageManager *manager = kopeteapp->sessionFactory()->create(
			m_myself, chatmembers, this,
			QString( "msn_logs/" + handle + ".log" ) );

		if( m_switchBoardSockets.find( manager ) )
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

void MSNProtocol::contactUnBlock( QString handle ) const
{
	m_notifySocket->removeContact( handle, 0, BL );
	m_notifySocket->addContact( handle, handle, 0, AL );
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
			slotPublicNameChanged( m_msnId, name );
			m_publicNameSyncMode = SyncToServer;
		}
	}
}

void MSNProtocol::slotDebugRawCommand()
{
	MSNDebugRawCmdDlg *dlg = new MSNDebugRawCmdDlg( 0L );
	int result = dlg->exec();
	if( result == QDialog::Accepted )
	{
		m_notifySocket->sendCommand( dlg->command(), dlg->params(),
					     dlg->addId() );
	}
	delete dlg;
}

void MSNProtocol::slotNotifySocketClosed( int state )
{
	if ( state == 0x10 ) // connection died unexpectedly
	{
		//KMessageBox::error( 0, i18n( "Connection with the MSN server was lost unexpectedly.\nIf you are unable to reconnect, please try again later." ), i18n( "Connection lost - MSN Plugin - Kopete" ) );
		Disconnect();
		kdDebug() << "MSNProtocol::slotNotifySocketClosed: Done." << endl;
	}
}

void MSNProtocol::slotSwitchBoardClosed( MSNSwitchBoardSocket *switchboard)
{
	QPtrDictIterator<MSNSwitchBoardSocket> it( m_switchBoardSockets );
	for( ; m_switchBoardSockets.count() && it.current(); ++it )
	{
		if( it == switchboard )
		{
			kdDebug() << "MSNProtocol::slotSwitchBoardClosed" << endl;

			// remove from the list, then make it kill itself
			m_switchBoardSockets.take( it.currentKey() )->deleteLater();
			break;
		}
	}
}

void MSNProtocol::slotContactDestroyed( KopeteContact *c )
{
	m_metaContacts.remove( c->metaContact() );
}

/** 
 *  eventually add a contact if the contact does not exist (O.G.)
 */
void MSNProtocol::slotUpdateChatMember(QString handle, bool add, QString publicName)
{
		if( add && !m_contacts.contains( handle ) )
      slotContactList( handle, publicName, i18n("Not in Contact List") , "FL" );
}


#include "msnprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

