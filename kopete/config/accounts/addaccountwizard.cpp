/*
    addaccountwizard.cpp - Kopete Add Account Wizard

    Copyright (c) 2003-2004 by Olivier Goffart       <ogoffart@tiscalinet.be>
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

#include <kcolorbutton.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kplugininfo.h>

#include "addaccountwizardpage1.h"
#include "addaccountwizardpage2.h"
#include "addaccountwizardpage3.h"
#include "editaccountwidget.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopeteprotocol.h"
#include "kopetepluginmanager.h"

AddAccountWizard::AddAccountWizard( QWidget *parent, const char *name, bool modal )
: KWizard( parent, name, modal, WDestructiveClose )
{

	//kdDebug( 14100 ) << k_funcinfo << endl;
	m_accountPage = 0L;
	m_proto = 0L;

	m_intro =         new AddAccountWizardPage1( this );
	m_selectService = new AddAccountWizardPage2( this );
	m_finish =        new AddAccountWizardPage3( this );

	addPage( m_intro,         m_intro->caption() );
	addPage( m_selectService, m_selectService->caption() );
	addPage( m_finish,        m_finish->caption() );

	QListViewItem *pluginItem = 0L;

	QValueList<KPluginInfo *> protocols = Kopete::PluginManager::self()->availablePlugins( "Protocols" );

	for ( QValueList<KPluginInfo *>::Iterator it = protocols.begin(); it != protocols.end(); ++it )
	{
		pluginItem = new QListViewItem( m_selectService->protocolListView );
		pluginItem->setText( 0, ( *it )->name() );
		pluginItem->setText( 1, ( *it )->comment() );
		pluginItem->setPixmap( 0, SmallIcon( ( *it )->icon() ) );
		m_protocolItems.insert( pluginItem, ( *it ) );
	}

	if ( protocols.count() == 1 )
	{
		pluginItem->setSelected( true );
		// I think it is important to select one protocol to make sure.
		//setAppropriate( m_selectService, false );
	}

	setNextEnabled( m_selectService, ( protocols.count() == 1 ) );
	setFinishEnabled( m_finish, true );

	connect( m_selectService->protocolListView, SIGNAL( clicked( QListViewItem * ) ),
		this, SLOT( slotProtocolListClicked( QListViewItem * ) ) );
	connect( m_selectService->protocolListView, SIGNAL( doubleClicked( QListViewItem * ) ),
		this, SLOT( slotProtocolListDoubleClicked( QListViewItem * ) ) );
	connect( m_selectService->protocolListView, SIGNAL( selectionChanged( QListViewItem * ) ),
		this, SLOT( slotProtocolListClicked( QListViewItem * ) ) );
}

AddAccountWizard::~AddAccountWizard()
{
	//kdDebug( 14100 ) << k_funcinfo << endl;
}

void AddAccountWizard::slotProtocolListClicked( QListViewItem * )
{
	//kdDebug( 14100 ) << k_funcinfo << endl;

	// Make sure only one protocol is selected before allowing the user to continue
	setNextEnabled( m_selectService, ( m_selectService->protocolListView->selectedItem() != 0 ) );
}

void AddAccountWizard::slotProtocolListDoubleClicked( QListViewItem *lvi)
{
	// Make sure the user clicked on an item
	if ( lvi )
		next();
}

void AddAccountWizard::accept()
{
	//kdDebug( 14100 ) << k_funcinfo << endl;
	Kopete::Account *account = m_accountPage->apply();
	if ( account && m_finish->mUseColor->isChecked() )
		account->setColor( m_finish->mColorButton->color() );

	if(m_proto)
	{
		//Make sur the protocol is correctly enabled.  This is not realy needed, but still good
		QString protocol_name=m_proto->pluginId().remove( "Protocol" ).lower();
		Kopete::PluginManager::self()->setPluginEnabled( protocol_name , true );
	}
	
	KWizard::accept();

	//Bug 76583: If "Connect automatically at startup" box is checked, Kopete should connect to that account upon creation 
	if(account && account->autoLogin())
		account->connect();
}

void AddAccountWizard::reject()
{
	if(m_proto && Kopete::AccountManager::self()->accounts(m_proto).isEmpty())
	{
		//FIXME: we should use a decent way to do that
		QString protocol_name=m_proto->pluginId().remove( "Protocol" ).lower();

//		Kopete::PluginManager::self()->setPluginEnabled( protocol_name , false );
		Kopete::PluginManager::self()->unloadPlugin( protocol_name );

	}

	KWizard::reject();

}

void AddAccountWizard::back()
{
	if ( currentPage() == dynamic_cast<QWidget *>( m_accountPage ) )
	{
		kdDebug( 14100 ) << k_funcinfo << "Deleting m_accountPage" << endl;

		// Deletes the accountPage, KWizard does not like deleting pages
		// using different pointers, it only seems to watch its own pointer
		delete currentPage();
		//removePage( dynamic_cast<QWidget *>( m_accountPage ) );
		//delete m_accountPage;
		m_accountPage = 0L;
		m_proto = 0L;

		// removePage() already goes back to previous page, no back() needed
	}
	else
	{
		KWizard::back();
	}
}

void AddAccountWizard::next()
{
	if ( currentPage() == m_selectService ||
		( currentPage() == m_intro && !appropriate( m_selectService ) ) )
	{
		if ( m_accountPage )
		{
			kdDebug( 14100 ) << k_funcinfo << "AccountPage still valid, part1!" << endl;
/*
			// FIXME: Why is this commented out? Is this buggy or obsolete? - Martijn
			kdDebug( 14100 ) << k_funcinfo << "Deleting accountPage, first part" << endl;
			removePage( dynamic_cast<QWidget *>( m_accountPage ) );
			delete m_accountPage;
			m_accountPage = 0L;
*/
		}

		QListViewItem *lvi = m_selectService->protocolListView->selectedItem();
		if ( lvi )
		{
			m_proto = dynamic_cast<Kopete::Protocol *>( Kopete::PluginManager::self()->loadPlugin( m_protocolItems[ lvi ]->pluginName() ) );
			if ( m_proto )
			{
				if ( m_accountPage )
				{
					kdDebug( 14100 ) << k_funcinfo << "AccountPage still valid, part2!" << endl;
/*
					// FIXME: Why is this commented out? Is this buggy or obsolete? - Martijn
					kdDebug( 14100 ) << k_funcinfo << "Deleting accountPage after finding selected Protocol" << endl;
					removePage( dynamic_cast<QWidget *>( m_accountPage ) );
					delete m_accountPage;
					m_accountPage = 0L;
*/
				}

				m_accountPage = m_proto->createEditAccountWidget( 0L, this );
				if ( !m_accountPage )
				{
					KMessageBox::queuedMessageBox( this, KMessageBox::Error,
						i18n( "This protocol does not currently support adding accounts." ),
						i18n( "Error While Adding Account" ) );
				}
				else
				{
					kdDebug( 14100 ) << k_funcinfo << "Adding Step Two page and switching to that one" << endl;
					insertPage( dynamic_cast<QWidget *>( m_accountPage ),
						i18n( "Step Two: Account Information" ), indexOf( m_finish ) );
					KWizard::next();
				}
			}
			else
			{
				KMessageBox::queuedMessageBox( this, KMessageBox::Error,
					i18n( "Cannot load the %1 protocol plugin." ).arg( m_protocolItems[ lvi ]->name() ), i18n( "Error While Adding Account" ) );
			}
		}
		return;
	}
	else if ( indexOf( currentPage() ) == 2 )
	{
		if ( !m_accountPage->validateData() )
			return;

		QColor col = Kopete::AccountManager::self()->guessColor( m_proto );

		m_finish->mColorButton->setColor( col );
		m_finish->mUseColor->setChecked( col.isValid() );
		KWizard::next();
	}
	else
	{
		KWizard::next();
	}
}

#include "addaccountwizard.moc"

// vim: set noet ts=4 sts=4 sw=4:

