/*
 * telepathycontactmanager.h - Telepathy Contact Manager
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
#ifndef TELEPATHY_CONTACTMANAGER_H
#define TELEPATHY_CONTACTMANAGER_H

#include <QtCore/QObject>

namespace QtTapioca
{
	class ContactList;
	class Contact;
}

class TelepathyAccount;
class TelepathyContact;
/**
 * @brief Manager for Telepathy contact list
 *
 * Manage all contact operations.
 * @author Michaël Larouche <michael.larouche@kdemail.net>
 */
class TelepathyContactManager : public QObject
{
	Q_OBJECT
public:
	TelepathyContactManager(TelepathyAccount *account);
	~TelepathyContactManager();

	/**
	 * @brief Add a new contact to the remote contact list
	 *
	 * @param contactId Contact id (ex: user@jabber.org) to add
	 * @return Internal Contact instance or 0 if operation failed.
	 */
	QtTapioca::Contact *addContact(const QString &contactId);

	/**
	 * @brief Remove the specified contact from the contact list
	 *
	 * This method delete the contact. No need to do it yourself.
	 *
	 * @param contact TelepathyContact instance to be removed from contact list.
	 */
	void removeContact(TelepathyContact *contact);

	/**
	 * @brief Set the Tapioca contact list object needed for
	 * all contact list management tasks.
	 *
	 * @param contactList QtTapioca contact list object
	 */
	void setContactList( QtTapioca::ContactList *contactList );

	/**
	 * @brief Load contact information from Telepathy into Kopete.
	 */
	void loadContacts();

private slots:
	void telepathyAuthorizationRequired(QtTapioca::Contact *newContact);
	void telepathySubscriptionAccepted(QtTapioca::Contact *contact);

private:
	TelepathyAccount *account();
	QtTapioca::ContactList *contactList();
	/**
	 * @brief Create a contact if required.
	 */
	void createContact(QtTapioca::Contact *telepathyContact);

private:
	class Private;
	Private *d;
};

#endif
