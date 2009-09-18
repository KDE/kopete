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
#include <QVariant>



/**
 * Encapsulates the generic actions associated with this protocol
 * @author Will Stephenson
 */
class TestbedProtocol : public Kopete::Protocol
{
	Q_OBJECT
public:
	TestbedProtocol(QObject *parent, const QVariantList &args);
    ~TestbedProtocol();
	/**
	 * Convert the serialised data back into a TestbedContact and add this
	 * to its Kopete::MetaContact
	 */
	virtual Kopete::Contact *deserializeContact(
			Kopete::MetaContact *metaContact,
			const QMap< QString, QString > & serializedData,
			const QMap< QString, QString > & addressBookData
		);
	/**
	 * Generate the widget needed to add TestbedContacts
	 */
	virtual AddContactPage * createAddContactWidget( QWidget *parent, Kopete::Account *account );
	/**
	 * Generate the widget needed to add/edit accounts for this protocol
	 */
	virtual KopeteEditAccountWidget * createEditAccountWidget( Kopete::Account *account, QWidget *parent );
	/**
	 * Generate a TestbedAccount
	 */
	virtual Kopete::Account * createNewAccount( const QString &accountId );
	/**
	 * Access the instance of this protocol
	 */
	static TestbedProtocol *protocol();
	/**
	 * Represents contacts that are Online
	 */
	const Kopete::OnlineStatus testbedOnline;
	/**
	 * Represents contacts that are Away
	 */
	const Kopete::OnlineStatus testbedAway;
	/**
	 * Represents contacts that are Busy
	 */
	const Kopete::OnlineStatus testbedBusy;
	/**
	 * Represents contacts that are Offline
	 */
	const Kopete::OnlineStatus testbedOffline;
protected:
	static TestbedProtocol *s_protocol;
};

#endif
