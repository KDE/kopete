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
#include <QVBoxLayout>
#include <QSet>

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
        if ( wlmAccount->serverName() != "messenger.hotmail.com" || wlmAccount->serverPort() != 1863 )
            m_preferencesWidget->optionOverrideServer->setChecked( true );

        m_preferencesWidget->m_serverName->setText( wlmAccount->serverName() );
        m_preferencesWidget->m_serverPort->setValue( wlmAccount->serverPort() );

        bool connected = account->isConnected();
        if ( connected )
            m_preferencesWidget->m_warning->hide();

        m_preferencesWidget->m_allowButton->setEnabled( connected );
        m_preferencesWidget->m_blockButton->setEnabled( connected );

        m_preferencesWidget->m_allowButton->setIcon( KIcon( "arrow-left" ) );
        m_preferencesWidget->m_blockButton->setIcon( KIcon( "arrow-right" ) );

        foreach ( QString contact, wlmAccount->allowList() )
            m_preferencesWidget->m_AL->addItem( contact );

        foreach ( QString contact, wlmAccount->blockList() )
            m_preferencesWidget->m_BL->addItem( contact );
    }

    connect( m_preferencesWidget->m_allowButton, SIGNAL(clicked()), this, SLOT(slotAllow()) );
    connect( m_preferencesWidget->m_blockButton, SIGNAL(clicked()), this, SLOT(slotBlock()) );
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
    WlmAccount* wlmAccount = static_cast<WlmAccount*>(account());

    account()->setExcludeConnect( m_preferencesWidget->m_autologin->isChecked() );
    m_preferencesWidget->m_password->save( &wlmAccount->password() );

    if (m_preferencesWidget->optionOverrideServer->isChecked() ) {
        config->writeEntry( "serverName", m_preferencesWidget->m_serverName->text().trimmed() );
        config->writeEntry( "serverPort", m_preferencesWidget->m_serverPort->value()  );
    }
    else {
        config->writeEntry( "serverName", "messenger.hotmail.com" );
        config->writeEntry( "serverPort", "1863" );
    }

    if ( wlmAccount->isConnected() )
    {
        QSet<QString> allowList = wlmAccount->allowList();
        QSet<QString> blockList = wlmAccount->blockList();

        for ( int i = 0; i < m_preferencesWidget->m_AL->count(); i++ )
        {
            QString contact = m_preferencesWidget->m_AL->item(i)->text();
            if ( !allowList.contains(contact) )
                wlmAccount->server()->mainConnection->unblockContact( contact.toAscii().data() );
        }

        for ( int i = 0; i < m_preferencesWidget->m_BL->count(); i++ )
        {
            QString contact = m_preferencesWidget->m_BL->item(i)->text();
            if ( !blockList.contains(contact) )
                wlmAccount->server()->mainConnection->blockContact( contact.toAscii().data() );
        }
    }

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

void WlmEditAccountWidget::slotAllow()
{
    if ( m_preferencesWidget->m_BL->selectedItems().isEmpty() )
        return;

    QListWidgetItem *item = m_preferencesWidget->m_BL->selectedItems().at(0);
    m_preferencesWidget->m_BL->takeItem( m_preferencesWidget->m_BL->row( item ) );
    m_preferencesWidget->m_AL->addItem( item );
}

void WlmEditAccountWidget::slotBlock()
{
    if ( m_preferencesWidget->m_AL->selectedItems().isEmpty() )
        return;

    QListWidgetItem *item = m_preferencesWidget->m_AL->selectedItems().at(0);
    m_preferencesWidget->m_AL->takeItem( m_preferencesWidget->m_AL->row( item ) );
    m_preferencesWidget->m_BL->addItem( item );
}

void WlmEditAccountWidget::slotOpenRegister()
{
    KToolInvocation::invokeBrowser( "http://register.passport.net/"  );
}

#include "wlmeditaccountwidget.moc"
