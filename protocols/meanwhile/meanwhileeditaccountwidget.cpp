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
#include <kdebug.h>
#include <kopeteaccount.h>
#include <kmessagebox.h>
#include <klocale.h>
#include "meanwhileprotocol.h"
#include "meanwhileserver.h"
#include "meanwhileaccount.h"
#include "meanwhileeditaccountwidget.h"

#include "log.h"

#define DEFAULT_SERVER "messaging.opensource.ibm.com"
#define DEFAULT_PORT 1533

MeanwhileEditAccountWidget::MeanwhileEditAccountWidget( 
                                QWidget* parent, 
                                Kopete::Account* theAccount,
                                MeanwhileProtocol *theProtocol)
    : MeanwhileEditAccountBase(parent),
      KopeteEditAccountWidget( theAccount )
{
    protocol = theProtocol;

    if (account())
    {
        mScreenName->setText(account()->accountId());
        mScreenName->setReadOnly(true); 
        mScreenName->setDisabled(true);
        if (account()->rememberPassword())
        {
            mRememberPassword->setChecked(true);
            mPassword->setText(account()->password());
        }
        mAutoConnect->setChecked(account()->autoLogin());
        MeanwhileAccount *myAccount = static_cast<MeanwhileAccount *>(account());
        mServerName->setText(myAccount->serverName());
        mServerPort->setValue(myAccount->serverPort());
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
        setAccount(
                new MeanwhileAccount( 
                        MeanwhileProtocol::protocol(), mScreenName->text()));

    MeanwhileAccount *myAccount = static_cast<MeanwhileAccount *>(account());

    myAccount->setAutoLogin(mAutoConnect->isChecked());

    if(mRememberPassword->isChecked())
        myAccount->setPassword(mPassword->text());
    else
        myAccount->setPassword(QString::null);

    myAccount->setServerName(mServerName->text());
    myAccount->setServerPort(mServerPort->value());

    return myAccount;
}

bool MeanwhileEditAccountWidget::validateData()
{
    if(mScreenName->text() == "")
    {   
        KMessageBox::queuedMessageBox(this, KMessageBox::Sorry,
            i18n("<qt>You must enter a valid screen name.</qt>"), 
            i18n("Meanwhile plugin"));
        return false;
    }
    if( (mRememberPassword->isChecked()) && (mPassword->text() == ""))
    {   
        KMessageBox::queuedMessageBox(this, KMessageBox::Sorry,
            i18n("<qt>You must deselect password remembering or enter a valid password.</qt>"), 
            i18n("Meanwhile plugin"));
        return false;
    }
    if (mServerName->text() == "")
    {
        KMessageBox::queuedMessageBox(this, KMessageBox::Sorry,
            i18n("<qt>You must enter the server's hostname/ip address.</qt>"), 
            i18n("Meanwhile plugin"));
        return false;
    }
    if (mServerPort->text() == 0)
    {
        KMessageBox::queuedMessageBox(this, KMessageBox::Sorry,
            i18n("<qt>0 is not a valid port number.</qt>"), 
            i18n("Meanwhile plugin"));
        return false;
    }
    return true;
}

void MeanwhileEditAccountWidget::slotSetServer2Default()
{
    mServerName->setText(DEFAULT_SERVER);
    mServerPort->setValue(DEFAULT_PORT);
}

#include "meanwhileeditaccountwidget.moc"
