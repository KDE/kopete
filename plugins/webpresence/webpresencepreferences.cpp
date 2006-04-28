/***************************************************************************
                          webpresencepreferences.cpp
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

#include <qlayout.h>
//Added by qt3to4:
#include <Q3VBoxLayout>

#include <kgenericfactory.h>
#include <kurlrequester.h>

#include "ui_webpresenceprefs.h"
#include "webpresencepreferences.h"

//TODO: Port to KConfigXT

typedef KGenericFactory<WebPresencePreferences> WebPresencePreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_webpresence, WebPresencePreferencesFactory("kcm_kopete_webpresence"))

WebPresencePreferences::WebPresencePreferences(QWidget *parent, const char* /*name*/, const QStringList &args)
		: KCModule(WebPresencePreferencesFactory::instance(), parent, args)
{
	QVBoxLayout* l = new QVBoxLayout( this );
	QWidget* w = new QWidget;
	preferencesDialog = new Ui::WebPresencePrefsUI;
	preferencesDialog->setupUi( w );
	l->addWidget( w );

	preferencesDialog->uploadURL->setMode( KFile::File );
	preferencesDialog->formatStylesheetURL->setFilter( "*.xsl" );

	// KAutoConfig stuff
// 	kautoconfig = new KAutoConfig(KGlobal::config(), this, "kautoconfig");
// 	connect(kautoconfig, SIGNAL(widgetModified()), SLOT(widgetModified()));
// 	connect(kautoconfig, SIGNAL(settingsChanged()), SLOT(widgetModified()));
// 	kautoconfig->addWidget(preferencesDialog, "Web Presence Plugin");
// 	kautoconfig->retrieveSettings(true);
}

WebPresencePreferences::~WebPresencePreferences()
{
	delete preferencesDialog;
}

void WebPresencePreferences::widgetModified()
{
	//emit KCModule::changed(kautoconfig->hasChanged());
}

void WebPresencePreferences::save()
{
// 	kautoconfig->saveSettings();
}

void WebPresencePreferences::defaults ()
{
// 	kautoconfig->resetSettings();
}

#include "webpresencepreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:
