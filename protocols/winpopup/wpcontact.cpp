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
#include "wpdebug.h"

WPContact::WPContact(Kopete::Account *account, const QString &newHostName, const QString &displayName, Kopete::MetaContact *metaContact)
	: Kopete::Contact(account, newHostName, metaContact)
{
	DEBUG(WPDMETHOD, "WPContact::WPContact(<account>, " << newHostName << ", " << displayName << ", <parent>)");
	DEBUG(WPDINFO, "I am " << this << "!");

	QString newDisplayName;
	for(unsigned i = 0; i < newHostName.length(); i++)
		if(!i)
			newDisplayName += newHostName[i].upper();
		else
			newDisplayName += newHostName[i].lower();

	setDisplayName(displayName.isEmpty() ? newDisplayName : displayName);
	myWasConnected = false;

	// Initialise and start the periodical checking for contact's status
	setOnlineStatus(static_cast<WPProtocol *>(protocol())->WPOffline);
//	slotCheckStatus();
	connect(&checkStatus, SIGNAL(timeout()), this, SLOT(slotCheckStatus()));
	checkStatus.start(1000, false);

	m_manager = 0L;
	m_infoDialog = 0L;
}

QPtrList<KAction> * WPContact::customContextMenuActions()
{
	//myActionCollection = new KActionCollection(parent);
	return 0L;
}

void WPContact::serialize(QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData)
{
	kdDebug(14180) << "WP::serialize(...)" << endl;

	Kopete::Contact::serialize(serializedData, addressBookData);
}

Kopete::MessageManager* WPContact::manager( bool )	// TODO: use the parameter as canCreate
{
	if( !m_manager )
	{
		// Set up the message managers
		QPtrList<Kopete::Contact> singleContact;
		singleContact.append(this);

		m_manager = Kopete::MessageManagerFactory::self()->create( account()->myself(), singleContact, protocol() );

		connect(m_manager, SIGNAL(messageSent(Kopete::Message &, Kopete::MessageManager *)), this, SLOT(slotSendMessage(Kopete::Message &)));
  		connect(m_manager, SIGNAL(messageSent(Kopete::Message &, Kopete::MessageManager *)), m_manager, SLOT(appendMessage(Kopete::Message &)));
		connect(m_manager, SIGNAL(destroyed()), this, SLOT(slotMessageManagerDestroyed()));
	}

	return m_manager;
}
/*
bool WPContact::isOnline() const
{
	kdDebug(14180) << "[WPContact::isOnline()]" << endl;
	return onlineStatus().status() != Kopete::OnlineStatus::Offline && onlineStatus().status() != Kopete::OnlineStatus::Unknown;
}
*/
bool WPContact::isReachable()
{
	kdDebug(14180) << "[WPContact::isReachable()]" << endl;
	return onlineStatus().status() != Kopete::OnlineStatus::Offline && onlineStatus().status() != Kopete::OnlineStatus::Unknown;
}

void WPContact::slotMessageManagerDestroyed()
{
	m_manager = 0L;
}

void WPContact::slotUserInfo()
{
	// TODO: show user info?
	kdDebug( 14180 ) << k_funcinfo << endl;


	if ( !m_infoDialog )
	{
		m_infoDialog = new WPUserInfo( this, static_cast<WPAccount*>( account() ) );
		if( !m_infoDialog )
			return;
		connect( m_infoDialog, SIGNAL( closing() ), this, SLOT( slotCloseUserInfoDialog() ) );
		m_infoDialog->show();
	}
	else
	{
		m_infoDialog->raise();
	}
}

void WPContact::slotCloseUserInfoDialog()
{
	m_infoDialog->delayedDestruct();
	m_infoDialog = 0L;
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

	myWasConnected = protocol() != 0 && account() != 0;
	WPAccount *acct = dynamic_cast<WPAccount *>(account());
	if (acct) {
		newIsOnline = acct->checkHost(contactId());
	}

	if(newIsOnline != isOnline() || myWasConnected != oldWasConnected)
		setOnlineStatus(myWasConnected ? newIsOnline ? WPProtocol::protocol()->WPOnline : WPProtocol::protocol()->WPOffline : WPProtocol::protocol()->WPOffline );
}

void WPContact::slotNewMessage(const QString &Body, const QDateTime &Arrival)
{
	DEBUG(WPDMETHOD, "WPContact::slotNewMessage(" << Body << ", " << Arrival.toString() << ")");

	QPtrList<Kopete::Contact> contactList;
	contactList.append(account()->myself());

	QRegExp subj("^Subject: ([^\n]*)\n(.*)$");
	Kopete::Message msg;

	if(subj.search(Body) == -1)
		msg = Kopete::Message(this, contactList, Body, Kopete::Message::Inbound);
	else
		msg = Kopete::Message(this, contactList, subj.cap(2), subj.cap(1), Kopete::Message::Inbound);

	manager()->appendMessage(msg);
}

void WPContact::slotSendMessage( Kopete::Message& message )
{
	DEBUG(WPDMETHOD, "WPContact::slotSendMessage(<message>)");
	// Warning: this could crash
	DEBUG(WPDINFO, message.to().first() << " is " << dynamic_cast<WPContact *>( message.to().first() )->contactId() );

	QString Message = (!message.subject().isEmpty() ? "Subject: " + message.subject() + "\n" : QString("")) + message.plainBody();
	WPAccount *acct = dynamic_cast<WPAccount *>(account());
	WPContact *contact = dynamic_cast<WPContact *>( message.to().first() );
	if (acct && contact) {
		acct->slotSendMessage( Message, contact->contactId() );
		m_manager->messageSucceeded();
	}
}

#include "wpcontact.moc"

