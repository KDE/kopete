/*
    kopetepluginmanager.cpp - Kopete Plugin Loader

    Copyright (c) 2002-2003 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart  <ogoffart @tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "config.h"

#include "kopetepluginmanager.h"

#if defined(HAVE_VALGRIND_H) && !defined(NDEBUG) && defined(__i386__)
// We don't want the per-skin includes, so pretend we have a skin header already
#define __VALGRIND_SOMESKIN_H
#include <valgrind/valgrind.h>
#endif

#include <qapplication.h>
#include <qfile.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qvaluestack.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kparts/componentfactory.h>
#include <kplugininfo.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>
#include <kurl.h>

#include "kopeteplugin.h"
#include "kopetecontactlist.h"
#include "kopeteaccountmanager.h"

namespace Kopete
{


class PluginManager::Private
{
public:
	Private() : shutdownMode( StartingUp ), isAllPluginsLoaded(false) {}

	// All available plugins, regardless of category, and loaded or not
	QValueList<KPluginInfo *> plugins;

	// Dict of all currently loaded plugins, mapping the KPluginInfo to
	// a plugin
	typedef QMap<KPluginInfo *, Plugin *> InfoToPluginMap;
	InfoToPluginMap loadedPlugins;

	// The plugin manager's mode. The mode is StartingUp until loadAllPlugins()
	// has finished loading the plugins, after which it is set to Running.
	// ShuttingDown and DoneShutdown are used during Kopete shutdown by the
	// async unloading of plugins.
	enum ShutdownMode { StartingUp, Running, ShuttingDown, DoneShutdown };
	ShutdownMode shutdownMode;

	// Plugins pending for loading
	QValueStack<QString> pluginsToLoad;

	static KStaticDeleter<PluginManager> deleter;

	bool isAllPluginsLoaded;
};

KStaticDeleter<PluginManager> PluginManager::Private::deleter;
PluginManager* PluginManager::s_self = 0L;

PluginManager* PluginManager::self()
{
	if ( !s_self )
		Private::deleter.setObject( s_self, new PluginManager() );

	return s_self;
}

PluginManager::PluginManager() : QObject( qApp ), d( new Private )
{
	d->plugins = KPluginInfo::fromServices( KTrader::self()->query( QString::fromLatin1( "Kopete/Plugin" ),
		QString::fromLatin1( "[X-Kopete-Version] == 1000900" ) ) );

	// We want to add a reference to the application's event loop so we
	// can remain in control when all windows are removed.
	// This way we can unload plugins asynchronously, which is more
	// robust if they are still doing processing.
	kapp->ref();
}

PluginManager::~PluginManager()
{
	if ( d->shutdownMode != Private::DoneShutdown )
		kdWarning( 14010 ) << k_funcinfo << "Destructing plugin manager without going through the shutdown process! Backtrace is: " << endl << kdBacktrace() << endl;

	// Quick cleanup of the remaining plugins, hope it helps
	// Note that deleting it.data() causes slotPluginDestroyed to be called, which
	// removes the plugin from the list of loaded plugins.
	while ( !d->loadedPlugins.empty() )
	{
		Private::InfoToPluginMap::ConstIterator it = d->loadedPlugins.begin();
		kdWarning( 14010 ) << k_funcinfo << "Deleting stale plugin '" << it.data()->name() << "'" << endl;
		delete it.data();
	}

	delete d;
}

QValueList<KPluginInfo *> PluginManager::availablePlugins( const QString &category ) const
{
	if ( category.isEmpty() )
		return d->plugins;

	QValueList<KPluginInfo *> result;
	QValueList<KPluginInfo *>::ConstIterator it;
	for ( it = d->plugins.begin(); it != d->plugins.end(); ++it )
	{
		if ( ( *it )->category() == category )
			result.append( *it );
	}

	return result;
}

PluginList PluginManager::loadedPlugins( const QString &category ) const
{
	PluginList result;
	
	for ( Private::InfoToPluginMap::ConstIterator it = d->loadedPlugins.begin();
	      it != d->loadedPlugins.end(); ++it )
	{
		if ( category.isEmpty() || it.key()->category() == category )
			result.append( it.data() );
	}
	
	return result;
}


KPluginInfo *PluginManager::pluginInfo( const Plugin *plugin ) const
{
	for ( Private::InfoToPluginMap::ConstIterator it = d->loadedPlugins.begin();
	      it != d->loadedPlugins.end(); ++it )
	{
		if ( it.data() == plugin )
			return it.key();
	}
	return 0;
}

void PluginManager::shutdown()
{
	if(d->shutdownMode != Private::Running)
	{
		kdDebug( 14010 ) << k_funcinfo << "called when not running.  / state = " << d->shutdownMode << endl;
		return;
	}

	d->shutdownMode = Private::ShuttingDown;
	

	/* save the contact list now, just in case a change was made very recently
	   and it hasn't autosaved yet
	   from a OO point of view, theses lines should not be there, but i don't
	   see better place -Olivier
	*/
	Kopete::ContactList::self()->save();
	Kopete::AccountManager::self()->save();
	
	// Remove any pending plugins to load, we're shutting down now :)
	d->pluginsToLoad.clear();
	
	// Ask all plugins to unload
	for ( Private::InfoToPluginMap::ConstIterator it = d->loadedPlugins.begin();
	      it != d->loadedPlugins.end(); /* EMPTY */ )
	{
		// Plugins could emit their ready for unload signal directly in response to this,
		// which would invalidate the current iterator. Therefore, we copy the iterator
		// and increment it beforehand.
		Private::InfoToPluginMap::ConstIterator current( it );
		++it;
		// FIXME: a much cleaner approach would be to just delete the plugin now. if it needs
		//  to do some async processing, it can grab a reference to the app itself and create
		//  another object to do it.
		current.data()->aboutToUnload();
	}
	
	// When running under valgrind, don't enable the timer because it will almost
	// certainly fire due to valgrind's much slower processing
#if defined(HAVE_VALGRIND_H) && !defined(NDEBUG) && defined(__i386__)
	if ( RUNNING_ON_VALGRIND )
		kdDebug(14010) << k_funcinfo << "Running under valgrind, disabling plugin unload timeout guard" << endl;
	else
#endif
		QTimer::singleShot( 3000, this, SLOT( slotShutdownTimeout() ) );
}

void PluginManager::slotPluginReadyForUnload()
{
	// Using QObject::sender() is on purpose here, because otherwise all
	// plugins would have to pass 'this' as parameter, which makes the API
	// less clean for plugin authors
	// FIXME: I don't buy the above argument. Add a Kopete::Plugin::emitReadyForUnload(void),
	//        and make readyForUnload be passed a plugin. - Richard
	Plugin *plugin = dynamic_cast<Plugin *>( const_cast<QObject *>( sender() ) );
	kdDebug( 14010 ) << k_funcinfo << plugin->pluginId() << "ready for unload" << endl;
	if ( !plugin )
	{
		kdWarning( 14010 ) << k_funcinfo << "Calling object is not a plugin!" << endl;
		return;
	}
	
	plugin->deleteLater();
}


void PluginManager::slotShutdownTimeout()
{
	// When we were already done the timer might still fire.
	// Do nothing in that case.
	if ( d->shutdownMode == Private::DoneShutdown )
		return;

	QStringList remaining;
	for ( Private::InfoToPluginMap::ConstIterator it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
		remaining.append( it.data()->pluginId() );

	kdWarning( 14010 ) << k_funcinfo << "Some plugins didn't shutdown in time!" << endl
		<< "Remaining plugins: " << remaining.join( QString::fromLatin1( ", " ) ) << endl
		<< "Forcing Kopete shutdown now." << endl;

	slotShutdownDone();
}

void PluginManager::slotShutdownDone()
{
	kdDebug( 14010 ) << k_funcinfo << endl;

	d->shutdownMode = Private::DoneShutdown;

	kapp->deref();
}

void PluginManager::loadAllPlugins()
{
	// FIXME: We need session management here - Martijn

	KConfig *config = KGlobal::config();
	if ( config->hasGroup( QString::fromLatin1( "Plugins" ) ) )
	{
		QMap<QString, bool> pluginsMap;

		QMap<QString, QString> entries = config->entryMap( QString::fromLatin1( "Plugins" ) );
		QMap<QString, QString>::Iterator it;
		for ( it = entries.begin(); it != entries.end(); ++it )
		{
			QString key = it.key();
			if ( key.endsWith( QString::fromLatin1( "Enabled" ) ) )
				pluginsMap.insert( key.left( key.length() - 7 ), (it.data() == QString::fromLatin1( "true" )) );
		}

		QValueList<KPluginInfo *> plugins = availablePlugins( QString::null );
		QValueList<KPluginInfo *>::ConstIterator it2 = plugins.begin();
		QValueList<KPluginInfo *>::ConstIterator end = plugins.end();
		for ( ; it2 != end; ++it2 )
		{
			// Protocols are loaded automatically so they aren't always in Plugins group. (fixes bug 167113)
			if ( (*it2)->category() == QString::fromLatin1( "Protocols" ) )
				continue;

			QString pluginName = (*it2)->pluginName();
			bool inMap = pluginsMap.contains( pluginName );
			if ( (inMap && pluginsMap[pluginName]) || (!inMap && (*it2)->isPluginEnabledByDefault()) )
			{
				if ( !plugin( pluginName ) )
					d->pluginsToLoad.push( pluginName );
			}
			else
			{
				//This happens if the user unloaded plugins with the config plugin page.
				// No real need to be assync because the user usualy unload few plugins
				// compared tto the number of plugin to load in a cold start. - Olivier
				if ( plugin( pluginName ) )
					unloadPlugin( pluginName );
			}
		}
	}
	else
	{
		// we had no config, so we load any plugins that should be loaded by default.
		QValueList<KPluginInfo *> plugins = availablePlugins( QString::null );
		QValueList<KPluginInfo *>::ConstIterator it = plugins.begin();
		QValueList<KPluginInfo *>::ConstIterator end = plugins.end();
		for ( ; it != end; ++it )
		{
			if ( (*it)->isPluginEnabledByDefault() )
				d->pluginsToLoad.push( (*it)->pluginName() );
		}
	}
	// Schedule the plugins to load
	QTimer::singleShot( 0, this, SLOT( slotLoadNextPlugin() ) );
}

void PluginManager::slotLoadNextPlugin()
{
	if ( d->pluginsToLoad.isEmpty() )
	{
		if ( d->shutdownMode == Private::StartingUp )
		{
			d->shutdownMode = Private::Running;
			d->isAllPluginsLoaded = true;
			emit allPluginsLoaded();
		}
		return;
	}

	QString key = d->pluginsToLoad.pop();
	loadPluginInternal( key );

	// Schedule the next run unconditionally to avoid code duplication on the
	// allPluginsLoaded() signal's handling. This has the added benefit that
	// the signal is delayed one event loop, so the accounts are more likely
	// to be instantiated.
	QTimer::singleShot( 0, this, SLOT( slotLoadNextPlugin() ) );
}

Plugin * PluginManager::loadPlugin( const QString &_pluginId, PluginLoadMode mode /* = LoadSync */ )
{
	QString pluginId = _pluginId;

	// Try to find legacy code
	// FIXME: Find any cases causing this, remove them, and remove this too - Richard
	if ( pluginId.endsWith( QString::fromLatin1( ".desktop" ) ) )
	{
		kdWarning( 14010 ) << "Trying to use old-style API!" << endl << kdBacktrace() << endl;
		pluginId = pluginId.remove( QRegExp( QString::fromLatin1( ".desktop$" ) ) );
	}

	if ( mode == LoadSync )
	{
		return loadPluginInternal( pluginId );
	}
	else
	{
		d->pluginsToLoad.push( pluginId );
		QTimer::singleShot( 0, this, SLOT( slotLoadNextPlugin() ) );
		return 0L;
	}
}

Plugin *PluginManager::loadPluginInternal( const QString &pluginId )
{
	//kdDebug( 14010 ) << k_funcinfo << pluginId << endl;

	KPluginInfo *info = infoForPluginId( pluginId );
	if ( !info )
	{
		kdWarning( 14010 ) << k_funcinfo << "Unable to find a plugin named '" << pluginId << "'!" << endl;
		return 0L;
	}

	if ( d->loadedPlugins.contains( info ) )
		return d->loadedPlugins[ info ];

	int error = 0;
	Plugin *plugin = KParts::ComponentFactory::createInstanceFromQuery<Plugin>( QString::fromLatin1( "Kopete/Plugin" ),
		QString::fromLatin1( "[X-KDE-PluginInfo-Name]=='%1'" ).arg( pluginId ), this, 0, QStringList(), &error );

	if ( plugin )
	{
		d->loadedPlugins.insert( info, plugin );
		info->setPluginEnabled( true );

		connect( plugin, SIGNAL( destroyed( QObject * ) ), this, SLOT( slotPluginDestroyed( QObject * ) ) );
		connect( plugin, SIGNAL( readyForUnload() ), this, SLOT( slotPluginReadyForUnload() ) );

		kdDebug( 14010 ) << k_funcinfo << "Successfully loaded plugin '" << pluginId << "'" << endl;

		emit pluginLoaded( plugin );
	}
	else
	{
		switch( error )
		{
		case KParts::ComponentFactory::ErrNoServiceFound:
			kdDebug( 14010 ) << k_funcinfo << "No service implementing the given mimetype "
				<< "and fullfilling the given constraint expression can be found." << endl;
			break;

		case KParts::ComponentFactory::ErrServiceProvidesNoLibrary:
			kdDebug( 14010 ) << "the specified service provides no shared library." << endl;
			break;

		case KParts::ComponentFactory::ErrNoLibrary:
			kdDebug( 14010 ) << "the specified library could not be loaded." << endl;
			break;

		case KParts::ComponentFactory::ErrNoFactory:
			kdDebug( 14010 ) << "the library does not export a factory for creating components." << endl;
			break;

		case KParts::ComponentFactory::ErrNoComponent:
			kdDebug( 14010 ) << "the factory does not support creating components of the specified type." << endl;
			break;
		}

		kdDebug( 14010 ) << k_funcinfo << "Loading plugin '" << pluginId << "' failed, KLibLoader reported error: '" << endl <<
			KLibLoader::self()->lastErrorMessage() << "'" << endl;
	}

	return plugin;
}

bool PluginManager::unloadPlugin( const QString &spec )
{
	//kdDebug(14010) << k_funcinfo << spec << endl;
	if( Plugin *thePlugin = plugin( spec ) )
	{
		thePlugin->aboutToUnload();
		return true;
	}
	else
		return false;
}



void PluginManager::slotPluginDestroyed( QObject *plugin )
{
	for ( Private::InfoToPluginMap::Iterator it = d->loadedPlugins.begin();
	      it != d->loadedPlugins.end(); ++it )
	{
		if ( it.data() == plugin )
		{
			d->loadedPlugins.erase( it );
			break;
		}
	}

	if ( d->shutdownMode == Private::ShuttingDown && d->loadedPlugins.isEmpty() )
	{
		// Use a timer to make sure any pending deleteLater() calls have
		// been handled first
		QTimer::singleShot( 0, this, SLOT( slotShutdownDone() ) );
	}
}




Plugin* PluginManager::plugin( const QString &_pluginId ) const
{
	// Hack for compatibility with Plugin::pluginId(), which returns
	// classname() instead of the internal name. Changing that is not easy
	// as it invalidates the config file, the contact list, and most likely
	// other code as well.
	// For now, just transform FooProtocol to kopete_foo.
	// FIXME: In the future we'll need to change this nevertheless to unify
	//        the handling - Martijn
	QString pluginId = _pluginId;
	if ( pluginId.endsWith( QString::fromLatin1( "Protocol" ) ) )
		pluginId = QString::fromLatin1( "kopete_" ) + _pluginId.lower().remove( QString::fromLatin1( "protocol" ) );
	// End hack

	KPluginInfo *info = infoForPluginId( pluginId );
	if ( !info )
		return 0L;

	if ( d->loadedPlugins.contains( info ) )
		return d->loadedPlugins[ info ];
	else
		return 0L;
}

KPluginInfo * PluginManager::infoForPluginId( const QString &pluginId ) const
{
	QValueList<KPluginInfo *>::ConstIterator it;
	for ( it = d->plugins.begin(); it != d->plugins.end(); ++it )
	{
		if ( ( *it )->pluginName() == pluginId )
			return *it;
	}

	return 0L;
}


bool PluginManager::setPluginEnabled( const QString &_pluginId, bool enabled /* = true */ )
{
	QString pluginId = _pluginId;

	KConfig *config = KGlobal::config();
	config->setGroup( "Plugins" );

	// FIXME: What is this for? This sort of thing is kconf_update's job - Richard
	if ( !pluginId.startsWith( QString::fromLatin1( "kopete_" ) ) )
		pluginId.prepend( QString::fromLatin1( "kopete_" ) );

	if ( !infoForPluginId( pluginId ) )
		return false;

	config->writeEntry( pluginId + QString::fromLatin1( "Enabled" ), enabled );
	config->sync();

	return true;
}

bool PluginManager::isAllPluginsLoaded() const
{
	return d->isAllPluginsLoaded;
}

} //END namespace Kopete


#include "kopetepluginmanager.moc"





