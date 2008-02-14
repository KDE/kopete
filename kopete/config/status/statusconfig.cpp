/*
    statusconfig.cpp - Kopete Status Config

    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "statusconfig.h"

#include "statusconfig_manager.h"
#include "statusconfig_general.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QTabWidget>

#include <kgenericfactory.h>
#include "kopetestatussettings.h"

K_PLUGIN_FACTORY( KopeteStatusConfigFactory, registerPlugin<StatusConfig>(); )
K_EXPORT_PLUGIN( KopeteStatusConfigFactory("kcm_kopete_statusconfig") )

StatusConfig::StatusConfig( QWidget *parent, const QVariantList &args )
: KCModule( KopeteStatusConfigFactory::componentData(), parent, args )
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	// since KSetting::Dialog has margins here, we don't need our own.
	layout->setContentsMargins( 0, 0, 0, 0);
	
	mStatusTabCtl = new QTabWidget(this);
	mStatusTabCtl->setObjectName("mStatusTabCtl");
	layout->addWidget( mStatusTabCtl );
	
	mPrfsManager = new StatusConfig_Manager( mStatusTabCtl );
	connect( mPrfsManager, SIGNAL(changed()), this, SLOT(changed()) );
	mStatusTabCtl->addTab( mPrfsManager, i18n("&Manager") );

	mPrfsGeneral = new StatusConfig_General( mStatusTabCtl );
	addConfig( Kopete::StatusSettings::self(), mPrfsGeneral );
	mStatusTabCtl->addTab( mPrfsGeneral, i18n("&General") );
}

void StatusConfig::load()
{
	KCModule::load();
	
}

void StatusConfig::save()
{
	KCModule::save();

	mPrfsManager->save();

	load();
}


#include "statusconfig.moc"
