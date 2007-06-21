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

#include <QPointer>
#include <QHeaderView>
#include <KGenericFactory>
#include <KMessageBox>

#include "addidentitywizard.h"
#include "kopeteidentitymanager.h"
#include "kopeteidentity.h"

class KopeteIdentityLVI : public QTreeWidgetItem
{
	public:
		KopeteIdentityLVI( Kopete::Identity *i, QTreeWidget* parent) : QTreeWidgetItem(parent) { 
		m_identity = i; }
		Kopete::Identity *identity() { return m_identity; }

	private:
		//need to be guarded because some identitys may be linked (that's the case of jabber transports)
		QPointer<Kopete::Identity> m_identity;
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

	connect( mButtonNew,    SIGNAL( clicked() ), this, SLOT( slotAddIdentity() ) );
	connect( mButtonEdit,   SIGNAL( clicked() ), this, SLOT( slotEditIdentity() ) );
	connect( mButtonRemove, SIGNAL( clicked() ), this, SLOT( slotRemoveIdentity() ) );
	connect( mIdentityList,  SIGNAL( itemSelectionChanged() ), this, SLOT( slotItemSelected() ) );
	connect( mIdentityList,  SIGNAL( itemDoubleClicked(QTreeWidgetItem*, int) ), this, SLOT( slotEditIdentity() ) );
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

	foreach(Kopete::Identity *i, Kopete::IdentityManager::self()->identities())
	{
		// Insert the item after the previous one
		lvi = new KopeteIdentityLVI( i, mIdentityList );
		lvi->setText( 0, i->identityId() );
		lvi->setIcon( 0, KIcon( i->customIcon()) );
		QFont font = lvi->font( 0 );
		font.setBold( true );
		lvi->setFont( 0, font );

		lvi->setSizeHint( 0, QSize(0, 42) );
		
	}

	slotItemSelected();
}

void KopeteIdentityConfig::slotItemSelected()
{
	m_protected=true;

	KopeteIdentityLVI *itemSelected = selectedIdentity();

	mButtonEdit->setEnabled( itemSelected );
	mButtonRemove->setEnabled( itemSelected );

	m_protected=false;
}

void KopeteIdentityConfig::slotAddIdentity()
{

	AddIdentityWizard *m_addwizard = new AddIdentityWizard( this );
	connect( m_addwizard, SIGNAL(finished()), this, SLOT(slotAddWizardDone()) );
	m_addwizard->show();
}

void KopeteIdentityConfig::slotEditIdentity()
{
	KopeteIdentityLVI *lvi = selectedIdentity();
	
	if ( !lvi || !lvi->identity() )
		return;

	Kopete::Identity *ident = lvi->identity();

	//TODO implement

	load();
	Kopete::IdentityManager::self()->save();
}

void KopeteIdentityConfig::slotRemoveIdentity()
{
	KopeteIdentityLVI *lvi = selectedIdentity();
	
	if ( !lvi || !lvi->identity() )
		return;

	Kopete::Identity *i = lvi->identity();
	if ( KMessageBox::warningContinueCancel( this, i18n( "Are you sure you want to remove the identity \"%1\"?", i->identityId() ),
		i18n( "Remove Identity" ), KGuiItem(i18n( "Remove Identity" ), "edit-delete"), KStandardGuiItem::cancel(),
		 "askRemoveIdentity", KMessageBox::Notify | KMessageBox::Dangerous ) == KMessageBox::Continue )
	{
		//FIXME: should handle account moving to another identity
		Kopete::IdentityManager::self()->removeIdentity( i );
		delete lvi;
	}
}

void KopeteIdentityConfig::slotAddWizardDone()
{
	save();
}

#include "kopeteidentityconfig.moc"

// vim: set noet ts=4 sts=4 sw=4:

