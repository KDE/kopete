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
#include <KopeteTelepathy/ui/telepathyaddcontactpage.h>

#include "ui_telepathyaddcontactpage.h"

#include <KopeteTelepathy/telepathyaccount.h>
#include <KopeteTelepathy/telepathycontact.h>
#include <KopeteTelepathy/telepathycontactmanager.h>
#include <KopeteTelepathy/telepathyprotocol.h>

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
        kWarning(TELEPATHY_DEBUG_AREA) << "TelepathyAccount->account()->connection() is null.";
        return false;
    }

    if (connection->status() != Tp::ConnectionStatusConnected) {
        KMessageBox::error(this, i18n("You must be connected to add a contact."), i18n("Telepathy plugin"));
        kWarning(TELEPATHY_DEBUG_AREA) << "connection.status() is not Connected.";
        return false;
    }

    // Get new id.
    QString newId = d->mainUi.textUserId->text();
    tAccount->addNewContact(newId);
    tAccount->addContact(newId, parentMetaContact);

    return true;
}


#include "telepathyaddcontactpage.moc"

