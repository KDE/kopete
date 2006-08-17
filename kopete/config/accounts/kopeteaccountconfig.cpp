/*
    accountconfig.cpp  -  Kopete account config page

    Copyright (c) 2003-2004 by Olivier Goffart        <ogoffart @ kde.org>
    Copyright (c) 2003      by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>

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
#include <qguardedptr.h>

#include <kcolorbutton.h>
#include <kpushbutton.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "addaccountwizard.h"
#include "editaccountwidget.h"
#include "kopeteaccountconfigbase.h"
#include "kopeteaccountmanager.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"

class KopeteAccountLVI : public KListViewItem
{
	public:
		KopeteAccountLVI( Kopete::Account *a, KListView *p ) : KListViewItem( p ){  m_account = a; }
		Kopete::Account *account() { return m_account; }

	private:
		//need to be guarded because some accounts may be linked (that's the case of jabber transports)
		QGuardedPtr<Kopete::Account> m_account;
};

typedef KGenericFactory<KopeteAccountConfig, QWidget> KopeteAccountConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_accountconfig, KopeteAccountConfigFactory( "kcm_kopete_accountconfig" ) )

KopeteAccountConfig::KopeteAccountConfig( QWidget *parent, const char * /* name */, const QStringList &args )
: KCModule( KopeteAccountConfigFactory::instance(), parent, args )
{

	( new QVBoxLayout( this ) )->setAutoAdd( true );
	m_view = new KopeteAccountConfigBase( this, "KopeteAccountConfig::m_view" );

	m_view->mButtonUp->setIconSet( SmallIconSet( "up" ) );
	m_view->mButtonDown->setIconSet( SmallIconSet( "down" ) );

	connect( m_view->mButtonNew,    SIGNAL( clicked() ), this, SLOT( slotAddAccount() ) );
	connect( m_view->mButtonEdit,   SIGNAL( clicked() ), this, SLOT( slotEditAccount() ) );
	connect( m_view->mButtonRemove, SIGNAL( clicked() ), this, SLOT( slotRemoveAccount() ) );
	connect( m_view->mButtonUp,     SIGNAL( clicked() ), this, SLOT( slotAccountUp() ) );
	connect( m_view->mButtonDown,   SIGNAL( clicked() ), this, SLOT( slotAccountDown() ) );
	connect( m_view->mAccountList,  SIGNAL( selectionChanged() ), this, SLOT( slotItemSelected() ) );
	connect( m_view->mAccountList,  SIGNAL( doubleClicked( QListViewItem * ) ), this, SLOT( slotEditAccount() ) );
	connect( m_view->mUseColor,     SIGNAL( toggled( bool ) ), this, SLOT( slotColorChanged() ) );
	connect( m_view->mColorButton,  SIGNAL( changed( const QColor & ) ), this, SLOT( slotColorChanged() ) );

	m_view->mAccountList->setSorting(-1);

	setButtons( Help );
	load();
}

void KopeteAccountConfig::save()
{
	uint priority = m_view->mAccountList->childCount();

	KopeteAccountLVI *i = static_cast<KopeteAccountLVI*>( m_view->mAccountList->firstChild() );
	while( i )
	{
		if(!i->account())
			continue;
		i->account()->setPriority( priority-- );
		i = static_cast<KopeteAccountLVI*>( i->nextSibling() );
	}

	QMap<Kopete::Account *, QColor>::Iterator it;
	for(it=m_newColors.begin() ; it != m_newColors.end() ; ++it)
		it.key()->setColor(it.data());
	m_newColors.clear();

	Kopete::AccountManager::self()->save();

	load(); //refresh the colred accounts (in case of apply)
}

void KopeteAccountConfig::load()
{
	KopeteAccountLVI *lvi = 0L;

	m_view->mAccountList->clear();

	QPtrList<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts();
	for ( Kopete::Account *i = accounts.first() ; i; i = accounts.next() )
	{
		// Insert the item after the previous one
		lvi = new KopeteAccountLVI( i, m_view->mAccountList );
		lvi->setText( 0, i->protocol()->displayName() );
		lvi->setPixmap( 0, i->accountIcon() );
		lvi->setText( 1, i->accountLabel() );
	}

	m_newColors.clear();
	slotItemSelected();
}

void KopeteAccountConfig::slotItemSelected()
{
	m_protected=true;
	KopeteAccountLVI *itemSelected = static_cast<KopeteAccountLVI*>( m_view->mAccountList->selectedItem() );

	m_view->mButtonEdit->setEnabled( itemSelected );
	m_view->mButtonRemove->setEnabled( itemSelected );

	if ( itemSelected &&  itemSelected->account() )
	{
		m_view->mButtonUp->setEnabled( itemSelected->itemAbove() );
		m_view->mButtonDown->setEnabled( itemSelected->itemBelow() );

		Kopete::Account *account = itemSelected->account();
		QColor color= m_newColors.contains(account) ? m_newColors[account] :  account->color();
		m_view->mUseColor->setEnabled( true );
		m_view->mUseColor->setChecked( color.isValid() );
		m_view->mColorButton->setColor( color );
		m_view->mColorButton->setEnabled( m_view->mUseColor->isChecked() );

	}
	else
	{
		m_view->mButtonUp->setEnabled( false );
		m_view->mButtonDown->setEnabled( false);
		m_view->mUseColor->setEnabled( false );
		m_view->mColorButton->setEnabled( false );
	}
	m_protected=false;
}

void KopeteAccountConfig::slotAccountUp()
{
	KopeteAccountLVI *itemSelected = static_cast<KopeteAccountLVI*>( m_view->mAccountList->selectedItem() );
	if ( !itemSelected )
		return;

	if ( itemSelected->itemAbove() )
		itemSelected->itemAbove()->moveItem( itemSelected );

	slotItemSelected();
	emit changed( true );
}

void KopeteAccountConfig::slotAccountDown()
{
	KopeteAccountLVI *itemSelected = static_cast<KopeteAccountLVI*>( m_view->mAccountList->selectedItem() );
	if ( !itemSelected )
		return;

	itemSelected->moveItem( itemSelected->itemBelow() );

	slotItemSelected();
	emit changed( true );
}

void KopeteAccountConfig::slotAddAccount()
{
	AddAccountWizard *m_addwizard = new AddAccountWizard( this, "addAccountWizard", true );
	connect( m_addwizard, SIGNAL( destroyed( QObject * ) ), this, SLOT( slotAddWizardDone() ) );
	m_addwizard->show();
}

void KopeteAccountConfig::slotEditAccount()
{
	KopeteAccountLVI *lvi = static_cast<KopeteAccountLVI*>( m_view->mAccountList->selectedItem() );
	if ( !lvi || !lvi->account() )
		return;

	Kopete::Account *ident = lvi->account();
	Kopete::Protocol *proto = ident->protocol();

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
	Kopete::AccountManager::self()->save();
}

void KopeteAccountConfig::slotRemoveAccount()
{
	KopeteAccountLVI *lvi = static_cast<KopeteAccountLVI*>( m_view->mAccountList->selectedItem() );
	if ( !lvi || !lvi->account() )
		return;

	Kopete::Account *i = lvi->account();
	if ( KMessageBox::warningContinueCancel( this, i18n( "Are you sure you want to remove the account \"%1\"?" ).arg( i->accountLabel() ),
		i18n( "Remove Account" ), KGuiItem(i18n( "Remove Account" ), "editdelete"),
		 "askRemoveAccount", KMessageBox::Notify | KMessageBox::Dangerous ) == KMessageBox::Continue )
	{
		Kopete::AccountManager::self()->removeAccount( i );
		delete lvi;
	}
}

void KopeteAccountConfig::slotAddWizardDone()
{
	save();
	load();
}

void KopeteAccountConfig::slotColorChanged()
{
	if(m_protected)  //this slot is called because we changed the button
		return;      // color because another account has been selected

	KopeteAccountLVI *lvi = static_cast<KopeteAccountLVI*>( m_view->mAccountList->selectedItem() );
	if ( !lvi || !lvi->account() )
		return;
	Kopete::Account *account = lvi->account();

	if(!account->color().isValid() && !m_view->mUseColor->isChecked() )
	{  //we don't use color for that account and nothing changed.
		m_newColors.remove(account);
		return;
	}
	else if(!m_view->mUseColor->isChecked())
	{  //the user disabled account coloring, but it was activated before
		m_newColors[account]=QColor();
		emit changed(true);
		return;
	}
	else if(account->color() == m_view->mColorButton->color() )
	{   //The color has not changed.
		m_newColors.remove(account);
		return;
	}
	else
	{
		m_newColors[account]=m_view->mColorButton->color();
		emit changed(true);
	}
}

#include "kopeteaccountconfig.moc"

// vim: set noet ts=4 sts=4 sw=4:

