/*
    addidentitywizard.cpp - Kopete Add Identity Wizard

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

#include "addidentitywizard.h"

#include <QHeaderView>
#include <KVBox>
#include <KIcon>

#include "kopeteidentity.h"
#include "kopeteidentitymanager.h"

class AddIdentityWizard::Private
{
public:
	Private()
		{
		}

	QTreeWidgetItem* selectedIdentity();

	QMap<QTreeWidgetItem *, Kopete::Identity *>  identityItems;
	//KopeteEditIdentityWidget *identityPage;
	KVBox *identityPageWidget;
	QWidget *selectIdentity;
	QWidget *finish;
	Ui::AddIdentityWizardPage1 uiSelectIdentity;
	//Ui::AddIdentityWizardPage2 uiFinish;
	KPageWidgetItem *selectIdentityItem;
};

AddIdentityWizard::AddIdentityWizard( QWidget *parent )
	: KAssistantDialog(parent)
	, d(new Private)
{
	// setup the select service page
	d->selectIdentity = new QWidget(this);
	d->uiSelectIdentity.setupUi(d->selectIdentity);
	d->uiSelectIdentity.identityList->setColumnCount( 1 );
	QHeaderView *header = d->uiSelectIdentity.identityList->header(); 
	header->setVisible(false);
	
	d->selectIdentityItem = addPage(d->selectIdentity,d->selectIdentity->windowTitle());
	setValid(d->selectIdentityItem, false);
		
	//d->identityPageWidget = new KVBox(this);
	//addPage(d->identityPageWidget,i18n("Step Two: Identity Information"));

	// setup the final page
	d->finish = new QWidget(this);
	//d->uiFinish.setupUi(d->finish);
	
	d->uiSelectIdentity.newOption->setChecked(true);
	d->uiSelectIdentity.identityName->setFocus();
	//add the available identities to the list
	foreach(Kopete::Identity *ident, Kopete::IdentityManager::self()->identities())
	{
		QTreeWidgetItem *identityItem = new QTreeWidgetItem(d->uiSelectIdentity.identityList);
		identityItem->setIcon(0, KIcon(ident->customIcon()));
		identityItem->setText(0, ident->identityId());
		d->identityItems.insert(identityItem, ident);
	}

	 
	// hook up the user input
	connect(d->uiSelectIdentity.identityList, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
		this, SLOT(slotValidate()));
	connect(d->uiSelectIdentity.identityName, SIGNAL(textChanged(const QString&)),
		this, SLOT(slotValidate()));
	connect(d->uiSelectIdentity.newOption, SIGNAL(toggled(bool)),
		this, SLOT(slotValidate()));
	connect(d->uiSelectIdentity.identityList, SIGNAL(itemSelectionChanged()),
		this, SLOT( slotValidate()));
	connect(d->uiSelectIdentity.identityList, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
		this, SLOT(slotIdentityListDoubleClicked()));

	slotValidate();
}

QTreeWidgetItem* AddIdentityWizard::Private::selectedIdentity()
{
	QList<QTreeWidgetItem*> selectedItems = uiSelectIdentity.identityList->selectedItems();
	if(!selectedItems.empty())
 		return selectedItems.first();
	return 0;
}

void AddIdentityWizard::slotValidate()
{
	bool valid = true;
	if (d->uiSelectIdentity.identityName->text().isEmpty())
		valid = false;
	
	if (d->uiSelectIdentity.newOption->isChecked())
		d->uiSelectIdentity.identityList->setEnabled(false);
	else if (d->uiSelectIdentity.duplicateOption->isChecked())
	{
		d->uiSelectIdentity.identityList->setEnabled(true);
		if (d->selectedIdentity() == 0)
			valid = false;
	}
	// Make sure a protocol is selected before allowing the user to continue
	setValid(d->selectIdentityItem, valid);
}

void AddIdentityWizard::slotIdentityListDoubleClicked()
{
	//FIXME validate data before advancing
	// proceed to the next wizard page if we double click a protocol
	next();
}

void AddIdentityWizard::back()
{
	//TODO implement
#if 0
	if (currentPage()->widget() == d->identityPageWidget)
	{
		// Deletes the identityPage, K3Wizard does not like deleting pages
		// using different pointers, it only seems to watch its own pointer
		delete d->identityPage;
		d->identityPage = 0;

		// removePage() already goes back to previous page, no back() needed
	}
#endif
	KAssistantDialog::back();
}

void AddIdentityWizard::next()
{
	if (currentPage()->widget() == d->selectIdentity)
	{
		QTreeWidgetItem *lvi = d->selectedIdentity();
		if(!d->identityItems[lvi])
		{ //no item selected
			return;
		}
		
		//FIXME check if need to duplicate identity
			
		KAssistantDialog::next();
	}
	else if (currentPage()->widget() == d->identityPageWidget)
	{
		//TODO implement
		KAssistantDialog::next();
	}
	else 
	{
		kDebug(14100) << k_funcinfo << "Next pressed on misc page" << endl;
		KAssistantDialog::next();
	}

}

void AddIdentityWizard::accept()
{
	Kopete::IdentityManager *manager = Kopete::IdentityManager::self();

	//TODO implement
	
	KAssistantDialog::accept();
}

void AddIdentityWizard::reject()
{
    KAssistantDialog::reject();
}

AddIdentityWizard::~AddIdentityWizard()
{
	delete d;
}

#include "addidentitywizard.moc"

// vim: set noet ts=4 sts=4 sw=4:

