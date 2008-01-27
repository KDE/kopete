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
#include <QVBoxLayout>

#include <klineedit.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>
#include <kleo/ui/keyrequester.h>

#include "cryptographypreferences.h"
#include "cryptographypreferences.moc"

#include "cryptographyconfig.h"

K_PLUGIN_FACTORY ( CryptographyPreferencesFactory, registerPlugin<CryptographyPreferences>(); )
K_EXPORT_PLUGIN ( CryptographyPreferencesFactory ( "kcm_kopete_cryptography" ) )


CryptographyPreferences::CryptographyPreferences ( QWidget *parent, const QVariantList &args )
		: KCModule ( CryptographyPreferencesFactory::componentData(), parent, args )
{
	setButtons ( Help | Apply | Default );
	
	// Add actual widget generated from ui file.
	QVBoxLayout* l = new QVBoxLayout ( this );
	QHBoxLayout* keyLayout = new QHBoxLayout ( 0 );

	QLabel * keyLabel = new QLabel ( i18n ( "Private Key: " ), this );

	key = new Kleo::EncryptionKeyRequester ( false/*multipleKeys*/, Kleo::EncryptionKeyRequester::OpenPGP, this, true/*onlyTrusted*/, true/*onlyValid*/ );
	key->setDialogMessage ( i18n ( "Select the key you want to use to sign and encrypt messages" ) );
	key->setDialogCaption ( i18n ( "Select the key you want to use to sign and encrypt messages" ) );
	key->setToolTip ( i18n ( "The private key used for decryption and signing" ) );
	key->setWhatsThis ( i18n ( "View and change the private key used for signing and encrypting messages using the Cryptography plugin" ) );

	QLabel * label = new QLabel ( i18n ( "Before you can sign messages or receive encrypted ones, you must select a private key for yourself.\n\nBefore you can send encrypted messages to someone, you must select their public key by right-clicking on their name in your contact list and choosing \"Select Public Key\"."), this );
	label->setWordWrap ( true );

	keyLabel->setBuddy( key );
	keyLayout->addWidget ( keyLabel );
	keyLayout->addWidget ( key );
	l->addLayout ( keyLayout );
	l->addWidget ( label );
	l->addStretch ();
	l->setSpacing ( 12 );


	connect ( key->dialogButton(), SIGNAL ( clicked() ), this, SLOT ( changed() ) );
	connect ( key->eraseButton(), SIGNAL ( clicked() ), this, SLOT ( changed() ) );

	load();
}

CryptographyPreferences::~CryptographyPreferences()
{
}

void CryptographyPreferences::load()
{
	key->setFingerprint ( CryptographyConfig::self()->fingerprint() );

	KCModule::load();
	emit changed ( false );
}

void CryptographyPreferences::save()
{
	CryptographyConfig::self()->setFingerprint ( key->fingerprint() );

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


// vim: set noet ts=4 sts=4 sw=4:
