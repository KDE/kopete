/*
    addaccountwizard.cpp - Kopete Add Account Wizard

    Copyright (c) 2003 by Olivier Goffart <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qcheckbox.h>
#include <klistview.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kcolorbutton.h>

#include "addaccountwizard.h"
#include "kopeteprotocol.h"
#include "pluginloader.h"
#include "editaccountwidget.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"

AddAccountWizard::AddAccountWizard( QWidget *parent, const char *name, bool modal )
	: KWizard(parent, name, modal)
{
//	kdDebug(14100) << k_funcinfo << "Called." << endl;
	accountPage = 0L;

	intro = new AddAccountWizardPage1(this);
	selectService = new AddAccountWizardPage2(this);
	finish = new AddAccountWizardPage3(this);

	addPage( intro, intro->caption() );
	addPage( selectService, selectService->caption() );
	addPage( finish, finish->caption() );

	/*QPtrList<KopetePlugin> plugins = LibraryLoader::pluginLoader()->plugins();
	for(KopetePlugin *p = plugins.first(); p; p = plugins.next())
	{
		KopeteProtocol *proto = dynamic_cast<KopeteProtocol*>(p);
		if( proto )
		{
			pluginItem = new QListViewItem(selectService->protocolListView);
			pluginItem->setText(0, proto->displayName());
			pluginItem->setPixmap(0, SmallIcon(proto->pluginIcon()));
			pluginCount++;
			m_protocolItems.insert(pluginItem, proto);
		}
	}*/

	int pluginCount = 0;
	QListViewItem *pluginItem=0L;

	QValueList<KopeteLibraryInfo> available = LibraryLoader::pluginLoader()->available();

	for(QValueList<KopeteLibraryInfo>::Iterator i = available.begin(); i != available.end(); ++i)
	{
//		bool exclusive = false;
		if(( *i ).type == "Kopete/Protocol" )
		{
			pluginItem = new QListViewItem(selectService->protocolListView);
			pluginItem->setText(0, ( *i ).name );
			pluginItem->setText( 1, ( *i ).comment );
			pluginItem->setPixmap(0, SmallIcon( (*i).icon ) );
			pluginCount++;
			m_protocolItems.insert(pluginItem, (*i));
		}
	}

	if(pluginCount == 1)
	{
		pluginItem->setSelected( true );
		// I think it is important to select one protocol to make sure.
		//setAppropriate( selectService, false );
	}

	setNextEnabled(selectService, (pluginCount == 1));
	setFinishEnabled(finish, true);

	connect(selectService->protocolListView, SIGNAL(clicked(QListViewItem *)),
		this, SLOT(slotProtocolListClicked(QListViewItem *)));
	connect(selectService->protocolListView, SIGNAL(doubleClicked(QListViewItem *)),
		this, SLOT(slotProtocolListDoubleClicked(QListViewItem *)));
	connect(selectService->protocolListView, SIGNAL(selectionChanged(QListViewItem *)), 
		this, SLOT(slotProtocolListClicked(QListViewItem *)));
}

AddAccountWizard::~AddAccountWizard()
{
//	kdDebug(14100) << k_funcinfo << "Called." << endl;
}

void AddAccountWizard::slotProtocolListClicked( QListViewItem *)
{
//	kdDebug(14100) << k_funcinfo << "Called." << endl;
	// Just makes sure only one protocol is selected before allowing the user to continue
	setNextEnabled(
		selectService,
		(selectService->protocolListView->selectedItem()!=0)
		);
}

void AddAccountWizard::slotProtocolListDoubleClicked( QListViewItem *lvi)
{
	if(lvi) //make sure the user clicked on a item
	{
		next();
	}
}


void AddAccountWizard::accept()
{
//	kdDebug(14100) << k_funcinfo << "Called." << endl;
	KopeteAccount *a = accountPage->apply();
	if(a)
		a->setColor( finish->mUseColor->isChecked() ? finish->mColorButton->color() : QColor() );
	deleteLater();
}

void AddAccountWizard::back()
{
	if (currentPage() == dynamic_cast<QWidget*>(accountPage))
	{
		kdDebug(14100) << k_funcinfo << "deleting accountPage..." << endl;
		// deletes accountPage actually, KWizard does not like deleting pages
		// using different pointers, it only seems to watch its own pointer
		delete(currentPage());
//		removePage(dynamic_cast<QWidget*>(accountPage));
//		delete accountPage;
		accountPage = 0L;
		return; // removePage() already goes back to previous page, no back() needed
	}
	KWizard::back();
}


void AddAccountWizard::next()
{
	if (currentPage() == selectService ||
		(currentPage() == intro && !appropriate(selectService)))
	{
		if(accountPage)
		{
			kdDebug(14100) << k_funcinfo << "accountPage still valid, part1!" << endl;
/*			kdDebug(14100) << k_funcinfo << "deleting accountPage, first part" << endl;
			removePage(dynamic_cast<QWidget*>(accountPage));
			delete accountPage;
			accountPage = 0L;
			*/
		}

		QListViewItem *lvi = selectService->protocolListView->selectedItem();
		if(lvi)
		{
			kdDebug( 14100 ) << k_funcinfo << "Trying to load plugin " << m_protocolItems[ lvi ].name << " by name" << endl;
			KopetePlugin *pl = LibraryLoader::pluginLoader()->searchByName( m_protocolItems[ lvi ].name );
			if( !pl )
			{
				kdDebug( 14100 ) << k_funcinfo << "Unable to load by name. Trying to load plugin " <<
					m_protocolItems[ lvi ].specfile << "by specfile" << endl;
				pl = LibraryLoader::pluginLoader()->loadPlugin( m_protocolItems[ lvi ].specfile );
			}
			prot = dynamic_cast <KopeteProtocol*> (pl);
			if(prot)
			{
				if(accountPage)
				{
					kdDebug(14100) << k_funcinfo << "accountPage still valid, part2!" << endl;
	/*				kdDebug(14100) << k_funcinfo << "deleting accountPage after finding selected Protocol" << endl;
					removePage(dynamic_cast<QWidget*>(accountPage));
					delete accountPage;
					accountPage = 0L;*/
				}

				accountPage = prot->createEditAccountWidget(0L, this);

				if (!accountPage)
				{
					KMessageBox::error(this,
						i18n("The author of this protocol hasn't implemented adding of accounts."),
						i18n("Error While Adding Account") );
				}
				else
				{
					kdDebug(14100) << k_funcinfo << "Adding Step Two page and switching to that one" << endl;
					insertPage(
						dynamic_cast<QWidget*>(accountPage),
						i18n("Step Two: Account Information"), indexOf(finish)
					);
					KWizard::next();
				}
			}
			else
			{
				KMessageBox::error(this, i18n("Impossible to load the protocol `%1'.").arg(m_protocolItems[lvi].name) , i18n("Error While Adding Account") );
			}
		}
		return;
	}
	else if(indexOf(currentPage()) == 2)
	{
		if(!accountPage->validateData())
			return;

		QColor col = KopeteAccountManager::manager()->guessColor(prot);

		finish->mColorButton->setColor(col);
		finish->mUseColor->setChecked(col.isValid());
		KWizard::next();
	}
	else
	{
		KWizard::next();
	}
}
#include "addaccountwizard.moc"
// vim: set noet ts=4 sts=4 sw=4:
