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

#include "kmsnchatservice.h"
#include "kmsnservicesocket.h"
#include "kopete.h"
#include "msnaddcontactpage.h"
#include "msncontact.h"
#include "msnmessagedialog.h"
#include "msnpreferences.h"
#include "msnprotocol.h"
#include "newuserimpl.h"
#include "statusbaricon.h"

MSNProtocol::MSNProtocol(): QObject(0, "MSNProtocol"), KopeteProtocol()
{
	if( s_protocol )
		kdDebug() << "MSNProtocol::MSNProtocol: WARNING: s_protocol already defined!" << endl;
	else
		s_protocol = this;

	m_status = FLN;
	mIsConnected = false;
	m_serial = 0;
	m_silent = false;

	m_serviceSocket = new KMSNServiceSocket;

	kdDebug() << "MSNProtocol::MSNProtocol: MSN Plugin Loading" << endl;

	initIcons();
	initActions();

	kdDebug() << "MSN Protocol Plugin: Creating Status Bar icon\n";
	statusBarIcon = new StatusBarIcon();
	QObject::connect(statusBarIcon, SIGNAL(rightClicked(const QPoint)), this, SLOT(slotIconRightClicked(const QPoint)));
	statusBarIcon->setPixmap( offlineIcon );

	kdDebug() << "MSN Protocol Plugin: Creating Config Module\n";
	mPrefs = new MSNPreferences( "msn_protocol", this );
	connect( mPrefs, SIGNAL( saved( void ) ),
		this, SIGNAL( settingsChanged( void ) ) );

	// Connect to the signals from the serviceSocket, which is possible now
	// the service has created the socket for us.
	connect( m_serviceSocket, SIGNAL( groupAdded( QString, uint,uint ) ),
		this, SLOT( slotGroupAdded( QString, uint, uint ) ) );
	connect( m_serviceSocket, SIGNAL( groupRenamed( QString, uint, uint ) ),
		this, SLOT( slotGroupRenamed( QString, uint, uint ) ) );
	connect( m_serviceSocket, SIGNAL( groupName( QString, uint ) ),
		this, SLOT( slotGroupListed( QString, uint ) ) );
	connect( m_serviceSocket, SIGNAL(groupRemoved( uint, uint ) ),
		this, SLOT( slotGroupRemoved( uint, uint ) ) );
	connect( m_serviceSocket, SIGNAL( statusChanged( QString ) ),
				this, SLOT( slotStateChanged( QString ) ) );
	connect( m_serviceSocket,
		SIGNAL( contactStatusChanged( QString, QString, QString ) ),
		this,
		SLOT( slotContactStatusChanged( QString, QString, QString ) ) );
	connect( m_serviceSocket,
		SIGNAL( contactList( QString, QString, QString, QString ) ),
		this, SLOT( slotContactList( QString, QString, QString, QString ) ) );
	connect( m_serviceSocket,
		SIGNAL( contactAdded( QString, QString, QString, uint, uint ) ),
		this,
		SLOT( slotContactAdded( QString, QString, QString, uint, uint ) ) );
	connect( m_serviceSocket,
		SIGNAL( contactRemoved( QString, QString, uint, uint ) ),
		this,
		SLOT( slotContactRemoved( QString, QString, uint, uint ) ) );
	connect( m_serviceSocket, SIGNAL( statusChanged( QString ) ),
		this, SLOT( slotStatusChanged( QString ) ) );
	connect( m_serviceSocket,
		SIGNAL( contactStatus( QString, QString, QString ) ),
		this, SLOT( slotContactStatus( QString, QString, QString ) ) );
	connect( m_serviceSocket, SIGNAL( connected( bool ) ),
		this, SLOT( slotConnected( bool ) ) );
	connect( m_serviceSocket, SIGNAL( publicNameChanged( QString, QString ) ),
		this, SLOT( slotPublicNameChanged( QString, QString ) ) );
	connect( m_serviceSocket, SIGNAL( newPublicName( QString ) ),
		this, SLOT( slotPublicNameReceived( QString ) ) );
	connect( m_serviceSocket,
		SIGNAL( invitedToChat( QString, QString, QString, QString, QString ) ),
		this,
		SLOT( slotCreateChat( QString, QString, QString, QString, QString ) ) );
	connect( m_serviceSocket, SIGNAL( startChat( QString, QString ) ),
		this, SLOT( slotCreateChat( QString, QString ) ) );

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

	if ( cfg->readBoolEntry( "AutoConnect", "0" ) )
		Connect();
}

