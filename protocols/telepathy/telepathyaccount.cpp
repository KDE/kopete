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
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kicon.h>

// Kopete includes
#include <kopetemetacontact.h>
#include <kopeteonlinestatus.h>
#include <kopetecontactlist.h>
#include <kopetechatsessionmanager.h>
#include <kopeteuiglobal.h>

// QtTapioca includes
#include <QtTapioca/ConnectionManagerFactory>
#include <QtTapioca/ContactList>
#include <QtTapioca/Contact>
#include <QtTapioca/UserContact>
#include <QtTapioca/TextChannel>

// Local includes
#include "telepathyprotocol.h"
#include "telepathycontact.h"
#include "telepathycontactmanager.h"
#include "telepathychatsession.h"

using namespace QtTapioca;

class TelepathyAccount::Private
{
public:
	Private()
	 : currentConnectionManager(0), currentConnection(0), contactManager(0)
	{}

	ConnectionManager *getConnectionManager();

	QString connectionManager;
	QString connectionProtocol;
	QList<ConnectionManager::Parameter> connectionParameters;
	QList<ConnectionManager::Parameter> allConnectionParameters;
	ConnectionManager *currentConnectionManager;
	Connection *currentConnection;
	Kopete::OnlineStatus initialStatus;
	TelepathyContactManager *contactManager;
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

	// FIXME: Maybe we should cache the action.
	KAction *changeAliasAction = new KAction( KIcon("userconfig"), i18n("&Change Alias..."), 0 );
	changeAliasAction->setEnabled( isConnected() );
	QObject::connect(changeAliasAction, SIGNAL(triggered(bool)), this, SLOT(slotSetAlias()));

	actionMenu->addSeparator();
	actionMenu->addAction( changeAliasAction );

	return actionMenu;
}

TelepathyContact *TelepathyAccount::myself()
{
	return static_cast<TelepathyContact*>( Kopete::Account::myself() );
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
				QObject::connect(d->currentConnection, SIGNAL(statusChanged(QtTapioca::Connection*, QtTapioca::Connection::Status, QtTapioca::Connection::Reason)), this, SLOT(telepathyStatusChanged(QtTapioca::Connection*, QtTapioca::Connection::Status, QtTapioca::Connection::Reason)));

				QObject::connect(d->currentConnection, SIGNAL(channelCreated(QtTapioca::Connection*, QtTapioca::Channel*)), this, SLOT(telepathyChannelCreated(QtTapioca::Connection*,QtTapioca::Channel*)));

				QObject::connect(this, SIGNAL(telepathyConnected()), this, SLOT(slotTelepathyConnected()));

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
		d->initialStatus = status;
		connect(status);
	}
	else if ( status.status() == Kopete::OnlineStatus::Offline )
	{
		disconnect();
	}
	else
	{
		if( d->currentConnection && d->currentConnection->userContact() )
		{
			kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Changing online status to " << status.description() << endl;
			d->currentConnection->userContact()->setPresence( TelepathyProtocol::protocol()->kopeteStatusToTelepathy(status) );

			setStatusMessage( reason );
		}
	}
}

void TelepathyAccount::setStatusMessage(const Kopete::StatusMessage &statusMessage)
{
	if( d->currentConnection && d->currentConnection->userContact() )
	{
		kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Setting status message to \"" << statusMessage.message() << "\"." << endl;

		d->currentConnection->userContact()->setPresenceMessage( statusMessage.message() );
	}
}

bool TelepathyAccount::changeAlias(const QString &newAlias)
{
	if( d->currentConnection && d->currentConnection->userContact() )
	{
		if( d->currentConnection->userContact()->setAlias( newAlias ) )
		{
			myself()->setNickName( newAlias );
			return true;
		}
	}

	return false;
}

void TelepathyAccount::slotSetAlias()
{
	QString currentAlias = myself()->nickName();

	bool ok;
	QString newAlias = KInputDialog::getText(
			i18n("Change alias"), 
			i18n("Enter the new alias by which you want to be visible to your friends:"), 
			currentAlias,
			&ok );

	if( ok )
	{
		if( !changeAlias(newAlias) )
		{
			KMessageBox::error( Kopete::UI::Global::mainWidget(), i18n("Current connection manager does not support changing the visible alias to your friends.") );
		}
	}
}

bool TelepathyAccount::createContact(const QString &contactId, Kopete::MetaContact *parentMetaContact)
{
	if( !contacts()[contactId] )
	{
		TelepathyContact *contact = new TelepathyContact(this, contactId, parentMetaContact);
		
		return contact != 0;
	}
	else
		kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Contact " << contactId << " already exists." << endl;

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

	// Clear current connection parameters
	d->allConnectionParameters.clear();
	d->connectionParameters.clear();

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

QList<QtTapioca::ConnectionManager::Parameter> TelepathyAccount::allConnectionParameters()
{
	if( d->allConnectionParameters.isEmpty() )
	{
		if( d->connectionProtocol.isEmpty() )
			readConfig();

		QList<ConnectionManager::Parameter> allParameters = d->getConnectionManager()->protocolParameters(d->connectionProtocol);
		foreach(ConnectionManager::Parameter parameter, allParameters)
		{
			ConnectionManager::Parameter newParameter = parameter;
			foreach(ConnectionManager::Parameter connectionParameter, d->connectionParameters)
			{
				// Use value from the saved connection parameter
				if( parameter.name() == connectionParameter.name() )
				{
					newParameter = ConnectionManager::Parameter( parameter.name(), connectionParameter.value() );
					break;
				}
			}

			d->allConnectionParameters.append( newParameter );
		}
	}
	
	return d->allConnectionParameters;
}

void TelepathyAccount::createTextChatSession(QtTapioca::TextChannel *newChannel)
{
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << endl;

	// Get contact id
	QString contactUri = newChannel->target()->uri();
	Kopete::Contact *destinationContact = contacts()[contactUri];
	if( destinationContact )
	{
		Kopete::ContactPtrList others;
		others.append( destinationContact );

		// Try to find an existing chatsession
		Kopete::ChatSession *currentChatSession = Kopete::ChatSessionManager::self()->findChatSession( myself(), others, protocol() );
		if( !currentChatSession )
		{
			kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Creating a new chat session" << endl;

			TelepathyChatSession *newChatSession = new TelepathyChatSession( myself(), others, protocol() );
			newChatSession->setTextChannel( newChannel );
		}
		else
			kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Found an existing chat session." << endl;
	}
}

QtTapioca::TextChannel *TelepathyAccount::createTextChannel(QtTapioca::Contact *internalContact)
{
	if( d->currentConnection && isConnected() )
	{
		return dynamic_cast<QtTapioca::TextChannel*>( d->currentConnection->createChannel(QtTapioca::Channel::Text, internalContact) );
	}

	return 0;
}

void TelepathyAccount::telepathyStatusChanged(QtTapioca::Connection *connection, QtTapioca::Connection::Status status, QtTapioca::Connection::Reason reason)
{
	Q_UNUSED(connection);

	switch(status)
	{
		case Connection::Connecting:
			kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Connecting...." << endl;
			break;
		case Connection::Connected:
			kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Connected using Telepathy :)" << endl;
			emit telepathyConnected();
			break;
		case Connection::Disconnected:
			kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Disconnected :(" << endl;
			break;
	}
	//TODO: reason
}

void TelepathyAccount::telepathyChannelCreated(QtTapioca::Connection *connection, QtTapioca::Channel *channel)
{
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo  << endl;

	Q_UNUSED(connection);

	if( channel->type() == QtTapioca::Channel::Text )
	{
		// Find or create a new chat session
		QtTapioca::TextChannel *textChannel = dynamic_cast<QtTapioca::TextChannel*>(channel);
		if( textChannel )
			createTextChatSession(textChannel);
	}
}

void TelepathyAccount::slotTelepathyConnected()
{
	// Set initial status to myself contact
	myself()->setOnlineStatus( d->initialStatus );
	// Set nickname to myself contact
	myself()->setNickName( d->currentConnection->userContact()->alias() );

	// Load contact list
	fetchContactList();
}

void TelepathyAccount::fetchContactList()
{
	contactManager()->setContactList( d->currentConnection->contactList() );
	contactManager()->loadContacts();
}

ConnectionManager *TelepathyAccount::Private::getConnectionManager()
{
	if( !currentConnectionManager )
	{
		currentConnectionManager = ConnectionManagerFactory::self()->getConnectionManagerByName(connectionManager);
	}

	return currentConnectionManager;
}

TelepathyContactManager *TelepathyAccount::contactManager()
{
	if( !d->contactManager )
	{
		d->contactManager = new TelepathyContactManager(this);
	}

	return d->contactManager;
}

#include "telepathyaccount.moc"
