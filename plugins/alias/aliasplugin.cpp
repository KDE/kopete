/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kgenericfactory.h>

#include "kopetemessagemanagerfactory.h"

#include "aliasplugin.h"

typedef KGenericFactory<AliasPlugin> AliasPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_alias, AliasPluginFactory( "kopete_alias" )  )
AliasPlugin * AliasPlugin::pluginStatic_ = 0L;

AliasPlugin::AliasPlugin( QObject *parent, const char * name, const QStringList & )
	: Kopete::Plugin( AliasPluginFactory::instance(), parent, name )
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
