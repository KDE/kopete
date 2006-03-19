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
#include <qregexp.h>

// KDE Includes
#include <kdebug.h>

// Kopete Includes

// Local Includes
#include "wpcontact.h"
#include "wpaccount.h"

WPContact::WPContact(Kopete::Account *account, const QString &newHostName, const QString &nickName, Kopete::MetaContact *metaContact)
	: Kopete::Contact(account, newHostName, metaContact)
{
//	kdDebug(14170) << "WPContact::WPContact(<account>, " << newHostName << ", " << nickName << ", <parent>)" << endl;
	kdDebug(14170) << "WPContact::WPContact: " << this << endl;

	QString theNickName = nickName;

	if (theNickName.isEmpty()) {
		// Construct nickname from hostname with first letter to upper. GF
		theNickName = newHostName.lower();
		theNickName = theNickName.replace(0, 1, theNickName[0].upper());
	}

	setNickName(theNickName);
	myWasConnected = false;


	m_manager = 0;
	m_infoDialog = 0;

	// Initialise and start the periodical checking for contact's status
	setOnlineStatus(static_cast<WPProtocol *>(protocol())->WPOffline);

	connect(&checkStatus, SIGNAL(timeout()), this, SLOT(slotCheckStatus()));
	checkStatus.start(1000, false);
}

QPtrList<KAction> * WPContact::customContextMenuActions()
{
	//myActionCollection = new KActionCollection(parent);
	return 0;
}

void WPContact::serialize(QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData)
{
//	kdDebug(14170) << "WP::serialize(...)" << endl;

	Kopete::Contact::serialize(serializedData, addressBookData);
}

Kopete::ChatSession* WPContact::manager( Kopete::Contact::CanCreateFlags /*canCreate*/ )	// TODO: use the parameter as canCreate
{
	if (m_manager == 0) {
		// Set up the message managers
		QPtrList<Kopete::Contact> singleContact;
		singleContact.append(this);

		m_manager = Kopete::ChatSessionManager::self()->create( account()->myself(), singleContact, protocol() );

		connect(m_manager, SIGNAL(messageSent(Kopete::Message &, Kopete::ChatSession *)), this, SLOT(slotSendMessage(Kopete::Message &)));
		connect(m_manager, SIGNAL(messageSent(Kopete::Message &, Kopete::ChatSession *)), m_manager, SLOT(appendMessage(Kopete::Message &)));
		connect(m_manager, SIGNAL(destroyed()), this, SLOT(slotChatSessionDestroyed()));
	}

	return m_manager;
}

/*
bool WPContact::isOnline() const
{
	kdDebug(14170) << "[WPContact::isOnline()]" << endl;
	return onlineStatus().status() != Kopete::OnlineStatus::Offline && onlineStatus().status() != Kopete::OnlineStatus::Unknown;
}
*/

bool WPContact::isReachable()
{
//	kdDebug(14170) << "[WPContact::isReachable()]" << endl;
	return onlineStatus().status() != Kopete::OnlineStatus::Offline && onlineStatus().status() != Kopete::OnlineStatus::Unknown;
}

void WPContact::slotChatSessionDestroyed()
{
	m_manager = 0;
}

void WPContact::slotUserInfo()
{
	kdDebug( 14170 ) << k_funcinfo << endl;

	if (!m_infoDialog) {
		m_infoDialog = new WPUserInfo( this, static_cast<WPAccount*>( account() ) );
		if (!m_infoDialog) return;
		connect( m_infoDialog, SIGNAL( closing() ), this, SLOT( slotCloseUserInfoDialog() ) );
		m_infoDialog->show();
	} else {
		m_infoDialog->raise();
	}
}

void WPContact::slotCloseUserInfoDialog()
{
	m_infoDialog->delayedDestruct();
	m_infoDialog = 0;
}

/*
void deleteContact()
{
//	deleteLater();
}
*/

void WPContact::slotCheckStatus()
{
	bool oldWasConnected = myWasConnected;
	bool newIsOnline = false;

	myWasConnected = protocol() != 0 && account() != 0;
	WPAccount *acct = dynamic_cast<WPAccount *>(account());
	if (acct) newIsOnline = acct->checkHost(contactId());

	if(newIsOnline != isOnline() || myWasConnected != oldWasConnected) {
		Kopete::OnlineStatus tmpStatus = WPProtocol::protocol()->WPOffline;
		if (myWasConnected && newIsOnline) {
				tmpStatus = WPProtocol::protocol()->WPOnline;
		}
		setOnlineStatus(tmpStatus);
	}
}

void WPContact::slotNewMessage(const QString &Body, const QDateTime &Arrival)
{
	kdDebug(14170) << "WPContact::slotNewMessage(" << Body << ", " << Arrival.toString() << ")" << endl;

	QPtrList<Kopete::Contact> contactList;
	contactList.append(account()->myself());

	QRegExp subj("^Subject: ([^\n]*)\n(.*)$");
	Kopete::Message msg;

	if(subj.search(Body) == -1) {
		msg = Kopete::Message(this, contactList, Body, Kopete::Message::Inbound);
	} else {
		msg = Kopete::Message(this, contactList, subj.cap(2), subj.cap(1), Kopete::Message::Inbound);
	}

	manager(Kopete::Contact::CannotCreate)->appendMessage(msg);
}

void WPContact::slotSendMessage( Kopete::Message& message )
{
//	kdDebug(14170) << "WPContact::slotSendMessage(<message>)" << endl;
	// Warning: this could crash
	kdDebug(14170) << message.to().first() << " is " << dynamic_cast<WPContact *>( message.to().first() )->contactId() << endl;

	QString Message = (!message.subject().isEmpty() ? "Subject: " + message.subject() + "\n" : QString("")) + message.plainBody();
	WPAccount *acct = dynamic_cast<WPAccount *>(account());
	WPContact *contact = dynamic_cast<WPContact *>( message.to().first() );
	if (acct && contact) {
		acct->slotSendMessage( Message, contact->contactId() );
		m_manager->messageSucceeded();
	}
}

#include "wpcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
