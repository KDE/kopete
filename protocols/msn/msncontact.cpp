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

#include "kmsnservice.h"
//#include "kopetechatwindow.h" // Debug, Martijn
#include "kopetestdaction.h"
#include "msncontact.h"
#include "msnprotocol.h"

// Constructor for no-groups
MSNContact::MSNContact( const QString &msnId, const QString &nickname,
	const QString &group, QObject *parent )
	: KopeteContact( parent )
{
	initContact( msnId, nickname, group );
}

void MSNContact::initContact( const QString &msnId, const QString &nickname,
	const QString &group )
{
	m_actionRemove = 0L;
	m_actionRemoveFromGroup = 0L;
	m_actionChat = 0L;
	m_actionInfo = 0L;
	m_actionHistory = 0L;
	m_actionMove = 0L;
	m_actionCopy = 0L;

	m_status = MSNProtocol::FLN;

	m_deleted = false;
	m_allowed = false;
	m_blocked = false;

	historyDialog = 0L;

	m_msnId = msnId;
	m_nickname = nickname;
	m_groups = group;
	hasLocalGroup = false;

	connect ( this, SIGNAL( chatToUser( QString ) ),
		MSNProtocol::protocol()->msnService(),
		SLOT( slotStartChatSession( QString ) ) );

	setName( nickname );
}

void MSNContact::showContextMenu(QPoint point, QString /*group*/)
{
	popup = new KPopupMenu();
	popup->insertTitle( i18n( "%1 (%2)" ).arg( name() ).arg( msnId() ) );

	// Chat with user
	if( !m_actionChat )
	{
		m_actionChat = KopeteStdAction::sendMessage(
			this, SLOT( slotChatThisUser() ), this, "m_actionChat" );
	}
	m_actionChat->plug( popup );
	popup->insertSeparator();

	// View History
	if( !m_actionHistory )
	{
		m_actionHistory = KopeteStdAction::viewHistory( this,
			SLOT( slotViewHistory() ), this, "m_actionHistory" );
	}
	m_actionHistory->plug( popup );
	popup->insertSeparator();

	// Move Contact
	if( !m_actionMove )
	{
		m_actionMove = KopeteStdAction::moveContact( this,
			SLOT( slotMoveThisUser() ), this, "m_actionMove" );
	}
	m_actionMove->plug( popup );

	// Copy Contact
	if( !m_actionCopy )
	{
		m_actionCopy = new KListAction( i18n( "Copy Contact" ), "editcopy", 0,
			this, SLOT( slotCopyThisUser() ), this, "m_actionCopy" );
	}
	m_actionCopy->setItems( MSNProtocol::protocol()->groups() );
	m_actionCopy->plug( popup );

	// Remove From Group
	if( !m_actionRemoveFromGroup )
	{
		m_actionRemoveFromGroup = new KAction( i18n( "Remove From Group" ),
			"edittrash", 0, this, SLOT( slotRemoveFromGroup() ),
			this, "m_actionRemoveFromGroup" );
	}
	m_actionRemoveFromGroup->plug( popup );

	// Remove Contact
	if( !m_actionRemove )
	{
		m_actionRemove = KopeteStdAction::deleteContact( this,
			SLOT( slotRemoveThisUser() ), this, "m_actionRemove" );
	}
	m_actionRemove->plug( popup );

	popup->popup( point );
}

void MSNContact::execute()
{
//	( new KopeteChatWindow( this, this, QString::null, 0 ) )->show(); // Debug, Martijn
	emit chatToUser( m_msnId );
}

void MSNContact::slotChatThisUser()
{
	emit chatToUser( m_msnId );
}

void MSNContact::slotRemoveThisUser()
{
	MSNProtocol::protocol()->removeContact( this );
//	delete this;
}

void MSNContact::slotRemoveFromGroup()
{
	// FIXME: This slot needs to know to remove from WHICH group!
	// for now, remove from the first group...
	MSNProtocol::protocol()->removeFromGroup( this, m_groups.first() );
}

void MSNContact::slotMoveThisUser()
{
	// FIXME: originating group should also be provided!
	if( m_actionMove )
	{
		MSNProtocol::protocol()->moveContact( this, m_groups.first(),
			m_actionMove->currentText() );
	}
}

void MSNContact::slotCopyThisUser()
{
	if( m_actionCopy )
		MSNProtocol::protocol()->copyContact( this, m_actionCopy->currentText() );
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
		historyDialog = new KopeteHistoryDialog(QString("kopete/msn_logs/%1.log").arg(m_msnId), name(), true, 50, 0, "MSNHistoryDialog");

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

MSNContact::ContactStatus MSNContact::status() const
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

		default:
		{
			return Offline;
			break;
		}
	}

}

QString MSNContact::statusText() const
{
	switch ( m_status )
	{
		case MSNProtocol::BLO: // blocked
		{
			return i18n("Blocked");
			break;
		}
		case MSNProtocol::NLN: // Online
		{
			return i18n("Online");
			break;
		}
		case MSNProtocol::BSY: // Busy
		{
			return i18n("Busy");
			break;
		}
		case MSNProtocol::IDL: // Idle
		{
			return i18n("Idle");
			break;
		}
		case MSNProtocol::AWY: // Away from computer
		{
			return i18n("Away From Computer");
			break;
		}
		case MSNProtocol::PHN: // On the phone
		{
			return i18n("On The Phone");
			break;
		}
		case MSNProtocol::BRB: // Be right back
		{
			return i18n("Be Right Back");
			break;
		}
		case MSNProtocol::LUN: // Out to lunch
		{
			return i18n("Out To Lunch");
			break;
		}

		default:
		{
			return i18n("Offline");
		}
	}
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

QString MSNContact::nickname() const
{
	return m_nickname;
}

QString MSNContact::name() const
{
	if( m_blocked )
		return i18n( "%1 (Blocked)" ).arg( nickname() );
	else
		return nickname();
}

void MSNContact::setNickname( const QString &nick )
{
	if( m_nickname != nick )
	{
		m_nickname = nick;
		emit nameChanged( name() );
	}
}

MSNProtocol::Status MSNContact::msnStatus() const
{
	return m_status;
}

void MSNContact::setMsnStatus( MSNProtocol::Status status )
{
	if( m_status == status )
		return;

	kdDebug() << "MSNContact::setMsnStatus: Setting status for " << m_msnId <<
		" to " << status << endl;
	m_status = status;

	if( m_status == MSNProtocol::BLO )
		setNickname( i18n( "%1 (Blocked)").arg( nickname() ) );

	emit statusChanged();
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
		emit nameChanged( name() );
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

QStringList MSNContact::groups() const
{
	return m_groups;
}

void MSNContact::addToGroup( const QString &group )
{
	m_groups.append( group );
}

void MSNContact::removeFromGroup( const QString &group )
{
	m_groups.remove( group );
}

#include "msncontact.moc"

// vim: noet ts=4 sts=4 sw=4:

