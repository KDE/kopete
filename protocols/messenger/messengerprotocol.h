/*
 * messengerprotocol.h - Windows Live Messenger Kopete protocol definition.
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 * 
 * Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>
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
#ifndef MESSENGERPROTOCOL_H
#define MESSENGERPROTOCOL_H

#include <QVariant>;

#include <kopeteprotocol.h>

#include <kopete_export.h>

namespace Kopete
{
	class Account;
}

class AddContactPage;
class KopeteEditAccountWidget;

/**
 * 
 * @author Michaël Larouche <larouche@kde.org>
 */
class MESSENGER_EXPORT MessengerProtocol : public Kopete::Protocol
{
public:
	MessengerProtocol(QObject *parent, const QVariantList &args);
	
	virtual Kopete::Account *createNewAccount(const QString &accountId);
	virtual AddContactPage *createAddContactWidget(QWidget *parent, Kopete::Account *account);
	virtual KopeteEditAccountWidget * createEditAccountWidget(Kopete::Account *account, QWidget *parent);

	static MessengerProtocol *protocol();

	virtual Kopete::Contact *deserializeContact( Kopete::MetaContact *metaContact,
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData );
private:
	static MessengerProtocol *s_self;
};
#endif
