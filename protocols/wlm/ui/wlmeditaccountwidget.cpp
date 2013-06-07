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
#include <QNetworkProxy>

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
 : QWidget (parent), KopeteEditAccountWidget(account), m_wlmAccount(0)
{
    m_preferencesWidget = new Ui::WlmAccountPreferences ();
    m_preferencesWidget->setupUi (this);
    m_preferencesWidget->mainTabWidget->setCurrentIndex(0);

    if ( account )
    {
        m_wlmAccount = static_cast<WlmAccount*>(account);

        m_preferencesWidget->m_passport->setText( m_wlmAccount->accountId() );
        m_preferencesWidget->m_password->load( &m_wlmAccount->password() );

        m_preferencesWidget->m_passport->setReadOnly( true );
        m_preferencesWidget->m_autologin->setChecked( account->excludeConnect()  );
        if ( m_wlmAccount->serverName() != "messenger.hotmail.com" || m_wlmAccount->serverPort() != 1863 )
            m_preferencesWidget->optionOverrideServer->setChecked( true );

        m_preferencesWidget->m_serverName->setText( m_wlmAccount->serverName() );
        m_preferencesWidget->m_serverPort->setValue( m_wlmAccount->serverPort() );

        if ( m_wlmAccount->isProxyEnabled() )
            m_preferencesWidget->optionEnableProxy->setChecked( true );

        m_preferencesWidget->m_proxyHost->setText( m_wlmAccount->proxyHost() );
        m_preferencesWidget->m_proxyPort->setValue( m_wlmAccount->proxyPort() );
        m_preferencesWidget->m_proxyUsername->setText( m_wlmAccount->proxyUsername() );
        m_preferencesWidget->m_proxyPassword->setText( m_wlmAccount->proxyPassword() );

        m_preferencesWidget->m_doNotSendEmoticons->setChecked( m_wlmAccount->doNotSendEmoticons() );
        m_preferencesWidget->m_doNotRequestEmoticons->setChecked( m_wlmAccount->doNotRequestEmoticons() );

        if(m_wlmAccount->proxyType() == QNetworkProxy::Socks5Proxy )
            m_preferencesWidget->m_radioProxySocks5->setChecked( true );
        else
            m_preferencesWidget->m_radioProxyHttp->setChecked( true );

        bool connected = account->isConnected();
        if ( connected )
            m_preferencesWidget->m_warning->hide();

        m_preferencesWidget->m_allowButton->setEnabled( connected );
        m_preferencesWidget->m_blockButton->setEnabled( connected );

        m_preferencesWidget->m_allowButton->setIcon( KIcon( "arrow-left" ) );
        m_preferencesWidget->m_blockButton->setIcon( KIcon( "arrow-right" ) );

        QSet<QString> serverSideContacts = m_wlmAccount->serverSideContacts();
        foreach ( const QString &contact, m_wlmAccount->allowList() )
        {
            QListWidgetItem *item = new QListWidgetItem( contact );
            if ( !serverSideContacts.contains( contact ) )
            {
                QFont f = item->font();
                f.setItalic( true );
                item->setFont( f );
            }
            m_preferencesWidget->m_AL->addItem( item );
        }

        foreach ( const QString &contact, m_wlmAccount->blockList() )
        {
            QListWidgetItem *item = new QListWidgetItem( contact );
            if ( !serverSideContacts.contains( contact ) )
            {
                QFont f = item->font();
                f.setItalic( true );
                item->setFont( f );
            }
            m_preferencesWidget->m_BL->addItem( item );
        }

        m_deleteActionAL = new QAction( i18n( "Delete" ), m_preferencesWidget->m_AL );
        m_preferencesWidget->m_AL->addAction( m_deleteActionAL );

        m_deleteActionBL = new QAction( i18n( "Delete" ), m_preferencesWidget->m_BL );
        m_preferencesWidget->m_BL->addAction( m_deleteActionBL );

        connect( m_preferencesWidget->m_AL, SIGNAL(itemSelectionChanged()), this, SLOT(updateActionsAL()) );
        connect( m_preferencesWidget->m_BL, SIGNAL(itemSelectionChanged()), this, SLOT(updateActionsBL()) );
        connect( m_deleteActionAL, SIGNAL(triggered(bool)), this, SLOT(deleteALItem()) );
        connect( m_deleteActionBL, SIGNAL(triggered(bool)), this, SLOT(deleteBLItem()) );
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
        setAccount( new WlmAccount( WlmProtocol::protocol(), m_preferencesWidget->m_passport->text().trimmed() ) );

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

    if (m_preferencesWidget->optionEnableProxy->isChecked() ) {
        config->writeEntry( "enableProxy", true );
        config->writeEntry( "proxyHost", m_preferencesWidget->m_proxyHost->text().trimmed() );
        config->writeEntry( "proxyPort", m_preferencesWidget->m_proxyPort->value()  );
        config->writeEntry( "proxyUsername", m_preferencesWidget->m_proxyUsername->text() );
        config->writeEntry( "proxyPassword", m_preferencesWidget->m_proxyPassword->text() );
        if(m_preferencesWidget->m_radioProxyHttp->isChecked())
            config->writeEntry( "proxyType", (uint)QNetworkProxy::HttpProxy );
        else
            config->writeEntry( "proxyType", (uint)QNetworkProxy::Socks5Proxy );
    }
    else {
        config->writeEntry( "enableProxy", false );
    }

    config->writeEntry( "doNotSendEmoticons", 
            m_preferencesWidget->m_doNotSendEmoticons->isChecked());

    config->writeEntry( "doNotRequestEmoticons", 
            m_preferencesWidget->m_doNotRequestEmoticons->isChecked());

    if ( wlmAccount->isConnected() )
    {
        QSet<QString> allowList = wlmAccount->allowList();
        QSet<QString> blockList = wlmAccount->blockList();

        for ( int i = 0; i < m_preferencesWidget->m_AL->count(); i++ )
        {
            QString contact = m_preferencesWidget->m_AL->item(i)->text();
            if ( !allowList.contains(contact) )
                wlmAccount->server()->mainConnection->unblockContact( contact.toLatin1().constData() );
        }

        for ( int i = 0; i < m_preferencesWidget->m_BL->count(); i++ )
        {
            QString contact = m_preferencesWidget->m_BL->item(i)->text();
            if ( !blockList.contains(contact) )
                wlmAccount->server()->mainConnection->blockContact( contact.toLatin1().constData() );
        }

        foreach ( const QString &contact, m_deletedContactsAL )
            wlmAccount->server()->mainConnection->removeFromList( MSN::LST_AL, contact.toLatin1().constData() );

        foreach ( const QString &contact, m_deletedContactsBL )
            wlmAccount->server()->mainConnection->removeFromList( MSN::LST_BL, contact.toLatin1().constData() );
    }

    return account ();
}

bool WlmEditAccountWidget::validateData ()
{
    QString contactId = m_preferencesWidget->m_passport->text().trimmed();
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

void WlmEditAccountWidget::updateActionsAL()
{
    bool enableDeleteAction = false;

    if ( m_wlmAccount && !m_preferencesWidget->m_AL->selectedItems().isEmpty() )
        enableDeleteAction = !m_wlmAccount->serverSideContacts().contains( m_preferencesWidget->m_AL->selectedItems().at(0)->text() );

    m_deleteActionAL->setEnabled( enableDeleteAction );
}

void WlmEditAccountWidget::updateActionsBL()
{
    bool enableDeleteAction = false;

    if ( m_wlmAccount && !m_preferencesWidget->m_BL->selectedItems().isEmpty() )
        enableDeleteAction = !m_wlmAccount->serverSideContacts().contains( m_preferencesWidget->m_BL->selectedItems().at(0)->text() );

    m_deleteActionBL->setEnabled( enableDeleteAction );
}

void WlmEditAccountWidget::deleteALItem()
{
    if ( m_wlmAccount && !m_preferencesWidget->m_AL->selectedItems().isEmpty() )
    {
        QListWidgetItem *item = m_preferencesWidget->m_AL->selectedItems().at(0);
        if ( !m_wlmAccount->serverSideContacts().contains( item->text() ) )
        {
            m_deletedContactsAL.insert( item->text() );
            m_preferencesWidget->m_AL->takeItem( m_preferencesWidget->m_AL->row( item ) );
        }
    }
}

void WlmEditAccountWidget::deleteBLItem()
{
    if ( m_wlmAccount && !m_preferencesWidget->m_BL->selectedItems().isEmpty() )
    {
        QListWidgetItem *item = m_preferencesWidget->m_BL->selectedItems().at(0);
        if ( !m_wlmAccount->serverSideContacts().contains( item->text() ) )
        {
            m_deletedContactsBL.insert( item->text() );
            m_preferencesWidget->m_BL->takeItem( m_preferencesWidget->m_BL->row( item ) );
        }
    }
}

void WlmEditAccountWidget::slotOpenRegister()
{
    KToolInvocation::invokeBrowser( "http://register.passport.net/"  );
}

#include "wlmeditaccountwidget.moc"
