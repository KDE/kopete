/*
    motionawaypreferences.cpp

    Kopete Motion Detector Auto-Away plugin

    Copyright (c) 2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>

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

#include "motionawaypreferences.h"
#include "motionawaypreferences.moc"

#include <qlayout.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>



MotionAwayPreferences::MotionAwayPreferences(const QString &pixmap,QObject *parent)
							: ConfigModule(i18n("Motion-Away"),i18n("Motion Plugin"),pixmap,parent)
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	preferencesDialog = new motionawayPrefsUI(this);

	KGlobal::config()->setGroup("MotionAway Plugin");
    preferencesDialog->m_videoDevice->setText(KGlobal::config()->readEntry("Device", "/dev/video0"));
	preferencesDialog->mAwayTimeout->setValue(KGlobal::config()->readNumEntry("Timeout", 1));
	preferencesDialog->mGoAvailable->setChecked(KGlobal::config()->readBoolEntry("GoAvailable", true));
}

MotionAwayPreferences::~MotionAwayPreferences()
{

}

void MotionAwayPreferences::save()
{
	KConfig *config=KGlobal::config();
	config->setGroup("MotionAway Plugin");
	config->writeEntry("Timeout", preferencesDialog->mAwayTimeout->value());
	config->writeEntry("GoAvailable", preferencesDialog->mGoAvailable->isChecked());
    config->writeEntry("Device", preferencesDialog->m_videoDevice->text());
	config->sync();
	emit saved();

}

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

