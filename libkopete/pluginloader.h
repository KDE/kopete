/*
    pluginloader.h - Kopete Plugin Loader

    Copyright (c) 2002 by Duncan Mac-Vicar Prett       <duncan@kde.org>

    Portions of this code based in Noatun plugin code:
    Copyright (c) 2000-2002 The Noatun Developers

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
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
	QString author;
	QString license;
	QString type;
	QString site;
	QString email;
	QString name;
	QString comment;
	QString icon;
};

bool operator ==(const KopeteLibraryInfo &, const KopeteLibraryInfo &);


/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
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

	/**
	 * FIXME: These 6 methods are only used internally and by
	 *        pluginconfig. Fix that code and remove them or
	 *        make them private.
	 *
	 * This is needed for the Plugin-List-View
	 * to see what plugins are required to show
	 * (when required by another noatun-plugin)
	 */
	KopeteLibraryInfo getInfo(const QString &spec) const;
	QValueList<KopeteLibraryInfo> available() const;
	QValueList<KopeteLibraryInfo> loaded() const;
	bool isLoaded(const QString &spec) const;
	void setModules(const QStringList &mods);
	bool loadPlugin( const QString &spec );

	/**
	 * Search by Id
	 * ex: "ICQProtocol"
	 */
	KopetePlugin *searchByID( const QString &Id );

	/**
	 * Search by name
	 * ex: "ICQ"
	 */
	KopetePlugin *searchByName(const QString&);
	
	/**
	 * The opposite of searchByName. Returns the name of the protocol.
	 */
	QString pluginName(KopetePlugin *plugin);

	/**
	 * loads all the enabled plugins
	 */
	bool loadAll();

	/**
	 * unload the plugin specified by spec
	 */
	bool remove(const QString &spec);

	/**
	 * unload the plugin that is plugin
	 */
	bool remove( KopetePlugin *plugin );

	QPtrList<KopetePlugin> plugins() const;

	/**
	 * Return all registered address book fields for a given plugin.
	 *
	 * Returns an empty QStringList if the plugin is invalid.
	 */
	QStringList addressBookFields( KopetePlugin *p ) const;

	/**
	 * Retrieve the name of the icon for a KopetePlugin
	 *
	 * May return an empty string if the given plugin is not loaded, or
	 * the .plugin file for this plugin specifies no icon name to use.
	 */
	QString pluginIcon( const QString &pluginId ) const;

signals:
	void pluginLoaded(KopetePlugin *);

private slots:
	/**
	 * Cleanup some references if the plugin is destroyed
	 */
	void slotPluginDestroyed( QObject *o );

private:
	LibraryLoader();

	QDict<KopetePlugin> mLibHash;

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

