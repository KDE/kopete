/*
    kopetepluginmanager.cpp - Kopete Plugin Loader

    Copyright (c) 2002-2003 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>

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
#include "kopeteaccountmanager.h"

class Kopete::PluginManager::KopetePluginManagerPrivate
{
public:
	// All available plugins, regardless of category, and loaded or not
	QValueList<KPluginInfo *> plugins;

	// Dict of all currently loaded plugins, mapping the KPluginInfo to
	// a plugin
	QMap<KPluginInfo *, Kopete::Plugin *> loadedPlugins;

	// The list of all address book keys used by each plugin
	QMap<Kopete::Plugin *, QStringList> addressBookFields;

	// The plugin manager's mode. The mode is StartingUp until loadAllPlugins()
	// has finished loading the plugins, after which it is set to Running.
	// ShuttingDown and DoneShutdown are used during Kopete shutdown by the
	// async unloading of plugins.
	enum ShutdownMode { StartingUp, Running, ShuttingDown, DoneShutdown };
	ShutdownMode shutdownMode;

	// Plugins pending for loading
	QValueStack<QString> pluginsToLoad;
};

// Put the static deleter in its anonymous namespace -- why?
namespace
{
	KStaticDeleter<Kopete::PluginManager> sd;
}

Kopete::PluginManager* Kopete::PluginManager::s_self = 0L;

Kopete::PluginManager* Kopete::PluginManager::self()
{
	if ( !s_self )
		sd.setObject( s_self, new Kopete::PluginManager() );

	return s_self;
}

Kopete::PluginManager::PluginManager()
: QObject( qApp )
{
	d = new KopetePluginManagerPrivate;

	// We want to add a reference to the application's event loop so we
	// can remain in control when all windows are removed.
	// This way we can unload plugins asynchronously, which is more
	// robust if they are still doing processing.
	kapp->ref();
	d->shutdownMode = KopetePluginManagerPrivate::StartingUp;

	d->plugins = KPluginInfo::fromServices( KTrader::self()->query( QString::fromLatin1( "Kopete/Plugin" ),
		QString::fromLatin1( "[X-Kopete-Version] == 1000900" ) ) );
}

Kopete::PluginManager::~PluginManager()
{
	if ( d->shutdownMode != KopetePluginManagerPrivate::DoneShutdown )
		kdWarning( 14010 ) << k_funcinfo << "Destructing plugin manager without going through the shutdown process!" << endl << kdBacktrace() << endl;

	// Quick cleanup of the remaining plugins, hope it helps
	QMap<KPluginInfo *, Kopete::Plugin *>::ConstIterator it;
	for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); /* EMPTY */ )
	{
		// Remove causes the iterator to become invalid, so pre-increment first
		QMap<KPluginInfo *, Kopete::Plugin *>::ConstIterator nextIt( it );
		++nextIt;
		kdWarning( 14010 ) << k_funcinfo << "Deleting stale plugin '" << it.data()->name() << "'" << endl;
		delete it.data();
		it = nextIt;
	}

	delete d;
}

QValueList<KPluginInfo *> Kopete::PluginManager::availablePlugins( const QString &category ) const
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

QMap<KPluginInfo *, Kopete::Plugin *> Kopete::PluginManager::loadedPlugins( const QString &category ) const
{
	QMap<KPluginInfo *, Kopete::Plugin *> result;
	QMap<KPluginInfo *, Kopete::Plugin *>::ConstIterator it;
	for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
	{
		if ( category.isEmpty() || it.key()->category() == category )
			result.insert( it.key(), it.data() );
	}

	return result;
}

void Kopete::PluginManager::shutdown()
{
//	kdDebug( 14010 ) << k_funcinfo << endl;

	d->shutdownMode = KopetePluginManagerPrivate::ShuttingDown;

	// Remove any pending plugins to load, we're shutting down now :)
	d->pluginsToLoad.clear();

	// Ask all plugins to unload
	QMap<KPluginInfo *, Kopete::Plugin *>::ConstIterator it;
	for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); /* EMPTY */ )
	{
		// Remove causes the iterator to become invalid, so pre-increment first
		QMap<KPluginInfo *, Kopete::Plugin *>::ConstIterator nextIt( it );
		++nextIt;
		it.data()->aboutToUnload();
		it = nextIt;
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

void Kopete::PluginManager::slotPluginReadyForUnload()
{
	// Using QObject::sender() is on purpose here, because otherwise all
	// plugins would have to pass 'this' as parameter, which makes the API
	// less clean for plugin authors
	Kopete::Plugin *plugin = dynamic_cast<Kopete::Plugin *>( const_cast<QObject *>( sender() ) );
	if ( !plugin )
	{
		kdWarning( 14010 ) << k_funcinfo << "Calling object is not a plugin!" << endl;
		return;
	}

	plugin->deleteLater();
}

void Kopete::PluginManager::slotShutdownTimeout()
{
	// When we were already done the timer might still fire.
	// Do nothing in that case.
	if ( d->shutdownMode == KopetePluginManagerPrivate::DoneShutdown )
		return;

	QStringList remaining;
	for ( QMap<KPluginInfo *, Kopete::Plugin *>::ConstIterator it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
		remaining.append( it.data()->pluginId() );

	kdWarning( 14010 ) << k_funcinfo << "Some plugins didn't shutdown in time!" << endl
		<< "Remaining plugins: " << remaining.join( QString::fromLatin1( ", " ) ) << endl
		<< "Forcing Kopete shutdown now." << endl;

	slotShutdownDone();
}

void Kopete::PluginManager::slotShutdownDone()
{
	kdDebug( 14010 ) << k_funcinfo << endl;

	d->shutdownMode = KopetePluginManagerPrivate::DoneShutdown;

	kapp->deref();
}

void Kopete::PluginManager::loadAllPlugins()
{
	// FIXME: We need session management here - Martijn

	KConfig *config = KGlobal::config();
	QMap<QString, QString> entries = config->entryMap( QString::fromLatin1( "Plugins" ) );
	QMap<QString, QString>::Iterator it;
	for ( it = entries.begin(); it != entries.end(); ++it )
	{
		QString key = it.key();
		if ( key.endsWith( QString::fromLatin1( "Enabled" ) ) )
		{
			key.setLength( key.length() - 7 );
			//kdDebug(14010) << k_funcinfo << "Set " << key << " to " << it.data() << endl;

			if ( it.data() == QString::fromLatin1( "true" ) )
			{
				if ( !plugin( key ) )
					d->pluginsToLoad.push( key );
			}
			else
			{
				//This happens if the user unloaded plugins with the config plugin page.
				// No real need to be assync because the user usualy unload few plugins 
				// compared tto the number of plugin to load in a cold start. - Olivier
				if ( plugin( key ) )
					unloadPlugin( key );
			}
		}
	}

	// Schedule the plugins to load
	QTimer::singleShot( 0, this, SLOT( slotLoadNextPlugin() ) );
}

void Kopete::PluginManager::slotLoadNextPlugin()
{
	if ( d->pluginsToLoad.isEmpty() )
	{
		if ( d->shutdownMode == KopetePluginManagerPrivate::StartingUp )
		{
			d->shutdownMode = KopetePluginManagerPrivate::Running;
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

Kopete::Plugin * Kopete::PluginManager::loadPlugin( const QString &_pluginId, PluginLoadMode mode /* = LoadSync */ )
{
	QString pluginId = _pluginId;

	// Try to find legacy code
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

Kopete::Plugin *Kopete::PluginManager::loadPluginInternal( const QString &pluginId )
{
//	kdDebug( 14010 ) << k_funcinfo << pluginId << endl;

	KPluginInfo *info = infoForPluginId( pluginId );
	if ( !info )
	{
		kdWarning( 14010 ) << k_funcinfo << "Unable to find a plugin named '" << pluginId << "'!" << endl;
		return 0L;
	}

	if ( d->loadedPlugins.contains( info ) )
		return d->loadedPlugins[ info ];

	int error = 0;
	Kopete::Plugin *plugin = KParts::ComponentFactory::createInstanceFromQuery<Kopete::Plugin>( QString::fromLatin1( "Kopete/Plugin" ),
		QString::fromLatin1( "[X-KDE-PluginInfo-Name]=='%1'" ).arg( pluginId ), this, 0, QStringList(), &error );

	if ( plugin )
	{
		d->loadedPlugins.insert( info, plugin );
		info->setPluginEnabled( true );

		connect( plugin, SIGNAL( destroyed( QObject * ) ), this, SLOT( slotPluginDestroyed( QObject * ) ) );
		connect( plugin, SIGNAL( readyForUnload() ), this, SLOT( slotPluginReadyForUnload() ) );

		d->addressBookFields.insert( plugin, plugin->addressBookFields() );

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

bool Kopete::PluginManager::unloadPlugin( const QString &spec )
{
//	kdDebug(14010) << k_funcinfo << spec << endl;

	QMap<KPluginInfo *, Kopete::Plugin *>::ConstIterator it;
	for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
	{
		if ( it.key()->pluginName() == spec )
		{
			it.data()->aboutToUnload();
			return true;
		}
	}

	return false;
}

void Kopete::PluginManager::slotPluginDestroyed( QObject *plugin )
{
	d->addressBookFields.remove( static_cast<Kopete::Plugin *>( plugin ) );

	QMap<KPluginInfo *, Kopete::Plugin *>::Iterator it;
	for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
	{
		if ( it.data() == plugin )
		{
			d->loadedPlugins.erase( it );
			break;
		}
	}

	if ( d->shutdownMode == KopetePluginManagerPrivate::ShuttingDown && d->loadedPlugins.isEmpty() )
	{
		// Use a timer to make sure any pending deleteLater() calls have
		// been handled first
		QTimer::singleShot( 0, this, SLOT( slotShutdownDone() ) );
	}
}

QStringList Kopete::PluginManager::addressBookFields( Kopete::Plugin *p ) const
{
	if ( d->addressBookFields.contains( p ) )
		return d->addressBookFields[ p ];
	else
		return QStringList();
}

Kopete::Plugin* Kopete::PluginManager::plugin( const QString &_pluginId ) const
{
	// Hack for compatibility with Kopete::Plugin::pluginId(), which returns
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

QString Kopete::PluginManager::pluginName( const Kopete::Plugin *plugin ) const
{
	QMap<KPluginInfo *, Kopete::Plugin *>::ConstIterator it;
	for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
	{
		if ( it.data() == plugin )
			return it.key()->name();
	}

	return QString::fromLatin1( "Unknown" );
}

QString Kopete::PluginManager::pluginId( const Kopete::Plugin *plugin ) const
{
	QMap<KPluginInfo *, Kopete::Plugin *>::ConstIterator it;
	for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
	{
		if ( it.data() == plugin )
			return it.key()->pluginName();
	}

	return QString::fromLatin1( "unknown" );
}

QString Kopete::PluginManager::pluginIcon( const Kopete::Plugin *plugin ) const
{
	QMap<KPluginInfo *, Kopete::Plugin *>::ConstIterator it;
	for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
	{
		if ( it.data() == plugin )
			return it.key()->icon();
	}

	return QString::fromLatin1( "Unknown" );
}

KPluginInfo * Kopete::PluginManager::infoForPluginId( const QString &pluginId ) const
{
	QValueList<KPluginInfo *>::ConstIterator it;
	for ( it = d->plugins.begin(); it != d->plugins.end(); ++it )
	{
		if ( ( *it )->pluginName() == pluginId )
			return *it;
	}

	return 0L;
}

bool Kopete::PluginManager::setPluginEnabled( const QString &_pluginId, bool enabled /* = true */ )
{
	QString pluginId = _pluginId;

	KConfig *config = KGlobal::config();
	config->setGroup( "Plugins" );

	if ( !pluginId.startsWith( QString::fromLatin1( "kopete_" ) ) )
		pluginId.prepend( QString::fromLatin1( "kopete_" ) );

	if ( !infoForPluginId( pluginId ) )
		return false;

	config->writeEntry( pluginId + QString::fromLatin1( "Enabled" ), enabled );
	config->sync();

	return true;
}

#include "kopetepluginmanager.moc"
// vim: set noet ts=4 sts=4 sw=4:
