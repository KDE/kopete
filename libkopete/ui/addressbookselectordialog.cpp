/*
    AddressBookSelectorDialog
    Nice Dialog to select a KDE AddressBook contact

    Copyright (c) 2005 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "addressbookselectordialog.h"

#include <qdialog.h>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <QVBoxLayout>
#include <KLocalizedString>
#include <QDialog>
#include <QPointer>
#include <ktreewidgetsearchline.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

namespace Kopete
{
namespace UI
{

QPushButton *okButton;

AddressBookSelectorDialog::AddressBookSelectorDialog(const QString &title, const QString &message, const QString &preSelectUid, QWidget *parent )
 : QDialog( parent )
{
	setWindowTitle( title );
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Help, this);
	QWidget *mainWidget = new QWidget(this);
	mainLayout->addWidget(mainWidget);
	okButton = buttonBox->button(QDialogButtonBox::Ok);
	okButton->setDefault(true);
	okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	//PORTING SCRIPT: WARNING mainLayout->addWidget(buttonBox) must be last item in layout. Please move it.
	mainLayout->addWidget(buttonBox);
	buttonBox->button(QDialogButtonBox::Cancel)->setShortcut(Qt::Key_Escape);
	buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
	QWidget *vbox=new QWidget(this);
	QVBoxLayout *vboxVBoxLayout = new QVBoxLayout(vbox);
	vboxVBoxLayout->setMargin(0);
	m_addressBookSelectorWidget= new AddressBookSelectorWidget(vbox);
	vboxVBoxLayout->addWidget(m_addressBookSelectorWidget);
	m_addressBookSelectorWidget->setLabelMessage(message);

//TODO PORT QT5 	vboxVBoxLayout->setSpacing( QDialog::spacingHint() );

	mainLayout->addWidget(vbox);
	okButton->setEnabled(false);
	//setHelp("linkaddressbook");
	//setHelp(QString(), "kopete");
	connect(m_addressBookSelectorWidget, SIGNAL(addresseeListClicked(QTreeWidgetItem*)), SLOT(slotWidgetAddresseeListClicked(QTreeWidgetItem*)));

	if ( !preSelectUid.isEmpty() )
		m_addressBookSelectorWidget->selectAddressee(preSelectUid);
}

AddressBookSelectorDialog::~AddressBookSelectorDialog()
{
}

KContacts::Addressee AddressBookSelectorDialog::getAddressee( const QString &title, const QString &message, const QString &preSelectUid, QWidget *parent)
{
	QPointer <AddressBookSelectorDialog> dialog = new AddressBookSelectorDialog(title, message, preSelectUid, parent);
	int result = dialog->exec();

	KContacts::Addressee adr;
	if ( result == QDialog::Accepted && dialog )
		adr = dialog->addressBookSelectorWidget()->addressee();

	delete dialog;

	return adr;
}

void AddressBookSelectorDialog::slotWidgetAddresseeListClicked( QTreeWidgetItem *addressee )
{
	// enable ok if a valid addressee is selected
	okButton->setEnabled( addressee ? addressee->isSelected() : false);
}

void AddressBookSelectorDialog::accept()
{
	QDialog::accept();
}

void AddressBookSelectorDialog::reject()
{
	QDialog::reject();
}

} // namespace UI
} // namespace Kopete


// vim: set noet ts=4 sts=4 sw=4:

