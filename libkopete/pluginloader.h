/*
    pluginloader.h - Kopete Plugin Loader

    Copyright (c) 2002-2003 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>

    Portions of this code are based on the Noatun plugin code
    Noatun    (c) 2000-2002 The Noatun Developers

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

#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H

#include <qdict.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include <klibloader.h>

class KopeteProtocol;
class KopetePlugin;

struct KopeteLibraryInfo
{
	QString specfile;
	QString filename;
	QString type;
	QString name;
	QString comment;
	QString icon;
	QString messagingProtocol;
};

bool operator ==(const KopeteLibraryInfo &, const KopeteLibraryInfo &);

/**
 * @author Duncan Mac-Vicar Prett <duncan@kde.org>
 * @author Martijn Klingens <klingens@kde.org>
 */
class LibraryLoader : public QObject
{
	Q_OBJECT

public:
	/**
	 * Retrieve the plugin loader instance.
	 */
	static LibraryLoader* pluginLoader();

	~LibraryLoader();

	 // FIXME: These 6 methods are only used internally and by
	 //        pluginconfig. Fix that code and remove them or
	 //        make them private.
	 // This is needed for the Plugin-List-View
	 // to see what plugins are required to show
	 // (when required by another kopete-plugin)
	KopeteLibraryInfo getInfo(const QString &spec) const;
	QValueList<KopeteLibraryInfo> available() const;
	QValueList<KopeteLibraryInfo> loaded() const;
	bool isLoaded(const QString &spec) const;
	KopetePlugin *loadPlugin( const QString &spec );

	/**
	 * @brief Search by ID
	 *
	 * ex: "ICQProtocol"
	 *
	 * @return The @ref KopetePlugin object found by the search
	 */
	KopetePlugin *searchByID( const QString &Id );

	/**
	 * @brief Search by name
	 *
	 * ex: "ICQ"
	 *
	 * @return The @ref KopetePlugin object found by the search
	 */
	KopetePlugin *searchByName(const QString&);

	/**
	 * @brief The opposite of searchByName.
	 *
	 * @return The name of the protocol.
	 */
	QString pluginName( const KopetePlugin *plugin ) const;

	/**
	 * @brief Loads all the enabled plugins
	 */
	bool loadAll();

	/**
	 * @brief Unload the plugin specified by @p spec
	 */
	bool remove(const QString &spec);

	/**
	 * @brief Retrieve a list of all loaded plugins
	 *
	 * @return a list of all loaded plugins or protocols
	 */
	QPtrList<KopetePlugin> plugins() const;

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

signals:
	/**
	 * @brief Signals a new plugin has just been loaded.
	 */
	void pluginLoaded(KopetePlugin *);

private slots:
	/**
	 * @brief Cleans up some references if the plugin is destroyed
	 */
	void slotPluginDestroyed( QObject *o );

	/**
	 * The current configuration has changed; reread the config file
	 */
	void slotKopeteSettingsChanged();

private:
	LibraryLoader();

	QDict<KopetePlugin> m_loadedPlugins;

	/**
	 * The list of all address book keys used by each plugin
	 */
	QMap<KopetePlugin *, QStringList> m_addressBookFields;

	/**
	 * A cache for plugin info, to avoid reparsing (and hence mutable)
	 */
	mutable QMap<QString, KopeteLibraryInfo> m_cachedInfo;

	static LibraryLoader *s_pluginLoader;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

