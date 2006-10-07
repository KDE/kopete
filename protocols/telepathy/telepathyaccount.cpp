/*
 * telepathyaccount.cpp - Telepathy Kopete Account.
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 *
 * Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#include "telepathyaccount.h"

// Qt includes
#include <QtCore/QMap>

// KDE includes
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>
#include <kconfig.h>
#include <kglobal.h>

// Kopete includes
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "kopetecontactlist.h"

// QtTapioca includes
#include <QtTapioca/ConnectionManagerFactory>

// Local includes
#include "telepathyprotocol.h"
#include "telepathycontact.h"

using namespace QtTapioca;

class TelepathyAccount::Private
{
public:
	Private()
	 : currentConnectionManager(0), currentConnection(0)
	{}

	ConnectionManager *getConnectionManager();

	QString connectionManager;
	QString connectionProtocol;
	QList<ConnectionManager::Parameter> connectionParameters;
	ConnectionManager *currentConnectionManager;
	Connection *currentConnection;
};

TelepathyAccount::TelepathyAccount(TelepathyProtocol *protocol, const QString &accountId)
 : Kopete::Account(protocol, accountId.toLower()), d(new Private)
{
	setMyself( new TelepathyContact(this, accountId, Kopete::ContactList::self()->myself()) );

	// Get ConnectionManager early
	d->getConnectionManager();
}

TelepathyAccount::~TelepathyAccount()
{
	delete d;
}

KActionMenu *TelepathyAccount::actionMenu()
{
	KActionMenu *actionMenu = Kopete::Account::actionMenu();

	return actionMenu;
}

void TelepathyAccount::connect(const Kopete::OnlineStatus &initialStatus)
{
	if( readConfig() )
	{
		kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Succesfully read config." << endl;
		kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Connecting to connection manager " << connectionManager() << " on protocol " << connectionProtocol() << endl;
		ConnectionManager *connectionManager = d->getConnectionManager();
		
		kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Actual connection manager: " << connectionManager->name() << endl;
		if( d->currentConnection )
		{
			delete d->currentConnection;
			d->currentConnection = 0;
		}

		if( connectionManager->isRunning() )
		{
			d->currentConnection = connectionManager->requestConnection( connectionProtocol(), connectionParameters() );
			if( d->currentConnection )
			{
				kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Got a valid connection." << endl;
				// Connect signals/slots
				QObject::connect(d->currentConnection, SIGNAL(statusChanged(Connection::Status, Connection::Reason)), this, SLOT(telepathyStatusChanged(Connection::Status, Connection::Reason)));
	
				d->currentConnection->connect( TelepathyProtocol::protocol()->kopeteStatusToTelepathy(initialStatus) );
			}
			else
			{
				kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Failed to get a valid connection." << endl;
				// TODO: Show an error message
			}
		}
		else
		{
			kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << connectionManager->name() << " is not running." << endl;
			// TODO: Shown an error message
		}
	}
}

void TelepathyAccount::disconnect()
{
	if( d->currentConnection )
		d->currentConnection->disconnect();
}

void TelepathyAccount::setOnlineStatus(const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason)
{
	if( !isConnected() )
	{
		kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Connecting with initial status: " << status.description() << endl;
		connect(status);
	}
	// FIXME: Temp
	else if ( status.status() == Kopete::OnlineStatus::Offline )
	{
		disconnect();
	}
	else
	{
		kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Chaning online status" << endl;
		//TODO:
	}
}

void TelepathyAccount::setStatusMessage(const Kopete::StatusMessage &statusMessage)
{
	
}

bool TelepathyAccount::createContact(const QString &contactId, Kopete::MetaContact *parentMetaContact)
{
	return false;
}

bool TelepathyAccount::readConfig()
{
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << endl;
	// Restore config not related to ConnectionManager parameters first
	// so that the UI for the protocol parameters will be generated
	KConfigGroup *accountConfig = configGroup();
	d->connectionManager = accountConfig->readEntry( QLatin1String("ConnectionManager"), QString() );
	d->connectionProtocol = accountConfig->readEntry( QLatin1String("SelectedProtocol"), QString() );

	// Get the preferences from the connection manager to get the right types
	QList<ConnectionManager::Parameter> tempParameters = d->getConnectionManager()->protocolParameters(d->connectionProtocol);

	// Now update the preferences
	KConfig *telepathyConfig = KGlobal::config();
	QMap<QString,QString> allEntries = telepathyConfig->entryMap( TelepathyProtocol::protocol()->formatTelepathyConfigGroup(d->connectionManager, d->connectionProtocol, accountId()) );
	QMap<QString,QString>::ConstIterator it, itEnd = allEntries.constEnd();
	for(it = allEntries.constBegin(); it != itEnd; ++it)
	{
		foreach(ConnectionManager::Parameter parameter, tempParameters)
		{
			if( parameter.name() == it.key() )
			{
				if( parameter.value().toString() != it.value() )
				{
					QVariant oldValue = parameter.value();
					QVariant newValue(oldValue.type());
					if ( oldValue.type() == QVariant::String )
						newValue = QVariant(it.value());
					else if( oldValue.type() == QVariant::Int )
						newValue = QVariant(it.value()).toInt();
					else if( oldValue.type() == QVariant::UInt )
						newValue = QVariant(it.value()).toUInt();
					else if( oldValue.type() == QVariant::Double )
						newValue = QVariant(it.value()).toDouble();
					else if( oldValue.type() == QVariant::Bool)
					{
						if( it.value().toLower() == "true")
							newValue = true;
						else
							newValue = false;
					}
					else
						newValue = QVariant(it.value());
// 					kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Name: " << parameter.name() << " Value: " << newValue << "Type: " << parameter.value().typeName() << endl;
					d->connectionParameters.append( ConnectionManager::Parameter(parameter.name(), newValue) );
					break;
				}
			}
		}
	}

	if( !d->connectionManager.isEmpty() &&
		!d->connectionProtocol.isEmpty() &&
		!d->connectionParameters.isEmpty() )
		return true;
	else
		return false;
}

QString TelepathyAccount::connectionManager() const
{
	return d->connectionManager;
}

QString TelepathyAccount::connectionProtocol() const
{
	return d->connectionProtocol;
}

QList<QtTapioca::ConnectionManager::Parameter> TelepathyAccount::connectionParameters() const
{
	foreach(ConnectionManager::Parameter parameter, d->connectionParameters)
	{
		kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Name: " << parameter.name() << " Value: " << parameter.value() << "Type: " << parameter.value().typeName() << endl;
	}
	return d->connectionParameters;
}

void TelepathyAccount::telepathyStatusChanged(Connection::Status status, Connection::Reason reason)
{
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << endl;

	switch(status)
	{
		case Connection::Connecting:
			kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Connecting...." << endl;
			break;
		case Connection::Connected:
			kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Connected using Telepathy :)" << endl;
			// FIXME: Use initial status
			myself()->setOnlineStatus( TelepathyProtocol::protocol()->Available );
			break;
		case Connection::Disconnected:
			kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Disconnected :(" << endl;
			break;
	}
	//TODO: reason
}

ConnectionManager *TelepathyAccount::Private::getConnectionManager()
{
	if( !currentConnectionManager )
	{
		currentConnectionManager = ConnectionManagerFactory::self()->getConnectionManagerByName(connectionManager);
	}

	return currentConnectionManager;
}
#include "telepathyaccount.moc"
