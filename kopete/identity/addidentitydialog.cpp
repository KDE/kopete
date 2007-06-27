/*
    addidentitydialog.cpp - Kopete Add Identity Dialog

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

#include "addidentitydialog.h"

#include <QHeaderView>
#include <QMap>
#include <KIcon>

#include "kopeteidentity.h"
#include "kopeteidentitymanager.h"

class AddIdentityDialog::Private
{
public:
	Private() {}

	QTreeWidgetItem* selectedIdentity();

	QMap<QTreeWidgetItem *, Kopete::Identity *>  identityItems;
	Ui::AddIdentityBase ui;
	Kopete::Identity *identity;
};

AddIdentityDialog::AddIdentityDialog( QWidget *parent )
	: KDialog(parent)
	, d(new Private)
{
	setButtons(KDialog::Ok | KDialog::Cancel);
	d->ui.setupUi(mainWidget());
	d->ui.identityList->setColumnCount( 1 );
	d->ui.title->setPixmap(KIcon("identity").pixmap(22,22), KTitleWidget::ImageRight);

	QHeaderView *header = d->ui.identityList->header(); 
	header->setVisible(false);
	
	d->ui.newOption->setChecked(true);
	d->ui.identityName->setFocus();
	d->identity = 0;
	//add the available identities to the list
	foreach(Kopete::Identity *ident, Kopete::IdentityManager::self()->identities())
	{
		QTreeWidgetItem *identityItem = new QTreeWidgetItem(d->ui.identityList);
		identityItem->setIcon(0, KIcon(ident->customIcon()));
		identityItem->setText(0, ident->identityId());
		d->identityItems.insert(identityItem, ident);
	}

	 
	// hook up the user input
	connect(d->ui.identityList, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
		this, SLOT(slotValidate()));
	connect(d->ui.identityName, SIGNAL(textChanged(const QString&)),
		this, SLOT(slotValidate()));
	connect(d->ui.newOption, SIGNAL(toggled(bool)),
		this, SLOT(slotValidate()));
	connect(d->ui.identityList, SIGNAL(itemSelectionChanged()),
		this, SLOT( slotValidate()));
	connect(d->ui.identityList, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
		this, SLOT(slotIdentityListDoubleClicked()));

	slotValidate();
}

QTreeWidgetItem* AddIdentityDialog::Private::selectedIdentity()
{
	QList<QTreeWidgetItem*> selectedItems = ui.identityList->selectedItems();
	if(!selectedItems.empty())
 		return selectedItems.first();
	return 0;
}

void AddIdentityDialog::slotValidate()
{
	bool valid = true;
	if (d->ui.identityName->text().isEmpty())
		valid = false;
	
	if (d->ui.newOption->isChecked())
		d->ui.identityList->setEnabled(false);
	else if (d->ui.duplicateOption->isChecked())
	{
		d->ui.identityList->setEnabled(true);
		if (d->selectedIdentity() == 0)
			valid = false;
	}
	enableButtonOk( valid );
}

void AddIdentityDialog::slotIdentityListDoubleClicked()
{
	accept();
}

void AddIdentityDialog::accept()
{
	Kopete::IdentityManager *manager = Kopete::IdentityManager::self();

	//TODO implement
	
	KDialog::accept();
}

void AddIdentityDialog::reject()
{
    KDialog::reject();
}

AddIdentityDialog::~AddIdentityDialog()
{
	delete d;
}

Kopete::Identity *AddIdentityDialog::identity()
{
	return d->identity;
}

Kopete::Identity *AddIdentityDialog::getIdentity(QWidget *parent)
{
	AddIdentityDialog dialog(parent);

	dialog.exec();

	// this will already return 0 if the dialog was not accepted
	return dialog.identity();
}

#include "addidentitydialog.moc"

// vim: set noet ts=4 sts=4 sw=4:

