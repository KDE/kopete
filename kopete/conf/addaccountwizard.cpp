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

#include <qlayout.h>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kdebug.h>

#include "addaccountwizard.h"
#include "kopeteprotocol.h"
#include "pluginloader.h"
#include "kopetemetacontact.h"
#include "editaccountwidget.h"

AddIdentityWizard::AddIdentityWizard( QWidget *parent, const char *name, bool modale )
: AddIdentityWizard_Base( parent, name, modale )
{
	identityPage=0L;
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

AddIdentityWizard::~AddIdentityWizard()
{
}

void AddIdentityWizard::slotProtocolListClicked( QListViewItem *)
{
	// Just makes sure only one protocol is selected before allowing the user to continue
	setNextEnabled(selectService, (bool)(protocolListView->selectedItem()));
}

void AddIdentityWizard::accept()
{
	identityPage->apply();
	deleteLater();
}

void AddIdentityWizard::next()
{
	if (currentPage() == selectService ||
		(currentPage() == intro && !appropriate( selectService )))
	{
		if(identityPage)
			delete identityPage;
			
		QListViewItem *lvi=protocolListView->selectedItem();
		if(KopeteProtocol *p=m_protocolItems[lvi] )
		{
			identityPage = p->createEditIdentityWidget(0L,this);

			//that will not be possible, but if a protocol doesn't have yet this page, we should show a inform text or textbox
			if (!identityPage)
				return;

			QWidget *qWidget = dynamic_cast<QWidget*>( identityPage );
			insertPage( qWidget, i18n( "Step Two: Account Information" ) , indexOf( finis) );
			QWizard::next();
		}
		return;
	}
	if (currentPage() != intro &&
		currentPage() != selectService &&
		currentPage() != finis)
	{
		if (identityPage && !identityPage->validateData())
			return;
	}
	QWizard::next();
}


#include "addaccountwizard.moc"

// vim: set noet ts=4 sts=4 sw=4:

