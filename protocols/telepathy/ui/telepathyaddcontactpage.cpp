/*
 * telepathyaddcontactpage.cpp - Telepathy Add Contact Page
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 *
 * Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>
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
#include "ui/telepathyaddcontactpage.h"

#include "ui_telepathyaddcontactpage.h"

#include "telepathyprotocol.h"

#include <telepathyaccount.h>
#include <telepathycontact.h>
#include <telepathycontactmanager.h>

#include <kopetecontactlist.h>
#include <kopetemetacontact.h>
#include <kopeteuiglobal.h>

#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>

#include <TelepathyQt4/Account>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingContacts>
#include <TelepathyQt4/PendingOperation>

class TelepathyAddContactPage::Private
{
public:
    Ui::TelepathyAddContactPage mainUi;
};

TelepathyAddContactPage::TelepathyAddContactPage(QWidget *parent)
        : AddContactPage(parent), d(new Private)
{
    d->mainUi.setupUi(this);
    d->mainUi.textUserId->setFocus();
}

TelepathyAddContactPage::~TelepathyAddContactPage()
{
    delete d;
}

bool TelepathyAddContactPage::validateData()
{
    // Nothing to valid for now
    return true;
}

bool TelepathyAddContactPage::apply(Kopete::Account *account, Kopete::MetaContact *parentMetaContact)
{
    if (!account->isConnected()) {
        KMessageBox::error(this, i18n("You must be connected to add a contact."), i18n("Telepathy plugin"));
        return false;
    }
    TelepathyAccount *tAccount = static_cast<TelepathyAccount*>(account);

    Tp::ConnectionPtr connection = tAccount->account()->connection();

    if (!connection) {
        KMessageBox::error(this, i18n("You must be connected to add a contact."), i18n("Telepathy plugin"));
        kWarning() << "TelepathyAccount->account()->connection() is null.";
        return false;
    }

    if (connection->status() != Tp::ConnectionStatusConnected) {
        KMessageBox::error(this, i18n("You must be connected to add a contact."), i18n("Telepathy plugin"));
        kWarning() << "connection.status() is not Connected.";
        return false;
    }

    // Get new id.
    QString newId = d->mainUi.textUserId->text();

    kDebug() << "Starting to apply adding contact" << newId;

    QObject::connect(connection->contactManager()->contactsForIdentifiers(QStringList() << newId),
            SIGNAL(finished(Tp::PendingOperation*)),
            new TelepathyAddContactAsyncContext(tAccount, parentMetaContact),
            SLOT(normalizedContactFetched(Tp::PendingOperation*)));

    return true;
}

TelepathyAddContactAsyncContext::TelepathyAddContactAsyncContext(TelepathyAccount *account,
        Kopete::MetaContact *parentMetaContact) : account(account), parentMetaContact(parentMetaContact)
{
}

void TelepathyAddContactAsyncContext::normalizedContactFetched(Tp::PendingOperation *op)
{
    Tp::PendingContacts *contacts = qobject_cast<Tp::PendingContacts *>(op);

    if (!account || !parentMetaContact) {
        kDebug() << "The account or meta contact went away, shutting down? Not doing anything";
        deleteLater();
        return;
    }

    if (contacts && contacts->isValid() && !contacts->contacts().isEmpty()) {
        QString normalizedId = contacts->contacts()[0]->id();

        kDebug() << "Success, adding contact - normalized to" << normalizedId;

        // TODO: reuse the contact fetched here instead of fetching a new one? (might get cached
        // in TpQt4 anyway)

        account->addContact(normalizedId, parentMetaContact);
        account->addNewContact(normalizedId);
    } else {
        if (!contacts->invalidIdentifiers().isEmpty()) {
            kDebug() << "The identifier was invalid";

            KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
                i18n("The contact ID was not valid. Check the contact ID and try again."),
                i18n("Telepathy Protocol"));
        } else {
            kDebug() << "Failure: " << op->errorName() << op->errorMessage();
        }

        // Check if KopeteContactListView::addContact only created the metacontact for us, in which
        // case we should remove it (as would've happened if we returned false synchronously from
        // apply())
        if (parentMetaContact->contacts().isEmpty()) {
            kDebug() << "  Deleting supplied empty metacontact";
            Kopete::ContactList::self()->removeMetaContact(parentMetaContact);
        }
    }

    deleteLater();
}

#include "telepathyaddcontactpage.moc"

