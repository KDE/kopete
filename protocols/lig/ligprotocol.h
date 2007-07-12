/*
    ligprotocol.h - Kopete Lig Protocol

    Copyright (c) 2007      by Cláudio da Silveira Pinheiro	<taupter@gmail.com>
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

#ifndef LIGPROTOCOL_H
#define LIGPROTOCOL_H

#include <kopeteprotocol.h>

/**
	@author Cláudio da Silveira Pinheiro <taupter@gmail.com>
*/
class LigProtocol : public Kopete::Protocol
{
	Q_OBJECT
public:
	LigProtocol(QObject *parent, const char *name, const QStringList &args);
	~LigProtocol();

	/**
	 * Convert the serialised data back into a LigContact and add this
	 * to its Kopete::MetaContact
	 */
	virtual Kopete::Contact *deserializeContact(
			Kopete::MetaContact *metaContact,
			const QMap< QString, QString > & serializedData,
			const QMap< QString, QString > & addressBookData
		);
	/**
	 * Generate the widget needed to add LigContacts
	 */
	virtual AddContactPage * createAddContactWidget( QWidget *parent, Kopete::Account *account );
	/**
	 * Generate the widget needed to add/edit accounts for this protocol
	 */
	virtual KopeteEditAccountWidget * createEditAccountWidget( Kopete::Account *account, QWidget *parent );
	/**
	 * Generate a LigAccount
	 */
	virtual Kopete::Account * createNewAccount( const QString &accountId );
	/**
	 * Access the instance of this protocol
	 */
	static LigProtocol *protocol();
	/**
	 * Represents contacts that are Online
	 */
	const Kopete::OnlineStatus ligOnline;
	/**
	 * Represents contacts that are Away
	 */
	const Kopete::OnlineStatus ligAway;
	/**
	 * Represents contacts that are Offline
	 */
	const Kopete::OnlineStatus ligOffline;
protected:
	static LigProtocol *s_protocol;



};

#endif
