/*
    addcontactwizard.h - Kopete's Add Contact Wizard

    Copyright (c) 2003 by Will Stephenson        <will@stevello.free-online.co.uk>
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
#include <kinputdialog.h>
#include <kinputdialog.h>

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
#include "kopetegroup.h"

AddContactWizard::AddContactWizard( QWidget *parent, const char *name )
: AddContactWizard_Base( parent, name )
{
	m_addressBook = 0L;  // so we can tell if it's already loaded

	// Populate the groups list
	KopeteGroupList groups=KopeteContactList::contactList()->groups();
	for( KopeteGroup *it = groups.first(); it; it = groups.next() )
	{
		QString groupname = it->displayName();
		if ( !groupname.isEmpty() )
			m_groupItems.insert(new QCheckListItem( groupList, groupname, QCheckListItem::CheckBox) , it ) ;
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
		//FIXME - I'm not sure the column 1 is a right place for the colored icon -Olivier
		accountLVI->setPixmap( 1, i->accountIcon() );
		m_accountItems.insert(accountLVI,i);
	}

	if ( accounts.count() == 1 )
	{
		accountLVI->setOn( true );
		setAppropriate( selectService, false );
	}

	setNextEnabled( selectService, ( accounts.count() == 1 ) );
	setNextEnabled( selectAddressee, false );
	setFinishEnabled( finis, true );

	// Addressee validation connections
	connect( addAddresseeButton, SIGNAL( clicked() ), SLOT( slotAddAddresseeClicked() ) );
	connect( chkAddressee, SIGNAL( toggled( bool ) ),
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

void AddContactWizard::slotLoadAddressees()
{
	addresseeListView->clear();
	KABC::AddressBook::Iterator it;
	for( it = m_addressBook->begin(); it != m_addressBook->end(); ++it )
		/*KABC::AddresseeItem *item =*/ new KABC::AddresseeItem( addresseeListView, (*it) );
}

void AddContactWizard::slotAddAddresseeClicked()
{
	// Pop up add addressee dialog
#if KDE_IS_VERSION (3,1,90)
	QString addresseeName = KInputDialog::getText( i18n( "New Address Book Entry" ),
												   i18n( "Name the new entry:" ),
												   QString::null, 0, this );
#else
	QString addresseeName = KLineEditDlg::getText( i18n( "New Address Book Entry" ),
												   i18n( "Name the new entry:" ),
												   QString::null, 0, this );
#endif

	if ( !addresseeName.isEmpty() )
	{
		KABC::Addressee addr;
		addr.setNameFromString( addresseeName );
		m_addressBook->insertAddressee( addr );
		KABC::Ticket *ticket = m_addressBook->requestSaveTicket();
		if ( !ticket )
		{
			kdError() << "Resource is locked by other application!" << endl;
		}
		else
		{
			if ( !m_addressBook->save( ticket ) )
			{
				kdError() << "Saving failed!" << endl;
#if KDE_IS_VERSION (3,1,90)
				m_addressBook->releaseSaveTicket( ticket );
#endif
			}
		}
	}
}

void AddContactWizard::slotCheckAddresseeChoice( bool on )
{
	setAppropriate( selectAddressee, on );
	if ( on )
	{
		// Get a reference to the address book
		if ( !m_addressBook )
		{
			m_addressBook = KABC::StdAddressBook::self( true );
			KABC::StdAddressBook::setAutomaticSave( false );
		}
		disconnect( m_addressBook, SIGNAL( addressBookChanged( AddressBook * ) ),
										   this, SLOT( slotLoadAddressees() ) );
		connect( m_addressBook, SIGNAL( addressBookChanged( AddressBook * ) ),
										this, SLOT( slotLoadAddressees() ) );
	}
	else
		disconnect( m_addressBook, SIGNAL( addressBookChanged( AddressBook * ) ),
										   this, SLOT( slotLoadAddressees() ) );
}

void AddContactWizard::slotAddresseeListClicked( QListViewItem *addressee )
{
	// enable next if a valid addressee is selected
	setNextEnabled( selectAddressee, addressee ? addressee->isSelected() : false );

	if ( KABC::AddresseeItem* i = static_cast<KABC::AddresseeItem *>( addressee ) )
		mDisplayName->setText( i->addressee().realName() );
}

void AddContactWizard::slotAddGroupClicked()
{
	QString groupName = KInputDialog::getText(
		i18n( "New Group" ),
		i18n( "Please enter the name for the new group:" )
		);
	if ( !groupName.isNull() )
		( new QCheckListItem( groupList, groupName, QCheckListItem::CheckBox ) )->setOn( true );
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
	// NOT SURE IF I MEANT TO TAKE THIS OUT - WILL
	//// set the KABC uid in the metacontact
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
			if(m_groupItems.contains(check))
				metaContact->addToGroup(m_groupItems[check]);
			else //it's a new group
				metaContact->addToGroup( KopeteContactList::contactList()->getGroup( check->text() ) );
			topLevel = false;
		}
	}
	if(topLevel)
		metaContact->addToGroup( KopeteGroup::topLevel() );

	bool ok = protocolPages.isEmpty();

	// get each protocol's contact
	QMap <KopeteAccount*,AddContactPage*>::Iterator it;
	for ( it = protocolPages.begin(); it != protocolPages.end(); ++it )
		ok |= it.data()->apply( it.key(), metaContact );

	if ( ok )
	{
		// set the KABC uid in the metacontact
		KABC::AddresseeItem* i = 0L;
		i = static_cast<KABC::AddresseeItem *>( addresseeListView->selectedItem() );
		if ( addresseeListView->isEnabled() && i )
			metaContact->setMetaContactId( i->addressee().uid() );
		// add it to the contact list
		KopeteContactList::contactList()->addMetaContact( metaContact );
	}
	else
		delete metaContact;

	disconnect( m_addressBook, SIGNAL( addressBookChanged( AddressBook * ) ), this, SLOT( slotLoadAddressees() ) );

	deleteLater();
}

void AddContactWizard::reject()
{
	disconnect( m_addressBook, SIGNAL( addressBookChanged( AddressBook * ) ), this, SLOT( slotLoadAddressees() ) );
	QWizard::reject();
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
				if (!m_accountItems[item])
					continue;

				AddContactPage *addPage = m_accountItems[item]->protocol()->createAddContactWidget(this, m_accountItems[item] );
				if (!addPage)
					continue;

				connect(addPage, SIGNAL(dataValid(bool)),
					this, SLOT(slotDataValid(bool)));
				addPage->show();

				insertPage( addPage, i18n( "The account name is prepended here",
					"%1 contact information" ).arg( item->text(0) ), indexOf( finis ) );
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
	if ( page == intro )
	{
		if ( chkAddressee->isChecked() && addresseeListView->firstChild() == 0 ) // We must check this as we might be showing this page because the back button was pressed
		{
			// Get a reference to the address book
			if ( m_addressBook == 0L )
			{
				m_addressBook = KABC::StdAddressBook::self( true );
				KABC::StdAddressBook::setAutomaticSave( false );
			}
			disconnect( m_addressBook, SIGNAL( addressBookChanged( AddressBook * ) ), this, SLOT( slotLoadAddressees() ) );
			connect( m_addressBook, SIGNAL( addressBookChanged( AddressBook * ) ), this, SLOT( slotLoadAddressees() ) );
			slotLoadAddressees();
		}
	}
	QWizard::showPage( page );
}

void AddContactWizard::slotDataValid(bool bOn)
{
	setNextEnabled(currentPage(), bOn);
}


#include "addcontactwizard.moc"

// vim: set noet ts=4 sts=4 sw=4:

