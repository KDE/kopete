/*
    msncontact.cpp - MSN Contact

    Copyright (c) 2002 Duncan Mac-Vicar Prett <duncan@kde.org>
              (c) 2002 Ryan Cumming           <bodnar42@phalynx.dhs.org>
              (c) 2002 Martijn Klingens       <klingens@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
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

#include "msninfo.h"
#include "msnmessagemanager.h"
#include "msnnotifysocket.h"
#include "msnprotocol.h"

MSNContact::MSNContact( KopeteProtocol *proto, const QString &id,
	const QString &displayName, KopeteMetaContact *parent ) : KopeteContact( proto, id, parent )
{
	m_actionBlock = 0L;
	m_actionCollection=0L;

	m_status = MSNProtocol::UNK;

//	m_deleted = false;
	m_allowed = false;
	m_blocked = false;
	m_reversed = false;

	connect ( this, SIGNAL( chatToUser( QString ) ),
		protocol(), SLOT( slotStartChatSession( QString ) ) );
	setDisplayName( displayName );

	setFileCapable(true);
}

MSNContact::~MSNContact()
{
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

QString MSNContact::data() const
{
	return contactId();
}

void MSNContact::execute()
{
	emit chatToUser( contactId() );
}

void MSNContact::slotBlockUser()
{
	MSNNotifySocket *notify = static_cast<MSNProtocol*>( protocol() )->notifySocket();
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

	MSNNotifySocket *notify = static_cast<MSNProtocol*>( protocol() )->notifySocket();
	if( notify )
	{
		if( m_serverGroups.isEmpty() || m_status == MSNProtocol::UNK )
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

KopeteContact::ContactStatus MSNContact::status() const
{
	switch ( m_status )
	{
		case MSNProtocol::NLN: // Online
		{
			return Online;
			break;
		}
		case MSNProtocol::BSY: // Busy
		case MSNProtocol::IDL: // Idle
		case MSNProtocol::AWY: // Away from computer
		case MSNProtocol::PHN: // On the phone
		case MSNProtocol::BRB: // Be right back
		case MSNProtocol::LUN: // Out to lunch
		{
			return Away;
			break;
		}
		case MSNProtocol::FLN: // Offline
		{
			return Offline;
			break;
		}
		case MSNProtocol::UNK:
		default:
		{
			return Unknown;
		}
	}
}

QString MSNContact::statusText() const
{
	QString statusText="";
	switch ( m_status )
	{
/*		case MSNProtocol::BLO: // blocked -- not used
		{
			return i18n("Blocked");
			break;
		}*/
		case MSNProtocol::NLN: // Online
		{
			statusText= i18n("Online");
			break;
		}
		case MSNProtocol::BSY: // Busy
		{
			statusText= i18n("Busy");
			break;
		}
		case MSNProtocol::IDL: // Idle
		{
			statusText= i18n("Idle");
			break;
		}
		case MSNProtocol::AWY: // Away from computer
		{
			statusText= i18n("Away From Computer");
			break;
		}
		case MSNProtocol::PHN: // On the phone
		{
			statusText= i18n("On the Phone");
			break;
		}
		case MSNProtocol::BRB: // Be right back
		{
			statusText= i18n("Be Right Back");
			break;
		}
		case MSNProtocol::LUN: // Out to lunch
		{
			statusText= i18n("Out to Lunch");
			break;
		}
		case MSNProtocol::FLN: // offline
		{
			statusText= i18n("Offline");
			break;
		}
		default:
		{
			statusText= i18n("Status not avaliable");
		}
	}
	if(isBlocked())
		statusText += i18n("|Blocked");
	return statusText;
}

QString MSNContact::statusIcon() const
{
	switch ( m_status )
	{
		case MSNProtocol::NLN: // Online
		{
			return isBlocked() ? "msn_online_blocked" : "msn_online";
			break;
		}
		case MSNProtocol::BSY: // Busy
		case MSNProtocol::PHN: // On the phone
		{
			return isBlocked() ? "msn_online_blocked" : "msn_na";
			break;
		}
		case MSNProtocol::IDL: // Idle
		case MSNProtocol::AWY: // Away from computer
		case MSNProtocol::BRB: // Be right back
		case MSNProtocol::LUN: // Out to lunch
		{
			return isBlocked() ? "msn_online_blocked" : "msn_away";
			break;
		}
		default:
			return isBlocked() ? "msn_offline_blocked" : "msn_offline";
	}
}

int MSNContact::importance() const
{
	switch ( m_status )
	{
/*		case MSNProtocol::BLO: // blocked
		{
			return 1;
			break;
		}*/
		case MSNProtocol::NLN: // Online
		{
			return 20;
			break;
		}
		case MSNProtocol::BSY: // Busy
		{
			return 13;
			break;
		}
		case MSNProtocol::IDL: // Idle
		{
			return 15;
			break;
		}
		case MSNProtocol::AWY: // Away from computer
		{
			return 10;
			break;
		}
		case MSNProtocol::PHN: // On the phone
		{
			return 12;
			break;
		}
		case MSNProtocol::BRB: // Be right back
		{
			return 14;
			break;
		}
		case MSNProtocol::LUN: // Out to lunch
		{
			return 11;
			break;
		}

		default:
		{
			return 0;
		}
	}
}

MSNProtocol::Status MSNContact::msnStatus() const
{
	return m_status;
}

void MSNContact::setMsnStatus( MSNProtocol::Status _status )
{
	if( m_status == _status )
		return;

//	kdDebug(14140) << "MSNContact::setMsnStatus: Setting status for " << contactId() <<
//		" to " << _status << endl;
	m_status = _status;

	emit statusChanged( this, status() );
//	emit statusChanged(  );
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
		emit statusChanged( this, MSNContact::status() );
	}
}

/*bool MSNContact::isDeleted() const
{
	return m_deleted;
}

void MSNContact::setDeleted( bool deleted )
{
	m_deleted = deleted;
} */

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


void MSNContact::setInfo(QString type, QString data)
{
	if( type == "PHH")
	{
		m_phoneHome=data;
	}
	else if( type == "PHW")
	{
		m_phoneWork=data;
	}
	else if( type == "PHM")
	{
		m_phoneMobile=data;
	}
	else if( type == "MOB")
	{
		if(data=="Y")
			m_phone_mob=true;
		else if (data=="N")
			m_phone_mob=false;
		else
			kdDebug(14140) <<"MSNContact::setInfo : unknow MOB " << data <<endl;
	}
	else
	{
		kdDebug(14140) <<"MSNContact::setInfo : unknow info " << type << " " <<data <<endl;
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

	serializedData[ "groups" ]  = groups;
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
		to->pluginData(protocol(),"id").isEmpty() && m_serverGroups.count() == 1 )
	{
		// If this contact is in the last group and the contact moved to top level, do nothing
		// (except when group 0 is the top-level group)
		kdDebug( 14140 ) << k_funcinfo << "Ignoring top-level group" << endl;
		return;
	}

	MSNNotifySocket *notify = static_cast<MSNProtocol*>( protocol() )->notifySocket();
	if( notify )
	{
		addToGroup( to );

		if( !from->pluginData(protocol(),"id").isEmpty() )
		{
			if( m_serverGroups.contains( from->pluginData(protocol(),"id").toUInt() ) )
				notify->removeContact( contactId(), from->pluginData(protocol(),"id").toUInt(), MSNProtocol::FL );
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

	MSNNotifySocket *notify = static_cast<MSNProtocol*>( protocol() )->notifySocket();
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

	MSNNotifySocket *notify = static_cast<MSNProtocol*>( protocol() )->notifySocket();
	if( notify )
	{
		if( !group->pluginData( protocol() , "id" ).isEmpty() )
		{
			if( !m_serverGroups.contains( group->pluginData(protocol(),"id").toUInt() ) )
				notify->addContact( contactId(), displayName(), group->pluginData(protocol(),"id").toUInt(), MSNProtocol::FL );
		}
		else if( group->displayName().isNull() || group->type() != KopeteGroup::Classic )
		{
			kdDebug( 14140 ) << k_funcinfo << "Ignoring top-level group" << endl;
		}
		else
		{
			static_cast<MSNProtocol*>( protocol() )->addGroup( group->displayName(), contactId() );
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

	MSNNotifySocket *notify = static_cast<MSNProtocol*>( protocol() )->notifySocket();
	if( notify )
	{
		if( m_serverGroups.count() == 1 )
		{
			// Do not remove the contact if this is the last group,
			// Kopete allows top-level ('groupless' contacts, but MSN doesn't
			kdDebug( 14140 ) << k_funcinfo << "Contact not removed. MSN requires all contacts to be in at least one group" << endl;
			return;
		}

		if( !group->pluginData( protocol() , "id" ).isEmpty() )
		{
			if( m_serverGroups.contains( group->pluginData(protocol(),"id").toUInt() ) )
				notify->removeContact( contactId(), group->pluginData(protocol(),"id").toUInt(), MSNProtocol::FL );
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
		KopeteContactPtrList chatmembers;
		chatmembers.append( this );
		KopeteMessageManager *_manager =
			KopeteMessageManagerFactory::factory()->findKopeteMessageManager(
				protocol()->myself(), chatmembers,
				protocol() );
		MSNMessageManager *manager= dynamic_cast<MSNMessageManager*>(_manager);
		if( !manager )
			manager = new MSNMessageManager( protocol(), protocol()->myself(), chatmembers );

		//Send the file
		manager->sendFile( filePath, altFileName, fileSize );
	}
}


#include "msncontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

