/*
 * This file is part of Kopete
 *
 * Copyright (C) 2009 Collabora Ltd. <info@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef KOPETE_PROTOCOL_TELEPATHY_TELEPATHYPROTOCOL_H
#define KOPETE_PROTOCOL_TELEPATHY_TELEPATHYPROTOCOL_H

#include <kopeteprotocol.h>

#include <QtCore/QVariantList>

/**
 * @brief This class is the TelepathyProtocol class which is exported in the Telepathy Protocol
 *        plugin only and is not present in the shared library. It contains only those methods which
 *        are not needed by the library to function, with the rest of what would be here in a normal
 *        Kopete Protocol Plugin located in telepathyprotocolinternal.h|cpp.
 */
class KOPETE_EXPORT TelepathyProtocol : public Kopete::Protocol
{
    Q_OBJECT

public:
    TelepathyProtocol(QObject *parent, const QVariantList &args);

    static TelepathyProtocol *protocol();

    /* Reimplemented virtual members of Kopete::Protocol. */
    virtual Kopete::Account *createNewAccount(const QString &accountId);

    virtual AddContactPage *createAddContactWidget(QWidget *parent, Kopete::Account *account);

    virtual KopeteEditAccountWidget *createEditAccountWidget(Kopete::Account *account,
                                                             QWidget *parent);

    virtual Kopete::Contact *deserializeContact(Kopete::MetaContact *metaContact,
                                                const QMap<QString, QString> &serializedData,
                                                const QMap<QString, QString> &addressBookData);

private:
    static TelepathyProtocol *s_self;

};


#endif  // Header guard

