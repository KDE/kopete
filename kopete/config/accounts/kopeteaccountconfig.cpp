/*
    accountconfig.cpp  -  Kopete account config page

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright (c) 2003-2007 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2003      by Martijn Klingens       <klingens@kde.org>

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

#include <kcolorbutton.h>
#include <kpushbutton.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kicon.h>

#include "addaccountwizard.h"
#include "editaccountwidget.h"
#include "kopeteaccountmanager.h"
#include "kopeteidentitymanager.h"
#include "kopeteidentity.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "accountidentitydialog.h"


class KopeteAccountLVI : public QTreeWidgetItem
{
	public:
		KopeteAccountLVI( Kopete::Account *a, QTreeWidget* parent) : QTreeWidgetItem(parent) , m_account(a) { }
		KopeteAccountLVI( Kopete::Account *a, QTreeWidgetItem* parent) : QTreeWidgetItem(parent) , m_account(a) { }
		Kopete::Account *account() { return m_account; }

	private:
		//need to be guarded because some accounts may be linked (that's the case of jabber transports)
		QPointer<Kopete::Account> m_account;
};

typedef KGenericFactory<KopeteAccountConfig, QWidget> KopeteAccountConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_accountconfig, KopeteAccountConfigFactory( "kcm_kopete_accountconfig" ) )

KopeteAccountConfig::KopeteAccountConfig( QWidget *parent, const QStringList &args )
: KCModule( KopeteAccountConfigFactory::componentData(), parent, args )
{
	setupUi( this );

	QHeaderView *header = mAccountList->header();
	header->setResizeMode( 1, QHeaderView::ResizeToContents );
	header->setResizeMode( 0, QHeaderView::Stretch );
	header->setVisible(false);

	mButtonNew->setIcon( KIcon("edit-add") );
	mButtonRemove->setIcon( KIcon("edit-delete") );

	connect( mButtonNew,    SIGNAL( clicked() ), this, SLOT( slotAddAccount() ) );
	connect( mButtonEdit,   SIGNAL( clicked() ), this, SLOT( slotEditAccount() ) );
	connect( mButtonIdentity, SIGNAL( clicked() ), this, SLOT( slotSelectIdentity() ) );
	connect( mButtonRemove, SIGNAL( clicked() ), this, SLOT( slotRemoveAccount() ) );
	connect( mAccountList,  SIGNAL( itemSelectionChanged() ), this, SLOT( slotItemSelected() ) );
	connect( mAccountList,  SIGNAL( itemDoubleClicked(QTreeWidgetItem*, int) ), this, SLOT( slotEditAccount() ) );
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

void KopeteAccountConfig::save()
{
	// The priority is disabled because of identities and because there is no way to edit it anymore
	/*uint priority = mAccountList->topLevelItemCount();
	
	KopeteAccountLVI *i;
	for( int j=0; j<mAccountList->topLevelItemCount(); ++j )
	{
		i = static_cast<KopeteAccountLVI*>( mAccountList->topLevelItem( j ) );
		if(!i->account())
			continue;
		i->account()->setPriority( priority-- );
	}*/

	Kopete::AccountManager::self()->save();

	//load(); //refresh the colred accounts (in case of apply)
}

void KopeteAccountConfig::load()
{
	mAccountList->clear();
		
	QHash<Kopete::Identity *,QTreeWidgetItem *> identityItemHash;
//	Kopete::Identity *defaultIdentity = Kopete::IdentityManager::self()->defaultIdentity();
	foreach(Kopete::Identity *i, Kopete::IdentityManager::self()->identities())
	{
		//KopeteIdentityLVI *lvi = new KopeteIdentityLVI( i, mIdentityList );
		QTreeWidgetItem *identityItem = new QTreeWidgetItem( mAccountList );
		// Insert the item after the previous one
		
		identityItem->setText( 0, i->identityId() );
		identityItem->setIcon( 0, KIcon( i->customIcon()) );
		
		identityItem->setExpanded( true );
		
		/*if (i == defaultIdentity)
		{
			QFont font = lvi->font( 0 );
			font.setBold( true );
			lvi->setFont( 0, font );
		}*/
		//lvi->setSizeHint( 0, QSize(0, 42) );
		
		identityItemHash.insert(i,identityItem);
	}

	foreach(Kopete::Account *i , Kopete::AccountManager::self()->accounts() )
	{
		Kopete::Identity *idnt = i->identity();
		
		KopeteAccountLVI *lvi;
		if(identityItemHash.contains(idnt))
			lvi = new KopeteAccountLVI( i, identityItemHash[idnt] );
		else
			lvi = new KopeteAccountLVI( i, mAccountList );
		lvi->setText( 0, i->accountLabel() );
		lvi->setIcon( 0, QIcon(i->myself()->onlineStatus().iconFor( i, 32)) );
		QFont font = lvi->font( 0 );
		font.setBold( true );
		lvi->setFont( 0, font );

		lvi->setSizeHint( 0, QSize(0, 42) );
		
		lvi->setText( 1, i->myself()->onlineStatus().statusTypeToString(i->myself()->onlineStatus().status()) );
		lvi->setTextAlignment( 1, Qt::AlignRight | Qt::AlignVCenter );
		lvi->setFont( 1, font );

		connect( i->myself(), SIGNAL(onlineStatusChanged(Kopete::Contact*, const Kopete::OnlineStatus&, const Kopete::OnlineStatus&)),
				 this, SLOT(slotOnlineStatusChanged(Kopete::Contact*, const Kopete::OnlineStatus&, const Kopete::OnlineStatus&)));
	}

	slotItemSelected();
}

void KopeteAccountConfig::slotItemSelected()
{
	m_protected=true;

	KopeteAccountLVI *itemSelected = selectedAccount();

	mButtonEdit->setEnabled( itemSelected );
	mButtonRemove->setEnabled( itemSelected );
	mButtonIdentity->setEnabled( itemSelected );

	m_protected=false;
}

void KopeteAccountConfig::slotAddAccount()
{
	AddAccountWizard *m_addwizard = new AddAccountWizard( this, true );
	connect( m_addwizard, SIGNAL(finished()), this, SLOT(slotAddWizardDone()) );
	m_addwizard->show();
}

void KopeteAccountConfig::slotEditAccount()
{
	KopeteAccountLVI *lvi = selectedAccount();
	
	if ( !lvi || !lvi->account() )
		return;

	Kopete::Account *ident = lvi->account();
	Kopete::Protocol *proto = ident->protocol();

	KDialog *editDialog = new KDialog( this );
	editDialog->setCaption( i18n("Edit Account" ) );
	editDialog->setButtons( KDialog::Ok | KDialog::Cancel );
	editDialog->setDefaultButton(KDialog::Ok);
	editDialog->showButtonSeparator(true);

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
	KopeteAccountLVI *lvi = selectedAccount();
	
	if ( !lvi || !lvi->account() )
		return;

	Kopete::Account *i = lvi->account();
	if ( KMessageBox::warningContinueCancel( this, i18n( "Are you sure you want to remove the account \"%1\"?", i->accountLabel() ),
		i18n( "Remove Account" ), KGuiItem(i18n( "Remove Account" ), "edit-delete"), KStandardGuiItem::cancel(),
		 "askRemoveAccount", KMessageBox::Notify | KMessageBox::Dangerous ) == KMessageBox::Continue )
	{
		Kopete::AccountManager::self()->removeAccount( i );
		delete lvi;
	}
}

void KopeteAccountConfig::slotSelectIdentity()
{
	KopeteAccountLVI *lvi = selectedAccount();
	
	if ( !lvi || !lvi->account() )
		return;

	Kopete::Account *a = lvi->account();

	AccountIdentityDialog::changeAccountIdentity( this, a, 0, i18n("Select an identity for the account:"));
	
	load();
}

void KopeteAccountConfig::slotOnlineStatusChanged( Kopete::Contact *contact,
												   const Kopete::OnlineStatus &status, 
												   const Kopete::OnlineStatus &oldStatus )
{
	//get all items
	QList<QTreeWidgetItem*> items = mAccountList->findItems("", Qt::MatchContains);
	QList<QTreeWidgetItem*>::iterator it;
	for (it = items.begin(); it != items.end(); ++it)
	{
		KopeteAccountLVI *i = dynamic_cast<KopeteAccountLVI*>(*it);
		if (!i)
			continue;

		if (i->account()->myself() == contact)
		{
			(*it)->setIcon( 0, QIcon(status.iconFor(i->account(), 32)) );
			(*it)->setText( 1, contact->onlineStatus().statusTypeToString(status.status()) );
			break;
		}
	}

}

void KopeteAccountConfig::slotAddWizardDone()
{
	save();
	load();
}

#include "kopeteaccountconfig.moc"

// vim: set noet ts=4 sts=4 sw=4:

