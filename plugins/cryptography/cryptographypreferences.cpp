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
#include <qtimer.h>

#include <klocale.h>
#include <klineedit.h>
#include <kconfig.h>

#include "cryptographyprefsbase.h"
#include "cryptographypreferences.h"


#include "kgpgselkey.h"

CryptographyPreferences::CryptographyPreferences(const QString &pixmap,QObject *parent)
							: ConfigModule(i18n("Cryptography"),i18n("Cryptography Plugin"),pixmap,parent)
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );

	preferencesDialog = new CryptographyPrefsUI(this);

	connect (preferencesDialog->m_selectOwnKey , SIGNAL(pressed()) , this , SLOT(slotSelectPressed()));

	reopen();

}

CryptographyPreferences::~CryptographyPreferences()
{
}


const QString& CryptographyPreferences::privateKey()
{
	return m_signKeyID;
}



void CryptographyPreferences::reopen()
{
	KGlobal::config()->setGroup("Cryptography Plugin");

	m_signKeyID=KGlobal::config()->readEntry("PGP private key", QString::null);
	preferencesDialog->m_editOwnKey->setText(m_signKeyID);

	preferencesDialog->m_cache->setButton(KGlobal::config()->readNumEntry( "Cache Passphrase", Keep) );
	preferencesDialog->m_time->setValue(KGlobal::config()->readNumEntry( "Cache Time", 15) );
	preferencesDialog->m_alsoMyKey->setChecked( KGlobal::config()->readBoolEntry( "Also My Key", false ) );

}

void CryptographyPreferences::save()
{
	KConfig *config = KGlobal::config();
	config->setGroup("Cryptography Plugin");
	config->writeEntry("PGP private key", m_signKeyID );

	config->writeEntry("Cache Passphrase",  preferencesDialog->m_cache->id(preferencesDialog->m_cache->selected() ) );
	config->writeEntry("Cache Time", preferencesDialog->m_time->value() );
	config->writeEntry("Also My Key", preferencesDialog->m_alsoMyKey->isChecked());

	config->sync();

}


void CryptographyPreferences::slotSelectPressed()
{
	KgpgSelKey *opts=new KgpgSelKey(this,0,false);
	opts->exec();
	if (opts->result()==true)
	{
		m_signKeyID=opts->getkeyID();
		m_signKeyMail=opts->getkeyMail();
		preferencesDialog->m_editOwnKey->setText(m_signKeyID);
	}
	delete opts;
}

CryptographyPreferences::CacheMode CryptographyPreferences::cacheMode()
{
	return (CryptographyPreferences::CacheMode) preferencesDialog->m_cache->id(preferencesDialog->m_cache->selected() );
}

unsigned int CryptographyPreferences::cacheTime()
{
	return preferencesDialog->m_time->value();
}

bool CryptographyPreferences::alsoMyKey()
{
	return preferencesDialog->m_alsoMyKey->isChecked();
}

#include "cryptographypreferences.moc"
