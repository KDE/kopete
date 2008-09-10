/*
    wlmeditaccountwidget.h - Kopete Wlm Protocol

    Copyright (c) 2008      by Tiago Salem Herrmann <tiagosh@gmail.com>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "wlmeditaccountwidget.h"

#include <QLayout>
#include <QLineEdit>
//Added by qt3to4:
#include <QVBoxLayout>
#include <kdebug.h>
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "ui_wlmaccountpreferences.h"
#include "wlmaccount.h"
#include "wlmprotocol.h"

WlmEditAccountWidget::WlmEditAccountWidget (QWidget * parent, Kopete::Account * account):QWidget (parent),
KopeteEditAccountWidget
(account)
{
    (new QVBoxLayout (this));
    kDebug (14210) << k_funcinfo;
    QWidget *
        w = new QWidget (this);
    layout ()->addWidget (w);
    m_preferencesWidget = new Ui::WlmAccountPreferences ();
    m_preferencesWidget->setupUi (w);
}

WlmEditAccountWidget::~WlmEditAccountWidget ()
{
}

Kopete::Account * WlmEditAccountWidget::apply ()
{
    QString
        accountName;
    if (m_preferencesWidget->m_acctName->text ().isEmpty ())
        accountName = "Wlm Account";
    else
        accountName = m_preferencesWidget->m_acctName->text ();

    if (account ())
        // FIXME: ? account()->setAccountLabel(accountName);
        account ()->myself ()->
            setProperty (Kopete::Global::Properties::self ()->nickName (),
                         accountName);
    else
        setAccount (new WlmAccount (WlmProtocol::protocol (), accountName));

    return account ();
}

bool
WlmEditAccountWidget::validateData ()
{
    //return !( m_preferencesWidget->m_acctName->text().isEmpty() );
    return true;
}

#include "wlmeditaccountwidget.moc"
