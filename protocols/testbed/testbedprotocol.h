/*
    testbedprotocol.h - Kopete Testbed Protocol

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

#ifndef TESTBEDPROTOCOL_H
#define TESTBEDPROTOCOL_H

#include <kopeteprotocol.h>


/**
 * Encapsulates the generic actions associated with this protocol
 * @author Will Stephenson
 */
class TestbedProtocol : public KopeteProtocol
{
	Q_OBJECT
public:
	TestbedProtocol(QObject *parent, const char *name, const QStringList &args);
    ~TestbedProtocol();
	/**
	 * Convert the serialised data back into a TestbedContact and add this
	 * to its KopeteMetaContact
	 */
	virtual KopeteContact *deserializeContact(
			KopeteMetaContact *metaContact,
			const QMap< QString, QString > & serializedData,
			const QMap< QString, QString > & addressBookData
		);
	/**
	 * Generate the widget needed to add TestbedContacts
	 */
	virtual AddContactPage * createAddContactWidget( QWidget *parent, KopeteAccount *account );
	/**
	 * Generate the widget needed to add/edit accounts for this protocol
	 */
	virtual KopeteEditAccountWidget * createEditAccountWidget( KopeteAccount *account, QWidget *parent );
	/**
	 * Generate a TestbedAccount
	 */
	virtual KopeteAccount * createNewAccount( const QString &accountId );
	/**
	 * Access the instance of this protocol
	 */
	static TestbedProtocol *protocol();
	/**
	 * Represents contacts that are Online
	 */
	const KopeteOnlineStatus testbedOnline;
	/**
	 * Represents contacts that are Away
	 */
	const KopeteOnlineStatus testbedAway;
	/**
	 * Represents contacts that are Offline
	 */
	const KopeteOnlineStatus testbedOffline;
protected:
	static TestbedProtocol *s_protocol;
};

#endif
