/*
    fastaddcontactwizard.cpp - Kopete's FastAdd Contact Wizard

	Copyright (c) 2003 by Will Stephenson		 <will@stevello.free-online.co.uk>
	Derived from AddContactWizard
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
#include <addcontactpage.h>

#include <kiconloader.h>
#include <klocale.h>

#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"

#include "fastaddcontactwizard.h"

FastAddContactWizard::FastAddContactWizard( QWidget *parent, const char *name )
: FastAddContactWizard_Base( parent, name )
{
	m_accountItems.clear();

	// Populate the accounts list
	Q3ListViewItem* accountLVI = 0L;
	QList<Kopete::Account*>  accounts = Kopete::AccountManager::self()->accounts();
	for(Kopete::Account *i=accounts.first() ; i; i=accounts.next() )
	{
		accountLVI= new Q3ListViewItem( protocolListView, i->accountLabel() );
		accountLVI->setText(1,i->protocol()->displayName() + QString::fromLatin1(" ") );
		accountLVI->setPixmap( 1, SmallIcon( i->protocol()->pluginIcon() ) );
		m_accountItems.insert(accountLVI,i);
	}

	if ( accounts.count() == 1 )
		protocolListView->setCurrentItem( accountLVI );

	// Account choice validation connections
	connect( protocolListView, SIGNAL(clicked(Q3ListViewItem *)), this, SLOT(slotProtocolListClicked(Q3ListViewItem *)));
	connect( protocolListView, SIGNAL(selectionChanged(Q3ListViewItem *)), this, SLOT(slotProtocolListClicked(Q3ListViewItem *)));
	connect( protocolListView, SIGNAL(spacePressed(Q3ListViewItem *)), this, SLOT(slotProtocolListClicked(Q3ListViewItem *)));

	setNextEnabled( selectService, false );
	setFinishEnabled(finis, true);
}

FastAddContactWizard::~FastAddContactWizard()
{
}

void FastAddContactWizard::slotProtocolListClicked( Q3ListViewItem *account)
{
	setNextEnabled( selectService, account? account->isSelected() : false );
}

void FastAddContactWizard::next()
{
	// If we're on the select account page
	// follow it with the add contact page for
	// the chosen protocol
	if ( currentPage() == selectService )
	{
		QMap <Kopete::Account*,AddContactPage*>::Iterator it;
		for ( it = protocolPages.begin(); it != protocolPages.end(); ++it )
		{
			delete it.data();
		}
		protocolPages.clear();

		Q3ListViewItem* item = protocolListView->selectedItem();
		AddContactPage *addPage = m_accountItems[item]->protocol()->createAddContactWidget(this, m_accountItems[item] );
		if (addPage)
		{
			QString title = i18n( "The account name is prepended here",
								 "%1 contact information" )
								 .arg( item->text(0) );
			addPage->show();
			insertPage( addPage, title, indexOf( finis ) );
			protocolPages.insert( m_accountItems[item] , addPage );
		}
		Q3Wizard::next();
		return;
	}

	// If we're not on any account specific pages,
	// we must be on an add account page, so make sure it validates
	if ( currentPage() != selectService && currentPage() != finis )
	{
		AddContactPage *ePage = dynamic_cast<AddContactPage *>(currentPage());
		if (!ePage || !ePage->validateData())
			return;
	}

	Q3Wizard::next();
}

void FastAddContactWizard::accept()
{
	Kopete::MetaContact *metaContact = new Kopete::MetaContact();

	metaContact->addToGroup( Kopete::Group::topLevel() );

	bool ok = protocolPages.isEmpty();

	// get each protocol's contact
	QMap <Kopete::Account*,AddContactPage*>::Iterator it;
	for ( it = protocolPages.begin(); it != protocolPages.end(); ++it )
		ok |= it.data()->apply( it.key(), metaContact );

	if ( ok )
	{
		// add it to the contact list
		Kopete::ContactList::self()->addMetaContact( metaContact );
	}
	else
		delete metaContact;

	deleteLater();
}

#include "fastaddcontactwizard.moc"
