/*
    kopeteblacklister.h - Kopete BlackLister

    Copyright (c) 2004     by Roie Kerstein        <sf_kersteinroie@bezeqint.net>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEBLACKLISTER_H
#define KOPETEBLACKLISTER_H

#include <QtCore/QObject>

namespace Kopete
{

class Contact;

/**
 * @brief Manages the list of blacklisted contacts for an account
 *
 * This class manages the list of contacts the user wishes
 * to ignore permanently. In order to use the this class, there is no need to
 * create an instance. Use the @ref Kopete::Account::blackLister() instead.
 *
 * Keep in mind that this class does not discard messages from blocked
 * users - It only manages the list. It is the up to the protocol to
 * check whether a user is blocked, and act accordingly. A protocol may
 * re-implement @ref Kopete::Account::block() and @ref Kopete::Account::unblock()
 * and use @ref Kopete::Account::blackLister() as a persistent list manager
 * only, or connect the signals @ref contactAdded() and @ref contactRemoved()
 * to its slots.
 *
 * @sa Kopete::Account::block() Kopete::Account::unblock()
 *
 * @author Roie Kerstein <sf_kersteinroie@bezeqint.net>
 */
class BlackLister : public QObject
{
	Q_OBJECT

public:
	/**
	 * Create an instance, and read the blacklist from disk if it exists.
	 * @param protocolId is the ID of the protocol owning accountId
	 * @param accountId is the ID of the owning Account.
	 * @param parent The QObject parent for this class.
	 * @param name The QObject name for this class.
	 */
	BlackLister( const QString &protocolId, const QString &accountId, QObject *parent = 0 );
	~BlackLister();

	/**
	 * \return @c true if @p contact is blocked, @c false otherwise.
	 */
	bool isBlocked( Contact *contact );

	/**
	 * \return @c true if the contact with ID @p contactId is blocked, @c false otherwise.
	 */
	bool isBlocked( const QString &contactId );

public slots:
	/**
	 * Add a contact to the blacklist.
	 *
	 * This function emits the @ref contactAdded() signal.
	 * @param contactId is the ID of the contact to be added to the list.
	 */
	void addContact( const QString &contactId );

	/**
	 * @overload
	 */
	void addContact( Contact *contact );

	/**
	 * \brief Remove a contact from the blacklist.
	 *
	 * Removes the contact from the blacklist.
	 * This function emits the @ref contactRemoved() signal.
	 * @param contact is the contact to be removed from the list.
	 */
	void removeContact( Contact *contact );

	/**
	 * @overload
	 */
	void removeContact( const QString &contactId );

signals:
	/**
	 * \brief A new contact has been added to the list
	 *
	 * Connect to this signal if you want to perform additional actions,
	 * and you prefer not to derive from this class.
	 */
	void contactAdded( const QString &contactId );

	/**
	 * \brief A contact has been removed from the list
	 *
	 * Connect to this signal if you want to perform additional actions,
	 * and you prefer not to derive from this class.
	 */
	void contactRemoved( const QString &contactId );

private:
	void saveToDisk();

	class Private;
	Private * const d;
};

}

#endif
