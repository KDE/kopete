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

#include "kopetepluginmanager.h"

#include <qapplication.h>
#include <qfile.h>
#include <qregexp.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kparts/componentfactory.h>
#include <kplugininfo.h>
#include <ksettings/dispatcher.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>
#include <kurl.h>

#include "kopeteplugin.h"
#include "kopeteaccountmanager.h"

class KopetePluginManager::KopetePluginManagerPrivate
{
public:
	// All available plugins, regardless of category, and loaded or not
	QValueList<KPluginInfo *> plugins;

	// Dict of all currently loaded plugins, mapping the KPluginInfo to
	// a plugin
	QMap<KPluginInfo *, KopetePlugin *> loadedPlugins;

	// The list of all address book keys used by each plugin
	QMap<KopetePlugin *, QStringList> addressBookFields;

	// When true we're shutting down and we should deref() the application
	// if all plugins are gone
	enum ShutdownMode { Running, ShuttingDown, DoneShutdown };
	ShutdownMode shutdownMode;
};

// Put the static deleter in its anonymous namespace
namespace
{
	KStaticDeleter<KopetePluginManager> sd;
}

KopetePluginManager* KopetePluginManager::s_self = 0L;

KopetePluginManager* KopetePluginManager::self()
{
	if ( !s_self )
		sd.setObject( s_self, new KopetePluginManager() );

	return s_self;
}

KopetePluginManager::KopetePluginManager()
: QObject( qApp )
{
	d = new KopetePluginManagerPrivate;

	// We want to add a reference to the application's event loop so we
	// can remain in control when all windows are removed.
	// This way we can unload plugins asynchronously, which is more
	// robust if they are still doing processing.
	kapp->ref();
	d->shutdownMode = KopetePluginManagerPrivate::Running;

	KSettings::Dispatcher::self()->registerInstance( KGlobal::instance(), this, SLOT( loadAllPlugins() ) );

	d->plugins = KPluginInfo::fromServices( KTrader::self()->query( QString::fromLatin1( "Kopete/Plugin" ) ) );
}

KopetePluginManager::~KopetePluginManager()
{
	if ( d->shutdownMode != KopetePluginManagerPrivate::DoneShutdown )
		kdWarning( 14010 ) << k_funcinfo << "Destructing plugin manager without going through the shutdown process!" << endl << kdBacktrace() << endl;

	// Quick cleanup of the remaining plugins, hope it helps
	QMap<KPluginInfo *, KopetePlugin *>::ConstIterator it;
	for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); /* EMPTY */ )
	{
		// Remove causes the iterator to become invalid, so pre-increment first
		QMap<KPluginInfo *, KopetePlugin *>::ConstIterator nextIt( it );
		++nextIt;
		kdWarning( 14010 ) << k_funcinfo << "Deleting stale plugin '" << it.data()->name() << "'" << endl;
		delete it.data();
		it = nextIt;
	}

	delete d;
}

QValueList<KPluginInfo *> KopetePluginManager::availablePlugins( const QString &category ) const
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

QMap<KPluginInfo *, KopetePlugin *> KopetePluginManager::loadedPlugins( const QString &category ) const
{
	QMap<KPluginInfo *, KopetePlugin *> result;
	QMap<KPluginInfo *, KopetePlugin *>::ConstIterator it;
	for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
	{
		if ( category.isEmpty() || it.key()->category() == category )
			result.insert( it.key(), it.data() );
	}

	return result;
}

void KopetePluginManager::shutdown()
{
	kdDebug( 14010 ) << k_funcinfo << endl;

	d->shutdownMode = KopetePluginManagerPrivate::ShuttingDown;

	//first: save the accounts before unloading them
	//FIXME: i don't like this depedence between KopetePluginManager and KopeteAccountManager
	KopeteAccountManager::manager()->save();

	// Ask all plugins to unload
	QMap<KPluginInfo *, KopetePlugin *>::ConstIterator it;
	for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); /* EMPTY */ )
	{
		// Remove causes the iterator to become invalid, so pre-increment first
		QMap<KPluginInfo *, KopetePlugin *>::ConstIterator nextIt( it );
		++nextIt;
		it.data()->aboutToUnload();
		it = nextIt;
	}

	QTimer::singleShot( 3000, this, SLOT( slotShutdownTimeout() ) );
}

void KopetePluginManager::slotPluginReadyForUnload()
{
	// Using QObject::sender() is on purpose here, because otherwise all
	// plugins would have to pass 'this' as parameter, which makes the API
	// less clean for plugin authors
	KopetePlugin *plugin = dynamic_cast<KopetePlugin *>( const_cast<QObject *>( sender() ) );
	if ( !plugin )
	{
		kdWarning( 14010 ) << k_funcinfo << "Calling object is not a plugin!" << endl;
		return;
	}

	plugin->deleteLater();
}

void KopetePluginManager::slotShutdownTimeout()
{
	kdWarning( 14010 ) << k_funcinfo << "Some plugins didn't shutdown in time!" << endl
		<< "Forcing Kopete shutdown now." << endl;

	slotShutdownDone();
}

void KopetePluginManager::slotShutdownDone()
{
	kdDebug( 14010 ) << k_funcinfo << endl;

	d->shutdownMode = KopetePluginManagerPrivate::DoneShutdown;

	kapp->deref();
}

void KopetePluginManager::loadAllPlugins()
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
//			kdDebug() << k_funcinfo << "Set " << key << " to " << it.data() << endl;

			if ( it.data() == QString::fromLatin1( "true" ) )
			{
				if ( !plugin( key ) )
				{
					loadPlugin( key );

					// FIXME: processEvents is evil, we need to use a queue and
					//        a singleshot timer instead - Martijn
					kapp->processEvents();
				}
			}
			else
			{
				if ( plugin( key ) )
					unloadPlugin( key );
			}
		}
	}
}

KopetePlugin *KopetePluginManager::loadPlugin( const QString &spec_ )
{
	QString spec = spec_;

	// Try to find legacy code
	if ( spec.endsWith( QString::fromLatin1( ".desktop" ) ) )
	{
		kdWarning( 14010 ) << "Trying to use old-style API!" << endl << kdBacktrace() << endl;
		spec = spec.remove( QRegExp( QString::fromLatin1( ".desktop$" ) ) );
	}

	kdDebug( 14010 ) << k_funcinfo << spec << endl;

	KPluginInfo *info = 0L;
	QValueList<KPluginInfo *>::ConstIterator it;
	for ( it = d->plugins.begin(); it != d->plugins.end(); ++it )
	{
		if ( ( *it )->pluginName() == spec )
		{
			info = *it;
			break;
		}
	}

	if ( !info )
	{
		kdWarning( 14010 ) << k_funcinfo << "Unable to find a plugin named '" << spec << "'!" << endl;
		return 0L;
	}

	if ( d->loadedPlugins.contains( info ) )
		return d->loadedPlugins[ info ];

	int error = 0;
	KopetePlugin *plugin = KParts::ComponentFactory::createInstanceFromQuery<KopetePlugin>(
		QString::fromLatin1( "Kopete/Plugin" ),
		QString::fromLatin1( "[X-KDE-PluginInfo-Name]=='%1'" ).arg( spec ), this, 0, QStringList(), &error );

	if ( plugin )
	{
		d->loadedPlugins.insert( info, plugin );
		info->setPluginEnabled( true );

		connect( plugin, SIGNAL( destroyed( QObject * ) ), this, SLOT( slotPluginDestroyed( QObject * ) ) );
		connect( plugin, SIGNAL( readyForUnload() ), this, SLOT( slotPluginReadyForUnload() ) );

		d->addressBookFields.insert( plugin, plugin->addressBookFields() );

		kdDebug( 14010 ) << k_funcinfo << "Successfully loaded plugin '" << spec << "'" << endl;

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

		kdDebug( 14010 ) << k_funcinfo << "Loading plugin '" << spec << "' failed, KLibLoader reported error: '" << endl <<
			KLibLoader::self()->lastErrorMessage() << "'" << endl;
	}

	return plugin;
}

bool KopetePluginManager::unloadPlugin( const QString &spec )
{
	kdDebug() << k_funcinfo << spec << endl;

	QMap<KPluginInfo *, KopetePlugin *>::ConstIterator it;
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

void KopetePluginManager::slotPluginDestroyed( QObject *plugin )
{
	d->addressBookFields.remove( static_cast<KopetePlugin *>( plugin ) );

	QMap<KPluginInfo *, KopetePlugin *>::Iterator it;
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

QStringList KopetePluginManager::addressBookFields( KopetePlugin *p ) const
{
	if ( d->addressBookFields.contains( p ) )
		return d->addressBookFields[ p ];
	else
		return QStringList();
}

KopetePlugin* KopetePluginManager::plugin( const QString &_pluginId ) const
{
	// Hack for compatibility with KopetePlugin::pluginId(), which returns
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

	KPluginInfo *info = 0L;
	QValueList<KPluginInfo *>::ConstIterator it;
	for ( it = d->plugins.begin(); it != d->plugins.end(); ++it )
	{
		if ( ( *it )->pluginName() == pluginId )
		{
			info = *it;
			break;
		}
	}

	if ( !info )
		return 0L;

	if ( d->loadedPlugins.contains( info ) )
		return d->loadedPlugins[ info ];
	else
		return 0L;
}

QString KopetePluginManager::pluginName( const KopetePlugin *plugin ) const
{
	QMap<KPluginInfo *, KopetePlugin *>::ConstIterator it;
	for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
	{
		if ( it.data() == plugin )
			return it.key()->name();
	}

	return QString::fromLatin1( "Unknown" );
}

QString KopetePluginManager::pluginId( const KopetePlugin *plugin ) const
{
	QMap<KPluginInfo *, KopetePlugin *>::ConstIterator it;
	for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
	{
		if ( it.data() == plugin )
			return it.key()->pluginName();
	}

	return QString::fromLatin1( "unknown" );
}

QString KopetePluginManager::pluginIcon( const KopetePlugin *plugin ) const
{
	QMap<KPluginInfo *, KopetePlugin *>::ConstIterator it;
	for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
	{
		if ( it.data() == plugin )
			return it.key()->icon();
	}

	return QString::fromLatin1( "Unknown" );
}

#include "kopetepluginmanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

