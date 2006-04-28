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
#include <kcmodule.h>

#include "ui_translatorprefsbase.h"
#include "translatorlanguages.h"

#include <q3combobox.h>

// TODO: Port to KConfigXT
class TranslatorPreferences;
typedef KGenericFactory<TranslatorPreferences> TranslatorConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_translator, TranslatorConfigFactory( "kcm_kopete_translator" ) )

class TranslatorPreferences : public KCModule
{
public:
	TranslatorPreferences( QWidget *parent = 0, const char * = 0, const QStringList &args = QStringList() ) : KCModule( TranslatorConfigFactory::instance(), parent, args )
	{
		QVBoxLayout* l = new QVBoxLayout( this );
		QWidget* w = new QWidget;
		preferencesDialog = new Ui::TranslatorPrefsUI;
		preferencesDialog->setupUi( w );
		l->addWidget( w );

		TranslatorLanguages languages;
		QMap<QString,QString>::ConstIterator i;
		QMap<QString,QString> m;

		m = languages.languagesMap();
		for ( i = m.begin(); i != m.end() ; ++i )
			preferencesDialog->myLang->insertItem( i.value(), languages.languageIndex(i.key()) );

		m = languages.servicesMap();
		for ( i = m.begin(); i != m.end() ; ++i )
			preferencesDialog->Service->insertItem( i.value(), languages.serviceIndex(i.key()) );

		//setMainWidget( w );
	}

	~TranslatorPreferences()
	{
		delete preferencesDialog;
	}

private:
	Ui::TranslatorPrefsUI *preferencesDialog;
};

