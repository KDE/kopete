/*
    msncontact.cpp - MSN Contact

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Ryan Cumming           <bodnar42@phalynx.dhs.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "msncontact.h"

#include <qcheckbox.h>

#include <kaction.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kopetecontactlist.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"

#include "msninfo.h"
#include "msnmessagemanager.h"
#include "msnnotifysocket.h"
#include "msnprotocol.h"
#include "msnaccount.h"

MSNContact::MSNContact( KopeteIdentity *identity, const QString &id, const QString &displayName, KopeteMetaContact *parent )
: KopeteContact( identity, id, parent )
{
	m_actionBlock = 0L;
	m_actionCollection=0L;

//	m_deleted = false;
	m_allowed = false;
	m_blocked = false;
	m_reversed = false;

	setDisplayName( displayName );

	setFileCapable( true );

	setOnlineStatus( MSNProtocol::protocol()->UNK );
}

KopeteMessageManager *MSNContact::manager( bool canCreate )
{
	KopeteContactPtrList chatmembers;
	chatmembers.append(this);

	KopeteMessageManager *_manager = KopeteMessageManagerFactory::factory()->findKopeteMessageManager(  identity()->myself(), chatmembers, protocol() );
	MSNMessageManager *manager = dynamic_cast<MSNMessageManager*>( _manager );
	if(!manager &&  canCreate)
			manager = new MSNMessageManager( protocol(), identity()->myself(), chatmembers );
	return manager;
}

KActionCollection *MSNContact::customContextMenuActions()
{
	if( m_actionCollection != 0L )
		delete m_actionCollection;

	m_actionCollection = new KActionCollection(this);

	// Block/unblock Contact
	if(m_actionBlock)
		delete m_actionBlock;
	QString label = isBlocked() ?
		i18n( "Unblock User" ) : i18n( "Block User" );
	m_actionBlock = new KAction( label,
		0, this, SLOT( slotBlockUser() ),
		this, "m_actionBlock" );

	m_actionCollection->insert( m_actionBlock );

	return m_actionCollection;
}

void MSNContact::slotBlockUser()
{
	MSNNotifySocket *notify = static_cast<MSNIdentity*>( identity() )->notifySocket();
	if( !notify )
	{
		KMessageBox::error( 0l,
			i18n( "<qt>Please go online to block/unblock contact</qt>" ),
			i18n( "MSN Plugin" ));
		return;
	}

	if( m_blocked )
	{
		notify->removeContact( contactId(), 0, MSNProtocol::BL );
//		if( !m_allowed )
//			notify->addContact( contactId(), contactId(), 0, MSNProtocol::AL );
	}
	else
	{
		if(m_allowed)
			notify->removeContact( contactId(), 0, MSNProtocol::AL);
		else
			notify->addContact( contactId(), contactId(), 0, MSNProtocol::BL );
	}
}

void MSNContact::slotUserInfo()
{
	KDialogBase *infoDialog=new KDialogBase( 0l, "infoDialog", /*modal = */false, QString::null, KDialogBase::Close , KDialogBase::Close, false );
	MSNInfo *info=new MSNInfo ( infoDialog,"info");
	info->m_id->setText( contactId() );
	info->m_displayName->setText(displayName());
	info->m_phh->setText(m_phoneHome);
	info->m_phw->setText(m_phoneWork);
	info->m_phm->setText(m_phoneMobile);
	info->m_reversed->setChecked(m_reversed);

	infoDialog->setMainWidget(info);
	infoDialog->setCaption(displayName());
	infoDialog->show();
}

void MSNContact::slotDeleteContact()
{
	kdDebug( 14140 ) << k_funcinfo << endl;

	MSNNotifySocket *notify = static_cast<MSNIdentity*>( identity() )->notifySocket();
	if( notify )
	{
		if( m_serverGroups.isEmpty() || onlineStatus() == MSNProtocol::protocol()->UNK )
		{
			kdDebug( 14140 ) << k_funcinfo << "Ohoh, contact already removed from server, just delete it" << endl;
			// FIXME: 1. Shouldn't this be a deleteLater, as this is a slot???
			//        2. Why is this needed at all? Why not simply return?     - Martijn
			delete this;
			return;
		}

		for( QMap<uint, KopeteGroup*>::Iterator it = m_serverGroups.begin(); it != m_serverGroups.end(); ++it )
			notify->removeContact( contactId(), it.key(), MSNProtocol::FL );
	}
	else
	{
		// FIXME: This case should be handled by Kopete, not by the plugins :( - Martijn
		KMessageBox::error( 0L, i18n( "<qt>Please go online to remove contact</qt>" ), i18n( "MSN Plugin" ));
	}
}

bool MSNContact::isBlocked() const
{
	return m_blocked;
}

void MSNContact::setBlocked( bool blocked )
{
	if( m_blocked != blocked )
	{
		m_blocked = blocked;
		//update the status
		setOnlineStatus(onlineStatus());
	}
}

bool MSNContact::isAllowed() const
{
	return m_allowed;
}

void MSNContact::setAllowed( bool allowed )
{
	m_allowed = allowed;
}

bool MSNContact::isReversed() const
{
	return m_reversed;
}

void MSNContact::setReversed( bool reversed )
{
	m_reversed= reversed;
}

void MSNContact::setInfo( QString type, QString data )
{
	if( type == "PHH" )
	{
		m_phoneHome = data;
	}
	else if( type == "PHW" )
	{
		m_phoneWork=data;
	}
	else if( type == "PHM" )
	{
		m_phoneMobile = data;
	}
	else if( type == "MOB" )
	{
		if( data == "Y" )
			m_phone_mob = true;
		else if( data == "N" )
			m_phone_mob = false;
		else
			kdDebug( 14140 ) << k_funcinfo << "Unknown MOB " << data << endl;
	}
	else
	{
		kdDebug( 14140 ) << k_funcinfo << "Unknow info " << type << " " << data << endl;
	}
}

void MSNContact::serialize( QMap<QString, QString> &serializedData, QMap<QString, QString> & /* addressBookData */ )
{
	// Contact id and display name are already set for us, only add the rest
	QString groups;
	bool firstEntry = true;
	for( QMap<uint, KopeteGroup *>::ConstIterator it = m_serverGroups.begin(); it != m_serverGroups.end(); ++it )
	{
		if( !firstEntry )
		{
			groups += ",";
			firstEntry = true;
		}
		groups += QString::number( it.key() );
	}

	QString lists="C";
	if(m_blocked)
		lists +="B";
	if(m_allowed)
		lists +="A";
	if(m_reversed)
		lists +="R";

	serializedData[ "groups" ]  = groups;
	serializedData[ "PHH" ]  = m_phoneHome;
	serializedData[ "PHW" ]  = m_phoneWork;
	serializedData[ "PHM" ]  = m_phoneMobile;
	serializedData[ "lists" ] = lists;
}

const QMap<uint, KopeteGroup*> & MSNContact::serverGroups() const
{
	return m_serverGroups;
}

KopeteGroupList MSNContact::groups() const
{
	KopeteGroupList result;
	for( QMap<uint, KopeteGroup *>::ConstIterator it = m_serverGroups.begin(); it != m_serverGroups.end(); ++it )
		result.append( it.data() );

	return result;
}

void MSNContact::moveToGroup( KopeteGroup *from, KopeteGroup *to )
{
	kdDebug() << k_funcinfo << from->displayName() << " -> " << to->displayName() << endl;

	if( !to )
	{
		removeFromGroup( from );
		return;
	}

	if( !from )
	{
		addToGroup( to );
		return;
	}

	if( ( to->displayName().isNull() || to->type() != KopeteGroup::Classic ) &&
		to->pluginData(protocol(), identity()->identityId() + " id").isEmpty() && m_serverGroups.count() == 1 )
	{
		// If this contact is in the last group and the contact moved to top level, do nothing
		// (except when group 0 is the top-level group)
		kdDebug( 14140 ) << k_funcinfo << "Ignoring top-level group" << endl;
		return;
	}

	MSNNotifySocket *notify = static_cast<MSNIdentity*>( identity() )->notifySocket();
	if( notify )
	{
		addToGroup( to );

		if( !from->pluginData(protocol(),identity()->identityId() + " id").isEmpty() )
		{
			if( m_serverGroups.contains( from->pluginData(protocol(),identity()->identityId() + " id").toUInt() ) )
				notify->removeContact( contactId(), from->pluginData(protocol(),identity()->identityId() + " id").toUInt(), MSNProtocol::FL );
		}
	}
	else
	{
		// FIXME: This should be handled by Kopete, not the plugin :( - Martijn
		KMessageBox::information( 0l,
			i18n( "<qt>Changes in the contact list when you are offline don't update the contact list server-side. Your changes may be lost</qt>" ),
				i18n( "MSN Plugin" ), "msn_OfflineContactList" );
	}
}

void MSNContact::rename( const QString &newName )
{
	//kdDebug() << k_funcinfo << "From: " << displayName() << ", to: " << newName << endl;

	if( newName == displayName() )
		return;

	MSNNotifySocket *notify = static_cast<MSNIdentity*>( identity() )->notifySocket();
	if( notify )
	{
		notify->changePublicName( newName, contactId() );
	}
	else
	{
		// FIXME: Move this to libkopete instead - Martijn
		KMessageBox::information( 0L,
			i18n( "<qt>Changes in the contact list when you are offline don't update the contact "
				"list server-side. Your changes may be lost</qt>" ),
			i18n( "MSN Plugin" ), "msn_OfflineContactList" );
	}
}

void MSNContact::addToGroup( KopeteGroup *group )
{
	//kdDebug( 14140 ) << k_funcinfo << group->displayName() << endl;

	if( !group )
		return;

	MSNNotifySocket *notify = static_cast<MSNIdentity*>( identity() )->notifySocket();
	if( notify )
	{
		if( !group->pluginData( protocol() , identity()->identityId() + " id" ).isEmpty() )
		{
			if( !m_serverGroups.contains( group->pluginData(protocol(),identity()->identityId() + " id").toUInt() ) )
				notify->addContact( contactId(), displayName(), group->pluginData(protocol(),identity()->identityId() + " id").toUInt(), MSNProtocol::FL );
		}
		else if( group->displayName().isNull() || group->type() != KopeteGroup::Classic )
		{	//top-level group
			if( m_serverGroups.isEmpty() )
			{	//if the contact was not in the contact list, we add it on the default group. that's happends when adding temporary
				notify->addContact( contactId(), displayName(), 0, MSNProtocol::FL );
			}
			else
				kdDebug( 14140 ) << k_funcinfo << "Ignoring top-level group" << endl;
		}
		else
		{
			static_cast<MSNIdentity*>( identity() )->addGroup( group->displayName(), contactId() );
		}
	}
	else
	{
		// FIXME: Move this to libkopete instead - Martijn
		KMessageBox::information( 0l,
			i18n( "<qt>Changes in the contact list when you are offline don't update the contact list server-side. Your changes may be lost</qt>" ),
				i18n( "MSN Plugin" ), "msn_OfflineContactList" );
	}
}

void MSNContact::removeFromGroup( KopeteGroup *group )
{
	//kdDebug( 14140 ) << k_funcinfo << group->displayName() << endl;

	if( !group )
		return;

	MSNNotifySocket *notify = static_cast<MSNIdentity*>( identity() )->notifySocket();
	if( notify )
	{
		if( m_serverGroups.count() == 1 )
		{
			// Do not remove the contact if this is the last group,
			// Kopete allows top-level ('groupless' contacts, but MSN doesn't
			kdDebug( 14140 ) << k_funcinfo << "Contact not removed. MSN requires all contacts to be in at least one group" << endl;
			return;
		}

		if( !group->pluginData( protocol() , identity()->identityId() + " id" ).isEmpty() )
		{
			if( m_serverGroups.contains( group->pluginData(protocol(),identity()->identityId() + " id").toUInt() ) )
				notify->removeContact( contactId(), group->pluginData(protocol(),identity()->identityId() + " id").toUInt(), MSNProtocol::FL );
		}
	}
	else
	{
		KMessageBox::information( 0l,
			i18n( "<qt>Changes in the contact list when you are offline don't update the contact list server-side. Your changes may be lost</qt>" ),
				i18n( "MSN Plugin" ), "msn_OfflineContactList" );
	}
}

void MSNContact::contactAddedToGroup( uint groupNumber, KopeteGroup *group )
{
	m_serverGroups.insert( groupNumber, group );
}

void MSNContact::removeFromGroup( unsigned int group )
{
	m_serverGroups.remove( group );
}

/**
 * FIXME: Make this a standard KMM API call
 */
void MSNContact::sendFile( const KURL &sourceURL, const QString &altFileName, const long unsigned int fileSize )
{
	QString filePath;

	//If the file location is null, then get it from a file open dialog
	if( !sourceURL.isValid() )
		filePath = KFileDialog::getOpenFileName( QString::null ,"*.*", 0l  , i18n( "Kopete File Transfer" ));
	else
		filePath = sourceURL.path(-1);
		
	//kdDebug(14140) << "MSNContact::sendFile: File chosen to send:" << fileName << endl;

	if ( !filePath.isEmpty() )
	{
		//Send the file
		static_cast<MSNMessageManager*>( manager() )->sendFile( filePath, altFileName, fileSize );
	}
}

void MSNContact::setDisplayName(const QString &Dname)
{	//call the protocted method
	KopeteContact::setDisplayName(Dname);
}

void MSNContact::setOnlineStatus(const KopeteOnlineStatus& status)
{
	if(isBlocked() && status.internalStatus() < 15)
	{
		KopeteContact::setOnlineStatus(KopeteOnlineStatus(status.status() , (status.weight()==0) ? 0 : (status.weight() -1)  ,
			protocol() , status.internalStatus()+15 , 
			(status.status()==KopeteOnlineStatus::Offline)? QString::fromLatin1("msn_offline_blocked") : QString::fromLatin1("msn_online_blocked")  ,
			status.caption() ,  i18n("%1|Blocked").arg( status.description() ) ) );
	}
	else 
	{
		if(status.internalStatus() >= 15)
		{	//the user is not blocked, but the status is blocked
			switch(status.internalStatus()-15)
			{
				case 1:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->NLN);
					break;
				case 2:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->BSY);
					break;
				case 3:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->BRB);
					break;
				case 4:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->AWY);
					break;
				case 5:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->PHN);
					break;
				case 6:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->LUN);
					break;
				case 7:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->FLN);
					break;
				case 8:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->IDL);
					break;
				default:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->UNK);
					break;
			}
		}
		else
			KopeteContact::setOnlineStatus(status);
	}
}

#include "msncontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

