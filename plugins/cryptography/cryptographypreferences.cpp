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

#include <QPushButton>
#include <QCheckBox>

#include <klineedit.h>
#include <kgenericfactory.h>
#include <kleo/ui/keyrequester.h>

#include "ui_cryptographyprefsbase.h"
#include "cryptographypreferences.h"
#include "cryptographypreferences.moc"

#include "cryptographyconfig.h"

typedef KGenericFactory<CryptographyPreferences> CryptographyPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY ( kcm_kopete_cryptography, CryptographyPreferencesFactory ( "kcm_kopete_cryptography" ) )

CryptographyPreferences::CryptographyPreferences ( QWidget *parent, const QStringList &args )
		: KCModule ( CryptographyPreferencesFactory::componentData(), parent, args )
{
	mConfig = new CryptographyConfig;
	
	// Add actual widget generated from ui file.
	QVBoxLayout* l = new QVBoxLayout ( this );
	QWidget *w = new QWidget;

	key = new Kleo::EncryptionKeyRequester ( false, Kleo::EncryptionKeyRequester::OpenPGP, this, true, true );
	key->setDialogMessage ( i18n ( "Select the secret key you want to use encrypt and or decrypt messages" ) );
	key->setDialogCaption ( i18n ( "Select the secret key you want to use encrypt and or decrypt messages" ) );

	QLabel * label = new QLabel ( i18n ("With this plugin you can encrypt messages so that nobody but your intended recipient can read them, and you can also sign messages, so that recipients can verify that a given message has actually come from you. <a href=\"http://en.wikipedia.org/wiki/Public-key_cryptography\">How this works</a>.\n\nBefore you can send encrypted messages to someone, you must select their public key by right-clicking on their name in your contact list, and choosing \"Select Public Key.\"\n\nNote: All messages become plain text when used with this plugin"), this );
	label->setWordWrap (true);
	
	mAskPassphraseOnStartup = new QCheckBox ( i18n ("Ask for passphrase on Kopete startup" ), this);

	mPreferencesDialog = new Ui::CryptographyPrefsUI;
	mPreferencesDialog->setupUi ( w );

	l->addWidget ( key );
	l->addWidget ( label );
	l->addWidget ( mAskPassphraseOnStartup );
	l->addWidget ( w );
	l->addStretch ();

	connect ( mAskPassphraseOnStartup, SIGNAL ( toggled ( bool ) ), this, SLOT ( slotAskOnStartupPressed ( bool ) ) );

	connect ( key->dialogButton(), SIGNAL ( clicked() ), this, SLOT ( slotModified() ) );
	connect ( key->eraseButton(), SIGNAL ( clicked() ), this, SLOT ( slotModified() ) );
	connect ( mAskPassphraseOnStartup, SIGNAL ( toggled ( bool ) ), this, SLOT ( slotModified() ) );
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
	mAskPassphraseOnStartup->setChecked ( mConfig->askPassphraseOnStartup() );
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
	mConfig->setAskPassphraseOnStartup ( mAskPassphraseOnStartup->isChecked() );
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
	mAskPassphraseOnStartup->setChecked ( false );
	mPreferencesDialog->onClose->setChecked ( true );
	mPreferencesDialog->cacheTime->setValue ( 15 );
	slotModified();
}

void CryptographyPreferences::slotAskOnStartupPressed ( bool b )
{
	mPreferencesDialog->never->setEnabled (!b);
	if (b)
		mPreferencesDialog->onClose->setChecked (true);
}

void CryptographyPreferences::slotModified()
{
	emit changed ( true );
}

// vim: set noet ts=4 sts=4 sw=4:
