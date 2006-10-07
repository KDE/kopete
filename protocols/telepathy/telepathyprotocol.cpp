/*
 * telepathyprotocol.cpp - Telepathy protocol definition.
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

#include "telepathyprotocol.h"

// KDE includes
#include <kgenericfactory.h>

// Kopete includes
#include <kopeteaccount.h>
#include <kopetemetacontact.h>

// Local includes
#include "telepathyaccount.h"
#include "telepathyeditaccountwidget.h"

using namespace QtTapioca;

typedef KGenericFactory<TelepathyProtocol> TelepathyProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_telepathy, TelepathyProtocolFactory("kopete_telepathy") )

TelepathyProtocol *TelepathyProtocol::s_self = 0;

TelepathyProtocol::TelepathyProtocol(QObject *parent, const QStringList &/*args*/)
 : Kopete::Protocol(TelepathyProtocolFactory::instance(), parent),
	// Create Kopete::OnlineStatus
	Available(Kopete::OnlineStatus::Online, 25, this, 1, QStringList(),
			i18n( "Available" ), i18n( "A&vailable" ), Kopete::OnlineStatusManager::Online ),
	Away(Kopete::OnlineStatus::Away, 18, this, 4, QStringList(QString::fromLatin1("contact_away_overlay")),
			i18n( "Away From Computer" ), i18n( "&Away" ), Kopete::OnlineStatusManager::Away ),
	Busy(Kopete::OnlineStatus::Away, 20, this, 2, QStringList(),
			i18n( "Busy" ), i18n( "&Busy" ), Kopete::OnlineStatusManager::Busy),
	Hidden(Kopete::OnlineStatus::Invisible, 3, this, 8, QStringList(QString::fromLatin1("contact_invisible_overlay")), 
			i18n( "Invisible" ), i18n( "&Hidden" ), Kopete::OnlineStatusManager::Invisible),
	ExtendedAway(Kopete::OnlineStatus::Away, 15, this, 4, QStringList(QString::fromLatin1("contact_away_overlay")),
			i18n( "Extented Away" ), i18n( "&Extented Away" ), Kopete::OnlineStatusManager::Away),
	Offline(Kopete::OnlineStatus::Offline, 0, this, 7, QStringList(),
			i18n( "Offline" ), i18n( "&Offline" ), Kopete::OnlineStatusManager::Offline,
			Kopete::OnlineStatusManager::DisabledIfOffline)
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

QString TelepathyProtocol::formatTelepathyConfigGroup(const QString &connectionManager, const QString &protocol, const QString &accountId)
{
	return QString("Telepathy_%1_%2_%3").arg(connectionManager).arg(protocol).arg(accountId);
}

QtTapioca::ContactInfo::Presence TelepathyProtocol::kopeteStatusToTelepathy(const Kopete::OnlineStatus &status)
{
	QtTapioca::ContactInfo::Presence telepathyPresence;

	if( status == Available )
		telepathyPresence = ContactInfo::Available;
	else if( status == Away )
		telepathyPresence = ContactInfo::Away;
	else if( status == Busy )
		telepathyPresence = ContactInfo::Busy;
	else if( status == Hidden )
		telepathyPresence = ContactInfo::Hidden;
	else if( status == ExtendedAway )
		telepathyPresence = ContactInfo::XA;
	else if( status == Offline )
		telepathyPresence = ContactInfo::Offline;

	return telepathyPresence;
}
Kopete::OnlineStatus TelepathyProtocol::telepathyStatusToKopete(QtTapioca::ContactInfo::Presence presence)
{
	Kopete::OnlineStatus result;
	switch(presence)
	{
		case ContactInfo::Available:
			result = Available;
			break;
		case ContactInfo::Away:
			result = Away;
			break;
		case ContactInfo::Busy:
			result = Busy;
			break;
		case ContactInfo::Hidden:
			result = Hidden;
			break;
		case ContactInfo::XA:
			result = ExtendedAway;
			break;
		case ContactInfo::Offline:
			result = Offline;
			break;
	}

	return result;
}

#include "telepathyprotocol.moc"
