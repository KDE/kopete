/*
    addaccountwizard.cpp - Kopete Add Account Wizard

    Copyright (c) 2003-2006 by Olivier Goffart       <ogoffart@kde.org>
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
#include <klocale.h>
#include <kmessagebox.h>
#include <kplugininfo.h>
#include <kvbox.h>

#include "editaccountwidget.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopeteprotocol.h"
#include "kopetepluginmanager.h"
#include "kopeteidentity.h"
#include "kopeteidentitymanager.h"

class AddAccountWizard::Private
{
public:
	Private()
		: accountPage(0)
		, proto(0)
		, identity(0L)
		{
		}

	QTreeWidgetItem* selectedProtocol();

	QMap<QTreeWidgetItem *, KPluginInfo>  protocolItems;
	KopeteEditAccountWidget *accountPage;
	KVBox *accountPageWidget;
	QWidget *selectService;
	QWidget *finish;
	Ui::AddAccountWizardPage1 uiSelectService;
	Ui::AddAccountWizardPage2 uiFinish;
	Kopete::Protocol *proto;
	KPageWidgetItem *selectServiceItem;
	Kopete::Identity *identity;
};

AddAccountWizard::AddAccountWizard( QWidget *parent, bool firstRun )
	: KAssistantDialog(parent)
	, d(new Private)
{
	// setup the select service page
	d->selectService = new QWidget(this);
	d->uiSelectService.setupUi(d->selectService);
	d->uiSelectService.protocolListView->setColumnCount( 2 );
	QStringList header;
	header << i18n("Name") << i18n("Description");
	d->uiSelectService.protocolListView->setHeaderLabels( header );
	if ( firstRun )
		d->uiSelectService.m_header->setText( i18nc( "1st message shown to users on first run of Kopete. Please keep the formatting.", "<h2>Welcome to Kopete</h2><p>Which messaging service do you want to connect to?</p>") );
	
	d->selectServiceItem = addPage(d->selectService,d->selectService->windowTitle());
	setValid(d->selectServiceItem, false);
		
	d->accountPageWidget = new KVBox(this);
	addPage(d->accountPageWidget,i18n("Step Two: Account Information"));

	// setup the final page
	d->finish = new QWidget(this);
	d->uiFinish.setupUi(d->finish);
	if ( firstRun )
		d->uiFinish.m_header->setText( i18nc( "2nd message shown to users on first run of Kopete. Please keep the formatting.", "<h2>Congratulations</h2><p>You have finished configuring the account. You can add more accounts with <i>Settings->Configure</i>.  Please click the \"Finish\" button.</p>") );
	addPage(d->finish,d->finish->windowTitle());

	// add the available messenger services to the dialogs list
	QList<KPluginInfo> protocols = Kopete::PluginManager::self()->availablePlugins("Protocols");
	qSort(protocols);
	for (QList<KPluginInfo>::Iterator it = protocols.begin(); it != protocols.end(); ++it)
	{
		QTreeWidgetItem *pluginItem = new QTreeWidgetItem(d->uiSelectService.protocolListView);
		pluginItem->setIcon(0, QIcon(SmallIcon(it->icon())));
		pluginItem->setText(0, it->name());
		pluginItem->setText(1, it->comment());

		d->protocolItems.insert(pluginItem, *it);
	}

	// focus the ListView
	QTreeWidget *protocol_list = d->uiSelectService.protocolListView;
	protocol_list->setFocus();
	
	
 
	// hook up the user input
	connect(d->uiSelectService.protocolListView, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotProtocolListClicked()));
	connect(d->uiSelectService.protocolListView, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotProtocolListClicked()));
	connect(d->uiSelectService.protocolListView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotProtocolListDoubleClicked()));
    setHelp(QString(),"kopete");
}

QTreeWidgetItem* AddAccountWizard::Private::selectedProtocol()
{
	QList<QTreeWidgetItem*> selectedItems = uiSelectService.protocolListView->selectedItems();
	if(!selectedItems.empty())
 		return selectedItems.first();
	return 0;
}

void AddAccountWizard::slotProtocolListClicked()
{
	// Make sure a protocol is selected before allowing the user to continue
	setValid(d->selectServiceItem, d->selectedProtocol() != 0);
}

void AddAccountWizard::slotProtocolListDoubleClicked()
{
	// proceed to the next wizard page if we double click a protocol
	next();
}

void AddAccountWizard::back()
{
	if (currentPage()->widget() == d->accountPageWidget)
	{
		// Deletes the accountPage, K3Wizard does not like deleting pages
		// using different pointers, it only seems to watch its own pointer
		delete d->accountPage;
		d->accountPage = 0;
		d->proto       = 0;

		// removePage() already goes back to previous page, no back() needed
	}
	KAssistantDialog::back();

	adjustSize();
	setMinimumSize(sizeHint());
}

void AddAccountWizard::next()
{
	if (currentPage()->widget() == d->selectService)
	{
		QTreeWidgetItem *lvi = d->selectedProtocol();
		if(!d->protocolItems[lvi].isValid())
		{ //no item selected
			return;
		}
		d->proto = qobject_cast<Kopete::Protocol *>(Kopete::PluginManager::self()->loadPlugin(d->protocolItems[lvi].pluginName()));
		if (!d->proto)
		{
			KMessageBox::queuedMessageBox(this, KMessageBox::Error,
				i18n("Cannot load the %1 protocol plugin.", d->protocolItems[lvi].name()),
				i18n("Error While Adding Account"));
			return;
		}

		d->accountPage = d->proto->createEditAccountWidget(0, d->accountPageWidget);
		if (!d->accountPage)
		{
			KMessageBox::queuedMessageBox(this, KMessageBox::Error,
				i18n("This protocol does not currently support adding accounts."),
				i18n("Error While Adding Account"));
			return;
		}
	
		KAssistantDialog::next();
	}
	else if (currentPage()->widget() == d->accountPageWidget)
	{
		// check the data of the page is valid
		if (!d->accountPage->validateData())
		{
			return;
		}

		QColor col = Kopete::AccountManager::self()->guessColor(d->proto);

		d->uiFinish.mColorButton->setColor(col);
		d->uiFinish.mUseColor->setChecked(col.isValid());
		KAssistantDialog::next();
	}
	else 
	{
		kDebug(14100) << "Next pressed on misc page";
		KAssistantDialog::next();
	}

	adjustSize();
	setMinimumSize(sizeHint());
}

void AddAccountWizard::accept()
{
	// registeredAccount shouldn't probably be called here. Anyway, if the account is already registered, 
	// it won't be registered twice
	Kopete::AccountManager *manager = Kopete::AccountManager::self();
	Kopete::Account        *account = d->accountPage->apply();

	// if the account wasn't created correctly then leave
	if (!account)
	{
		reject();
		return;
	}

	// Set a valid identity before registering the account
	if (!d->identity)
	{
		account->setIdentity(Kopete::IdentityManager::self()->defaultIdentity());
	}
	else
	{
		account->setIdentity(d->identity);
	}

	account = manager->registerAccount(account);
	// if the account wasn't created correctly then leave
	if (!account)
	{
		reject();
		return;
	}

	// Make sure the protocol is correctly enabled.  This is not really needed, but still good
	const QString PROTO_NAME = d->proto->pluginId().remove("Protocol").toLower();
	Kopete::PluginManager::self()->setPluginEnabled(PROTO_NAME , true);

	// setup the custom colour
	if (d->uiFinish.mUseColor->isChecked())
	{
		account->setColor(d->uiFinish.mColorButton->color());
	}

	// connect if necessary
	if (d->uiFinish.mConnectNow->isChecked())
	{
		account->connect();
	}

	KAssistantDialog::accept();
}

void AddAccountWizard::reject()
{
    // if we have a protocol plugin loaded and it is not being used, unload it
	if (d->proto)
	{
		bool hasAccount=false;
		foreach( Kopete::Account *act, Kopete::AccountManager::self()->accounts() )
		{
			if( act->protocol() == d->proto )
			{
				hasAccount=true;
				break;
			}
		}
		if(hasAccount)
		{
			const QString PROTO_NAME = d->proto->pluginId().remove("Protocol").toLower();
			Kopete::PluginManager::self()->unloadPlugin(PROTO_NAME);
		}
	}

	KAssistantDialog::reject();
}

AddAccountWizard::~AddAccountWizard()
{
	delete d;
}

void AddAccountWizard::setIdentity( Kopete::Identity *identity )
{
	d->identity = identity;
}


#include "addaccountwizard.moc"

// vim: set noet ts=4 sts=4 sw=4:

