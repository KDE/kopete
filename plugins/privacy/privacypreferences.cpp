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

#include "privacypreferences.h"
#include "privacyconfig.h"
#include "ui_privacydialog.h"

#include <kgenericfactory.h>
#include <kinputdialog.h>

#include <QLayout>
#include <QVBoxLayout>

typedef KGenericFactory<PrivacyPreferences> PrivacyPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_privacy, PrivacyPreferencesFactory( "kcm_kopete_privacy" ) )

PrivacyPreferences::PrivacyPreferences(QWidget *parent, const QStringList &args)
	: KCModule(PrivacyPreferencesFactory::instance(), parent, args)
{
	kDebug() << k_funcinfo << "called." << endl;
	
	QVBoxLayout* layout = new QVBoxLayout( this );
	QWidget* widget = new QWidget;
	prefUi = new Ui::PrivacyPrefsUI;
	prefUi->setupUi( widget );
	layout->addWidget( widget );

	connect(prefUi->radioAllowAll, SIGNAL(toggled(bool)), this, SLOT(slotModified()));
	connect(prefUi->radioOnlyWhiteList, SIGNAL(toggled(bool)), this, SLOT(slotModified()));
	connect(prefUi->radioAllButBlackList, SIGNAL(toggled(bool)), this, SLOT(slotModified()));
	connect(prefUi->radioOnlyContactList, SIGNAL(toggled(bool)), this, SLOT(slotModified()));
	connect(prefUi->chkDropAtLeastOne, SIGNAL(toggled(bool)), this, SLOT(slotModified()));
	connect(prefUi->chkDropAtLeastOne, SIGNAL(toggled(bool)), this, SLOT(slotChkDropAtLeastOneToggled(bool)));
	connect(prefUi->chkDropAll, SIGNAL(toggled(bool)), this, SLOT(slotModified()));
	connect(prefUi->chkDropAll, SIGNAL(toggled(bool)), this, SLOT(slotChkDropAllToggled(bool)));
	connect(prefUi->editDropAll, SIGNAL(textChanged()), this, SLOT(slotModified()));
	connect(prefUi->editDropAtLeastOne, SIGNAL(textChanged()), this, SLOT(slotModified()));
	connect(prefUi->listWhiteList, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(slotModified()));
	connect(prefUi->listBlackList, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(slotModified()));

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
	kDebug() << k_funcinfo << "called." << endl;
	delete prefUi;
}

void PrivacyPreferences::load()
{
	kDebug() << k_funcinfo << "called." << endl;

	PrivacyConfig::self()->readConfig();

	prefUi->radioAllowAll->setChecked( PrivacyConfig::sender_AllowAll() );
	prefUi->radioOnlyWhiteList->setChecked( PrivacyConfig::sender_AllowNoneButWhiteList() );
	prefUi->listWhiteList->addItems( PrivacyConfig::whiteList() );
	prefUi->radioAllButBlackList->setChecked( PrivacyConfig::sender_AllowAllButBlackList() );
	prefUi->listBlackList->addItems( PrivacyConfig::blackList() );
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
	kDebug(14310) << k_funcinfo << "called." << endl;
	PrivacyConfig::setSender_AllowAll(prefUi->radioAllowAll->isChecked());
	PrivacyConfig::setSender_AllowNoneButWhiteList(prefUi->radioOnlyWhiteList->isChecked());
	QStringList whiteList;
	for( int i = 0; i < prefUi->listWhiteList->count(); ++i )
	{
		whiteList.append( prefUi->listWhiteList->item(i)->text() );
	}	
	PrivacyConfig::setWhiteList(whiteList);
	PrivacyConfig::setSender_AllowAllButBlackList(prefUi->radioAllButBlackList->isChecked());
	QStringList blackList;
	for( int i = 0; i < prefUi->listBlackList->count(); ++i )
	{
		blackList.append( prefUi->listWhiteList->item(i)->text() );
	}	
	PrivacyConfig::setBlackList(blackList);
	PrivacyConfig::setSender_AllowNoneButContactList(prefUi->radioOnlyContactList->isChecked());

	PrivacyConfig::setContent_DropIfAny(prefUi->chkDropAtLeastOne->isChecked());
	PrivacyConfig::setDropIfAny(prefUi->editDropAtLeastOne->text());
	PrivacyConfig::setContent_DropIfAll(prefUi->chkDropAll->isChecked());
	PrivacyConfig::setDropIfAll(prefUi->editDropAll->text());

	PrivacyConfig::self()->writeConfig();
	emit KCModule::changed(false);
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
	bool ok;
	QString contact = KInputDialog::getText(
		i18n( "Add contact to Whitelist" ), i18n( "Please enter the userId of the contact that should be whitelisted" ), QString::null, &ok );
	
	if( !ok )
		return;

	prefUi->listWhiteList->addItem( contact );

	emit KCModule::changed(true);
}

void PrivacyPreferences::slotBtnAddToBlackListClicked()
{
	bool ok;
	QString contact = KInputDialog::getText(
		i18n( "Add contact to Blacklist" ), i18n( "Please enter the userId of the contact that should be blacklisted" ), QString::null, &ok );
	
	if( !ok )
		return;

	prefUi->listBlackList->addItem( contact );

	emit KCModule::changed(true);
}

void PrivacyPreferences::slotBtnClearWhiteListClicked()
{
	prefUi->listWhiteList->clear();

	emit KCModule::changed(true);
}

void PrivacyPreferences::slotBtnClearBlackListClicked()
{
	prefUi->listBlackList->clear();

	emit KCModule::changed(true);
}

void PrivacyPreferences::slotBtnRemoveFromWhiteListClicked()
{
	prefUi->listWhiteList->takeItem( prefUi->listWhiteList->currentRow() );
	
	emit KCModule::changed(true);
}

void PrivacyPreferences::slotBtnRemoveFromBlackListClicked()
{
	prefUi->listBlackList->takeItem( prefUi->listBlackList->currentRow() );
	emit KCModule::changed(true);
}

#include "privacypreferences.moc"
