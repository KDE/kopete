/*
 * telepathyaccount.cpp - Telepathy Kopete Account.
 *
 * Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>
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
#include <QtTapioca/ConnectionManager>
#include <QtTapioca/ConnectionManagerFactory>

// Local includes
#include "telepathyprotocol.h"
#include "telepathycontact.h"

using namespace QtTapioca;

class TelepathyAccount::Private
{
public:
	QString connectionManager;
	QString connectionProtocol;
	QList<ConnectionManager::Parameter> connectionParameters;
};

TelepathyAccount::TelepathyAccount(TelepathyProtocol *protocol, const QString &accountId)
 : Kopete::Account(protocol, accountId.toLower()), d(new Private)
{
	setMyself( new TelepathyContact(this, accountId, Kopete::ContactList::self()->myself()) );
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
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Connecting to connection manager" << endl;
	if( readConfig() )
	{
		
	}
}

void TelepathyAccount::disconnect()
{
	
}

void TelepathyAccount::setOnlineStatus(const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason)
{
	if( !isConnected() )
	{
		kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Connecting with initial status: " << status.description() << endl;
		connect(status);
	}
	else
	{
		kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Chaning online status" << endl;
		// TODO
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

	// Now fill the preferences
	KConfig *telepathyConfig = KGlobal::config();
	QMap<QString,QString> allEntries = telepathyConfig->entryMap( TelepathyProtocol::protocol()->formatTelepathyConfigGroup(d->connectionManager, d->connectionProtocol, accountId()) );
	QMap<QString,QString>::ConstIterator it, itEnd = allEntries.constEnd();
	for(it = allEntries.constBegin(); it != itEnd; ++it)
	{
		ConnectionManager::Parameter parameter(it.key(), it.value());
		d->connectionParameters.append(parameter);
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
	return d->connectionParameters;
}


#include "telepathyaccount.moc"
