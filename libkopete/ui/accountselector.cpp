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

#include <q3header.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <QVBoxLayout>

#include <kdebug.h>
#include <k3listview.h>

class AccountListViewItem : public K3ListViewItem
{
	private:
		Kopete::Account *mAccount;

	public:
		AccountListViewItem(Q3ListView *parent, Kopete::Account *acc)
			: K3ListViewItem(parent)
		{
			if (acc==0)
				return;

			/*kDebug(14010) <<
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
		K3ListView *lv;
		Kopete::Protocol *proto;
};


AccountSelector::AccountSelector(QWidget *parent)
	: QWidget(parent)
{
	//kDebug(14010) << "for no special protocol";
	d = new AccountSelectorPrivate;
	d->proto = 0;
	initUI();
}


AccountSelector::AccountSelector(Kopete::Protocol *proto, QWidget *parent) : QWidget(parent)
{
	//kDebug(14010) << " for protocol " << proto->pluginId();
	d = new AccountSelectorPrivate;
	d->proto = proto;
	initUI();
}


AccountSelector::~AccountSelector()
{
	kDebug(14010) ;
	delete d;
}


void AccountSelector::initUI()
{
	kDebug(14010) ;
	QVBoxLayout *layout = new QVBoxLayout(this);
	d->lv = new K3ListView(this);
	d->lv->setFullWidth(true);
	d->lv->addColumn(QString::fromLatin1(""));
	d->lv->header()->hide();
	layout->addWidget(d->lv);
	setLayout(layout);
	kDebug(14010) << "creating list of all accounts";
	foreach(Kopete::Account *account , Kopete::AccountManager::self()->accounts() )
	{
		if( !d->proto  ||  account->protocol() == d->proto )
			new AccountListViewItem(d->lv, account);
	}

	connect(d->lv, SIGNAL(selectionChanged(Q3ListViewItem*)),
		this, SLOT(slotSelectionChanged(Q3ListViewItem*)));
}


void AccountSelector::setSelected(Kopete::Account *account)
{
	if (account==0)
		return;

	Q3ListViewItemIterator it(d->lv);
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

	Q3ListViewItemIterator it(d->lv);
	while (it.current())
	{
		if(static_cast<AccountListViewItem *>(it.current())->account() == account)
			return true;
	}
	return false;
}


Kopete::Account *AccountSelector::selectedItem()
{
	//kDebug(14010) ;

	if (d->lv->selectedItem() != 0)
		return static_cast<AccountListViewItem *>(d->lv->selectedItem())->account();
	return 0;
}


void AccountSelector::slotSelectionChanged(Q3ListViewItem *item)
{
	//kDebug(14010) ;
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
