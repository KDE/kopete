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
#include <kmessagebox.h>
#include <kdebug.h>
#include "msnuser.h"
#include <switchboard.h>

MSNContact::MSNContact(QListViewItem *parent, QString userid, const QString name, MSNProtocol *protocol)
	: IMContact(parent)
{
	mProtocol = protocol;
	mName = name;
	mUserID = userid;
	messageTimer = new QTimer();
	messageQueue = new QValueStack<MSNMessage>;
	isMessageIcon = false;
	// We connect this signal so that we can tell when a user's status changes
	QObject::connect(protocol->engine, SIGNAL(userStateChange(QString, QString, QString)), this, SLOT(slotUserStateChange (QString, QString, QString) ));
	QObject::connect(protocol->engine->switchboard, SIGNAL(messageReceived(QString ,QString) ), this, SLOT(slotNewMessage(QString, QString)));
	QObject::connect(messageTimer, SIGNAL(timeout()), this, SLOT(slotFlashIcon()));

	QString tmp = name;
	//tmp.append(" (");
	//tmp.append(QString::number(uin));
	//tmp.append(")");
	setText(0,tmp);

	//slotUserStateChanged(uin, (protocol->kxContacts->getContact(uin)).status, 0);
}

void MSNContact::rightButtonPressed(const QPoint &point)
{

}

void MSNContact::leftButtonDoubleClicked()
{
	if (messageQueue->isEmpty() == false)
	{
		MSNMessage tmpMessage = messageQueue->pop();
		if (messageQueue->isEmpty() == true)
		{
			messageTimer->stop();
			if (isMessageIcon == true)
			{
				slotFlashIcon();
			}
		}
		QString tmp = "<qt>Message from ";
		tmp.append(mName);
		tmp.append(":\n");
		tmp.append(tmpMessage.message);
		tmp.append("</qt>");
		KMessageBox::information(kopeteapp->mainWindow(), tmp, QString(QString("Message from ").append(mName)));
	} else {
		KMessageBox::information(kopeteapp->mainWindow(), "Send a message to this user here.", "Send a message");
	}

}

void MSNContact::slotUserStateChange (QString state, QString handle, QString publicname)
{
	if (handle == mUserID)
	{
		mStatus = state;
		isMessageIcon = false;
		if (mProtocol->engine->getUserStatus(handle) == "NLN")
		{
			kopeteapp->contactList()->onlineBranch->insertItem(this);
			setPixmap(0, mProtocol->onlineIcon);
		}
		if ( mProtocol->engine->getUserStatus(handle) == "FLN")
		{
			kopeteapp->contactList()->offlineBranch->insertItem(this);
			setPixmap(0, mProtocol->offlineIcon);
		}
		/*
		if (status == STATUS_AWAY)
		{
			kopeteapp->contactList()->onlineBranch->insertItem(this);
			setPixmap(0, mProtocol->awayIcon);
		}
		if (status == STATUS_DND || status == STATUS_DND_99)
		{
			kopeteapp->contactList()->onlineBranch->insertItem(this);
			setPixmap(0, mProtocol->dndIcon);
		}
		if (status == STATUS_NA_99 || status == STATUS_NA)
		{
			kopeteapp->contactList()->onlineBranch->insertItem(this);
			setPixmap(0, mProtocol->naIcon);
		}
		if (status == STATUS_OCCUPIED || status == STATUS_OCCUPIED_MAC)
		{
			kopeteapp->contactList()->onlineBranch->insertItem(this);
			setPixmap(0, mProtocol->occupiedIcon);
		}
	*/
	}
}


void MSNContact::slotNewMessage(QString userid, QString message)
{
	/*
	if (uin == mUIN)
	{
		messageQueue->prepend(message);
		messageTimer->start(1000, false);
	}
	*/
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

