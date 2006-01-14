/*
    addaccountwizard.cpp - Kopete Add Account Wizard

    Copyright (c) 2003-2004 by Olivier Goffart       <ogoffart @ kde.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2003-2004 by the Kopete developers <kopete-devel@kde.org>

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
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kplugininfo.h>

#include "addaccountwizardpage1.h"
#include "addaccountwizardpage2.h"
#include "editaccountwidget.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopeteprotocol.h"
#include "kopetepluginmanager.h"

AddAccountWizard::AddAccountWizard( QWidget *parent, const char *name, bool modal, bool firstRun )
	: 
	KWizard(parent, name, modal, WDestructiveClose),
	m_accountPage(0),
	m_proto(0)
{
	// setup the select service page
	m_selectService = new AddAccountWizardPage1(this);
  if ( firstRun )
		m_selectService->m_header->setText( i18n( "1st message shown to users on first run of Kopete. Please keep the formatting.", "<h2>Welcome to Kopete</h2><p>Which messaging service do you want to connect to?</p>") );
	addPage(m_selectService, m_selectService->caption());
	setNextEnabled(m_selectService, false);

	// setup the final page
	m_finish = new AddAccountWizardPage2(this);
  if ( firstRun )
		m_finish->m_header->setText( i18n( "2nd message shown to users on first run of Kopete. Please keep the formatting.", "<h2>Congratulations</h2><p>You have finished configuring the account. You can add more accounts with <i>Settings->Configure</i>.  Please click the \"Finish\" button.</p>") );
	addPage(m_finish, m_finish->caption());
	setFinishEnabled(m_finish, true);

	// add the available messanger services to the dialogs list
	QValueList<KPluginInfo *> protocols = Kopete::PluginManager::self()->availablePlugins("Protocols");
	for (QValueList<KPluginInfo *>::Iterator it = protocols.begin(); it != protocols.end(); ++it)
	{
		QListViewItem *pluginItem = new QListViewItem(m_selectService->protocolListView);
		pluginItem->setPixmap(0, SmallIcon((*it)->icon()));
		pluginItem->setText(0, (*it)->name());
		pluginItem->setText(1, (*it)->comment());

		m_protocolItems.insert(pluginItem, *it);
	}

	// focus the ListView and select the first item
	QListView &protocol_list = *m_selectService->protocolListView;
	protocol_list.setFocus();
	if (protocol_list.childCount() > 0)
	{
		protocol_list.setSelected(protocol_list.firstChild(), true);
	}
 
	// hook up the user input
	connect(m_selectService->protocolListView, SIGNAL(clicked(QListViewItem *)),
		this, SLOT(slotProtocolListClicked(QListViewItem *)));
	connect(m_selectService->protocolListView, SIGNAL(selectionChanged(QListViewItem *)),
		this, SLOT( slotProtocolListClicked(QListViewItem *)));
	connect(m_selectService->protocolListView, SIGNAL(doubleClicked(QListViewItem *)),
		this, SLOT(slotProtocolListDoubleClicked(QListViewItem *)));
}

void AddAccountWizard::slotProtocolListClicked( QListViewItem * )
{
	// Make sure a protocol is selected before allowing the user to continue
	setNextEnabled(m_selectService, m_selectService->protocolListView->selectedItem() != 0);
}

void AddAccountWizard::slotProtocolListDoubleClicked( QListViewItem *lvi )
{
	// proceed to the next wizard page if we double click a protocol
	next();
}

void AddAccountWizard::back()
{
	if (currentPage() == dynamic_cast<QWidget *>(m_accountPage))
	{
		// Deletes the accountPage, KWizard does not like deleting pages
		// using different pointers, it only seems to watch its own pointer
		delete currentPage();
        
		m_accountPage = 0;
		m_proto       = 0;

		// removePage() already goes back to previous page, no back() needed
	}
	else
	{
		KWizard::back();
	}
}

void AddAccountWizard::next()
{
	if ( currentPage() == m_selectService &&
		   ( m_selectService->protocolListView->selectedItem() ) )
	{
		QListViewItem *lvi = m_selectService->protocolListView->selectedItem();

		m_proto = dynamic_cast<Kopete::Protocol *>(Kopete::PluginManager::self()->loadPlugin(m_protocolItems[lvi]->pluginName()));
		if (!m_proto)
		{
			KMessageBox::queuedMessageBox(this, KMessageBox::Error,
				i18n("Cannot load the %1 protocol plugin.").arg(m_protocolItems[lvi]->name()), 
				i18n("Error While Adding Account"));
			return;
		}

		m_accountPage = m_proto->createEditAccountWidget(0, this);
		if (!m_accountPage)
		{
			KMessageBox::queuedMessageBox(this, KMessageBox::Error,
				i18n("This protocol does not currently support adding accounts."),
				i18n("Error While Adding Account"));
			return;
		}
	
		insertPage(dynamic_cast<QWidget *>(m_accountPage), i18n("Step Two: Account Information"), indexOf(m_finish));
		KWizard::next();
	}
	else if (currentPage() == dynamic_cast<QWidget *>(m_accountPage))
	{
		// check the data of the page is valid
		if (!m_accountPage->validateData())
		{
			return;
		}

		QColor col = Kopete::AccountManager::self()->guessColor(m_proto);

		m_finish->mColorButton->setColor(col);
		m_finish->mUseColor->setChecked(col.isValid());
		KWizard::next();
	}
	else 
	{
		kdDebug(14100) << k_funcinfo << "Next pressed on misc page" << endl;
		KWizard::next();
	}

    // if it's the finish page, focus the finish button
	if (currentPage() == m_finish)
	{
		finishButton()->setFocus();
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
	const QString PROTO_NAME = m_proto->pluginId().remove("Protocol").lower();
	Kopete::PluginManager::self()->setPluginEnabled(PROTO_NAME , true);

	// setup the custom colour
	if (m_finish->mUseColor->isChecked())
	{
		account->setColor(m_finish->mColorButton->color());
	}

	// connect if neccessary
	if (m_finish->mConnectNow->isChecked())
	{
		account->connect();
	}

	KWizard::accept();
}

void AddAccountWizard::reject()
{
    // if we have a protocol plugin loaded and its not being used, unload it
	if (m_proto && Kopete::AccountManager::self()->accounts(m_proto).isEmpty())
	{
		const QString PROTO_NAME = m_proto->pluginId().remove("Protocol").lower();
		Kopete::PluginManager::self()->unloadPlugin(PROTO_NAME);
	}

	KWizard::reject();
}

#include "addaccountwizard.moc"

// vim: set noet ts=4 sts=4 sw=4:

