/*
    pluginloader.cpp - Kopete Plugin Loader

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

#include "pluginloader.h"

#include <qapplication.h>
#include <qdir.h>
#include <qfile.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kparts/componentfactory.h>
#include <ksimpleconfig.h>
#include <kstaticdeleter.h>
#include <kstddirs.h>
#include <kurl.h>

#include "kopeteplugin.h"

class KopeteLibraryInfo;

// Put the static deleter in its anonymous namespace
namespace
{
	KStaticDeleter<LibraryLoader> sd;
}

bool operator ==(const KopeteLibraryInfo &a, const KopeteLibraryInfo &b)
{
	// Feels like cheating, doesn't it?
	return a.specfile == b.specfile;
}

LibraryLoader* LibraryLoader::s_pluginLoader = 0L;

LibraryLoader* LibraryLoader::pluginLoader()
{
	if( !s_pluginLoader )
		sd.setObject( s_pluginLoader, new LibraryLoader() );

	return s_pluginLoader;
}

LibraryLoader::LibraryLoader()
: QObject( qApp )
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
	l = loaded();
	for(QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
	{
		removeNow((*i).specfile);
	}
}

QPtrList<KopetePlugin> LibraryLoader::plugins() const
{
	QPtrList<KopetePlugin> list;
	QDictIterator<LibraryLoader::PluginLibrary> i( mLibHash );
	for( ; i.current(); ++i )
		list.append( i.current()->plugin );

	return list;
}

QValueList<KopeteLibraryInfo> LibraryLoader::loaded() const
{
	QValueList<KopeteLibraryInfo> items;

	QDictIterator<LibraryLoader::PluginLibrary> i( mLibHash );
	for( ; i.current(); ++i )
		if (isLoaded(i.currentKey()))
			items.append(getInfo(i.currentKey()));

	return items;
}

bool LibraryLoader::loadAll()
{
	KConfig *config=KGlobal::config();
	config->setGroup("");
	QStringList modules = config->readListEntry("Modules");

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
	QMap<QString, KopeteLibraryInfo>::iterator cached = m_cachedInfo.find(spec);        
	if (cached != m_cachedInfo.end() )
		return *cached;
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
	info.license=file.readEntry("License");
	m_cachedInfo[spec]=info;
	return info;
}

bool LibraryLoader::isLoaded(const QString &spec) const
{
	PluginLibrary *lib=mLibHash[spec];
	if (!lib) return false;
	return lib->plugin;
}

void LibraryLoader::setModules(const QStringList &mods)
{
	KConfig *config=KGlobal::config();
	config->setGroup("");
	config->writeEntry("Modules", mods);
	KGlobal::config()->sync();
}

void LibraryLoader::add(const QString &spec)
{
	PluginLibrary *lib=mLibHash[spec];
	if (lib)
		if (lib->plugin) return;

	loadSO(spec);
}

QValueList<KopeteLibraryInfo> LibraryLoader::available() const
{
	QValueList<KopeteLibraryInfo> items;
	QStringList files=KGlobal::dirs()->findAllResources("appdata", "*.plugin", false, true);
	for (QStringList::Iterator i=files.begin(); i!=files.end(); ++i)
		items.append(getInfo(*i));

	return items;
}

bool LibraryLoader::loadSO(const QString &spec)
{
	if( !isLoaded(spec) )
	{
		KopeteLibraryInfo info = getInfo(spec);
		if (info.specfile != spec)
			return false;

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
			KParts::ComponentFactory::createInstanceFromFactory<KopetePlugin>
			( listitem->library->factory(), 0L /* FIXME: parent object */ );

		connect( listitem->plugin, SIGNAL( destroyed( QObject * ) ),
			SLOT( slotPluginDestroyed( QObject * ) ) );

		// Automatically load the i18n catalogue for the plugin
		KGlobal::locale()->insertCatalogue( info.filename );

		listitem->plugin->init();

		m_addressBookFields.insert( listitem->plugin,
			listitem->plugin->addressBookFields() );

		kdDebug() << "[LibraryLoader] loadSO(), loading " << spec << " successful"<< endl;
		emit pluginLoaded(listitem->plugin);
		return true;
	}
	else
	{
		kdDebug() << "[LibraryLoader] loadSO(), " << spec << " is already loaded!" << endl;
		return false;
	}
}

bool LibraryLoader::remove(const QString &spec)
{
	removeNow(spec);

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

bool LibraryLoader::remove(const KopetePlugin *plugin)
{
	for (QDictIterator<PluginLibrary> i(mLibHash); i.current(); ++i)
	{
		if (i.current()->plugin==plugin)
			return remove(i.currentKey());
	}
	return false;

}

void LibraryLoader::removeNow(const QString &spec)
{
	PluginLibrary *lib=mLibHash[spec];
	if (!lib)
		return;

	// Added by Duncan 20/01/2002
	// We need to call unload function for the plugin
	lib->plugin->unload();

	// Some plugins delete themselves on unload, so 'lib' can be a dangling
	// pointer here. Refetch before continuing
	lib=mLibHash[ spec ];
	if( !lib )
		return;

	delete lib->plugin;
	lib->plugin=0;

	mLibHash.remove(spec);
	lib->library->unload();
	delete lib;
}

void LibraryLoader::slotPluginDestroyed( QObject *o )
{
	m_addressBookFields.remove( static_cast<KopetePlugin *>( o ) );

	QDictIterator<PluginLibrary> it( mLibHash );
	for( ; it.current(); ++it )
	{
		if( it.current()->plugin == o )
		{
			it.current()->library->unload();
			delete it.current();
			mLibHash.remove( it.currentKey() );

			break;
		}
	}

	// FIXME: Most likely most data structures here leak and are bound
	// to cause crashes. Find and identify those.
}

QStringList LibraryLoader::addressBookFields( KopetePlugin *p ) const
{
	if( m_addressBookFields.contains( p ) )
		return m_addressBookFields[ p ];
	else
		return QStringList();
}

KopetePlugin * LibraryLoader::searchByName(const QString &name)
{
	for (QDictIterator<PluginLibrary> i(mLibHash); i.current(); ++i)
	{
		if (getInfo(i.currentKey()).name==name)
			return (*i)->plugin;
	}
	return 0L; 
}

#include <pluginloader.moc>

// vim: set noet ts=4 sts=4 sw=4:

