/*
    translatorprefs.cpp

    Kopete Translator plugin

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
#include "translatorplugin.h"
#include "translatorlanguages.h"

#include <qlayout.h>
#include <kautoconfig.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <kdebug.h>
#include <qbuttongroup.h>

typedef KGenericFactory<TranslatorPreferences> TranslatorPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_translator, TranslatorPreferencesFactory( "kcm_kopete_translator" )  );


TranslatorPreferences::TranslatorPreferences(QWidget* parent, const char* /*name*/, const QStringList &args)
							: KCModule( TranslatorPreferencesFactory::instance(), parent, args)
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );

	preferencesDialog = new TranslatorPrefsUI(this);
	languages = new TranslatorLanguages;

	QMap<QString,QString>::ConstIterator i;
	QMap<QString,QString> m;

	m = languages->languagesMap();
	for ( i = m.begin(); i != m.end() ; ++i )
		preferencesDialog->myLang->insertItem( i.data(), languages->languageIndex(i.key()) );

	m = languages->servicesMap();
	for ( i = m.begin(); i != m.end() ; ++i )
		preferencesDialog->Service->insertItem( i.data(), languages->serviceIndex(i.key()) );

	//KAutoConfig Stuff
	kautoconfig = new KAutoConfig(KGlobal::config(), this, "kautoconfig");
	connect(kautoconfig, SIGNAL(widgetModified()), SLOT(widgetModified()));
	connect(kautoconfig, SIGNAL(settingsChanged()), SLOT(widgetModified()));
	kautoconfig->addWidget(preferencesDialog, "Cryptography Plugin");
	kautoconfig->retrieveSettings(true);

}

TranslatorPreferences::~TranslatorPreferences()
{
	delete kautoconfig;
}

void TranslatorPreferences::widgetModified()
{
	setChanged(kautoconfig->hasChanged());
}

void TranslatorPreferences::defaults()
{
	kautoconfig->resetSettings();
}

void TranslatorPreferences::save()
{
	kautoconfig->saveSettings();
}

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

