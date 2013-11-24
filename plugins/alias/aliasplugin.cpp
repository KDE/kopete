/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "aliasplugin.h"

#include <kgenericfactory.h>

#include "kopetechatsessionmanager.h"

K_PLUGIN_FACTORY( AliasPluginFactory, registerPlugin<AliasPlugin>(); )
K_EXPORT_PLUGIN( AliasPluginFactory( "kopete_alias" ) )

AliasPlugin * AliasPlugin::pluginStatic_ = 0L;

AliasPlugin::AliasPlugin( QObject *parent, const QVariantList & )
	: Kopete::Plugin( AliasPluginFactory::componentData(), parent )
{
	if( !pluginStatic_ )
		pluginStatic_ = this;

}

AliasPlugin::~AliasPlugin()
{
	pluginStatic_ = 0L;
}

AliasPlugin * AliasPlugin::plugin()
{
	return pluginStatic_ ;
}

#include "aliasplugin.moc"
