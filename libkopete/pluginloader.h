/*
    pluginloader.h - Kopete Plugin Loader

    Copyright (c) 2001-2002 by the Kopete developers  <kopete-devel@kde.org>

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
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include <klibloader.h>

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
	QStringList require;
};

bool operator ==(const KopeteLibraryInfo &, const KopeteLibraryInfo &);

class KopeteProtocol;
class Plugin;

class LibraryLoader
{
	friend class Kopete;
	friend class Plugin;
	friend class KopeteProtocol;
	friend class AddWizardImpl;
	struct PluginLibrary
	{
		Plugin *plugin;
		KLibrary *library;
	};

public:
	LibraryLoader();
	~LibraryLoader();

	QValueList<KopeteLibraryInfo> available() const;
	QValueList<KopeteLibraryInfo> loaded() const;

    /**
	 * Search by Id
	 */
	Plugin *searchByID( QString &Id );

	/**
	 * loads all the enabled plugins
	 */
	bool loadAll(void);
	bool loadAll(const QStringList &);

	bool isLoaded(const QString &spec) const;
	void add(const QString &spec);
	void setModules(const QStringList &mods);
	/**
	 * unload the plugin specified by spec
	 */
	bool remove(const QString &spec);
	/**
	 * unload the plugin that is plugin
	 */
	bool remove(const LibraryLoader::PluginLibrary *plugin);
	bool remove(const Plugin *plugin);
	
	//Playlist *playlist() const;

	/**
	 * This is needed for the Plugin-List-View
	 * to see what plugins are required to show
	 * (when required by another noatun-plugin)
	**/
	KopeteLibraryInfo getInfo(const QString &spec) const;
	QList<Plugin> plugins() const;
	
private:
	bool loadSO(const QString &spec);
	void removeNow(const QString &spec);

private:
	QDict<LibraryLoader::PluginLibrary> mLibHash;
	//Playlist *mPlaylist;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

