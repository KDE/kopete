/*
    msnaccount.h - Manages a single MSN account

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <klineeditdlg.h>
#include <kmessagebox.h>
#include <kiconloader.h>

#include "kopeteonlinestatus.h"

#include "msnaccount.h"
#include "msnprotocol.h"
#include "msncontact.h"
#include "msnnotifysocket.h"
#include "msnmessagemanager.h"
#include "msnpreferences.h"
#include "newuserimpl.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopetegroup.h"
#include "kopetemessage.h"
#include "kopeteviewmanager.h"

MSNIdentity::MSNIdentity( MSNProtocol *parent, const QString& IdentityID, const char *name )
: KopeteIdentity ( parent, IdentityID , name )
{
	m_notifySocket = 0L;
	m_connectstatus = MSNProtocol::protocol()->NLN;
	m_addWizard_metaContact=0L;

	// Init the myself contact
	// FIXME: I think we should add a global self metaContact (Olivier)
	m_myself = new MSNContact( this, identityId(), identityId(), 0L );
	m_myself->setOnlineStatus( MSNProtocol::protocol()->FLN );

	QObject::connect( KopeteContactList::contactList(), SIGNAL( groupRenamed( KopeteGroup *, const QString & ) ),
		SLOT( slotKopeteGroupRenamed( KopeteGroup * ) ) );
	QObject::connect( KopeteContactList::contactList(), SIGNAL( groupRemoved( KopeteGroup * ) ),
		SLOT( slotKopeteGroupRemoved( KopeteGroup * ) ) );

	//initActions
	m_actionMenu = new KActionMenu( IdentityID, this );
	m_menuTitleId=m_actionMenu->popupMenu()->insertTitle( i18n( "%2 <%1>" ).arg(identityId()).arg(m_myself->displayName()) );

	KAction* actionGoOnline = new KAction ( i18n("Go O&nline"), "msn_online", 0, this, SLOT(slotGoOnline()), m_actionMenu, "actionMSNConnect" );
	KAction* actionGoOffline = new KAction ( i18n("Go &Offline"), "msn_offline", 0, this, SLOT(slotGoOffline()), m_actionMenu, "actionMSNConnect" );
	KAction* actionGoAway = new KAction ( i18n("Set &Away"), "msn_away", 0, this, SLOT(slotGoAway()), m_actionMenu, "actionMSNConnect" );
	KAction* actionGoBusy = new KAction ( i18n("Set &Busy"), "msn_na", 0, this, SLOT(slotGoBusy()), m_actionMenu, "actionMSNConnect" );
	KAction* actionGoBeRightBack = new KAction ( i18n("Set Be &Right Back"), "msn_away", 0, this, SLOT(slotGoBeRightBack()), m_actionMenu, "actionMSNConnect" );
	KAction* actionGoOnThePhone = new KAction ( i18n("Set on the &Phone"), "msn_na", 0, this, SLOT(slotGoOnThePhone()), m_actionMenu, "actionMSNConnect" );
	KAction* actionGoOutToLunch = new KAction ( i18n("Set Out to &Lunch"), "msn_away", 0, this, SLOT(slotGoOutToLunch()), m_actionMenu, "actionMSNConnect" );
	KAction* actionGoInvisible = new KAction ( i18n("Set &Invisible"), "msn_offline", 0, this, SLOT(slotGoInvisible()), m_actionMenu, "actionMSNConnect" );

	KAction* renameAction = new KAction ( i18n( "&Change Nickname..." ), QString::null, 0, this, SLOT( slotChangePublicName() ), m_actionMenu, "renameAction" );
	KAction* startChatAction = new KAction ( i18n( "&Start Chat..." ), "mail_generic", 0, this, SLOT( slotStartChat() ), m_actionMenu, "startChatAction" );

	m_openInboxAction = new KAction ( i18n( "Open Inbo&x" ), "mail_generic", 0, this, SLOT( slotOpenInbox() ), m_actionMenu, "m_openInboxAction" );
	m_openInboxAction->setEnabled(false);

	m_actionMenu->insert( actionGoOnline );
	m_actionMenu->insert( actionGoAway );
	m_actionMenu->insert( actionGoBusy );
	m_actionMenu->insert( actionGoBeRightBack );
	m_actionMenu->insert( actionGoOnThePhone );
	m_actionMenu->insert( actionGoOutToLunch );
	m_actionMenu->insert( actionGoInvisible );
	m_actionMenu->insert( actionGoOffline );

	m_actionMenu->popupMenu()->insertSeparator();
	m_actionMenu->insert( renameAction );
	m_actionMenu->insert( startChatAction );
	m_actionMenu->popupMenu()->insertSeparator();
	m_actionMenu->insert( m_openInboxAction );


#if !defined NDEBUG
	KActionMenu *debugMenu = new KActionMenu( "Debug", m_actionMenu );
	KAction *debugRawCommand = new KAction( i18n( "Send Raw C&ommand..." ), 0,
		this, SLOT( slotDebugRawCommand() ), debugMenu, "m_debugRawCommand" );
	debugMenu->insert( debugRawCommand );
	m_actionMenu->popupMenu()->insertSeparator();
	m_actionMenu->insert( debugMenu );
#endif

}

MSNIdentity::~MSNIdentity()
{
}

void MSNIdentity::loaded()
{
	QString publicName= pluginData(protocol(),identityId() + QString::fromLatin1(" displayName"));
	if(!publicName.isNull())
		m_myself->setDisplayName(publicName);
}

void MSNIdentity::setAway( bool away )
{
	if( away )
		setOnlineStatus( MSNProtocol::protocol()->IDL );
	else //if( m_myself->onlineStatus() == MSNProtocol::statusIDL )
		setOnlineStatus( MSNProtocol::protocol()->NLN );
}

KopeteContact* MSNIdentity::myself() const
{
	return m_myself;
}

void MSNIdentity::connect()
{
	if( isConnected() )
	{
		kdDebug(14140) << "MSNIdentity::connect: Ignoring Connect request "
			<< "(Already Connected)" << endl;
		return;
	}

	if(m_notifySocket)
	{
		kdDebug(14140) << "MSNIdentity::connect: Ignoring Connect request (Already connecting)"  <<endl;
		return;
	}


	QString passwd=getPassword();
	if(passwd.isNull())
	{
		kdDebug(14140) << "MSNIdentity::connect: Abort connection (null password)"  <<endl;
		return;
	}

	//FIXME: don't use protocol here
	m_notifySocket = new MSNNotifySocket( this, identityId() );

	QObject::connect( m_notifySocket, SIGNAL( groupAdded( QString, uint,uint ) ),
		SLOT( slotGroupAdded( QString, uint ) ) );
	QObject::connect( m_notifySocket, SIGNAL( groupRenamed( QString, uint, uint ) ),
		SLOT( slotGroupRenamed( QString, uint ) ) );
	QObject::connect( m_notifySocket, SIGNAL( groupListed( QString, uint ) ),
		SLOT( slotGroupAdded( QString, uint ) ) );
	QObject::connect( m_notifySocket, SIGNAL(groupRemoved( uint, uint ) ),
		SLOT( slotGroupRemoved( uint ) ) );
	QObject::connect( m_notifySocket, SIGNAL( contactList( QString, QString, QString, QString ) ),
		SLOT( slotContactListed( QString, QString, QString, QString ) ) );
	QObject::connect( m_notifySocket, SIGNAL( contactAdded( QString, QString, QString, uint, uint ) ),
		SLOT( slotContactAdded( QString, QString, QString, uint, uint ) ) );
	QObject::connect( m_notifySocket, SIGNAL( contactRemoved( QString, QString, uint, uint ) ),
		SLOT( slotContactRemoved( QString, QString, uint, uint ) ) );
	QObject::connect( m_notifySocket, SIGNAL( statusChanged( const KopeteOnlineStatus & ) ),
		SLOT( slotStatusChanged( const KopeteOnlineStatus & ) ) );
	QObject::connect( m_notifySocket, SIGNAL( onlineStatusChanged( MSNSocket::OnlineStatus ) ),
		SLOT( slotNotifySocketStatusChanged( MSNSocket::OnlineStatus ) ) );
	QObject::connect( m_notifySocket, SIGNAL( publicNameChanged( QString ) ),
		SLOT( slotPublicNameChanged( QString ) ) );
	QObject::connect( m_notifySocket, SIGNAL( invitedToChat( QString, QString, QString, QString, QString ) ),
		SLOT( slotCreateChat( QString, QString, QString, QString, QString ) ) );
	QObject::connect( m_notifySocket, SIGNAL( startChat( QString, QString ) ),
		SLOT( slotCreateChat( QString, QString ) ) );
	QObject::connect( m_notifySocket, SIGNAL( socketClosed( int ) ),
		SLOT( slotNotifySocketClosed( int ) ) );
	QObject::connect( m_notifySocket, SIGNAL( hotmailSeted( bool ) ),
		m_openInboxAction, SLOT( setEnabled( bool ) ) );

	m_notifySocket->setStatus( m_connectstatus );
	m_notifySocket->connect( passwd );
	m_myself->setOnlineStatus( MSNProtocol::protocol()->CNT );
	m_openInboxAction->setEnabled(false);

}
void MSNIdentity::disconnect()
{
	if( m_notifySocket )
		m_notifySocket->disconnect();
}



KActionMenu* MSNIdentity::actionMenu()
{
	m_actionMenu->setIcon( m_myself->onlineStatus().icon() );
	m_actionMenu->setText( i18n( "%2 <%1>" ).arg( identityId() ).arg( m_myself->displayName() ) );
	m_actionMenu->popupMenu()->changeTitle( m_menuTitleId, SmallIcon( m_myself->onlineStatus().icon() ),
		i18n( "%2 <%1>" ).arg( identityId() ).arg( m_myself->displayName() ) );

	return m_actionMenu;
}

MSNNotifySocket *MSNIdentity::notifySocket()
{
	return m_notifySocket;
}


void MSNIdentity::slotGoOnline()
{
	//m_connectstatus=NLN;
	if( !isConnected() )
		connect();
	else
		setOnlineStatus( MSNProtocol::protocol()->NLN );
}

void MSNIdentity::slotGoOffline()
{
	disconnect();
	//m_connectstatus=NLN;
}

void MSNIdentity::slotGoAway()
{
	setOnlineStatus( MSNProtocol::protocol()->AWY );
}

void MSNIdentity::slotGoBusy()
{
	setOnlineStatus( MSNProtocol::protocol()->BSY );
}

void MSNIdentity::slotGoBeRightBack()
{
	setOnlineStatus( MSNProtocol::protocol()->BRB );
}

void MSNIdentity::slotGoOnThePhone()
{
	setOnlineStatus( MSNProtocol::protocol()->PHN );
}

void MSNIdentity::slotGoOutToLunch()
{
	setOnlineStatus( MSNProtocol::protocol()->LUN );
}

void MSNIdentity::slotGoInvisible()
{
	setOnlineStatus( MSNProtocol::protocol()->HDN );
}

void MSNIdentity::setOnlineStatus( const KopeteOnlineStatus &status )
{
	if (m_notifySocket )
	{
		m_notifySocket->setStatus( status );
	}
	else
	{
		m_connectstatus = status;
		connect();
	}
}

void MSNIdentity::slotStartChat()
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
		i18n( "Start Chat - MSN Plugin" ),
		i18n( "Please enter the email address of the person with whom you want to chat:" ),
		QString::null, &ok );
	if( ok )
	{
		if( handle.contains('@') ==1 && handle.contains('.') >=1)
		{
			m_msgHandle = handle;
			// don't crash when we were disconnected before we got the address
			if ( m_notifySocket )
				m_notifySocket->createChatSession();
		}
		else
		{
			KMessageBox::error(0l, i18n("<qt>You must enter a valid e-mail address</qt>"), i18n("MSN Plugin"));
		}
	}
}

void MSNIdentity::slotDebugRawCommand()
{
#if !defined NDEBUG
//TODO:
/*	if ( !isConnected() )
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
	delete dlg;*/
#endif
}


void MSNIdentity::slotChangePublicName()
{
	bool ok;
	QString name = KLineEditDlg::getText( i18n( "Change Nickname - MSN Plugin - Kopete" ),
		i18n( "Enter the new public name by which you want to be visible to your friends on MSN:" ),
		m_myself->displayName(), &ok );

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
			//m_publicNameSyncMode = SyncToServer;
		}
	}
}

void MSNIdentity::slotOpenInbox()
{
	if(m_notifySocket)
		m_notifySocket->slotOpenInbox();
}

void MSNIdentity::slotNotifySocketStatusChanged( MSNSocket::OnlineStatus status )
{
	//TODO:
	if ( status == MSNSocket::Connected )
	{
		// Sync public name when needed
		/*if( m_publicNameSyncNeeded )
		{
			kdDebug(14140) << "MSNProtocol::slotOnlineStatusChanged: Syncing public name to "
				<< m_publicName << endl;
			setPublicName( m_publicName );
			m_publicNameSyncNeeded = false;
		}
		else
		{
			kdDebug(14140) << "MSNProtocol::slotOnlineStatusChanged: Leaving public name as "
				<< m_publicName << endl;
		}
		// Now pending changes are updated if we want to sync both ways
		m_publicNameSyncMode = SyncBoth;
		*/
//		setStatusIcon( "msn_online" );
	}
	else if( status == MSNSocket::Disconnected )
	{
		/*KopeteMessageManagerDict sessions =
			KopeteMessageManagerFactory::factory()->protocolSessions( this );
		QIntDictIterator<KopeteMessageManager> kmmIt( sessions );
		for( ; kmmIt.current() ; ++kmmIt )
		{
			// Disconnect all active chats (but don't actually remove the
			// chat windows, the user might still want to view them!)
			MSNMessageManager *msnMM =
				dynamic_cast<MSNMessageManager *>( kmmIt.current() );
			if( msnMM )
			{
				kdDebug(14140) << "MSNProtocol::slotOnlineStatusChanged: "
					<< "Closed MSNMessageManager because the protocol socket "
					<< "closed." << endl;
				msnMM->slotCloseSession();
			}
		}*/

		//FIXME: only do it for contact for this identity
		QDictIterator<KopeteContact> it( contacts() );
		for ( ; it.current() ; ++it )
			( *it )->setOnlineStatus( MSNProtocol::protocol()->FLN );

		m_allowList.clear();
		m_blockList.clear();
		m_groupList.clear();

//		setStatusIcon( "msn_offline" );
		m_openInboxAction->setEnabled(false);

		// Reset flags. They can't be set in the connect method, because
		// offline changes might have been made before. Instead the c'tor
		// sets the defaults, and the disconnect slot resets those defaults
		// FIXME: Can't we share this code?
		//m_publicNameSyncMode = SyncFromServer;
	}
	else if( status == MSNSocket::Connecting )
	{
//		for( QDictIterator<KopeteContact> it( contacts() ); it.current() ; ++it )
//			static_cast<MSNContact *>( *it )->setMsnStatus( MSNProtocol::FLN );
	}
	//static_cast<MSNProtocol*>(protocol())->slotNotifySocketStatusChanged(status);
}

void MSNIdentity::slotNotifySocketClosed( int /*state*/ )
{
	kdDebug(14140) << "MSNIdentity::slotNotifySocketClosed" << endl;
	//FIXME: Kopete crash when i show this message box...
/*	if ( state == 0x10 ) // connection died unexpectedly
	{
		KMessageBox::error( qApp->mainWidget(), i18n( "Connection with the MSN server was lost unexpectedly.\nIf you are unable to reconnect, please try again later." ), i18n( "Connection lost - MSN Plugin - Kopete" ) );
	}*/
	m_notifySocket->deleteLater();
	m_notifySocket=0l;
	m_myself->setOnlineStatus( MSNProtocol::protocol()->FLN );
	m_openInboxAction->setEnabled(false);
//	kdDebug(14140) << "MSNIdentity::slotNotifySocketClosed - done" << endl;
}

void MSNIdentity::slotStatusChanged( const KopeteOnlineStatus &status )
{
	kdDebug(14140) << "MSNIdentity::slotStatusChanged: " << status.internalStatus() <<  endl;
	m_myself->setOnlineStatus( status );
}

void MSNIdentity::slotPublicNameChanged( QString publicName )
{
	if( publicName != m_myself->displayName() )
	{
		//if( m_publicNameSyncMode & SyncFromServer )
		//{
			m_myself->setDisplayName( publicName );
			setPluginData( protocol(),identityId() + QString::fromLatin1( " displayName" ), publicName );

/*			m_publicNameSyncMode = SyncBoth;
		}
		else
		{
			// Check if name differs, and schedule sync if needed
			if( m_publicNameSyncMode & SyncToServer )
				m_publicNameSyncNeeded = true;
			else
				m_publicNameSyncNeeded = false;
		}*/
	}
}

void MSNIdentity::setPublicName( const QString &publicName )
{
	if(m_notifySocket)
	{
		m_notifySocket->changePublicName( publicName );
	}
}

void MSNIdentity::slotGroupAdded( QString groupName, uint groupNumber )
{
	// We have pending groups that we need add a contact to
	if( tmp_addToNewGroup.count() > 0 )
	{
		for( QValueList<QPair<QString,QString> >::Iterator it = tmp_addToNewGroup.begin(); it != tmp_addToNewGroup.end(); ++it )
		{
			if( ( *it ).second == groupName )
			{
				kdDebug( 14140 ) << k_funcinfo << "Adding to new group: " << ( *it ).first <<  endl;
				notifySocket()->addContact( ( *it ).first, ( *it ).first, groupNumber, MSNProtocol::FL );
			}
		}

		// FIXME: Although we check for groupName above we clear regardless of the outcome? :) - Martijn
		tmp_addToNewGroup.clear();
	}

	if( m_groupList.contains( groupNumber ) )
	{
		// Group can already be in the list since the idle timer does a 'List Groups'
		// command. Simply return, don't issue a warning.
		// kdDebug( 14140 ) << k_funcinfo << "Group " << groupName << " already in list, skipped." << endl;
		return;
	}

	// Find the appropriate KopeteGroup, or create one
	QPtrList<KopeteGroup> groupList = KopeteContactList::contactList()->groups();
	KopeteGroup *fallBack = 0L;
	for( KopeteGroup *g = groupList.first(); g; g = groupList.next() )
	{
		//FIXME!!!!!  a different mapping for each identities will be welcome!
		if( !g->pluginData( protocol() ,identityId() +  " id" ).isEmpty() )
		{
			if( g->pluginData(protocol(),identityId() + " id").toUInt() == groupNumber )
			{
				m_groupList.insert( groupNumber, g );
				QString oldGroupName;
				if( g->pluginData(protocol(),identityId() + " displayName") != groupName )
				{
					// The displayName of the group has been modified by another client
					slotGroupRenamed( groupName, groupNumber );
				}
				else if( g->displayName() != groupName )
				{
					// The displayName was changed in Kopete while we were offline
					// FIXME: update the server right now
				}
				return;
			}
		}
		else
		{
			// If we found a group with the same displayName but no plugin data
			// use that instead. This group is only used if no exact match is
			// found in the list.
			// FIXME: When adding a group in Kopete we already need to inform the
			//        plugins about the KopeteGroup*, so they know which to use
			//        and this code path can be removed (or kept solely for people
			//        who migrate from older versions) - Martijn
			if( g->displayName() == groupName )
				fallBack = g;
		}
	}

	if( !fallBack )
	{
		fallBack = new KopeteGroup( groupName );
		KopeteContactList::contactList()->addGroup( fallBack );
	}

	fallBack->setPluginData( protocol(),identityId() + " id", QString::number( groupNumber ) );
	fallBack->setPluginData( protocol(),identityId() + " displayName", groupName );
	m_groupList.insert( groupNumber, fallBack );
}

void MSNIdentity::slotGroupRenamed( QString groupName, uint groupNumber )
{
	if( m_groupList.contains( groupNumber ) )
	{
		m_groupList[ groupNumber ]->setPluginData( protocol(),identityId() + " id", QString::number( groupNumber ) );
		m_groupList[ groupNumber ]->setPluginData( protocol(),identityId() + " displayName", groupName );
		m_groupList[ groupNumber ]->setDisplayName( groupName );
	}
	else
	{
		slotGroupAdded( groupName, groupNumber );
	}
}

void MSNIdentity::slotGroupRemoved( uint group )
{
	if( m_groupList.contains( group ) )
	{
		// FIXME: we should realy emty data in the group... but in most of cases, the group is already del
		// FIXME: Shouldn't we fix the memory management instead then??? - Martijn
		//m_groupList[ group ]->setPluginData( this, QStringList() );
		m_groupList.remove( group );
	}
}

void MSNIdentity::addGroup( const QString &groupName , const QString& contactToAdd )
{
	if(!contactToAdd.isNull())
		tmp_addToNewGroup << QPair<QString,QString>(contactToAdd,groupName);

	if(m_notifySocket)
		m_notifySocket->addGroup( groupName );
}

void MSNIdentity::slotKopeteGroupRenamed(KopeteGroup *g)
{
	if(g->type()==KopeteGroup::Classic)
	{
		if(!g->pluginData(protocol(),identityId() + " id").isEmpty())
		{
			if( m_groupList.contains( g->pluginData(protocol(),identityId() + " id").toUInt() ) )
				notifySocket()->renameGroup( g->displayName(), g->pluginData(protocol(), identityId() + " id").toUInt() );
		}
	}
}

void MSNIdentity::slotKopeteGroupRemoved(KopeteGroup *g)
{
	if(!g->pluginData(protocol(),identityId() + " id").isEmpty())
	{
		unsigned int groupNumber=g->pluginData(protocol(), identityId() + " id").toUInt();
		if( !m_groupList.contains( groupNumber ) )
		{
			//the group is maybe already removed in the server
			slotGroupRemoved(groupNumber);
			return;
		}

		if ( groupNumber==0 )
		{
			//the group #0 can't be deleted
			//then we set it as the top-level group
			if(g == KopeteGroup::toplevel)
				return;

			KopeteGroup::toplevel->setPluginData(protocol(), identityId() + " id","0");
			KopeteGroup::toplevel->setPluginData(protocol(),identityId() + " displayName",g->pluginData(protocol(), identityId() + " displayName"));
			g->setPluginData(protocol(), identityId() + " id",QString::null); //the group should be soon deleted, but make sure

			return;
		}
		
		if(m_notifySocket)
		{
			//if contact are contains only in the group we are removing, move it from the group 0
			//FIXME: use only contact for this identity
			QDictIterator<KopeteContact> it( contacts() );
			for ( ; it.current() ; ++it )
			{
				MSNContact *c = static_cast<MSNContact *>( it.current() );
				if( c->serverGroups().contains( groupNumber ) && c->serverGroups().count() == 1 )
					m_notifySocket->addContact( c->contactId(), c->displayName(), 0, MSNProtocol::FL );
			}
			m_notifySocket->removeGroup( groupNumber );
		}
	}
}

void MSNIdentity::slotContactListed( QString handle, QString publicName, QString group, QString list )
{
	// On empty lists handle might be empty, ignore that
	if( handle.isEmpty() )
		return;

	QStringList contactGroups = QStringList::split( ",", group, false );
	if( list == "FL" )
	{
		KopeteMetaContact *metaContact = KopeteContactList::contactList()->findContact( protocol()->pluginId(), identityId(), handle );
		if( metaContact )
		{
			// Contact exists, update data.
			// Merging difference between server contact list and KopeteContact's contact list into MetaContact's contact-list
			//kdDebug( 14140 ) << k_funcinfo << "******* updating existing contact: " << handle << "!" << endl;
			MSNContact *c = static_cast<MSNContact*>( metaContact->findContact( protocol()->pluginId(), identityId(), handle ) );
			c->setOnlineStatus( MSNProtocol::protocol()->FLN );
			c->setDisplayName( publicName ); 

			const QMap<uint, KopeteGroup *>serverGroups = c->serverGroups();
			for( QStringList::ConstIterator it = contactGroups.begin(); it != contactGroups.end(); ++it )
			{
				uint serverGroup = ( *it ).toUInt();
				if( !serverGroups.contains( serverGroup ) )
				{
					// The contact has been added in a group by another client
					c->contactAddedToGroup( serverGroup, m_groupList[ serverGroup ] );
					metaContact->addToGroup( m_groupList[ serverGroup ] );
				}
			}

			for( QMap<uint, KopeteGroup *>::ConstIterator it = serverGroups.begin(); it != serverGroups.end(); ++it )
			{
				if( !contactGroups.contains( QString::number( it.key() ) ) )
				{
					// The contact has been removed from a group by another client
					c->removeFromGroup( it.key() );
					metaContact->removeFromGroup( m_groupList[ it.key() ] );
				}
			}

			// FIXME: Update server if the contact has been moved to another group while MSN was offline
		}
		else
		{
			metaContact = new KopeteMetaContact();
	
			MSNContact *msnContact = new MSNContact( this, handle, publicName, metaContact );
			msnContact->setOnlineStatus( MSNProtocol::protocol()->FLN );

			for( QStringList::Iterator it = contactGroups.begin();
				it != contactGroups.end(); ++it )
			{
				uint groupNumber = ( *it ).toUInt();
				msnContact->contactAddedToGroup( groupNumber, m_groupList[ groupNumber ] );
				metaContact->addToGroup( m_groupList[ groupNumber ] );
			}
			KopeteContactList::contactList()->addMetaContact( metaContact );
		}
	}
	else
	{
		//FIXME: merge theses two method
		slotContactAdded(handle , publicName , list , 0 , 0 );
	}
	
}

void MSNIdentity::slotContactAdded( QString handle, QString publicName,
	QString list, uint  /*serial*/ , uint group )
{
	if( list == "FL" )
	{
		bool new_contact=false;
		if( !contacts()[ handle ] )
		{
			KopeteMetaContact *m = KopeteContactList::contactList()->findContact( protocol()->pluginId(), QString::null, handle );
			if(m)
			{
				kdDebug(14140) << "MSNProtocol::slotContactAdded: Warning: the contact was found in the contactlist but not referenced in the protocol" <<endl;
				MSNContact *c = static_cast<MSNContact*>(m->findContact( protocol()->pluginId(), QString::null, handle ));
				c->contactAddedToGroup( group, m_groupList[ group ] );
			}
			else
			{
				new_contact=true;

				if(m_addWizard_metaContact)
					m=m_addWizard_metaContact;
				else
					m=new KopeteMetaContact();

				MSNContact *c = new MSNContact( this, handle, publicName, m );
				c->contactAddedToGroup( group, m_groupList[ group ] );

				if(!m_addWizard_metaContact)
				{
					m->addToGroup(m_groupList[group]);
					KopeteContactList::contactList()->addMetaContact(m);
				}
				c->setOnlineStatus( MSNProtocol::protocol()->FLN );

				m_addWizard_metaContact=0L;
			}
		}
		if(!new_contact)
		{
			MSNContact *c = static_cast<MSNContact *>( contacts()[ handle ] );
			if( c->onlineStatus() == MSNProtocol::protocol()->UNK )
				c->setOnlineStatus( MSNProtocol::protocol()->FLN );

			if( c->metaContact()->isTemporary() )
				c->metaContact()->setTemporary( false, m_groupList[ group ] );
			else
				c->contactAddedToGroup( group, m_groupList[ group ] );
		}

		if(!m_allowList.contains(handle) && !m_blockList.contains(handle))
			notifySocket()->addContact( handle, handle, 0, MSNProtocol::AL );
	}
	else if( list == "BL" )
	{
		if( contacts()[ handle ] )
			static_cast<MSNContact *>( contacts()[ handle ] )->setBlocked( true );
		if( !m_blockList.contains( handle ) )
			m_blockList.append( handle );
	}
	else if( list == "AL" )
	{
		if( contacts()[ handle ] )
			static_cast<MSNContact *>( contacts()[ handle ] )->setAllowed( true );
		if( !m_allowList.contains( handle ) )
			m_allowList.append( handle );
	}
	else if( list == "RL" )
	{
		// search for new Contacts
		if( !contacts()[ handle ])
		{
			// FIXME: Users in the allow list or block list now never trigger the
			// 'new user' dialog, which makes it impossible to add those here.
			// Not necessarily bad, but the usability effects need more thought
			// before I declare it good :-)
			if(!m_allowList.contains( handle ) && !m_blockList.contains( handle ))
			{
				NewUserImpl *authDlg = new NewUserImpl(0);
				authDlg->setHandle(handle, publicName);
				QObject::connect( authDlg, SIGNAL(addUser( const QString & )), this, SLOT(slotAddContact( const QString & )));
				QObject::connect( authDlg, SIGNAL(blockUser( QString )), this, SLOT(slotBlockContact( QString )));
				authDlg->show();
			}
		}
		else
		{
			static_cast<MSNContact *>( contacts()[ handle ] )->setReversed( true );
		}
	}
}

void MSNIdentity::slotContactRemoved( QString handle, QString list, uint /* serial */, uint group )
{
	if( list == "BL" )
	{
		m_blockList.remove(handle);
		if(!m_allowList.contains(handle))
			notifySocket()->addContact( handle, handle, 0, MSNProtocol::AL );
	}

	if( list == "AL" )
	{
		m_allowList.remove(handle);
		if(!m_blockList.contains(handle))
			notifySocket()->addContact( handle, handle, 0, MSNProtocol::BL );
	}

	MSNContact *c = static_cast<MSNContact*>( contacts()[ handle ] );
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
			c->removeFromGroup( group );
		}
		else if( list == "BL" )
		{
			c->setBlocked( false );
		}
		else if( list == "AL" )
		{
			c->setAllowed( false );
		}
	}
}

void MSNIdentity::slotCreateChat( QString address, QString auth)
{
	slotCreateChat( 0L, address, auth, m_msgHandle, m_msgHandle );
}

void MSNIdentity::slotCreateChat( QString ID, QString address, QString auth,
	QString handle, QString  publicName  )
{
	handle = handle.lower();

	kdDebug(14140) << "MSNIdentity::slotCreateChat: Creating chat for " << handle << endl;

	if( !contacts()[ handle ] )
		addContact( handle, publicName, 0L, QString::null, true);

	MSNContact *c = static_cast<MSNContact*>( contacts()[ handle ] );

	if ( c && myself() )
	{
		static_cast<MSNMessageManager*>( c->manager(true) )->createChat( handle, address, auth, ID );
		if(ID && MSNPreferences::notifyNewChat() )
		{
			QString body=i18n("%1 has opened a new chat").arg(c->displayName());
			KopeteMessage tmpMsg = KopeteMessage( c , c->manager()->members() , body , KopeteMessage::Internal, KopeteMessage::PlainText);
			KopeteViewManager::viewManager()->readMessages( c->manager(), true );
			c->manager()->appendMessage(tmpMsg);
		}
	}
}

void MSNIdentity::slotStartChatSession( QString handle )
{
	//FIXME: raise the manager if it does exist

	// First create a message manager, because we might get an existing
	// manager back, in which case we likely also have an active switchboard
	// connection to reuse...
	MSNContact *c = static_cast<MSNContact*>( contacts()[ handle ] );
	//if( isConnected() && c && myself() && handle != m_msnId )
	if( m_notifySocket && c && myself() && handle != identityId() )
	{
		if(!c->manager() || !static_cast<MSNMessageManager*>( c->manager() )->service())
		{
			kdDebug(14140) << "MSNIdentity::slotStartChatSession: "
				<< "Creating new switchboard connection" << endl;

			//FIXME: what's happend when the user try to open two socket in the same time????  can the m_msgHandle be altered??
			m_msgHandle = handle;
			m_notifySocket->createChatSession();
		}
	}
}

//--------
void MSNIdentity::slotBlockContact( QString handle ) 
{
	if(m_allowList.contains(handle))
		notifySocket()->removeContact( handle, 0, MSNProtocol::AL);
	else if(!m_blockList.contains(handle))
		notifySocket()->addContact( handle, handle, 0, MSNProtocol::BL );
}

void MSNIdentity::slotAddContact( const QString &userName )
{
	addContact( userName );
}
//----------

bool MSNIdentity::addContactToMetaContact( const QString &contactId, const QString &displayName, KopeteMetaContact *metaContact )
{
	if( isConnected() ) 
	{
		if( !metaContact->isTemporary() )
		{
			m_addWizard_metaContact=metaContact;
			//This is a normal contact. Get all the groups this MetaContact is in
			QPtrList<KopeteGroup> groupList = metaContact->groups();
			if( !groupList.isEmpty() )
			{
				for ( KopeteGroup *group = groupList.first(); group; group = groupList.next() )
				{
					//For each group, ensure it is on the MSN server
					if( !group->pluginData( protocol() , identityId() + " id" ).isEmpty() )
					{
						//Add the contact to the group on the server
						m_notifySocket->addContact( contactId, displayName, group->pluginData(protocol(),identityId() + " id").toUInt(), MSNProtocol::FL );
					}
					else
					{
						if(group->displayName().isEmpty())
						{	//the toplevel group
							m_notifySocket->addContact( contactId, displayName, 0, MSNProtocol::FL );
						}
						else //Create the group and add the contact
						{
							tmp_addToNewGroup << QPair<QString,QString>( contactId, group->displayName() );
							addGroup( group->displayName() );
						}
					}
				}
			}
			//TODO: Find out if this contact was reallt added or not!
			return true;
		}
		else
		{
			//This is a temporary contact. (a person who messaged us but is not on our conntact list.
			//We don't want to create it on the server.Just create the local contact object and add it
			MSNContact *newContact = new MSNContact( this, contactId, contactId, metaContact );
			return (newContact != 0L);
		}
	} 
//	else //We aren't connected! Can't add a contact
	//TODO: add contact whan offline, and sync with server
	return false;
}

#include "msnaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:

