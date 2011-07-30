/*
    kopetepluginconfig.cpp - Configure the Kopete plugins

    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2006      by Michaël Larouche      <larouche@kde.org>
    Copyright (c) 2007      by Will Stephenson       <wstephenson@kde.org>

    Kopete    (c) 2001-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetepluginconfig.h"

// Qt includes
#include <QtCore/QByteArray>
#include <QtGui/QVBoxLayout>

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kpluginselector.h>
#include <kpluginfactory.h>
#include <ksettings/dispatcher.h>
#include <KPluginInfo>

// Kopete includes
#include "kopetepluginmanager.h"

K_PLUGIN_FACTORY( KopetePluginConfigFactory,
		registerPlugin<KopetePluginConfig>(); )
K_EXPORT_PLUGIN( KopetePluginConfigFactory("kcm_kopete_pluginconfig") )

KopetePluginConfig::KopetePluginConfig( QWidget *parent, const QVariantList &args )
: KCModule(KopetePluginConfigFactory::componentData(), parent, args)
{
	m_pluginSelector = new KPluginSelector( this );
	
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->addWidget( m_pluginSelector );

	connect( m_pluginSelector, SIGNAL(changed(bool)), this, SLOT(changed()) );
	connect( m_pluginSelector, SIGNAL(configCommitted(QByteArray)),
		this, SLOT(reparseConfiguration(QByteArray)) );

	m_pluginSelector->addPlugins( Kopete::PluginManager::self()->availablePlugins( "Plugins" ),
	                               KPluginSelector::ReadConfigFile, i18n( "General Plugins" ), "Plugins" );
	m_pluginSelector->load();
}

KopetePluginConfig::~KopetePluginConfig()
{
}

void KopetePluginConfig::reparseConfiguration(const QByteArray&conf)
{
	KSettings::Dispatcher::reparseConfiguration(conf);
}

void KopetePluginConfig::load()
{
	m_pluginSelector->load();
	
	KCModule::load();
}

void KopetePluginConfig::defaults()
{
	m_pluginSelector->defaults();
}

void KopetePluginConfig::save()
{
	m_pluginSelector->save();
	Kopete::PluginManager::self()->loadAllPlugins();

	KCModule::save();
}

#include "kopetepluginconfig.moc"

// vim: set noet ts=4 sts=4 sw=4:

