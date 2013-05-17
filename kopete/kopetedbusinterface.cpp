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
#include "kopetedbusinterface_p.h"

// Qt includes
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QLatin1String>

// KDE includes
#include <kurl.h>
#include <kplugininfo.h>

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
#include <kopetepluginmanager.h>
#include <kopetepicture.h>
#include <kopeteviewmanager.h>
#include <kopetemessageevent.h>

// Local includes
#include "kopeteadaptor.h"

KopeteDBusInterface::KopeteDBusInterface(QObject *parent) :
	QObject(parent), d(new KopeteDBusInterfacePrivate())
{
	setObjectName("KopeteDBusInterface");
	new KopeteAdaptor(this);
	QDBusConnection::sessionBus().registerObject("/Kopete", this);

	QObject::connect(d, SIGNAL(contactChanged(QString)), this, SIGNAL(contactChanged(QString)));
}

KopeteDBusInterface::~KopeteDBusInterface()
{
}

QStringList KopeteDBusInterface::protocols() const
{
	QStringList list;
	foreach(KPluginInfo p, Kopete::PluginManager::self()->availablePlugins("Protocols"))
	{ 
		list << p.name();
	}
	return list;
}

QStringList KopeteDBusInterface::contacts() const
{
	return d->listContact(Kopete::ContactList::self()->metaContacts());
}

QStringList KopeteDBusInterface::contactsByFilter(const QString &filter) const
{
	QList<Kopete::MetaContact*> completeList =
			Kopete::ContactList::self()->metaContacts();
	QList<Kopete::MetaContact*> filteredList;

	Kopete::MetaContact *contact;

	if (filter.toLower() == QLatin1String("online"))
	{
		// "online" returns contacts that are not offline, which means that
		// those being away, busy etc. are included as well. 
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
	else if ( filter.toLower() == QLatin1String("away") )
	{
		foreach(contact, completeList)
		{
			if( contact->status() == Kopete::OnlineStatus::Away )
			{
				filteredList << contact;
			}
		}
	}
	else if ( filter.toLower() == QLatin1String("busy") )
	{
		foreach(contact, completeList)
		{
			if( contact->status() == Kopete::OnlineStatus::Busy )
			{
				filteredList << contact;
			}
		}
	}

	return d->listContact(filteredList);
}

void KopeteDBusInterface::setIdentityNickName(const QString &nickName,
		const QString &identityId)
{
	Kopete::Identity *identity = 0;

	if (identityId.isEmpty())
	{
		identity = Kopete::IdentityManager::self()->defaultIdentity();
	}
	else
	{
		identity = Kopete::IdentityManager::self()->findIdentity(identityId);
	}

	if (identity)
	{
		identity->setProperty(Kopete::Global::Properties::self()->nickName(),
				nickName);
	}
}

void KopeteDBusInterface::setIdentityAvatar(const QString &avatarUrl,
		const QString &identityId)
{
	Kopete::Identity *identity = 0;

	if (identityId.isEmpty())
	{
		identity = Kopete::IdentityManager::self()->defaultIdentity();
	}
	else
	{
		identity = Kopete::IdentityManager::self()->findIdentity(identityId);
	}

	if (identity)
	{
		// Add the avatar using AvatarManager
		Kopete::AvatarManager::AvatarEntry avatarEntry;
		avatarEntry.name = "D-Bus Avatar";
		avatarEntry.path = KUrl(avatarUrl).path();
		avatarEntry.category = Kopete::AvatarManager::User;

		avatarEntry = Kopete::AvatarManager::self()->add(avatarEntry);

		identity->setProperty(Kopete::Global::Properties::self()->photo(),
				avatarEntry.path);
	}
}

void KopeteDBusInterface::setIdentityOnlineStatus(const QString &status,
		const QString &message, const QString &identityId)
{
	Kopete::Identity *identity = 0;

	if (identityId.isEmpty())
	{
		identity = Kopete::IdentityManager::self()->defaultIdentity();
	}
	else
	{
		identity = Kopete::IdentityManager::self()->findIdentity(identityId);
	}

	if (identity)
	{
		identity->setOnlineStatus(d->status2Value(status), message);
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
	Kopete::Identity * identity =
			Kopete::IdentityManager::self()->findIdentity(id);
	if (identity)
	{
		return identity->label();
	}
	else
	{
		return QString();
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
	Kopete::AccountManager::self()->setOnlineStatus(
			Kopete::OnlineStatusManager::Online, QString(),
			Kopete::AccountManager::ConnectIfOffline);
}

void KopeteDBusInterface::disconnectAll()
{
	Kopete::AccountManager::self()->setOnlineStatus(
			Kopete::OnlineStatusManager::Offline);
}

void KopeteDBusInterface::suspend()
{
	Kopete::AccountManager::self()->suspend();
}

void KopeteDBusInterface::resume()
{
	Kopete::AccountManager::self()->resume();
}

void KopeteDBusInterface::setOnlineStatus(const QString &status,
		const QString &message)
{
	Kopete::AccountManager::self()->setOnlineStatus(d->status2Value(status),
			message);
}

void KopeteDBusInterface::setStatusMessage(const QString &message)
{
	Kopete::AccountManager::self()->setStatusMessage(message);
}

void KopeteDBusInterface::sendMessage(const QString &contactId,
		const QString &message)
{
	Kopete::MetaContact *destMetaContact = d->findContact(contactId);
	if (destMetaContact && destMetaContact->isReachable())
	{
		Kopete::Contact *destContact = destMetaContact->execute();
		if (destContact)
		{
			Kopete::Message newMessage(destContact->account()->myself(),
					destContact);
			newMessage.setPlainBody(message);
			newMessage.setDirection(Kopete::Message::Outbound);

			destContact->manager(Kopete::Contact::CanCreate)->sendMessage(
					newMessage);
		}
	}
}

void KopeteDBusInterface::openChat(const QString &contactId)
{
	Kopete::MetaContact *contact = d->findContact(contactId);
	if (contact && contact->isReachable())
	{
		Kopete::Contact *preferredContact = contact->preferredContact();
		if (preferredContact && preferredContact->account()
				&& preferredContact != preferredContact->account()->myself())
		{
			contact->execute();
		}
	}
}

QString KopeteDBusInterface::getDisplayName(const QString &contactId)
{
	Kopete::MetaContact *contact = d->findContact(contactId);
	if (contact)
		return contact->displayName();
	else
		return "";
}

bool KopeteDBusInterface::isContactOnline(const QString &contactId)
{
	Kopete::MetaContact *contact = d->findContact(contactId);
	if (contact)
		return contact->isOnline();
	else
		return false;
}

bool KopeteDBusInterface::addContact(const QString &protocolName,
		const QString &accountId, const QString &contactId,
		const QString &displayName, const QString &groupName)
{
	QString protocolId = protocolName;
	if (!protocolName.contains("Protocol"))
	{
		protocolId += QLatin1String("Protocol");
	}

	// Find the account using the given parameters on D-Bus
	Kopete::Account *account = Kopete::AccountManager::self()->findAccount(
			protocolId, accountId);

	if (account)
	{
		QString contactName;
		Kopete::Group *realGroup = 0;

		if (displayName.isEmpty())
		{
			contactName = contactId;
		}
		else
		{
			contactName = displayName;
		}

		if (!groupName.isEmpty())
			realGroup = Kopete::ContactList::self()->findGroup(groupName);

		account->addContact(contactId, contactName, realGroup,
				Kopete::Account::DontChangeKABC);

		return true;
	}

	return false;
}

void KopeteDBusInterface::sendFile(const QString &contactId,
		const QString &fileUrl)
{
	Kopete::MetaContact *destMetaContact = d->findContact(contactId);
	if (destMetaContact && destMetaContact->isReachable())
	{
		Kopete::Contact *destContact = destMetaContact->execute();
		if (destContact && destContact->isFileCapable())
		{
			destContact->sendFile(KUrl(fileUrl));
		}
	}
}

bool KopeteDBusInterface::isConnected(const QString &protocolName,
		const QString &accountId)
{
	QString protocolId = protocolName;
	if (!protocolName.contains("Protocol"))
	{
		protocolId += QLatin1String("Protocol");
	}

	Kopete::Account *account = Kopete::AccountManager::self()->findAccount(
			protocolId, accountId);
	return account ? account->isConnected() : false;
}

void KopeteDBusInterface::connect(const QString &protocolName,
		const QString &accountId)
{
	QString protocolId = protocolName;
	if (!protocolName.contains("Protocol"))
	{
		protocolId += QLatin1String("Protocol");
	}

	Kopete::Account *account = Kopete::AccountManager::self()->findAccount(
			protocolId, accountId);
	if (account)
	{
		account->connect();
	}
}

void KopeteDBusInterface::disconnect(const QString &protocolName,
		const QString &accountId)
{
	QString protocolId = protocolName;
	if (!protocolName.contains("Protocol"))
	{
		protocolId += QLatin1String("Protocol");
	}

	Kopete::Account *account = Kopete::AccountManager::self()->findAccount(
			protocolId, accountId);
	if (account)
	{
		account->disconnect();
	}
}

QVariantMap KopeteDBusInterface::contactProperties(const QString &contactId)
{
	QVariantMap properties;
	Kopete::MetaContact *contact = d->findContact(contactId);

	if (contact)
	{
		properties["status"] = Kopete::OnlineStatus::statusTypeToString(
				contact->status());
		properties["message_reachable"] = contact->isReachable();
		properties["file_reachable"] = contact->canAcceptFiles();
		properties["display_name"] = contact->displayName();
		properties["id"] = contact->metaContactId().toString();
		if (contact->photoSource() == Kopete::MetaContact::SourceCustom)
		{
			properties["picture"] = contact->customPhoto().prettyUrl();
		}
		else
		{
			properties["picture"] = contact->picture().path();
		}
		properties["idle_time"] = qulonglong(contact->idleTime());
		if (contact->preferredContact())
		{
			/** @todo: export status message title as well or merge both? */
			properties["status_message"]
					= contact->preferredContact()->statusMessage().message();
		}
		
		QStringList messages;
		foreach(Kopete::Contact *subContact, contact->contacts())
		{
	        QList<Kopete::MessageEvent*> pendingMessages = KopeteViewManager::viewManager()->pendingMessages(subContact);
	        foreach(Kopete::MessageEvent *event, pendingMessages)
	        {
	        	messages << event->message().parsedBody();
	        }
		}
		properties["pending_messages"] = messages;
	}

	return properties;
}

#include "kopetedbusinterface.moc"
