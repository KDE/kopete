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
	m_moving=false;

	connect ( this, SIGNAL( chatToUser( QString ) ),
		protocol(), SLOT( slotStartChatSession( QString ) ) );
	connect (this , SIGNAL( moved(KopeteMetaContact*,KopeteContact*) ),
		this, SLOT (slotMoved(KopeteMetaContact*) ));
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
	kdDebug(14140) << "MSNContact::slotDeleteContact" << endl;

	MSNNotifySocket *notify = static_cast<MSNProtocol*>( protocol() )->notifySocket();
	if( notify )
	{
		m_moving=false;

		if(m_groups.isEmpty() || m_status==MSNProtocol::UNK)
		{
			kdDebug(14140) << "MSNContact::slotDeleteContact : ohoh, contact already removed from server, just delete it" <<endl;
			delete this;
			return;
		}

		for( QValueList<unsigned int>::Iterator it = m_groups.begin(); it != m_groups.end(); ++it )
		{
			notify->removeContact( contactId(), *it, MSNProtocol::FL );
		}
	}
	else
	{
		KMessageBox::error( 0l,
			i18n( "<qt>Please go online to remove contact</qt>" ),
				i18n( "MSN Plugin" ));
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
			return "msn_online";
			break;
		}
		case MSNProtocol::BSY: // Busy
		case MSNProtocol::PHN: // On the phone
		{
			return "msn_na";
			break;
		}
		case MSNProtocol::IDL: // Idle
		case MSNProtocol::AWY: // Away from computer
		case MSNProtocol::BRB: // Be right back
		case MSNProtocol::LUN: // Out to lunch
		{
			return "msn_away";
			break;
		}
		default:
			return "msn_offline";
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
		kdDebug(14140) <<"MSNContact::setInfo : unknow info " << type << " " <<data <<endl;

	
}


QValueList<unsigned int> MSNContact::groups()
{
	return m_groups;
}

void MSNContact::moveToGroup( KopeteGroup *from, KopeteGroup *to )
{
	if(!to)
	{
		removeFromGroup(from);
		return;
	}
	if(!from)
	{
		addToGroup(to);
		return;
	}
	if((to->displayName().isNull() || to->type() != KopeteGroup::Classic) && to->pluginData(protocol()).isEmpty() && m_groups.count()==1)
	{	//if this contact is in the last group and the contact moved to top level, do nothing
		//		(exepted if the group 0 is the top-level group)
		kdDebug(14140) << "MSNContact::moveToGroup: ignoring top-level group" << endl;
		return;
	}

//	kdDebug(14140) << "MSNContact::moveToGroup: " << from->displayName() << " => " << to->displayName() << endl;
	MSNNotifySocket *notify = static_cast<MSNProtocol*>( protocol() )->notifySocket();
	if( notify )
	{
		m_moving=true;
		addToGroup(to);

		QStringList strL=from->pluginData(protocol());
		if(strL.count() >= 1)
		{
			if( m_groups.contains(strL.first().toUInt()) )
				notify->removeContact( contactId(), strL.first().toUInt(), MSNProtocol::FL );
		}
	}
	else
	{
		KMessageBox::information( 0l,
			i18n( "<qt>Changes in the contact list when you are offline don't update the contact list server-side. Your changes may be lost</qt>" ),
				i18n( "MSN Plugin" ), "msn_OfflineContactList" );
	}
}

void MSNContact::addToGroup( KopeteGroup *group )
{
	if(!group)
		return;

//	kdDebug(14140) << "MSNContact::addToGroup: " << group->displayName() << endl;
	
	MSNNotifySocket *notify = static_cast<MSNProtocol*>( protocol() )->notifySocket();
	if( notify )
	{
		QStringList strL=group->pluginData(protocol());
		if(strL.count() >= 1 )
		{
			if(!m_groups.contains(strL.first().toUInt()))
					notify->addContact( contactId(), displayName(), strL.first().toUInt(), MSNProtocol::FL );
		}
		else if(group->displayName().isNull() || group->type() != KopeteGroup::Classic)
		{
			kdDebug(14140) << "MSNContact::addToGroup: ignoring top-level group" << endl;
		}
		else
		{
			static_cast<MSNProtocol*>( protocol() )->addGroup( group->displayName(), contactId() );
		}
	}
	else
	{
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

	m_moving = false;

	MSNNotifySocket *notify = static_cast<MSNProtocol*>( protocol() )->notifySocket();
	if( notify )
	{
		if( m_groups.count() == 1 )
		{
			// Do not remove the contact if this is the last group,
			// Kopete allows top-level ('groupless' contacts, but MSN doesn't
			kdDebug( 14140 ) << k_funcinfo << "Contact not removed. MSN requires all contacts to be in at least one group" << endl;
			return;
		}

		QStringList strL = group->pluginData( protocol() );
		if( strL.count() >= 1 )
		{
			if( m_groups.contains( strL.first().toUInt() ) )
				notify->removeContact( contactId(), strL.first().toUInt(), MSNProtocol::FL );
		}
	}
	else
	{
		KMessageBox::information( 0l,
			i18n( "<qt>Changes in the contact list when you are offline don't update the contact list server-side. Your changes may be lost</qt>" ),
				i18n( "MSN Plugin" ), "msn_OfflineContactList" );
	}
}

void MSNContact::slotAddedToGroup( unsigned int group )
{
	m_moving=false;
	m_groups.append( group );
}

void MSNContact::slotRemovedFromGroup(unsigned int group)
{
	m_groups.remove(group);
}

void MSNContact::slotMoved(KopeteMetaContact* from)
{
	kdDebug(14140) <<"MSNContact::slotMoved" <<endl;
	QPtrList<KopeteGroup> groups_new=metaContact()->groups();
	QPtrList<KopeteGroup> groups_old=from->groups();

	for ( KopeteGroup *g=groups_new.first(); g; g = groups_new.next() )
	{
		if(!m_groups.contains(g->pluginData(protocol()).first().toUInt()))
			addToGroup(g);
	}
	for ( KopeteGroup *g=groups_old.first(); g; g = groups_old.next() )
	{
		if(!groups_new.contains(g) && m_groups.contains(g->pluginData(protocol()).first().toUInt()))
			removeFromGroup(g);
	}
}

void MSNContact::sendFile(const KURL &sourceURL, const QString &altFileName, 
	const long unsigned int fileSize)
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

