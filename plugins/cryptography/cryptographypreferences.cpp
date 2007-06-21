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
#include "selectkeydialog.h"

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
	l->addWidget ( w );
	mConfig = new CryptographyConfig;

	connect ( mPreferencesDialog->selectKey, SIGNAL ( pressed() ), this, SLOT ( slotSelectPressed() ) );
	connect ( mPreferencesDialog->askPassPhrase, SIGNAL ( toggled ( bool ) ), this, SLOT ( slotAskPressed ( bool ) ) );

	connect ( mPreferencesDialog->privateKeyId, SIGNAL ( textChanged ( QString ) ), this, SLOT ( slotModified() ) );
	connect ( mPreferencesDialog->alsoMyKey, SIGNAL ( toggled ( bool ) ), this, SLOT ( slotModified() ) );
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

	mPreferencesDialog->privateKeyId->setText ( mConfig->privateKeyId() );
	mPreferencesDialog->alsoMyKey->setChecked ( mConfig->alsoMyKey() );
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
	mConfig->setPrivateKeyId ( mPreferencesDialog->privateKeyId->text() );
	mConfig->setAlsoMyKey ( mPreferencesDialog->alsoMyKey->isChecked() );
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
	mPreferencesDialog->privateKeyId->clear();
	mPreferencesDialog->alsoMyKey->setChecked(false);
	mPreferencesDialog->askPassPhrase->setChecked(false);
	mPreferencesDialog->onClose->setChecked(true);
	mPreferencesDialog->cacheTime->setValue(15);
	slotModified();
}

void CryptographyPreferences::slotSelectPressed()
{
	QString id;
	SelectKeyDialog opts ( id, this );
	opts.exec();
	if ( opts.result() ==QDialog::Accepted )
		mPreferencesDialog->privateKeyId->setText ( id );
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
