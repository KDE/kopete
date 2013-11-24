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
#include "meanwhileeditaccountwidget.h"

#include <kdebug.h>
#include <kopeteaccount.h>
#include <kopetepasswordwidget.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "meanwhileprotocol.h"
#include "meanwhileaccount.h"
#include "meanwhilesession.h"

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

    /* setup client identifier combo box */
    setupClientList();

    if (account())
    {
	int clientID, verMajor, verMinor;
	bool useCustomID;

        ui.mScreenName->setText(account()->accountId());
        ui.mScreenName->setReadOnly(true);
        ui.mPasswordWidget->load(&static_cast<MeanwhileAccount*>(account())->password());
        ui.mAutoConnect->setChecked(account()->excludeConnect());
        MeanwhileAccount *myAccount = static_cast<MeanwhileAccount *>(account());

        useCustomID = myAccount->getClientIDParams(&clientID,
                &verMajor, &verMinor);

        ui.mServerName->setText(myAccount->getServerName());
        ui.mServerPort->setValue(myAccount->getServerPort());

        if (useCustomID) {
            selectClientListItem(clientID);
            ui.mClientVersionMajor->setValue(verMajor);
            ui.mClientVersionMinor->setValue(verMinor);
            ui.chkCustomClientID->setChecked(true);
        }
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

void MeanwhileEditAccountWidget::setupClientList()
{
    const struct MeanwhileClientID *id;
    int i = 0;

    for (id = MeanwhileSession::getClientIDs(); id->name; id++, i++) {
        QString name = QString("%1 (0x%2)")
                           .arg(QString(id->name))
                           .arg(id->id, 0, 16);

	mwDebug() << "name: " << name << endl;

        ui.mClientID->insertItem(i, name);

        if (id->id == mwLogin_MEANWHILE)
            ui.mClientID->setCurrentIndex(i);
    }
}

void MeanwhileEditAccountWidget::selectClientListItem(int selectedID)
{
    const struct MeanwhileClientID *id;
    int i = 0;

    for (id = MeanwhileSession::getClientIDs(); id->name; id++, i++) {
        if (id->id == selectedID) {
            ui.mClientID->setCurrentIndex(i);
            break;
        }
    }
}


Kopete::Account* MeanwhileEditAccountWidget::apply()
{
    if(!account())
        setAccount(new MeanwhileAccount(protocol, ui.mScreenName->text()));

    MeanwhileAccount *myAccount = static_cast<MeanwhileAccount *>(account());

    myAccount->setExcludeConnect(ui.mAutoConnect->isChecked());

    ui.mPasswordWidget->save(&static_cast<MeanwhileAccount*>(account())->password());

    myAccount->setServerName(ui.mServerName->text().trimmed());
    myAccount->setServerPort(ui.mServerPort->value());

    if (ui.chkCustomClientID->isChecked()) {
        const struct MeanwhileClientID *ids = MeanwhileSession::getClientIDs();
        myAccount->setClientID(ids[ui.mClientID->currentIndex()].id,
	ui.mClientVersionMajor->value(),
	ui.mClientVersionMinor->value());
    } else {
        myAccount->resetClientID();
    }


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
    int clientID, verMajor, verMinor;

    MeanwhileSession::getDefaultClientIDParams(&clientID,
            &verMajor, &verMinor);

    ui.mServerName->setText(DEFAULT_SERVER);
    ui.mServerPort->setValue(DEFAULT_PORT);
    ui.chkCustomClientID->setChecked(false);
    selectClientListItem(clientID);
    ui.mClientVersionMajor->setValue(verMajor);
    ui.mClientVersionMinor->setValue(verMinor);
}

#include "meanwhileeditaccountwidget.moc"
