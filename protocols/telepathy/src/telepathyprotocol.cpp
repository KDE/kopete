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

#include "telepathyprotocol.h"

#include "ui/telepathyaddcontactpage.h"
#include "ui/telepathyeditaccountwidget.h"

#include <telepathyaccount.h>
#include <telepathycontact.h>
#include <telepathyprotocolinternal.h>

#include <kopeteaccountmanager.h>

#include <KDebug>
#include <KGenericFactory>

K_PLUGIN_FACTORY(TelepathyProtocolFactory, registerPlugin<TelepathyProtocol>();)
K_EXPORT_PLUGIN(TelepathyProtocolFactory("kopete_telepathy"))

TelepathyProtocol *TelepathyProtocol::s_self = 0;

TelepathyProtocol::TelepathyProtocol(QObject *parent, const QVariantList &args)
 : Kopete::Protocol(TelepathyProtocolFactory::componentData(), parent)
{
    kDebug();

    Q_UNUSED(args);

    s_self = this;

    // Construct the singleton instance of the TelepathyProtocolInternal.
    // Wow, this is the craziest hack I've ever written! (And that's really saying something...)
    new TelepathyProtocolInternal(qobject_cast<Kopete::Protocol*>(this));

    addAddressBookField("messaging/telepathy", Kopete::Plugin::MakeIndexField);
}

TelepathyProtocol *TelepathyProtocol::protocol()
{
    return s_self;
}

Kopete::Account *TelepathyProtocol::createNewAccount(const QString &accountId)
{
    kDebug();

    return new TelepathyAccount(this, accountId);
}

AddContactPage *TelepathyProtocol::createAddContactWidget(QWidget *parent,
                                                                  Kopete::Account *account)
{
    kDebug();

    Q_UNUSED(account);

    return new TelepathyAddContactPage(parent); // FIXME: KDE Bug 205771
}

KopeteEditAccountWidget *TelepathyProtocol::createEditAccountWidget(Kopete::Account *account,
                                                                    QWidget *parent)
{
    kDebug();

    return new TelepathyEditAccountWidget(account, parent);
}

Kopete::Contact *TelepathyProtocol::deserializeContact(Kopete::MetaContact *metaContact,
                                                       const QMap<QString, QString> &serializedData,
                                                       const QMap<QString, QString> &addressBookData)
{
    kDebug();

    // We don't do any proper address book integration at the moment.
    Q_UNUSED(addressBookData);

    // Get the contact ID and account ID from the serialized Contact Data.
    QString contactId = serializedData["contactId"];
    QString accountId = serializedData["accountId"];

    // Get the Account this contact belongs to from the Kopete Account Manager.
    QList<Kopete::Account*> accounts = Kopete::AccountManager::self()->accounts(this);

    Q_FOREACH (Kopete::Account *account, accounts) {
        if (account->accountId() == accountId) {
            return new TelepathyContact(qobject_cast<TelepathyAccount*>(account),
                                        contactId,
                                        metaContact);
        }
    }

    return 0;
}


#include "telepathyprotocol.moc"

