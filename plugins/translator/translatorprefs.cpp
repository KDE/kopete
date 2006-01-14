/*
    translatorprefs.cpp - Kopete Translator plugin

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart       <ogoffart @ kde.org>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#include <kgenericfactory.h>
#include "kcautoconfigmodule.h"
#include "translatorprefsbase.h"
#include "translatorlanguages.h"
#include <qcombobox.h>


class TranslatorPreferences;
typedef KGenericFactory<TranslatorPreferences> TranslatorConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_translator, TranslatorConfigFactory( "kcm_kopete_translator" ) )

class TranslatorPreferences : public KCAutoConfigModule
{
public:
	TranslatorPreferences( QWidget *parent = 0, const char * = 0, const QStringList &args = QStringList() ) : KCAutoConfigModule( TranslatorConfigFactory::instance(), parent, args )
	{
		TranslatorPrefsUI *preferencesDialog = new TranslatorPrefsUI(this);

		TranslatorLanguages languages;
		QMap<QString,QString>::ConstIterator i;
		QMap<QString,QString> m;

		m = languages.languagesMap();
		for ( i = m.begin(); i != m.end() ; ++i )
			preferencesDialog->myLang->insertItem( i.data(), languages.languageIndex(i.key()) );

		m = languages.servicesMap();
		for ( i = m.begin(); i != m.end() ; ++i )
			preferencesDialog->Service->insertItem( i.data(), languages.serviceIndex(i.key()) );

		setMainWidget( preferencesDialog , "Translator Plugin");
	}
};

