/***************************************************************************
                          motionawaypreferences.cpp  -  description
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

#include <kgenericfactory.h>
#include <kautoconfig.h>

#include "motionawayprefs.h"
#include "motionawaypreferences.h"

typedef KGenericFactory<MotionAwayPreferences> MotionAwayPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_motionaway, MotionAwayPreferencesFactory("kcm_kopete_motionaway"))

MotionAwayPreferences::MotionAwayPreferences(QWidget *parent, const char* /*name*/, const QStringList &args)
							: KCModule(MotionAwayPreferencesFactory::instance(), parent, args)
{
	// Add actuall widget generated from ui file.
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	preferencesDialog = new motionawayPrefsUI(this);
	
	// KAutoConfig stuff
	kautoconfig = new KAutoConfig(KGlobal::config(), this, "kautoconfig");
	connect(kautoconfig, SIGNAL(widgetModified()), SLOT(widgetModified()));
	connect(kautoconfig, SIGNAL(settingsChanged()), SLOT(widgetModified()));
    kautoconfig->addWidget(preferencesDialog, "MotionAway Plugin");
	kautoconfig->retrieveSettings(true);
}

void MotionAwayPreferences::widgetModified()
{
	setChanged(kautoconfig->hasChanged());
}

void MotionAwayPreferences::save()
{
	kautoconfig->saveSettings();
}

void MotionAwayPreferences::defaults ()
{
	kautoconfig->resetSettings();
}

#include "motionawaypreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:
