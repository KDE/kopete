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

#include <qlayout.h>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>

#include "addcontactpage.h"
#include "addcontactwizard.h"
#include "kopetegroup.h"
#include "kopetecontactlist.h"
#include "kopeteprotocol.h"
#include "pluginloader.h"
#include "protocolboxitem.h"
#include "kopetemetacontact.h"

AddContactWizard::AddContactWizard( QWidget *parent, const char *name )
: AddContactWizard_Base( parent, name )
{
	QStringList groups = KopeteContactList::contactList()->groups().toStringList();

	QStringList::ConstIterator it = groups.begin();
	for( ; it != groups.end(); ++it )
	{
		QString groupname = *it;

		if ( !groupname.isNull() )
			new QCheckListItem( groupList, groupname, QCheckListItem::CheckBox);
	}

	int pluginCount = 0;
	ProtocolBoxItem *pluginItem = 0L;

	QPtrList<KopetePlugin> plugins = LibraryLoader::pluginLoader()->plugins();
	for( KopetePlugin *p = plugins.first() ; p ; p = plugins.next() )
	{
		KopeteProtocol *proto = dynamic_cast<KopeteProtocol*>( p );
		if( proto )
		{
			pluginItem = new ProtocolBoxItem( protocolListView, proto->pluginId() );
			pluginItem->protocol = proto;
			pluginCount++;
		}
	}

	if ( pluginCount == 1 )
	{
		pluginItem->setOn( true );
		setAppropriate( selectService, false );
	}

	setNextEnabled(selectService, (pluginCount == 1));
	setFinishEnabled(finis, true);

	connect( addGroupButton, SIGNAL(clicked()) , SLOT(slotAddGroupClicked()) );
	connect( protocolListView, SIGNAL(clicked(QListViewItem *)), this, SLOT(slotProtocolListClicked(QListViewItem *)));
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

	setNextEnabled(selectGroup ,oneIsChecked);
}

void AddContactWizard::slotGroupListClicked( QListViewItem *)
{
	//top-level contacts are allowed
/*
	bool oneIsChecked = false;
	for (QListViewItem *listItem = groupList->firstChild(); listItem != 0; listItem = listItem->itemBelow())
	{
		QCheckListItem *check = dynamic_cast<QCheckListItem *>(listItem);
		if (!check) 
		{
			kdDebug(14000) << "WARNING : AddContactWizard::slotGroupListClicked : one listItem is not a CheckListItem" << endl;
		}
		else 	if (check->isOn())
		{
			oneIsChecked = true;
			break;
		}
	}
	setNextEnabled(page2,oneIsChecked);
  */
}

void AddContactWizard::slotAddGroupClicked()
{
	bool ok;
	QString groupName = KLineEditDlg::getText(
		i18n( "New Group - Kopete" ),
		i18n( "Please enter the name for the new group" ),
		QString::null, &ok );

	if ( !groupName.isNull() && ok)
		new QCheckListItem( groupList, groupName, QCheckListItem::CheckBox);
}

void AddContactWizard::slotRemoveGroupClicked()
{
}
#include <iostream>
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
	
cout << "finishing page: " << protocolPages.first() << endl;
	for (AddContactPage *ePage = protocolPages.first(); 
		 ePage; 
		 ePage = protocolPages.next())
	{
cout << "finishing page: " << ePage << endl;
		ePage->slotFinish(m); 
	}
	KopeteContactList::contactList()->addMetaContact(m);
	delete(this);
}

void AddContactWizard::next()
{
	if (currentPage() == selectService ||
		(currentPage() == intro && !appropriate( selectService )))
	{
		for (AddContactPage *ePage = protocolPages.first(); 
			 ePage != 0; 
			 ePage = protocolPages.first())
		{
			protocolPages.remove(ePage);
			delete ePage;
		}

		// We don't keep track of this pointer because it gets deleted when the wizard does (which is what we want)
		for (QListViewItemIterator it(protocolListView); it.current(); ++it)
		{
			ProtocolBoxItem *item = dynamic_cast<ProtocolBoxItem *>(it.current());
			if (item && item->isOn())
			{
				// this shouldn't happen either, but I hate crashes
				if (!item->protocol) continue;

				AddContactPage *addPage = item->protocol->createAddContactWidget(this);
				if (!addPage) continue;

				QString title = i18n( "The protocol name is prepended here",
									  "%1 contact information" )
									 .arg( item->text() );
				addPage->show();
				insertPage( addPage, title, indexOf( selectGroup) );
				protocolPages.append( addPage );
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

