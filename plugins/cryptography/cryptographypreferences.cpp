/*
    cryptographypreferences.cpp  -  description

    Copyright (c) 2002      by Olivier Goffart <ogoffart@kde.org>

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
#include "kgpgselkey.h"

// TODO: Port to KConfigXT

typedef KGenericFactory<CryptographyPreferences> CryptographyPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_cryptography, CryptographyPreferencesFactory("kcm_kopete_cryptography"))

CryptographyPreferences::CryptographyPreferences(QWidget *parent, const QStringList &args)
							: KCModule(CryptographyPreferencesFactory::componentData(), parent, args)
{
	// Add actuall widget generated from ui file.
	QWidget* w = new QWidget(this);
	preferencesDialog = new Ui::CryptographyPrefsUI;
	preferencesDialog->setupUi(this);

	connect (preferencesDialog->m_selectOwnKey , SIGNAL(pressed()) , this , SLOT(slotSelectPressed()));
	//setMainWidget( preferencesDialog ,"Cryptography Plugin");
}

CryptographyPreferences::~CryptographyPreferences()
{
	delete preferencesDialog;
}

void CryptographyPreferences::slotSelectPressed()
{
	KgpgSelKey opts(this,0,false);
	opts.exec();
	if (opts.result()==QDialog::Accepted)
		preferencesDialog->PGP_private_key->setText(opts.getkeyID());
}

#include "cryptographypreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:
