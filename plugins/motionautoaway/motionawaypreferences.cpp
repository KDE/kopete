/*
    Kopete Motion Detector Auto-Away plugin

    Copyright (c) 2002-2004 by Duncan Mac-Vicar Prett <duncan@kde.org>
    
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

#include <qlayout.h>
#include <qobject.h>
#include <qcheckbox.h>

#include <kgenericfactory.h>
#include <klineedit.h>
#include <knuminput.h>

#include "motionawayprefs.h"
#include "motionawaypreferences.h"
#include "motionawayconfig.h"

typedef KGenericFactory<MotionAwayPreferences> MotionAwayPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_motionaway, MotionAwayPreferencesFactory("kcm_kopete_motionaway"))

MotionAwayPreferences::MotionAwayPreferences(QWidget *parent, const char* /*name*/, const QStringList &args)
							: KCModule(MotionAwayPreferencesFactory::instance(), parent, args)
{
	// Add actuall widget generated from ui file.
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	preferencesDialog = new motionawayPrefsUI(this);
	connect(preferencesDialog->BecomeAvailableWithActivity, SIGNAL(toggled(bool)), this, SLOT(slotWidgetModified()));
	connect(preferencesDialog->AwayTimeout, SIGNAL(valueChanged(int)), this, SLOT(slotWidgetModified()));
	connect(preferencesDialog->VideoDevice, SIGNAL(textChanged(const QString &)), this, SLOT(slotWidgetModified()));
	load();
}

void MotionAwayPreferences::load()
{
	MotionAwayConfig::self()->readConfig();
	preferencesDialog->AwayTimeout->setValue(MotionAwayConfig::self()->awayTimeout());
	preferencesDialog->BecomeAvailableWithActivity->setChecked(MotionAwayConfig::self()->becomeAvailableWithActivity());
	preferencesDialog->VideoDevice->setText(MotionAwayConfig::self()->videoDevice());
	emit KCModule::changed(false);
}

void MotionAwayPreferences::slotWidgetModified()
{
	emit KCModule::changed(true);
}

void MotionAwayPreferences::save()
{
	MotionAwayConfig::self()->setAwayTimeout(preferencesDialog->AwayTimeout->value());
	MotionAwayConfig::self()->setBecomeAvailableWithActivity(preferencesDialog->BecomeAvailableWithActivity->isChecked());
	MotionAwayConfig::self()->setVideoDevice(preferencesDialog->VideoDevice->text());
	MotionAwayConfig::self()->writeConfig();
	emit KCModule::changed(false);
}

#include "motionawaypreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:
