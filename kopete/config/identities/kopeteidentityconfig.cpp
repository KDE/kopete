/*
    identityconfig.cpp  -  Kopete identity config page

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

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

#include "kopeteidentityconfig.h"

#include <QHeaderView>
#include <KGenericFactory>
#include <KMessageBox>

#include "addidentitydialog.h"
#include "accountidentitydialog.h"
#include "identitydialog.h"
#include "kopeteidentitymanager.h"
#include "kopeteidentity.h"

class KopeteIdentityLVI : public QTreeWidgetItem
{
	public:
		KopeteIdentityLVI( Kopete::Identity *i, QTreeWidget* parent) : QTreeWidgetItem(parent) { 
		m_identity = i; }
		Kopete::Identity *identity() { return m_identity; }

	private:
		Kopete::Identity *m_identity;
};

typedef KGenericFactory<KopeteIdentityConfig, QWidget> KopeteIdentityConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_identityconfig, KopeteIdentityConfigFactory( "kcm_kopete_identityconfig" ) )

KopeteIdentityConfig::KopeteIdentityConfig( QWidget *parent, const QStringList &args )
: KCModule( KopeteIdentityConfigFactory::componentData(), parent, args )
{
	setupUi( this );

	QHeaderView *header = mIdentityList->header();
	header->setResizeMode( 0, QHeaderView::Stretch );
	header->setVisible(false);

	mButtonNew->setIcon( KIcon("edit-add") );
	mButtonRemove->setIcon( KIcon("edit-delete") );

	connect( mButtonNew,     SIGNAL( clicked() ), this, SLOT( slotAddIdentity() ) );
	connect( mButtonEdit,    SIGNAL( clicked() ), this, SLOT( slotEditIdentity() ) );
	connect( mButtonDefault, SIGNAL( clicked() ), this, SLOT( slotSetDefaultIdentity() ) );
	connect( mButtonRemove,  SIGNAL( clicked() ), this, SLOT( slotRemoveIdentity() ) );
	connect( mIdentityList,  SIGNAL( itemSelectionChanged() ), this, SLOT( slotItemSelected() ) );
	connect( mIdentityList,  SIGNAL( itemDoubleClicked(QTreeWidgetItem*, int) ), this, SLOT( slotEditIdentity() ) );

	// identity manager signals
	Kopete::IdentityManager *manager = Kopete::IdentityManager::self();
	connect( manager, SIGNAL(identityRegistered(Kopete::Identity*)), this, SLOT(load()));
	connect( manager, SIGNAL(identityUnregistered(Kopete::Identity*)), this, SLOT(load()));
	connect( manager, SIGNAL(defaultIdentityChanged(Kopete::Identity*)), this, SLOT(load()));

	setButtons( Help );
	load();
}

KopeteIdentityLVI* KopeteIdentityConfig::selectedIdentity()
{
	QList<QTreeWidgetItem*> selectedItems = mIdentityList->selectedItems();
	if(!selectedItems.empty())
 		return static_cast<KopeteIdentityLVI*>( selectedItems.first() );
	return 0;
}

void KopeteIdentityConfig::save()
{
	Kopete::IdentityManager::self()->save();
	load(); 
}

void KopeteIdentityConfig::load()
{
	KopeteIdentityLVI *lvi = 0L;

	mIdentityList->clear();

	Kopete::Identity *defaultIdentity = Kopete::IdentityManager::self()->defaultIdentity();
	foreach(Kopete::Identity *i, Kopete::IdentityManager::self()->identities())
	{
		// Insert the item after the previous one
		lvi = new KopeteIdentityLVI( i, mIdentityList );
		lvi->setText( 0, i->identityId() );
		lvi->setIcon( 0, KIcon( i->customIcon()) );
		
		if (i == defaultIdentity)
		{
			QFont font = lvi->font( 0 );
			font.setBold( true );
			lvi->setFont( 0, font );
		}

		lvi->setSizeHint( 0, QSize(0, 42) );
		
	}

	slotItemSelected();
}

void KopeteIdentityConfig::slotItemSelected()
{
	m_protected=true;

	KopeteIdentityLVI *itemSelected = selectedIdentity();

	mButtonEdit->setEnabled( itemSelected );
	mButtonDefault->setEnabled( itemSelected );
	if ( Kopete::IdentityManager::self()->identities().count() <= 1 )
		mButtonRemove->setEnabled( false );
	else
		mButtonRemove->setEnabled( itemSelected );

	m_protected=false;
}

void KopeteIdentityConfig::slotAddIdentity()
{
	Kopete::Identity *ident = AddIdentityDialog::getIdentity(this);

	if (!ident)
		return;

	ident = Kopete::IdentityManager::self()->registerIdentity(ident);
	if (ident)
	{
		IdentityDialog dialog(ident, this);
		dialog.exec();
	}

	save();
}

void KopeteIdentityConfig::slotEditIdentity()
{
	KopeteIdentityLVI *lvi = selectedIdentity();
	
	if ( !lvi || !lvi->identity() )
		return;

	Kopete::Identity *ident = lvi->identity();

	IdentityDialog dialog(ident, this);
	dialog.exec();

	load();
	Kopete::IdentityManager::self()->save();
}

void KopeteIdentityConfig::slotSetDefaultIdentity()
{
	KopeteIdentityLVI *lvi = selectedIdentity();
	
	if ( !lvi || !lvi->identity() )
		return;

	Kopete::IdentityManager::self()->setDefaultIdentity( lvi->identity() );
}

void KopeteIdentityConfig::slotRemoveIdentity()
{
	KopeteIdentityLVI *lvi = selectedIdentity();
	
	if ( !lvi || !lvi->identity() )
		return;

	Kopete::Identity *i = lvi->identity();

	if (!i->accounts().count())
	{
		if ( KMessageBox::warningContinueCancel( this, i18n( "Are you sure you want to remove the identity \"%1\"?", i->identityId() ),
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
					i18n("Before removing the identity %1, the following accounts must be" 
						 "assigned to another identity:", i->identityId())) )
		{
			Kopete::IdentityManager::self()->removeIdentity( i );
			delete lvi;
		}
	}
	// if we removed the default identity, this will trigger an update
	Kopete::IdentityManager::self()->defaultIdentity();
}

#include "kopeteidentityconfig.moc"

// vim: set noet ts=4 sts=4 sw=4:

