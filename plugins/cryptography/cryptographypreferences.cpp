/*
    cryptographypreferences.cpp  -  description

    Copyright (c) 2002      by Olivier Goffart <ogoffart@kde.org>
    Copyright (c) 2007      by Charles Connell <charles@connells.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#include <qpushbutton.h>

#include <klineedit.h>
#include <kgenericfactory.h>

#include "ui_cryptographyprefsbase.h"
#include "cryptographypreferences.h"
#include "cryptographypreferences.moc"

#include "cryptographyconfig.h"

typedef KGenericFactory<CryptographyPreferences> CryptographyPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY ( kcm_kopete_cryptography, CryptographyPreferencesFactory ( "kcm_kopete_cryptography" ) )

CryptographyPreferences::CryptographyPreferences ( QWidget *parent, const QStringList &args )
		: KCModule ( CryptographyPreferencesFactory::componentData(), parent, args )
{
	// Add actual widget generated from ui file.
	QVBoxLayout* l = new QVBoxLayout ( this );
	QWidget *w = new QWidget;
	mPreferencesDialog = new Ui::CryptographyPrefsUI;
	mPreferencesDialog->setupUi ( w );
	
	key = new Kleo::EncryptionKeyRequester (false, Kleo::EncryptionKeyRequester::OpenPGP, this, true, true);
	key->setDialogMessage (i18n ("Select the secret key you want to use encrypt and or decrypt messages"));
	key->setDialogCaption (i18n ("Select the secret key you want to use encrypt and or decrypt messages"));
	
	l->addWidget (key);
	l->addWidget ( w );
	mConfig = new CryptographyConfig;

	connect ( mPreferencesDialog->askPassPhrase, SIGNAL ( toggled ( bool ) ), this, SLOT ( slotAskPressed ( bool ) ) );

	connect ( key->dialogButton(), SIGNAL ( clicked() ), this, SLOT ( slotModified() ) );
	connect ( key->eraseButton(), SIGNAL ( clicked() ), this, SLOT (slotModified() ) );
	connect ( mPreferencesDialog->askPassPhrase, SIGNAL ( toggled ( bool ) ), this, SLOT ( slotModified() ) );
	connect ( mPreferencesDialog->onClose, SIGNAL ( toggled ( bool ) ), this, SLOT ( slotModified() ) );
	connect ( mPreferencesDialog->time, SIGNAL ( toggled ( bool ) ), this, SLOT ( slotModified() ) );
	connect ( mPreferencesDialog->never, SIGNAL ( toggled ( bool ) ), this, SLOT ( slotModified() ) );
	connect ( mPreferencesDialog->cacheTime, SIGNAL ( valueChanged ( int ) ), this, SLOT ( slotModified() ) );
	
	load();
}

CryptographyPreferences::~CryptographyPreferences()
{
	delete mPreferencesDialog;
	delete mConfig;
}

void CryptographyPreferences::load()
{
	mConfig->load();

	key->setFingerprint ( mConfig->fingerprint() );
	mPreferencesDialog->askPassPhrase->setChecked ( mConfig->askPassPhrase() );
	mPreferencesDialog->cacheTime->setValue ( mConfig->cacheTime() );

	if ( mConfig->cacheTime() == CryptographyConfig::Close )
		mPreferencesDialog->onClose->setChecked ( true );
	if ( mConfig->cacheTime() == CryptographyConfig::Time )
		mPreferencesDialog->time->setChecked ( true );
	if ( mConfig->cacheTime() == CryptographyConfig::Never )
		mPreferencesDialog->never->setChecked ( true );


	KCModule::load();
	emit changed ( false );
}

void CryptographyPreferences::save()
{
	mConfig->setFingerprint ( key->fingerprint() );
	mConfig->setAskPassPhrase ( mPreferencesDialog->askPassPhrase->isChecked() );
	mConfig->setCacheTime ( mPreferencesDialog->cacheTime->value() );

	if ( mPreferencesDialog->onClose->isChecked() )
		mConfig->setCacheMode ( CryptographyConfig::Close );
	if ( mPreferencesDialog->time->isChecked() )
		mConfig->setCacheMode ( CryptographyConfig::Time );
	if ( mPreferencesDialog->never->isChecked() )
		mConfig->setCacheMode ( CryptographyConfig::Never );

	mConfig->save();

	KCModule::save();
	emit changed ( false );
}

void CryptographyPreferences::defaults()
{
	key->eraseButton()->click();
	mPreferencesDialog->askPassPhrase->setChecked(false);
	mPreferencesDialog->onClose->setChecked(true);
	mPreferencesDialog->cacheTime->setValue(15);
	slotModified();
}

void CryptographyPreferences::slotAskPressed ( bool b )
{
	mPreferencesDialog->cacheBehavior->setEnabled ( !b );
}

void CryptographyPreferences::slotModified()
{
	emit changed ( true );
}

// vim: set noet ts=4 sts=4 sw=4:
