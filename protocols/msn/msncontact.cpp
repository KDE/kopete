/***************************************************************************
                          msncontact.cpp  -  description
                             -------------------
    begin                : Thu Jan 24 2002
    copyright            : (C) 2002 by duncan
    email                : duncan@tarro
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klocale.h>
#include "msncontact.h"
#include "msncontact.moc"

#include <contactlist.h>
#include "kmsncontact.h"
#include <kmsnchatservice.h>

#include <qcursor.h>
#include <qstringlist.h>
#include <qlistview.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qstring.h>

#include <kmessagebox.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kdebug.h>

/* Constructor for no-groups */

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

void MSNContact::initContact(QString userid, const QString name, MSNProtocol *protocol)
{
//	messageTimer = new QTimer();
	messageQueue = new QValueStack<MSNMessageStruct>;
	isMessageIcon = false;

	// We connect this signal so that we can tell when a user's status changes
	connect(protocol->engine, SIGNAL(updateContact(QString, uint)), this, SLOT(slotUpdateContact (QString, uint) ));
	connect(protocol->engine, SIGNAL(contactRemoved(QString, QString)), this, SLOT(slotContactRemoved (QString, QString) ));

	connect ( this, SIGNAL(chatToUser(QString)), protocol->engine, SLOT( slotStartChatSession(QString)) );
//	connect ( messageTimer, SIGNAL(timeout()), this, SLOT(slotFlashIcon()));
	connect ( protocol->engine, SIGNAL(connectedToService(bool)), this, SLOT(slotDeleteMySelf(bool)));

	QString tmp = name;
	setName  ( tmp );
	initActions();
	slotUpdateContact( mUserID, mProtocol->engine->getStatus( mUserID ) );		
}

void MSNContact::initActions()
{
	actionChat				= new KAction ( i18n("Start Chat"), "idea", 0, this, SLOT(slotChatThisUser()), this, "actionChat" );
	actionRemoveFromGroup	= new KAction ( i18n("Remove From Group"), "edittrash", 0, this, SLOT(slotRemoveFromGroup()), this, "actionRemove" );
	actionRemove			= new KAction ( i18n("Delete Contact"), "edittrash", 0, this, SLOT(slotRemoveThisUser()), this, "actionDelete" );
	actionContactCopy		= new KListAction ( i18n("Copy Contact"), "editcopy", 0, this, SLOT(slotCopyThisUser()), this, "actionCopy" );
	actionContactMove		= new KListAction ( i18n("Move Contact"), "editcut", 0, this, SLOT(slotMoveThisUser()), this, "actionMove" );
	actionHistory			= new KAction ( i18n("View History"), "history", 0, this, SLOT(slotViewHistory()), this, "actionHistory" );
}

void MSNContact::showContextMenu(QPoint point)
{
	QStringList grouplist1 = mProtocol->engine->getGroups();
	QStringList grouplist2 = mProtocol->engine->getGroups();
	actionContactMove->setItems(grouplist1);
	actionContactCopy->setItems(grouplist2);

	popup = new KPopupMenu();
	popup->insertTitle(mUserID);
	actionChat->plug( popup );
	popup->insertSeparator();
	actionHistory->plug( popup );
	popup->insertSeparator();
	actionRemoveFromGroup->plug( popup );
	actionRemove->plug( popup );
	actionContactCopy->plug( popup );
	actionContactMove->plug( popup );
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
	mProtocol->engine->contactDelete(mUserID);
	delete this;
}

void MSNContact::slotRemoveFromGroup()
{
	QString group;
	group = mGroup;
	mProtocol->engine->contactRemove(mUserID, group);
}

void MSNContact::slotMoveThisUser()
{
	QString newgroup;
	QString oldgroup;
	newgroup = actionContactMove->currentText();
	oldgroup = mGroup;
	mProtocol->engine->contactMove( mUserID, oldgroup, newgroup);	
}

void MSNContact::slotCopyThisUser()
{
	QString newgroup;
	newgroup = actionContactCopy->currentText();
	mProtocol->engine->contactCopy( mUserID, newgroup);
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
	isMessageIcon = false;
	QString tmppublicname = mProtocol->engine->getPublicName( handle);

	if ( status == FLN ) // offline
		setName( tmppublicname );

	switch ( status )
	{
		case BLO: // blocked
		{
			setName( i18n("%1 (Blocked)").arg(tmppublicname) );
			break;
		}
		case NLN: // Online
		{
			setName( tmppublicname );
			break;
		}
		case BSY: // Busy
		{
			setName( tmppublicname );
			break;
		}
		case IDL: // Idle
		{
			setName( tmppublicname );
			break;
		}
		case AWY: // Away from computer
		{
			setName( tmppublicname );
			break;
		}
		case PHN: // On the phone
		{
			setName( tmppublicname );
			break;
		}
		case BRB: // Be right back
		{
			setName( tmppublicname );
			break;
		}
		case LUN: // Out to lunch
		{
			setName( tmppublicname );
			break;
		}
	}
}

/*
void MSNContact::slotNewMessage(QString userid, QString publicname, QString message)
{
	if (uin == mUIN)
	{
		messageQueue->prepend(message);
		messageTimer->start(1000, false);
	}
}
*/


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
		historyDialog = new KopeteHistoryDialog(QString("kopete/msn_logs/%1.log").arg(mUserID), mName, 0, "ICQHistoryDialog");

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

