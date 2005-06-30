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

#include <kgenericfactory.h>
#include <kautoconfig.h>
#include <kurlrequester.h>

#include "webpresenceprefs.h"
#include "webpresencepreferences.h"

typedef KGenericFactory<WebPresencePreferences> WebPresencePreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_webpresence, WebPresencePreferencesFactory("kcm_kopete_webpresence"))

WebPresencePreferences::WebPresencePreferences(QWidget *parent, const char* /*name*/, const QStringList &args)
		: KCModule(WebPresencePreferencesFactory::instance(), parent, args)
{
	// Add actuall widget generated from ui file.
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	preferencesDialog = new WebPresencePrefsUI(this);
	preferencesDialog->uploadURL->setMode( KFile::File );
	preferencesDialog->formatStylesheetURL->setFilter( "*.xsl" );

	// KAutoConfig stuff
	kautoconfig = new KAutoConfig(KGlobal::config(), this, "kautoconfig");
	connect(kautoconfig, SIGNAL(widgetModified()), SLOT(widgetModified()));
	connect(kautoconfig, SIGNAL(settingsChanged()), SLOT(widgetModified()));
	kautoconfig->addWidget(preferencesDialog, "Web Presence Plugin");
	kautoconfig->retrieveSettings(true);
}

void WebPresencePreferences::widgetModified()
{
	emit KCModule::changed(kautoconfig->hasChanged());
}

void WebPresencePreferences::save()
{
	kautoconfig->saveSettings();
}

void WebPresencePreferences::defaults ()
{
	kautoconfig->resetSettings();
}

#include "webpresencepreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:
