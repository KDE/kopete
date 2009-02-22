/*
    webpresencepreferences.cpp

    Copyright (c) 2002      by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#include "webpresencepreferences.h"

#include <kgenericfactory.h>
#include <kurlrequester.h>

#include "ui_webpresenceprefs.h"
#include "webpresenceconfig.h"

K_PLUGIN_FACTORY(WebPresencePreferencesFactory, registerPlugin<WebPresencePreferences>();)
K_EXPORT_PLUGIN(WebPresencePreferencesFactory("kcm_kopete_webpresence"))

WebPresencePreferences::WebPresencePreferences(QWidget *parent, const QVariantList &args)
		: KCModule(WebPresencePreferencesFactory::componentData(), parent, args)
{
	QVBoxLayout* l = new QVBoxLayout( this );
	QWidget* w = new QWidget;
	m_preferencesDialog = new Ui::WebPresencePrefsUI;
	m_preferencesDialog->setupUi( w );
	l->addWidget( w );
	
	addConfig( WebPresenceConfig::self(), w );

	m_preferencesDialog->kcfg_uploadURL->setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
	m_preferencesDialog->kcfg_formatStylesheetURL->setFilter( "*.xsl" );
}

WebPresencePreferences::~WebPresencePreferences()
{
	delete m_preferencesDialog;
}

#include "webpresencepreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:
