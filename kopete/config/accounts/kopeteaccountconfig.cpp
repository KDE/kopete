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

#include "kopeteaccountconfig.h"

#include <qcheckbox.h>
#include <qlayout.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kdialogbase.h>
#include <klistview.h>
#include <kcolorbutton.h>
#include <kgenericfactory.h>
#include <ktrader.h>
#include <kdebug.h>
#include <qfileinfo.h>
#include <kstandarddirs.h>
#include <kcombobox.h>

#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "addaccountwizard.h"
#include "kopeteaccountconfigbase.h"
#include "editaccountwidget.h"
#include "kopeteaccountmanager.h"


typedef KGenericFactory<KopeteAccountConfig, QWidget> KopeteAccountConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_accountconfig, KopeteAccountConfigFactory( "kcm_kopete_accountconfig" ) );


KopeteAccountConfig::KopeteAccountConfig(QWidget *parent, const char * /* name */, const QStringList &args) :
		KCModule( KopeteAccountConfigFactory::instance(), parent, args )
{
	previousAccount = 0L;

	(new QVBoxLayout(this))->setAutoAdd(true);
	m_view = new KopeteAccountConfigBase(this, "KopeteAccountConfig::m_view");

	m_view->mButtonUp->setPixmap(SmallIcon("up"));
	m_view->mButtonDown->setPixmap(SmallIcon("down"));
	m_view->mAccountList->setSorting(-1);

	connect(m_view->mButtonNew, SIGNAL(clicked()),
		this, SLOT(slotAddAccount()));
	connect(m_view->mAccountList, SIGNAL(doubleClicked(QListViewItem*)),
		this, SLOT(slotEditAccount()));
	connect(m_view->mButtonEdit, SIGNAL(clicked()),
		this, SLOT(slotEditAccount()));
	connect(m_view->mButtonRemove, SIGNAL(clicked()),
		this, SLOT(slotRemoveAccount()));
	connect(m_view->mAccountList, SIGNAL(selectionChanged()),
		this, SLOT(slotItemSelected()));
	connect(m_view->mButtonUp, SIGNAL(clicked()),
		this, SLOT(slotAccountUp()));
	connect(m_view->mButtonDown, SIGNAL(clicked()),
		this, SLOT(slotAccountDown()));

	QStringList mChatStyles = KGlobal::dirs()->findAllResources(
		"appdata", QString::fromLatin1("styles/*.xsl") );
	m_view->mStylesheet->clear();
	for( QStringList::Iterator it = mChatStyles.begin(); it != mChatStyles.end(); ++it)
	{
		QFileInfo fi( *it );
		QString fileName = fi.fileName().section('.',0,0);
		m_view->mStylesheet->insertItem( fileName );
		itemMap.insert(fileName, *it);
	}
		
	setButtons(Help);
	load();
}

void KopeteAccountConfig::save()
{
	if(previousAccount)
	{
		previousAccount->setColor(
			m_view->mUseColor->isChecked() ? m_view->mColorButton->color() : QColor() );
		previousAccount->setStylesheet(
			m_view->mUseStylesheet->isChecked() ? itemMap[ m_view->mStylesheet->currentText() ] : QString::null );
	}

	KopeteAccountManager::manager()->save();
}

void KopeteAccountConfig::load()
{
	QListViewItem* lvi=0L;

	m_view->mAccountList->clear();
	m_accountItems.clear();

	QPtrList<KopeteAccount> accounts=KopeteAccountManager::manager()->accounts();
	for(KopeteAccount *i=accounts.first() ; i; i=accounts.next() )
	{
		lvi=new QListViewItem(m_view->mAccountList,lvi);  //insert the item after the previous
		lvi->setText(0, i->protocol()->displayName());
		lvi->setPixmap(0, SmallIcon(i->protocol()->pluginIcon()));
		lvi->setText(1, i->accountId());
		m_accountItems.insert(lvi,i);
	}

	slotItemSelected();
}

void KopeteAccountConfig::slotItemSelected()
{
	QListViewItem *itemSelected = m_view->mAccountList->selectedItem();

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

	//we shouldn't really save data before apply :-s
	if(previousAccount)
	{
		previousAccount->setColor(
			m_view->mUseColor->isChecked() ? m_view->mColorButton->color() : QColor() );
			
		previousAccount->setStylesheet(
			m_view->mUseStylesheet->isChecked() ? itemMap[ m_view->mStylesheet->currentText() ] : QString::null );
	}

	KopeteAccount *a = m_accountItems[itemSelected];
	previousAccount = a;
	if(a)
	{
		m_view->mUseColor->setEnabled(true);
		m_view->mUseStylesheet->setEnabled(true);
		m_view->mUseColor->setChecked(a->color().isValid());
		m_view->mUseStylesheet->setChecked( !(a->styleSheet().isNull() || a->styleSheet().isEmpty()) );
		m_view->mColorButton->setColor(a->color());
		m_view->mStylesheet->setCurrentItem( QFileInfo(a->styleSheet()).fileName().section('.',0,0) );
		m_view->mColorButton->setEnabled(m_view->mUseColor->isChecked());
		m_view->mStylesheet->setEnabled( m_view->mUseStylesheet->isChecked() );
	}
	else
	{
		m_view->mUseColor->setEnabled(false);
		m_view->mColorButton->setEnabled(false);
		m_view->mStylesheet->setEnabled(false);
		m_view->mUseStylesheet->setEnabled(false);
	}
}

void KopeteAccountConfig::slotAccountUp()
{
	QListViewItem *itemSelected = m_view->mAccountList->selectedItem();
	if(!itemSelected)
		return;
	if(itemSelected->itemAbove())
		itemSelected->itemAbove()->moveItem( itemSelected );

	KopeteAccountManager::manager()->moveAccount( m_accountItems[itemSelected] , KopeteAccountManager::Up );

	slotItemSelected();
}

void KopeteAccountConfig::slotAccountDown()
{
	QListViewItem *itemSelected = m_view->mAccountList->selectedItem();
	if(!itemSelected)
		return;
	itemSelected->moveItem( itemSelected->itemBelow() );

	KopeteAccountManager::manager()->moveAccount( m_accountItems[itemSelected] , KopeteAccountManager::Down );

	slotItemSelected();
}

void KopeteAccountConfig::slotAddAccount()
{
	AddAccountWizard *m_addwizard;
	m_addwizard= new AddAccountWizard(this, "addAccountWizard", true);
	connect(m_addwizard, SIGNAL(destroyed(QObject*)), this, SLOT(slotAddWizardDone()));
	m_addwizard->show();
}

void KopeteAccountConfig::slotEditAccount()
{
	QListViewItem *lvi=m_view->mAccountList->selectedItem();
	if(!lvi)
		return;

	KopeteAccount *ident=m_accountItems[lvi];
	KopeteProtocol *proto=ident->protocol();

	KDialogBase *editDialog=new KDialogBase(this,"KopeteAccountConfig::editDialog",
		true, i18n("Edit Account"),
		KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true );

	EditAccountWidget *m_accountWidget = proto->createEditAccountWidget(ident,editDialog);
	if (!m_accountWidget)
		return;

	editDialog->setMainWidget(dynamic_cast<QWidget*>(m_accountWidget));
	if(editDialog->exec() == QDialog::Accepted)
	{
		if(m_accountWidget->validateData())
			m_accountWidget->apply();
	}
	editDialog->deleteLater();
	load();
}

void KopeteAccountConfig::slotRemoveAccount()
{
	QListViewItem *lvi=m_view->mAccountList->selectedItem();
	if(!lvi)
		return;

	KopeteAccount *i=m_accountItems[lvi];
	if(KMessageBox::warningContinueCancel(this,
		i18n("Are you sure you want to remove the account \"%1\"?").arg(i->accountId()),
		i18n("Remove Account"),
		i18n("Remove Account")) == KMessageBox::Continue )
	{
		previousAccount=0L;
		m_accountItems.remove(lvi);
		KopeteAccountManager::manager()->removeAccount(i);
		delete lvi;
	}
}

void KopeteAccountConfig::slotAddWizardDone()
{
	load();
}

#include "kopeteaccountconfig.moc"
// vim: set noet ts=4 sts=4 sw=4:
