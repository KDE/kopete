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

	connection = NULL;
	remotePort = 0;
}

BonjourContact::~BonjourContact()
{
	if (connection) {
		delete connection;
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

	// This is Blocking, we may lose upto 5 seconds here
	if (! connection) {
		QString localName = account()->property("fullName").toString();
		setConnection(new BonjourContactConnection(remoteAddress, remotePort, localName, fullName));
	}

	// Blocking Again. Upto another 3 seconds
	connection->waitReady(3000);
	connection->sendMessage(message);

	// give it back to the manager to display
	manager()->appendMessage( message );
	// tell the manager it was sent successfully
	manager()->messageSucceeded();
}

void BonjourContact::receivedMessage(Kopete::Message message)
{
	Kopete::ChatSession *session = manager(CanCreate);
	session->appendMessage(message);
}

void BonjourContact::slotChatSessionDestroyed()
{
	//FIXME: the chat window was closed?  Take appropriate steps.

	if (connection) {
		connection->sayGoodBye();
		delete connection;
		connection = NULL;
	}

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

void BonjourContact::setfullName(const QString &n_fullName)
{
	fullName = n_fullName;
}

const QString BonjourContact::getfullName() const
{
	return fullName;
}

void BonjourContact::settextdata(const QMap <QString, QByteArray> &n_textdata)
{
	textdata = n_textdata;
}

const QMap <QString, QByteArray> BonjourContact::gettextdata() const
{
	return textdata;
}

const bool BonjourContact::isRemoteAddress(const QHostAddress &host) const
{
	if (remoteAddress == host)
		return true;
	else
		return false;
}

void BonjourContact::setConnection(BonjourContactConnection *c)
{
	if (connection)
		delete connection;

	connection = c;

	// We set the parent of the socket to us, so that we ensure that only we
	// can delete the connection (and the socket hence)
	connection->setParent(this);

	connect(connection, SIGNAL(disconnected(BonjourContactConnection *)),
			this, SLOT(connectionDisconnected(BonjourContactConnection *)));

	connect(connection, SIGNAL(messageReceived(Kopete::Message)),
			this, SLOT(receivedMessage(Kopete::Message)));
}

void BonjourContact::connectionDisconnected(BonjourContactConnection *c)
{
	if (c == connection) {
		delete connection;
		connection = NULL;
	}
}

#include "bonjourcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

