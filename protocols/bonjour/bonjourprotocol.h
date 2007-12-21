/*
    bonjourprotocol.h - Kopete Bonjour Protocol

    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
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
 * Encapsulates the generic actions associated with this protocol
 * @author Will Stephenson
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
	 * Generate the widget needed to add BonjourContacts
	 */
	virtual AddContactPage * createAddContactWidget( QWidget *parent, Kopete::Account *account );
	/**
	 * Generate the widget needed to add/edit accounts for this protocol
	 */
	virtual KopeteEditAccountWidget * createEditAccountWidget( Kopete::Account *account, QWidget *parent );
	/**
	 * Generate a BonjourAccount
	 */
	virtual Kopete::Account * createNewAccount( const QString &accountId );
	/**
	 * Access the instance of this protocol
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
