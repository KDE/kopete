/*
    bonjourprotocol.cpp - Kopete Bonjour Protocol

    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.u>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "bonjourprotocol.h"

#include <QList>
#include <kpluginfactory.h>
#include <kdebug.h>

#include <kopeteaccountmanager.h>
#include "bonjouraccount.h"
#include "bonjourcontact.h"
#include "bonjouraddcontactpage.h"
#include "bonjoureditaccountwidget.h"

K_PLUGIN_FACTORY(BonjourProtocolFactory, registerPlugin<BonjourProtocol>();
                 )
K_EXPORT_PLUGIN(BonjourProtocolFactory("kopete_bonjour"))

BonjourProtocol *BonjourProtocol::s_protocol = nullptr;

BonjourProtocol::BonjourProtocol(QObject *parent, const QVariantList & /*args*/)
    : Kopete::Protocol(parent)
    , bonjourOnline(Kopete::OnlineStatus::Online, 25, this, 0, QStringList(QString()),
                    i18n("Online"), i18n("O&nline"), Kopete::OnlineStatusManager::Online)
    , bonjourAway(Kopete::OnlineStatus::Away, 25, this, 1, QStringList(QStringLiteral("msn_away")),
                  i18nc("This Means the User is Away", "Away"),
                  i18nc("This Means the User is Away", "&Away"),
                  Kopete::OnlineStatusManager::Away)
    , bonjourOffline(Kopete::OnlineStatus::Offline, 25, this, 2, QStringList(QString()),
                     i18n("Offline"), i18n("O&ffline"), Kopete::OnlineStatusManager::Offline)
{
    qDebug()<<"Protocol Icon is: "<<pluginIcon();

    s_protocol = this;
}

BonjourProtocol::~BonjourProtocol()
{
}

Kopete::Contact *BonjourProtocol::deserializeContact(
    Kopete::MetaContact *metaContact, const QMap<QString, QString> &serializedData, const QMap<QString, QString> & /* addressBookData */)
{
    QString contactId = serializedData[ QStringLiteral("contactId") ];
    QString accountId = serializedData[ QStringLiteral("accountId") ];
    Kopete::Contact::NameType nameType
        = Kopete::Contact::nameTypeFromString(serializedData[ QStringLiteral("preferredNameType") ]);

    QList<Kopete::Account *> accounts = Kopete::AccountManager::self()->accounts(this);
    Kopete::Account *account = 0;
    foreach (Kopete::Account *acct, accounts) {
        if (acct->accountId() == accountId) {
            account = acct;
        }
    }

    if (!account) {
        qDebug() << "Account doesn't exist, skipping";
        return 0;
    }

    BonjourContact *contact = new BonjourContact(account, contactId, metaContact);
    contact->setPreferredNameType(nameType);
    return contact;
}

AddContactPage *BonjourProtocol::createAddContactWidget(QWidget *parent, Kopete::Account * /* account */)
{
    qDebug()<< "Creating Add Contact Page";
    return new BonjourAddContactPage(parent);
}

KopeteEditAccountWidget *BonjourProtocol::createEditAccountWidget(Kopete::Account *account, QWidget *parent)
{
    qDebug() << "Creating Edit Account Page";
    return new BonjourEditAccountWidget(parent, account);
}

Kopete::Account *BonjourProtocol::createNewAccount(const QString &accountId)
{
    return new BonjourAccount(this, accountId);
}

BonjourProtocol *BonjourProtocol::protocol()
{
    return s_protocol;
}

#include "bonjourprotocol.moc"