MSNProtocol::~MSNProtocol()
{
	m_groupList.clear();

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

		m_msnId = KGlobal::config()->readEntry( "UserID", "" );
		m_password = KGlobal::config()->readEntry( "Password", "" );
		//m_msnService->setMyPublicName(KGlobal::config()->readEntry("Nick", ""));
		m_serviceSocket->connectToService( m_msnId, m_password, m_serial,
			m_silent );
		statusBarIcon->setMovie( connectingIcon );
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
		m_serviceSocket->closeService();
	}
	else
	{
    	kdDebug() << "MSN Plugin: Ignoring Disconnect request (Im not Connected)" << endl;
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
	actionGoOnline = new KAction ( i18n("Go online"), "msn_online", 0, this, SLOT(slotGoOnline()), this, "actionMSNConnect" );
	actionGoOffline = new KAction ( i18n("Go Offline"), "msn_offline", 0, this, SLOT(slotGoOffline()), this, "actionMSNConnect" );
	actionGoAway = new KAction ( i18n("Go Away"), "msn_away", 0, this, SLOT(slotGoAway()), this, "actionMSNConnect" );
	actionStatusMenu = new KActionMenu("MSN",this);
	actionStatusMenu->insert( actionGoOnline );
	actionStatusMenu->insert( actionGoOffline );
	actionStatusMenu->insert( actionGoAway );

	actionStatusMenu->plug( kopeteapp->systemTray()->contextMenu(), 1 );
}

void MSNProtocol::slotIconRightClicked( const QPoint /* point */ )
{
	KGlobal::config()->setGroup("MSN");
	QString handle = KGlobal::config()->readEntry("UserID", i18n("(User ID not set)"));

	KPopupMenu *popup = new KPopupMenu(statusBarIcon);
	popup->insertTitle(handle);
	actionGoOnline->plug( popup );
	actionGoOffline->plug( popup );
	actionGoAway->plug( popup );
	popup->popup(QCursor::pos());
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

void MSNProtocol::slotMessageDialogClosing(QString handle)
{
	mChatWindows.setAutoDelete(true);
	MSNMessageDialog *messageDialog = mChatWindows.first();
	for ( ; messageDialog; messageDialog = mChatWindows.next() )
	{
		if ( messageDialog->contact()->msnId() == handle )
		{
			mChatWindows.remove(messageDialog);
		}
	}
}

void MSNProtocol::slotGoOnline()
{
	kdDebug() << "MSN Plugin: Going Online" << endl;
	if (!isConnected() )
		Connect();
	else
		m_serviceSocket->setStatus( NLN );
}
void MSNProtocol::slotGoOffline()
{
	kdDebug() << "MSN Plugin: Going Offline" << endl;
	if (isConnected() )
		Disconnect();
	else // disconnect while trying to connect. Anyone know a better way? (remenic)
	{
		m_serviceSocket->kill();
		statusBarIcon->setPixmap(offlineIcon);
	}
}

void MSNProtocol::slotGoAway()
{
	kdDebug() << "MSN Plugin: Going Away" << endl;
	if (!isConnected() )
		Connect();
	m_serviceSocket->setStatus( AWY );
}

void MSNProtocol::slotPublicNameReceived(QString publicName)
{
	m_publicName = publicName;
}

void MSNProtocol::slotConnected( bool c )
{
	mIsConnected = c;
	if ( c )
	{
		m_publicName = m_serviceSocket->_publicName;
		mIsConnected = true;

		QStringList contacts;
		QString group, publicname, userid;

		statusBarIcon->setPixmap( onlineIcon );

		// FIXME: is there any way to do a faster sync of msn groups?
		/* Now we sync local groups that dont exist in server */
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
					kdDebug() << "MSN Plugin: Sync: Local group " << localgroup << " dont exists in server!" << endl;
					/*
					QString notexistsMsg = i18n(
						"the group %1 doesn't exist in MSN server group list, if you want to move" \
						" a MSN contact to this group you need to add it to MSN server, do you want" \
						" to add this group to the server group list?" ).arg(localgroup);
					useranswer = KMessageBox::warningYesNo (kopeteapp->mainWindow(), notexistsMsg , i18n("New local group found...") );
					*/
					addGroup( localgroup );
				}
			}
		}
	}
	else
	{
		QMap<QString, MSNContact*>::Iterator it = m_contacts.begin();
		while( it != m_contacts.end() )
		{
			delete *it;
			m_contacts.remove( it );
			it = m_contacts.begin();
		}

		m_groupList.clear();
		mIsConnected = false;
		statusBarIcon->setPixmap(offlineIcon);

		m_status = FLN;
		m_serial = 0;
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
	m_serviceSocket->removeContact( handle, 0, AL);
	m_serviceSocket->addContact( handle, m_contacts[ handle ]->nickname(),
		0, BL );
}

void MSNProtocol::addContact( const QString &userID )
{
	if( isConnected() )
	{
		m_serviceSocket->addContact( userID, m_contacts[ userID ]->nickname(),
			0, FL );
		m_serviceSocket->addContact( userID, m_contacts[ userID ]->nickname(),
			0, AL );
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
		m_serviceSocket->removeContact( id, groupNumber( (*it).latin1() ),
			FL );
	}

	if( m_contacts[ id ]->isBlocked() )
		m_serviceSocket->removeContact( id, 0, BL );
}

void MSNProtocol::removeFromGroup( const MSNContact *c,
	const QString &group ) const
{
	int g = groupNumber( group );
	if( g != -1 )
		m_serviceSocket->removeContact( c->msnId(), g, FL );
}

void MSNProtocol::moveContact( const MSNContact *c,
	const QString &oldGroup, const QString &newGroup ) const
{
	int og = groupNumber( oldGroup );
	int ng = groupNumber( newGroup );

	if( og != -1 && ng != -1 )
	{
		m_serviceSocket->addContact( c->msnId(), c->nickname(), ng,
			FL);
		m_serviceSocket->removeContact( c->msnId(), og, FL );
	}
}

void MSNProtocol::copyContact( const MSNContact *c,
	const QString &newGroup ) const
{
	int g = groupNumber( newGroup );
	if( g != -1 )
	{
		m_serviceSocket->addContact( c->msnId(), c->nickname(), g,
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
		m_serviceSocket->addGroup( groupName );
}

void MSNProtocol::renameGroup( const QString &oldGroup,
	const QString &newGroup )
{
	int g = groupNumber( oldGroup );
	if( g != -1 )
		m_serviceSocket->renameGroup( newGroup, g );
}

void MSNProtocol::removeGroup( const QString &name )
{
	int g = groupNumber( name );
	if( g != -1 )
		m_serviceSocket->removeGroup( g );
}

MSNProtocol::Status MSNProtocol::status() const
{
	return m_status;
}

MSNProtocol::Status MSNProtocol::convertStatus( QString status ) const
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
		m_contacts[ handle ]->setNickname( publicName );
	}
}

void MSNProtocol::slotContactStatusChanged( QString handle, QString publicName,
	QString status )
{
	kdDebug() << "MSNProtocol::slotContactStatusChanged: " << handle << " (" <<
		publicName << ") has status " << status << endl;

	if( m_contacts.contains( handle ) )
	{
		if( status == "FLN" )
			m_contacts[ handle ]->setMsnStatus( FLN );
		else
		{
			m_contacts[ handle ]->setMsnStatus( convertStatus( status ) );
			m_contacts[ handle ]->setNickname( publicName );
		}
	}
}

void MSNProtocol::slotContactList( QString handle, QString publicName,
	QString group, QString list )
{
	MSNContact *c;
	QStringList groups;
	groups = QStringList::split(",", group, false );
	if( list == "FL" )
	{
		// FIXME: Proper MSNContact CTOR!
		c = new MSNContact( handle, publicName, QString::null, 0L );
		for( QStringList::Iterator it = groups.begin();
			it != groups.end(); ++it )
		{
			c->addToGroup( groupName( (*it).toUInt() ) );
			addToContactList( c, groupName( (*it).toUInt() ) );
		}
	}
	else if( list == "BL" )
	{
		if( m_contacts.contains( handle ) )
			m_contacts[ handle ]->setBlocked( true );
	}
	else if( list == "AL" )
	{
		// deleted Contacts might still be in allow list
		if( !m_contacts.contains( handle ) )
		{
			// FIXME: Proper MSNContact ctor required!
			c = new MSNContact( handle, publicName, QString::null, 0L );
			c->setDeleted( true );
			addToContactList( c, "UNKNOWN GROUP!" );
		}
	}
	else if( list == "RL" )
	{
		// search for new Contacts
		if( m_contacts.contains( handle ) )
			m_contacts[ handle ]->setNickname( publicName );
		else
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
	uint serial, uint group )
{
	m_serial = serial;

	QString gn = groupName( group );
	if( gn.isNull() )
		gn = i18n( "Unknown" );

	if( m_contacts.contains( handle ) )
	{
		if( group )
			m_contacts[ handle ]->removeFromGroup( gn );

		if( list == "BL" )
		{
			// the contact is removed from the blocked list,
			// add it to the AL list
			m_contacts[ handle ]->setBlocked( false );
			m_serviceSocket->addContact( handle,
				m_contacts[ handle ]->nickname(), 0, AL );
		}
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
	QString list, uint serial, uint group )
{
	m_serial = serial;

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
	}
	else
	{
		// contact not found, create new one
		if( list == "FL" )
		{
			// FIXME: Proper MSNContact ctor!
			MSNContact *c = new MSNContact( handle, publicName, gn, 0L );
			c->setDeleted( true );
			addToContactList( c, gn );
		}
		else if( list == "AL" )
		{
			// FIXME: Proper MSNContact ctor!
			MSNContact *c = new MSNContact( handle, publicName, QString::null, 0L );
			c->setBlocked( false );
			addToContactList( c, gn );
		}
	}
}

void MSNProtocol::slotStatusChanged( QString status )
{
	m_status = convertStatus( status );
}

void MSNProtocol::slotPublicNameChanged(QString handle, QString publicName)
{
	if( handle == m_msnId )
		m_publicName = publicName;
}

void MSNProtocol::setPublicName( const QString &publicName )
{
	m_serviceSocket->changePublicName( publicName );
}

void MSNProtocol::slotCreateChat( QString address, QString auth)
{
	slotCreateChat( 0L, address, auth, m_msgHandle, publicName() );
}

void MSNProtocol::slotCreateChat( QString ID, QString address, QString auth,
	QString handle, QString /* publicName */ )
{
	// FIXME: Don't we leak this ?
	KMSNChatService *chatService = new KMSNChatService();
	chatService->setHandle( m_msnId );
	chatService->connectToSwitchBoard( ID, address, auth );

	MSNMessageDialog *messageDialog;

	// Maybe we have a copy of us in another group
	for( messageDialog = mChatWindows.first() ; messageDialog; messageDialog = mChatWindows.next() )
	{
		if ( messageDialog->contact()->msnId() == handle )
			break;
	}

	if( messageDialog && messageDialog->isVisible() )
	{
		kdDebug() << "MSN Plugin: Incoming chat but Window already opened for " << handle <<"\n";
		messageDialog->setBoard( chatService );
		connect( chatService, SIGNAL(msgReceived(QString,QString,QString, QFont, QColor)),messageDialog,SLOT(slotMessageReceived(QString,QString,QString, QFont, QColor)));
		messageDialog->raise();
	}
	else
	{
		kdDebug() << "MSN Plugin: Incoming chat, no window, creating window for " << handle <<"\n";
		// FIXME: MSN message dialog needs status

		messageDialog = new MSNMessageDialog( m_contacts[ handle ], chatService, this );
//		connect( this, SIGNAL(userStateChanged(QString)), messageDialog, SLOT(slotUserStateChanged(QString)) );
		connect( messageDialog, SIGNAL(closing(QString)), this, SLOT(slotMessageDialogClosing(QString)) );

		mChatWindows.append( messageDialog );
		messageDialog->show();
	}
}

void MSNProtocol::slotStartChatSession( QString handle )
{
	if( isConnected() )
	{
		if( handle == m_msnId )
			return;
		m_msgHandle = handle;
		m_serviceSocket->createChatSession();
	}
}

void MSNProtocol::contactUnBlock( QString handle )
{
	m_serviceSocket->removeContact( handle, 0, BL );
}

#include "msnprotocol.moc"

// vim: set ts=4 sts=4 sw=4 noet:

