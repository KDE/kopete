/*
    meanwhileeditaccountwidget.cpp - edit an account

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
#include <kdebug.h>
#include <kopeteaccount.h>
#include <kopetepasswordwidget.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "meanwhileprotocol.h"
#include "meanwhileaccount.h"
#include "meanwhileeditaccountwidget.h"

#define DEFAULT_SERVER "messaging.opensource.ibm.com"
#define DEFAULT_PORT 1533

MeanwhileEditAccountWidget::MeanwhileEditAccountWidget(
                                QWidget* parent,
                                Kopete::Account* theAccount,
                                MeanwhileProtocol *theProtocol)
    : QWidget(parent),
      KopeteEditAccountWidget( theAccount )
{
    protocol = theProtocol;

    ui.setupUi(this);

    if (account())
    {
        ui.mScreenName->setText(account()->accountId());
        ui.mScreenName->setReadOnly(true);
        ui.mPasswordWidget->load(&static_cast<MeanwhileAccount*>(account())->password());
        ui.mAutoConnect->setChecked(account()->excludeConnect());
        MeanwhileAccount *myAccount = static_cast<MeanwhileAccount *>(account());
        ui.mServerName->setText(myAccount->getServerName());
        ui.mServerPort->setValue(myAccount->getServerPort());
    }
    else
    {
        slotSetServer2Default();
    }

    connect(ui.btnServerDefaults, SIGNAL(clicked()), SLOT(slotSetServer2Default()));

 // ### TODO?   show();
}

MeanwhileEditAccountWidget::~MeanwhileEditAccountWidget()
{
}


Kopete::Account* MeanwhileEditAccountWidget::apply()
{
    if(!account())
        setAccount(new MeanwhileAccount(protocol, ui.mScreenName->text()));

    MeanwhileAccount *myAccount = static_cast<MeanwhileAccount *>(account());

    myAccount->setExcludeConnect(ui.mAutoConnect->isChecked());

    ui.mPasswordWidget->save(&static_cast<MeanwhileAccount*>(account())->password());

    myAccount->setServerName(ui.mServerName->text());
    myAccount->setServerPort(ui.mServerPort->value());

    return myAccount;
}

bool MeanwhileEditAccountWidget::validateData()
{
    if (ui.mScreenName->text().isEmpty())
    {
        KMessageBox::queuedMessageBox(this, KMessageBox::Sorry,
            i18n("<qt>You must enter a valid screen name.</qt>"), 
            i18n("Meanwhile Plugin"));
        return false;
    }
    if (!ui.mPasswordWidget->validate())
    {
        KMessageBox::queuedMessageBox(this, KMessageBox::Sorry,
            i18n("<qt>You must deselect password remembering or enter a valid password.</qt>"), 
            i18n("Meanwhile Plugin"));
        return false;
    }
    if (ui.mServerName->text().isEmpty())
    {
        KMessageBox::queuedMessageBox(this, KMessageBox::Sorry,
            i18n("<qt>You must enter the server's hostname/ip address.</qt>"), 
            i18n("Meanwhile Plugin"));
        return false;
    }
    if (ui.mServerPort->text() == 0)
    {
        KMessageBox::queuedMessageBox(this, KMessageBox::Sorry,
            i18n("<qt>0 is not a valid port number.</qt>"), 
            i18n("Meanwhile Plugin"));
        return false;
    }
    return true;
}

void MeanwhileEditAccountWidget::slotSetServer2Default()
{
    ui.mServerName->setText(DEFAULT_SERVER);
    ui.mServerPort->setValue(DEFAULT_PORT);
}

#include "meanwhileeditaccountwidget.moc"
