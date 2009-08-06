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

#include "accounts-list-model.h"

#include "account-item.h"

#include <KDebug>
#include <KIcon>

#include <TelepathyQt4/Account>

AccountsListModel::AccountsListModel(QObject *parent)
 : QAbstractListModel(parent)
{
    kDebug();

    m_unreadyAccounts.clear();
    m_readyAccounts.clear();
}

AccountsListModel::~AccountsListModel()
{
    kDebug();

    // TODO: Implement me!
}

int AccountsListModel::rowCount(const QModelIndex &index) const
{
    // If the index is the root item, then return the row count.
    if (index == QModelIndex()) {
       return m_readyAccounts.size();
    }

    // Otherwise, return 0 (as this is a list model, so all items
    // are children of the root item).
    return 0;
}

QVariant AccountsListModel::data(const QModelIndex &index, int role) const
{
    QVariant data;
    Tp::AccountPtr account = m_readyAccounts.at(index.row())->account();

    switch(role)
    {
    case Qt::DisplayRole:
        data = QVariant(account->displayName());
        break;

    case Qt::DecorationRole:
        data = QVariant(m_readyAccounts.at(index.row())->icon());
        break;

    case Qt::CheckStateRole:
        if(account->isEnabled()) {
            data = QVariant(Qt::Checked);
        }
        else {
            data = QVariant(Qt::Unchecked);
        }
        break;

    default:
        break;
    }

    return data;
}

bool AccountsListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    kDebug();
    if(role == Qt::CheckStateRole) {
        m_readyAccounts.at(index.row())->account()->setEnabled(value.toInt() == Qt::Checked);
        return true;
    }
    return false;
}

Qt::ItemFlags AccountsListModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
}

void AccountsListModel::addAccount(const Tp::AccountPtr &account)
{
    kDebug() << "Creating a new AccountItem from account:" << account.data();

    // Check if the account is already in the model.
    bool found = false;

    foreach (const AccountItem* ai, m_unreadyAccounts) {
        if (ai->account() == account) {
            found = true;
            break;
        }
    }

    if (!found) {
        foreach (const AccountItem* ai, m_readyAccounts) {
            if (ai->account() == account) {
                found = true;
                break;
            }
        }
    }

    if (found) {
       kWarning() << "Requested to add account"
                  << account.data()
                  << "to model, but it is already present. Doing nothing.";
   } else {
       kDebug() << "Account not already in model. Create new AccountItem from account:"
                << account.data();

       AccountItem *item = new AccountItem(account, this);
       m_unreadyAccounts.append(item);
       connect(item, SIGNAL(ready()), SLOT(onAccountItemReady()));
       connect(item, SIGNAL(removed()), SLOT(onAccountItemRemoved()));
       connect(item, SIGNAL(updated()), SLOT(onAccountItemUpdated()));
   }
}

void AccountsListModel::editAccount(const QModelIndex &index)
{
    kDebug();

    if(!index.isValid()) {
        kWarning() << "Can't edit Account: Invalid index.";
        return;
    }

    AccountItem *accountItem = m_readyAccounts.at(index.row());

    if (!accountItem) {
        kWarning() << "Account item is null.";
        return;
    }

    accountItem->edit();
}

void AccountsListModel::removeAccount(const QModelIndex &index)
{
    kDebug();

    if(!index.isValid()) {
        kDebug() << "Can't remove Account: Invalid index";
        return;
    }
    AccountItem *accountItem = m_readyAccounts.at(index.row());

    Q_ASSERT(accountItem);

    accountItem->remove();
}

void AccountsListModel::onAccountItemReady()
{
    kDebug();

    AccountItem *item = qobject_cast<AccountItem*>(sender());

    Q_ASSERT(item);
    if (!item) {
        kWarning() << "Not an AccountItem pointer:" << sender();
        return;
    }

    Q_ASSERT(m_unreadyAccounts.contains(item));
    if (!m_unreadyAccounts.contains(item)) {
        kWarning() << "Unready Accounts list does not contain Account Item:" << item;
        return;
    }

    Q_ASSERT(!m_readyAccounts.contains(item));
    if (m_readyAccounts.contains(item)) {
        kWarning() << "Ready Accounts list already contains Account Item:" << item;
        return;
    }

    beginInsertRows(QModelIndex(), m_readyAccounts.size(), m_readyAccounts.size());
    m_readyAccounts.append(item);
    m_unreadyAccounts.removeAll(item);
    endInsertRows();
}

void AccountsListModel::onAccountItemRemoved()
{
    kDebug();

    AccountItem *item = qobject_cast<AccountItem*>(sender());

    Q_ASSERT(item);
    if (!item) {
        kWarning() << "Not an AccountItem pointer:" << sender();
        return;
    }

    beginRemoveRows(QModelIndex(), m_readyAccounts.lastIndexOf(item),
                    m_readyAccounts.lastIndexOf(item));
    m_readyAccounts.removeAll(item);
    m_unreadyAccounts.removeAll(item);
    endRemoveRows();

    Q_ASSERT(!m_readyAccounts.contains(item));
    if (m_readyAccounts.contains(item)) {
        kWarning() << "Ready Accounts still contains Accout Item:" << item;
    }

    Q_ASSERT(!m_unreadyAccounts.contains(item));
    if (m_unreadyAccounts.contains(item)) {
        kWarning() << "Unready Accounts still contains Account Item:" << item;
    }
}

void AccountsListModel::onAccountItemUpdated()
{
    kDebug();
    
    AccountItem *item = qobject_cast<AccountItem*>(sender());

    Q_ASSERT(item);
    if (!item) {
        kWarning() << "Not an AccountItem pointer:" << sender();
        return;
    }

    QModelIndex index = createIndex(m_readyAccounts.lastIndexOf(item), 0);
    emit dataChanged(index, index);
}


#include "accounts-list-model.moc"

