/*
 * telepathycontact.h - Telepathy Kopete Contact.
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
#ifndef TELEPATHYCONTACT_H
#define TELEPATHYCONTACT_H

// Qt Includes
#include <QtCore/QMap>
#include <QtCore/QList>

// Kopete includes
#include <kopetecontact.h>

// QtTapioca includes
#include <QtTapioca/ContactBase>

class KAction;

namespace QtTapioca
{
	class Contact;
}

namespace Kopete
{
	class ChatSession;
	class MetaContact;
}
class TelepathyAccount;

using namespace QtTapioca;

class TelepathyContact : public Kopete::Contact
{
	Q_OBJECT
public:
	TelepathyContact(TelepathyAccount *account, const QString &contactId, Kopete::MetaContact *parent);
	~TelepathyContact();

	virtual bool isReachable();
	virtual void serialize(QMap< QString, QString >& serializedData, QMap< QString, QString >& addressBookData);
	
	virtual QList<KAction *> *customContextMenuActions();
	virtual Kopete::ChatSession *manager( CanCreateFlags canCreate = CannotCreate );

	void setInternalContact(QtTapioca::Contact *internalContact);

	/**
	 * @brief Reimplement Kopete::Contact::account() to cast to specific TelepathyAccount
	 * @return TelepathyAccount for this contact.
	 */
	TelepathyAccount *account();

	/**
	 * @brief Get the internal Tapioca contact.
	 * @return the internal Tapioca contact instance (a reference)
	 */
	QtTapioca::Contact *internalContact();

public slots:
	/**
	 * @brief Delete the contact on server and remove it from Kopete.
	 */
	virtual void deleteContact();

private slots:
	/**
	 * @brief Called when the contact has updated its presence information.
	 * @param contactInfo Refering ContactBase
	 * @param presence New presence.
	 * @param presenceMessage New presenceMessage, if any.
	 */
	void slotPresenceUpdated(ContactBase *contactInfo, ContactBase::Presence presence, const QString &presenceMessage);

private:
	class Private;
	Private *d;
};
#endif
