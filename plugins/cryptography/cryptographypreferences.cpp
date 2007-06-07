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
//#include "cryptographyconfig.h"
#include "cryptographypreferences.h"
#include "cryptographypreferences.moc"
#include "selectkeydialog.h"

typedef KGenericFactory<CryptographyPreferences> CryptographyPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_cryptography, CryptographyPreferencesFactory("kcm_kopete_cryptography"))

CryptographyPreferences::CryptographyPreferences(QWidget *parent, const QStringList &args)
							: KCModule(CryptographyPreferencesFactory::componentData(), parent, args)
{
	// Add actual widget generated from ui file.
    QVBoxLayout* l = new QVBoxLayout(this);
    QWidget *w = new QWidget;
	preferencesDialog = new Ui::CryptographyPrefsUI;
	preferencesDialog->setupUi(w);
    l->addWidget(w);
//    addConfig(CryptographyConfig::self(), w);

    if ( preferencesDialog->kcfg_DontAskForPassphrase->checkState() != Qt::Checked )
      preferencesDialog->kcfg_CacheBehavior->setEnabled( false );

	connect (preferencesDialog->selectKey, SIGNAL(pressed()), this, SLOT(slotSelectPressed()));
}

CryptographyPreferences::~CryptographyPreferences()
{
	delete preferencesDialog;
}

void CryptographyPreferences::load()
{
	KCModule::load();
}

void CryptographyPreferences::save()
{
	KCModule::save();
}

void CryptographyPreferences::defaults()
{
}

void CryptographyPreferences::slotSelectPressed()
{
	SelectKeyDialog opts(this, false);
	opts.exec();
	if (opts.result()==QDialog::Accepted)
		preferencesDialog->kcfg_PrivateKeyId->setText(opts.getkeyID());
}

// vim: set noet ts=4 sts=4 sw=4:
