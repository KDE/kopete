/*
    msnpreferences.cpp - MSN Preferences Widget

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart        <ogoffart@tiscalinet.be>

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

#include "msnpreferences.h"

#include <qcheckbox.h>
#include <qlayout.h>

#include <kconfig.h>
#include <kautoconfig.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <kcmodule.h>

#include "msnprefs.h"

typedef KGenericFactory<MSNPreferences> MSNProtocolConfigFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_kopete_msn, MSNProtocolConfigFactory( "kcm_kopete_msn" ) )

MSNPreferences::MSNPreferences( QWidget *parent, const char * /* name */, const QStringList &args )
: KCModule( MSNProtocolConfigFactory::instance(), parent, args )
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );

	kautoconfig = new KAutoConfig(KGlobal::config(), this, "kautoconfig");
	connect(kautoconfig, SIGNAL(widgetModified()), SLOT(slotSettingsDirty()));
	connect(kautoconfig, SIGNAL(settingsChanged()), SLOT(slotSettingsDirty()));
    kautoconfig->addWidget( new msnPrefsUI( this ) , "MSN");

	load();
}

void MSNPreferences::load()
{
	kautoconfig->retrieveSettings(true);
}

void MSNPreferences::save()
{
	kautoconfig->saveSettings();
	setChanged( false );
}

void MSNPreferences::defaults ()
{
	kautoconfig->resetSettings();
}


void MSNPreferences::slotSettingsDirty()
{
	// Just mark settings dirty, even if the user undoes his changes,
	// because KPrefs will handle it in the near future.
	setChanged( kautoconfig->hasChanged() );
}

#include "msnpreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:

