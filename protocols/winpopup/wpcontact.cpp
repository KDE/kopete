/***************************************************************************
                          wpcontact.cpp  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@indigoarchive.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// Qt Includes
#include <qdatetime.h>
#include <qfont.h>
#include <qregexp.h>

// KDE Includes
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>

// Kopete Includes
#include "kopetestdaction.h"
#include "kopetemessage.h"
#include "kopetemessagemanager.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"

// Local Includes
#include "wpcontact.h"
#include "wpprotocol.h"
#include "wpdebug.h"

WPContact::WPContact(const QString &host, WPProtocol *protocol, KopeteMetaContact *parent) : KopeteContact(protocol, parent)
{
	DEBUG(WPDMETHOD, "WPContact::WPContact(" << host << ", <protocol>, <parent>)");

	QString newDisplayName;
	for(unsigned i = 0; i < host.length(); i++)
		if(!i)
			newDisplayName += host[i].upper();
		else
			newDisplayName += host[i].lower();

	setDisplayName(newDisplayName);
	myProtocol = protocol;
	myHost = host;
	myHistoryDialog = 0;

	// Initialise and start the periodical checking for contact's status
	myIsOnline = true;
	emit statusChanged(this, this->status());
	slotCheckStatus();
	connect(&checkStatus, SIGNAL(timeout()), this, SLOT(slotCheckStatus()));
	checkStatus.start(1000, false);

	// Set up the message managers
	QPtrList<KopeteContact> singleContact;
	singleContact.append(this);
	myEmailManager = KopeteMessageManagerFactory::factory()->create(myProtocol->myself(), singleContact, myProtocol, KopeteMessageManager::Email);
	connect(myEmailManager, SIGNAL(messageSent(const KopeteMessage &, KopeteMessageManager *)), this, SLOT(slotSendMessage(const KopeteMessage &)));
	connect(myEmailManager, SIGNAL(messageSent(const KopeteMessage &, KopeteMessageManager *)), myEmailManager, SLOT(appendMessage(const KopeteMessage &)));
	myChatManager = KopeteMessageManagerFactory::factory()->create(myProtocol->myself(), singleContact, myProtocol, KopeteMessageManager::ChatWindow);
	connect(myChatManager, SIGNAL(messageSent(const KopeteMessage &, KopeteMessageManager *)), this, SLOT(slotSendMessage(const KopeteMessage &)));
	connect(myChatManager, SIGNAL(messageSent(const KopeteMessage &, KopeteMessageManager *)), myChatManager, SLOT(appendMessage(const KopeteMessage &)));

	connect (this , SIGNAL( moved(KopeteMetaContact*,KopeteContact*) ), this, SLOT (slotMovedToMetaContact() ));
	connect (metaContact() , SIGNAL( aboutToSave(KopeteMetaContact*) ), protocol, SLOT (serialize(KopeteMetaContact*) ));


	// Set up the context menu
	myActionCollection = new KActionCollection(this);
}

void WPContact::slotCheckStatus()
{
//	DEBUG(WPDMETHOD, "WPContact::slotCheckStatus()");

	bool oldIsOnline = myIsOnline;
	if(myProtocol)
		myIsOnline = myProtocol->checkHost(myHost);
	else
		myIsOnline = false;
	if(oldIsOnline != myIsOnline)
		emit statusChanged(this, status());
}

void WPContact::execute()
{
	DEBUG(WPDMETHOD, "WPContact::execute()");

	KGlobal::config()->setGroup("WinPopup");
	if(KGlobal::config()->readBoolEntry("EmailDefault", true))
	{
		myEmailManager->readMessages();
		myEmailManager->slotSendEnabled(true);
	}
	else
		myChatManager->readMessages();
}

void WPContact::slotNewMessage(const QString &Body, const QDateTime &Arrival)
{
	DEBUG(WPDMETHOD, "WPContact::slotNewMessage(" << Body << ", " << Arrival.toString() << ")");

	QPtrList<KopeteContact> contactList;
	contactList.append(myProtocol->myself());

	QRegExp subj("^Subject: ([^\n]*)\n(.*)$");
	if(subj.search(Body) == -1)
		myChatManager->appendMessage(KopeteMessage(this, contactList, Body, KopeteMessage::Inbound));
	else
	{
		myEmailManager->appendMessage(KopeteMessage(this, contactList, subj.cap(2), subj.cap(1), KopeteMessage::Inbound));
		myEmailManager->slotSendEnabled(false);
	}
}

void WPContact::slotSendMessage(const KopeteMessage& message)
{
	DEBUG(WPDMETHOD, "WPContact::slotSendMessage(<message>)");
	
	QString Message = (message.subject() != "" ? "Subject: " + message.subject() + "\n" : QString("")) + message.plainBody();
	myProtocol->slotSendMessage(Message, dynamic_cast<WPContact *>(message.to().first())->host());
}

void WPContact::slotViewHistory()
{
	if(!myHistoryDialog)
	{
		myHistoryDialog = new KopeteHistoryDialog(QString("wp_logs/%1.log").arg(myHost), displayName(), true, 50, 0, "WPHistoryDialog");
		connect(myHistoryDialog, SIGNAL(closing()), this, SLOT(slotCloseHistoryDialog()));
	}
}

void WPContact::slotCloseHistoryDialog()
{
	delete myHistoryDialog;
	myHistoryDialog = 0;
}

void WPContact::slotMovedToMetaContact()
{
	connect (metaContact() , SIGNAL( aboutToSave(KopeteMetaContact*) ), protocol(), SLOT (serialize(KopeteMetaContact*) ));
}


#include "wpcontact.moc"

