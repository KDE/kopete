/***************************************************************************
                          cryptographypreferences.cpp  -  description
                             -------------------
    begin                : jeu nov 14 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart@tiscalinet.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <klineedit.h>
#include <kconfig.h>
#include <kgenericfactory.h>

#include "cryptographyprefsbase.h"
#include "cryptographypreferences.h"
#include "cryptographyconfig.h"
#include "kgpgselkey.h"

typedef KGenericFactory<CryptographyPreferences> CryptographyPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_cryptography, CryptographyPreferencesFactory("kcm_kopete_cryptography"));

CryptographyPreferences::CryptographyPreferences(QWidget *parent, const char* /*name*/, const QStringList &args)
							: KCModule(CryptographyPreferencesFactory::instance(), parent, args)
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );

	preferencesDialog = new CryptographyPrefsUI(this);
	m_config = new CryptographyConfig();

	connect (preferencesDialog->m_selectOwnKey , SIGNAL(pressed()) , this , SLOT(slotSelectPressed()));

	load();

}

CryptographyPreferences::~CryptographyPreferences()
{
	save();
	delete preferencesDialog;
	delete m_config;
}


void CryptographyPreferences::load()
{
	m_config->load();
	preferencesDialog->m_editOwnKey->setText(m_config->privateKey());
	preferencesDialog->m_noPassphrase->setChecked(m_config->askPassPhrase());
	preferencesDialog->m_cache->setButton(m_config->cacheMode());
	preferencesDialog->m_alsoMyKey->setChecked(m_config->encrypt());
	preferencesDialog->m_time->setValue(m_config->cacheTime());
	setChanged(false);
}

void CryptographyPreferences::save()
{
	m_config->setCacheMode(preferencesDialog->m_cache->selectedId());
	m_config->setCacheTime(preferencesDialog->m_time->value());
	m_config->setEncrypt(preferencesDialog->m_alsoMyKey->isChecked());
	m_config->setAskPassPhrase(preferencesDialog->m_noPassphrase->isChecked());
	m_config->setPrivateKey(preferencesDialog->m_editOwnKey->text());
	m_config->save();
	setChanged(false);
}


void CryptographyPreferences::slotSelectPressed()
{

	KgpgSelKey *opts=new KgpgSelKey(this,0,false);
	opts->exec();
	if (opts->result()==true)
	{
		m_config->setPrivateKey(opts->getkeyID());
		m_signKeyMail=opts->getkeyMail();
		preferencesDialog->m_editOwnKey->setText(m_config->privateKey());

	}
	delete opts;
}

#include "cryptographypreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:
