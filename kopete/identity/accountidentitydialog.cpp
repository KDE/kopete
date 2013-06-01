/*
    accountidentitydialog.cpp - Kopete Add Identity Dialog

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

    Kopete    (c) 2003-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "accountidentitydialog.h"
#include "ui_accountidentitybase.h"

#include <QHeaderView>
#include <QMap>
#include <QPointer>
#include <KIcon>
#include <KMessageBox>

#include "kopeteidentity.h"
#include "kopeteidentitymanager.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopeteprotocol.h"

class AccountIdentityDialog::Private
{
public:
	Private() 
	{
		hiddenIdentity = 0;
		currentIdentity = 0;
	}

	QTreeWidgetItem* selectedIdentity();

	QMap<QTreeWidgetItem *, Kopete::Identity *>  identityItems;
	Ui::AccountIdentityBase ui;
	Kopete::Identity *hiddenIdentity;
	Kopete::Identity *currentIdentity;
	QList<Kopete::Account*> accounts;
};

AccountIdentityDialog::AccountIdentityDialog( QWidget *parent )
	: KDialog(parent)
	, d(new Private)
{
	setButtons(KDialog::Ok | KDialog::Cancel);
	d->ui.setupUi(mainWidget());
	d->ui.identityList->setColumnCount( 1 );
	d->ui.title->setPixmap(KIcon("identity").pixmap(22,22), KTitleWidget::ImageRight);

	QHeaderView *header = d->ui.identityList->header(); 
	header->setVisible(false);
	
	// hook up the user input
	connect(d->ui.identityList, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotValidate()));
	connect(d->ui.identityList, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotValidate()));
	connect(d->ui.identityList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotIdentityListDoubleClicked()));

	// identity manager signals
	Kopete::IdentityManager *manager = Kopete::IdentityManager::self();
	connect(manager, SIGNAL(identityRegistered(Kopete::Identity*)), this, SLOT(slotLoadIdentities()));
	connect(manager, SIGNAL(identityUnregistered(const Kopete::Identity*)), this, SLOT(slotLoadIdentities()));
	
	// account manager signals
	Kopete::AccountManager *acmanager = Kopete::AccountManager::self();
	connect(acmanager, SIGNAL(accountOnlineStatusChanged(Kopete::Account*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
			this, SLOT(slotLoadAccounts()));
	slotLoadIdentities();
	slotValidate();
}

QTreeWidgetItem* AccountIdentityDialog::Private::selectedIdentity()
{
	QList<QTreeWidgetItem*> selectedItems = ui.identityList->selectedItems();
	if(!selectedItems.empty())
 		return selectedItems.first();
	return 0;
}

void AccountIdentityDialog::slotValidate()
{
	// if no identity was selected, we can't continue
	enableButtonOk( d->selectedIdentity() );
}

void AccountIdentityDialog::slotIdentityListDoubleClicked()
{
	if (d->selectedIdentity())
		accept();
}

void AccountIdentityDialog::slotLoadIdentities()
{
	// clear things before loading again
	d->identityItems.clear();
	d->ui.identityList->clear();

	//add the available identities to the list
	foreach(Kopete::Identity *ident, Kopete::IdentityManager::self()->identities())
	{
		// if we were asked to hide an identity, do not show it
		if (ident == d->hiddenIdentity)
			continue;

		QTreeWidgetItem *identityItem = new QTreeWidgetItem(d->ui.identityList);
		identityItem->setIcon(0, KIcon(ident->customIcon()));
		identityItem->setText(0, ident->label());
		d->identityItems.insert(identityItem, ident);
		if (ident == d->currentIdentity)
			identityItem->setSelected(true);
	}
}

void AccountIdentityDialog::slotLoadAccounts()
{
	d->currentIdentity = 0;

	// set the accounts label
	QString text;
	foreach(Kopete::Account *account, d->accounts)
	{
		if (account->identity() != d->currentIdentity)
			d->currentIdentity = account->identity();

		text += QString("<nobr><img src=\"kopete-account-icon:%3:%4\"> <b>%1:</b> %2 <br/>")
					.arg(account->protocol()->displayName())
					.arg(account->accountLabel())
				    .arg(QString(QUrl::toPercentEncoding( account->protocol()->pluginId() )))
					.arg(QString(QUrl::toPercentEncoding( account->accountId() )));
	}

	d->ui.accounts->setText(text);
	slotLoadIdentities();
}

void AccountIdentityDialog::accept()
{
	Kopete::Identity *ident = d->identityItems[d->selectedIdentity()]; 
	if (!ident)
		return;

	foreach(Kopete::Account *account, d->accounts)
	{
		account->setIdentity(ident);
	}

	KDialog::accept();
}

void AccountIdentityDialog::reject()
{
    KDialog::reject();
}

AccountIdentityDialog::~AccountIdentityDialog()
{
	delete d;
}

void AccountIdentityDialog::setAccount( Kopete::Account *account )
{
	d->accounts.clear();
	d->accounts.append( account );
	slotLoadAccounts();
}

void AccountIdentityDialog::setAccounts( QList<Kopete::Account*> accountList )
{
	d->accounts = accountList;
	slotLoadAccounts();
}

void AccountIdentityDialog::setMessage( const QString &text )
{
	d->ui.selectText->setText( text );
}

void AccountIdentityDialog::setHiddenIdentity( Kopete::Identity *ident )
{
	d->hiddenIdentity = ident;
	slotLoadIdentities();
}

// static member functions

bool AccountIdentityDialog::changeAccountIdentity( QWidget *parent, Kopete::Account *account, 
													Kopete::Identity *hidden_ident,
													const QString &message )
{
	QPointer <AccountIdentityDialog> dialog = new AccountIdentityDialog( parent );

	dialog->setAccount( account );
	dialog->setHiddenIdentity( hidden_ident );
	if ( !message.isEmpty() )
		dialog->setMessage( message );

	int ret = dialog->exec();
	delete dialog;
	return ret;
}

bool AccountIdentityDialog::changeAccountIdentity( QWidget *parent, QList<Kopete::Account*> accountList, 
										Kopete::Identity *hidden_ident,
										const QString &message )
{
	QPointer <AccountIdentityDialog> dialog = new AccountIdentityDialog( parent );

	dialog->setAccounts( accountList );
	dialog->setHiddenIdentity( hidden_ident );
	if ( !message.isEmpty() )
		dialog->setMessage( message );

	int ret = dialog->exec();
	delete dialog;
	return ret;
}

#include "accountidentitydialog.moc"

// vim: set noet ts=4 sts=4 sw=4:

