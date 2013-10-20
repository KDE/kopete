/*
    accountconfig.cpp  -  Kopete account config page

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright (c) 2003-2007 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2003      by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2007      by Will Stephenson        <wstephenson@kde.org>

    Kopete    (c) 2003-2007 by the Kopete developers  <kopete-devel@kde.org>

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

#include <QtGui/QCheckBox>
#include <QtGui/QLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QBoxLayout>
#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtGui/QContextMenuEvent>

#include <kcolorbutton.h>
#include <kpushbutton.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kpluginfactory.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kicon.h>
#include <kaction.h>
#include <kmenu.h>
#include <kcolordialog.h>

#include "addaccountwizard.h"
#include "editaccountwidget.h"
#include "kopeteaccountmanager.h"
#include "kopeteidentitymanager.h"
#include "kopeteidentity.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "accountidentitydialog.h"
#include "identitydialog.h"


K_PLUGIN_FACTORY( KopeteAccountConfigFactory,
		registerPlugin<KopeteAccountConfig>(); )
K_EXPORT_PLUGIN( KopeteAccountConfigFactory("kcm_kopete_accountconfig") )

KopeteAccountConfig::KopeteAccountConfig( QWidget *parent, const QVariantList &args )
: KCModule( KopeteAccountConfigFactory::componentData(), parent, args )
{
	setupUi( this );

	QHeaderView *header = mAccountList->header();
	header->setResizeMode( 1, QHeaderView::ResizeToContents );
	header->setResizeMode( 0, QHeaderView::Stretch );
	header->setVisible(false);
	
	configureActions();
	configureMenus();

	connect( mAccountList,  SIGNAL(itemPositionChanged()), this, SLOT(changed()) );
	connect( mAccountList,  SIGNAL(itemSelectionChanged()), this, SLOT(slotItemSelected()) );
	connect( mAccountList,  SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(slotModify()) );
	connect( mAccountList,  SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(slotItemChanged(QTreeWidgetItem*)) );
	connect( mAccountList,  SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(slotItemClicked(QTreeWidgetItem*,int)) );

	// this ensures that newly created accounts are assigned to the selected identity
	connect( Kopete::AccountManager::self(), SIGNAL(accountRegistered(Kopete::Account*)), this, SLOT(slotAccountAdded(Kopete::Account*)) );
    connect( Kopete::AccountManager::self(), SIGNAL(accountUnregistered(const Kopete::Account*)), this, SLOT(slotAccountRemoved(const Kopete::Account*)) );

	mAccountList->installEventFilter( this );

	setButtons( Help );
	load();
}

KopeteAccountLVI* KopeteAccountConfig::selectedAccount()
{
	QList<QTreeWidgetItem*> selectedItems = mAccountList->selectedItems();
	if(!selectedItems.empty())
		return dynamic_cast<KopeteAccountLVI*>( selectedItems.first() );
	return 0;
}

KopeteIdentityLVI* KopeteAccountConfig::selectedIdentity()
{
	QList<QTreeWidgetItem*> selectedItems = mAccountList->selectedItems();
	if(!selectedItems.empty())
		return dynamic_cast<KopeteIdentityLVI*>( selectedItems.first() );
	return 0;
}

void KopeteAccountConfig::save()
{
	uint priority = 0;
	for ( int i = 0; i < mAccountList->topLevelItemCount(); i++ )
		priority += mAccountList->topLevelItem( i )->childCount();
			
	for ( int i = 0; i < mAccountList->topLevelItemCount(); i++ )
	{
		KopeteIdentityLVI* identity = dynamic_cast<KopeteIdentityLVI*>( mAccountList->topLevelItem( i ) );
		for ( int j = 0; j < identity->childCount(); j++ )
		{
			KopeteAccountLVI* account = dynamic_cast<KopeteAccountLVI*>( identity->child( j ) );
			account->account()->setIdentity( identity->identity() );
			account->account()->setPriority( priority-- );
		}
	}

	Kopete::AccountManager::self()->save();
	Kopete::IdentityManager::self()->save();


	//load(); //refresh the colred accounts (in case of apply)
}

bool accountPriorityLessThan(const Kopete::Account* a, const Kopete::Account* b)
{
	return (a->priority() > b->priority());
}

bool identityPriorityLessThan(const Kopete::Identity* a, const Kopete::Identity* b)
{
	if ( a->accounts().isEmpty() )
		return false;
	
	if ( b->accounts().isEmpty() && !a->accounts().isEmpty() )
		return true;
	
	return (a->accounts().first()->priority() > b->accounts().first()->priority());
}

void KopeteAccountConfig::load()
{
	mAccountList->clear();

	QHash<Kopete::Identity *,QTreeWidgetItem *> identityItemHash;
	Kopete::Identity *defaultIdentity = Kopete::IdentityManager::self()->defaultIdentity();

	// Sort by priority, the priority is take from identity accounts because identity doesn't have priority
	QList<Kopete::Identity*> identityList = Kopete::IdentityManager::self()->identities();
	qSort( identityList.begin(), identityList.end(), identityPriorityLessThan );

	foreach(Kopete::Identity *i, identityList)
	{
		//KopeteIdentityLVI *lvi = new KopeteIdentityLVI( i, mIdentityList );
		QTreeWidgetItem *identityItem = new KopeteIdentityLVI( i, mAccountList );
		// Insert the item after the previous one
		
		identityItem->setText( 0, i->label() );
		identityItem->setIcon( 0, KIcon( i->customIcon()) );
		
		identityItem->setExpanded( true );
		
		if (i == defaultIdentity)
		{
			QFont font = identityItem->font( 0 );
			font.setBold( true );
			identityItem->setFont( 0, font );
			identityItem->setSelected( true );
		}
		//identityItem->setSizeHint( 0, QSize(0, 42) );
		
		identityItemHash.insert(i,identityItem);
	}

	// Sort by priority
	QList<Kopete::Account*> accountList = Kopete::AccountManager::self()->accounts();
	qSort( accountList.begin(), accountList.end(), accountPriorityLessThan );

	foreach( Kopete::Account *account, accountList )
	{
		Kopete::Identity *idnt = account->identity();
		
		Q_ASSERT(identityItemHash.contains(idnt));
		KopeteAccountLVI *lvi = new KopeteAccountLVI( account, identityItemHash[idnt] );
		lvi->setText( 0, account->accountLabel() );
		lvi->setIcon( 0, account->myself()->onlineStatus().iconFor( account ) );
		QFont font = lvi->font( 0 );
		font.setBold( true );
		lvi->setFont( 0, font );

		lvi->setSizeHint( 0, QSize(0, 42) );
		
		lvi->setText( 1, account->myself()->onlineStatus().description() );
		lvi->setTextAlignment( 1, Qt::AlignRight | Qt::AlignVCenter );
		lvi->setFont( 1, font );
	
		lvi->setFlags( (lvi->flags() & ~Qt::ItemIsDropEnabled) | Qt::ItemIsUserCheckable );
		lvi->setCheckState ( 0, account->excludeConnect() ? Qt::Unchecked : Qt::Checked );

		connect( account->myself(), SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
				 this, SLOT(slotOnlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)));
	}

	changed( false );
	slotItemSelected();
}

void KopeteAccountConfig::slotItemSelected()
{
	bool accountSelected = selectedAccount();
	bool hasMultipleIdentities = ( Kopete::IdentityManager::self()->identities().size() > 1 );
	mButtonAccountModify->setEnabled( accountSelected );
	mButtonAccountRemove->setEnabled( accountSelected );
	m_actionAccountSwitchIdentity->setEnabled( accountSelected && hasMultipleIdentities );
	mButtonAccountSwitchIdentity->setEnabled( m_actionAccountSwitchIdentity->isEnabled() );
	mButtonAccountSetColor->setEnabled( accountSelected );

	bool identitySelected = selectedIdentity();
	bool isDefaultIdentity = (identitySelected && Kopete::IdentityManager::self()->defaultIdentity() == selectedIdentity()->identity());
	mButtonAccountAdd->setEnabled( identitySelected );
	mButtonIdentityCopy->setEnabled( identitySelected );
	mButtonIdentityModify->setEnabled( identitySelected );
	m_actionIdentityRemove->setEnabled( identitySelected && !isDefaultIdentity );
	mButtonIdentityRemove->setEnabled( m_actionIdentityRemove->isEnabled() );
	m_actionIdentitySetDefault->setEnabled( identitySelected && !isDefaultIdentity );
	mButtonIdentitySetDefault->setEnabled( m_actionIdentitySetDefault->isEnabled() );
}

void KopeteAccountConfig::slotAddAccount()
{
	AddAccountWizard *addwizard = new AddAccountWizard( this, true );
	KopeteIdentityLVI *ilvi = selectedIdentity();
	if (ilvi)
	{
		addwizard->setIdentity( ilvi->identity() );
	}
	addwizard->show();
}

void KopeteAccountConfig::slotModify()
{
	KopeteAccountLVI *alvi = selectedAccount();
	KopeteIdentityLVI *ilvi = selectedIdentity();
	
	if ( ilvi && ilvi->identity() )
		return modifyIdentity( ilvi->identity() );

	if ( alvi && alvi->account() )
		return modifyAccount( alvi->account() );
}
	
	
void KopeteAccountConfig::modifyAccount(Kopete::Account *account)
{
	Kopete::Protocol *proto = account->protocol();

	QPointer <KDialog> editDialog = new KDialog( this );
	editDialog->setCaption( i18n("Modify Account" ) );
	editDialog->setButtons( KDialog::Ok | KDialog::Cancel );
	editDialog->setDefaultButton(KDialog::Ok);
	editDialog->showButtonSeparator(true);

	KopeteEditAccountWidget *m_accountWidget = proto->createEditAccountWidget( account, editDialog );
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
	delete editDialog;

	load();
	Kopete::AccountManager::self()->save();
}

void KopeteAccountConfig::modifyIdentity(Kopete::Identity *)
{
	KopeteIdentityLVI *lvi = selectedIdentity();
	
	if ( !lvi || !lvi->identity() )
		return;

	Kopete::Identity *ident = lvi->identity();

	QPointer <IdentityDialog> dialog = new IdentityDialog(ident, this);
	dialog->exec();
	delete dialog;

	load();
	Kopete::IdentityManager::self()->save();
}

void KopeteAccountConfig::removeAccount()
{
	KopeteAccountLVI *lvi = selectedAccount();
	
	if ( lvi && lvi->account() ) {
		Kopete::Account *i = lvi->account();
		if ( KMessageBox::warningContinueCancel( this, i18n( "Are you sure you want to remove the account \"%1\"?", i->accountLabel() ),
					i18n( "Remove Account" ), KGuiItem(i18n( "Remove Account" ), "edit-delete"), KStandardGuiItem::cancel(),
					QString(), KMessageBox::Notify | KMessageBox::Dangerous ) == KMessageBox::Continue )
		{
			Kopete::AccountManager::self()->removeAccount( i );
		}
	}
}

void KopeteAccountConfig::removeIdentity()
{
	KopeteIdentityLVI *lvi = selectedIdentity();
	Kopete::Identity *i;
	
	if ( lvi && ( i = lvi->identity() ) ) {
		if (!i->accounts().count())
		{
			if ( KMessageBox::warningContinueCancel( this, i18n( "Are you sure you want to remove the identity \"%1\"?", i->label() ),
						i18n( "Remove Identity" ), KGuiItem(i18n( "Remove Identity" ), "edit-delete"), KStandardGuiItem::cancel(),
						"askRemoveIdentity", KMessageBox::Notify | KMessageBox::Dangerous ) == KMessageBox::Continue )
			{
				Kopete::IdentityManager::self()->removeIdentity( i );
				delete lvi;
			}
		}
		else
		{
			// if there are any accounts linked to this identity, need to change them before removing the identity
			if ( AccountIdentityDialog::changeAccountIdentity( this, i->accounts(), i, 
						i18n("Before removing the identity %1, the following accounts must be " 
							"assigned to another identity:", i->label())) )
			{
				Kopete::IdentityManager::self()->removeIdentity( i );
				delete lvi;
			}
		}
		// if we removed the default identity, this will trigger an update
		Kopete::IdentityManager::self()->defaultIdentity();
		save();
		// To be sure that accounts with relocated identities appear, reload
		load();
	}
}

void KopeteAccountConfig::slotAccountSwitchIdentity()
{
	KopeteAccountLVI *lvi = selectedAccount();

	if ( !lvi || !lvi->account() )
		return;

	// If there are only two identities, don't show the dialog,
	// just switch to the other identity
	Kopete::Account *a = lvi->account();
	if ( Kopete::IdentityManager::self()->identities().size() == 2 ) {
		foreach ( Kopete::Identity * id, Kopete::IdentityManager::self()->identities() ) {
			if ( id != a->identity() ) {
				a->setIdentity( id );
				break;
			}
		}
	} else {
		AccountIdentityDialog::changeAccountIdentity( this, a, 0, i18n("Select an identity for the account:"));
	}

	load();
}

void KopeteAccountConfig::slotAccountSetColor()
{
	KopeteAccountLVI *lvi = selectedAccount();

	if ( !lvi || !lvi->account() )
		return;

	Kopete::Account *a = lvi->account();

	QColor color = a->color();

	if ( KColorDialog::getColor(color, Qt::black, this) == KColorDialog::Accepted ) {
		a->setColor(color);
	}

	load();
}

void KopeteAccountConfig::slotSetDefaultIdentity()
{
	KopeteIdentityLVI *lvi = selectedIdentity();
	
	if ( !lvi || !lvi->identity() )
		return;

	Kopete::IdentityManager::self()->setDefaultIdentity( lvi->identity() );
	load();
}

void KopeteAccountConfig::slotAddIdentity()
{
	Kopete::Identity *ident = new Kopete::Identity(i18n("New Identity"));

	if (!ident)
		return;

	QPointer <IdentityDialog> dialog = new IdentityDialog(ident, this);
	if ( dialog->exec() == QDialog::Accepted ) {
		ident = Kopete::IdentityManager::self()->registerIdentity(ident);
		if (ident) {
			Kopete::IdentityManager::self()->save();
			load();
		}
	} else {
		delete ident;
	}
	delete dialog;
}

void KopeteAccountConfig::slotCopyIdentity()
{
	Kopete::Identity * existing = selectedIdentity()->identity();
	uint copyCount = 2;
	QString newLabel = i18nc( "String used for creating first copy of a named item",
			"Copy of %1", existing->label() );
	Kopete::Identity::List ids = Kopete::IdentityManager::self()->identities();
	QStringList idLabels;
	foreach ( Kopete::Identity *i, ids ) {
		idLabels.append( i->label() );
	}
	while ( idLabels.contains( newLabel ) && copyCount < 100 ) {
		newLabel = i18nc( "String used for creating second and subsequent copies of a named item",
			"Copy %1 of %2", copyCount++, existing->label() );
	}
	Kopete::Identity * ident = existing->clone();
	ident->setLabel( newLabel );

	QPointer <IdentityDialog> dialog = new IdentityDialog(ident, this);
	if ( dialog->exec() == QDialog::Accepted ) {
		if ( Kopete::IdentityManager::self()->registerIdentity(ident) ) {
			load();
		}
	} else {
		delete ident;
	}
	delete dialog;
}

void KopeteAccountConfig::slotOnlineStatusChanged( Kopete::Contact *contact,
												   const Kopete::OnlineStatus &newStatus, 
												   const Kopete::OnlineStatus &/*oldStatus*/)
{
	//get all items
	QList<QTreeWidgetItem*> items = mAccountList->findItems("", Qt::MatchContains | Qt::MatchRecursive);
	QList<QTreeWidgetItem*>::iterator it;
	for (it = items.begin(); it != items.end(); ++it)
	{
		KopeteAccountLVI *i = dynamic_cast<KopeteAccountLVI*>(*it);
		if (!i || !i->account())
			continue;

		if (i->account()->myself() == contact)
		{
			(*it)->setIcon( 0, newStatus.iconFor(i->account()) );
			(*it)->setText( 1, newStatus.description() );
			break;
		}
	}

}

void KopeteAccountConfig::slotAccountAdded( Kopete::Account * account )
{
	save();
	load();
	Q_UNUSED(account);
}

void KopeteAccountConfig::slotAccountRemoved( const Kopete::Account * account )
{
	QList<QTreeWidgetItem*> items = mAccountList->findItems("", Qt::MatchContains | Qt::MatchRecursive);
	QList<QTreeWidgetItem*>::iterator it;
	for (it = items.begin(); it != items.end(); ++it)
	{
		KopeteAccountLVI *lvi = dynamic_cast<KopeteAccountLVI*>(*it);
		if ( lvi && lvi->account() == account)
		{
			delete lvi;
			break;
		}
	}
}

void KopeteAccountConfig::slotItemChanged(QTreeWidgetItem* item)
{
	if(!item)
		return;
	KopeteAccountLVI *a = dynamic_cast<KopeteAccountLVI*>(item);
	KopeteIdentityLVI *i = dynamic_cast<KopeteIdentityLVI*>(item->parent());
	if(a && i)
	{
		if(a->account()->identity() != i->identity() )
		{
			a->account()->setIdentity( i->identity() );
			changed(true);
		}
	}
}

bool KopeteAccountConfig::eventFilter( QObject *obj, QEvent *event )
{
	if ( obj == mAccountList && event->type() == QEvent::ContextMenu )
	{
		QContextMenuEvent *cmEvent = static_cast<QContextMenuEvent *>(event);
		KopeteIdentityLVI *ilvi = selectedIdentity();
		if ( ilvi && ilvi->identity() )
		{
			m_identityContextMenu->popup(cmEvent->globalPos());
		}

		KopeteAccountLVI *alvi = selectedAccount();
		if ( alvi && alvi->account() )
		{
			m_accountContextMenu->popup(cmEvent->globalPos());
		}
		return true;
	}

	return QObject::eventFilter( obj, event );
}

void KopeteAccountConfig::configureActions()
{
	// Add account
	m_actionAccountAdd = new KAction( i18n( "&Add Account..." ), this );
	m_actionAccountAdd->setIcon( KIcon("list-add") );
	mButtonAccountAdd->setIcon( m_actionAccountAdd->icon() );
	mButtonAccountAdd->setText( m_actionAccountAdd->text() );
	connect( m_actionAccountAdd, SIGNAL(triggered(bool)), this, SLOT(slotAddAccount()) );
	connect( mButtonAccountAdd, SIGNAL(clicked()), m_actionAccountAdd, SLOT(trigger()) );

	// Modify account
	m_actionAccountModify = new KAction( i18n( "&Modify Account..." ), this );
	m_actionAccountModify->setIcon( KIcon("configure") );
	mButtonAccountModify->setIcon( m_actionAccountModify->icon() );
	mButtonAccountModify->setText( m_actionAccountModify->text() );
	connect( m_actionAccountModify, SIGNAL(triggered(bool)), this, SLOT(slotModify()));
	connect( mButtonAccountModify, SIGNAL(clicked()), m_actionAccountModify, SLOT(trigger()) );

	// Remove account
	m_actionAccountRemove = new KAction( i18n( "&Remove Account" ), this );
	m_actionAccountRemove->setIcon( KIcon("edit-delete") );
	m_actionAccountRemove->setShortcut(KShortcut(Qt::Key_Delete));
	mButtonAccountRemove->setIcon( m_actionAccountRemove->icon() );
	mButtonAccountRemove->setText( m_actionAccountRemove->text() );
	connect( m_actionAccountRemove, SIGNAL(triggered(bool)), this, SLOT(removeAccount()) );
	connect( mButtonAccountRemove, SIGNAL(clicked()), m_actionAccountRemove, SLOT(trigger()) );

	// Switch identity for an account
	m_actionAccountSwitchIdentity = new KAction( i18n( "&Switch Identity..." ), this );
	mButtonAccountSwitchIdentity->setText( m_actionAccountSwitchIdentity->text() );
	connect( m_actionAccountSwitchIdentity, SIGNAL(triggered(bool)), this, SLOT(slotAccountSwitchIdentity()) );
	connect( mButtonAccountSwitchIdentity, SIGNAL(clicked()), m_actionAccountSwitchIdentity, SLOT(trigger()) );

	// Set/clear custom color for account
	m_actionAccountSetColor = new KAction( i18n( "Set C&olor..." ), this );
	mButtonAccountSetColor->setText( m_actionAccountSetColor->text() );
	connect( m_actionAccountSetColor, SIGNAL(triggered(bool)), this, SLOT(slotAccountSetColor()) );
	connect( mButtonAccountSetColor, SIGNAL(clicked()), m_actionAccountSetColor, SLOT(trigger()) );

	// Add identity
	m_actionIdentityAdd = new KAction( i18n( "Add &Identity..." ), this );
	m_actionIdentityAdd->setIcon( KIcon("list-add") );
	mButtonIdentityAdd->setIcon( m_actionIdentityAdd->icon() );
	mButtonIdentityAdd->setText( m_actionIdentityAdd->text() );
	connect( m_actionIdentityAdd, SIGNAL(triggered(bool)), this, SLOT(slotAddIdentity()) );
	connect( mButtonIdentityAdd, SIGNAL(clicked()), m_actionIdentityAdd, SLOT(trigger()) );

	// Copy identity
	m_actionIdentityCopy = new KAction( i18n( "&Copy Identity..." ), this );
	m_actionIdentityCopy->setIcon( KIcon("edit-copy") );
	mButtonIdentityCopy->setIcon( m_actionIdentityCopy->icon() );
	mButtonIdentityCopy->setText( m_actionIdentityCopy->text() );
	connect( m_actionIdentityCopy, SIGNAL(triggered(bool)), this, SLOT(slotCopyIdentity()) );
	connect( mButtonIdentityCopy, SIGNAL(clicked()), m_actionIdentityCopy, SLOT(trigger()) );

	// Modify identity
	m_actionIdentityModify = new KAction( i18n( "M&odify Identity..." ), this );
	m_actionIdentityModify->setIcon( KIcon("configure") );
	mButtonIdentityModify->setIcon( m_actionIdentityModify->icon() );
	mButtonIdentityModify->setText( m_actionIdentityModify->text() );
	connect( m_actionIdentityModify, SIGNAL(triggered(bool)), this, SLOT(slotModify()) );
	connect( mButtonIdentityModify, SIGNAL(clicked()), m_actionIdentityModify, SLOT(trigger()) );

	// Remove identity
	m_actionIdentityRemove = new KAction( i18n( "R&emove Identity" ), this );
	m_actionIdentityRemove->setIcon( KIcon("edit-delete") );
	mButtonIdentityRemove->setIcon( m_actionIdentityRemove->icon() );
	mButtonIdentityRemove->setText( m_actionIdentityRemove->text() );
	connect( m_actionIdentityRemove, SIGNAL(triggered(bool)), this, SLOT(removeIdentity()) );
	connect( mButtonIdentityRemove, SIGNAL(clicked()), m_actionIdentityRemove, SLOT(trigger()) );

	// Switch identity for an identity
	m_actionIdentitySetDefault = new KAction( i18n( "Set &Default" ), this );
	mButtonIdentitySetDefault->setText( m_actionIdentitySetDefault->text() );
	connect( m_actionIdentitySetDefault, SIGNAL(triggered(bool)), this, SLOT(slotSetDefaultIdentity()) );
	connect( mButtonIdentitySetDefault, SIGNAL(clicked()), m_actionIdentitySetDefault, SLOT(trigger()) );
}

void KopeteAccountConfig::configureMenus()
{
	// Account management context menu
	m_accountContextMenu = new KMenu ( this );
	m_accountContextMenu->addAction( m_actionAccountModify );
	m_accountContextMenu->addAction( m_actionAccountRemove );
	m_accountContextMenu->addAction( m_actionAccountSetColor );

	// Identity management context menu
	m_identityContextMenu = new KMenu ( this );
	m_identityContextMenu->addAction( m_actionAccountAdd );
	m_identityContextMenu->addSeparator();
	m_identityContextMenu->addAction( m_actionIdentityModify );
	m_identityContextMenu->addAction( m_actionIdentityRemove );
	m_identityContextMenu->addAction( m_actionIdentitySetDefault );
}

void KopeteAccountConfig::slotItemClicked( QTreeWidgetItem * item, int /*column*/ )
{
	KopeteAccountLVI *account = static_cast<KopeteAccountLVI*>( item );
	if ( account &&  account->parent() )
		account->account()->setExcludeConnect ( account->checkState(0) == Qt::Unchecked ? true : false );
}


#include "kopeteaccountconfig.moc"

// vim: set noet ts=4 sts=4 sw=4:
