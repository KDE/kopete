/*
 * This file is part of telepathy-accounts-kcm
 *
 * Copyright (C) 2009 Collabora Ltd. <http://www.collabora.co.uk/>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "account-item.h"

#include "accounts-list-model.h"
#include "edit-account-dialog.h"

#include <KApplication>
#include <KDebug>
#include <KIcon>

#include <QtCore/QTimer>
#include <QtGui/QPainter>

#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>

AccountItem::AccountItem(const Tp::AccountPtr &account, AccountsListModel *parent)
 : QObject(parent),
   m_account(account),
   m_icon(new KIcon()),
   m_editAccountDialog(0)
{
    kDebug();

    //connect AccountPtr signals to AccountItem signals
    connect(m_account.data(),
            SIGNAL(stateChanged(bool)),
            SIGNAL(updated()));
    connect(m_account.data(),
            SIGNAL(displayNameChanged(const QString&)),
            SIGNAL(updated()));

    //initialize icon only when the account is ready
    connect(this, SIGNAL(ready()), SLOT(generateIcon()));

    // We should look to see if the "account" instance we are passed is ready
    // yet. If not, we should get it ready now.
    Tp::Features features;
    features << Tp::Account::FeatureCore
             << Tp::Account::FeatureAvatar
             << Tp::Account::FeatureProtocolInfo;

    if (m_account->isReady(features)) {
        QTimer::singleShot(0, this, SIGNAL(ready()));
    } else {
        connect(m_account->becomeReady(features),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onAccountReady(Tp::PendingOperation*)));
    }
}

AccountItem::~AccountItem()
{
    kDebug();

    delete m_icon;
}

Tp::AccountPtr AccountItem::account() const
{
    return m_account;
}

void AccountItem::edit()
{
    kDebug();

    // If the edit dialog already exists, focus it.
    if (m_editAccountDialog) {
        m_editAccountDialog->focusWidget();
        return;
    }

    // FIXME: There should be a driving test for those who want to become parents... :'(
    QWidget *p = qobject_cast<QWidget*>(parent()->parent());
    m_editAccountDialog = new EditAccountDialog(this, p); // FIXME: Argh I'm going to jump off a bridge

    // Connect to the dialog's signals.
    connect(m_editAccountDialog,
            SIGNAL(finished()),
            SLOT(onAccountEdited()));

    m_editAccountDialog->show();
}

void AccountItem::remove()
{
    kDebug() << "Account about to be removed";

    Tp::PendingOperation *op = m_account->remove();
    connect(op,
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountRemoved(Tp::PendingOperation*)));
}

const KIcon& AccountItem::icon() const
{
    Q_ASSERT(m_icon != 0);

    return *m_icon;
}

void AccountItem::generateIcon()
{
    kDebug();

    QString iconPath = account()->icon();
    //if the icon has not been setted, we use the protocol icon
    if(iconPath.isEmpty()) {
        iconPath = QString("im-%1").arg(account()->protocol());
    }

    delete m_icon;
    m_icon = new KIcon(iconPath);

    if(!account()->isValid()) {
        //we paint a warning symbol in the right-bottom corner
        QPixmap pixmap = m_icon->pixmap(32, 32);
        QPainter painter(&pixmap);
        KIcon("dialog-error").paint(&painter, 15, 15, 16, 16);

        delete m_icon;
        m_icon = new KIcon(pixmap);
    }

    Q_EMIT(updated());
}

void AccountItem::onAccountReady(Tp::PendingOperation *op)
{
    kDebug();

    if (op->isError()) {
        kDebug() << "An error occurred in making and Account ready."
                 << op->errorName()
                 << op->errorMessage();
        return;
    }

    Q_EMIT ready();
}

void AccountItem::onAccountRemoved(Tp::PendingOperation *op)
{
    kDebug();

    if (op->isError()) {
        kDebug() << "An error occurred removing the Account."
                 << op->errorName()
                 << op->errorMessage();
        return;
    }

    Q_EMIT removed();
}

void AccountItem::onAccountEdited()
{
    kDebug();

    m_editAccountDialog->deleteLater();
    m_editAccountDialog = 0;

    Q_EMIT updated();
}


#include "account-item.moc"

