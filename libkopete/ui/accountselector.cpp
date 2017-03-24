/*
    accountselector.cpp - An Accountselector

    Copyright (c) 2004      by Stefan Gehn <metz AT gehn.net>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "accountselector.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"

#include <QIcon>
#include <QHeaderView>
#include <QTreeWidget>
#include <qlayout.h>
#include <QVBoxLayout>

#include <kdebug.h>

class AccountListViewItem : public QTreeWidgetItem
{
private:
    Kopete::Account *mAccount;

public:
    AccountListViewItem(QTreeWidget *parent, Kopete::Account *acc)
        : QTreeWidgetItem(parent)
    {
        if (acc == 0) {
            return;
        }

        /*qCDebug(LIBKOPETE_LOG) <<
            "account name = " << acc->accountId() << endl;*/
        mAccount = acc;
        setText(0, mAccount->accountId());
        setIcon(0, QIcon(mAccount->accountIcon()));
    }

    Kopete::Account *account()
    {
        return mAccount;
    }
};

// ----------------------------------------------------------------------------

class AccountSelectorPrivate
{
public:
    QTreeWidget *lv;
    Kopete::Protocol *proto;
};

AccountSelector::AccountSelector(QWidget *parent)
    : QWidget(parent)
{
    //qCDebug(LIBKOPETE_LOG) << "for no special protocol";
    d = new AccountSelectorPrivate;
    d->proto = 0;
    initUI();
}

AccountSelector::AccountSelector(Kopete::Protocol *proto, QWidget *parent) : QWidget(parent)
{
    //qCDebug(LIBKOPETE_LOG) << " for protocol " << proto->pluginId();
    d = new AccountSelectorPrivate;
    d->proto = proto;
    initUI();
}

AccountSelector::~AccountSelector()
{
    qCDebug(LIBKOPETE_LOG);
    delete d;
}

void AccountSelector::initUI()
{
    qCDebug(LIBKOPETE_LOG);
    QVBoxLayout *layout = new QVBoxLayout(this);
    d->lv = new QTreeWidget(this);
    d->lv->header()->setResizeMode(QHeaderView::ResizeToContents);
    d->lv->setHeaderLabel(QStringLiteral(""));
    d->lv->headerItem()->setHidden(true);
    layout->addWidget(d->lv);
    setLayout(layout);
    qCDebug(LIBKOPETE_LOG) << "creating list of all accounts";
    foreach (Kopete::Account *account, Kopete::AccountManager::self()->accounts()) {
        if (!d->proto || account->protocol() == d->proto) {
            new AccountListViewItem(d->lv, account);
        }
    }

    connect(d->lv, SIGNAL(currentItemChanged(QTreeWidgetItem *,QTreeWidgetItem *)),
            this, SLOT(slotSelectionChanged(QTreeWidgetItem *)));
}

void AccountSelector::setSelected(Kopete::Account *account)
{
    if (account == 0) {
        return;
    }

    QTreeWidgetItemIterator it(d->lv);
    while (*it)
    {
        if (static_cast<AccountListViewItem *>(*it)->account() == account) {
            (*it)->setSelected(true);
            return;
        }
        ++it;
    }
}

bool AccountSelector::isSelected(Kopete::Account *account)
{
    if (account == 0) {
        return false;
    }

    QTreeWidgetItemIterator it(d->lv);
    while (*it)
    {
        if (static_cast<AccountListViewItem *>(*it)->account() == account) {
            return true;
        }
        ++it;
    }
    return false;
}

Kopete::Account *AccountSelector::selectedItem()
{
    //qCDebug(LIBKOPETE_LOG) ;

    if (d->lv->currentItem()) {
        return static_cast<AccountListViewItem *>(d->lv->currentItem())->account();
    }
    return 0;
}

void AccountSelector::slotSelectionChanged(QTreeWidgetItem *item)
{
    //qCDebug(LIBKOPETE_LOG) ;
    if (item != 0) {
        Kopete::Account *account = static_cast<AccountListViewItem *>(item)->account();
        if (account != 0) {
            emit selectionChanged(account);
            return;
        }
    }

    emit selectionChanged(0);
}

// vim: set noet ts=4 sts=4 sw=4:
