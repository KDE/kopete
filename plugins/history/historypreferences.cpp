/*
    historypreferences.cpp

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qlayout.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kautoconfig.h>
#include <kglobal.h>
#include <kconfig.h>
#include <knuminput.h>
#include <kcolorbutton.h>
#include <kgenericfactory.h>

#include "historyprefsui.h"
#include "historypreferences.h"


typedef KGenericFactory<HistoryPreferences> HistoryConfigFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_kopete_history, HistoryConfigFactory( "kcm_kopete_history" ) )

HistoryPreferences::HistoryPreferences( QWidget *parent, const char * /* name */, const QStringList &args )
: KCModule( HistoryConfigFactory::instance(), parent, args )
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );

	kautoconfig = new KAutoConfig(KGlobal::config(), this, "kautoconfig");
	connect(kautoconfig, SIGNAL(widgetModified()), SLOT(slotSettingsChanged()));
	connect(kautoconfig, SIGNAL(settingsChanged()), SLOT(slotSettingsChanged()));
    kautoconfig->addWidget( new HistoryPrefsUI(this) , "History Plugin");

	load();
}

/*HistoryPreferences::~HistoryPreferences()
{
}*/


void HistoryPreferences::load()
{
	kautoconfig->retrieveSettings(true);
}

void HistoryPreferences::save()
{
	KConfig *config = KGlobal::config();
	config->setGroup("History Plugin");
	config->writeEntry("Version",  "0.8" );
	kautoconfig->saveSettings();
}

void HistoryPreferences::defaults ()
{
	kautoconfig->resetSettings();
}


void HistoryPreferences::slotSettingsChanged()
{
	// Just mark settings dirty, even if the user undoes his changes,
	// because KPrefs will handle it in the near future.
	setChanged( kautoconfig->hasChanged() );
}

#include "historypreferences.moc"
