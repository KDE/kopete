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
#include <qapplication.h>

#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>

#include "kopetecontactlist.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"

#include "msnaddcontactpage.h"
#include "msncontact.h"
#include "msndebugrawcmddlg.h"
#include "msnidentity.h"
#include "msnnotifysocket.h"
#include "msnpreferences.h"
#include "msnprotocol.h"
#include "msnmessagemanager.h"
#include "newuserimpl.h"

K_EXPORT_COMPONENT_FACTORY( kopete_msn, KGenericFactory<MSNProtocol> );

MSNProtocol::MSNProtocol( QObject *parent, const char *name,
	const QStringList & /* args */ )
: KopeteProtocol( parent, name )
{
	QString protocolId = this->pluginId();

	// Go in experimental mode: enable the new API :-)
	//enableStreaming( true );

	m_status = FLN;
	m_connectstatus=NLN;
	mIsConnected = false;
	m_notifySocket = 0L;
	m_myself=0L;

	m_identity = new MSNIdentity( this, "m_identity" );

	kdDebug() << "MSNProtocol::MSNProtocol: MSN Plugin Loading" << endl;

	mPrefs= new MSNPreferences( "msn_protocol", this );
	connect( mPrefs, SIGNAL(saved()) , this , SLOT ( slotPreferencesSaved() ));
	slotPreferencesSaved();

	m_publicNameSyncMode = SyncFromServer;
	m_publicNameSyncNeeded = false;

	m_addWizard_metaContact=0L;
	m_connectstatus=NLN;

	initActions();

	setStatusIcon( "msn_offline" );

	// FIXME: I think we should add a global self metaContact (Olivier)
	m_myself = new MSNContact( this, m_msnId, m_publicName, 0L );

	if ( mPrefs->autoConnect() )
		Connect();

	connect ( KopeteContactList::contactList() , SIGNAL(groupRenamed(KopeteGroup*,const QString&)) , SLOT (slotKopeteGroupRenamed(KopeteGroup*)));
	connect ( KopeteContactList::contactList() , SIGNAL(groupRemoved(KopeteGroup*)) , SLOT (slotKopeteGroupRemoved(KopeteGroup*)));
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
	kdDebug() << "MSNProtocol::unload" << endl;

	Disconnect();

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
		int r = KMessageBox::warningContinueCancel( qApp->mainWidget(),
			i18n("<qt>You have not yet specified a username for MSN. "
				"You can specify your MSN settings in the Kopete configuration dialog<br>"
				"Get an MSN account <a href=\"http://login.hotmail.passport.com/cgi-bin/register/en/default.asp\">here</a><br>"
				"Do you want to configure MSN now?</qt>" ),
			i18n( "MSN plugin not configured yet" ),
			KGuiItem( i18n( "C&onfigure..." ), "configure" ), QString::null,
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
	m_notifySocket = new MSNNotifySocket( this, m_msnId );

	connect( m_notifySocket, SIGNAL( groupAdded( QString, uint,uint ) ),
		this, SLOT( slotGroupAdded( QString, uint ) ) );
	connect( m_notifySocket, SIGNAL( groupRenamed( QString, uint, uint ) ),
		this, SLOT( slotGroupRenamed( QString, uint ) ) );
	connect( m_notifySocket, SIGNAL( groupListed( QString, uint ) ),
		this, SLOT( slotGroupAdded( QString, uint ) ) );
	connect( m_notifySocket, SIGNAL(groupRemoved( uint, uint ) ),
		this, SLOT( slotGroupRemoved( uint ) ) );
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
	setStatusIcon( "msn_connecting" );
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

void MSNProtocol::serialize( KopeteMetaContact *metaContact)
{
	QStringList stream;
	QPtrList<KopeteContact> contacts = metaContact->contacts();
	for( KopeteContact *c = contacts.first(); c ; c = contacts.next() )
	{
		if ( c->protocol()->pluginId() != this->pluginId() ) // not our contact, next one please
				continue;

		MSNContact *m = dynamic_cast<MSNContact*>(c);

		if( m )
		{
			QString groups;
			QValueList<unsigned int> list=m->groups();
			for ( QValueList<unsigned int>::Iterator it = list.begin(); it != list.end(); ++it)
			{
				if(!groups.isEmpty()) groups+=",";
				groups+=QString::number(*it);
			}
			stream << m->contactId() << m->displayName() << groups ;
		}
	}
	metaContact->setPluginData(this , stream);
}

void MSNProtocol::deserialize( KopeteMetaContact *metaContact,
	const QStringList &strList )
{
/*	kdDebug() << "MSNProtocol::deserialize: " << metaContact->displayName()
		<< ", [ " << strList.join( ", " ) << " ]" << endl;*/

	QString protocolId = this->pluginId();

	uint idx = 0;
	while( idx < strList.size() )
	{
		QString passport    = strList[ idx ];
		QString displayName = strList[ idx + 1 ];
		QStringList groups  = QStringList::split( ",", strList[ idx + 2 ] );

		// Create MSN contact
		MSNContact *c = new MSNContact( this, passport, displayName, metaContact );
		c->setMsnStatus( MSNProtocol::FLN );
		for(QStringList::Iterator it = groups.begin() ; it != groups.end(); ++it )
			c->slotAddedToGroup(  (*it).toUInt()  );

		metaContact->addContact( c);

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
		SmallIcon( statusIcon() ),
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
}

KActionMenu* MSNProtocol::protocolActions()
{
	actionStatusMenu->popupMenu()->changeTitle(
		m_menuTitleId,
		SmallIcon( statusIcon() ),
		i18n( "%1 (%2)" ).arg( m_publicName ).arg( m_msnId ) );

	return actionStatusMenu;
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
			KMessageBox::error(0l, i18n("<qt>You must enter a valid e-mail address</qt>"), i18n("MSN Plugin"));
		}
	}
}

void MSNProtocol::slotNotifySocketStatusChanged( MSNSocket::OnlineStatus status )
{
	kdDebug() << "MSNProtocol::slotOnlineStatusChanged: " << status <<endl;
	mIsConnected = (status == MSNSocket::Connected);
	if ( mIsConnected )
	{
		//KopeteMessageManagerFactory::factory()->cleanSessions(this);
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

		setStatusIcon( "msn_online" );

/*		QStringList localgroups = KopeteContactList::contactList()->groups().toStringList() ;
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

				// Groups doesnt match any server group 
				if ( exists == 0 )
				{
					kdDebug() << "MSNProtocol::slotOnlineStatusChanged: Sync: Local group " << localgroup << " doesn't exist on server!" << endl;
					addGroup( localgroup );
				}
			}
		}*/
	}
	else if( status == MSNSocket::Disconnected )
	{
		KopeteMessageManagerDict sessions =
			KopeteMessageManagerFactory::factory()->protocolSessions( this );
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

		QDictIterator<KopeteContact> it( contacts() );
		for ( ; it.current() ; ++it )
		{
			static_cast<MSNContact *>( *it )->setMsnStatus( MSNProtocol::FLN );
		}

		m_allowList.clear();
		m_blockList.clear();
		m_groupList.clear();

		mIsConnected = false;
		setStatusIcon( "msn_offline" );
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
		QDictIterator<KopeteContact> it( contacts() );
		for ( ; it.current() ; ++it )
		{
			static_cast<MSNContact *>( *it )->setMsnStatus( MSNProtocol::UNK );
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
			setStatusIcon( "msn_online" );
			break;
		case AWY:
		case IDL:
		case BRB:
		case LUN:
			setStatusIcon( "msn_away" );
			break;
		case BSY:
		case PHN:
			setStatusIcon( "msn_na" );
			break;
		case FLN:
		default:
			setStatusIcon( "msn_offline" );
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

void MSNProtocol::addContact( const QString &userID , KopeteMetaContact *m,
	const QString & /* group */ )
{
	if( isConnected() )
	{
		m_addWizard_metaContact=m;

		if(m && !m->groups().isEmpty())
		{
			QPtrList<KopeteGroup> gprs=m->groups();
			for ( KopeteGroup *g=gprs.first(); g; g = gprs.next() )
			{
				QStringList strL=g->pluginData(this);
				if(strL.count()>=1)
				{
					m_notifySocket->addContact( userID, userID, strL.first().toUInt(), FL );
				}
				else
				{
					tmp_addToNewGroup << QPair<QString,QString>(userID,g->displayName());
					addGroup(g->displayName());
				}
			}
		}
		else
		{
			m_notifySocket->addContact( userID, userID, 0, FL );
		  //TODO:
		  /*if(!group.isNull()) 
		  {
			int g = groupNumber( group );
			if(g==-1) {
				tmp_addToNewGroup << QPair<QString,QString>(userID,group);
				addGroup(group);
			}
			else {
				m_notifySocket->addContact( userID, userID, g, FL );
		  }*/
		}
	}
}


/*QStringList MSNProtocol::groups() const
{
	QStringList result;
	QMap<uint, QString>::ConstIterator it;
	for( it = m_groupList.begin(); it != m_groupList.end(); ++it )
		result.append( *it );

//	kdDebug() << "MSNProtocol::groups(): " << result.join(", " ) << endl;
	return result;
} */

void MSNProtocol::slotGroupAdded( QString groupName, uint groupNumber )
{
	if(tmp_addToNewGroup.count()>0)
	{
		for ( QValueList<QPair<QString,QString> >::Iterator it = tmp_addToNewGroup.begin(); it != tmp_addToNewGroup.end(); ++it)
		{
			if((*it).second==groupName)
			{
				kdDebug() << "MSNProtocol::slotGroupAdded : Adding to new group: " << (*it).first <<  endl;
				m_notifySocket->addContact( (*it).first, (*it).first, groupNumber, FL );
			}
		}
		tmp_addToNewGroup.clear();
	}

	if( m_groupList.contains( groupNumber ) )
	{
		kdDebug() << "MSNProtocol::slotGroupAdded: WARNING: group already in list" <<endl;
		return;
	}

	QPtrList<KopeteGroup> list= KopeteContactList::contactList()->groups();
	KopeteGroup *convient=0L;
	for ( KopeteGroup *g=list.first(); g; g = list.next() )
	{
		QStringList strL=g->pluginData(this);
		if(strL.count() >= 1)
		{
			if(strL.first().toUInt() == groupNumber)
			{
				m_groupList.insert( groupNumber, g );
				QString oldServerName;
				if(strL.count()>=2)
					oldServerName=strL[1];
				if(oldServerName != groupName)
				{	// the dispalyName of the group has been modified by another client
					strL.clear();
					strL.append( QString::number(groupNumber) );
					strL.append( groupName );
					g->setPluginData(this, strL);
					if(g->type() == KopeteGroup::Classic)
						g->setDisplayName( groupName );
				}
				else if(g->displayName() != groupName)
				{	//the displayName was changed in kopete when we was offline (update the server right now)
					//TODO
				}
				return;
			}
		}
		else
		{	//a group with the same dysplayName but with no msn id set convient
			if(g->displayName()==groupName)
				convient=g;
		}
	}

	if(!convient)
	{
		convient=new KopeteGroup(groupName);
		KopeteContactList::contactList()->addGroup(convient);
	}

	QStringList strL;
	strL.append( QString::number(groupNumber) );
	strL.append( groupName );
	convient->setPluginData(this, strL);

	m_groupList.insert( groupNumber, convient );

}

void MSNProtocol::slotGroupRenamed( QString groupName, uint groupNumber )
{
	if( m_groupList.contains( groupNumber ) )
	{
		QStringList strL;
		strL.append( QString::number(groupNumber) );
		strL.append( groupName );
		m_groupList[groupNumber]->setPluginData(this, strL);
		m_groupList[groupNumber]->setDisplayName(groupName);
	}
}

void MSNProtocol::slotGroupRemoved( uint group )
{
	if( m_groupList.contains( group ) )
	{
		//FIXME: we should realy emty data in the group... but in most of cases, the group is already del
		//m_groupList[group]->setPluginData(this, QStringList());
		m_groupList.remove( group );
	}
}

void MSNProtocol::addGroup( const QString &groupName , const QString& contactToAdd )
{
	if(!contactToAdd.isNull())
		tmp_addToNewGroup << QPair<QString,QString>(contactToAdd,groupName);

//	if( !( m_groupList.contains( groupName ) ) )
		m_notifySocket->addGroup( groupName );
}

void MSNProtocol::slotKopeteGroupRenamed(KopeteGroup *g)
{
	if(g->type()==KopeteGroup::Classic)
	{
		QStringList strL=g->pluginData(this);
		if(strL.count()>=1)
		{
			if( m_groupList.contains( strL.first().toUInt() ) )
				m_notifySocket->renameGroup( g->displayName(), strL.first().toUInt() );
		}
	}
}

void MSNProtocol::slotKopeteGroupRemoved(KopeteGroup *g)
{
	QStringList strL=g->pluginData(this);
	if(strL.count()>=1)
	{
		unsigned int groupNumber=strL.first().toUInt();
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

			g->setPluginData(this, QStringList());
			KopeteGroup::toplevel->setPluginData(this, strL);

			return;
		}

		//if contact are contains only in the group we are removing, move it from the group 0
		QDictIterator<KopeteContact> it( contacts() );
		for ( ; it.current() ; ++it )
		{
			MSNContact *c = static_cast<MSNContact *>( it.current() );
			if( c->groups().contains( groupNumber ) && c->groups().count() == 1 )
				m_notifySocket->addContact( c->contactId(), c->displayName(), 0, MSNProtocol::FL );
		}
		m_notifySocket->removeGroup( groupNumber );
	}
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
		KopeteMetaContact *m = KopeteContactList::contactList()->findContact(
			pluginId(), QString::null, handle );

		if( m )
		{
			//Contact exists, update data.
			//Merging difference between server contact list and KopeteContact's contact list into MetaContact's contact-list
			MSNContact *c = static_cast<MSNContact*>(m->findContact( pluginId(), QString::null, handle ));
			c->setMsnStatus(FLN);
			c->setDisplayName(publicName);
			QValueList<unsigned int> contactGroups=c->groups();
			QValueList<unsigned int> serverGroups=c->groups();

			for( QStringList::Iterator it = groups.begin(); it != groups.end(); ++it )
			{
				unsigned int serverGroup=(*it).toUInt();
				serverGroups.append(serverGroup);
				if(!contactGroups.contains(serverGroup))
				{	//the contact has been added in a group by another client
					c->slotAddedToGroup(serverGroup);
					m->addToGroup(m_groupList[serverGroup]);
				}
			}
			for( QValueList<unsigned int>::ConstIterator it = contactGroups.begin(); it != contactGroups.end(); ++it )
			{
				unsigned int cGroup=*it;
				if(!serverGroups.contains(cGroup))
				{	//the contact has been removed from a group by another client
					c->slotRemovedFromGroup(cGroup);
					m->removeFromGroup(m_groupList[cGroup]);
				}
			}
			//TODO: update server if the contact has been moved to other group when msn is offline
		}
		else
		{
			m = new KopeteMetaContact();

			MSNContact *msnContact = new MSNContact( this, handle, publicName, m );
			msnContact->setMsnStatus( FLN );

			for( QStringList::Iterator it = groups.begin();
				it != groups.end(); ++it )
			{
				msnContact->slotAddedToGroup(  (*it).toUInt()  );
				m->addToGroup(m_groupList[(*it).toUInt()]);
			}
			m->addContact( msnContact );
			KopeteContactList::contactList()->addMetaContact(m);
		}
	}
	else if( list == "BL" )
	{
		if( !m_blockList.contains( handle ) )
			m_blockList.append( handle );
		if( contacts()[ handle ] )
			static_cast<MSNContact *>( contacts()[ handle ] )->setBlocked( true );
	}
	else if( list == "AL" )
	{
		if( !m_allowList.contains( handle ) )
			m_allowList.append( handle );
		if( contacts()[ handle ] )
			static_cast<MSNContact *>( contacts()[ handle ] )->setAllowed( true );
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

		if( contacts()[ handle ] )
			static_cast<MSNContact *>( contacts()[ handle ] )->setReversed( true );
	}
}

void MSNProtocol::slotContactRemoved( QString handle, QString list,
	uint /* serial */, uint group )
{

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
			c->slotRemovedFromGroup( group );
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
		}
	}
}

void MSNProtocol::slotContactAdded( QString handle, QString publicName,
	QString list, uint /* serial */, uint group )
{
	if( list == "FL" )
	{
		bool new_contact=false;
		if( !contacts()[ handle ] )
		{
			KopeteMetaContact *m = KopeteContactList::contactList()->findContact( this->pluginId(), QString::null, handle );
			if(m)
			{
				kdDebug() << "MSNProtocol::slotContactAdded: Warning: the contact was found in the contactlist but not referanced in the protocol" <<endl;
				MSNContact *c = static_cast<MSNContact*>(m->findContact( this->pluginId(), QString::null, handle ));
				c->slotAddedToGroup( group );
			}
			else
			{
				new_contact=true;

				if(m_addWizard_metaContact)
					m=m_addWizard_metaContact;
				else
					m=new KopeteMetaContact();

				MSNContact *c = new MSNContact( this, handle, publicName, m );
				c->slotAddedToGroup( group );

				if(!m_addWizard_metaContact)
				{
					m->addToGroup(m_groupList[group]);
					KopeteContactList::contactList()->addMetaContact(m);
				}
				m->addContact( c);
				c->setMsnStatus(FLN);

				m_addWizard_metaContact=0L;
			}
		}
		if(!new_contact)
		{
			MSNContact *c = static_cast<MSNContact *>( contacts()[ handle ] );
			if(c->msnStatus()==UNK)
				c->setMsnStatus(FLN);

			if(c->metaContact()->isTemporary())
				c->metaContact()->setTemporary(false,m_groupList[group]);
			else
				c->slotAddedToGroup( group );
		}

		if(!m_allowList.contains(handle) && !m_blockList.contains(handle))
			m_notifySocket->addContact( handle, handle, 0, AL );
	}
	if( list == "BL" )
	{
		if( contacts()[ handle ] )
			static_cast<MSNContact *>( contacts()[ handle ] )->setBlocked( true );
		if( !m_blockList.contains( handle ) )
			m_blockList.append( handle );
	}
	if( list == "AL" )
	{
		if( contacts()[ handle ] )
			static_cast<MSNContact *>( contacts()[ handle ] )->setAllowed( true );
		if( !m_allowList.contains( handle ) )
			m_allowList.append( handle );
	}
	if( list == "RL" )
	{
		if( !contacts()[ handle ] )
		{
			NewUserImpl *authDlg = new NewUserImpl(0);
			authDlg->setHandle(handle, publicName);
			connect( authDlg, SIGNAL(addUser( QString )), this, SLOT(slotAddContact( QString )));
			connect( authDlg, SIGNAL(blockUser( QString )), this, SLOT(slotBlockContact( QString )));
			authDlg->show();
		}
		else
		{
			static_cast<MSNContact *>( contacts()[ handle ] )->setReversed( true );
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

	if( !contacts()[ handle ] )
	{
		KopeteMetaContact *m = KopeteContactList::contactList()->findContact( pluginId(), QString::null, handle );
		if( !m )
		{
			m = new KopeteMetaContact();
			m->setTemporary(true);
			QString protocolid = this->pluginId();

			MSNContact *msnContact = new MSNContact( this, handle, publicName, m );

			m->addContact( msnContact);
			KopeteContactList::contactList()->addMetaContact(m);
		}
	}

	KopeteContact *c = contacts()[ handle ];
	if ( c && m_myself )
	{
		KopeteContactPtrList chatmembers;
		chatmembers.append(c);

		KopeteMessageManager *_manager =
			KopeteMessageManagerFactory::factory()->findKopeteMessageManager( m_myself,
				chatmembers, this  );
		MSNMessageManager *manager =
			dynamic_cast<MSNMessageManager*>( _manager );

		if( !manager )
		{
			manager = new MSNMessageManager( this, m_myself, chatmembers );
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
	KopeteContact *c = contacts()[ handle ];
	if( isConnected() && c && m_myself && handle != m_msnId )
	{
		KopeteContactPtrList chatmembers;
		chatmembers.append(c);

		KopeteMessageManager *_manager = KopeteMessageManagerFactory::factory()->findKopeteMessageManager( m_myself, chatmembers, this  );
		MSNMessageManager *manager= dynamic_cast<MSNMessageManager*>(_manager);
		if(!manager)
		{
			manager = new MSNMessageManager( this, m_myself, chatmembers );
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
		KMessageBox::error( qApp->mainWidget(), i18n( "Connection with the MSN server was lost unexpectedly.\nIf you are unable to reconnect, please try again later." ), i18n( "Connection lost - MSN Plugin - Kopete" ) );
	}*/
	//m_notifySocket->deleteLater();
	delete m_notifySocket;
	m_notifySocket=0l;
	mIsConnected = false;
	setStatusIcon( "msn_offline" );
	m_openInboxAction->setEnabled(false);
	kdDebug() << "MSNProtocol::slotNotifySocketClosed - done" << endl;
}

KActionCollection * MSNProtocol::customChatActions(KopeteMessageManager * manager)
{
	MSNMessageManager *msnMM= dynamic_cast<MSNMessageManager*>(manager);
	if(!msnMM)
		return 0L;

	return msnMM->chatActions();
}

void MSNProtocol::slotPreferencesSaved()
{
	m_password   = mPrefs->password();
//	m_publicName = mPrefs->publicName();

	if(m_msnId != mPrefs->msnId())
	{
		m_msnId  = mPrefs->msnId();
		if( m_myself && m_myself->contactId() != m_msnId )
		{
			Disconnect();
			delete m_myself;
			m_myself = new MSNContact( this, m_msnId, m_publicName, 0L );
		}
	}
}

#include "msnprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

