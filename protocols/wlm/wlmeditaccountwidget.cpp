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
#include <kmessagebox.h>
#include <ktoolinvocation.h>

#include "kopeteuiglobal.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "ui_wlmaccountpreferences.h"
#include "wlmaccount.h"
#include "wlmprotocol.h"

WlmEditAccountWidget::WlmEditAccountWidget (QWidget * parent, Kopete::Account * account)
 : QWidget (parent), KopeteEditAccountWidget(account)
{
    m_preferencesWidget = new Ui::WlmAccountPreferences ();
    m_preferencesWidget->setupUi (this);
    m_preferencesWidget->mainTabWidget->setCurrentIndex(0);

    if ( account )
    {
        KConfigGroup * config = account->configGroup();
        WlmAccount* wlmAccount = static_cast<WlmAccount*>(account);

        m_preferencesWidget->m_passport->setText( wlmAccount->accountId() );
        m_preferencesWidget->m_password->load( &wlmAccount->password() );

        m_preferencesWidget->m_passport->setReadOnly( true );
        m_preferencesWidget->m_autologin->setChecked( account->excludeConnect()  );
        if ( wlmAccount->serverName() != "messenger.hotmail.com" || wlmAccount->serverPort() != 1863 || wlmAccount->useHttpMethod() )
            m_preferencesWidget->optionOverrideServer->setChecked( true );

        m_preferencesWidget->m_serverName->setText( wlmAccount->serverName() );
        m_preferencesWidget->m_serverPort->setValue( wlmAccount->serverPort() );
        m_preferencesWidget->optionUseHttpMethod->setChecked( wlmAccount->useHttpMethod() );
    }

    connect( m_preferencesWidget->buttonRegister, SIGNAL(clicked()), this, SLOT(slotOpenRegister()));

    QWidget::setTabOrder( m_preferencesWidget->m_passport, m_preferencesWidget->m_password->mRemembered );
    QWidget::setTabOrder( m_preferencesWidget->m_password->mRemembered, m_preferencesWidget->m_password->mPassword );
    QWidget::setTabOrder( m_preferencesWidget->m_password->mPassword, m_preferencesWidget->m_autologin );
}

WlmEditAccountWidget::~WlmEditAccountWidget ()
{
    delete m_preferencesWidget;
}

Kopete::Account * WlmEditAccountWidget::apply ()
{
    if ( !account() )
        setAccount( new WlmAccount( WlmProtocol::protocol (), m_preferencesWidget->m_passport->text () ) );

    KConfigGroup *config = account()->configGroup();

    account()->setExcludeConnect( m_preferencesWidget->m_autologin->isChecked() );
    m_preferencesWidget->m_password->save( &static_cast<WlmAccount *>(account())->password() );

    if (m_preferencesWidget->optionOverrideServer->isChecked() ) {
        config->writeEntry( "serverName", m_preferencesWidget->m_serverName->text().trimmed() );
        config->writeEntry( "serverPort", m_preferencesWidget->m_serverPort->value()  );
    }
    else {
        config->writeEntry( "serverName", "messenger.hotmail.com" );
        config->writeEntry( "serverPort", "1863" );
    }

    config->writeEntry( "useHttpMethod", m_preferencesWidget->optionUseHttpMethod->isChecked() );

    return account ();
}

bool WlmEditAccountWidget::validateData ()
{
    QString contactId = m_preferencesWidget->m_passport->text();
    if ( WlmProtocol::validContactId( contactId ) )
        return true;

    KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
                                   i18n( "<qt>You must enter a valid WLM passport.</qt>" ), i18n( "WLM Plugin" ) );
    return false;
}

void WlmEditAccountWidget::slotOpenRegister()
{
    KToolInvocation::invokeBrowser( "http://register.passport.net/"  );
}

#include "wlmeditaccountwidget.moc"
