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
//Added by qt3to4:
#include <QBoxLayout>
#include <qpointer.h>

#include <kcolorbutton.h>
#include <kpushbutton.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <k3listview.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "addaccountwizard.h"
#include "editaccountwidget.h"
#include "kopeteaccountmanager.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"

class KopeteAccountLVI : public K3ListViewItem
{
	public:
		KopeteAccountLVI( Kopete::Account *a, K3ListView *p ) : K3ListViewItem( p ){  m_account = a; }
		Kopete::Account *account() { return m_account; }

	private:
		//need to be guarded because some accounts may be linked (that's the case of jabber transports)
		QPointer<Kopete::Account> m_account;
};

typedef KGenericFactory<KopeteAccountConfig, QWidget> KopeteAccountConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_accountconfig, KopeteAccountConfigFactory( "kcm_kopete_accountconfig" ) )

KopeteAccountConfig::KopeteAccountConfig( QWidget *parent, const char * /* name */, const QStringList &args )
: KCModule( KopeteAccountConfigFactory::instance(), parent, args )
{

	( new QVBoxLayout( this ) )->setAutoAdd( true );
	m_view = new QWidget( this );
	setObjectName( "KopeteAccountConfig::m_view" );
	setupUi( m_view );

	mButtonUp->setIconSet( SmallIconSet( "up" ) );
	mButtonDown->setIconSet( SmallIconSet( "down" ) );

	connect( mButtonNew,    SIGNAL( clicked() ), this, SLOT( slotAddAccount() ) );
	connect( mButtonEdit,   SIGNAL( clicked() ), this, SLOT( slotEditAccount() ) );
	connect( mButtonRemove, SIGNAL( clicked() ), this, SLOT( slotRemoveAccount() ) );
	connect( mButtonUp,     SIGNAL( clicked() ), this, SLOT( slotAccountUp() ) );
	connect( mButtonDown,   SIGNAL( clicked() ), this, SLOT( slotAccountDown() ) );
	connect( mAccountList,  SIGNAL( selectionChanged() ), this, SLOT( slotItemSelected() ) );
	connect( mAccountList,  SIGNAL( doubleClicked( Q3ListViewItem * ) ), this, SLOT( slotEditAccount() ) );
	connect( mUseColor,     SIGNAL( toggled( bool ) ), this, SLOT( slotColorChanged() ) );
	connect( mColorButton,  SIGNAL( changed( const QColor & ) ), this, SLOT( slotColorChanged() ) );

	mAccountList->setSorting(-1);

	setButtons( Help );
	load();
}

void KopeteAccountConfig::save()
{
	uint priority = mAccountList->childCount();

	KopeteAccountLVI *i = static_cast<KopeteAccountLVI*>( mAccountList->firstChild() );
	while( i )
	{
		if(!i->account())
			continue;
		i->account()->setPriority( priority-- );
		i = static_cast<KopeteAccountLVI*>( i->nextSibling() );
	}

	QMap<Kopete::Account *, QColor>::Iterator it;
	for(it=m_newColors.begin() ; it != m_newColors.end() ; ++it)
		it.key()->setColor(it.value());
	m_newColors.clear();

	Kopete::AccountManager::self()->save();

	load(); //refresh the colred accounts (in case of apply)
}

void KopeteAccountConfig::load()
{
	KopeteAccountLVI *lvi = 0L;

	mAccountList->clear();

	QListIterator<Kopete::Account *> it( Kopete::AccountManager::self()->accounts() );
	Kopete::Account *i;
	while ( it.hasNext() )
	{
		i = it.next();
		// Insert the item after the previous one
		lvi = new KopeteAccountLVI( i, mAccountList );
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
	KopeteAccountLVI *itemSelected = static_cast<KopeteAccountLVI*>( mAccountList->selectedItem() );

	mButtonEdit->setEnabled( itemSelected );
	mButtonRemove->setEnabled( itemSelected );

	if ( itemSelected &&  itemSelected->account() )
	{
		mButtonUp->setEnabled( itemSelected->itemAbove() );
		mButtonDown->setEnabled( itemSelected->itemBelow() );

		Kopete::Account *account = itemSelected->account();
		QColor color= m_newColors.contains(account) ? m_newColors[account] :  account->color();
		mUseColor->setEnabled( true );
		mUseColor->setChecked( color.isValid() );
		mColorButton->setColor( color );
		mColorButton->setEnabled( mUseColor->isChecked() );

	}
	else
	{
		mButtonUp->setEnabled( false );
		mButtonDown->setEnabled( false);
		mUseColor->setEnabled( false );
		mColorButton->setEnabled( false );
	}
	m_protected=false;
}

void KopeteAccountConfig::slotAccountUp()
{
	KopeteAccountLVI *itemSelected = static_cast<KopeteAccountLVI*>( mAccountList->selectedItem() );
	if ( !itemSelected )
		return;

	if ( itemSelected->itemAbove() )
		itemSelected->itemAbove()->moveItem( itemSelected );

	slotItemSelected();
	emit changed( true );
}

void KopeteAccountConfig::slotAccountDown()
{
	KopeteAccountLVI *itemSelected = static_cast<KopeteAccountLVI*>( mAccountList->selectedItem() );
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
	KopeteAccountLVI *lvi = static_cast<KopeteAccountLVI*>( mAccountList->selectedItem() );
	if ( !lvi || !lvi->account() )
		return;

	Kopete::Account *ident = lvi->account();
	Kopete::Protocol *proto = ident->protocol();

	KDialog *editDialog = new KDialog( this, i18n( "Edit Account" ), KDialog::Ok | KDialog::Cancel );
	editDialog->setDefaultButton(KDialog::Ok);
	editDialog->enableButtonSeparator(true);

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
	KopeteAccountLVI *lvi = static_cast<KopeteAccountLVI*>( mAccountList->selectedItem() );
	if ( !lvi || !lvi->account() )
		return;

	Kopete::Account *i = lvi->account();
	if ( KMessageBox::warningContinueCancel( this, i18n( "Are you sure you want to remove the account \"%1\"?", i->accountLabel() ),
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

	KopeteAccountLVI *lvi = static_cast<KopeteAccountLVI*>( mAccountList->selectedItem() );
	if ( !lvi || !lvi->account() )
		return;
	Kopete::Account *account = lvi->account();

	if(!account->color().isValid() && !mUseColor->isChecked() )
	{  //we don't use color for that account and nothing changed.
		m_newColors.remove(account);
		return;
	}
	else if(!mUseColor->isChecked())
	{  //the user disabled account coloring, but it was activated before
		m_newColors[account]=QColor();
		emit changed(true);
		return;
	}
	else if(account->color() == mColorButton->color() )
	{   //The color has not changed.
		m_newColors.remove(account);
		return;
	}
	else
	{
		m_newColors[account]=mColorButton->color();
		emit changed(true);
	}
}

#include "kopeteaccountconfig.moc"

// vim: set noet ts=4 sts=4 sw=4:

