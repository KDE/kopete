/*
    bonjourprotocol.h - Kopete Bonjour Protocol

    Copyright (c) 2007      by Tejas Dinkar          <tejas@gja.in>
    Copyright (c) 2003      by Will Stephenson	     <will@stevello.free-online.co.uk>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef BONJOURPROTOCOL_H
#define BONJOURPROTOCOL_H

#include <kopeteprotocol.h>


/**
 * @brief This Represents the Bonjour Protocol
 *
 * Encapsulates the generic actions associated with this protocol
 * Usually, there is only a single instance of this class at any time
 *
 * @author Tejas Dinkar <tejas\@gja.in>
 */
class BonjourProtocol : public Kopete::Protocol
{
	Q_OBJECT
public:
	BonjourProtocol(QObject *parent, const QStringList &args);
    ~BonjourProtocol();
	/**
	 * Convert the serialised data back into a BonjourContact and add this
	 * to its Kopete::MetaContact
	 */
	virtual Kopete::Contact *deserializeContact(
			Kopete::MetaContact *metaContact,
			const QMap< QString, QString > & serializedData,
			const QMap< QString, QString > & addressBookData
		);

	/**
	 * @brief Generate an Add Contact Page (not actually useful)
	 *
	 * As you cannot actually add an contact, this basically brings up an ugly message
	 *
	 * @param account is the account to add contact to
	 * @param parent The parent of the 'to be returned' widget
	 *
	 * @return The Add Contact Page Widget
	 */
	virtual AddContactPage * createAddContactWidget( QWidget *parent, Kopete::Account *account );

	/**
	 * @brief Generate an Edit Account Page
	 *
	 * Generate the widget needed to add/edit accounts for this protocol
	 *
	 * @param account is the account to edit. If @c NULL, a new account is made
	 * @param parent The parent of the 'to be returned' widget
	 *
	 * @return The Edit Account Page Widget
	 */
	virtual KopeteEditAccountWidget * createEditAccountWidget( Kopete::Account *account, QWidget *parent );

	/**
	 * @brief Create a New Account
	 *
	 * This will Generate a BonjourAccount
	 * @param accountId A Unique String to identify the Account
	 * @return The Newly Created Account
	 */
	virtual Kopete::Account * createNewAccount( const QString &accountId );

	/**
	 * @brief Access the instance of this protocol
	 * @return The Instance of this protocol
	 */
	static BonjourProtocol *protocol();

	/**
	 * Represents contacts that are Online
	 */
	const Kopete::OnlineStatus bonjourOnline;
	/**
	 * Represents contacts that are Away
	 */
	const Kopete::OnlineStatus bonjourAway;
	/**
	 * Represents contacts that are Offline
	 */
	const Kopete::OnlineStatus bonjourOffline;
protected:
	static BonjourProtocol *s_protocol;
};

#endif
