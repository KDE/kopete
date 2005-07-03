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
#include <qpushbutton.h>
#include <qlayout.h>
#include <kopeteaccount.h>
#include <kopetemetacontact.h>
#include <qlineedit.h>

#include "meanwhileprotocol.h"
#include "meanwhileaccount.h"
#include "meanwhileplugin.h"

MeanwhileAddContactPage::MeanwhileAddContactPage( 
                                QWidget* parent, 
                                Kopete::Account *_account)
        : AddContactPage(parent, 0L), theAccount(_account),
            theParent(parent)
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );
    theDialog = new MeanwhileAddContactBase(this);
    MeanwhileAccount *account = 
        static_cast<MeanwhileAccount *>(_account);
    if (account->infoPlugin->canProvideMeanwhileId())
    {
        QObject::connect(theDialog->btnFindUser, SIGNAL(clicked()),
                        SLOT(slotFindUser()));
    }
    else
        theDialog->btnFindUser->setDisabled(true);
    theDialog->show();
}

MeanwhileAddContactPage::~MeanwhileAddContactPage()
{
}

void MeanwhileAddContactPage::slotFindUser()
{
    MeanwhileAccount *account = 
        static_cast<MeanwhileAccount *>(theAccount);
    account->infoPlugin->getMeanwhileId(theParent,
                    theDialog->contactID);
}

bool MeanwhileAddContactPage::apply( 
                    Kopete::Account* a, 
                    Kopete::MetaContact* m )
{
    QString displayName = theDialog->contactID->text();
    MeanwhileAccount* myAccount = static_cast<MeanwhileAccount*>(a);
    return myAccount->addContact(displayName, m, Kopete::Account::ChangeKABC );
}

bool MeanwhileAddContactPage::validateData()
{
    return ! theDialog->contactID->text().isEmpty();
}

#include "meanwhileaddcontactpage.moc"
