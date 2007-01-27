/*
 * messengerprotocol.cpp - Windows Live Messenger Kopete protocol definition.
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 * 
 * Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>
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

#include "messengerprotocol.h"

// KDE includes
#include <kgenericfactory.h>

// Kopete includes
#include <kopeteaccount.h>
#include <kopetemetacontact.h>

typedef KGenericFactory<MessengerProtocol> MessengerProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_messenger, MessengerProtocolFactory("kopete_messenger") )

MessengerProtocol *MessengerProtocol::s_self = 0;

MessengerProtocol::MessengerProtocol(QObject *parent, const QStringList &/*args*/)
 : Kopete::Protocol(MessengerProtocolFactory::componentData(), parent)
{
	s_self = this;

	addAddressBookField( "messaging/messenger", Kopete::Plugin::MakeIndexField );
}

MessengerProtocol *MessengerProtocol::protocol()
{
	return s_self;
}

Kopete::Account *MessengerProtocol::createNewAccount(const QString &accountId)
{
	// TODO
	return 0;
}

AddContactPage *MessengerProtocol::createAddContactWidget(QWidget *parent, Kopete::Account *account)
{
	return 0;
}

KopeteEditAccountWidget *MessengerProtocol::createEditAccountWidget(Kopete::Account *account, QWidget *parent)
{
	return 0;
}

Kopete::Contact *MessengerProtocol::deserializeContact( Kopete::MetaContact *metaContact, 
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData )
{
	return 0;
}
