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

#include "historydialog.h"
#include "kmsnservice.h"
#include "kopetestdaction.h"
#include "msncontact.h"
#include "msnprotocol.h"

// Constructor for no-groups
MSNContact::MSNContact(QString userid, const QString name, QString group, MSNProtocol *protocol)
	: KopeteContact(protocol)
{
	mProtocol = protocol;
	mName = name;
	mGroup =  group;
	mUserID = userid;
	hasLocalGroup = false;

	historyDialog = 0L;

	initContact(userid, name, protocol);
}

void MSNContact::initContact( QString /* userid */, const QString name, MSNProtocol *protocol)
{
	// We connect this signal so that we can tell when a user's status changes
	connect( protocol, SIGNAL( updateContact( QString, uint ) ),
				this, SLOT( slotUpdateContact( QString, uint ) ) );
	connect( protocol, SIGNAL( contactRemoved( QString, QString ) ),
				this, SLOT( slotContactRemoved( QString, QString ) ) );

	connect ( this, SIGNAL(chatToUser(QString)), protocol->msnService(), SLOT( slotStartChatSession(QString)) );
	connect ( protocol, SIGNAL( connectedToService( bool ) ), this, SLOT( slotDeleteMySelf( bool ) ) );

	QString tmp = name;
	setName  ( tmp );
	initActions();
	slotUpdateContact( mUserID, mProtocol->contactStatus( mUserID ) );
}

void MSNContact::initActions()
{
	actionChat				= KopeteStdAction::sendMessage(this, SLOT(slotChatThisUser()), this, "actionChat" );
	actionRemoveFromGroup	= new KAction( i18n("Remove From Group"), "edittrash", 0, this, SLOT(slotRemoveFromGroup()), this, "actionRemove" );
	actionRemove			= KopeteStdAction::deleteContact(this, SLOT(slotRemoveThisUser()), this, "actionDelete" );
	actionContactCopy		= new KListAction ( i18n("Copy Contact"), "editcopy", 0, this, SLOT(slotCopyThisUser()), this, "actionCopy" );
	actionContactMove		= KopeteStdAction::moveContact(this, SLOT(slotMoveThisUser()), this, "actionMove" );
	actionHistory			= KopeteStdAction::viewHistory(this, SLOT(slotViewHistory()), this, "actionHistory" );
}

void MSNContact::showContextMenu(QPoint point)
{
	QStringList grouplist = mProtocol->groups();
	actionContactCopy->setItems(grouplist);

	popup = new KPopupMenu();
	popup->insertTitle(mUserID);
	actionChat->plug( popup );
	popup->insertSeparator();
	actionHistory->plug( popup );
	popup->insertSeparator();
	actionContactMove->plug( popup );
	actionContactCopy->plug( popup );
	actionRemoveFromGroup->plug( popup );
	actionRemove->plug( popup );
	popup->popup(point);
}

void MSNContact::execute()
{
	emit chatToUser( mUserID );
}

void MSNContact::slotChatThisUser()
{
	emit chatToUser( mUserID );
}

void MSNContact::slotRemoveThisUser()
{
	mProtocol->removeContact( mUserID );
	delete this;
}

void MSNContact::slotRemoveFromGroup()
{
	mProtocol->removeFromGroup( mUserID, mGroup );
}

void MSNContact::slotMoveThisUser()
{
	mProtocol->moveContact( mUserID, mGroup, actionContactMove->currentText() );
}

void MSNContact::slotCopyThisUser()
{
	mProtocol->copyContact( mUserID, actionContactCopy->currentText() );
}

void MSNContact::slotContactRemoved(QString handle, QString group)
{
	if ( (handle == mUserID) && ( group == mGroup ) )
	{
		delete this;
	}
}

void MSNContact::slotUpdateContact ( QString handle, uint status)
{
	if (handle != mUserID) // not our contact
		return;

	if ( status == mStatus ) // no statuschange
		return;

	kdDebug() << "MSN Plugin: Contact " << handle <<" request update (" << status << ")\n";
	mStatus = status;
	QString tmppublicname = mProtocol->publicName( handle );

	if (mStatus == BLO)
		setName( i18n("%1 (Blocked)").arg(tmppublicname) );
	else
		setName( tmppublicname );

	emit statusChanged();
}

void MSNContact::slotDeleteMySelf(bool connected)
{
	if (!connected)
	{
		delete this;
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
		historyDialog = new KopeteHistoryDialog(QString("kopete/msn_logs/%1.log").arg(mUserID), mName, true, 50, 0, "MSNHistoryDialog");

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
	switch ( mStatus )
	{
		case NLN: // Online
		{
			return Online;
			break;
		}
		case BSY: // Busy
		case IDL: // Idle
		case AWY: // Away from computer
		case PHN: // On the phone
		case BRB: // Be right back
		case LUN: // Out to lunch
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
	switch ( mStatus )
	{
		case BLO: // blocked
		{
			return i18n("Blocked");
			break;
		}
		case NLN: // Online
		{
			return i18n("Online");
			break;
		}
		case BSY: // Busy
		{
			return i18n("Busy");
			break;
		}
		case IDL: // Idle
		{
			return i18n("Idle");
			break;
		}
		case AWY: // Away from computer
		{
			return i18n("Away From Computer");
			break;
		}
		case PHN: // On the phone
		{
			return i18n("On The Phone");
			break;
		}
		case BRB: // Be right back
		{
			return i18n("Be Right Back");
			break;
		}
		case LUN: // Out to lunch
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
	switch ( mStatus )
	{
		case NLN: // Online
		{
			return "msn_online";
			break;
		}
		case BSY: // Busy
		case PHN: // On the phone
		{
			return "msn_na";
			break;
		}
		case IDL: // Idle
		case AWY: // Away from computer
		case BRB: // Be right back
		case LUN: // Out to lunch
		{
			return "msn_away";
			break;
		}
	}

	return "msn_offline";
}

int MSNContact::importance() const
{
	switch ( mStatus )
	{
		case BLO: // blocked
		{
			return 1;
			break;
		}
		case NLN: // Online
		{
			return 20;
			break;
		}
		case BSY: // Busy
		{
			return 13;
			break;
		}
		case IDL: // Idle
		{
			return 15;
			break;
		}
		case AWY: // Away from computer
		{
			return 10;
			break;
		}
		case PHN: // On the phone
		{
			return 12;
			break;
		}
		case BRB: // Be right back
		{
			return 14;
			break;
		}
		case LUN: // Out to lunch
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

#include "msncontact.moc"

// vim: noet ts=4 sts=4 sw=4:

