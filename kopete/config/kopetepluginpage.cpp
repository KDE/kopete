/*
    kopetepluginpage.cpp - Kopete Plugin Loader KCM

    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2001-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetepluginpage.h"

#include <kgenericfactory.h>
#include <kplugininfo.h>
#include <kpluginselector.h>
#include <ktrader.h>

typedef KGenericFactory<KopetePluginConfig, QWidget> KopetePluginConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_pluginconfig, KopetePluginConfigFactory( "kcm_kopete_pluginconfig" ) );

KopetePluginConfig::KopetePluginConfig( QWidget *parent, const char * /* name */, const QStringList &args )
: KSettings::PluginPage( KopetePluginConfigFactory::instance(), parent, args )
{
	QValueList<KPluginInfo*> pluginInfo = KPluginInfo::fromServices( KTrader::self()->query( "Kopete/Protocol" ) );
	pluginSelector()->addPlugins( pluginInfo, i18n( "Protocols" ), "Protocols" );
	pluginSelector()->addPlugins( KGlobal::instance()->instanceName(), i18n( "General Plugins" ), "Plugins" );
}

// vim: set noet ts=4 sts=4 sw=4:

