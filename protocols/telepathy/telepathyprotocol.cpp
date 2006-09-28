/*
 * telepathyprotocol.cpp - Telepathy protocol definition.
 *
 * Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>
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

#include "telepathyprotocol.h"

// KDE includes
#include <kgenericfactory.h>

// Kopete includes
#include <kopeteaccount.h>
#include <kopetemetacontact.h>

// Local includes
#include "telepathyaccount.h"
#include "telepathyeditaccountwidget.h"

typedef KGenericFactory<TelepathyProtocol> TelepathyProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_telepathy, TelepathyProtocolFactory("kopete_telepathy") )

TelepathyProtocol *TelepathyProtocol::s_self = 0;

TelepathyProtocol::TelepathyProtocol(QObject *parent, const QStringList &/*args*/)
 : Kopete::Protocol(TelepathyProtocolFactory::instance(), parent)
{
	s_self = this;

	addAddressBookField( "messaging/telepathy", Kopete::Plugin::MakeIndexField );
}

TelepathyProtocol *TelepathyProtocol::protocol()
{
	return s_self;
}

Kopete::Account *TelepathyProtocol::createNewAccount(const QString &accountId)
{
	return new TelepathyAccount(this, accountId);
}

AddContactPage *TelepathyProtocol::createAddContactWidget(QWidget *parent, Kopete::Account *account)
{
	return 0;
}

KopeteEditAccountWidget *TelepathyProtocol::createEditAccountWidget(Kopete::Account *account, QWidget *parent)
{
	return new TelepathyEditAccountWidget(account, parent);
}

Kopete::Contact *TelepathyProtocol::deserializeContact( Kopete::MetaContact *metaContact, 
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData )
{
	return 0;
}
