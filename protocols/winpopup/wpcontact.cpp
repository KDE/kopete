/***************************************************************************
                          wpcontact.cpp  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@kde.org
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

WPContact::WPContact(KopeteAccount *account, const QString &newHostName, const QString &displayName, KopeteMetaContact *metaContact)
	: KopeteContact(account, newHostName, metaContact)
{
	DEBUG(WPDMETHOD, "WPContact::WPContact(<account>, " << newHostName << ", " << displayName << ", <parent>)");

	QString newDisplayName;
	for(unsigned i = 0; i < newHostName.length(); i++)
		if(!i)
			newDisplayName += newHostName[i].upper();
		else
			newDisplayName += newHostName[i].lower();

	setDisplayName(displayName == QString::null || displayName == "" ? newDisplayName : displayName);
	myWasConnected = false;

	// Initialise and start the periodical checking for contact's status
	setOnlineStatus(static_cast<WPProtocol *>(protocol())->WPOffline);
	slotCheckStatus();
	connect(&checkStatus, SIGNAL(timeout()), this, SLOT(slotCheckStatus()));
	checkStatus.start(1000, false);

	m_manager = 0L;

	// Set up the context menu
	myActionCollection = new KActionCollection(this);
}

void WPContact::serialize(QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData)
{
	kdDebug(14180) << "WP::serialize(...)" << endl;
	
	KopeteContact::serialize(serializedData, addressBookData);
}

KopeteMessageManager* WPContact::manager( bool )
{
	if( !m_manager )
	{
		// Set up the message managers
		QPtrList<KopeteContact> singleContact;
		singleContact.append(this);

		m_manager = KopeteMessageManagerFactory::factory()->create( protocol()->myself(), singleContact, protocol() );

		connect(m_manager, SIGNAL(messageSent(KopeteMessage &, KopeteMessageManager *)), this, SLOT(slotSendMessage(KopeteMessage &)));
  		connect(m_manager, SIGNAL(messageSent(KopeteMessage &, KopeteMessageManager *)), m_manager, SLOT(appendMessage(KopeteMessage &)));
		connect(m_manager, SIGNAL(destroyed()), this, SLOT(slotMessageManagerDestroyed()));
		connect(this, SIGNAL(messageSuccess()), m_manager, SLOT(messageSuccess()));
	}

	return m_manager;
}
/*
bool WPContact::isOnline() const
{
	kdDebug(14180) << "[WPContact::isOnline()]" << endl;
	return onlineStatus().status() != KopeteOnlineStatus::Offline && onlineStatus().status() != KopeteOnlineStatus::Unknown;
}
*/
bool WPContact::isReachable()
{
	kdDebug(14180) << "[WPContact::isReachable()]" << endl;
	return onlineStatus().status() != KopeteOnlineStatus::Offline && onlineStatus().status() != KopeteOnlineStatus::Unknown;
}

void WPContact::slotMessageManagerDestroyed()
{
	m_manager = 0L;
}

void WPContact::slotUserInfo()
{
	// TODO: show user info?
}

void slotDeleteContact()
{
//	deleteLater();
}

void WPContact::slotCheckStatus()
{
//	DEBUG(WPDMETHOD, "WPContact::slotCheckStatus()");

	bool oldWasConnected = myWasConnected;
	bool newIsOnline = false;

	myWasConnected = protocol() != 0;
	if(account()) newIsOnline = static_cast<WPAccount *>(account())->checkHost(theHostName);
	
	if(newIsOnline != isOnline() || myWasConnected != oldWasConnected)
		setOnlineStatus(myWasConnected ? newIsOnline ? WPProtocol::protocol()->WPOnline : WPProtocol::protocol()->WPOffline : WPProtocol::protocol()->WPOffline );
}

void WPContact::slotNewMessage(const QString &Body, const QDateTime &Arrival)
{
	DEBUG(WPDMETHOD, "WPContact::slotNewMessage(" << Body << ", " << Arrival.toString() << ")");

	QPtrList<KopeteContact> contactList;
	contactList.append(protocol()->myself());

	QRegExp subj("^Subject: ([^\n]*)\n(.*)$");
	KopeteMessage msg;

	if(subj.search(Body) == -1)
		msg = KopeteMessage(this, contactList, Body, KopeteMessage::Inbound);
	else
		msg = KopeteMessage(this, contactList, subj.cap(2), subj.cap(1), KopeteMessage::Inbound);

	manager()->appendMessage(msg);
}

void WPContact::slotSendMessage( KopeteMessage& message )
{
	DEBUG(WPDMETHOD, "WPContact::slotSendMessage(<message>)");
	
	QString Message = (!message.subject().isEmpty() ? "Subject: " + message.subject() + "\n" : QString("")) + message.plainBody();
	static_cast<WPAccount *>(account())->slotSendMessage( Message, dynamic_cast<WPContact *>( message.to().first() )->hostName() );
	
	emit messageSuccess();
}

#include "wpcontact.moc"

