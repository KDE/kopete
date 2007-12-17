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
#include <qlayout.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <kdebug.h>
#include <kopeteaccount.h>
#include <kopetepasswordwidget.h>
#include <kmessagebox.h>
#include <klocale.h>
#include "meanwhileprotocol.h"
#include "meanwhileaccount.h"
#include "meanwhileeditaccountwidget.h"
#include "meanwhilesession.h"

#define DEFAULT_SERVER "messaging.opensource.ibm.com"
#define DEFAULT_PORT 1533

void MeanwhileEditAccountWidget::setupClientList()
{
    const struct MeanwhileClientID *id;
    int i = 0;

    for (id = MeanwhileSession::getClientIDs(); id->name; id++, i++) {
        QString name = QString("%1 (0x%2)")
                           .arg(QString(id->name))
                           .arg(id->id, 0, 16);

        mClientID->insertItem(name, i);

        if (id->id == mwLogin_MEANWHILE)
            mClientID->setCurrentItem(i);
    }
}

void MeanwhileEditAccountWidget::selectClientListItem(int selectedID)
{
    const struct MeanwhileClientID *id;
    int i = 0;

    for (id = MeanwhileSession::getClientIDs(); id->name; id++, i++) {
        if (id->id == selectedID) {
            mClientID->setCurrentItem(i);
            break;
        }
    }
}

MeanwhileEditAccountWidget::MeanwhileEditAccountWidget( 
                                QWidget* parent, 
                                Kopete::Account* theAccount,
                                MeanwhileProtocol *theProtocol)
    : MeanwhileEditAccountBase(parent),
      KopeteEditAccountWidget( theAccount )
{
    protocol = theProtocol;

    /* setup client identifier combo box */
    setupClientList();

    if (account())
    {
        int clientID, verMajor, verMinor;
        bool useCustomID;

        mScreenName->setText(account()->accountId());
        mScreenName->setReadOnly(true); 
        mScreenName->setDisabled(true);
        mPasswordWidget->load(&static_cast<MeanwhileAccount*>(account())->password());
        mAutoConnect->setChecked(account()->excludeConnect());

        MeanwhileAccount *myAccount = static_cast<MeanwhileAccount *>(account());
        useCustomID = myAccount->getClientIDParams(&clientID,
                &verMajor, &verMinor);

        mServerName->setText(myAccount->getServerName());
        mServerPort->setValue(myAccount->getServerPort());

        if (useCustomID) {
            selectClientListItem(clientID);
            mClientVersionMajor->setValue(verMajor);
            mClientVersionMinor->setValue(verMinor);
            chkCustomClientID->setChecked(true);
        }

    }
    else
    {
        slotSetServer2Default();
    }

    QObject::connect(btnServerDefaults, SIGNAL(clicked()),
            SLOT(slotSetServer2Default()));

    show();
}

MeanwhileEditAccountWidget::~MeanwhileEditAccountWidget()
{
}


Kopete::Account* MeanwhileEditAccountWidget::apply()
{
    if(!account())
        setAccount(new MeanwhileAccount(protocol, mScreenName->text()));

    MeanwhileAccount *myAccount = static_cast<MeanwhileAccount *>(account());

    myAccount->setExcludeConnect(mAutoConnect->isChecked());

    mPasswordWidget->save(&static_cast<MeanwhileAccount*>(account())->password());

    myAccount->setServerName(mServerName->text());
    myAccount->setServerPort(mServerPort->value());

    if (chkCustomClientID->isChecked()) {
        const struct MeanwhileClientID *ids = MeanwhileSession::getClientIDs();
        myAccount->setClientID(ids[mClientID->currentItem()].id,
                mClientVersionMajor->value(),
                mClientVersionMinor->value());
    } else {
        myAccount->resetClientID();
    }

    return myAccount;
}

bool MeanwhileEditAccountWidget::validateData()
{
    if(mScreenName->text().isEmpty())
    {
        KMessageBox::queuedMessageBox(this, KMessageBox::Sorry,
            i18n("<qt>You must enter a valid screen name.</qt>"), 
            i18n("Meanwhile Plugin"));
        return false;
    }
    if( !mPasswordWidget->validate() )
    {
        KMessageBox::queuedMessageBox(this, KMessageBox::Sorry,
            i18n("<qt>You must deselect password remembering or enter a valid password.</qt>"), 
            i18n("Meanwhile Plugin"));
        return false;
    }
    if (mServerName->text().isEmpty())
    {
        KMessageBox::queuedMessageBox(this, KMessageBox::Sorry,
            i18n("<qt>You must enter the server's hostname/ip address.</qt>"), 
            i18n("Meanwhile Plugin"));
        return false;
    }
    if (mServerPort->text() == 0)
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

    mServerName->setText(DEFAULT_SERVER);
    mServerPort->setValue(DEFAULT_PORT);
    chkCustomClientID->setChecked(false);
    selectClientListItem(clientID);
    mClientVersionMajor->setValue(verMajor);
    mClientVersionMinor->setValue(verMinor);
}

#include "meanwhileeditaccountwidget.moc"
