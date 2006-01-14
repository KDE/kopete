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

#include <qheader.h>
#include <qlayout.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <klistview.h>

class AccountListViewItem : public KListViewItem
{
	private:
		Kopete::Account *mAccount;

	public:
		AccountListViewItem(QListView *parent, Kopete::Account *acc)
			: KListViewItem(parent)
		{
			if (acc==0)
				return;

			/*kdDebug(14010) << k_funcinfo <<
				"account name = " << acc->accountId() << endl;*/
			mAccount = acc;
			setText(0, mAccount->accountId());
			setPixmap(0, mAccount->accountIcon());
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
		KListView *lv;
		Kopete::Protocol *proto;
};


AccountSelector::AccountSelector(QWidget *parent, const char *name)
	: QWidget(parent, name)
{
	//kdDebug(14010) << k_funcinfo << "for no special protocol" << endl;
	d = new AccountSelectorPrivate;
	d->proto = 0;
	initUI();
}


AccountSelector::AccountSelector(Kopete::Protocol *proto, QWidget *parent,
	const char *name) : QWidget(parent, name)
{
	//kdDebug(14010) << k_funcinfo << " for protocol " << proto->pluginId() << endl;
	d = new AccountSelectorPrivate;
	d->proto = proto;
	initUI();
}


AccountSelector::~AccountSelector()
{
	kdDebug(14010) << k_funcinfo << endl;
	delete d;
}


void AccountSelector::initUI()
{
	kdDebug(14010) << k_funcinfo << endl;
	(new QVBoxLayout(this))->setAutoAdd(true);
	d->lv = new KListView(this);
	d->lv->setFullWidth(true);
	d->lv->addColumn(QString::fromLatin1(""));
	d->lv->header()->hide();

	if(d->proto != 0)
	{
		kdDebug(14010) << k_funcinfo << "creating list for a certain protocol" << endl;
		QDict<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts(d->proto);
		QDictIterator<Kopete::Account> it(accounts);
		for(; Kopete::Account *account = it.current(); ++it)
		{
			new AccountListViewItem(d->lv, account);
		}
	}
	else
	{
		kdDebug(14010) << k_funcinfo << "creating list of all accounts" << endl;
		QPtrList<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts();
		Kopete::Account *account = 0;
		for(account = accounts.first(); account; account = accounts.next())
		{
			new AccountListViewItem(d->lv, account);
		}
	}

	connect(d->lv, SIGNAL(selectionChanged(QListViewItem *)),
		this, SLOT(slotSelectionChanged(QListViewItem *)));
}


void AccountSelector::setSelected(Kopete::Account *account)
{
	if (account==0)
		return;

	QListViewItemIterator it(d->lv);
	while (it.current())
	{
		if(static_cast<AccountListViewItem *>(it.current())->account() == account)
		{
			it.current()->setSelected(true);
			return;
		}
	}
}


bool AccountSelector::isSelected(Kopete::Account *account)
{
	if (account==0)
		return false;

	QListViewItemIterator it(d->lv);
	while (it.current())
	{
		if(static_cast<AccountListViewItem *>(it.current())->account() == account)
			return true;
	}
	return false;
}


Kopete::Account *AccountSelector::selectedItem()
{
	//kdDebug(14010) << k_funcinfo << endl;

	if (d->lv->selectedItem() != 0)
		return static_cast<AccountListViewItem *>(d->lv->selectedItem())->account();
	return 0;
}


void AccountSelector::slotSelectionChanged(QListViewItem *item)
{
	//kdDebug(14010) << k_funcinfo << endl;
	if (item != 0)
	{
		Kopete::Account *account = static_cast<AccountListViewItem *>(item)->account();
		if (account != 0)
		{
			emit selectionChanged(account);
			return;
		}
	}

	emit selectionChanged(0);
}

#include "accountselector.moc"
// vim: set noet ts=4 sts=4 sw=4:
