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

AddAccountWizard::AddAccountWizard( QWidget *parent, const char *name, bool modale )
: AddAccountWizard_Base( parent, name, modale )
{
	accountPage=0L;
	int pluginCount = 0;
	QListViewItem *pluginItem=0L;


	QPtrList<KopetePlugin> plugins = LibraryLoader::pluginLoader()->plugins();
	for( KopetePlugin *p = plugins.first() ; p ; p = plugins.next() )
	{
		KopeteProtocol *proto = dynamic_cast<KopeteProtocol*>( p );
		if( proto )
		{
			pluginItem = new QListViewItem( protocolListView );
			pluginItem->setText(0, proto->displayName());
			pluginItem->setPixmap( 0, SmallIcon( proto->pluginIcon() ) );
			pluginCount++;
			m_protocolItems.insert(pluginItem, proto);
		}
	}

	if ( pluginCount == 1 )
	{
		pluginItem->setSelected( true );
		// I think it is important to select one protocl to make sure.
		//setAppropriate( selectService, false );
	}

	setNextEnabled(selectService, (pluginCount == 1));
	setFinishEnabled(finis, true);

	connect( protocolListView, SIGNAL(clicked(QListViewItem *)), this, SLOT(slotProtocolListClicked(QListViewItem *)));
}

AddAccountWizard::~AddAccountWizard()
{
}

void AddAccountWizard::slotProtocolListClicked( QListViewItem *)
{
	// Just makes sure only one protocol is selected before allowing the user to continue
	setNextEnabled(selectService, (bool)(protocolListView->selectedItem()));
}

void AddAccountWizard::accept()
{
	KopeteAccount *a=accountPage->apply();
	if(a)
		a->setColor( mUseColor->isChecked() ? mColorButton->color() : QColor() );
	deleteLater();
}

void AddAccountWizard::next()
{
	if (currentPage() == selectService ||
		(currentPage() == intro && !appropriate( selectService )))
	{
		if(accountPage)
			delete accountPage;

		QListViewItem *lvi = protocolListView->selectedItem();
		if( lvi && m_protocolItems[lvi] )
		{
			if( accountPage )
				delete accountPage;

			accountPage = m_protocolItems[lvi]->createEditAccountWidget(0L,this);

			if (!accountPage) {
				KMessageBox::error( this, i18n( "The author of this protocol hasn't implemented Adding of Accounts" ),
														i18n( "Error while adding account" ) );
				return;
			}

			insertPage( dynamic_cast<QWidget*>(accountPage), i18n( "Step Two: Account Information" ), indexOf(finis) );
			QWizard::next();
		}
		return;
	}
	else if( indexOf( currentPage() ) == 2 )
	{
		if( !accountPage->validateData() )
			return;

		QColor col=KopeteAccountManager::manager()->guessColor( m_protocolItems[protocolListView->selectedItem()] );
		mColorButton->setColor(col );
		mUseColor->setChecked(col.isValid());
		QWizard::next();
	}
	else
		QWizard::next();
}


#include "addaccountwizard.moc"

// vim: set noet ts=4 sts=4 sw=4:

