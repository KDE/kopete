/*
    pluginloader.cpp - Kopete Plugin Loader

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>

    Portions of this code based in Noatun plugin code:
    Copyright (c) 2000-2002 The Noatun Developers

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

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
#include <kstandarddirs.h>
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
	QDictIterator<KopetePlugin> i( mLibHash );
	while( i.current() )
	{
		// Remove causes the iterator to auto-increment, so
		// only increment explicitly when not removing
		if( getInfo( i.currentKey() ).type != QString::fromLatin1( "Kopete/Protocol" ) )
			remove( i.current() );
		else
			++i;
	}
	i.toFirst();
	while( i.current() )
		remove( i.current() );

	kdDebug(14010) << "LibraryLoader::~LibraryLoader(): all plugins removed" << endl;
}

QPtrList<KopetePlugin> LibraryLoader::plugins() const
{
	QPtrList<KopetePlugin> list;
	QDictIterator<KopetePlugin> i( mLibHash );
	for( ; i.current(); ++i )
		list.append( i.current() );

	return list;
}

QValueList<KopeteLibraryInfo> LibraryLoader::loaded() const
{
	QValueList<KopeteLibraryInfo> items;

	QDictIterator<KopetePlugin> i( mLibHash );
	for( ; i.current(); ++i )
	{
		if( mLibHash[ i.currentKey() ] )
			items.append( getInfo( i.currentKey() ) );
	}

	return items;
}

bool LibraryLoader::loadAll()
{
	KConfig *config=KGlobal::config();
	config->setGroup("");
	QStringList modules = config->readListEntry("Plugins");

	// Session management...
/*
	for(QStringList::ConstIterator i=modules.begin(); i!=modules.end(); ++i)
	{
		KopeteLibraryInfo info=getInfo(*i);
		if (!info.type.contains("sm"))
			continue;
		loadPlugin( *i );
	}
*/
	// load all the protocols in the first
	for(QStringList::ConstIterator i=modules.begin(); i!=modules.end(); ++i)
	{
		KopeteLibraryInfo info=getInfo(*i);
		if( !info.type.contains( QString::fromLatin1( "Kopete/Protocol" ) ) )
			continue;

		if ( !loadPlugin( *i ) )
			kdDebug(14010) << "[LibraryLoader] loading " << (*i) << " failed!" << endl;
	}

	// load all misc plugins
	for(QStringList::ConstIterator i=modules.begin(); i!=modules.end(); ++i)
	{
		KopeteLibraryInfo info=getInfo(*i);
		if( !info.type.contains( QString::fromLatin1( "Kopete/Plugin" ) ) )
			continue;

		if ( !loadPlugin( *i ) )
			kdDebug(14010) << "[LibraryLoader] loading " << (*i) << " failed!" << endl;
	}

	return true;
}

KopeteLibraryInfo LibraryLoader::getInfo(const QString &spec) const
{
	QMap<QString, KopeteLibraryInfo>::iterator cached = m_cachedInfo.find(spec);
	if (cached != m_cachedInfo.end() )
		return *cached;

	KopeteLibraryInfo info;
	QString specPath = ( spec[ 0 ] == '/' ) ? spec : KGlobal::dirs()->findResource( "appdata", spec );
	if( !QFile::exists( specPath ) )
		return info;

	KSimpleConfig file( specPath );
	if( spec.find( '/' ) >= 0 )
		info.specfile = KURL( spec ).fileName();
	else
		info.specfile = spec;

	file.setGroup( QString::fromLatin1( "Desktop Entry" ) );

	info.filename = file.readEntry( "X-KDE-Library" );
	info.author   = file.readEntry( "X-Kopete-Author" );
	info.site     = file.readEntry( "X-Kopete-Site" );
	info.email    = file.readEntry( "X-Kopete-Email" );
	info.type     = file.readEntry( "ServiceTypes" );
	info.name     = file.readEntry( "Name" );
	info.comment  = file.readEntry( "Comment" );
	info.license  = file.readEntry( "X-Kopete-License" );
	info.icon     = file.readEntry( "Icon" );

	m_cachedInfo[ spec ] = info;
	return info;
}

bool LibraryLoader::isLoaded( const QString &spec ) const
{
	KopetePlugin *p = mLibHash[ spec ];
	return p;
}

void LibraryLoader::setModules(const QStringList &mods)
{
	KConfig *config=KGlobal::config();
	config->setGroup("");
	config->writeEntry("Plugins", mods);
	KGlobal::config()->sync();
}

QValueList<KopeteLibraryInfo> LibraryLoader::available() const
{
	QValueList<KopeteLibraryInfo> items;
	QStringList files = KGlobal::dirs()->findAllResources( "appdata", QString::fromLatin1( "*.desktop" ), false, true );
	for( QStringList::Iterator i=files.begin(); i!=files.end(); ++i )
		items.append(getInfo(*i));

	return items;
}

bool LibraryLoader::loadPlugin( const QString &spec )
{
	KopetePlugin *plugin = mLibHash[ spec ];
	if( !plugin )
	{
		KopeteLibraryInfo info = getInfo( spec );
		if( info.specfile != spec )
			return false;

		// Get the library loader instance
		KLibLoader *loader = KLibLoader::self();

		KLibrary *lib = loader->library( QFile::encodeName( info.filename) );
		if( !lib )
		{
			kdDebug(14010) << "LibraryLoader::loadPlugin: Error while loading plugin: " << loader->lastErrorMessage() << endl;
			return false;
		}
		plugin = KParts::ComponentFactory::createInstanceFromFactory<KopetePlugin> ( lib->factory(), this );
		mLibHash.insert( spec, plugin );

		connect( plugin, SIGNAL( destroyed( QObject * ) ),
			SLOT( slotPluginDestroyed( QObject * ) ) );

		// Automatically load the i18n catalogue for the plugin
		KGlobal::locale()->insertCatalogue( info.filename );

		plugin->init();

		m_addressBookFields.insert( plugin, plugin->addressBookFields() );

		kdDebug(14010) << "LibraryLoader::loadPlugin: Successfully loaded plugin '" << spec << "'."<< endl;

		emit pluginLoaded( plugin );
		return true;
	}
	else
	{
		kdDebug(14010) << "LibraryLoader::loadPlugin: Plugin '" << spec << "' is already loaded!" << endl;
		return false;
	}
}

bool LibraryLoader::remove( const QString &spec )
{
	KopetePlugin *plugin = mLibHash[ spec ];
	if( !plugin )
		return false;

	remove( plugin );
	return true;
}

bool LibraryLoader::remove( KopetePlugin *p )
{
	if( !p )
		return false;

	kdDebug(14010) << "LibraryLoader::remove: Removing plugin: " << p->pluginId() << endl;

	// Added by Duncan 20/01/2002
	// We need to call unload function for the plugin
	p->unload();

	delete p;

	return true;
}

void LibraryLoader::slotPluginDestroyed( QObject *o )
{
	m_addressBookFields.remove( static_cast<KopetePlugin *>( o ) );

	QDictIterator<KopetePlugin> it( mLibHash );
	for( ; it.current() ; ++it )
	{
		if( it.current() == o )
		{
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
	for( QDictIterator<KopetePlugin> i( mLibHash ); i.current(); ++i )
	{
		if (getInfo(i.currentKey()).name==name)
			return (*i);
	}
	return 0L;
}

KopetePlugin* LibraryLoader::searchByID( const QString &Id )
{
	QValueList<KopeteLibraryInfo> l = loaded();

	for ( QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i )
	{
		KopetePlugin *tmp_plug = mLibHash[ ( *i ).specfile ];
		if( QString::fromLatin1( tmp_plug->pluginId() ) == Id )
			return tmp_plug;
	}
	return NULL;
}

QString LibraryLoader::pluginName(KopetePlugin *plugin)
{
	for( QDictIterator<KopetePlugin> i( mLibHash ); i.current(); ++i )
	{
		if (i.current() == plugin)
			return getInfo(i.currentKey()).name;
	}
	return QString::fromLatin1( "ERROR plugin unknown" );
}

QString LibraryLoader::pluginIcon( const QString &pluginId ) const
{
	QDictIterator<KopetePlugin> i( mLibHash );
	for( ; i.current(); ++i )
	{
		if( i.currentKey() == pluginId );
			return getInfo( i.currentKey() ).icon;
	}

	return QString::null;
}

#include <pluginloader.moc>

// vim: set noet ts=4 sts=4 sw=4:

