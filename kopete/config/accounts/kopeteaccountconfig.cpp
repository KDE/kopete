/*
    accountconfig.cpp  -  Kopete account config page

    Copyright (c) 2003      by Olivier Goffart        <ogoffart@tiscalinet.be>
    Copyright (c) 2003      by Martijn Klingens       <klingens@kde.org>

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

#include "kopeteaccountconfig.h"

#include <qcheckbox.h>
#include <qlayout.h>

#include <kcolorbutton.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "addaccountwizard.h"
#include "editaccountwidget.h"
#include "kopeteaccount.h"
#include "kopeteaccountconfigbase.h"
#include "kopeteaccountmanager.h"
#include "kopeteprotocol.h"

typedef KGenericFactory<KopeteAccountConfig, QWidget> KopeteAccountConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_accountconfig, KopeteAccountConfigFactory( "kcm_kopete_accountconfig" ) )

KopeteAccountConfig::KopeteAccountConfig( QWidget *parent, const char * /* name */, const QStringList &args )
: KCModule( KopeteAccountConfigFactory::instance(), parent, args )
{
	previousAccount = 0L;

	( new QVBoxLayout( this ) )->setAutoAdd( true );
	m_view = new KopeteAccountConfigBase( this, "KopeteAccountConfig::m_view" );

	//m_view->mButtonUp->setPixmap( SmallIcon( "up" ) );
	//m_view->mButtonDown->setPixmap( SmallIcon( "down" ) );
	m_view->mAccountList->setSorting( -1 );

	connect( m_view->mButtonNew,    SIGNAL( clicked() ), this, SLOT( slotAddAccount() ) );
	connect( m_view->mButtonEdit,   SIGNAL( clicked() ), this, SLOT( slotEditAccount() ) );
	connect( m_view->mButtonRemove, SIGNAL( clicked() ), this, SLOT( slotRemoveAccount() ) );
	//connect( m_view->mButtonUp,     SIGNAL( clicked() ), this, SLOT( slotAccountUp() ) );
	//connect( m_view->mButtonDown,   SIGNAL( clicked() ), this, SLOT( slotAccountDown() ) );
	connect( m_view->mAccountList,  SIGNAL( selectionChanged() ), this, SLOT( slotItemSelected() ) );
	connect( m_view->mAccountList,  SIGNAL( doubleClicked( QListViewItem * ) ), this, SLOT( slotEditAccount() ) );

	setButtons( Help );
	load();
	
}

void KopeteAccountConfig::save()
{
	if ( previousAccount )
		previousAccount->setColor( m_view->mUseColor->isChecked() ? m_view->mColorButton->color() : QColor() );

	KopeteAccountManager::manager()->save();
}

void KopeteAccountConfig::load()
{
	QListViewItem *lvi = 0L;

	m_view->mAccountList->clear();
	m_accountItems.clear();

	QPtrList<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts();
	for ( KopeteAccount *i = accounts.first() ; i; i = accounts.next() )
	{
		// Insert the item after the previous one
		lvi = new QListViewItem( m_view->mAccountList, lvi );
		lvi->setText( 0, i->protocol()->displayName() );
		lvi->setPixmap( 0, SmallIcon( i->protocol()->pluginIcon() ) );
		lvi->setText( 1, i->accountId() );
		m_accountItems.insert( lvi, i );
	}

	slotItemSelected();
}

void KopeteAccountConfig::slotItemSelected()
{
	QListViewItem *itemSelected = m_view->mAccountList->selectedItem();

	m_view->mButtonEdit->setEnabled( itemSelected );
	m_view->mButtonRemove->setEnabled( itemSelected );

        /* FIXME uncomment when implementing all acc order func        
        if ( itemSelected )
	{
		m_view->mButtonUp->setEnabled( itemSelected->itemAbove() );
		m_view->mButtonDown->setEnabled( itemSelected->itemBelow() );
	}
	else
	{
		m_view->mButtonUp->setEnabled( itemSelected );
		m_view->mButtonDown->setEnabled( itemSelected );
	}
        */

	// We shouldn't really save data before apply :-s
	if ( previousAccount )
		previousAccount->setColor( m_view->mUseColor->isChecked() ? m_view->mColorButton->color() : QColor() );

	KopeteAccount *a = m_accountItems[ itemSelected ];
	previousAccount = a;
	if ( a )
	{
		m_view->mUseColor->setEnabled( true );
		m_view->mUseColor->setChecked( a->color().isValid() );
		m_view->mColorButton->setColor( a->color() );
		m_view->mColorButton->setEnabled( m_view->mUseColor->isChecked() );
	}
	else
	{
		m_view->mUseColor->setEnabled( false );
		m_view->mColorButton->setEnabled( false );
	}
}

void KopeteAccountConfig::slotAccountUp()
{
  /*
	QListViewItem *itemSelected = m_view->mAccountList->selectedItem();
	if ( !itemSelected )
		return;

	if ( itemSelected->itemAbove() )
		itemSelected->itemAbove()->moveItem( itemSelected );

	KopeteAccountManager::manager()->moveAccount( m_accountItems[ itemSelected ] , KopeteAccountManager::Up );

	slotItemSelected();
	setChanged(true);
  */
}        

void KopeteAccountConfig::slotAccountDown()
{
  /*
	QListViewItem *itemSelected = m_view->mAccountList->selectedItem();
	if ( !itemSelected )
		return;

	itemSelected->moveItem( itemSelected->itemBelow() );

	KopeteAccountManager::manager()->moveAccount( m_accountItems[ itemSelected ] , KopeteAccountManager::Down );

	slotItemSelected();
	setChanged(true);
  */
}

void KopeteAccountConfig::slotAddAccount()
{
	AddAccountWizard *m_addwizard = new AddAccountWizard( this, "addAccountWizard", true );
	connect( m_addwizard, SIGNAL( destroyed( QObject * ) ), this, SLOT( slotAddWizardDone() ) );
	m_addwizard->show();
}

void KopeteAccountConfig::slotEditAccount()
{
	QListViewItem *lvi = m_view->mAccountList->selectedItem();
	if ( !lvi )
		return;

	KopeteAccount *ident = m_accountItems[ lvi ];
	KopeteProtocol *proto = ident->protocol();

	KDialogBase *editDialog = new KDialogBase( this, "KopeteAccountConfig::editDialog", true,
		i18n( "Edit Account" ), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true );

	KopeteEditAccountWidget *m_accountWidget = proto->createEditAccountWidget( ident, editDialog );
	if ( !m_accountWidget )
		return;

	// FIXME: Why the #### is EditAccountWidget not a QWidget?!? This sideways casting
	//        is braindead and error-prone. Looking at MSN the only reason I can see is
	//        because it allows direct subclassing of designer widgets. But what is
	//        wrong with embedding the designer widget in an empty QWidget instead?
	//        Also, if this REALLY has to be a pure class and not a widget, then the
	//        class should at least be renamed to EditAccountIface instead - Martijn
	QWidget *w = dynamic_cast<QWidget *>( m_accountWidget );
	if ( !w )
		return;

	editDialog->setMainWidget( w );
	if ( editDialog->exec() == QDialog::Accepted )
	{
		if( m_accountWidget->validateData() )
			m_accountWidget->apply();
	}

	// FIXME: Why deleteLater? It shouldn't be in use anymore at this point - Martijn
	editDialog->deleteLater();
	load();
	KopeteAccountManager::manager()->save();
}

void KopeteAccountConfig::slotRemoveAccount()
{
	QListViewItem *lvi = m_view->mAccountList->selectedItem();
	if ( !lvi )
		return;

	KopeteAccount *i = m_accountItems[ lvi ];
	if ( KMessageBox::warningContinueCancel( this, i18n( "Are you sure you want to remove the account \"%1\"?" ).arg( i->accountId() ),
		i18n( "Remove Account" ), i18n( "Remove Account" ) ) == KMessageBox::Continue )
	{
		previousAccount = 0L;
		m_accountItems.remove( lvi );
		KopeteAccountManager::manager()->removeAccount( i );
		delete lvi;
	}
}

void KopeteAccountConfig::slotAddWizardDone()
{
	load();
	KopeteAccountManager::manager()->save();
}

#include "kopeteaccountconfig.moc"

// vim: set noet ts=4 sts=4 sw=4:

