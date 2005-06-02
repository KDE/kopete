/*
    addcontactwizard.h - Kopete's Add Contact Wizard

    Copyright (c) 2004 by Olivier Goffart        <ogoffart @ kde.org>
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
#include <qlayout.h>
#include <q3vbox.h>
//Added by qt3to4:
#include <Q3PtrList>
#include <kapplication.h>
#include <kconfig.h>
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
#include "addressbookselectorwidget.h"
#include "addcontactwizard.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"
#include "kopetegroup.h"

AddContactWizard::AddContactWizard( QWidget *parent, const char *name )
: AddContactWizard_Base( parent, name )
{
    //QVBox *kabcPageVbox = new QVBox(this->page(1));
	m_addressbookSelectorWidget = new Kopete::UI::AddressBookSelectorWidget(this->page(1));
	#warning FIXME find references to selectAddresseeLayout
//	selectAddresseeLayout->addWidget(m_addressbookSelectorWidget);

	// Populate the groups list
	Kopete::GroupList groups=Kopete::ContactList::self()->groups();
	for( Kopete::Group *it = groups.first(); it; it = groups.next() )
	{
		QString groupname = it->displayName();
		if ( !groupname.isEmpty() )
			m_groupItems.insert(new Q3CheckListItem( groupList, groupname, Q3CheckListItem::CheckBox) , it ) ;
	}

	protocolListView->clear();
	m_accountItems.clear();

	// Populate the accounts list
	Q3CheckListItem* accountLVI = 0;
	Q3PtrList<Kopete::Account>  accounts = Kopete::AccountManager::self()->accounts();
	for(Kopete::Account *i=accounts.first() ; i; i=accounts.next() )
	{
		accountLVI= new Q3CheckListItem( protocolListView, i->accountLabel(), Q3CheckListItem::CheckBox);
		accountLVI->setText(1,i->protocol()->displayName() + QString::fromLatin1(" ") );
		//FIXME - I'm not sure the column 1 is a right place for the colored icon -Olivier
		accountLVI->setPixmap( 1, i->accountIcon() );
		m_accountItems.insert(accountLVI,i);
	}
	protocolListView->setCurrentItem( protocolListView->firstChild() );
	groupList->setCurrentItem( groupList->firstChild() );

	if ( accounts.count() == 1 )
	{
		accountLVI->setOn( true );
		setAppropriate( selectService, false );
	}

	setNextEnabled( selectService, accounts.count() == 1 );
	setNextEnabled( selectAddressee, false );
	setFinishEnabled( finis, true );

	// Addressee validation connections
	connect( chkAddressee, SIGNAL( toggled( bool ) ),
			SLOT( slotCheckAddresseeChoice( bool ) ) );
	connect( m_addressbookSelectorWidget, SIGNAL(addresseeListClicked( Q3ListViewItem * )), SLOT(slotAddresseeListClicked( Q3ListViewItem * )) );

	// Group manipulation connection
	connect( addGroupButton, SIGNAL(clicked()) , SLOT(slotAddGroupClicked()) );

	// Account choice validation connections
	connect( protocolListView, SIGNAL(clicked(Q3ListViewItem *)), this, SLOT(slotProtocolListClicked(Q3ListViewItem *)));
	connect( protocolListView, SIGNAL(selectionChanged(Q3ListViewItem *)), this, SLOT(slotProtocolListClicked(Q3ListViewItem *)));
	connect( protocolListView, SIGNAL(spacePressed(Q3ListViewItem *)), this, SLOT(slotProtocolListClicked(Q3ListViewItem *)));

	// read sticky settings
	KConfig *config = kapp->config();
	config->setGroup("Add Contact Wizard");
	bool useKABC = config->readBoolEntry( "UseAddressBook", false );
	chkAddressee->setChecked( useKABC );
	setAppropriate( selectAddressee, useKABC );
	// load address book, if using KABC
	slotCheckAddresseeChoice( useKABC );
}


AddContactWizard::~AddContactWizard()
{
}

void AddContactWizard::slotCheckAddresseeChoice( bool on )
{
	setAppropriate( selectAddressee, on );
}

void AddContactWizard::slotAddresseeListClicked( Q3ListViewItem */*addressee*/ )
{
	// enable next if a valid addressee is selected
	bool selected = m_addressbookSelectorWidget->addresseeSelected();
	setNextEnabled( selectAddressee, selected );

	if ( selected )
		mDisplayName->setText( m_addressbookSelectorWidget->addressee().realName() );
}

void AddContactWizard::slotAddGroupClicked()
{
	QString groupName = KInputDialog::getText(
		i18n( "New Group" ),
		i18n( "Please enter the name for the new group:" )
		);
	if ( !groupName.isNull() )
		( new Q3CheckListItem( groupList, groupName, Q3CheckListItem::CheckBox ) )->setOn( true );
}

void AddContactWizard::slotProtocolListClicked( Q3ListViewItem *)
{
	// Just makes sure a protocol is selected before allowing the user to continue
	bool oneIsChecked = false;

	for (Q3ListViewItemIterator it(protocolListView); it.current(); ++it)
	{
		Q3CheckListItem *check = dynamic_cast<Q3CheckListItem *>(it.current());
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
	Kopete::MetaContact *metaContact = new Kopete::MetaContact();

	// set the display name if required
	if ( !mDisplayName->text().isEmpty() )
	{
		metaContact->setNameSource( 0 );
		metaContact->setDisplayName( mDisplayName->text() );
	}
	
	// set the metacontact's groups
	bool topLevel = true;
	for ( Q3ListViewItemIterator it( groupList ); it.current(); ++it )
	{
		Q3CheckListItem *check = dynamic_cast<Q3CheckListItem *>( it.current() );
		if ( check && check->isOn() )
		{
			if(m_groupItems.contains(check))
				metaContact->addToGroup(m_groupItems[check]);
			else //it's a new group
				metaContact->addToGroup( Kopete::ContactList::self()->findGroup( check->text() ) );
			topLevel = false;
		}
	}
	if(topLevel)
		metaContact->addToGroup( Kopete::Group::topLevel() );

	bool ok = protocolPages.isEmpty();

	// get each protocol's contact
	QMap <Kopete::Account*,AddContactPage*>::Iterator it;
	for ( it = protocolPages.begin(); it != protocolPages.end(); ++it )
		ok |= it.data()->apply( it.key(), metaContact );

	if ( ok )
	{
		if ( chkAddressee->isChecked() && m_addressbookSelectorWidget->addresseeSelected() )
			metaContact->setMetaContactId( m_addressbookSelectorWidget->addressee().uid() );

		// add it to the contact list
		Kopete::ContactList::self()->addMetaContact( metaContact );
	}
	else
		delete metaContact;

	// write sticky settings
	KConfig *config = kapp->config();
	config->setGroup("Add Contact Wizard");
	config->writeEntry( "UseAddressBook", chkAddressee->isChecked() );
	config->sync();
	deleteLater();
}

void AddContactWizard::reject()
{
	Q3Wizard::reject();
}

void AddContactWizard::next()
{
	// If the we're on the select account page
	// follow it with the add contact pages for
	// the chosen protocols
	if (currentPage() == selectService ||
		(currentPage() == intro && !appropriate( selectService )))
	{
		QStringList usedAccounts;
		// We don't keep track of this pointer because it gets deleted when the wizard does (which is what we want)
		for (Q3ListViewItemIterator it(protocolListView); it.current(); ++it)
		{
			Q3CheckListItem *item = dynamic_cast<Q3CheckListItem *>(it.current());
			if (item && item->isOn())
			{
				Kopete::Account *i=m_accountItems[item];
				// this shouldn't happen either, but I hate crashes
				if (!i)
					continue;

				usedAccounts.append( i->protocol()->pluginId() + i->accountId() );

				if(protocolPages.contains(i))
					continue;

				AddContactPage *addPage = i->protocol()->createAddContactWidget(this, i );
				if (!addPage)
					continue;

				connect(addPage, SIGNAL(dataValid( AddContactPage *, bool )),
					this, SLOT( slotDataValid( AddContactPage *, bool )));
				addPage->show();

				insertPage( addPage, i18n( "The user has to select the contact to add to the given account name", 
					"Choose New Contact For %1 Account <b>%2</b>" ).arg( i->protocol()->displayName() ).arg( item->text(0) ), indexOf( finis ) );
				protocolPages.insert( i , addPage );
			}
		}

		//remove pages that were eventualy added previusely, and needs to be removed if the user pressed back.
		QMap <Kopete::Account*,AddContactPage*>::Iterator it;
		for ( it = protocolPages.begin(); it != protocolPages.end(); ++it )
		{
			Kopete::Account *i=it.key();
			if( !i || !usedAccounts.contains( i->protocol()->pluginId() + i->accountId() ) )
			{
				delete it.data();
				protocolPages.remove(it);
			}
		}
		Q3Wizard::next();
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

	Q3Wizard::next();
}

void AddContactWizard::showPage( QWidget *page )
{
	if ( page == intro )
		setNextEnabled( page, true); // make sure the first page's Next is always enabled

	Q3Wizard::showPage( page );

	if ( page == finis )
		finishButton()->setFocus();
}

void AddContactWizard::slotDataValid(AddContactPage *onPage, bool bOn)
{
	// some plugins emit dataValid when they are not visible.
	// so we need to enable the page which is signalling, not just the current page
	setNextEnabled( onPage, bOn);
}


#include "addcontactwizard.moc"

// vim: set noet ts=4 sts=4 sw=4:

