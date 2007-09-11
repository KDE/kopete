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

K_PLUGIN_FACTORY ( CryptographyPreferencesFactory,
                   registerPlugin<CryptographyPreferences>();
                 )
K_EXPORT_PLUGIN ( CryptographyPreferencesFactory ( "kcm_kopete_cryptography" ) )


CryptographyPreferences::CryptographyPreferences ( QWidget *parent, const QVariantList &args )
		: KCModule ( CryptographyPreferencesFactory::componentData(), parent, args )
{
	// Add actual widget generated from ui file.
	QVBoxLayout* l = new QVBoxLayout ( this );
	QWidget *w = new QWidget;

	key = new Kleo::EncryptionKeyRequester ( false, Kleo::EncryptionKeyRequester::OpenPGP, this, true, true );
	key->setDialogMessage ( i18n ( "Select the key you want to use to decrypt and sign messages" ) );
	key->setDialogCaption ( i18n ( "Select the key you want to use to decrypt and sign messages" ) );
	key->setToolTip ( i18n ( "The private key used for decryption and signing" ) );
	key->setWhatsThis ( i18n ( "See and change the private key used for decryption and signing of messages using the Cryptography plugin" ) );

	QLabel * label = new QLabel ( i18n ( "With this plugin you can encrypt messages so that nobody but your intended recipient can read them, and you can also sign messages, so that recipients can verify that a given message has actually come from you."
	                                     "<a href=\"http://en.wikipedia.org/wiki/Public-key_cryptography\">How this works</a>.\n\n"
	                                     "Before you can send encrypted messages to someone, you must select their public key by right-clicking on their name in your contact list, and choosing \"Select Public Key.\"\n\n"
	                                     "All messages become plain text when used with this plugin" ), this );
	label->setWordWrap ( true );

	// We want the password available ASAP and forever so that upon decryption time, we can do it without prompting for password.
	// If another message arrives while the password prompt is still up, the second message will prompt again, and go above the first messages's prompt. upon decryption, messages will be displayed in a backwards order (VERY BAD)
	mAskPassphraseOnStartup = new QCheckBox ( i18n ( "Ask for passphrase on Kopete startup (Recommended)" ), this );
	mAskPassphraseOnStartup->setToolTip ( i18n ( "Ask for the passphrase when Kopete starts" ) );
	mAskPassphraseOnStartup->setWhatsThis ( i18n ( "Ask for the passphrase when Kopete starts."
			"This is recommended so that cryptography can be used seemlessly after startup." ) );

	mPreferencesDialog = new Ui::CryptographyPrefsUI;
	mPreferencesDialog->setupUi ( w );

	l->addWidget ( key );
	l->addWidget ( label );
	l->addWidget ( mAskPassphraseOnStartup );
	l->addWidget ( w );
	l->addStretch ();

	connect ( mAskPassphraseOnStartup, SIGNAL ( toggled ( bool ) ), this, SLOT ( slotAskOnStartupPressed ( bool ) ) );

	connect ( key->dialogButton(), SIGNAL ( clicked() ), this, SLOT ( changed() ) );
	connect ( key->eraseButton(), SIGNAL ( clicked() ), this, SLOT ( changed() ) );
	connect ( mAskPassphraseOnStartup, SIGNAL ( toggled ( bool ) ), this, SLOT ( changed() ) );
	connect ( mPreferencesDialog->onClose, SIGNAL ( toggled ( bool ) ), this, SLOT ( changed() ) );
	connect ( mPreferencesDialog->time, SIGNAL ( toggled ( bool ) ), this, SLOT ( changed() ) );
	connect ( mPreferencesDialog->never, SIGNAL ( toggled ( bool ) ), this, SLOT ( changed() ) );
	connect ( mPreferencesDialog->cacheTime, SIGNAL ( valueChanged ( int ) ), this, SLOT ( changed() ) );

	load();
}

CryptographyPreferences::~CryptographyPreferences()
{
	delete mPreferencesDialog;
}

void CryptographyPreferences::load()
{
	key->setFingerprint ( CryptographyConfig::self()->fingerprint() );
	mAskPassphraseOnStartup->setChecked ( CryptographyConfig::self()->askPassphraseOnStartup() );
	mPreferencesDialog->cacheTime->setValue ( CryptographyConfig::self()->cacheTime() );

	if ( CryptographyConfig::self()->cacheTime() == CryptographyConfig::Close )
		mPreferencesDialog->onClose->setChecked ( true );
	if ( CryptographyConfig::self()->cacheTime() == CryptographyConfig::Time )
		mPreferencesDialog->time->setChecked ( true );
	if ( CryptographyConfig::self()->cacheTime() == CryptographyConfig::Never )
		mPreferencesDialog->never->setChecked ( true );

	KCModule::load();
	emit changed ( false );
}

void CryptographyPreferences::save()
{
	CryptographyConfig::self()->setFingerprint ( key->fingerprint() );
	CryptographyConfig::self()->setAskPassphraseOnStartup ( mAskPassphraseOnStartup->isChecked() );
	CryptographyConfig::self()->setCacheTime ( mPreferencesDialog->cacheTime->value() );

	if ( mPreferencesDialog->onClose->isChecked() )
		CryptographyConfig::self()->setCacheMode ( CryptographyConfig::Close );
	if ( mPreferencesDialog->time->isChecked() )
		CryptographyConfig::self()->setCacheMode ( CryptographyConfig::Time );
	if ( mPreferencesDialog->never->isChecked() )
		CryptographyConfig::self()->setCacheMode ( CryptographyConfig::Never );

	CryptographyConfig::self()->save();

	KCModule::save();
	emit changed ( false );
}

void CryptographyPreferences::defaults()
{
	CryptographyConfig::self()->defaults();
	load();
	changed();
}

void CryptographyPreferences::slotAskOnStartupPressed ( bool b )
{
	mPreferencesDialog->never->setEnabled ( !b );
	if ( b )
		mPreferencesDialog->onClose->setChecked ( true );
}


// vim: set noet ts=4 sts=4 sw=4:
