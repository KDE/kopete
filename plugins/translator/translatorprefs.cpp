/*
    translatorprefs.cpp

    Kopete Translatorfish Translator plugin

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#include "translatorprefs.moc"
#include "translatorprefsbase.h"

#include <qlayout.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>
#include <kdebug.h>
#include <qlabel.h>
#include <qgroupbox.h>


#include <knuminput.h>

TranslatorPreferences::TranslatorPreferences(const QString &pixmap,QObject *parent)
							: ConfigModule(i18n("Translator"),i18n("Translator Plugin"),pixmap,parent)
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );

	m_lc = 0;

	preferencesDialog = new TranslatorPrefsUI(this);

	addLanguage( "en" , "English" );
    addLanguage( "es" , "Spanish" );
	addLanguage( "de" , "German" );

	KGlobal::config()->setGroup("Translator Plugin");

	preferencesDialog->m_LangBox->setCurrentItem( m_keyMap[KGlobal::config()->readEntry("myLang", "en")]);
}

TranslatorPreferences::~TranslatorPreferences()
{
}

const QString& TranslatorPreferences::myLang()
{
	return m_langMap[ preferencesDialog->m_LangBox->currentItem() ];
}

void TranslatorPreferences::addLanguage( const QString &key, const QString &name)
{
	m_langMap[m_lc] = key;
    m_keyMap[key] = m_lc;

	m_descMap[key] = name;

	preferencesDialog->m_LangBox->insertItem( name, m_lc);

	m_lc++;

}

void TranslatorPreferences::save()
{
	KConfig *config = KGlobal::config();
	config->setGroup("Translator Plugin");
	config->writeEntry("myLang", myLang() );
	config->sync();
	emit saved();

}

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

