/*
    msnaccount.h - Manages a single MSN account

    Copyright (c) 2003-2004 by Olivier Goffart       <ogoffart@tiscalinet.be>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "msnaccount.h"

#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kmdcodec.h>

#include <qfile.h>
#include <qregexp.h>

#include "msncontact.h"
#include "msnnotifysocket.h"
#include "msnmessagemanager.h"
#include "newuserimpl.h"
#include "kopetecontactlist.h"
#include "kopetegroup.h"
#include "kopetemetacontact.h"
#include "kopetepassword.h"
#include "kopeteuiglobal.h"
#include "kopeteglobal.h"

#include "sha1.h"


#if !defined NDEBUG
#include "msndebugrawcmddlg.h"
#include <kglobal.h>
#endif

MSNAccount::MSNAccount( MSNProtocol *parent, const QString& AccountID, const char *name )
: Kopete::PasswordedAccount ( parent, AccountID, 0, name )
{
	m_notifySocket = 0L;
	m_connectstatus = MSNProtocol::protocol()->NLN;
	m_addWizard_metaContact = 0L;

	// Init the myself contact
	// FIXME: I think we should add a global self metaContact ( Olivier )
	setMyself( new MSNContact( this, accountId(), 0L ) );
	//myself()->setOnlineStatus( MSNProtocol::protocol()->FLN );

	QObject::connect( Kopete::ContactList::self(), SIGNAL( groupRenamed( Kopete::Group *, const QString & ) ),
		SLOT( slotKopeteGroupRenamed( Kopete::Group * ) ) );
	QObject::connect( Kopete::ContactList::self(), SIGNAL( groupRemoved( Kopete::Group * ) ),
		SLOT( slotKopeteGroupRemoved( Kopete::Group * ) ) );

	m_openInboxAction = new KAction( i18n( "Open Inbo&x..." ), "mail_generic", 0, this, SLOT( slotOpenInbox() ), this, "m_openInboxAction" );
	m_changeDNAction = new KAction( i18n( "&Change Display Name..." ), QString::null, 0, this, SLOT( slotChangePublicName() ), this, "renameAction" );
	m_startChatAction = new KAction( i18n( "&Start Chat..." ), "mail_generic", 0, this, SLOT( slotStartChat() ), this, "startChatAction" );


	KConfigGroup *config=configGroup();

	QString publicName = config->readEntry(  QString::fromLatin1( "displayName" ) );
	if ( !publicName.isNull() )
		myself()->setProperty( Kopete::Global::Properties::self()->nickName() , publicName );
	m_blockList   = config->readListEntry(  "blockList" ) ;
	m_allowList   = config->readListEntry(  "allowList" ) ;
	m_reverseList = config->readListEntry(  "reverseList"  ) ;

	static_cast<MSNContact *>( myself() )->setInfo( "PHH", config->readEntry("PHH") );
	static_cast<MSNContact *>( myself() )->setInfo( "PHM", config->readEntry("PHM") );
	static_cast<MSNContact *>( myself() )->setInfo( "PHW", config->readEntry("PHW") );

	//construct the group list
	//Before 2003-11-14 the MSN server allowed us to download the group list without downloading the whole contactlist, but it's not possible anymore
	QPtrList<Kopete::Group> groupList = Kopete::ContactList::self()->groups();
	for ( Kopete::Group *g = groupList.first(); g; g = groupList.next() )
	{
		QString groupNumber=g->pluginData( protocol(), accountId() + " id" );
		if ( !groupNumber.isEmpty() )
			m_groupList.insert( groupNumber.toUInt() , g );
	}
}


QString MSNAccount::serverName()
{
	return configGroup()->readEntry(  "serverName" , "messenger.hotmail.com" );
}

uint MSNAccount::serverPort()
{
	return configGroup()->readNumEntry(  "serverPort" , 1863 );
}

void MSNAccount::setAway( bool away, const QString & awayReason )
{
	m_awayReason = awayReason;
	if ( away )
		setOnlineStatus( MSNProtocol::protocol()->IDL );
	else // if ( myself()->onlineStatus() == MSNProtocol::statusIDL )
		setOnlineStatus( MSNProtocol::protocol()->NLN );
}

void MSNAccount::connectWithPassword( const QString &passwd )
{
	if ( isConnected() )
	{
		kdDebug( 14140 ) << k_funcinfo <<"Ignoring Connect request "
			<< "(Already Connected)" << endl;
		return;
	}

	if ( m_notifySocket )
	{
		kdDebug( 14140 ) << k_funcinfo <<"Ignoring Connect request (Already connecting)"  << endl;
		return;
	}

	m_password = passwd;

	if ( m_password.isNull() )
	{
		kdDebug( 14140 ) << k_funcinfo <<"Abort connection (null password)"  << endl;
		return;
	}


	if ( contacts().count() <= 1 )
	{
		// Maybe the contactlist.xml has been removed, and the serial number not updated
		// ( the 1 is for the myself contact )
		configGroup()->writeEntry( "serial", 0 );
	}

	m_openInboxAction->setEnabled( false );

	createNotificationServer(serverName(), serverPort());
}

void MSNAccount::createNotificationServer( const QString &host, uint port )
{
	if(m_notifySocket) //we are switching from one to another notifysocket.
	{
		//remove every slots to that socket, so we won't delete receive signals
		// from the old socket thinking they are from the new one
		QObject::disconnect( m_notifySocket , 0, this, 0 );
		m_notifySocket->deleteLater(); //be sure it will be deleted
		m_notifySocket=0L;
	}


	myself()->setOnlineStatus( MSNProtocol::protocol()->CNT );


	m_notifySocket = new MSNNotifySocket( this, accountId() , m_password);

	QObject::connect( m_notifySocket, SIGNAL( groupAdded( const QString&, uint ) ),
		SLOT( slotGroupAdded( const QString&, uint ) ) );
	QObject::connect( m_notifySocket, SIGNAL( groupRenamed( const QString&, uint ) ),
		SLOT( slotGroupRenamed( const QString&, uint ) ) );
	QObject::connect( m_notifySocket, SIGNAL( groupListed( const QString&, uint ) ),
		SLOT( slotGroupAdded( const QString&, uint ) ) );
	QObject::connect( m_notifySocket, SIGNAL( groupRemoved( uint ) ),
		SLOT( slotGroupRemoved( uint ) ) );
	QObject::connect( m_notifySocket, SIGNAL( contactList( const QString&, const QString&, uint, const QString& ) ),
		SLOT( slotContactListed( const QString&, const QString&, uint, const QString& ) ) );
	QObject::connect( m_notifySocket, SIGNAL( contactAdded( const QString&, const QString&, const QString&, uint ) ),
		SLOT( slotContactAdded( const QString&, const QString&, const QString&, uint ) ) );
	QObject::connect( m_notifySocket, SIGNAL( contactRemoved( const QString&, const QString&, uint ) ),
		SLOT( slotContactRemoved( const QString&, const QString&, uint ) ) );
	QObject::connect( m_notifySocket, SIGNAL( statusChanged( const Kopete::OnlineStatus & ) ),
		SLOT( slotStatusChanged( const Kopete::OnlineStatus & ) ) );
	QObject::connect( m_notifySocket, SIGNAL( onlineStatusChanged( MSNSocket::OnlineStatus ) ),
		SLOT( slotNotifySocketStatusChanged( MSNSocket::OnlineStatus ) ) );
	QObject::connect( m_notifySocket, SIGNAL( publicNameChanged( const QString& ) ),
		SLOT( slotPublicNameChanged( const QString& ) ) );
	QObject::connect( m_notifySocket, SIGNAL( invitedToChat( const QString&, const QString&, const QString&, const QString&, const QString& ) ),
		SLOT( slotCreateChat( const QString&, const QString&, const QString&, const QString&, const QString& ) ) );
	QObject::connect( m_notifySocket, SIGNAL( startChat( const QString&, const QString& ) ),
		SLOT( slotCreateChat( const QString&, const QString& ) ) );
	QObject::connect( m_notifySocket, SIGNAL( socketClosed() ),
		SLOT( slotNotifySocketClosed() ) );
	QObject::connect( m_notifySocket, SIGNAL( newContactList() ),
		SLOT( slotNewContactList() ) );
	QObject::connect( m_notifySocket, SIGNAL( receivedNotificationServer(const QString&, uint )  ),
		SLOT(createNotificationServer(const QString&, uint ) ) );
	QObject::connect( m_notifySocket, SIGNAL( hotmailSeted( bool ) ),
		m_openInboxAction, SLOT( setEnabled( bool ) ) );

	m_notifySocket->setStatus( m_connectstatus );
	m_notifySocket->connect(host, port);
}

void MSNAccount::disconnect()
{
	if ( m_notifySocket )
		m_notifySocket->disconnect();
}

KActionMenu * MSNAccount::actionMenu()
{
	KActionMenu *m_actionMenu = new KActionMenu( accountId(), myself()->onlineStatus().iconFor( this ),  this );
	m_actionMenu->popupMenu()->insertTitle( myself()->onlineStatus().iconFor( myself() ), i18n( "%2 <%1>" ).
		arg( accountId(), myself()->property( Kopete::Global::Properties::self()->nickName()).value().toString() ) );

	if ( isConnected() )
	{
		m_openInboxAction->setEnabled( true );
		m_startChatAction->setEnabled( true );
		m_changeDNAction->setEnabled( true );
	}
	else
	{
		m_openInboxAction->setEnabled( false );
		m_startChatAction->setEnabled( false );
		m_changeDNAction->setEnabled( false );
	}

	m_actionMenu->insert( new KAction ( i18n( "Set O&nline" ),        MSNProtocol::protocol()->NLN.iconFor( this ), 0,
		this, SLOT( slotGoOnline() ), m_actionMenu, "actionMSNConnect" ) );
	m_actionMenu->insert( new KAction ( i18n( "Set &Away" ),          MSNProtocol::protocol()->AWY.iconFor( this ), 0,
		this, SLOT( slotGoAway() ), m_actionMenu, "actionMSNConnect" ) );
	m_actionMenu->insert( new KAction ( i18n( "Set &Busy" ),          MSNProtocol::protocol()->BSY.iconFor( this ), 0,
		this, SLOT( slotGoBusy() ), m_actionMenu, "actionMSNConnect" ) );
	m_actionMenu->insert( new KAction ( i18n( "Set Be &Right Back" ), MSNProtocol::protocol()->BRB.iconFor( this ), 0,
		this, SLOT( slotGoBeRightBack() ), m_actionMenu, "actionMSNConnect" ) );
	m_actionMenu->insert( new KAction ( i18n( "Set on &Phone" ),      MSNProtocol::protocol()->PHN.iconFor( this ), 0,
		this, SLOT( slotGoOnThePhone() ), m_actionMenu, "actionMSNConnect" ) );
	m_actionMenu->insert( new KAction ( i18n( "Set Out to &Lunch" ),  MSNProtocol::protocol()->LUN.iconFor( this ), 0,
		this, SLOT( slotGoOutToLunch() ), m_actionMenu, "actionMSNConnect" ) );
	m_actionMenu->insert( new KAction ( i18n( "Set &Invisible" ),     MSNProtocol::protocol()->HDN.iconFor( this ), 0,
		this, SLOT( slotGoInvisible() ), m_actionMenu, "actionMSNConnect" ) );
	m_actionMenu->insert( new KAction ( i18n( "Set &Offline" ),       MSNProtocol::protocol()->FLN.iconFor( this ), 0,
		this, SLOT( slotGoOffline() ), m_actionMenu, "actionMSNConnect" ) );

	m_actionMenu->popupMenu()->insertSeparator();

	m_actionMenu->insert( m_changeDNAction );
	m_actionMenu->insert( m_startChatAction );

	m_actionMenu->popupMenu()->insertSeparator();

	m_actionMenu->insert( m_openInboxAction );

#if !defined NDEBUG
	KActionMenu *debugMenu = new KActionMenu( "Debug", m_actionMenu );
	debugMenu->insert( new KAction( i18n( "Send Raw C&ommand..." ), 0,
		this, SLOT( slotDebugRawCommand() ), debugMenu, "m_debugRawCommand" ) );
	m_actionMenu->popupMenu()->insertSeparator();
	m_actionMenu->insert( debugMenu );
#endif

	return m_actionMenu;
}

MSNNotifySocket *MSNAccount::notifySocket()
{
	return m_notifySocket;
}

void MSNAccount::slotGoOnline()
{
	m_awayReason = QString::null;
	setOnlineStatus( MSNProtocol::protocol()->NLN );
}

void MSNAccount::slotGoOffline()
{
	disconnect();
	// m_connectstatus = NLN;
}

void MSNAccount::slotGoAway()
{
	m_awayReason = QString::null;
	setOnlineStatus( MSNProtocol::protocol()->AWY );
}

void MSNAccount::slotGoBusy()
{
	m_awayReason = QString::null;
	setOnlineStatus( MSNProtocol::protocol()->BSY );
}

void MSNAccount::slotGoBeRightBack()
{
	m_awayReason = QString::null;
	setOnlineStatus( MSNProtocol::protocol()->BRB );
}

void MSNAccount::slotGoOnThePhone()
{
	m_awayReason = QString::null;
	setOnlineStatus( MSNProtocol::protocol()->PHN );
}

void MSNAccount::slotGoOutToLunch()
{
	m_awayReason = QString::null;
	setOnlineStatus( MSNProtocol::protocol()->LUN );
}

void MSNAccount::slotGoInvisible()
{
	m_awayReason = QString::null;
	setOnlineStatus( MSNProtocol::protocol()->HDN );
}

void MSNAccount::setOnlineStatus( const Kopete::OnlineStatus &status )
{
	if ( m_notifySocket )
	{
		m_notifySocket->setStatus( status );
	}
	else
	{
		m_connectstatus = status;
		connect();
	}
}

void MSNAccount::slotStartChat()
{

	bool ok;
	QString handle = KInputDialog::getText( i18n( "Start Chat - MSN Plugin" ),
		i18n( "Please enter the email address of the person with whom you want to chat:" ), QString::null, &ok ).lower();
	if ( ok )
	{
		if ( MSNProtocol::validContactId( handle ) )
		{
			if ( !contacts()[ handle ] )
				addContact( handle, handle, 0L, Kopete::Account::DontChangeKABC, QString::null, true );

			contacts()[ handle ]->execute();
		}
		else
		{
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
				i18n( "<qt>You must enter a valid email address.</qt>" ), i18n( "MSN Plugin" ) );
		}
	}
}

void MSNAccount::slotDebugRawCommand()
{
#if !defined NDEBUG
	if ( !isConnected() )
		return;

	MSNDebugRawCmdDlg *dlg = new MSNDebugRawCmdDlg( 0L );
	int result = dlg->exec();
	if ( result == QDialog::Accepted && m_notifySocket )
	{
		m_notifySocket->sendCommand( dlg->command(), dlg->params(),
					dlg->addId(), dlg->msg().replace( "\n", "\r\n" ).utf8() );
	}
	delete dlg;
#endif
}

void MSNAccount::slotChangePublicName()
{
	bool ok;
	QString name = KInputDialog::getText( i18n( "Change Display Name - MSN Plugin" ),
		i18n( "Enter the new display name by which you want to be visible to your friends on MSN:" ),
		myself()->property( Kopete::Global::Properties::self()->nickName()).value().toString(), &ok );

	if ( ok )
	{
		if ( name.length() > 387 )
		{
			KMessageBox::error( Kopete::UI::Global::mainWidget(),
				i18n( "<qt>The display name you entered is too long. Please use a shorter name.\n"
					"Your display name has <b>not</b> been changed.</qt>" ),
				i18n( "Change Display Name - MSN Plugin" ) );
			return;
		}

		if ( isConnected() )
			setPublicName( name );
		else
		{
			// Bypass the protocol, it doesn't work, call the slot
			// directly. Upon connect the name will be synced.
			// FIXME: Use a single code path instead!
			slotPublicNameChanged( name );
			// m_publicNameSyncMode = SyncToServer;
		}
	}
}

void MSNAccount::slotOpenInbox()
{
	if ( m_notifySocket )
		m_notifySocket->slotOpenInbox();
}

void MSNAccount::slotNotifySocketStatusChanged( MSNSocket::OnlineStatus status )
{
	// FIXME:
	if ( status == MSNSocket::Connected )
	{
		// Sync public name when needed
/*
		if ( m_publicNameSyncNeeded )
		{
			kdDebug( 14140 ) << "MSNProtocol::slotOnlineStatusChanged: Syncing public name to "
				<< m_publicName << endl;
			setPublicName( m_publicName );
			m_publicNameSyncNeeded = false;
		}
		else
		{
			kdDebug( 14140 ) << "MSNProtocol::slotOnlineStatusChanged: Leaving public name as "
				<< m_publicName << endl;
		}
		// Now pending changes are updated if we want to sync both ways
		m_publicNameSyncMode = SyncBoth;
*/
		// setStatusIcon( "msn_online" );
	}
	else if ( status == MSNSocket::Disconnected )
	{
/*
		Kopete::MessageManagerDict sessions =
			Kopete::MessageManagerFactory::self()->protocolSessions( this );
		QIntDictIterator<Kopete::MessageManager> kmmIt( sessions );
		for ( ; kmmIt.current(); ++kmmIt )
		{
			// Disconnect all active chats (but don't actually remove the
			// chat windows, the user might still want to view them!)
			MSNMessageManager *msnMM = dynamic_cast<MSNMessageManager *>( kmmIt.current() );
			if ( msnMM )
			{
				kdDebug( 14140 ) << "MSNProtocol::slotOnlineStatusChanged: "
					<< "Closed MSNMessageManager because the protocol socket "
					<< "closed." << endl;
				msnMM->slotCloseSession();
			}
		}
*/

		// FIXME: only do it for contact for this account
		QDictIterator<Kopete::Contact> it( contacts() );
		for ( ; it.current(); ++it )
			( *it )->setOnlineStatus( MSNProtocol::protocol()->FLN );

/*
		m_allowList.clear();
		m_blockList.clear();
		m_reverseList.clear();

		m_groupList.clear();
*/
		// setStatusIcon( "msn_offline" );

		// Reset flags. They can't be set in the connect method, because
		// offline changes might have been made before. Instead the c'tor
		// sets the defaults, and the disconnect slot resets those defaults
		// FIXME: Can't we share this code?
		// m_publicNameSyncMode = SyncFromServer;
	}
	else if ( status == MSNSocket::Connecting )
	{
		// for ( QDictIterator<Kopete::Contact> it( contacts() ); it.current(); ++it )
		//	static_cast<MSNContact *>( *it )->setMsnStatus( MSNProtocol::FLN );
	}
	// static_cast<MSNProtocol *>( protocol() )->slotNotifySocketStatusChanged( status );
}

void MSNAccount::slotNotifySocketClosed()
{
	kdDebug( 14140 ) << k_funcinfo << endl;

	password().setWrong( m_notifySocket->badPassword() );
	m_notifySocket->deleteLater();
	m_notifySocket = 0l;
	myself()->setOnlineStatus( MSNProtocol::protocol()->FLN );
	if ( password().isWrong() )
		connect();

#if 0
	else if ( state == 0x10 ) // connection died unexpectedly
	{
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error , i18n( "The connection with the MSN server was lost unexpectedly.\n"
			"If you cannot reconnect now, the server might be down. In that case, please try again later." ),
			i18n( "Connection Lost - MSN Plugin" ), KMessageBox::Notify );
	}
#endif

	// kdDebug( 14140 ) << "MSNAccount::slotNotifySocketClosed - done" << endl;
}

void MSNAccount::slotStatusChanged( const Kopete::OnlineStatus &status )
{
//	kdDebug( 14140 ) << k_funcinfo  << status.internalStatus() <<  endl;
	myself()->setOnlineStatus( status );
}

void MSNAccount::slotPublicNameChanged( const QString& publicName )
{
	QString oldNick=myself()->property( Kopete::Global::Properties::self()->nickName()).value().toString() ;
	if ( publicName != oldNick )
	{
		myself()->setProperty( Kopete::Global::Properties::self()->nickName(), publicName );
		configGroup()->writeEntry( "displayName" , publicName );
	}
}

void MSNAccount::setPublicName( const QString &publicName )
{
	if ( m_notifySocket )
	{
		m_notifySocket->changePublicName( publicName );
	}
}

void MSNAccount::slotGroupAdded( const QString& groupName, uint groupNumber )
{
	if ( m_groupList.contains( groupNumber ) )
	{
		// Group can already be in the list since the idle timer does a 'List Groups'
		// command. Simply return, don't issue a warning.
		// kdDebug( 14140 ) << k_funcinfo << "Group " << groupName << " already in list, skipped." << endl;
		return;
	}

	//--------- Find the appropriate Kopete::Group, or create one ---------//
	QPtrList<Kopete::Group> groupList = Kopete::ContactList::self()->groups();
	Kopete::Group *fallBack = 0L;

	//check if we have one in the old group list. if yes, update the id translate map.
	for(QMap<unsigned int, Kopete::Group*>::Iterator it=m_oldGroupList.begin() ; it != m_oldGroupList.end() ; ++it )
	{
		Kopete::Group *g=it.data();
		if (g && g->pluginData( protocol(), accountId() + " displayName" ) == groupName  &&
				g->pluginData( protocol(), accountId() + " id" ).isEmpty() )
		{ //it has the same name! we got it.    (and it is not yet an msn group)
			fallBack=g;
			/*if ( g->displayName() != groupName )
			{
				// The displayName was changed in Kopete while we were offline
				// FIXME: update the server right now
			}*/
			break;
		}
	}

	if(!fallBack)
	{
		//it's certenly a new group !  search if one already exist with the same displayname.
		for ( Kopete::Group *g = groupList.first(); g; g = groupList.next() )
		{
			/*   --This has been replaced by the loop right before.
			if ( !g->pluginData( protocol(), accountId() +  " id" ).isEmpty() )
			{
				if ( g->pluginData( protocol(), accountId() + " id" ).toUInt() == groupNumber )
				{
					m_groupList.insert( groupNumber, g );
					QString oldGroupName;
					if ( g->pluginData( protocol(), accountId() + " displayName" ) != groupName )
					{
						// The displayName of the group has been modified by another client
						slotGroupRenamed( groupName, groupNumber );
					}
					return;
				}
			}
			else {*/
			if ( g->displayName() == groupName && (groupNumber==0 || g->type()==Kopete::Group::Normal)  &&
					g->pluginData( protocol(), accountId() + " id" ).isEmpty()  )
			{
				fallBack = g;
				kdDebug( 14140 ) << k_funcinfo << "We didn't found the group " << groupName <<" in the old MSN group.  But kopete has already one with the same name." <<  endl;
				break;
			}
		}
	}

	if ( !fallBack )
	{
		if( groupNumber==0  )
		{	// The group #0 is an unremovable group. his default name is "~" ,
			// but the official client rename it i18n("others contact") at the first
			// connection.
			// In many case, the users don't use that group as a real group, or just as
			// a group to put all contact that are not sorted.
			fallBack = Kopete::Group::topLevel();
		}
		else
		{
			fallBack = new Kopete::Group( groupName );
			Kopete::ContactList::self()->addGroup( fallBack );
			kdDebug( 14140 ) << k_funcinfo << "We didn't found the group " << groupName <<" So we're creating a new one." <<  endl;

		}
	}

	fallBack->setPluginData( protocol(), accountId() + " id", QString::number( groupNumber ) );
	fallBack->setPluginData( protocol(), accountId() + " displayName", groupName );
	m_groupList.insert( groupNumber, fallBack );

	// We have pending groups that we need add a contact to
	if ( tmp_addToNewGroup.contains(groupName) )
	{
		QStringList list=tmp_addToNewGroup[groupName];
		for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
		{
			QString contactId =*it;
			kdDebug( 14140 ) << k_funcinfo << "Adding to new group: " << contactId <<  endl;
			notifySocket()->addContact( contactId, contacts()[ contactId ] ?
					contacts()[contactId]->property( Kopete::Global::Properties::self()->nickName()).value().toString()
					: contactId, groupNumber, MSNProtocol::FL );
		}
		tmp_addToNewGroup.remove(groupName);
	}
}

void MSNAccount::slotGroupRenamed( const QString& groupName, uint groupNumber )
{
	if ( m_groupList.contains( groupNumber ) )
	{
		m_groupList[ groupNumber ]->setPluginData( protocol(), accountId() + " id", QString::number( groupNumber ) );
		m_groupList[ groupNumber ]->setPluginData( protocol(), accountId() + " displayName", groupName );
		m_groupList[ groupNumber ]->setDisplayName( groupName );
	}
	else
	{
		slotGroupAdded( groupName, groupNumber );
	}
}

void MSNAccount::slotGroupRemoved( uint group )
{
	if ( m_groupList.contains( group ) )
	{
		// FIXME: we should really empty data in the group... but in most of cases, the group is already del
		// FIXME: Shouldn't we fix the memory management instead then??? - Martijn
		// m_groupList[ group ]->setPluginData( this, QStringList() );
		m_groupList.remove( group );
	}
}

void MSNAccount::addGroup( const QString &groupName, const QString& contactToAdd )
{
	if ( !contactToAdd.isNull()  )
	{
		if( tmp_addToNewGroup.contains(groupName) )
		{
			tmp_addToNewGroup[groupName].append(contactToAdd);
			//A group with the same name is about to be added,
			// we don't need to add a second group with the same name
			kdDebug( 14140 ) << k_funcinfo << "no need to re-add " << groupName << " for " << contactToAdd  <<  endl;
			return;
		}
		else
		{
			tmp_addToNewGroup.insert(groupName,QStringList(contactToAdd));
			kdDebug( 14140 ) << k_funcinfo << "preparing to add " << groupName << " for " << contactToAdd  <<  endl;
		}
	}

	if ( m_notifySocket )
		m_notifySocket->addGroup( groupName );

}

void MSNAccount::slotKopeteGroupRenamed( Kopete::Group *g )
{
	if ( notifySocket() && g->type() == Kopete::Group::Normal )
	{
		if ( !g->pluginData( protocol(), accountId() + " id" ).isEmpty() &&
			g->displayName() != g->pluginData( protocol(), accountId() + " displayName" ) &&
			m_groupList.contains( g->pluginData( protocol(), accountId() + " id" ).toUInt() ) )
		{
			notifySocket()->renameGroup( g->displayName(), g->pluginData( protocol(), accountId() + " id" ).toUInt() );
		}
	}
}

void MSNAccount::slotKopeteGroupRemoved( Kopete::Group *g )
{
	if ( !g->pluginData( protocol(), accountId() + " id" ).isEmpty() )
	{
		unsigned int groupNumber = g->pluginData( protocol(), accountId() + " id" ).toUInt();
		if ( !m_groupList.contains( groupNumber ) )
		{
			// the group is maybe already removed in the server
			slotGroupRemoved( groupNumber );
			return;
		}

		if ( groupNumber == 0 )
		{
			// the group #0 can't be deleted
			// then we set it as the top-level group
			if ( g->type() == Kopete::Group::TopLevel )
				return;

			Kopete::Group::topLevel()->setPluginData( protocol(), accountId() + " id", "0" );
			Kopete::Group::topLevel()->setPluginData( protocol(), accountId() + " displayName", g->pluginData( protocol(), accountId() + " displayName" ) );
			g->setPluginData( protocol(), accountId() + " id", QString::null ); // the group should be soon deleted, but make sure

			return;
		}

		if ( m_notifySocket )
		{
			/*  -not needed anymore  remember that contact are supposed to be deleted when a group is deleted.
			// if contact are contains only in the group we are removing, move it from the group 0
			QDictIterator<Kopete::Contact> it( contacts() );
			for ( ; it.current(); ++it )
			{
				MSNContact *c = static_cast<MSNContact *>( it.current() );
				if ( c->serverGroups().contains( groupNumber ) && c->serverGroups().count() == 1 )
					m_notifySocket->addContact( c->contactId(), c->displayName(), 0, MSNProtocol::FL );
			}*/
			m_notifySocket->removeGroup( groupNumber );
		}
		//this is also done later, but he have to do it now!
		// (in slotGroupRemoved)
		m_groupList.remove(groupNumber);
	}

	//remove it from the old list
	for(QMap<unsigned int, Kopete::Group*>::Iterator it=m_oldGroupList.begin() ; it != m_oldGroupList.end() ; ++it )
	{
		if(g==it.data())
		{
			m_oldGroupList.remove(it);
			break;
		}
	}
}

void MSNAccount::slotNewContactList()
{
		m_oldGroupList=m_groupList;
		for(QMap<unsigned int, Kopete::Group*>::Iterator it=m_oldGroupList.begin() ; it != m_oldGroupList.end() ; ++it )
		{	//they are about to be changed
			it.data()->setPluginData( protocol(), accountId() + " id", QString::null );
		}

		m_allowList.clear();
		m_blockList.clear();
		m_reverseList.clear();
		m_groupList.clear();
		KConfigGroup *config=configGroup();
		config->writeEntry( "blockList" , QString::null ) ;
		config->writeEntry( "allowList" , QString::null );
		config->writeEntry( "reverseList" , QString::null );

		// clear all date information which will be received.
		// if the information is not anymore on the server, it will not be received
		QDictIterator<Kopete::Contact> it( contacts() );
		for ( ; it.current(); ++it )
		{
			MSNContact *c = static_cast<MSNContact *>( *it );
			c->setBlocked( false );
			c->setAllowed( false );
			c->setReversed( false );
			c->setInfo( "PHH", QString::null );
			c->setInfo( "PHW", QString::null );
			c->setInfo( "PHM", QString::null );
		}
}

void MSNAccount::slotContactListed( const QString& handle, const QString& publicName, uint lists, const QString& group )
{
	// On empty lists handle might be empty, ignore that
	if ( handle.isEmpty() )
		return;

	MSNContact *c = static_cast<MSNContact *>( contacts()[ handle ] );

	if ( lists & 1 )	// FL
	{
		QStringList contactGroups = QStringList::split( ",", group, false );
		if ( c )
		{
			if( !c->metaContact() )
			{
				kdWarning( 14140 ) << k_funcinfo << "the contact " << c->contactId() << " has no meta contact" <<endl;
				Kopete::MetaContact *metaContact = new Kopete::MetaContact();

				c->setMetaContact(metaContact);
				Kopete::ContactList::self()->addMetaContact( metaContact );
			}

			// Contact exists, update data.
			// Merging difference between server contact list and Kopete::Contact's contact list into MetaContact's contact-list
			c->setOnlineStatus( MSNProtocol::protocol()->FLN );
			c->setProperty( Kopete::Global::Properties::self()->nickName() ,  publicName );

			const QMap<uint, Kopete::Group *> oldServerGroups = c->serverGroups();
			c->clearServerGroups();
			for ( QStringList::ConstIterator it = contactGroups.begin(); it != contactGroups.end(); ++it )
			{
				uint newServerGroupID =  ( *it ).toUInt();
				Kopete::Group *newServerGroup=m_groupList[ newServerGroupID ];
				c->contactAddedToGroup( newServerGroupID, newServerGroup );
				if( !c->metaContact()->groups().contains(newServerGroup) )
				{
					// The contact has been added in a group by another client
					c->metaContact()->addToGroup( newServerGroup , Kopete::MetaContact::DontSyncGroups  );
				}
			}

			for ( QMap<uint, Kopete::Group *>::ConstIterator it = oldServerGroups.begin(); it != oldServerGroups.end(); ++it )
			{
				Kopete::Group *old_group=m_oldGroupList[it.key()];
				if(old_group)
				{
					QString oldnewID=old_group->pluginData(protocol() , accountId() +" id");
					if ( !oldnewID.isEmpty() && contactGroups.contains( oldnewID ) )
						continue; //ok, it's correctn no need to do anything.

					c->metaContact()->removeFromGroup( old_group , Kopete::MetaContact::DontSyncGroups );
				}
			}

			// Update server if the contact has been moved to another group while MSN was offline
			c->syncGroups();
		}
		else
		{
			Kopete::MetaContact *metaContact = new Kopete::MetaContact();

			c = new MSNContact( this, handle, metaContact );
			c->setOnlineStatus( MSNProtocol::protocol()->FLN );
			c->setProperty( Kopete::Global::Properties::self()->nickName() , publicName );

			for ( QStringList::Iterator it = contactGroups.begin();
				it != contactGroups.end(); ++it )
			{
				uint groupNumber = ( *it ).toUInt();
				c->contactAddedToGroup( groupNumber, m_groupList[ groupNumber ] );
				metaContact->addToGroup( m_groupList[ groupNumber ] );
			}
			Kopete::ContactList::self()->addMetaContact( metaContact );
		}
	}
	else //the contact is _not_ in the FL, it has been removed
	{
		if(c)
		{
			c->setOnlineStatus( static_cast<MSNProtocol*>(protocol())->UNK );
			c->clearServerGroups();
			//TODO: display a message and suggest to remove the contact.
			//  but i fear a simple messageBox  QuestionYesNo here gives a nice crash.
			   //delete ct;
		}
	}
	if ( lists & 2 )
		slotContactAdded( handle, publicName, "AL", 0 );
	else if(c)
		c->setAllowed(false);
	if ( lists & 4 )
		slotContactAdded( handle, publicName, "BL", 0 );
	else if(c)
		c->setBlocked(false);
	if ( lists & 8 )
		slotContactAdded( handle, publicName, "RL", 0 );
	else if(c)
		c->setReversed(false);
}

void MSNAccount::slotContactAdded( const QString& handle, const QString& publicName,
	const QString& list, uint group )
{
	if ( list == "FL" )
	{
		bool new_contact = false;
		if ( !contacts()[ handle ] )
		{
			// FIXME: findContact() only works if the account id is an existing account - see Kopete::AccountManager::findAccount() for the reason
			Kopete::MetaContact *m = Kopete::ContactList::self()->findContact( protocol()->pluginId(), QString::null, handle );
			if ( m )
			{
				kdDebug( 14140 ) << k_funcinfo
					<< "Warning: the contact was found in the contactlist but not referenced in the protocol" << endl;
				MSNContact *c = static_cast<MSNContact *>( m->findContact( protocol()->pluginId(), QString::null, handle ) );
				c->contactAddedToGroup( group, m_groupList[ group ] );
			}
			else
			{
				new_contact = true;

				if ( m_addWizard_metaContact )
					m = m_addWizard_metaContact;
				else
					m = new Kopete::MetaContact();

				MSNContact *c = new MSNContact( this, handle, m );
				c->contactAddedToGroup( group, m_groupList[ group ] );
				c->setProperty( Kopete::Global::Properties::self()->nickName() , publicName);

				if ( !m_addWizard_metaContact )
				{
					m->addToGroup( m_groupList[ group ] );
					Kopete::ContactList::self()->addMetaContact( m );
				}
				c->setOnlineStatus( MSNProtocol::protocol()->FLN );

				m_addWizard_metaContact = 0L;
			}
		}
		if ( !new_contact )
		{
			MSNContact *c = static_cast<MSNContact *>( contacts()[ handle ] );
			if ( c->onlineStatus() == MSNProtocol::protocol()->UNK )
				c->setOnlineStatus( MSNProtocol::protocol()->FLN );

			if ( c->metaContact() && c->metaContact()->isTemporary() )
				c->metaContact()->setTemporary( false, m_groupList[ group ] );
			else
				c->contactAddedToGroup( group, m_groupList[ group ] );
		}

		if ( !m_allowList.contains( handle ) && !m_blockList.contains( handle ) )
			notifySocket()->addContact( handle, handle, 0, MSNProtocol::AL );
	}
	else if ( list == "BL" )
	{
		if ( contacts()[ handle ] )
			static_cast<MSNContact *>( contacts()[ handle ] )->setBlocked( true );
		if ( !m_blockList.contains( handle ) )
		{
			m_blockList.append( handle );
			configGroup()->writeEntry( "blockList" , m_blockList ) ;
		}
	}
	else if ( list == "AL" )
	{
		if ( contacts()[ handle ] )
			static_cast<MSNContact *>( contacts()[ handle ] )->setAllowed( true );
		if ( !m_allowList.contains( handle ) )
		{
			m_allowList.append( handle );
			configGroup()->writeEntry( "allowList" , m_allowList ) ;
		}
	}
	else if ( list == "RL" )
	{
		// search for new Contacts
		Kopete::Contact *ct=contacts()[ handle ];
		if ( !ct || !ct->metaContact() || ct->metaContact()->isTemporary() )
		{
			// Users in the allow list or block list now never trigger the
			// 'new user' dialog, which makes it impossible to add those here.
			// Not necessarily bad, but the usability effects need more thought
			// before I declare it good :- )
			if ( !m_allowList.contains( handle ) && !m_blockList.contains( handle ) )
			{
				NewUserImpl *authDlg = new NewUserImpl( 0 );
				authDlg->setHandle( handle, publicName );
				QObject::connect( authDlg, SIGNAL( addUser( const QString &, const QString& ) ),
					this, SLOT( slotAddContact( const QString &, const QString& ) ) );
				QObject::connect( authDlg, SIGNAL( blockUser( const QString& ) ),
					this, SLOT( slotBlockContact( const QString& ) ) );
				authDlg->show();
			}
		}
		else
		{
			static_cast<MSNContact *>( ct )->setReversed( true );
		}
		m_reverseList.append( handle );
		configGroup()->writeEntry( "reversekList" , m_reverseList ) ;
	}
}

void MSNAccount::slotContactRemoved( const QString& handle, const QString& list, uint group )
{
	if ( list == "BL" )
	{
		m_blockList.remove( handle );
		configGroup()->writeEntry( "blockList" , m_blockList ) ;
		if ( !m_allowList.contains( handle ) )
			notifySocket()->addContact( handle, handle, 0, MSNProtocol::AL );
	}
	else if ( list == "AL" )
	{
		m_allowList.remove( handle );
		configGroup()->writeEntry( "allowList" , m_allowList ) ;
		if ( !m_blockList.contains( handle ) )
			notifySocket()->addContact( handle, handle, 0, MSNProtocol::BL );
	}
	else if ( list == "RL" )
	{
		m_reverseList.remove( handle );
		configGroup()->writeEntry( "reversekList" , m_reverseList ) ;
	}

	MSNContact *c = static_cast<MSNContact *>( contacts()[ handle ] );
	if ( c )
	{
		if ( list == "RL" )
		{
			// Contact is removed from the reverse list
			// only MSN can do this, so this is currently not supported
			c->setReversed( false );
/*
			InfoWidget *info = new InfoWidget( 0 );
			info->title->setText( "<b>" + i18n( "Contact removed!" ) +"</b>" );
			QString dummy;
			dummy = "<center><b>" + imContact->getPublicName() + "( " +imContact->getHandle()  +" )</b></center><br>";
			dummy += i18n( "has removed you from his contact list!" ) + "<br>";
			dummy += i18n( "This contact is now removed from your contact list" );
			info->infoText->setText( dummy );
			info->setCaption( "KMerlin - Info" );
			info->show();
*/
		}
		else if ( list == "FL" )
		{
			// Contact is removed from the FL list, remove it from the group
			c->contactRemovedFromGroup( group );
		}
		else if ( list == "BL" )
		{
			c->setBlocked( false );
		}
		else if ( list == "AL" )
		{
			c->setAllowed( false );
		}
	}
}

void MSNAccount::slotCreateChat( const QString& address, const QString& auth )
{
	slotCreateChat( 0L, address, auth, m_msgHandle, m_msgHandle );
}

void MSNAccount::slotCreateChat( const QString& ID, const QString& address, const QString& auth,
	const QString& handle_, const QString&  publicName )
{
	QString handle = handle_.lower();

	if ( handle.isEmpty() )
	{
		// we have lost the handle?
		// forget it
		// ( that can be because the user try to open two swichboard in the same time )
		return;
	}

//	kdDebug( 14140 ) << k_funcinfo <<"Creating chat for " << handle << endl;

	if ( !contacts()[ handle ] )
		addContact( handle, publicName, 0L, Kopete::Account::DontChangeKABC, QString::null, true );

	MSNContact *c = static_cast<MSNContact *>( contacts()[ handle ] );

	if ( c && myself() )
	{
		// we can't use simply c->manager(true) here to get the manager, because this will re-open
		// another chat session, and then, close this new one. We have to re-create the manager manualy.
		MSNMessageManager *manager = dynamic_cast<MSNMessageManager*>( c->manager( false ) );
		if(!manager)
		{
			Kopete::ContactPtrList chatmembers;
			chatmembers.append(c);
			manager = new MSNMessageManager( protocol(), myself(), chatmembers  );
		}

		manager->createChat( handle, address, auth, ID );

		KGlobal::config()->setGroup( "MSN" );
		bool notifyNewChat = KGlobal::config()->readBoolEntry( "NotifyNewChat", false );
		if ( !ID.isEmpty() && notifyNewChat )
		{
			// this temporary message should open the window if they not exist
			QString body = i18n( "%1 has started a chat with you" ).arg( c->metaContact()->displayName() );
			Kopete::Message tmpMsg = Kopete::Message( c, manager->members(), body, Kopete::Message::Internal, Kopete::Message::PlainText );
			manager->appendMessage( tmpMsg );
		}
	}

	m_msgHandle = QString::null;
}

