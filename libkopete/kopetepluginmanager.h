/*
    kopetepluginmanager.h - Kopete Plugin Loader

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

#ifndef KOPETEPLUGINMANAGER_H
#define KOPETEPLUGINMANAGER_H

#include <qmap.h>
#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

class KPluginInfo;

class KopetePlugin;

/**
 * @author Duncan Mac-Vicar Prett <duncan@kde.org>
 * @author Martijn Klingens <klingens@kde.org>
 */
class KopetePluginManager : public QObject
{
	Q_OBJECT

public:
	/**
	 * Retrieve the plugin loader instance.
	 */
	static KopetePluginManager* self();

	~KopetePluginManager();

	/**
	 * Returns a list of all available plugins for the given category.
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
	 * Returns a list of all plugins that are actually loaded.
	 * If you omit the category you get all, otherwise it's a filtered list.
	 * See also @ref availablePlugins().
	 */
	QMap<KPluginInfo *, KopetePlugin *> loadedPlugins( const QString &category = QString::null ) const;

	/**
	 * @brief Search by plugin name. This is the key used as X-KDE-PluginInfo-Name in
	 * the .desktop file, e.g. "kopete_jabber"
	 *
	 * @return The @ref KopetePlugin object found by the search, or a null
	 * pointer if the plugin is not loaded.
	 *
	 * If you want to also load the plugin you can better use @ref loadPlugin, which returns
	 * the pointer to the plugin if it's already loaded.
	 */
	KopetePlugin *plugin( const QString &pluginName ) const;

	/**
	 * @brief Return the short user-visible name of the plugin.
	 *
	 * If you want to have the internal name, use @ref pluginId() instead.
	 *
	 * @return The name of the protocol, in the user's locale.
	 */
	QString pluginName( const KopetePlugin *plugin ) const;

	/**
	 * @brief Return the internal name of the plugin.
	 *
	 * You cannot display this name on the screen, it's used internally for
	 * passing around IDs. Use @ref pluginName() for a string ready for display.
	 *
	 * @return The name of the protocol, in the user's locale.
	 */
	QString pluginId( const KopetePlugin *plugin ) const;

	/**
	 * @brief Unload the plugin specified by @p pluginName
	 */
	bool unloadPlugin( const QString &pluginName );

	/**
	 * @brief Retrieve all registered address book fields for a given plugin.
	 *
	 * @return A QStringList object containing all the address book fields
	 * available to the plugin.
	 * @return An empty QStringList if the plugin is invalid.
	 */
	QStringList addressBookFields( KopetePlugin *p ) const;

	/**
	 * @brief Retrieve the name of the icon for a @ref KopetePlugin.
	 *
	 * @return An empty string if the given plugin is not loaded
	 * or the filename of the icon to use.
	 */
	QString pluginIcon( const KopetePlugin *plugin ) const;

public slots:
	/**
	 * @brief Load a single plugin by plugin name. Returns an existing plugin
	 * if one is already loaded in memory.
	 *
	 * See also @ref plugin().
	 */
	KopetePlugin *loadPlugin( const QString &pluginName );

	/**
	 * @brief Loads all the enabled plugins. Also used to reread the
	 * config file when the configuration has changed.
	 */
	void loadAllPlugins();

signals:
	/**
	 * @brief Signals a new plugin has just been loaded.
	 */
	void pluginLoaded( KopetePlugin *plugin );

private slots:
	/**
	 * @brief Cleans up some references if the plugin is destroyed
	 */
	void slotPluginDestroyed( QObject *plugin );

	/**
	 * unloadAllPlugins() starts a timer, when it fires we force all plugins
	 * to be unloaded here by deref()-ing the event loop to trigger the plugin
	 * manager's destruction
	 */
	void slotUnloadAllPluginsTimeout();

	/**
	 * Unload all plugins on Kopete quit. This slot is called when the
	 * main window is closed in Kopete::quitKopete() to asynchronously
	 * unload plugins.
	 *
	 * After 3 seconds all plugins should be removed; what's still left
	 * by then is unloaded through a hard delete instead.
	 *
	 * Note that this call also derefs the plugin manager from the event
	 * loop, so do NOT call this method when not terminating Kopete!
	 */
	void unloadAllPlugins();

private:
	KopetePluginManager();

	static KopetePluginManager *s_self;

	class KopetePluginManagerPrivate;
	KopetePluginManagerPrivate *d;
};

#endif // KOPETEPLUGINMANAGER_H

// vim: set noet ts=4 sts=4 sw=4:

