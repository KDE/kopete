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

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>

#include "kopete.h"
#include "kopetecontactlistview.h"
#include "kopetestdaction.h"
#include "msncontact.h"
#include "msnprotocol.h"

MSNContact::MSNContact( QString &protocolId, const QString &msnId,
	const QString &displayName, const QString &group,
	KopeteMetaContact *parent )
: KopeteContact( protocolId, parent )
{
	m_actionBlock = 0L;
	m_actionCollection=0L;

	m_status = MSNProtocol::FLN;

	m_deleted = false;
	m_allowed = false;
	m_blocked = false;

	m_moving=false;

	historyDialog = 0L;

	m_msnId = msnId;
	if( !group.isEmpty() )
		m_groups = group;
	hasLocalGroup = false;

	connect ( this, SIGNAL( chatToUser( QString ) ),
		MSNProtocol::protocol(),
		SLOT( slotStartChatSession( QString ) ) );

	setDisplayName( displayName );
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
	if( isBlocked() )
		MSNProtocol::protocol()->contactUnBlock( m_msnId );
	else
		MSNProtocol::protocol()->blockContact( m_msnId );
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
	//TODO: show a dialog box with names, phone number and other info
}

void MSNContact::slotDeleteContact()
{
	kdDebug() << "MSNContact::slotDeleteContact" << endl;
	m_moving=false;
	MSNProtocol::protocol()->removeContact( this );
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
	emit statusChanged(  );
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

bool MSNContact::isDeleted() const
{
	return m_deleted;
}

void MSNContact::setDeleted( bool deleted )
{
	m_deleted = deleted;
}

bool MSNContact::isAllowed() const
{
	return m_allowed;
}

void MSNContact::setAllowed( bool allowed )
{
	m_allowed = allowed;
}

QStringList MSNContact::groups()
{
	return m_groups;
}

void MSNContact::moveToGroup( const QString &from, const QString &to )
{
	if(to.isNull())
	{
		removeFromGroup(from);
		return;
	}
	if(from.isNull())
	{
		addToGroup(to);
		return;
	}

	m_moving=true;
	MSNProtocol::protocol()->moveContact( this, from, to );
}

void MSNContact::addToGroup( const QString &group )
{
	if(group.isNull())
	{
		kdDebug() << "MSNContact::addToGroup: ignoring top-level group" << endl;
		return;
	}
	if(!m_groups.contains(group))
		MSNProtocol::protocol()->addContactToGroup( this, group );
}

void MSNContact::removeFromGroup( const QString &group )
{
	if(group.isNull())
	{
		kdDebug() << "MSNContact::removeFromGroup: ignoring top-level group" << endl;
		return;
	}
	m_moving=false;
	if(m_groups.contains(group))
		MSNProtocol::protocol()->removeContactFromGroup( this, group );
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


#include "msncontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

