/*
    privacypreferences.cpp

    Copyright (c) 2006 by Andre Duffeck             <andre@duffeck.de>
    Kopete    (c) 2003-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <QLayout>
#include <QVBoxLayout>
#include <QHeaderView>

#include <kgenericfactory.h>
#include <kinputdialog.h>
#include <kdialog.h>
#include <kvbox.h>

#include "kopeteuiglobal.h"
#include "kopetemetacontact.h"
#include "kopetepluginmanager.h"
#include "metacontactselectorwidget.h"
#include "privacyconfig.h"
#include "ui_privacydialog.h"
#include "privacyaccountlistmodel.h"
#include "privacypreferences.h"

typedef KGenericFactory<PrivacyPreferences> PrivacyPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_privacy, PrivacyPreferencesFactory( "kcm_kopete_privacy" ) )

PrivacyPreferences::PrivacyPreferences(QWidget *parent, const QStringList &args)
	: KCModule(PrivacyPreferencesFactory::instance(), parent, args)
{
	kDebug(14313) << k_funcinfo << "called." << endl;
	
	QVBoxLayout* layout = new QVBoxLayout( this );
	QWidget* widget = new QWidget;
	prefUi = new Ui::PrivacyPrefsUI;
	prefUi->setupUi( widget );
	layout->addWidget( widget );

	m_whiteListModel = new PrivacyAccountListModel;
	m_blackListModel = new PrivacyAccountListModel;

	prefUi->listWhiteList->setSelectionBehavior( QAbstractItemView::SelectRows );
	prefUi->listWhiteList->horizontalHeader()->hide();
	prefUi->listWhiteList->verticalHeader()->hide();
	prefUi->listWhiteList->setModel( m_whiteListModel );
	prefUi->listBlackList->setSelectionBehavior( QAbstractItemView::SelectRows );
	prefUi->listBlackList->horizontalHeader()->hide();
	prefUi->listBlackList->verticalHeader()->hide();
	prefUi->listBlackList->setModel( m_blackListModel );

	connect(PrivacyConfig::self(), SIGNAL(configChanged()), this, SLOT(slotConfigChanged()));

	connect(prefUi->radioAllowAll, SIGNAL(toggled(bool)), this, SLOT(slotModified()));
	connect(prefUi->radioOnlyWhiteList, SIGNAL(toggled(bool)), this, SLOT(slotModified()));
	connect(prefUi->radioAllButBlackList, SIGNAL(toggled(bool)), this, SLOT(slotModified()));
	connect(prefUi->radioOnlyContactList, SIGNAL(toggled(bool)), this, SLOT(slotModified()));
	connect(prefUi->chkDropAtLeastOne, SIGNAL(toggled(bool)), this, SLOT(slotModified()));
	connect(prefUi->chkDropAtLeastOne, SIGNAL(toggled(bool)), prefUi->editDropAtLeastOne, SLOT(setEnabled(bool)));
	connect(prefUi->chkDropAll, SIGNAL(toggled(bool)), this, SLOT(slotModified()));
	connect(prefUi->chkDropAll, SIGNAL(toggled(bool)), prefUi->editDropAll, SLOT(setEnabled(bool)));
	connect(prefUi->editDropAll, SIGNAL(textChanged(const QString &)), this, SLOT(slotModified()));
	connect(prefUi->editDropAtLeastOne, SIGNAL(textChanged(const QString &)), this, SLOT(slotModified()));

	connect(prefUi->btnAddToWhiteList, SIGNAL(clicked()), this, SLOT(slotBtnAddToWhiteListClicked()));
	connect(prefUi->btnAddToBlackList, SIGNAL(clicked()), this, SLOT(slotBtnAddToBlackListClicked()));
	connect(prefUi->btnClearWhiteList, SIGNAL(clicked()), this, SLOT(slotBtnClearWhiteListClicked()));
	connect(prefUi->btnClearBlackList, SIGNAL(clicked()), this, SLOT(slotBtnClearBlackListClicked()));
	connect(prefUi->btnRemoveFromWhiteList, SIGNAL(clicked()), this, SLOT(slotBtnRemoveFromWhiteListClicked()));
	connect(prefUi->btnRemoveFromBlackList, SIGNAL(clicked()), this, SLOT(slotBtnRemoveFromBlackListClicked()));
	load();
}

PrivacyPreferences::~PrivacyPreferences()
{
	kDebug(14313) << k_funcinfo << "called." << endl;
	delete prefUi;
	delete m_whiteListModel;
	delete m_blackListModel;
}

void PrivacyPreferences::load()
{
	kDebug(14313) << k_funcinfo << "called." << endl;

	PrivacyConfig::self()->readConfig();

	prefUi->radioAllowAll->setChecked( PrivacyConfig::sender_AllowAll() );
	prefUi->radioOnlyWhiteList->setChecked( PrivacyConfig::sender_AllowNoneButWhiteList() );
	m_whiteListModel->loadAccounts( PrivacyConfig::whiteList() );
	prefUi->radioAllButBlackList->setChecked( PrivacyConfig::sender_AllowAllButBlackList() );
	m_blackListModel->loadAccounts( PrivacyConfig::blackList() );
	prefUi->radioOnlyContactList->setChecked( PrivacyConfig::sender_AllowNoneButContactList() );

	prefUi->chkDropAtLeastOne->setChecked( PrivacyConfig::content_DropIfAny() );
	prefUi->editDropAtLeastOne->setText( PrivacyConfig::dropIfAny() );
	prefUi->editDropAtLeastOne->setEnabled( PrivacyConfig::content_DropIfAny() );
	prefUi->chkDropAll->setChecked( PrivacyConfig::content_DropIfAll() );
	prefUi->editDropAll->setText( PrivacyConfig::dropIfAll() );
	prefUi->editDropAll->setEnabled( PrivacyConfig::content_DropIfAll() );

	emit KCModule::changed(false);
}

void PrivacyPreferences::save()
{
	kDebug(14313) << k_funcinfo << "called." << endl;
	PrivacyConfig::setSender_AllowAll(prefUi->radioAllowAll->isChecked());
	PrivacyConfig::setSender_AllowNoneButWhiteList(prefUi->radioOnlyWhiteList->isChecked());
	PrivacyConfig::setWhiteList(m_whiteListModel->toStringList());
	PrivacyConfig::setSender_AllowAllButBlackList(prefUi->radioAllButBlackList->isChecked());
	PrivacyConfig::setBlackList(m_blackListModel->toStringList());
	PrivacyConfig::setSender_AllowNoneButContactList(prefUi->radioOnlyContactList->isChecked());

	PrivacyConfig::setContent_DropIfAny(prefUi->chkDropAtLeastOne->isChecked());
	PrivacyConfig::setDropIfAny(prefUi->editDropAtLeastOne->text());
	PrivacyConfig::setContent_DropIfAll(prefUi->chkDropAll->isChecked());
	PrivacyConfig::setDropIfAll(prefUi->editDropAll->text());

	PrivacyConfig::self()->writeConfig();
	emit KCModule::changed(false);
}

void PrivacyPreferences::slotConfigChanged()
{
	kDebug(14313) << k_funcinfo << "called." << endl;
	load();
}

void PrivacyPreferences::slotModified()
{
	emit KCModule::changed(true);
}

void PrivacyPreferences::slotChkDropAtLeastOneToggled( bool enabled )
{
	prefUi->editDropAtLeastOne->setEnabled( enabled );
}

void PrivacyPreferences::slotChkDropAllToggled( bool enabled )
{
	prefUi->editDropAll->setEnabled( enabled );
}

void PrivacyPreferences::slotBtnAddToWhiteListClicked()
{
	KDialog *addDialog = new KDialog( Kopete::UI::Global::mainWidget() );
	addDialog->setCaption( i18n( "Add Contact to Whitelist" ) );
	addDialog->setButtons( KDialog::Ok | KDialog::Cancel );
	addDialog->setDefaultButton( KDialog::Ok );
	addDialog->showButtonSeparator( true );

	KVBox *box = new KVBox( addDialog );
	box->setSpacing( KDialog::spacingHint() );
	Kopete::UI::MetaContactSelectorWidget *selector = new Kopete::UI::MetaContactSelectorWidget( box );
	selector->setLabelMessage(i18n( "Select the meta contact to which you want to add:" ));
	addDialog->setMainWidget(box);
	if( addDialog->exec() == QDialog::Accepted )
	{
		Kopete::MetaContact *mc = selector->metaContact();
		if( mc )
		{
			foreach( Kopete::Contact *contact, mc->contacts() )
			{
				
			}
		}
	}

	addDialog->deleteLater();

	emit KCModule::changed(true);
}

void PrivacyPreferences::slotBtnAddToBlackListClicked()
{
	emit KCModule::changed(true);
}

void PrivacyPreferences::slotBtnClearWhiteListClicked()
{
	if( m_whiteListModel->rowCount() )
		m_whiteListModel->removeRows( 0, m_whiteListModel->rowCount() );

	emit KCModule::changed(true);
}

void PrivacyPreferences::slotBtnClearBlackListClicked()
{
	if( m_blackListModel->rowCount() )
		m_blackListModel->removeRows( 0, m_blackListModel->rowCount() );

	emit KCModule::changed(true);
}

void PrivacyPreferences::slotBtnRemoveFromWhiteListClicked()
{
	foreach(QModelIndex index, prefUi->listWhiteList->selectionModel()->selectedRows() )
	{
		m_whiteListModel->removeRow( index.row() );
	}
	
	emit KCModule::changed(true);
}

void PrivacyPreferences::slotBtnRemoveFromBlackListClicked()
{
	foreach(QModelIndex index, prefUi->listBlackList->selectionModel()->selectedRows() )
	{
		m_blackListModel->removeRow( index.row() );
	}

	emit KCModule::changed(true);
}

#include "privacypreferences.moc"
