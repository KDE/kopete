/*
    bonjourcontact.cpp - Kopete Bonjour Protocol

	Copyright (c) 2007      by Tejas Dinkar		<tejas@gja.in>
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "bonjourcontact.h"

#include <QList>
#include <QHostAddress>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kopeteaccount.h"
#include "kopetechatsessionmanager.h"
#include "kopetemetacontact.h"

#include "bonjouraccount.h"
#include "bonjourfakeserver.h"
#include "bonjourprotocol.h"

BonjourContact::BonjourContact( Kopete::Account* _account, const QString &uniqueName,
		const QString &displayName, Kopete::MetaContact *parent )
: Kopete::Contact( _account, uniqueName, parent )
{
	kDebug( 14210 ) << " uniqueName: " << uniqueName << ", displayName: " << displayName;
	m_type = BonjourContact::Null;
	// FIXME: ? setDisplayName( displayName );
	m_msgManager = 0L;

	setOnlineStatus( BonjourProtocol::protocol()->bonjourOffline );

	socket = NULL;
	remotePort = 0;
}

BonjourContact::~BonjourContact()
{
	if (socket) {
		if (socket->isValid())
			socket->close();

		delete socket;
	}
	remotePort = 0;
}

void BonjourContact::setType( BonjourContact::Type type )
{
	m_type = type;
}

bool BonjourContact::isReachable()
{
    return true;
}

void BonjourContact::serialize( QMap< QString, QString > &serializedData, QMap< QString, QString > & /* addressBookData */ )
{
    QString value;
	switch ( m_type )
	{
	case Null:
		value = QLatin1String("null");
	case Echo:
		value = QLatin1String("echo");
	case Group:
		value = QLatin1String("group");
	}
	serializedData[ "contactType" ] = value;
}

Kopete::ChatSession* BonjourContact::manager( CanCreateFlags canCreateFlags )
{
	kDebug( 14210 ) ;
	if ( m_msgManager )
	{
		return m_msgManager;
	}
	else if ( canCreateFlags == CanCreate )
	{
		QList<Kopete::Contact*> contacts;
		contacts.append(this);
		Kopete::ChatSession::Form form = ( m_type == Group ?
				  Kopete::ChatSession::Chatroom : Kopete::ChatSession::Small );
		m_msgManager = Kopete::ChatSessionManager::self()->create(account()->myself(), contacts, protocol(), form );
		connect(m_msgManager, SIGNAL(messageSent(Kopete::Message&, Kopete::ChatSession*)),
				this, SLOT( sendMessage( Kopete::Message& ) ) );
		connect(m_msgManager, SIGNAL(destroyed()), this, SLOT(slotChatSessionDestroyed()));
		return m_msgManager;
	}
	else
	{
		return 0;
	}
}


QList<KAction *> *BonjourContact::customContextMenuActions() //OBSOLETE
{
	//FIXME!!!  this function is obsolete, we should use XMLGUI instead
	/*m_actionCollection = new KActionCollection( this, "userColl" );
	m_actionPrefs = new KAction(i18n( "&Contact Settings" ), 0, this,
			SLOT( showContactSettings( )), m_actionCollection, "contactSettings" );

	return m_actionCollection;*/
	return 0L;
}

void BonjourContact::showContactSettings()
{
	//BonjourContactSettings* p = new BonjourContactSettings( this );
	//p->show();
}

void BonjourContact::sendMessage( Kopete::Message &message )
{
	kDebug( 14210 ) ;
	// convert to the what the server wants
	// For this 'protocol', there's nothing to do
	// send it
	static_cast<BonjourAccount *>( account() )->server()->sendMessage(
			message.to().first()->contactId(),
			message.plainBody() );
	// give it back to the manager to display
	manager()->appendMessage( message );
	// tell the manager it was sent successfully
	manager()->messageSucceeded();
}

void BonjourContact::receivedMessage( const QString &message )
{
	Kopete::ContactPtrList contactList;
	contactList.append( account()->myself() );
	// Create a Kopete::Message
	Kopete::Message newMessage( this, contactList );
	newMessage.setPlainBody( message );
	newMessage.setDirection( Kopete::Message::Inbound );

	// Add it to the manager
	manager()->appendMessage (newMessage);
}

void BonjourContact::slotChatSessionDestroyed()
{
	//FIXME: the chat window was closed?  Take appropriate steps.
	m_msgManager = 0L;
}

void BonjourContact::setremoteHostName(const QString &nremoteHostName)
{
	remoteHostName = nremoteHostName;

	// FIXME: Resolve the following Avahi Dependency BonjourAccount::resolveHostName
	remoteAddress = QHostAddress(BonjourAccount::resolveHostName(remoteHostName));
}

const QString BonjourContact::getremoteHostName() const
{
	return remoteHostName;
}

const QHostAddress BonjourContact::getremoteAddress() const
{
	return remoteAddress;
}

void BonjourContact::setremotePort(const short int &nremotePort)
{
	remotePort = nremotePort;
}

const short int BonjourContact::getremotePort() const
{
	return remotePort;
}

const bool BonjourContact::isRemoteAddress(const QHostAddress &host) const
{
	if (remoteAddress == host)
		return true;
	else
		return false;
}

#include "bonjourcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

