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

#include <kopeteaccount.h>
#include <kopetechatsessionmanager.h>
#include <kopetemetacontact.h>

#include "bonjouraccount.h"
#include "bonjourprotocol.h"

// FIXME: Here, we hardcode the icon (bonjour_protocol) into the constructor.
// This shouldn't be necessary
BonjourContact::BonjourContact( Kopete::Account* _account, const QString &uniqueName,
		Kopete::MetaContact *parent )
: Kopete::Contact( _account, uniqueName, parent, QString("bonjour_protocol") ), connection(NULL), 
	remoteHostName(), remoteAddress(), remotePort(0), m_msgManager(NULL)
{
	kDebug()<< " uniqueName: " << uniqueName;

	setOnlineStatus( BonjourProtocol::protocol()->bonjourOffline );
}

BonjourContact::~BonjourContact()
{
	kDebug()<<"Deleting Contact!";
	delete connection;
	remotePort = 0;
}

bool BonjourContact::isReachable()
{
    return true;
}

void BonjourContact::serialize( QMap< QString, QString > & /*serializedData*/, QMap< QString, QString > & /* addressBookData */ )
{
	// Really Do Nothing
}

Kopete::ChatSession* BonjourContact::manager( CanCreateFlags canCreateFlags )
{
	kDebug();
	if ( m_msgManager )
	{
		return m_msgManager;
	}
	else if ( canCreateFlags == CanCreate )
	{
		QList<Kopete::Contact*> contacts;
		contacts.append(this);
		Kopete::ChatSession::Form form = ( Kopete::ChatSession::Small );
		m_msgManager = Kopete::ChatSessionManager::self()->create(account()->myself(), contacts, protocol(), form );
		connect(m_msgManager, SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)),
				this, SLOT(sendMessage(Kopete::Message&)) );
		connect(m_msgManager, SIGNAL(destroyed()), this, SLOT(slotChatSessionDestroyed()));
		return m_msgManager;
	}
	else
	{
		return 0;
	}
}

void BonjourContact::showContactSettings()
{
	//BonjourContactSettings* p = new BonjourContactSettings( this );
	//p->show();
}

void BonjourContact::sendMessage( Kopete::Message &message )
{
	kDebug();

	// This is Blocking, we may lose upto 5 seconds here
	if (! connection) {
		QString localName = account()->property("username").toString();
		setConnection(new BonjourContactConnection(remoteAddress, remotePort, localName, username));
	}

	// Blocking Again. Upto another 3 seconds
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
}

const QString BonjourContact::getremoteHostName() const
{
	return remoteHostName;
}

void BonjourContact::setremoteAddress(const QHostAddress &nremoteAddress)
{
	remoteAddress = nremoteAddress;
}

const QHostAddress BonjourContact::getremoteAddress() const
{
	return remoteAddress;
}

void BonjourContact::setremotePort(const short int &nremotePort)
{
	remotePort = nremotePort;
}

short int BonjourContact::getremotePort() const
{
	return remotePort;
}

void BonjourContact::setusername(const QString &n_username)
{
	username = n_username;
}

const QString BonjourContact::getusername() const
{
	return username;
}

void BonjourContact::settextdata(const QMap <QString, QByteArray> &n_textdata)
{
	textdata = n_textdata;
}

const QMap <QString, QByteArray> BonjourContact::gettextdata() const
{
	return textdata;
}

bool BonjourContact::isRemoteAddress(const QHostAddress &host) const
{
	if (remoteAddress == host)
		return true;
	else
		return false;
}

void BonjourContact::setConnection(BonjourContactConnection *c)
{
	delete connection;

	connection = c;

	// We set the parent of the socket to us, so that we ensure that only we
	// can delete the connection (and the socket hence)
	connection->setParent(this);

	connect(connection, SIGNAL(disconnected(BonjourContactConnection*)),
			this, SLOT(connectionDisconnected(BonjourContactConnection*)));

	connect(connection, SIGNAL(messageReceived(Kopete::Message)),
			this, SLOT(receivedMessage(Kopete::Message)));
}

void BonjourContact::connectionDisconnected(BonjourContactConnection *c)
{
	if (c == connection) {
		connection->deleteLater();
		connection = NULL;
	}
}

#include "bonjourcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

