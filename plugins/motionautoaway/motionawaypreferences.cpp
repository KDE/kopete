/*
    Kopete Motion Detector Auto-Away plugin

    Copyright (c) 2002-2004 by Duncan Mac-Vicar Prett <duncan@kde.org>
    
    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

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
//Added by qt3to4:
#include <QVBoxLayout>

#include <kgenericfactory.h>
#include <klineedit.h>
#include <knuminput.h>

#include "ui_motionawayprefs.h"
#include "motionawaypreferences.h"
#include "motionawayconfig.h"

K_PLUGIN_FACTORY(MotionAwayPreferencesFactory,
		 registerPlugin<MotionAwayPreferences>();
		)
K_EXPORT_PLUGIN(MotionAwayPreferencesFactory ( "kcm_kopete_motionaway" ))


MotionAwayPreferences::MotionAwayPreferences ( QWidget *parent, const QVariantList &args )
	: KCModule ( MotionAwayPreferencesFactory::componentData(), parent, args )
{
	// Add actuall widget generated from ui file.
	QVBoxLayout * l = new QVBoxLayout (this);
	QWidget * w = new QWidget (this);
	l->addWidget (w);
	preferencesDialog = new Ui::motionawayPrefsUI();
	preferencesDialog->setupUi (w);
	connect(preferencesDialog->BecomeAvailableWithActivity, SIGNAL(toggled(bool)), this, SLOT(changed()));
	connect(preferencesDialog->AwayTimeout, SIGNAL(valueChanged(int)), this, SLOT(changed()));
	connect(preferencesDialog->VideoDevice, SIGNAL(textChanged(const QString &)), this, SLOT(changed()));
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

void MotionAwayPreferences::save()
{
	MotionAwayConfig::self()->setAwayTimeout(preferencesDialog->AwayTimeout->value());
	MotionAwayConfig::self()->setBecomeAvailableWithActivity(preferencesDialog->BecomeAvailableWithActivity->isChecked());
	MotionAwayConfig::self()->setVideoDevice(preferencesDialog->VideoDevice->text());
	MotionAwayConfig::self()->writeConfig();
	emit KCModule::changed(false);
}

void MotionAwayPreferences::defaults()
{
	MotionAwayConfig::self()->setDefaults();
	load();
	changed();
}

#include "motionawaypreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:
