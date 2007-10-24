/*
    kopetedbusinterface.h - Kopete D-Bus interface

    Copyright (c) 2007      by Michaël Larouche      <larouche@kde.org>
    Copyright (c) 2007         Will Stephenson       <wstephenson@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "kopetedbusinterface.h"

// Qt includes
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QLatin1String>

// KDE includes
#include <kurl.h>

// Kopete includes
#include <kopetecontactlist.h>
#include <kopetemetacontact.h>
#include <kopetecontact.h>
#include <kopeteidentity.h>
#include <kopeteidentitymanager.h>
#include <kopeteavatarmanager.h>
#include <kopeteaccount.h>
#include <kopeteaccountmanager.h>
#include <kopeteonlinestatus.h>
#include <kopetemessage.h>
#include <kopetechatsession.h>
#include <kopetegroup.h>

// Local includes
#include "kopeteadaptor.h"

QStringList listContact(const QList<Kopete::MetaContact*> &contactList)
{
	QStringList result;

	foreach(Kopete::MetaContact *contact, contactList)
	{
		result << contact->metaContactId();
	}

	return result;
}

Kopete::OnlineStatusManager::Categories status2Value(const QString &status)
{
	Kopete::OnlineStatusManager::Categories statusValue;
	if( status.toLower() == QLatin1String("online") || status.toLower() == QLatin1String("available") )
	{
		statusValue = Kopete::OnlineStatusManager::Online;
	}
	else if( status.toLower() == QLatin1String("busy") )
	{
		statusValue = Kopete::OnlineStatusManager::Busy;
	}
	else if( status.toLower() == QLatin1String("away") )
	{
		statusValue = Kopete::OnlineStatusManager::Away;
	}

	return statusValue;
}

KopeteDBusInterface::KopeteDBusInterface(QObject *parent)
 : QObject(parent)
{
	new KopeteAdaptor(this);
	QDBusConnection::sessionBus().registerObject("/Kopete", this);
}

KopeteDBusInterface::~KopeteDBusInterface()
{
}

QStringList KopeteDBusInterface::contacts() const
{
	return listContact(Kopete::ContactList::self()->metaContacts());
}

QStringList KopeteDBusInterface::contactsByFilter(const QString &filter) const
{
	QList<Kopete::MetaContact*> completeList = Kopete::ContactList::self()->metaContacts();
	QList<Kopete::MetaContact*> filteredList;

	Kopete::MetaContact *contact;
	
	if( filter.toLower() == QLatin1String("online") )
	{
		foreach(contact, completeList)
		{
			if( contact->isOnline() )
				filteredList << contact;
		}
	}
	else if( filter.toLower() == QLatin1String("reachable") )
	{
		foreach(contact, completeList)
		{
			if( contact->isReachable() )
				filteredList << contact;
		}
	}
	else if( filter.toLower() == QLatin1String("filecapable") )
	{
		foreach(contact, completeList)
		{
			if( contact->canAcceptFiles() )
				filteredList << contact;
		}
	}

	return listContact(filteredList);
}

void KopeteDBusInterface::setIdentityNickName(const QString &nickName, const QString &identityId)
{
	Kopete::Identity *identity = 0;

	if( identityId.isEmpty() )
	{
		identity = Kopete::IdentityManager::self()->defaultIdentity();
	}
	else
	{
		identity = Kopete::IdentityManager::self()->findIdentity(identityId);
	}

	if( identity )
	{
		identity->setProperty( Kopete::Global::Properties::self()->nickName(), nickName );
	}
}

void KopeteDBusInterface::setIdentityAvatar(const QString &avatarUrl, const QString &identityId)
{
	Kopete::Identity *identity = 0;

	if( identityId.isEmpty() )
	{
		identity = Kopete::IdentityManager::self()->defaultIdentity();
	}
	else
	{
		identity = Kopete::IdentityManager::self()->findIdentity(identityId);
	}

	if( identity )
	{
		// Add the avatar using AvatarManager
		Kopete::AvatarManager::AvatarEntry avatarEntry;
		avatarEntry.name = "D-Bus Avatar";
		avatarEntry.path = KUrl(avatarUrl).path();
		avatarEntry.category = Kopete::AvatarManager::User;
		
		avatarEntry = Kopete::AvatarManager::self()->add(avatarEntry);

		identity->setProperty( Kopete::Global::Properties::self()->photo(), avatarEntry.path );
	}
}

void KopeteDBusInterface::setIdentityOnlineStatus(const QString &status, const QString &message, const QString &identityId)
{
	Kopete::Identity *identity = 0;
	
	if( identityId.isEmpty() )
	{
		identity = Kopete::IdentityManager::self()->defaultIdentity();
	}
	else
	{
		identity = Kopete::IdentityManager::self()->findIdentity(identityId);
	}

	if( identity )
	{
		identity->setOnlineStatus( status2Value(status), message );
	}
}

QStringList KopeteDBusInterface::identities() const
{
	QStringList result;

	foreach(Kopete::Identity *identity, Kopete::IdentityManager::self()->identities())
	{
		result << identity->id();
	}

	return result;
}

QString KopeteDBusInterface::labelForIdentity(const QString & id) const
{
    Kopete::Identity * identity = Kopete::IdentityManager::self()->findIdentity(id);
    if ( identity ) {
        return identity->label();
    }
}

QStringList KopeteDBusInterface::accounts() const
{
	QStringList result;

	foreach(Kopete::Account *account, Kopete::AccountManager::self()->accounts())
	{
		result << account->accountId();
	}

	return result;
}

void KopeteDBusInterface::connectAll()
{
	Kopete::AccountManager::self()->connectAll();
}

void KopeteDBusInterface::disconnectAll()
{
	Kopete::AccountManager::self()->disconnectAll();
}

void KopeteDBusInterface::setOnlineStatus(const QString &status, const QString &message)
{
	Kopete::AccountManager::self()->setOnlineStatus( status2Value(status), message );
}

void KopeteDBusInterface::setStatusMessage(const QString &message)
{
	Kopete::AccountManager::self()->setStatusMessage(message);
}

void KopeteDBusInterface::sendMessage(const QString &displayName, const QString &message)
{
	Kopete::MetaContact *destMetaContact = Kopete::ContactList::self()->findMetaContactByDisplayName(displayName);
	if( destMetaContact && destMetaContact->isReachable() )
	{
		Kopete::Contact *destContact = destMetaContact->execute();
		if( destContact )
		{
			Kopete::Message newMessage( destContact->account()->myself(), destContact );
			newMessage.setPlainBody( message );
			newMessage.setDirection( Kopete::Message::Outbound );

			destContact->manager(Kopete::Contact::CanCreate)->sendMessage( newMessage );
		}
	}
}

bool KopeteDBusInterface::addContact(const QString &protocolName, const QString &accountId, const QString &contactId, const QString &displayName, const QString &groupName)
{
	QString protocolId = protocolName;
	if( !protocolName.contains("Protocol") )
	{
		protocolId += QLatin1String("Protocol");
	}

	// Find the account using the given parameters on D-Bus
	Kopete::Account *account = Kopete::AccountManager::self()->findAccount( protocolId, accountId );

	if( account )
	{
		QString contactName;
		Kopete::Group *realGroup=0;

		if( displayName.isEmpty() )
		{
			contactName = contactId;
		}
		else
		{
			contactName = displayName;
		}

		if ( !groupName.isEmpty() )
			realGroup = Kopete::ContactList::self()->findGroup( groupName );

		account->addContact( contactId, contactName, realGroup, Kopete::Account::DontChangeKABC);

		return true;
	}

	return false;
}

void KopeteDBusInterface::sendFile(const QString &displayName, const QString &fileUrl)
{
	Kopete::MetaContact *destMetaContact = Kopete::ContactList::self()->findMetaContactByDisplayName(displayName);
	if( destMetaContact && destMetaContact->isReachable() )
	{
		Kopete::Contact *destContact = destMetaContact->execute();
		if( destContact && destContact->isFileCapable() )
		{
			destContact->sendFile( KUrl(fileUrl) );
		}
	}
}

void KopeteDBusInterface::connect(const QString &protocolName, const QString &accountId)
{
	QString protocolId = protocolName;
	if( !protocolName.contains("Protocol") )
	{
		protocolId += QLatin1String("Protocol");
	}

	Kopete::Account *account = Kopete::AccountManager::self()->findAccount(protocolId, accountId);
	if( account )
	{
		account->connect();
	}
}

void KopeteDBusInterface::disconnect(const QString &protocolName, const QString &accountId)
{
	QString protocolId = protocolName;
	if( !protocolName.contains("Protocol") )
	{
		protocolId += QLatin1String("Protocol");
	}

	Kopete::Account *account = Kopete::AccountManager::self()->findAccount(protocolId, accountId);
	if( account )
	{
		account->disconnect();
	}
}

#include "kopetedbusinterface.moc"
