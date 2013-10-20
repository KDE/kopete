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

#include "wpcontact.h"

// Qt Includes
#include <qregexp.h>
#include <QList>

// KDE Includes
#include <kdebug.h>

// Kopete Includes

// Local Includes
#include "wpaccount.h"

WPContact::WPContact(Kopete::Account *account, const QString &newHostName, const QString &nickName, Kopete::MetaContact *metaContact)
	: Kopete::Contact(account, newHostName, metaContact)
{
//	kDebug(14170) << "WPContact::WPContact(<account>, " << newHostName << ", " << nickName << ", <parent>)";
	kDebug(14170) << "WPContact::WPContact: " << this;

	QString theNickName = nickName;

	if (theNickName.isEmpty()) {
		// Construct nickname from hostname with first letter to upper. GF
		theNickName = newHostName.toLower();
		theNickName = theNickName.replace(0, 1, theNickName[0].toUpper());
	}

	setNickName(theNickName);
	myWasConnected = false;


	m_manager = 0;
	m_infoDialog = 0;

	// Initialise and start the periodical checking for contact's status
	setOnlineStatus(static_cast<WPProtocol *>(protocol())->WPOffline);

	connect(&checkStatus, SIGNAL(timeout()), this, SLOT(slotCheckStatus()));
	checkStatus.setSingleShot(false);
	checkStatus.start(1000);
}

QList<KAction*> *WPContact::customContextMenuActions()
{
	//myActionCollection = new KActionCollection(parent);
	return 0;
}

void WPContact::serialize(QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData)
{
//	kDebug(14170) << "WP::serialize(...)";

	Kopete::Contact::serialize(serializedData, addressBookData);
}

Kopete::ChatSession* WPContact::manager( Kopete::Contact::CanCreateFlags /*canCreate*/ )	// TODO: use the parameter as canCreate
{
	if (m_manager == 0) {
		// Set up the message managers
		QList<Kopete::Contact*> singleContact;
		singleContact.append(this);

		m_manager = Kopete::ChatSessionManager::self()->create( account()->myself(), singleContact, protocol() );

		connect(m_manager, SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)), this, SLOT(slotSendMessage(Kopete::Message&)));
		connect(m_manager, SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)), m_manager, SLOT(appendMessage(Kopete::Message&)));
		connect(m_manager, SIGNAL(destroyed()), this, SLOT(slotChatSessionDestroyed()));
	}

	return m_manager;
}

/*
bool WPContact::isOnline() const
{
	kDebug(14170) << "[WPContact::isOnline()]";
	return onlineStatus().status() != Kopete::OnlineStatus::Offline && onlineStatus().status() != Kopete::OnlineStatus::Unknown;
}
*/

bool WPContact::isReachable()
{
//	kDebug(14170) << "[WPContact::isReachable()]";
	return onlineStatus().status() != Kopete::OnlineStatus::Offline && onlineStatus().status() != Kopete::OnlineStatus::Unknown;
}

void WPContact::slotChatSessionDestroyed()
{
	m_manager = 0;
}

void WPContact::slotUserInfo()
{
	kDebug( 14170 ) ;

	if (!m_infoDialog) {
		m_infoDialog = new WPUserInfo( this );
		if (!m_infoDialog) return;
		connect( m_infoDialog, SIGNAL(closing()), this, SLOT(slotCloseUserInfoDialog()) );
		m_infoDialog->show();
	} else {
		m_infoDialog->raise();
	}
}

void WPContact::slotCloseUserInfoDialog()
{
	m_infoDialog->deleteLater();
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
	kDebug(14170) << "WPContact::slotNewMessage(" << Body << ", " << Arrival.toString() << ')';

	QList<Kopete::Contact*> contactList;
	contactList.append(account()->myself());

	QRegExp subj("^Subject: ([^\n]*)\n(.*)$");
	Kopete::Message msg(this, contactList);
	msg.setDirection( Kopete::Message::Inbound );
	msg.setTimestamp(Arrival);

	if(subj.indexIn(Body) == -1) {
		msg.setPlainBody( Body );

	} else {
		msg.setPlainBody( subj.cap(2) );
		msg.setSubject( subj.cap(1) );
	}

	manager(Kopete::Contact::CannotCreate)->appendMessage(msg);
}

void WPContact::slotSendMessage( Kopete::Message& message )
{
//	kDebug(14170) << "WPContact::slotSendMessage(<message>)";
	// Warning: this could crash
	kDebug(14170) << message.to().first() << " is " << dynamic_cast<WPContact *>( message.to().first() )->contactId();

	QString Message = QString((!message.subject().isEmpty() ? "Subject: " + message.subject() + '\n' : QString()) + message.plainBody()).trimmed();
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
