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

void MSNContact::initContact(QString userid, const QString name, MSNProtocol *protocol)
{
    messageTimer = new QTimer();
	messageQueue = new QValueStack<MSNMessageStruct>;
	isMessageIcon = false;
	// We connect this signal so that we can tell when a user's status changes
	QObject::connect(protocol->engine, SIGNAL(updateContact(QString, uint)), this, SLOT(slotUpdateContact (QString, uint) ));
	QObject::connect(protocol->engine, SIGNAL(startChat(KMSNChatService *, QString)), this, SLOT(slotIncomingChat (KMSNChatService *, QString) ));
	QObject::connect(protocol->engine, SIGNAL(contactRemoved(QString, QString)), this, SLOT(slotContactRemoved (QString, QString) ));
	
	QObject::connect(this, SIGNAL(chatToUser(QString)), protocol->engine, SLOT( slotStartChatSession(QString)) );
	QObject::connect(messageTimer, SIGNAL(timeout()), this, SLOT(slotFlashIcon()));
    QObject::connect(protocol->engine, SIGNAL(connectedToService(bool)), this, SLOT(slotDeleteMySelf(bool)));
	QString tmp = name;
	//tmp.append(" (");
	//tmp.append(QString::number(uin));
	//tmp.append(")");
	setText(0,tmp);
    initActions();
	//slotUserStateChanged(uin, (protocol->kxContacts->getContact(uin)).status, 0);
		
}

void MSNContact::initActions()
{
	actionChat = new KAction ( i18n("Start Chat"), "idea", 0, this, SLOT(slotChat()), this, "actionChat" );
	actionRemove = new KAction ( i18n("Delete contact"), "edittrash", 0, this, SLOT(slotRemoveThisUser()), this, "actionRemove" );
	actionGroupList = new KListAction ( i18n("Move contact"), "editcut", 0, this, SLOT(slotMoveThisUser()), this, "actionMove" );	
}

void MSNContact::rightButtonPressed(const QPoint &point)
{
	QStringList grouplist = *(kopeteapp->contactList()->groupStringList);
	actionGroupList->setItems(grouplist);

	popup = new KPopupMenu();
	popup->insertTitle(mUserID);
	actionChat->plug( popup );
	actionRemove->plug( popup );
	actionGroupList->plug( popup );
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

void MSNContact::slotIncomingChat(KMSNChatService *newboard, QString reqUserID)
{
	if ( reqUserID == mUserID )
	{
 		if (messageBoxInited == true && messageBox->isVisible() == true)
 		{
 			kdDebug() << "MSN Plugin: Incoming chat but Window opened for " << reqUserID <<"\n";
			messageBox->mBoard = newboard;
			connect(newboard,SIGNAL(msgReceived(QString,QString,QString)),messageBox,SLOT(slotMsgReceived(QString,QString,QString)));
					
			messageBox->raise();
 			return;
 		}
 		kdDebug() << "MSN Plugin: Incoming chat , no window, creating window for " << reqUserID <<"\n";
		messageBox = new MSNMessage(this, mUserID, mName, mStatus, newboard,mProtocol);
 		QObject::connect(this, SIGNAL(userStateChanged(QString)), messageBox, SLOT(slotUserStateChanged(QString)));
		messageBoxInited = true;
 		messageBox->show();
	}
}

void MSNContact::slotMessageBoxClosing()
{
	if (messageBoxInited == true && messageBox->isVisible() == true)
	{
		messageBoxInited = false;
		delete messageBox;
	}
}

void MSNContact::slotRemoveThisUser()
{
	mProtocol->engine->contactDelete(mUserID);
	delete this;
}

void MSNContact::slotMoveThisUser()
{
	QString newgroup;
	QString oldgroup;
	QStringList grouplist;
	int counter;

	grouplist = mProtocol->engine->getGroups();
	newgroup = actionGroupList->currentText();
	oldgroup = parentGroup->text(0);
	for ( QStringList::Iterator it = grouplist.begin(); it != grouplist.end(); ++it )
	{
		if (*it == newgroup );
		{
			counter++;
		}
    }
	if ( counter == 0 )
	{
		/* Group dont exist in server, we create it */
		mProtocol->engine->groupAdd( newgroup);
	}
	mProtocol->engine->contactMove( mUserID, oldgroup, newgroup);
}

void MSNContact::slotContactRemoved(QString handle, QString nick)
{
}

void MSNContact::slotUpdateContact (QString handle , uint status)
{
	if (handle == mUserID)
	{
		kdDebug() << "MSN Plugin: Contact " << handle <<" request update (" << status << ")\n";
		//mStatus = state;
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

