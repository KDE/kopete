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

#include <qlineedit.h>
#include <qcheckbox.h>
#include <kdialogbase.h>
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kopete.h"
#include "kopetecontactlistview.h"
#include "kopetestdaction.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "msncontact.h"
#include "msnprotocol.h"
#include "msnnotifysocket.h"
#include "kopetewindow.h"

#include "msninfo.h"

MSNContact::MSNContact( QString &protocolId, const QString &msnId,
	const QString &displayName, const QString &group,
	KopeteMetaContact *parent )
: KopeteContact( protocolId, parent )
{
	m_actionBlock = 0L;
	m_actionCollection=0L;

	m_status = MSNProtocol::FLN;

//	m_deleted = false;
	m_allowed = false;
	m_blocked = false;
	m_reversed = false;

	m_moving=false;

	historyDialog = 0L;

	m_msnId = msnId;
	if( !group.isEmpty() )
		m_groups = group;

	connect ( this, SIGNAL( chatToUser( QString ) ),
		MSNProtocol::protocol(),
		SLOT( slotStartChatSession( QString ) ) );

	setDisplayName( displayName );

	if(parent)
	{
		connect (parent , SIGNAL( movedToGroup( KopeteGroup*, KopeteGroup* , KopeteMetaContact*) ),
				this, SLOT (moveToGroup(KopeteGroup*,KopeteGroup*) ));
		connect (parent , SIGNAL( addedToGroup( KopeteGroup* , KopeteMetaContact*) ),
				this, SLOT (addToGroup(KopeteGroup*) ));
		connect (parent , SIGNAL( removedFromGroup(  KopeteGroup* , KopeteMetaContact*) ),
				this, SLOT (removeFromGroup(KopeteGroup*) ));
		connect (this , SIGNAL( moved(KopeteMetaContact*,KopeteContact*) ),
				this, SLOT (slotMoved(KopeteMetaContact*) ));
	}

}

