/*
    accountconfig.cpp  -  Kopete account config page

    Copyright (c) 2003 by Olivier Goffart <ogoffart@tiscalinet.be>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "accountconfig.h"

#include <qpushbutton.h>
#include <qlistview.h>
#include <qlayout.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kdialogbase.h>

#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "addaccountwizard.h"
#include "accountconfigbase.h"
#include "editaccountwidget.h"
#include "kopeteaccountmanager.h"

IdentityConfig::IdentityConfig(QWidget * parent) :
	ConfigModule (i18n("Accounts"),i18n("Here You Can Manage Your Accounts"),"personal", parent )
{
	QVBoxLayout *topLayout = new QVBoxLayout( this );
	m_view=new IdentityConfigBase(this);
	topLayout->add(m_view);

	m_view->mButtonUp->setPixmap( SmallIcon("up") );
	m_view->mButtonDown->setPixmap( SmallIcon("down") );
	m_view->mIdentityList->setSorting( -1 );

	connect( m_view->mButtonNew , SIGNAL(clicked() ) , this , SLOT( slotAddIdentity()));
	connect( m_view->mButtonEdit , SIGNAL(clicked() ) , this , SLOT( slotEditIdentity()));
	connect( m_view->mButtonRemove , SIGNAL(clicked() ) , this , SLOT( slotRemoveIdentity()));
	connect( m_view->mIdentityList , SIGNAL(selectionChanged() ) , this , SLOT( slotItemSelected()));
	connect( m_view->mButtonUp , SIGNAL(clicked() ) , this , SLOT( slotIdentityUp()));
	connect( m_view->mButtonDown , SIGNAL(clicked() ) , this , SLOT( slotIdentityDown()));

	slotItemSelected();
//	m_addwizard=0L;
}

IdentityConfig::~IdentityConfig()
{
}

void IdentityConfig::save()
{
	KopeteIdentityManager::manager()->save();
}

void IdentityConfig::reopen()
{
	m_view->mIdentityList->clear();
	m_identityItems.clear();

	QPtrList<KopeteIdentity>  identities = KopeteIdentityManager::manager()->identities();
	for(KopeteIdentity *i=identities.first() ; i; i=identities.next() )
	{
		QListViewItem* lvi= new QListViewItem(m_view->mIdentityList);
		lvi->setText(0,i->protocol()->displayName() + QString::fromLatin1(" ") );
		lvi->setPixmap( 0, SmallIcon( i->protocol()->pluginIcon() ) );
		lvi->setText(1, i->identityId());
		m_identityItems.insert(lvi,i);
	}
}

void IdentityConfig::slotItemSelected()
{
	QListViewItem *itemSelected = m_view->mIdentityList->selectedItem();

	m_view->mButtonEdit->setEnabled( itemSelected );
	m_view->mButtonRemove->setEnabled( itemSelected );

	if( itemSelected )
	{
		m_view->mButtonUp->setEnabled( itemSelected->itemAbove() );
		m_view->mButtonDown->setEnabled( itemSelected->itemBelow() );
	}
	else
	{
		m_view->mButtonUp->setEnabled( itemSelected );
		m_view->mButtonDown->setEnabled( itemSelected );
	}
}

void IdentityConfig::slotIdentityUp()
{
	QListViewItem *itemSelected = m_view->mIdentityList->selectedItem();
	itemSelected->itemAbove()->moveItem( itemSelected );

	slotItemSelected();
}

void IdentityConfig::slotIdentityDown()
{
	QListViewItem *itemSelected = m_view->mIdentityList->selectedItem();
	itemSelected->moveItem( itemSelected->itemBelow() );

	slotItemSelected();
}

void IdentityConfig::slotAddIdentity()
{
	AddIdentityWizard *m_addwizard;
	m_addwizard= new AddIdentityWizard( this , "addIdentityWizard" , true);
	connect(m_addwizard, SIGNAL( destroyed(QObject*)) , this, SLOT (slotAddWizardDone()));
	m_addwizard->show();
}

void IdentityConfig::slotEditIdentity()
{
	QListViewItem *lvi=m_view->mIdentityList->selectedItem();
	if(!lvi)
		return;

	KopeteIdentity *ident=m_identityItems[lvi];
	KopeteProtocol *proto=ident->protocol();
	
	KDialogBase *editDialog= new KDialogBase( this,"editDialog", true, i18n( "Edit Account" ),
				KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true );
	editDialog->resize( 525, 400 );
	EditIdentityWidget *m_identityWidget = proto->createEditIdentityWidget(ident,editDialog);
	QWidget *qWidget = dynamic_cast<QWidget*>( m_identityWidget );

	if (!m_identityWidget)
		return;

	editDialog->setMainWidget(qWidget);
	if(editDialog->exec() == QDialog::Accepted )
	{
		if(m_identityWidget->validateData())
		{
			m_identityWidget->apply();
		}
	}
	editDialog->deleteLater();
	reopen();
}

void IdentityConfig::slotRemoveIdentity()
{
	QListViewItem *lvi=m_view->mIdentityList->selectedItem();
	if(lvi)
	{
		KopeteIdentity *i=m_identityItems[lvi];
		if( KMessageBox::questionYesNo( this, i18n("Are you sure you want to remove the account %1").arg( i->identityId() ), i18n("Remove Identity")) == KMessageBox::Yes )
		{
			m_identityItems.remove(lvi);
			i->deleteLater();
			delete lvi;
		}
	}
}

void IdentityConfig::slotAddWizardDone()
{
	reopen();
}

#include "accountconfig.moc"


