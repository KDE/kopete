/***************************************************************************
                          msnprotocol.cpp  -  MSN Plugin
                             -------------------
    Copyright (c) 2002        by Duncan Mac-Vicar P. <duncan@kde.org>
    Copyright (c) 2002        by Martijn Klingens    <klingens@kde.org>
    Copyright (c) 2002-2003   by Olivier Goffart     <ogoffart@tiscalinet.be>

    Copyright (c) 2002-2003  by the Kopete developers  <kopete-devel@kde.org>
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <qapplication.h>
#include <qtimer.h>

#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>

#include "kopetecontactlist.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "kopeteidentitymanager.h"
#include "kopetemetacontact.h"
#include "kopeteviewmanager.h"

#include "msnaddcontactpage.h"
#include "msneditidentitywidget.h"
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
	QString protocolId = pluginId();

	// Go in experimental mode: enable the new API :-)
	//enableStreaming( true );

	m_status = FLN;
	m_connectstatus = NLN;
	mIsConnected = false;


	kdDebug(14140) << "MSNProtocol::MSNProtocol: MSN Plugin Loading" << endl;

	mPrefs= new MSNPreferences( "msn_protocol", this );
	QObject::connect( mPrefs, SIGNAL( saved() ), this , SLOT( slotPreferencesSaved() ) );
	slotPreferencesSaved();

	m_publicNameSyncMode = SyncFromServer;
	m_publicNameSyncNeeded = false;

	m_addWizard_metaContact=0L;

	setStatusIcon( "msn_offline" );


	QObject::connect( KopeteContactList::contactList(),
		SIGNAL( groupRenamed( KopeteGroup *, const QString & ) ),
		SLOT( slotKopeteGroupRenamed( KopeteGroup * ) ) );
	QObject::connect( KopeteContactList::contactList(),
		SIGNAL( groupRemoved( KopeteGroup * ) ),
		SLOT( slotKopeteGroupRemoved( KopeteGroup * ) ) );

	if( mPrefs->autoConnect() )
	{
		kdDebug(14140) << "[MSNProtocol] autoconnect..." << endl;
		QTimer::singleShot( 0, this, SLOT( connect() ) );
	}

	addAddressBookField( "messaging/msn", KopetePlugin::MakeIndexField );
}

MSNProtocol::~MSNProtocol()
{
}

void MSNProtocol::init()
{
}

bool MSNProtocol::unload()
{
	kdDebug(14140) << "MSNProtocol::unload" << endl;

	disconnect();

	m_groupList.clear();
	m_allowList.clear();
	m_blockList.clear();

	if(notifySocket())
	{
		kdDebug(14140) << "MSNProtocol::unload: WARNING NotifySocket was not deleted" <<endl;
		delete notifySocket();
	}

	return KopeteProtocol::unload();
}

void MSNProtocol::connect()
{

}

void MSNProtocol::disconnect()
{
}

bool MSNProtocol::isConnected() const
{
	return mIsConnected;
}


void MSNProtocol::setAway(void)
{

}

void MSNProtocol::setAvailable(void)
{

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

void MSNProtocol::deserializeContact( KopeteMetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> & /* addressBookData */ )
{
	QString contactId   = serializedData[ "contactId" ];
	QString displayName = serializedData[ "displayName" ];
	QStringList groups  = QStringList::split( ",", serializedData[ "groups" ] );

	// Create MSN contact
	MSNContact *c = new MSNContact( this, contactId, displayName, metaContact );
	c->setMsnStatus( MSNProtocol::FLN );
	for( QStringList::Iterator it = groups.begin() ; it != groups.end(); ++it )
		c->contactAddedToGroup( ( *it ).toUInt(), m_groupList[ ( *it ).toUInt() ] );
}

KopeteContact* MSNProtocol::myself() const
{
	QDict<KopeteIdentity> dict=KopeteIdentityManager::manager()->identities(this);
	QDictIterator<KopeteIdentity> it( dict ); 
    for( ; MSNIdentity *identity=static_cast<MSNIdentity*>(it.current()); ++it )
	{
		if(identity->myself())
		{
			return identity->myself();
		}
	}
	return 0L;
}

AddContactPage *MSNProtocol::createAddContactWidget(QWidget *parent)
{
	return (new MSNAddContactPage(this,parent));
}

EditIdentityWidget *MSNProtocol::createEditIdentityWidget(KopeteIdentity *identity, QWidget *parent)
{
	return new MSNEditIdentityWidget(this,identity,parent);
}

KopeteIdentity *MSNProtocol::createNewIdentity(const QString &identityId)
{
	return new MSNIdentity(this, identityId);
}

KActionMenu* MSNProtocol::protocolActions()
{
	KActionMenu *menu=new KActionMenu("MSN",this);
	
	QDict<KopeteIdentity> dict=KopeteIdentityManager::manager()->identities(this);
	QDictIterator<KopeteIdentity> it( dict ); 
    for( ; MSNIdentity *identity=static_cast<MSNIdentity*>(it.current()); ++it )
	{
		menu->insert(identity->actionMenu());
	}
	return menu;
}

// NOTE: CALL THIS ONLY BEING CONNECTED
void MSNProtocol::slotSyncContactList()
{
	if ( ! mIsConnected )
	{
		return;
	}
	// First, delete D marked contacts
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
void MSNProtocol::slotNotifySocketStatusChanged( MSNSocket::OnlineStatus status )
{
	kdDebug(14140) << "MSNProtocol::slotOnlineStatusChanged: " << status <<endl;
	mIsConnected = (status == MSNSocket::Connected);
	if( status == MSNSocket::Disconnected )
	{
		KopeteMessageManagerDict sessions =
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
/*			else
			{
				kdDebug(14140) << "MSNProtocol::slotOnlineStatusChanged: "
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
//		m_openInboxAction->setEnabled(false);

		m_status = FLN;

		// Reset flags. They can't be set in the connect method, because
		// offline changes might have been made before. Instead the c'tor
		// sets the defaults, and the disconnect slot resets those defaults
		// FIXME: Can't we share this code?
		m_publicNameSyncMode = SyncFromServer;
	}
	else if( status == MSNSocket::Connecting )
	{
		for( QDictIterator<KopeteContact> it( contacts() ); it.current() ; ++it )
			static_cast<MSNContact *>( *it )->setMsnStatus( MSNProtocol::FLN );
	}
}



const QString MSNProtocol::protocolIcon()
{
	return "msn_online";
}



void MSNProtocol::slotBlockContact( QString handle ) 
{
	if(m_allowList.contains(handle))
		notifySocket()->removeContact( handle, 0, AL);
	else if(!m_blockList.contains(handle))
		notifySocket()->addContact( handle, handle, 0, BL );
}

bool MSNProtocol::addContactToMetaContact( const QString &contactId, const QString &displayName,
		KopeteMetaContact *metaContact )
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
					if( !group->pluginData( this , "id" ).isEmpty() )
					{
						//Add the contact to the group on the server
						notifySocket()->addContact( contactId, displayName, group->pluginData(this,"id").toUInt(), FL );
					}
					else
					{
						//Create the group and add the contact
						tmp_addToNewGroup << QPair<QString,QString>( contactId, group->displayName() );
						addGroup( group->displayName() );
					}
				}
			} else {
				kdDebug(14140) << "[MSNProtocol::addContactToMetaContact() This MetaContact isn't in a group!" << endl;
			}
			//TODO: Find out if this contact was reallt added or not!
			return true;
		} else {
			//This is a temporary contact. (a person who messaged us but is not on our conntact list.
			//We don't want to create it on the server.Just create the local contact object and add it
			MSNContact *newContact = new MSNContact( this, contactId, contactId, metaContact );
			return (newContact != 0L);
		}
	} else {
		//We aren't connected! Can't add a contact
		return false;
	}
}

void MSNProtocol::slotGroupAdded( QString groupName, uint groupNumber )
{
	// We have pending groups that we need add a contact to
	// FIXME: This is dangerous, since in theory there could be more groups being
	// created, resulting in Kopete adding contacts into the wrong group!
	// Better use s/th like a QMap<KopeteGroup *, QMap< QString id, QString nick> >
	// or similar - Martijn
	if( tmp_addToNewGroup.count() > 0 )
	{
		for( QValueList<QPair<QString,QString> >::Iterator it = tmp_addToNewGroup.begin(); it != tmp_addToNewGroup.end(); ++it )
		{
			if( ( *it ).second == groupName )
			{
				kdDebug( 14140 ) << k_funcinfo << "Adding to new group: " << ( *it ).first <<  endl;
				notifySocket()->addContact( ( *it ).first, ( *it ).first, groupNumber, FL );
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
		if( !g->pluginData( this , "id" ).isEmpty() )
		{
			if( g->pluginData(this,"id").toUInt() == groupNumber )
			{
				m_groupList.insert( groupNumber, g );
				QString oldGroupName;
				if( g->pluginData(this,"displayName") != groupName )
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

	fallBack->setPluginData( this,"id", QString::number( groupNumber ) );
	fallBack->setPluginData( this,"displayName", groupName );
	m_groupList.insert( groupNumber, fallBack );
}

void MSNProtocol::slotGroupRenamed( QString groupName, uint groupNumber )
{
	if( m_groupList.contains( groupNumber ) )
	{
		//m_groupList[ groupNumber ]->setPluginData( this,"id", QString::number( groupNumber ) );
		m_groupList[ groupNumber ]->setPluginData( this,"displayName", groupName );
		m_groupList[ groupNumber ]->setDisplayName( groupName );
	}
	else
	{
		slotGroupAdded( groupName, groupNumber );
	}
}

void MSNProtocol::slotGroupRemoved( uint group )
{
	if( m_groupList.contains( group ) )
	{
		// FIXME: we should realy emty data in the group... but in most of cases, the group is already del
		// FIXME: Shouldn't we fix the memory management instead then??? - Martijn
		//m_groupList[ group ]->setPluginData( this, QStringList() );
		m_groupList.remove( group );
	}
}

void MSNProtocol::addGroup( const QString &groupName , const QString& contactToAdd )
{
	if(!contactToAdd.isNull())
		tmp_addToNewGroup << QPair<QString,QString>(contactToAdd,groupName);

//	if( !( m_groupList.contains( groupName ) ) )
		notifySocket()->addGroup( groupName );
}

void MSNProtocol::slotKopeteGroupRenamed(KopeteGroup *g)
{
	if(g->type()==KopeteGroup::Classic)
	{
		if(!g->pluginData(this,"id").isEmpty())
		{
			if( m_groupList.contains( g->pluginData(this,"id").toUInt() ) )
				notifySocket()->renameGroup( g->displayName(), g->pluginData(this,"id").toUInt() );
		}
	}
}

void MSNProtocol::slotKopeteGroupRemoved(KopeteGroup *g)
{
	if(!g->pluginData(this,"id").isEmpty())
	{
		unsigned int groupNumber=g->pluginData(this,"id").toUInt();
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

			KopeteGroup::toplevel->setPluginData(this,"id","0");
			KopeteGroup::toplevel->setPluginData(this,"displayName",g->pluginData(this,"displayName"));
			g->setPluginData(this,"id",QString::null); //the group should be soon deleted, but make sure

			return;
		}

		//if contact are contains only in the group we are removing, move it from the group 0
		QDictIterator<KopeteContact> it( contacts() );
		for ( ; it.current() ; ++it )
		{
			MSNContact *c = static_cast<MSNContact *>( it.current() );
			if( c->serverGroups().contains( groupNumber ) && c->serverGroups().count() == 1 )
				notifySocket()->addContact( c->contactId(), c->displayName(), 0, MSNProtocol::FL );
		}
		if(notifySocket())
			notifySocket()->removeGroup( groupNumber );
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

void MSNProtocol::slotContactListed( QString handle, QString publicName, QString group, QString list )
{
	// On empty lists handle might be empty, ignore that
	if( handle.isEmpty() )
		return;

	QStringList contactGroups = QStringList::split( ",", group, false );
	if( list == "FL" )
	{
		KopeteMetaContact *metaContact = KopeteContactList::contactList()->findContact( pluginId(), QString::null, handle );
		if( metaContact )
		{
			// Contact exists, update data.
			// Merging difference between server contact list and KopeteContact's contact list into MetaContact's contact-list
			kdDebug( 14140 ) << k_funcinfo << "******* updating existing contact: " << handle << "!" << endl;
			MSNContact *c = static_cast<MSNContact*>( metaContact->findContact( pluginId(), QString::null, handle ) );
			c->setMsnStatus( FLN );
			c->rename( publicName ); // FIXME: setDisplayName() is protected, but is rename() actually what we want here?

			const QMap<uint, KopeteGroup *> &serverGroups = c->serverGroups();
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
			KopeteContactList::contactList()->addMetaContact( metaContact );

			MSNContact *msnContact = new MSNContact( this, handle, publicName, metaContact );
			msnContact->setMsnStatus( FLN );

			for( QStringList::Iterator it = contactGroups.begin();
				it != contactGroups.end(); ++it )
			{
				uint groupNumber = ( *it ).toUInt();
				msnContact->contactAddedToGroup( groupNumber, m_groupList[ groupNumber ] );
				metaContact->addToGroup( m_groupList[ groupNumber ] );
			}
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
			kdDebug(14140) << "MSNProtocol: Contact not found in list!" << endl;

			NewUserImpl *authDlg = new NewUserImpl(0);
			authDlg->setHandle(handle, publicName);
			QObject::connect( authDlg, SIGNAL(addUser( const QString & )), this, SLOT(slotAddContact( const QString & )));
			QObject::connect( authDlg, SIGNAL(blockUser( QString )), this, SLOT(slotBlockContact( QString )));
			authDlg->show();
		}

		if( contacts()[ handle ] )
			static_cast<MSNContact *>( contacts()[ handle ] )->setReversed( true );
	}
}

void MSNProtocol::slotContactRemoved( QString handle, QString list, uint /* serial */, uint group )
{
	if( list == "BL" )
	{
		m_blockList.remove(handle);
		if(!m_allowList.contains(handle))
			notifySocket()->addContact( handle, handle, 0, AL );
	}

	if( list == "AL" )
	{
		m_allowList.remove(handle);
		if(!m_blockList.contains(handle))
			notifySocket()->addContact( handle, handle, 0, BL );
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

void MSNProtocol::slotContactAdded( QString handle, QString publicName,
	QString list, uint /* serial */, uint group )
{
	if( list == "FL" )
	{
		bool new_contact=false;
		if( !contacts()[ handle ] )
		{
			KopeteMetaContact *m = KopeteContactList::contactList()->findContact( pluginId(), QString::null, handle );
			if(m)
			{
				kdDebug(14140) << "MSNProtocol::slotContactAdded: Warning: the contact was found in the contactlist but not referanced in the protocol" <<endl;
				MSNContact *c = static_cast<MSNContact*>(m->findContact( pluginId(), QString::null, handle ));
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
				c->contactAddedToGroup( group, m_groupList[ group ] );
		}

		if(!m_allowList.contains(handle) && !m_blockList.contains(handle))
			notifySocket()->addContact( handle, handle, 0, AL );
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
			QObject::connect( authDlg, SIGNAL(addUser( const QString & )), this, SLOT(slotAddContact( const QString & )));
			QObject::connect( authDlg, SIGNAL(blockUser( QString )), this, SLOT(slotBlockContact( QString )));
			authDlg->show();
		}
		else
		{
			static_cast<MSNContact *>( contacts()[ handle ] )->setReversed( true );
		}
	}
}

void MSNProtocol::slotAddContact( const QString &userName )
{
	addContact( userName );
}

void MSNProtocol::slotCreateChat( QString address, QString auth)
{
	slotCreateChat( 0L, address, auth, m_msgHandle, m_msgHandle );
}

void MSNProtocol::slotCreateChat( QString ID, QString address, QString auth,
	QString handle, QString  publicName  )
{
	handle = handle.lower();

	kdDebug(14140) << "MSNProtocol::slotCreateChat: Creating chat for " <<
		handle << endl;

	if( !contacts()[ handle ] )
		addContact( handle, publicName, 0L, QString::null, true);

	MSNContact *c = static_cast<MSNContact*>( contacts()[ handle ] );

	if ( c && myself() )
	{
		static_cast<MSNMessageManager*>( c->manager(true) )->createChat( handle, address, auth, ID );
		if(ID && mPrefs->notifyNewChat() )
		{
			QString body=i18n("%1 has opened a new chat").arg(c->displayName());
			KopeteMessage tmpMsg = KopeteMessage( c , c->manager()->members() , body , KopeteMessage::Internal, KopeteMessage::PlainText);
			KopeteViewManager::viewManager()->readMessages( c->manager(), true );
			c->manager()->appendMessage(tmpMsg);
		}
	}
}

void MSNProtocol::slotStartChatSession( QString handle )
{
	// First create a message manager, because we might get an existing
	// manager back, in which case we likely also have an active switchboard
	// connection to reuse...
	MSNContact *c = static_cast<MSNContact*>( contacts()[ handle ] );
	if( isConnected() && c && myself() && handle != m_msnId )
	{
		KopeteContactPtrList chatmembers;

		if(!c->manager() || !static_cast<MSNMessageManager*>( c->manager() )->service())
		{
			kdDebug(14140) << "MSNProtocol::slotStartChatSession: "
				<< "Creating new switchboard connection" << endl;

			//FIXME: what's happend when the user try to open two socket in the same time????  can the m_msgHandle be altered??
			m_msgHandle = handle;
			notifySocket()->createChatSession();
		}
	}
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
	/*m_password   = mPrefs->password();
//	m_publicName = mPrefs->publicName();

	if(m_msnId != mPrefs->msnId())
	{
		m_msnId  = mPrefs->msnId();
		if( m_myself && m_myself->contactId() != m_msnId )
		{
			disconnect();
			delete m_myself;
			m_myself = new MSNContact( this, m_msnId, m_publicName, 0L );
		}
	}*/
}

MSNNotifySocket *MSNProtocol::notifySocket()
{
	QDict<KopeteIdentity> dict=KopeteIdentityManager::manager()->identities(this);
	QDictIterator<KopeteIdentity> it( dict ); 
    for( ; MSNIdentity *identity=static_cast<MSNIdentity*>(it.current()); ++it )
	{
		if(identity->notifySocket())
		{
			return identity->notifySocket();
		}
	}
	return 0L;
};


#include "msnprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