MSNContact::~MSNContact()
{
//	kdDebug() << "MSNContact::~MSNContact" << endl;
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


QString MSNContact::id() const
{
	return m_msnId;
}

QString MSNContact::data() const
{
	return m_msnId;
}

void MSNContact::execute()
{
	emit chatToUser( m_msnId );
}

void MSNContact::slotBlockUser()
{
	MSNNotifySocket *notify=MSNProtocol::protocol()->notifySocket();
	if( !notify )
	{
		KMessageBox::error( 0l,
			i18n( "<qt>Please go online to block/unblock contact</qt>" ),
			i18n( "MSN Plugin" ));
		return;
	}

	if( m_blocked )
	{
		notify->removeContact( m_msnId, 0, MSNProtocol::BL );
		if( !m_allowed )
			notify->addContact( m_msnId, m_msnId, 0, MSNProtocol::AL );
	}
	else
	{
		if(m_allowed)
			notify->removeContact( m_msnId, 0, MSNProtocol::AL);
		notify->addContact( m_msnId, m_msnId, 0, MSNProtocol::BL );
	}
}

void MSNContact::slotViewHistory()
{
	kdDebug() << "MSN Plugin: slotViewHistory()" << endl;

	if (historyDialog != 0L)
	{
		historyDialog->raise();
	}
	else
	{
		historyDialog = new KopeteHistoryDialog(QString("msn_logs/%1.log").arg(m_msnId), displayName(), true, 50, 0, "MSNHistoryDialog");

		connect ( historyDialog, SIGNAL(closing()), this, SLOT(slotCloseHistoryDialog()) );
		connect ( historyDialog, SIGNAL(destroyed()), this, SLOT(slotHistoryDialogClosing()) );
	}
}

void MSNContact::slotCloseHistoryDialog()
{
	kdDebug() << "MSN Plugin: slotCloseHistoryDialog()" << endl;
	delete historyDialog;
}

void MSNContact::slotHistoryDialogClosing()
{
	kdDebug() << "MSN Plugin: slotHistoryDialogClosing()" << endl;
	if (historyDialog != 0L)
	{
		historyDialog = 0L;
	}
}

void MSNContact::slotUserInfo()
{
	KDialogBase *infoDialog=new KDialogBase( 0l, "infoDialog", /*modal = */false, QString::null, KDialogBase::Close , KDialogBase::Close, false );
	MSNInfo *info=new MSNInfo ( infoDialog,"info");
	info->m_id->setText(m_msnId);
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
	kdDebug() << "MSNContact::slotDeleteContact" << endl;

	MSNNotifySocket *notify=MSNProtocol::protocol()->notifySocket();
	if( notify )
	{
		m_moving=false;

		if(m_groups.isEmpty())
		{
			kdDebug() << "MSNContact::slotDeleteContact : ohoh, contact already removed from server, just delete it" <<endl;
			delete this;
			return;
		}

		for( QStringList::Iterator it = m_groups.begin(); it != m_groups.end(); ++it )
		{
			notify->removeContact( m_msnId, MSNProtocol::protocol()->groupNumber( *it ), MSNProtocol::FL );
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
		default:
		{
			return Offline;
			break;
		}
	}
}

QString MSNContact::statusText() const
{
	QString statusText="";
	switch ( m_status )
	{
		case MSNProtocol::BLO: // blocked -- not used
		{
			return i18n("Blocked");
			break;
		}
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
		default:
		{
			statusText= i18n("Offline");
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
		case MSNProtocol::BLO: // blocked
		{
			return 1;
			break;
		}
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

QString MSNContact::msnId() const
{
	return m_msnId;
}

void MSNContact::setMsnId( const QString &id )
{
	m_msnId = id;
}

MSNProtocol::Status MSNContact::msnStatus() const
{
	return m_status;
}

void MSNContact::setMsnStatus( MSNProtocol::Status _status )
{
	if( m_status == _status )
		return;

	kdDebug() << "MSNContact::setMsnStatus: Setting status for " << m_msnId <<
		" to " << _status << endl;
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
			kdDebug() <<"MSNContact::setInfo : unknow MOB " << data <<endl;
	}
	else
		kdDebug() <<"MSNContact::setInfo : unknow info " << type << " " <<data <<endl;

	
}


QStringList MSNContact::groups()
{
	return m_groups;
}

void MSNContact::moveToGroup( KopeteGroup *to, KopeteGroup *from )
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

//	kdDebug() << "MSNContact::moveToGroup" <<endl;
	MSNNotifySocket *notify=MSNProtocol::protocol()->notifySocket();
	if( notify )
	{
		m_moving=true;
		addToGroup(to);
		int g = MSNProtocol::protocol()->groupNumber( from->displayName() );
		if( g != -1 && m_groups.contains(from->displayName()))
			notify->removeContact( m_msnId, g, MSNProtocol::FL );
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
//	kdDebug() << "MSNContact::addToGroup" <<endl;
	if(!group)
		return;
	if(group->displayName().isNull())
	{
		kdDebug() << "MSNContact::addToGroup: ignoring top-level group" << endl;
		return;
	}
	if(m_groups.contains(group->displayName()))
		return;

	MSNNotifySocket *notify=MSNProtocol::protocol()->notifySocket();
	if( notify )
	{
		int g = MSNProtocol::protocol()->groupNumber( group->displayName() );
		if(g!=-1)
		{
			notify->addContact( m_msnId, m_msnId, g, MSNProtocol::FL );
		}
		else
		{
			MSNProtocol::protocol()->addGroup(group->displayName(), m_msnId);
		}
	}
	else
	{
		KMessageBox::information( 0l,
			i18n( "<qt>Changes in the contact list when you are offline don't update the contact list server-side. Your changes may be lost</qt>" ),
				i18n( "MSN Plugin" ), "msn_OfflineContactList" );
	}
}

void MSNContact::removeFromGroup( KopeteGroup * group )
{
//	kdDebug() << "MSNContact::removeFromGroup" <<endl;
	if(!group)
		return;
	if(group->displayName().isNull())
	{
		kdDebug() << "MSNContact::removeFromGroup: ignoring top-level group" << endl;
		return;
	}
	if(!m_groups.contains(group->displayName()))
		return;

	m_moving=false;

	MSNNotifySocket *notify=MSNProtocol::protocol()->notifySocket();
	if( notify )
	{
		if(m_groups.count()==1)
		{
			//Do not remove the contact if he has no group:
			//Kopete allow top-level contact
			kdDebug() << "MSNContact::removeFromGroup : contact not removed.  MSN requires all contacts to be in at least one group" <<endl;
			return;
		}
		int g = MSNProtocol::protocol()->groupNumber( group->displayName() );
		if( g != -1 )
			notify->removeContact( m_msnId, g, MSNProtocol::FL );
	}
	else
	{
		KMessageBox::information( 0l,
			i18n( "<qt>Changes in the contact list when you are offline don't update the contact list server-side. Your changes may be lost</qt>" ),
				i18n( "MSN Plugin" ), "msn_OfflineContactList" );
	}
}

void MSNContact::addedToGroup(QString group)
{
	m_moving=false;
	m_groups.append(group);
}
void MSNContact::removedFromGroup(QString group)
{
	m_groups.remove(group);
}

void MSNContact::addThisTemporaryContact(KopeteGroup* group)
{
	if(!group || group->displayName().isNull())
		MSNProtocol::protocol()->addContact( m_msnId );
	else
		addToGroup(  group );
}

void MSNContact::slotMoved(KopeteMetaContact* from)
{
	kdDebug() <<"MSNContact::slotMoved" <<endl;
	QStringList groups_new=metaContact()->groups().toStringList();
	QStringList groups_old=from->groups().toStringList();
	QStringList groups_current=groups();

	for( QStringList::ConstIterator it = groups_new.begin(); it != groups_new.end(); ++it )
	{
		QString group=*it;
		if(!groups_current.contains(group))
			addToGroup(KopeteContactList::contactList()->getGroup(group));
	}
	for( QStringList::ConstIterator it = groups_old.begin(); it != groups_old.end(); ++it )
	{
		QString group=*it;
		if(groups_current.contains(group) && !groups_new.contains(group))
			removeFromGroup(KopeteContactList::contactList()->getGroup(group));
	}

	disconnect (from , SIGNAL( movedToGroup( KopeteGroup*, KopeteGroup* , KopeteMetaContact*) ),
			this, SLOT (moveToGroup(KopeteGroup*,KopeteGroup*) ));
	disconnect (from , SIGNAL( addedToGroup( KopeteGroup* , KopeteMetaContact*) ),
			this, SLOT (addToGroup(KopeteGroup*) ));
	disconnect (from , SIGNAL( removedFromGroup(  KopeteGroup* , KopeteMetaContact*) ),
			this, SLOT (removeFromGroup(KopeteGroup*) ));

	connect (metaContact() , SIGNAL( movedToGroup( KopeteGroup*, KopeteGroup* , KopeteMetaContact*) ),
			this, SLOT (moveToGroup(KopeteGroup*,KopeteGroup*) ));
	connect (metaContact() , SIGNAL( addedToGroup( KopeteGroup* , KopeteMetaContact*) ),
			this, SLOT (addToGroup(KopeteGroup*) ));
	connect (metaContact() , SIGNAL( removedFromGroup(  KopeteGroup* , KopeteMetaContact*) ),
			this, SLOT (removeFromGroup(KopeteGroup*) ));


}

#include "msncontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

