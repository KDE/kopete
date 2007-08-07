/*
    kopetepluginconfig.cpp - Configure the Kopete plugins

    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2006      by Michaël Larouche      <larouche@kde.org>

    Kopete    (c) 2001-2006 by the Kopete developers <kopete-devel@kde.org>

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
#include <kgenericfactory.h>
#include <ksettings/dispatcher.h>
#include <KPluginInfo>

// Kopete includes
#include "kopetepluginmanager.h"

class KopetePluginConfigPrivate
{
public:
	KPluginSelector *pluginSelector;
};

typedef KGenericFactory<KopetePluginConfig, QWidget> KopetePluginConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_pluginconfig, KopetePluginConfigFactory( "kcm_kopete_pluginconfig" ) )

KopetePluginConfig::KopetePluginConfig( QWidget *parent, const QStringList &args )
: KCModule(KopetePluginConfigFactory::componentData(), parent, args), d(new KopetePluginConfigPrivate)
{
	d->pluginSelector = new KPluginSelector( this );
	
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->addWidget( d->pluginSelector );

	connect( d->pluginSelector, SIGNAL(changed(bool)), this, SLOT(changed()) );
	connect( d->pluginSelector, SIGNAL(configCommitted(const QByteArray&) ),
		this, SLOT(reparseConfiguration(const QByteArray&)) );

	d->pluginSelector->addPlugins( Kopete::PluginManager::self()->availablePlugins( "Plugins" ),
	                               KPluginSelector::ReadConfigFile, i18n( "General Plugins" ), "Plugins" );
	d->pluginSelector->load();
}

KopetePluginConfig::~KopetePluginConfig()
{
	delete d;
}

void KopetePluginConfig::reparseConfiguration(const QByteArray&conf)
{
	KSettings::Dispatcher::reparseConfiguration(conf);
}

void KopetePluginConfig::load()
{
	d->pluginSelector->load();
	
	KCModule::load();
}

void KopetePluginConfig::defaults()
{
	d->pluginSelector->defaults();
}

void KopetePluginConfig::save()
{
	d->pluginSelector->save();
	Kopete::PluginManager::self()->loadAllPlugins();

	KCModule::save();
}

#include "kopetepluginconfig.moc"

// vim: set noet ts=4 sts=4 sw=4:

