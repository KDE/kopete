/*
    addaccountwizard.cpp - Kopete Add Account Wizard

    Copyright (c) 2003-2006 by Olivier Goffart       <ogoffart @ kde.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2003-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "addaccountwizard.h"

#include <qcheckbox.h>
#include <qlabel.h>

#include <kcolorbutton.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <k3listview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kplugininfo.h>
#include <kvbox.h>

#include "editaccountwidget.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopeteprotocol.h"
#include "kopetepluginmanager.h"

AddAccountWizard::AddAccountWizard( QWidget *parent, bool firstRun )
	: 
	KAssistantDialog(parent),
	m_accountPage(0),
	m_proto(0)
{
	// setup the select service page
	m_selectService = new QWidget(this);
	m_uiSelectService.setupUi(m_selectService);
	m_uiSelectService.protocolListView->setColumnCount( 2 );
	QStringList header;
	header << i18n("Name") << i18n("Description");
	m_uiSelectService.protocolListView->setHeaderLabels( header );
	if ( firstRun )
		m_uiSelectService.m_header->setText( i18nc( "1st message shown to users on first run of Kopete. Please keep the formatting.", "<h2>Welcome to Kopete</h2><p>Which messaging service do you want to connect to?</p>") );
	
	m_selectServiceItem = addPage(m_selectService,m_selectService->windowTitle());
	setValid(m_selectServiceItem, false);
		
	m_accountPageWidget = new KVBox(this);
	addPage(m_accountPageWidget,i18n("Step Two: Account Information"));

	// setup the final page
	m_finish = new QWidget(this);
	m_uiFinish.setupUi(m_finish);
	if ( firstRun )
		m_uiFinish.m_header->setText( i18nc( "2nd message shown to users on first run of Kopete. Please keep the formatting.", "<h2>Congratulations</h2><p>You have finished configuring the account. You can add more accounts with <i>Settings->Configure</i>.  Please click the \"Finish\" button.</p>") );
	addPage(m_finish,m_finish->windowTitle());

	// add the available messenger services to the dialogs list
	QList<KPluginInfo *> protocols = Kopete::PluginManager::self()->availablePlugins("Protocols");
	for (QList<KPluginInfo *>::Iterator it = protocols.begin(); it != protocols.end(); ++it)
	{
		QTreeWidgetItem *pluginItem = new QTreeWidgetItem(m_uiSelectService.protocolListView);
		pluginItem->setIcon(0, QIcon(SmallIcon((*it)->icon())));
		pluginItem->setText(0, (*it)->name());
		pluginItem->setText(1, (*it)->comment());

		m_protocolItems.insert(pluginItem, *it);
	}

	// focus the ListView and select the first item
	QTreeWidget *protocol_list = m_uiSelectService.protocolListView;
	protocol_list->setFocus();
	if (protocol_list->topLevelItemCount() > 0)
		protocol_list->setItemSelected( protocol_list->topLevelItem(0), true );
	else
		protocol_list->setItemSelected( protocol_list->topLevelItem(0), false );
	
	
 
	// hook up the user input
	connect(m_uiSelectService.protocolListView, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
		this, SLOT(slotProtocolListClicked()));
	connect(m_uiSelectService.protocolListView, SIGNAL(itemSelectionChanged()),
		this, SLOT( slotProtocolListClicked()));
	connect(m_uiSelectService.protocolListView, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
		this, SLOT(slotProtocolListDoubleClicked()));
}

QTreeWidgetItem* AddAccountWizard::selectedProtocol()
{
	QList<QTreeWidgetItem*> selectedItems = m_uiSelectService.protocolListView->selectedItems();
	if(!selectedItems.empty())
 		return selectedItems.first();
	return 0;
}

void AddAccountWizard::slotProtocolListClicked()
{
	// Make sure a protocol is selected before allowing the user to continue
	setValid(m_selectServiceItem, selectedProtocol() != 0);
}

void AddAccountWizard::slotProtocolListDoubleClicked()
{
	// proceed to the next wizard page if we double click a protocol
	next();
}

void AddAccountWizard::back()
{
	if (currentPage()->widget() == m_accountPageWidget)
	{
		// Deletes the accountPage, K3Wizard does not like deleting pages
		// using different pointers, it only seems to watch its own pointer
		delete m_accountPage;
		m_accountPage = 0;
		m_proto       = 0;

		// removePage() already goes back to previous page, no back() needed
	}
	KAssistantDialog::back();
}

void AddAccountWizard::next()
{
	if (currentPage()->widget() == m_selectService)
	{
		QTreeWidgetItem *lvi = selectedProtocol();
		if(!m_protocolItems[lvi])
		{ //no item selected
			return;
		}
		m_proto = qobject_cast<Kopete::Protocol *>(Kopete::PluginManager::self()->loadPlugin(m_protocolItems[lvi]->pluginName()));
		if (!m_proto)
		{
			KMessageBox::queuedMessageBox(this, KMessageBox::Error,
				i18n("Cannot load the %1 protocol plugin.", m_protocolItems[lvi]->name()), 
				i18n("Error While Adding Account"));
			return;
		}

		m_accountPage = m_proto->createEditAccountWidget(0, m_accountPageWidget);
		if (!m_accountPage)
		{
			KMessageBox::queuedMessageBox(this, KMessageBox::Error,
				i18n("This protocol does not currently support adding accounts."),
				i18n("Error While Adding Account"));
			return;
		}
	
		KAssistantDialog::next();
	}
	else if (currentPage()->widget() == m_accountPageWidget)
	{
		// check the data of the page is valid
		if (!m_accountPage->validateData())
		{
			return;
		}

		QColor col = Kopete::AccountManager::self()->guessColor(m_proto);

		m_uiFinish.mColorButton->setColor(col);
		m_uiFinish.mUseColor->setChecked(col.isValid());
		KAssistantDialog::next();
	}
	else 
	{
		kDebug(14100) << k_funcinfo << "Next pressed on misc page" << endl;
		KAssistantDialog::next();
	}

}

void AddAccountWizard::accept()
{
	// registeredAccount shouldn't probably be called here. Anyway, if the account is already registered, 
	// it won't be registered twice
	Kopete::AccountManager *manager = Kopete::AccountManager::self();
	Kopete::Account        *account = manager->registerAccount(m_accountPage->apply());

	// if the account wasn't created correctly then leave
	if (!account)
	{
		return;
	}

	// Make sure the protocol is correctly enabled.  This is not really needed, but still good
	const QString PROTO_NAME = m_proto->pluginId().remove("Protocol").toLower();
	Kopete::PluginManager::self()->setPluginEnabled(PROTO_NAME , true);

	// setup the custom colour
	if (m_uiFinish.mUseColor->isChecked())
	{
		account->setColor(m_uiFinish.mColorButton->color());
	}

	// connect if necessary
	if (m_uiFinish.mConnectNow->isChecked())
	{
		account->connect();
	}

	KAssistantDialog::accept();
}

void AddAccountWizard::reject()
{
    // if we have a protocol plugin loaded and its not being used, unload it
	if (m_proto)
	{
		bool hasAccount=false;
		foreach( Kopete::Account *act, Kopete::AccountManager::self()->accounts() )
		{
			if( act->protocol() == m_proto )
			{
				hasAccount=true;
				break;
			}
		}
		if(hasAccount)
		{
			const QString PROTO_NAME = m_proto->pluginId().remove("Protocol").toLower();
			Kopete::PluginManager::self()->unloadPlugin(PROTO_NAME);
		}
	}

	KAssistantDialog::reject();
}

AddAccountWizard::~AddAccountWizard()
{
}

#include "addaccountwizard.moc"

// vim: set noet ts=4 sts=4 sw=4:

