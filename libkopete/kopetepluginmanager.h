/*
    kopetepluginmanager.h - Kopete Plugin Loader

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@ tiscalinet.be>

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

#ifndef KOPETEPLUGINMANAGER_H
#define KOPETEPLUGINMANAGER_H

#include <qobject.h>
#include <qptrlist.h>
#include <qvaluelist.h>

class KPluginInfo;


namespace Kopete {

class Plugin;


/**
 * @brief Manager of plugins and protocols
 *
 * PluginManager is a sigleton, you may access it with @ref PluginManager::self()
 *
 *
 * @author Martijn Klingens <klingens@kde.org>
 * @author Olivier Goffart <ogoffart @ tiscalinet.be>
 */
class PluginManager : public QObject
{
	Q_OBJECT

public:
	/**
	 * Retrieve the plugin loader instance.
	 */
	static PluginManager* self();
	~PluginManager();
	
	/**
	 * Plugin loading mode. Used by @ref loadPlugin. Code that doesn't want to block
	 * the GUI and/or lot a lot of plugins at once should use Async loading.
	 * The default is sync loading.
	 */
	enum PluginLoadMode { LoadSync, LoadAsync };



	/**
	 * @brief Returns a list of available plugins.
	 *
	 * Returns the list of plugin which are availalbe to be loaded from the given categorie.
	 *
	 * Currently there are two categories, "Plugins" and "Protocols", but
	 * you can add your own categories if you want.
	 *
	 * If you pass an empty string you get the complete list of ALL plugins.
	 *
	 * You can query all information on the plugins through the @ref KPluginInfo
	 * interface.
	 */
	QValueList<KPluginInfo *> availablePlugins( const QString &category = QString::null ) const;

	/**
	 * @brief Returns a list of loaded plugins
	 *
	 * Returns a list of all plugins that are actually loaded from the given categorie.
	 * If you omit the category you get all, otherwise it's a filtered list.
	 * See also @ref availablePlugins().
	 */
	QPtrList<Plugin> loadedPlugins( const QString &category = QString::null ) const;
	
	
	/**
	 * @brief Search by plugin name. This is the key used as X-KDE-PluginInfo-Name in
	 * the .desktop file, e.g. "kopete_jabber"
	 *
	 * @return The @ref Plugin object found by the search, or a null
	 * pointer if the plugin is not loaded.
	 *
	 * If you want to also load the plugin you can better use @ref loadPlugin, which returns
	 * the pointer to the plugin if it's already loaded.
	 */
	Plugin *plugin( const QString &pluginName ) const;

	/**
	 * @return the KPluginInfo for the specified plugin
	 */
	KPluginInfo *pluginInfo( const Plugin * ) const;


	/**
	 * @brief Unload the plugin specified by @p pluginName
	 * @param pluginName is the name used as  X-KDE-PluginInfo-Name in  the .desktop file, e.g. "kopete_jabber"
	 * @return true if the plugin has been unloaded.
	 */
	bool unloadPlugin( const QString &pluginName );


	/**
	 * Enable a plugin.
	 *
	 * This marks a plugin as enabled in the config file, so loadAll()
	 * can pick it up later.
	 *
	 * This method does not actually load a plugin, it only edits the
	 * config file.
	 *
	 * @param name is the name of the plugin as it is listed in the .desktop
	 * file in the X-KDE-Library field.
	 *
	 * Returns false when no appropriate plugin can be found.
	 */
	bool setPluginEnabled( const QString &name, bool enabled = true );



	/**
	 * Shuts down the plugin manager on Kopete shutdown, but first
	 * unloads all plugins asynchronously.
	 *
	 * After 3 seconds all plugins should be removed; what's still left
	 * by then is unloaded through a hard delete instead.
	 *
	 * Note that this call also derefs the plugin manager from the event
	 * loop, so do NOT call this method when not terminating Kopete!
	 */
	void shutdown();


public slots:
	/**
	 * @brief Load a single plugin by plugin name.
	 * Returns an existing plugin if one is already loaded in memory.
	 *
	 * If mode is set to Async, the plugin will be queued and loaded in
	 * the background. This method will return a null pointer. To get
	 * the loaded plugin you can track the @ref pluginLoaded() signal.
	 *
	 * See also @ref plugin().
	 */
	Plugin * loadPlugin( const QString &pluginId, PluginLoadMode mode = LoadSync );

	/**
	 * @brief Loads all the enabled plugins.
	 *  Also used to reread the config file when the configuration has changed.
	 */
	void loadAllPlugins();



signals:
	/**
	 * @brief Signals a new plugin has just been loaded.
	 */
	void pluginLoaded( Plugin *plugin );

	/**
	 * @brief All plugins have been loaded by the plugin manager.
	 *
	 * This signal is emitted exactly ONCE, when the plugin manager has emptied
	 * its plugin queue for the first time. This means that if you call an async
	 * loadPlugin() before loadAllPlugins() this signal is probably emitted after
	 * the initial call completes, unless you are quick enough to fill the queue
	 * before it completes, which is a dangerous race you shouldn't count upon :)
	 *
	 * The signal is delayed one event loop iteration through a singleShot timer,
	 * but that is not guaranteed to be enough for account instantiation. You may
	 * need an additional timer for it in the code if you want to programmatically
	 * act on it.
	 *
	 * If you use the signal for enabling/disabling GUI objects there is little
	 * chance a user is able to activate them in the short while that's remaining,
	 * the slow part of the code is over now and the remaining processing time
	 * is neglectable for the user.
	 */
	void allPluginsLoaded();


private slots:
	/**
	 * @brief Cleans up some references if the plugin is destroyed
	 */
	void slotPluginDestroyed( QObject *plugin );

	/**
	 * @internal
	 * shutdown() starts a timer, when it fires we force all plugins
	 * to be unloaded here by deref()-ing the event loop to trigger the plugin
	 * manager's destruction
	 */
	void slotShutdownTimeout();

	/**
	 * @internal
	 * Common entry point to deref() the KApplication. Used both by the clean
	 * shutdown and the timeout condition of slotShutdownTimeout()
	 */
	void slotShutdownDone();

	/**
	 * @internal
	 * Emitted by a Plugin when it's ready for unload
	 */
	void slotPluginReadyForUnload();

	/**
	 * @internal
	 * Load a plugin from our queue. Does nothing if the queue is empty.
	 * Schedules itself again if more plugins are pending.
	 */
	void slotLoadNextPlugin();

private:
	PluginManager();

	/**
	 * @internal
	 *
	 * The internal method for loading plugins.
	 * Called by @ref loadPlugin directly or through the queue for async plugin
	 * loading.
	 */
	Plugin * loadPluginInternal( const QString &pluginId );

	/**
	 * @internal
	 *
	 * Find the KPluginInfo structure by key. Reduces some code duplication.
	 *
	 * Returns a null pointer when no plugin info is found.
	 */
	KPluginInfo * infoForPluginId( const QString &pluginId ) const;

	static PluginManager *s_self;

	class Private;
	Private *d;
};


} //END namespace Kopete

#endif // KOPETEPLUGINMANAGER_H
