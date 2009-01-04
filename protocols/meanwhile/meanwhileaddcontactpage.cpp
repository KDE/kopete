/*
    meanwhileaddcontactpage.cpp - add a contact

    Copyright (c) 2003-2004 by Sivaram Gottimukkala  <suppandi@gmail.com>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "meanwhileaddcontactpage.h"
#include <kopeteaccount.h>
#include <kopetemetacontact.h>

#include "meanwhileprotocol.h"
#include "meanwhileaccount.h"
#include "meanwhileplugin.h"

MeanwhileAddContactPage::MeanwhileAddContactPage(
                                QWidget* parent,
                                Kopete::Account *_account)
        : AddContactPage(parent), theAccount(_account),
          theParent(parent)
{
    ui.setupUi(this);

    MeanwhileAccount *account = static_cast<MeanwhileAccount *>(_account);
    if (account->infoPlugin->canProvideMeanwhileId()) {
        connect(ui.btnFindUser, SIGNAL(clicked()), SLOT(slotFindUser()));
    } else {
        ui.btnFindUser->setDisabled(true);
    }
    ui.contactID->setFocus();
}

MeanwhileAddContactPage::~MeanwhileAddContactPage()
{
}

void MeanwhileAddContactPage::slotFindUser()
{
    MeanwhileAccount *account = static_cast<MeanwhileAccount *>(theAccount);
    account->infoPlugin->getMeanwhileId(theParent, ui.contactID);
}

bool MeanwhileAddContactPage::apply(
                    Kopete::Account* a,
                    Kopete::MetaContact* m )
{
    QString displayName = ui.contactID->text();
    MeanwhileAccount* myAccount = static_cast<MeanwhileAccount*>(a);
    return myAccount->addContact(displayName, m, Kopete::Account::ChangeKABC );
}

bool MeanwhileAddContactPage::validateData()
{
    return !ui.contactID->text().isEmpty();
}

#include "meanwhileaddcontactpage.moc"
