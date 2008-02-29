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
#include "messengeraccount.h"
#include "ui/messengereditaccountwidget.h"
#include "ui/messengeraddcontactpage.h"

K_PLUGIN_FACTORY( MessengerProtocolFactory, registerPlugin<MessengerProtocol>(); )
K_EXPORT_PLUGIN( MessengerProtocolFactory( "kopete_messenger" ) )

MessengerProtocol *MessengerProtocol::s_self = 0;

MessengerProtocol::MessengerProtocol(QObject *parent, const QVariantList &/*args*/)
 : Kopete::Protocol(MessengerProtocolFactory::componentData(), parent)
{
	s_self = this;

	addAddressBookField( "messaging/messenger", Kopete::Plugin::MakeIndexField );
}

MessengerProtocol *MessengerProtocol::protocol()
{
	return s_self;
}

bool MessengerProtocol::validContactId(const QString& userid)
{
	return( userid.count("@") ==1 && userid.count(".") >=1 /*&& userid.count(QChar(' ')) == 1*/ );
}

Kopete::Account *MessengerProtocol::createNewAccount(const QString &accountId)
{
	// TODO
	return new MessengerAccount(this, accountId);
}

AddContactPage *MessengerProtocol::createAddContactWidget(QWidget *parent, Kopete::Account *account)
{
	MessengerAccount *messengerAccount=dynamic_cast<MessengerAccount*>(account);
	return (new MessengerAddContactPage(messengerAccount,parent));
}

KopeteEditAccountWidget *MessengerProtocol::createEditAccountWidget(Kopete::Account *account, QWidget *parent)
{
	kDebug () << "Edit Account Widget MESSAGER";
	MessengerAccount *messengerAccount=dynamic_cast < MessengerAccount * >(account);
	if(messengerAccount || !account)
		return new MessengerEditAccountWidget (this,messengerAccount , parent);

	return 0l;
}

Kopete::Contact *MessengerProtocol::deserializeContact( Kopete::MetaContact *metaContact, 
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData )
{
	return 0;
}