void MSNAccount::slotStartChatSession( const QString& handle )
{
	// First create a message manager, because we might get an existing
	// manager back, in which case we likely also have an active switchboard
	// connection to reuse...

	if ( !m_msgHandle.isNull() )
	{
		// Hep hep hep l'ami! I am not crazy! i know you are trying to open
		// a new chat session, but the previous one is not yet created.
		// if it is with the same m_msgHandle, that is certenely because you
		// are impatient, but you have to wait the server reply
		if ( m_msgHandle == handle )
			return;

		// else, you are trying to open two sessions in the same time
		// it will have conflict. but i can't return, may be you ask a signle
		// session because the previous one was not created due to a randomely error
		// never mind then, i don't care
	}

	MSNContact *c = static_cast<MSNContact *>( contacts()[ handle ] );
	// if ( isConnected() && c && myself() && handle != m_msnId )
	if ( m_notifySocket && c && myself() && handle != accountId() )
	{
		if ( !c->manager() || !static_cast<MSNMessageManager *>( c->manager() )->service() )
		{
			m_msgHandle = handle;
			m_notifySocket->createChatSession();
		}
	}
}

void MSNAccount::slotBlockContact( const QString &handle )
{
	if ( m_notifySocket )
	{
		if ( m_allowList.contains( handle ) )
			m_notifySocket->removeContact( handle, 0, MSNProtocol::AL );
		else if ( !m_blockList.contains( handle ) )
			m_notifySocket->addContact( handle, handle, 0, MSNProtocol::BL );
	}
}

void MSNAccount::slotAddContact( const QString &userName, const QString &displayName )
{
	if ( m_notifySocket )
		m_notifySocket->addContact( userName, displayName, 0, MSNProtocol::FL );
}

bool MSNAccount::createContact( const QString &contactId, Kopete::MetaContact *metaContact )
{
	if ( m_notifySocket )
	{
		if ( !metaContact->isTemporary() )
		{
			m_addWizard_metaContact = metaContact;
			// This is a normal contact. Get all the groups this MetaContact is in
			bool added = false;
			QPtrList<Kopete::Group> groupList = metaContact->groups();
			for ( Kopete::Group *group = groupList.first(); group; group = groupList.next() )
			{
				// For each group, ensure it is on the MSN server
				if ( !group->pluginData( protocol(), accountId() + " id" ).isEmpty() )
				{
					int Gid=group->pluginData( protocol(), accountId() + " id" ).toUInt();
					if(!m_groupList.contains(Gid))
					{ // ohoh!   something is corrupted on the contactlist.xml
					  // anyway, we never should add a contact to an unexisting group on the server.

						//repair the problem
						group->setPluginData( protocol() , accountId() + " id" , QString::null);
						group->setPluginData( protocol() , accountId() + " displayName" , QString::null);
						kdDebug( 14140 ) << k_funcinfo << " Group " << group->displayName() << " marked with id #" <<Gid << " does not seems to be anymore on the server" << endl;

					}
					else
					{
						// Add the contact to the group on the server
						m_notifySocket->addContact( contactId, metaContact->displayName(), Gid, MSNProtocol::FL );
						added = true;
					}
				}
				if(!added)
				{
					if ( !group->displayName().isEmpty() && group->type() == Kopete::Group::Normal )
					{  // not the top-level
						// Create the group and add the contact
						// FIXME: if for a reason or another the group can't be added, the contact will not be added.
						addGroup( group->displayName(), contactId );
						added = true;
					}
				}
			}
			if ( !added )
			{
				// only on top-level, or in no groups ( add it to the default group )
				m_notifySocket->addContact( contactId, metaContact->displayName(), 0, MSNProtocol::FL );
			}

			// FIXME: Find out if this contact was really added or not!
			return true;
		}
		else
		{
			// This is a temporary contact. ( a person who messaged us but is not on our conntact list.
			// We don't want to create it on the server.Just create the local contact object and add it
			MSNContact *newContact = new MSNContact( this, contactId, metaContact );
			return ( newContact != 0L );
		}
	}
	// else // We aren't connected! Can't add a contact
	// FIXME: add contact when offline, and sync with server
	return false;
}

bool MSNAccount::isHotmail() const
{
	if ( !m_openInboxAction )
		return false;
	return m_openInboxAction->isEnabled();
}

QString MSNAccount::pictureObject()
{
	if(m_pictureObj.isNull())
		resetPictureObject(true); //silent=true to keep infinite loop away
	return m_pictureObj;
}

void MSNAccount::resetPictureObject(bool silent)
{
	QString old=m_pictureObj;

	if(configGroup()->readBoolEntry("exportCustomPicture"))
		m_pictureObj="";
	else
	{
		QFile pictFile(locateLocal( "appdata", "msnpicture-"+ accountId().lower().replace(QRegExp("[./~]"),"-")  +".png"  ) );
		if(!pictFile.open(IO_ReadOnly))
			m_pictureObj="";
		else
		{
			QByteArray ar=pictFile.readAll();
			QString sha1d= QString((KCodecs::base64Encode(SHA1::hash(ar))));

			QString size=QString::number( pictFile.size() );
			QString all= "Creator"+accountId()+"Size"+size+"Type3LocationTFR2C.tmpFriendlyAAA=SHA1D"+ sha1d;
			m_pictureObj="<msnobj Creator=\"" + accountId() + "\" Size=\"" + size  + "\" Type=\"3\" Location=\"TFR2C.tmp\" Friendly=\"AAA=\" SHA1D=\""+sha1d+"\" SHA1C=\""+ QString(KCodecs::base64Encode(SHA1::hashString(all.utf8())))  +"\"/>";
		}
	}

	if(old!=m_pictureObj && isConnected() && m_notifySocket && !silent)
	{
		//update the msn pict
		m_notifySocket->setStatus( myself()->onlineStatus() );
	}
}


#include "msnaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:

