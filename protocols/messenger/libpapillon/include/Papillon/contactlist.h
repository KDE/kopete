/*
   contactlist.h - Windows Live Messenger Contact List

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
 */
#ifndef PAPILLONCONTACTLIST_H
#define PAPILLONCONTACTLIST_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <Papillon/Macros>
#include <Papillon/Enums>

class QDomDocument;

namespace Papillon 
{

class Client;
class Contact;
class FetchContactListJob;

/**
 * @class ContactList contactlist.h <Papillon/ContactList>
 * @brief Manage contact list.
 *
 * @author Michaël Larouche <larouche@kde.org>
*/
class PAPILLON_EXPORT ContactList : public QObject
{
	Q_OBJECT
public:
	/**
	 * @brief Create a new ContactList.
	 */
	ContactList(Client *client);
	/**
	 * d-tor
	 */
	~ContactList();

	/**
	 * @brief Get the Contact instance for the given contactId
	 *
	 * @param contactId can be the contact GUID or the passport ID.
	 * @return Contact instance if found or 0(null)
	 */
	Contact *contact(const QString &contactId);

	/**
	 * @brief Return the contacts on your contact list (aka forward list)
	 * @return List of Contact on forward list.
	 */
	QList<Papillon::Contact*> contacts() const;
	/**
	 * @brief Return the contacts on your allowed list.
	 *
	 * Allow list is the list of contacts that you allow to see your presence
	 * @return List of Contact on Allow list.
	 */
	QList<Papillon::Contact*> allowList() const;
	/**
	 * @brief Return the contacts on your block list.
	 *
	 * Block list of the list of contacts that you refuse to show your presence.
	 * @return List of Contact on Block list.
	 */
	QList<Papillon::Contact*> blockList() const;
	/**
	 * @brief Return the contacts on your reverse list.
	 *
	 * Reverse list is the lists of contacts that are subscribed to your presence.
	 * @return List of Contact on Reverse list.
	 */
	QList<Papillon::Contact*> reverseList() const;
	/**
	 * @brief Return the contacts on your pending list.
	 *
	 * Pending list is the list of contacts that added you on their contact list
	 * and waiting for your approval.
	 * @return List of Contact on Pending list.
	 */
	QList<Papillon::Contact*> pendingList() const;

public slots:
	/**
	 * @brief Start the fetching of contact list and address book.
	 * This method start an asynchronous task to fetch the contact list and
	 * the address book.
	 *
	 * Listen to contactListLoaded() signal to be notified of completion of this task.
	 */
	void load();

signals:
	/**
	 * Emitted when contact list and address book has been fetched successfully.
	 */
	void contactListLoaded();

private:
	/**
	 * @internal
	 * Get the current instance of Client.
	 * @return the current Client pointer.
	 */
	Client *client();

	/**
	 * @internal
	 * Create a new contact or return an existing contact.
	 * Used by Contact list jobs.
	 * @return Contact instance
	 */
	Contact *createContact(const QString &contactId);

private:
	class Private;
	Private *d;

	friend class FetchContactListJob;
};

}

#endif
