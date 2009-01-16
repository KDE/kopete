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

// TelepathyQt4 includes
#include <QtTapioca/ContactBase>

class KAction;

namespace QtTapioca
{
	class Contact;
	class Avatar;
}

namespace Kopete
{
	class ChatSession;
	class MetaContact;
}
class TelepathyAccount;

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
	 * @param contactBase Referring ContactBase
	 * @param presence New presence.
	 * @param presenceMessage New presenceMessage, if any.
	 */
	void telepathyPresenceUpdated(QtTapioca::ContactBase *contactBase, QtTapioca::ContactBase::Presence presence, const QString &presenceMessage);

	/**
	 * @brief Called when contact has changed its alias.
	 * @param contactBase Referring ContactBase
	 * @param alias New alias
	 */
	void telepathyAliasChanged(QtTapioca::ContactBase *contactBase, const QString &alias);

	/**
	 * @brief Called when contact has changed its avatar.
	 * @param contact Referring ContactBase
	 * @param newToken New token for the avatar update
	 */
	void telepathyAvatarChanged(QtTapioca::ContactBase *contact, const QString &newToken);

	/**
	 * @brief Called when we got avatar from contact.
	 * @param contact Referring ContactBase
	 * @param avatar New avatar data
	 */
	void telepathyAvatarReceived(QtTapioca::ContactBase *contact, QtTapioca::Avatar *avatar);

	void actionAuthorize();
	void actionSubscribe();

private:
	class Private;
	Private *d;
};
#endif
