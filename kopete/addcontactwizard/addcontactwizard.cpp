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


#include <klocale.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <kpushbutton.h>
#include <kdebug.h>
#include <klistview.h>

#include <addcontactpage.h>
#include "addcontactwizard.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"

AddContactWizard::AddContactWizard( QWidget *parent, const char *name )
: AddContactWizard_Base( parent, name )
{
	KopeteGroupList groups=KopeteContactList::contactList()->groups();
	for( KopeteGroup *it = groups.first(); it; it = groups.next() )
	{
		QString groupname = it->displayName();
		if ( !groupname.isEmpty() )
			new QCheckListItem( groupList, groupname, QCheckListItem::CheckBox);
	}

	protocolListView->clear();
	m_accountItems.clear();

	QCheckListItem* accountLVI=0L;
	QPtrList<KopeteAccount>  accounts = KopeteAccountManager::manager()->accounts();
	for(KopeteAccount *i=accounts.first() ; i; i=accounts.next() )
	{
		accountLVI= new QCheckListItem( protocolListView, i->accountId(), QCheckListItem::CheckBox);
		accountLVI->setText(1,i->protocol()->displayName() + QString::fromLatin1(" ") );
		accountLVI->setPixmap( 1, SmallIcon( i->protocol()->pluginIcon() ) );
		m_accountItems.insert(accountLVI,i);
	}


	if ( accounts.count() == 1 )
	{
		accountLVI->setOn( true );
		setAppropriate( selectService, false );
	}

	setNextEnabled(selectService, (accounts.count() == 1));
	setFinishEnabled(finis, true);

	connect( addGroupButton, SIGNAL(clicked()) , SLOT(slotAddGroupClicked()) );
	connect( protocolListView, SIGNAL(clicked(QListViewItem *)), this, SLOT(slotProtocolListClicked(QListViewItem *)));
	connect( protocolListView, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(slotProtocolListClicked(QListViewItem *)));
	connect( protocolListView, SIGNAL(spacePressed(QListViewItem *)), this, SLOT(slotProtocolListClicked(QListViewItem *)));
}

AddContactWizard::~AddContactWizard()
{
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

void AddContactWizard::slotAddGroupClicked()
{
	bool ok;
	QString groupName = KLineEditDlg::getText(
		i18n( "New Group - Kopete" ),
		i18n( "Please enter the name for the new group:" ),
		QString::null, &ok );

	if ( !groupName.isNull() && ok)
		new QCheckListItem( groupList, groupName, QCheckListItem::CheckBox);
}


void AddContactWizard::accept()
{
	KopeteMetaContact *m = new KopeteMetaContact();
	bool topLevel = true;
	for (QListViewItemIterator it(groupList); it.current(); ++it)
	{
		QCheckListItem *check = dynamic_cast<QCheckListItem *>(it.current());
		if (check && check->isOn())
		{
			m->addToGroup(KopeteContactList::contactList()->getGroup(check->text()));
			topLevel = false;
		}
	}
	m->setTopLevel(topLevel);

	bool ok=false;

	QMap <KopeteAccount*,AddContactPage*>::Iterator it;
	for ( it = protocolPages.begin(); it != protocolPages.end(); ++it ) 
	{
		ok |= it.data()->apply(it.key(),m); 
	}
	if(ok)
		KopeteContactList::contactList()->addMetaContact(m);
	else
		delete m;
	delete this;
}

void AddContactWizard::next()
{
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
				insertPage( addPage, title, indexOf( selectGroup) );
				protocolPages.insert( m_accountItems[item] , addPage );
			}
		}
		QWizard::next();
		return;
	}
	if (currentPage() != intro && 
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

#include "addcontactwizard.moc"

// vim: set noet ts=4 sts=4 sw=4:

