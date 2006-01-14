/***************************************************************************
                          cryptographypreferences.cpp  -  description
                             -------------------
    begin                : jeu nov 14 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart @ kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qpushbutton.h>

#include <klineedit.h>
#include <kgenericfactory.h>

#include "cryptographyprefsbase.h"
#include "cryptographypreferences.h"
#include "kgpgselkey.h"

typedef KGenericFactory<CryptographyPreferences> CryptographyPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_cryptography, CryptographyPreferencesFactory("kcm_kopete_cryptography"))

CryptographyPreferences::CryptographyPreferences(QWidget *parent, const char* /*name*/, const QStringList &args)
							: KCAutoConfigModule(CryptographyPreferencesFactory::instance(), parent, args)
{
	// Add actuall widget generated from ui file.
	preferencesDialog = new CryptographyPrefsUI(this);
	connect (preferencesDialog->m_selectOwnKey , SIGNAL(pressed()) , this , SLOT(slotSelectPressed()));
	setMainWidget( preferencesDialog ,"Cryptography Plugin");
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
