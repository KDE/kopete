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

#include "msncontact.h"
#include <contactlist.h>
#include <qcursor.h>
#include <qstringlist.h>

#include <kmessagebox.h>
#include <kdebug.h>
#include "kmsncontact.h"
#include <kmsnchatservice.h>

/* Constructor for no-groups */

MSNContact::MSNContact(QString userid, const QString name, MSNProtocol *protocol)
	: IMContact(kopeteapp->contactList())
{
	mProtocol = protocol;
	mName = name;
	mUserID = userid;
    hasLocalGroup = false;
	initContact(userid, name, protocol);
	
}


MSNContact::MSNContact(QListViewItem *parent, QString userid, const QString name, MSNProtocol *protocol)
	: IMContact(parent)
{
	mProtocol = protocol;
	mName = name;
	mUserID = userid;
    hasLocalGroup = true;
	parentGroup = parent;
	initContact(userid, name, protocol);
	
}

QString MSNContact::key(int column, bool ascending) const
{
	switch(mStatus_n)
	{
  		case BLO:
  		{
			return "G"+ text(0);
  			break;
  		}
  		case NLN:
  		{
			return "A"+ text(0);
  			break;
  		}
  		case FLN:
  		{
			return "Z"+ text(0);
  			break;
  		}
  		case BSY:
  		{
			return "F"+ text(0);
  			break;
  		}
  		case IDL:
  		{
			return "A"+ text(0);
  			break;
  		}
  		case AWY:
  		{
			return "B"+ text(0);
  			break;
  		}
  		case PHN:
  		{
			return "C"+ text(0);
  			break;
  		}
  		case BRB:
  		{
			return "D"+ text(0);
  			break;
  		}
  		case LUN:
  		{
			return "E"+ text(0);
  			break;
  		}
   	}	
}

void MSNContact::initContact(QString userid, const QString name, MSNProtocol *protocol)
{
	messageTimer = new QTimer();
	messageQueue = new QValueStack<MSNMessageStruct>;
	isMessageIcon = false;
	// We connect this signal so that we can tell when a user's status changes
	QObject::connect(protocol->engine, SIGNAL(updateContact(QString, uint)), this, SLOT(slotUpdateContact (QString, uint) ));
	QObject::connect(protocol->engine, SIGNAL(contactRemoved(QString, QString)), this, SLOT(slotContactRemoved (QString, QString) ));
	
	QObject::connect(this, SIGNAL(chatToUser(QString)), protocol->engine, SLOT( slotStartChatSession(QString)) );
	QObject::connect(messageTimer, SIGNAL(timeout()), this, SLOT(slotFlashIcon()));
	QObject::connect(protocol->engine, SIGNAL(connectedToService(bool)), this, SLOT(slotDeleteMySelf(bool)));
	QString tmp = name;
	setText(0,tmp);
	initActions();
	slotUpdateContact( mUserID, mProtocol->engine->getStatus( mUserID ) );		
}

void MSNContact::initActions()
{
	actionChat = new KAction ( i18n("Start Chat"), "idea", 0, this, SLOT(slotChatThisUser()), this, "actionChat" );
	actionRemoveFromGroup = new KAction ( i18n("Remove from group"), "edittrash", 0, this, SLOT(slotRemoveFromGroup()), this, "actionRemove" );
	actionRemove = new KAction ( i18n("Delete contact"), "edittrash", 0, this, SLOT(slotRemoveThisUser()), this, "actionDelete" );
	actionContactCopy = new KListAction ( i18n("Copy contact"), "editcopy", 0, this, SLOT(slotCopyThisUser()), this, "actionCopy" );	
	actionContactMove = new KListAction ( i18n("Move contact"), "editcut", 0, this, SLOT(slotMoveThisUser()), this, "actionMove" );	
	actionHistory = new KAction ( i18n("View History"), "history", 0, this, SLOT(slotHistory()), this, "actionDelete" );

}

void MSNContact::rightButtonPressed(const QPoint &point)
{
	QStringList grouplist1 = mProtocol->engine->getGroups();
	QStringList grouplist2 = mProtocol->engine->getGroups();
	actionContactMove->setItems(grouplist1);
	actionContactCopy->setItems(grouplist2);

	popup = new KPopupMenu();
	popup->insertTitle(mUserID);
	actionChat->plug( popup );
	actionRemoveFromGroup->plug( popup );
	actionRemove->plug( popup );
	actionContactCopy->plug( popup );
	actionContactMove->plug( popup );
	actionHistory->plug( popup );
	popup->popup(QCursor::pos());
}

void MSNContact::leftButtonDoubleClicked()
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
	group = parentGroup->text(0);
	mProtocol->engine->contactRemove(mUserID, group);
}

void MSNContact::slotMoveThisUser()
{
	QString newgroup;
	QString oldgroup;
	newgroup = actionContactMove->currentText();
	oldgroup = parentGroup->text(0);
	mProtocol->engine->contactMove( mUserID, oldgroup, newgroup);
	
}

void MSNContact::slotCopyThisUser()
{
	QString newgroup;
	newgroup = actionContactCopy->currentText();
	mProtocol->engine->contactCopy( mUserID, newgroup);
}

void MSNContact::slotHistory()
{

}

void MSNContact::slotContactRemoved(QString handle, QString group)
{
	if ( (handle == mUserID) && ( group == parentGroup->text(0) ) )
	{
		delete this;
	}
}

void MSNContact::slotUpdateContact (QString handle , uint status)
{
	if (handle == mUserID)
	{
		kdDebug() << "MSN Plugin: Contact " << handle <<" request update (" << status << ")\n";
		mStatus_n = status;
		isMessageIcon = false;
        QString tmppublicname = mProtocol->engine->getPublicName( handle);
		switch(status)
		{
   			case BLO:
   			{
   				setText(0,  tmppublicname + " ( " + i18n("Blocked") + " )" );
   				break;
   			}
   			case NLN:
   			{
   				setText(0, tmppublicname );
   				setPixmap(0, mProtocol->onlineIcon );
   				break;
   			}
   			case FLN:
   			{
				setText(0, tmppublicname );
   				setPixmap(0, mProtocol->offlineIcon );
   				break;
   			}
   			case BSY:
   			{
				setText(0, tmppublicname );
   				setPixmap(0, mProtocol->awayIcon );
   				break;
   			}
   			case IDL:
   			{
				setText(0, tmppublicname );
   				setPixmap(0, mProtocol->awayIcon );
   				break;
   			}
   			case AWY:
   			{
				setText(0, tmppublicname );
   				setPixmap(0, mProtocol->awayIcon );
   				break;
   			}
   			case PHN:
   			{
				setText(0, tmppublicname );
   				setPixmap(0, mProtocol->awayIcon );
   				break;
   			}
   			case BRB:
   			{
				setText(0, tmppublicname );
   				setPixmap(0, mProtocol->awayIcon );
   				break;
   			}
   			case LUN:
   			{
				setText(0, tmppublicname );
   				setPixmap(0, mProtocol->awayIcon );
   				break;
   			}
		}
		/* We need to resort the group */
		parentGroup->sortChildItems(0,0);
	}
}

void MSNContact::slotNewMessage(QString userid, QString publicname, QString message)
{
	/*
	if (uin == mUIN)
	{
		messageQueue->prepend(message);
		messageTimer->start(1000, false);
	}
	*/
}

void MSNContact::slotDeleteMySelf(bool connected)
{
	if (!connected)
	{
		delete this;
	}
}

void MSNContact::slotFlashIcon()
{
	/*
	if (isMessageIcon == true)
	{
		isMessageIcon = false;
		if (mStatus == STATUS_ONLINE)
		{
			setPixmap(0, mProtocol->contactOnlineIcon);
		}
		if (mStatus == STATUS_OFFLINE)
		{
			setPixmap(0, mProtocol->contactOfflineIcon);
		}
		if (mStatus == STATUS_AWAY)
		{
			setPixmap(0, mProtocol->awayIcon);
		}
		if (mStatus == STATUS_DND || mStatus == STATUS_DND_99)
		{
			setPixmap(0, mProtocol->dndIcon);
		}
		if (mStatus == STATUS_NA_99 || mStatus == STATUS_NA)
		{
			setPixmap(0, mProtocol->naIcon);
		}
		if (mStatus == STATUS_OCCUPIED || mStatus == STATUS_OCCUPIED_MAC)
		{
			setPixmap(0, mProtocol->occupiedIcon);
		}
	} else {
		isMessageIcon = true;
		setPixmap(0, mProtocol->messageIcon);
	}
	*/
}

