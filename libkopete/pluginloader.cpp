/*
    pluginloader.cpp - Kopete Plugin Loader

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

#include <qdir.h>
#include <qfile.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>
#include <kurl.h>
#include <kparts/componentfactory.h>

#include "kopete.h"
#include "plugin.h"
#include "pluginloader.h"

class KopeteLibraryInfo;

bool operator ==(const KopeteLibraryInfo &a, const KopeteLibraryInfo &b)
{
	// Feels like cheating, doesn't it?
	return a.specfile == b.specfile;
}

LibraryLoader::LibraryLoader()
{
}

LibraryLoader::~LibraryLoader()
{
	QValueList<KopeteLibraryInfo> l;

	l = loaded();
	for(QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
	{
		if((*i).type != "protocol" && (*i).type != "ui" && (*i).type != "dock")
		{
			removeNow((*i).specfile);
		}
	}
	/*
	l = loaded();
	for(QValueList<NoatunLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
	{
		if((*i).type == "userinterface")
		{
			removeNow((*i).specfile);
		}
	}
	*/
	l = loaded();
	for(QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
	{
		removeNow((*i).specfile);
	}
}

QValueList<KopeteLibraryInfo> LibraryLoader::available() const
{
	QValueList<KopeteLibraryInfo> items;
	QStringList files=KGlobal::dirs()->findAllResources("appdata", "*.plugin", false, true);
	for (QStringList::Iterator i=files.begin(); i!=files.end(); ++i)
		items.append(getInfo(*i));

	return items;
}

QList<Plugin> LibraryLoader::plugins() const
{
	QList<Plugin> list;
	for (QDictIterator<LibraryLoader::PluginLibrary> i(mLibHash); i.current(); ++i)
		list.append(i.current()->plugin);
	return list;
}

bool LibraryLoader::loadAll(void)
{
	KConfig *config=KGlobal::config();
	config->setGroup("");
	QStringList modules = config->readListEntry("Modules");
	return loadAll(modules);
}

bool LibraryLoader::loadAll(const QStringList &modules)
{
	// Session management...
/*
	for(QStringList::ConstIterator i=modules.begin(); i!=modules.end(); ++i)
	{
		KopeteLibraryInfo info=getInfo(*i);
		if (!info.type.contains("sm"))
			continue;
		loadSO(*i);
	}
*/
	// load all the protocols in the first
	for(QStringList::ConstIterator i=modules.begin(); i!=modules.end(); ++i)
	{
		KopeteLibraryInfo info=getInfo(*i);
		if (!info.type.contains("protocol"))
			continue;

		if ( !loadSO(*i) )
			kdDebug() << "[LibraryLoader] loading " << (*i) << " failed!" << endl;
	}

	// load all misc plugins
	for(QStringList::ConstIterator i=modules.begin(); i!=modules.end(); ++i)
	{
		KopeteLibraryInfo info=getInfo(*i);
		if (!info.type.contains("other"))
			continue;

		if ( !loadSO(*i) )
			kdDebug() << "[LibraryLoader] loading " << (*i) << " failed!" << endl;
	}

	return true;
}

KopeteLibraryInfo LibraryLoader::getInfo(const QString &spec) const
{
	KopeteLibraryInfo info;
	QString specPath = (spec[0]=='/') ? spec : KGlobal::dirs()->findResource("appdata", spec);
	if (!QFile::exists(specPath))
		return info;
	KSimpleConfig file(specPath);
	if (spec.find('/')>=0)
		info.specfile=KURL(spec).fileName();
	else
		info.specfile=spec;
	info.filename=file.readEntry("Filename");
	info.author=file.readEntry("Author");
	info.site=file.readEntry("Site");
	info.email=file.readEntry("Email");
	info.type=file.readEntry("Type");
	info.name=file.readEntry("Name");
	info.comment=file.readEntry("Comment");
	info.require=file.readListEntry("Require");
	info.license=file.readEntry("License");
	return info;
}

bool LibraryLoader::isLoaded(const QString &spec) const
{
	PluginLibrary *lib=mLibHash[spec];
	if (!lib) return false;
	return lib->plugin;
}

bool LibraryLoader::loadSO(const QString &spec)
{
	if( !isLoaded(spec) )
	{
		KopeteLibraryInfo info = getInfo(spec);
		if (info.specfile != spec)
			return false;

		for (QStringList::ConstIterator it = info.require.begin(); it != info.require.end(); ++it)
			loadSO(*it);

		// get the library loader instance
		KLibLoader *loader = KLibLoader::self();

		PluginLibrary *listitem=mLibHash[spec];

		if (!listitem)
		{
			KLibrary *lib = loader->library( QFile::encodeName(info.filename) );
			if (!lib)
			{
				kdDebug() << "[LibraryLoader] loadSO(), error while loading library: " << loader->lastErrorMessage() << endl;
				return false;
			}
			listitem = new PluginLibrary;
			listitem->library = lib;
			mLibHash.insert(spec, listitem);
		}

		listitem->plugin =
			KParts::ComponentFactory::createInstanceFromFactory<Plugin>
			( listitem->library->factory(), 0L /* FIXME: parent object */ );

		connect( listitem->plugin, SIGNAL( destroyed( QObject * ) ),
			SLOT( slotPluginDestroyed( QObject * ) ) );

		// Automatically load the i18n catalogue for the plugin
		KGlobal::locale()->insertCatalogue( info.filename );

		listitem->plugin->init();

		m_addressBookFields.insert( listitem->plugin,
			listitem->plugin->addressBookFields() );

		kdDebug() << "[LibraryLoader] loadSO(), loading " << spec << " successful"<< endl;
		return true;
	}
	else
	{
		kdDebug() << "[LibraryLoader] loadSO(), " << spec << " is already loaded!" << endl;
		return false;
	}
}

void LibraryLoader::add(const QString &spec)
{
	PluginLibrary *lib=mLibHash[spec];
	if (lib)
		if (lib->plugin) return;

	loadSO(spec);
}

void LibraryLoader::setModules(const QStringList &mods)
{
	KConfig *config=KGlobal::config();
	config->setGroup("");
	config->writeEntry("Modules", mods);
	KGlobal::config()->sync();
}

bool LibraryLoader::remove(const QString &spec)
{
	removeNow(spec);

	// exit if this is the last UI
	/*
	if (getInfo(spec).type=="userinterface")
	{
		QValueList<NoatunLibraryInfo> l=loaded();
		bool isanotherui=false;
		for (QValueList<NoatunLibraryInfo>::Iterator i=l.begin(); i!=l.end(); ++i)
		{
			if ((*i).specfile!=spec && (*i).type=="userinterface")
				isanotherui=true;
		}
		if (!isanotherui)
			kapp->exit();
	}
  */
	return true;
}

bool LibraryLoader::remove(const PluginLibrary *pl)
{
	for (QDictIterator<PluginLibrary> i(mLibHash); i.current(); ++i)
	{
		if (i.current()==pl)
			return remove(i.currentKey());
	}
	return false;
}

bool LibraryLoader::remove(const Plugin *plugin)
{
	for (QDictIterator<PluginLibrary> i(mLibHash); i.current(); ++i)
	{
		if (i.current()->plugin==plugin)
			return remove(i.currentKey());
	}
	return false;

}

/*
Playlist *LibraryLoader::playlist() const
{
        return mPlaylist;
}
*/

QValueList<KopeteLibraryInfo> LibraryLoader::loaded() const
{
	QValueList<KopeteLibraryInfo> items;

	for (QDictIterator<PluginLibrary> i(mLibHash); i.current(); ++i)
		if (isLoaded(i.currentKey()))
			items.append(getInfo(i.currentKey()));

	return items;
}

void LibraryLoader::removeNow(const QString &spec)
{
	KopeteLibraryInfo info = getInfo(spec);
	if (info.specfile == spec)
	{
		QValueList<KopeteLibraryInfo> l = loaded();
		for (QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
		{
			for (QStringList::ConstIterator it = (*i).require.begin(); it != (*i).require.end(); ++it)
				if (*it == spec)
				removeNow((*i).specfile);
		}
	}

	PluginLibrary *lib=mLibHash[spec];

	if (!lib)
		return;

	// Added by Duncan 20/01/2002
	// We need to call unload function for the plugin
	lib->plugin->unload();

	delete lib->plugin;
	lib->plugin=0;

	mLibHash.remove(spec);
//	delete lib->library;
//	delete lib;
}

Plugin* LibraryLoader::searchByID( QString &Id )
{
	QValueList<KopeteLibraryInfo> l = loaded();

	for (QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
	{
		Plugin *tmp_plug = mLibHash[(*i).specfile]->plugin;
		if ( tmp_plug->id() == Id )
		{
			return tmp_plug;
		}
	}
	return NULL;
}

void LibraryLoader::slotPluginDestroyed( QObject *o )
{
	Plugin *p = dynamic_cast<Plugin *>( o );
	if( p )
	{
		m_addressBookFields.remove( p );

		// FIXME: Most likely most data structures here leak and are bound
		// to cause crashes. Find and identify those.
	}
}

QStringList LibraryLoader::addressBookFields( Plugin *p ) const
{
	if( m_addressBookFields.contains( p ) )
		return m_addressBookFields[ p ];
	else
		return QStringList();
}

#include <pluginloader.moc>

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */

// vim: set noet ts=4 sts=4 sw=4:

