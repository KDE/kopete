/*
    AddressBookSelectorWidget

    Copyright (c) 2005 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Based on LinkAddressBookUI whose code was shamelessly stolen from
    kopete's add new contact wizard, used in Konversation, and then
    reappropriated by Kopete.

    LinkAddressBookUI:
    Copyright (c) 2004 by John Tapsell           <john@geola.co.uk>
    Copyright (c) 2003-2005 by Will Stephenson   <will@stevello.free-online.co.uk>
    Copyright (c) 2002 by Nick Betcher           <nbetcher@kde.org>
    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "addressbookselectorwidget.h"

#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>

#include <kdeversion.h>
#include <kinputdialog.h>

#include <kpushbutton.h>
#include <kdebug.h>
#include <ktreewidgetsearchline.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <QHeaderView>
#include <QTreeWidget>
#include <QTreeWidgetItem>


#include <addresseeitem.h>
#include "kabcpersistence.h"

using namespace Kopete::UI;

namespace Kopete
{
namespace UI
{

AddressBookSelectorWidget::AddressBookSelectorWidget( QWidget *parent, const char *name )
		: QWidget(parent), Ui::AddressBookSelectorWidget_Base()
{
	setObjectName(name);
	setupUi(this);

	m_addressBook = Kopete::KABCPersistence::self()->addressBook();

	// Addressee validation connections
	connect( addAddresseeButton, SIGNAL(clicked()), SLOT(slotAddAddresseeClicked()) );
	connect( addAddresseeButton, SIGNAL(clicked()), SIGNAL(addAddresseeClicked()) );

	connect( addresseeListView, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
			SIGNAL(addresseeListClicked(QTreeWidgetItem*)) );
	connect( addresseeListView, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
			SIGNAL(addresseeListClicked(QTreeWidgetItem*)) );
	//FIXME: Find the correct replacement for Q3ListView::spacePressed(Q3ListViewItem*)
	connect( addresseeListView, SIGNAL(itemPressed(QTreeWidgetItem*,int)),
			SIGNAL(addresseeListClicked(QTreeWidgetItem*)) );

	connect( m_addressBook, SIGNAL(addressBookChanged(AddressBook*)), this, SLOT(slotLoadAddressees()) );

	//We should add a clear QAction here.  But we can't really do that with a designer file :\  this sucks

	addresseeListView->header()->setResizeMode(QHeaderView::ResizeToContents);
	addresseeListView->currentItem()->setIcon(2, QIcon(QLatin1String("internet-mail")));
	addresseeListView->currentItem()->setText(2, i18n("Email"));

	kListViewSearchLine->setTreeWidget(addresseeListView);
	slotLoadAddressees();

	addresseeListView->header()->setResizeMode(QHeaderView::ResizeToContents);
	addresseeListView->setColumnWidth(0, 63); //Photo is 60, and it's nice to have a small gap, imho
}


AddressBookSelectorWidget::~AddressBookSelectorWidget()
{
	disconnect( m_addressBook, SIGNAL(addressBookChanged(AddressBook*)), this, SLOT(slotLoadAddressees()) );
}


KContacts::Addressee AddressBookSelectorWidget::addressee()
{
	AddresseeItem *item = 0L;
	item = static_cast<AddresseeItem *>( addresseeListView->currentItem() );

	if ( item )
		m_addressee = item->addressee();

	return m_addressee;
}

void AddressBookSelectorWidget::selectAddressee( const QString &uid )
{
	// iterate trough list view
	QTreeWidgetItemIterator it( addresseeListView );
	while( (*it) )
	{
		AddresseeItem *addrItem = (AddresseeItem *) ( (*it) );
		if ( addrItem->addressee().uid() == uid )
		{
			// select the contact item
			addrItem->setSelected( true );
			addresseeListView->scrollToItem( addrItem, QAbstractItemView::EnsureVisible );
		}
		++it;
	}
}

bool AddressBookSelectorWidget::addresseeSelected()
{
	return addresseeListView->currentItem() ? true : false;
}

/**  Read in contacts from addressbook, and select the contact that is for our nick. */
void AddressBookSelectorWidget::slotLoadAddressees()
{
	addresseeListView->clear();
	KContacts::AddressBook::Iterator it;
	for( it = m_addressBook->begin(); it != m_addressBook->end(); ++it )
	{
		new AddresseeItem( addresseeListView, (*it));
	}

}

void AddressBookSelectorWidget::setLabelMessage( const QString &msg )
{
	lblHeader->setTextFormat(Qt::PlainText);
	lblHeader->setText(msg);
}

void AddressBookSelectorWidget::slotAddAddresseeClicked()
{
	// Pop up add addressee dialog
	QString addresseeName = KInputDialog::getText( i18n( "New Address Book Entry" ), i18n( "Name the new entry:" ), QString(), 0, this );

	if ( !addresseeName.isEmpty() )
	{
		KContacts::Addressee addr;
		addr.setNameFromString( addresseeName );
		m_addressBook->insertAddressee(addr);
		Kopete::KABCPersistence::self()->writeAddressBook( 0 );
		slotLoadAddressees();
		// select the addressee we just added
		QList<QTreeWidgetItem *> added = addresseeListView->findItems( addresseeName, 0, 1 );
		foreach(QTreeWidgetItem *wi, added) {
			wi->setSelected( true );
			addresseeListView->scrollToItem(wi, QAbstractItemView::EnsureVisible);
		}
		kListViewSearchLine->clear();
		kListViewSearchLine->updateSearch();
		kListViewSearchLine->clear();
		kListViewSearchLine->updateSearch();
		//addresseeListView->scrollToItem( added, 0 );
	}
}

} // namespace UI
} // namespace Kopete

#include "addressbookselectorwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:

