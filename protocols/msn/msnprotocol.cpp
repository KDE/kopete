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
#include "msnidentity.h"
#include "msnnotifysocket.h"
#include "msnpreferences.h"
#include "msnprotocol.h"
#include "msnmessagemanager.h"

K_EXPORT_COMPONENT_FACTORY( kopete_msn, KGenericFactory<MSNProtocol> );

MSNProtocol::MSNProtocol( QObject *parent, const char *name,
	const QStringList & /* args */ )
: KopeteProtocol( parent, name )
{
	QString protocolId = pluginId();

	// Go in experimental mode: enable the new API :-)
	//enableStreaming( true );

	kdDebug(14140) << "MSNProtocol::MSNProtocol: MSN Plugin Loading" << endl;

	mPrefs= new MSNPreferences( "msn_protocol", this );
	QObject::connect( mPrefs, SIGNAL( saved() ), this , SLOT( slotPreferencesSaved() ) );
	slotPreferencesSaved();

	m_publicNameSyncMode = SyncFromServer;
	m_publicNameSyncNeeded = false;

	m_addWizard_metaContact=0L;
	m_menu=0L;

	setStatusIcon( "msn_offline" );

	addAddressBookField( "messaging/msn", KopetePlugin::MakeIndexField );
}

MSNProtocol::~MSNProtocol()
{
}

void MSNProtocol::deserializeContact( KopeteMetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> & /* addressBookData */ )
{
	QString contactId   = serializedData[ "contactId" ];
	QString identityId = serializedData[ "identityId" ];
	QString displayName = serializedData[ "displayName" ];
	QStringList groups  = QStringList::split( ",", serializedData[ "groups" ] );	
	
	QDict<KopeteIdentity> identities=KopeteIdentityManager::manager()->identities(this);
	
	//Kopete 0.6.x contactlist
	if(identityId.isNull())
	{
		identityId=mPrefs->msnId();
	}
	KopeteIdentity *identity=identities[identityId];
	if(!identity)
	{
		identity=createNewIdentity(identityId);
	}

	// Create MSN contact
	MSNContact *c = new MSNContact( identity, contactId, displayName, metaContact );
	c->setMsnStatus( MSNProtocol::FLN );
	for( QStringList::Iterator it = groups.begin() ; it != groups.end(); ++it )
		c->contactAddedToGroup( ( *it ).toUInt(), 0L  /* FIXME - m_groupList[ ( *it ).toUInt() ]*/ );
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
	delete m_menu;
	m_menu=0L;
	QDict<KopeteIdentity> dict=KopeteIdentityManager::manager()->identities(this);
	QDictIterator<KopeteIdentity> it( dict ); 
	if(dict.count() == 1 )
	{
		return static_cast<MSNIdentity*>(it.current())->actionMenu();
	}

	KActionMenu *m_menu=new KActionMenu(displayName(),protocolIcon(),this);

	for( ; MSNIdentity *identity=static_cast<MSNIdentity*>(it.current()); ++it )
	{
		m_menu->insert(identity->actionMenu());
	}
	return m_menu;
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


/*void MSNProtocol::slotNotifySocketStatusChanged( MSNSocket::OnlineStatus status )
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
}*/


const QString MSNProtocol::protocolIcon()
{
	return "msn_online";
}

bool MSNProtocol::addContactToMetaContact( const QString &contactId, const QString &displayName,
		KopeteMetaContact *metaContact )
{
//	if( isConnected() )
//	{
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
						//Create the group and add the contact //FIXME!!! (add this in the correct identity only)
						QDict<KopeteIdentity> dict=KopeteIdentityManager::manager()->identities(this);
						QDictIterator<KopeteIdentity> it( dict ); 
    					for( ; MSNIdentity *identity=static_cast<MSNIdentity*>(it.current()); ++it )
						{
							if(identity->notifySocket())
							{
								identity->tmp_addToNewGroup << QPair<QString,QString>( contactId, group->displayName() );
								identity->addGroup( group->displayName() );
							}
						}
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
//	} else {
		//We aren't connected! Can't add a contact
//		return false;
//	}
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
		
	MSNIdentity *identity=0l;
	QDict<KopeteIdentity> dict=KopeteIdentityManager::manager()->identities(this);
	QDictIterator<KopeteIdentity> it( dict ); 
    for( ; (identity=static_cast<MSNIdentity*>(it.current())); ++it )
	{
		if(identity->notifySocket())
		{
			break;
		}
	}
	if(!identity) return;

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
					c->contactAddedToGroup( serverGroup, identity->m_groupList[ serverGroup ] );
					metaContact->addToGroup( identity->m_groupList[ serverGroup ] );
				}
			}

			for( QMap<uint, KopeteGroup *>::ConstIterator it = serverGroups.begin(); it != serverGroups.end(); ++it )
			{
				if( !contactGroups.contains( QString::number( it.key() ) ) )
				{
					// The contact has been removed from a group by another client
					c->removeFromGroup( it.key() );
					metaContact->removeFromGroup( identity->m_groupList[ it.key() ] );
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
				msnContact->contactAddedToGroup( groupNumber, identity->m_groupList[ groupNumber ] );
				metaContact->addToGroup( identity->m_groupList[ groupNumber ] );
			}
		}
	}
	
}


void MSNProtocol::slotContactAdded( QString handle, QString publicName,
	QString list, uint /* serial */, uint group )
{
	MSNIdentity *identity=0l;
	QDict<KopeteIdentity> dict=KopeteIdentityManager::manager()->identities(this);
	QDictIterator<KopeteIdentity> it( dict ); 
    for( ; (identity=static_cast<MSNIdentity*>(it.current())); ++it )
	{
		if(identity->notifySocket())
		{
			break;
		}
	}
	if(!identity) return;


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
				c->contactAddedToGroup( group, identity->m_groupList[ group ] );
			}
			else
			{
				new_contact=true;

				if(m_addWizard_metaContact)
					m=m_addWizard_metaContact;
				else
					m=new KopeteMetaContact();

				MSNContact *c = new MSNContact( this, handle, publicName, m );
				c->contactAddedToGroup( group, identity->m_groupList[ group ] );

				if(!m_addWizard_metaContact)
				{
					m->addToGroup(identity->m_groupList[group]);
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
				c->metaContact()->setTemporary(false, identity->m_groupList[group]);
			else
				c->contactAddedToGroup( group, identity->m_groupList[ group ] );
		}

		if(!identity->m_allowList.contains(handle) && !identity->m_blockList.contains(handle))
			notifySocket()->addContact( handle, handle, 0, AL );
	}
}

//TODO: remove (OBSOLETE)
void MSNProtocol::slotStartChatSession( QString handle )
{
	QDict<KopeteIdentity> dict=KopeteIdentityManager::manager()->identities(this);
	QDictIterator<KopeteIdentity> it( dict ); 
    for( ; MSNIdentity *identity=static_cast<MSNIdentity*>(it.current()); ++it )
	{
		if(identity->notifySocket())
		{
			identity->slotStartChatSession(handle);
			break;
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

void MSNProtocol::addGroup( const QString &groupName , const QString& contactToAdd )
{
QDict<KopeteIdentity> dict=KopeteIdentityManager::manager()->identities(this);
	QDictIterator<KopeteIdentity> it( dict ); 
    for( ; MSNIdentity *identity=static_cast<MSNIdentity*>(it.current()); ++it )
	{
		if(identity->notifySocket())
		{
			identity->addGroup(groupName,contactToAdd);
			break;
		}
	}
}



#include "msnprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

