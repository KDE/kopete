/*
    translatorpreferences.cpp - Kopete Translator plugin

    Copyright (c) 2010      by Igor Poboiko <igor.poboiko@gmail.com>
    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart <ogoffart@kde.org>

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

#include "translatorpreferences.h"

#include <qlayout.h>
#include <qboxlayout.h>
#include <qwidget.h>

#include <kgenericfactory.h>
#include <kopetepluginmanager.h>

#include "translatorplugin.h"
#include "translatorconfig.h"
#include "ui_translatorprefsbase.h"

K_PLUGIN_FACTORY( TranslatorPreferencesFactory, registerPlugin<TranslatorPreferences>(); )
K_EXPORT_PLUGIN( TranslatorPreferencesFactory( "kcm_kopete_translator" ) )

TranslatorPreferences::TranslatorPreferences(QWidget *parent, const QVariantList &args)
	: KCModule(TranslatorPreferencesFactory::componentData(), parent, args)
{
	kDebug(14308) << "called.";

	QVBoxLayout* l = new QVBoxLayout( this );
	QWidget* w = new QWidget;

	p = new Ui::TranslatorPrefsUI;
	p->setupUi( w );
	l->addWidget( w );

	addConfig(TranslatorConfig::self(), w);

	m_languages = new TranslatorLanguages();
	updateLanguageList();

	QMap<QString,QString>::ConstIterator i;
	QMap<QString,QString> m;

	m = m_languages->servicesMap();
	for ( i = m.constBegin(); i != m.constEnd() ; ++i )
		p->service->insertItem( m_languages->serviceIndex(i.key()), i.value() );

	connect(p->defaultLanguage, SIGNAL(activated(int)),
		this, SLOT(slotModified()));
	connect(p->service, SIGNAL(activated(int)),
		this, SLOT(updateLanguageList()));

	if (Kopete::PluginManager::self()->plugin("kopete_translator"))
		connect( this, SIGNAL(preferencesChanged()),
			Kopete::PluginManager::self()->plugin("kopete_translator"), SLOT(loadSettings()) );

}

TranslatorPreferences::~TranslatorPreferences()
{
	kDebug(14308) << "called.";
	delete p;
}

void TranslatorPreferences::load()
{
	kDebug(14308) << "called.";
	KCModule::load();
	p->service->setCurrentIndex(m_languages->serviceIndex(TranslatorConfig::service()));
	updateLanguageList();
	p->defaultLanguage->setCurrentIndex(m_languages->languageIndex(TranslatorConfig::service(), TranslatorConfig::defaultLanguage()));
}

void TranslatorPreferences::save()
{
	kDebug(14308) << "called.";
	KCModule::save();
	TranslatorConfig::setService(m_languages->serviceKey(p->service->currentIndex()));
	TranslatorConfig::setDefaultLanguage(m_languages->languageKey(TranslatorConfig::service(), p->defaultLanguage->currentIndex()));
	TranslatorConfig::self()->writeConfig();
	emit preferencesChanged();
}

void TranslatorPreferences::slotModified()
{
	kDebug(14308) << "called.";
	emit KCModule::changed(true);
}

void TranslatorPreferences::updateLanguageList()
{
	kDebug(14308) << "called.";
	// Updating default language list
	QMap<QString,QString>::ConstIterator i;
	QMap<QString,QString> m;
	QString service = m_languages->serviceKey( p->service->currentIndex() );
	m = m_languages->languagesMap( service );
	p->defaultLanguage->clear();
	for ( i = m.constBegin(); i != m.constEnd() ; ++i )
		p->defaultLanguage->insertItem( m_languages->languageIndex(service, i.key()), i.value() );

	emit KCModule::changed(true);
}


void TranslatorPreferences::slotShowPreviousChanged(bool on)
{
	kDebug(14308) << "called.";
	Q_UNUSED(on);
	emit KCModule::changed(true);
}
