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

	void setContactList( QtTapioca::ContactList *contactList );
	/**
	 * @brief Load contact information from Telepathy into Kopete.
	 */
	void loadContacts();

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
