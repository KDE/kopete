/*
    addcontactwizard.cpp - Kopete's Add Contact Wizard

    Copyright (c) 2002 by Nick Betcher           <nbetcher@kde.org>
    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
//	CONDITIONS FOR PROGRESSING:
//	Welcome page
//		true
//	|
//	V
//	Select Address Book Entry
//		( Addressee is selected AND is not already associated with a contact )
//			OR Do not use address book is checked
//	|
//	V
//	Select Display Name and Group
//		true
//	|
//	V
//	Select Account
//		( Only 1 account ) OR ( An account is selected )
//	|
//	V
//	(Each AddContactPage)
//		( Own conditions)
//	|
//	V
//	Finish
//		true

#include <qcheckbox.h>
#include <klocale.h>
#include <kiconloader.h>

#include <kdeversion.h>
#if KDE_IS_VERSION( 3, 1, 90 )
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif

#include <kpushbutton.h>
#include <kdebug.h>
#include <klistview.h>
// used for its AddresseeItem class
#include <kabc/addresseedialog.h>
#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>

#include <addcontactpage.h>
#include "addcontactwizard.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"

AddContactWizard::AddContactWizard( QWidget *parent, const char *name )
: AddContactWizard_Base( parent, name )
{
	// Populate the groups list
	KopeteGroupList groups=KopeteContactList::contactList()->groups();
	for( KopeteGroup *it = groups.first(); it; it = groups.next() )
	{
		QString groupname = it->displayName();
		if ( !groupname.isEmpty() )
			new QCheckListItem( groupList, groupname, QCheckListItem::CheckBox);
	}

	protocolListView->clear();
	m_accountItems.clear();

	// Populate the accounts list
	QCheckListItem* accountLVI=0L;
	QPtrList<KopeteAccount>  accounts = KopeteAccountManager::manager()->accounts();
	for(KopeteAccount *i=accounts.first() ; i; i=accounts.next() )
	{
		accountLVI= new QCheckListItem( protocolListView, i->accountId(), QCheckListItem::CheckBox);
		accountLVI->setText(1,i->protocol()->displayName() + QString::fromLatin1(" ") );
		accountLVI->setPixmap( 1, SmallIcon( i->protocol()->pluginIcon() ) );
		m_accountItems.insert(accountLVI,i);
	}

	// Populate the addressee list
	// This could be slow - is there a better way of doing it (progressive loading?)
	loadAddressees();



	if ( accounts.count() == 1 )
	{
		accountLVI->setOn( true );
		setAppropriate( selectService, false );
	}

	setNextEnabled( selectAddressee, false );
	setNextEnabled(selectService, (accounts.count() == 1));
	setFinishEnabled(finis, true);

	// FIXME: steal/add a create addressee widget
	addAddresseeButton->setEnabled( false );
	// Addressee validation connections
	connect( addAddresseeButton, SIGNAL( clicked() ), SLOT( slotAddAddresseeClicked() ) );
	connect( chkNoAddressee, SIGNAL( toggled( bool ) ),
			SLOT( slotCheckAddresseeChoice( bool ) ) );
	connect( addresseeListView, SIGNAL( clicked(QListViewItem * ) ),
			SLOT( slotAddresseeListClicked( QListViewItem * ) ) );
	connect( addresseeListView, SIGNAL( selectionChanged( QListViewItem * ) ),
			SLOT( slotAddresseeListClicked( QListViewItem * ) ) );
	connect( addresseeListView, SIGNAL( spacePressed( QListViewItem * ) ),
			SLOT( slotAddresseeListClicked( QListViewItem * ) ) );

	// Group manipulation connection
	connect( addGroupButton, SIGNAL(clicked()) , SLOT(slotAddGroupClicked()) );

	// Account choice validation connections
	connect( protocolListView, SIGNAL(clicked(QListViewItem *)), this, SLOT(slotProtocolListClicked(QListViewItem *)));
	connect( protocolListView, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(slotProtocolListClicked(QListViewItem *)));
	connect( protocolListView, SIGNAL(spacePressed(QListViewItem *)), this, SLOT(slotProtocolListClicked(QListViewItem *)));
}

AddContactWizard::~AddContactWizard()
{
}

void AddContactWizard::loadAddressees()
{
	addresseeListView->clear();
	KABC::AddressBook* ab = KABC::StdAddressBook::self();
	KABC::AddressBook::Iterator it;
	for( it = ab->begin(); it != ab->end(); ++it )
		KABC::AddresseeItem *item = new KABC::AddresseeItem( addresseeListView, (*it) );
}

void AddContactWizard::slotAddAddresseeClicked()
{
	// Pop up add addressee dialog
}

void AddContactWizard::slotCheckAddresseeChoice( bool on )
{
	// Disable addressee selection widgets
	addresseeListView->setEnabled( !on );
	addAddresseeButton->setEnabled( !on );
	if ( on )
		setNextEnabled( selectAddressee, true );
	else
	{
		if ( addresseeListView->selectedItem() )
			setNextEnabled( selectAddressee, true );
		else
			setNextEnabled( selectAddressee, false );
	}
}

void AddContactWizard::slotAddresseeListClicked( QListViewItem *addressee )
{
	// check a valid addressee is selected
	setNextEnabled( selectAddressee, addressee ? addressee->isSelected() : false );
}

void AddContactWizard::slotAddGroupClicked()
{
#if KDE_IS_VERSION( 3, 1, 90 )
	QString groupName = KInputDialog::getText(
		i18n( "New Group" ),
		i18n( "Please enter the name for the new group:" )
		);
#else
	QString groupName = KLineEditDlg::getText(
		i18n( "New Group" ),
		i18n( "Please enter the name for the new group:" )
		);
#endif
	if ( !groupName.isNull() )
		new QCheckListItem( groupList, groupName, QCheckListItem::CheckBox );
}

void AddContactWizard::slotProtocolListClicked( QListViewItem *)
{
	// Just makes sure a protocol is selected before allowing the user to continue
	bool oneIsChecked = false;

	for (QListViewItemIterator it(protocolListView); it.current(); ++it)
	{
		QCheckListItem *check = dynamic_cast<QCheckListItem *>(it.current());
		if (check && check->isOn())
		{
			oneIsChecked = true;
			break;
		}
	}

	setNextEnabled(selectService, oneIsChecked);
}

void AddContactWizard::accept()
{
	KopeteMetaContact *metaContact = new KopeteMetaContact();

	// set the display name if required
	if ( !mDisplayName->text().isEmpty() )
	{
		metaContact->setTrackChildNameChanges( false );
		metaContact->setDisplayName( mDisplayName->text() );
	}

	// set the KABC uid in the metacontact
	KABC::AddresseeItem *item = 0L;
	item = static_cast<KABC::AddresseeItem *>( addresseeListView->selectedItem() );
	if ( addresseeListView->isEnabled() && item )
		metaContact->setMetaContactId( item->addressee().uid() );

	// set the metacontact's groups
	bool topLevel = true;
	for ( QListViewItemIterator it( groupList ); it.current(); ++it )
	{
		QCheckListItem *check = dynamic_cast<QCheckListItem *>( it.current() );
		if ( check && check->isOn() )
		{
			metaContact->addToGroup( KopeteContactList::contactList()->getGroup( check->text() ) );
			topLevel = false;
		}
	}
	m->setTopLevel( topLevel );

	bool ok = false;

	// get each protocol's contact
	QMap <KopeteAccount*,AddContactPage*>::Iterator it;
	for ( it = protocolPages.begin(); it != protocolPages.end(); ++it )
		ok |= it.data()->apply( it.key(), metaContact );

	// add it to the contact list
	if ( ok )
		KopeteContactList::contactList()->addMetaContact( metaContact );
	else
		delete metaContact;

	deleteLater();
}

void AddContactWizard::next()
{
	// If the we're on the select account page
	// follow it with the add contact pages for
	// the chosen protocols
	if (currentPage() == selectService ||
		(currentPage() == intro && !appropriate( selectService )))
	{
		QMap <KopeteAccount*,AddContactPage*>::Iterator it;
		for ( it = protocolPages.begin(); it != protocolPages.end(); ++it )
		{
			delete it.data();
		}
		protocolPages.clear();

		// We don't keep track of this pointer because it gets deleted when the wizard does (which is what we want)
		for (QListViewItemIterator it(protocolListView); it.current(); ++it)
		{
			QCheckListItem *item = dynamic_cast<QCheckListItem *>(it.current());
			if (item && item->isOn())
			{
				// this shouldn't happen either, but I hate crashes
				if (!m_accountItems[item]) continue;

				AddContactPage *addPage = m_accountItems[item]->protocol()->createAddContactWidget(this, m_accountItems[item] );
				if (!addPage) continue;

				QString title = i18n( "The account name is prepended here",
									 "%1 contact information" )
									 .arg( item->text(0) );
				addPage->show();
				insertPage( addPage, title, indexOf( finis ) );
				protocolPages.insert( m_accountItems[item] , addPage );
			}
		}
		QWizard::next();
		return;
	}

	// If we're not on any account specific pages,
	// we must be on an add account page, so make sure it validates
	if (currentPage() != intro &&
		currentPage() != selectAddressee &&
		currentPage() != selectService &&
		currentPage() != selectGroup &&
		currentPage() != finis)
	{
		AddContactPage *ePage = dynamic_cast<AddContactPage *>(currentPage());
		if (!ePage || !ePage->validateData())
			return;
	}

	QWizard::next();
}

void AddContactWizard::showPage( QWidget *page )
{
	if ( page == selectGroup )
	{
		if ( addresseeListView->isEnabled() )
		{
			if ( KABC::AddresseeItem* i = static_cast<KABC::AddresseeItem *>( addresseeListView->selectedItem() ) )
				mDisplayName->setText( i->addressee().realName() );
			else
				mDisplayName->setText( QString::null );
		}
	}
	QWizard::showPage( page );
}

#include "addcontactwizard.moc"

// vim: set noet ts=4 sts=4 sw=4:

