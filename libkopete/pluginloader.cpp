/*
    pluginloader.cpp - Kopete Plugin Loader

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

#include "pluginloader.h"

#include <qapplication.h>
#include <qfile.h>
#include <qregexp.h>

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

struct KopeteLibraryInfo
{
	QString specfile;
	QString filename;
	QString type;
	QString name;
	QString icon;
	QString messagingProtocol;
};

class KopetePluginManagerPrivate
{
public:
	// All available plugins, regardless of category, and loaded or not
	QValueList<KPluginInfo *> plugins;
};

bool operator ==(const KopeteLibraryInfo &, const KopeteLibraryInfo &);

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

LibraryLoader* LibraryLoader::self()
{
	if ( !s_pluginLoader )
		sd.setObject( s_pluginLoader, new LibraryLoader() );

	return s_pluginLoader;
}

LibraryLoader::LibraryLoader()
: QObject( qApp )
{
	d = new KopetePluginManagerPrivate;

	KSettings::Dispatcher::self()->registerInstance( KGlobal::instance(), this, SLOT( slotKopeteSettingsChanged() ) );

	d->plugins = KPluginInfo::fromServices( KTrader::self()->query( QString::fromLatin1( "Kopete/Plugin" ) ) );
}

LibraryLoader::~LibraryLoader()
{
	QDictIterator<KopetePlugin> i( m_loadedPlugins );
	while( i.current() )
	{
		// Remove causes the iterator to auto-increment, so
		// only increment explicitly when not removing
		if( getInfo( i.currentKey() ).type != QString::fromLatin1( "Kopete/Protocol" ) )
			delete i.current();
		else
			++i;
	}

	i.toFirst();
	while( i.current() )
		delete i.current();

	delete d;

	kdDebug(14010) << k_funcinfo << "All plugins removed" << endl;
}

QValueList<KPluginInfo *> LibraryLoader::availablePlugins( const QString &category ) const
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

QPtrList<KopetePlugin> LibraryLoader::plugins() const
{
	QPtrList<KopetePlugin> list;
	QDictIterator<KopetePlugin> i( m_loadedPlugins );
	for( ; i.current(); ++i )
		list.append( i.current() );

	return list;
}

bool LibraryLoader::loadAll()
{
	// FIXME: We need session management here - Martijn

	slotKopeteSettingsChanged();

	return true;
}

KopeteLibraryInfo LibraryLoader::getInfo( const QString &spec ) const
{
	QMap<QString, KopeteLibraryInfo>::iterator cached = m_cachedInfo.find( spec );
	if ( cached != m_cachedInfo.end() )
		return *cached;

	KopeteLibraryInfo info;
	QString specPath = ( spec[ 0 ] == '/' ) ? spec : KGlobal::dirs()->findResource( "services", spec + QString::fromLatin1( ".desktop" ) );
	if( !QFile::exists( specPath ) )
		return info;

	KSimpleConfig file( specPath );
	if( spec.find( '/' ) >= 0 )
		info.specfile = KURL( spec ).fileName();
	else
		info.specfile = spec + QString::fromLatin1( ".desktop" );

	file.setGroup( QString::fromLatin1( "Desktop Entry" ) );

	info.filename = file.readEntry( "X-KDE-Library" );
	info.type     = file.readEntry( "ServiceTypes" );
	info.name     = file.readEntry( "Name" );
	info.icon     = file.readEntry( "Icon" );
	info.messagingProtocol = file.readEntry( "X-Kopete-Messaging-Protocol" );

	if ( info.type != QString::fromLatin1( "Kopete/Plugin" ) && info.type != QString::fromLatin1( "Kopete/Protocol" ) )
		return KopeteLibraryInfo();

	m_cachedInfo[ spec ] = info;
	return info;
}

KopetePlugin *LibraryLoader::loadPlugin( const QString &spec_ )
{
	QString spec = spec_.lower().remove( QRegExp( QString::fromLatin1( ".desktop$" ) ) );

	kdDebug( 14010 ) << k_funcinfo << spec << endl;

	KopetePlugin *plugin = m_loadedPlugins[ spec ];
	if(plugin)
		return plugin;

	int error = 0;
	plugin = KParts::ComponentFactory::createInstanceFromQuery<KopetePlugin>(
		QString::fromLatin1( "Kopete/Plugin" ),
		QString::fromLatin1( "[X-KDE-PluginInfo-Name]=='kopete_%1'" ).arg( spec ), this, 0, QStringList(), &error );

	if( plugin )
	{
		m_loadedPlugins.insert(spec, plugin);

		connect(plugin, SIGNAL(destroyed(QObject *)),
			this, SLOT(slotPluginDestroyed(QObject *)));

		m_addressBookFields.insert(plugin, plugin->addressBookFields());

		kdDebug(14010) << k_funcinfo << "Successfully loaded plugin '" << spec << "'"<< endl;

		emit pluginLoaded(plugin);
	}
	else
	{
		switch(error)
		{
			case KParts::ComponentFactory::ErrNoServiceFound:
				kdDebug(14010) << k_funcinfo << "No service implementing the given mimetype "
					<< "and fullfilling the given constraint expression can be found." << endl;
				break;

			case KParts::ComponentFactory::ErrServiceProvidesNoLibrary:
				kdDebug(14010) << "the specified service provides no shared library." << endl;
				break;

			case KParts::ComponentFactory::ErrNoLibrary:
				kdDebug(14010) << "the specified library could not be loaded." << endl;
				break;

			case KParts::ComponentFactory::ErrNoFactory:
				kdDebug(14010) << "the library does not export a factory for"
					<< " creating components." << endl;
				break;

			case KParts::ComponentFactory::ErrNoComponent:
				kdDebug(14010) << "the factory does not support creating components of the"
					<< " specified type." << endl;
				break;
		}

		kdDebug(14010) << k_funcinfo << "Loading plugin '" << spec <<
			"' failed, KLibLoader reported error: '" << endl <<
			KLibLoader::self()->lastErrorMessage() << "'" << endl;
	}

	return plugin;
}

bool LibraryLoader::remove( const QString &spec )
{
	KopetePlugin *plugin = m_loadedPlugins[ spec ];
	if( !plugin )
		return false;

	delete plugin;

	return true;
}

void LibraryLoader::slotPluginDestroyed( QObject *o )
{
	m_addressBookFields.remove( static_cast<KopetePlugin *>( o ) );

	QDictIterator<KopetePlugin> it( m_loadedPlugins );
	for( ; it.current() ; ++it )
	{
		if( it.current() == o )
		{
			m_loadedPlugins.remove( it.currentKey() );
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
	for( QDictIterator<KopetePlugin> i( m_loadedPlugins ); i.current(); ++i )
	{
		if (getInfo(i.currentKey()).name==name)
			return (*i);
	}
	return 0L;
}

KopetePlugin* LibraryLoader::searchByID( const QString &Id )
{
	kdDebug() << k_funcinfo << Id << endl;

	for ( QDictIterator<KopetePlugin> it( m_loadedPlugins ); it.current(); ++it )
	{
		if ( it.current()->pluginId() == Id )
			return it.current();
	}

	return 0L;
}

QString LibraryLoader::pluginName( const KopetePlugin *plugin ) const
{
	for( QDictIterator<KopetePlugin> i( m_loadedPlugins ); i.current(); ++i )
	{
		if( i.current() == plugin )
			return getInfo( i.currentKey() ).name;
	}
	return QString::fromLatin1( "ERROR: plugin unknown" );
}

QString LibraryLoader::pluginIcon( const KopetePlugin *plugin ) const
{
	for( QDictIterator<KopetePlugin> i( m_loadedPlugins ); i.current(); ++i )
	{
		if( i.current() == plugin )
			return getInfo( i.currentKey() ).icon;
	}
	return QString::fromLatin1( "ERROR: plugin unknown" );
}

void LibraryLoader::slotKopeteSettingsChanged()
{
	kdDebug() << k_funcinfo << endl;

	KConfig *config = KGlobal::config();
	QMap<QString, QString> entries = config->entryMap( QString::fromLatin1( "Plugins" ) );
	QMap<QString, QString>::Iterator it;
	for ( it = entries.begin(); it != entries.end(); ++it )
	{
		QString key = it.key();
		if ( key.endsWith( QString::fromLatin1( "Enabled" ) ) )
		{
			key.setLength( key.length() - 7 );
			kdDebug() << k_funcinfo << "Set " << key << " to " << it.data() << endl;

			// As a workaround until all plugins are ported and this class can be
			// rewritten just call the old API, assuming the key is prefixed with
			// "kopete_" - Martijn
			if ( key.startsWith( QString::fromLatin1( "kopete_" ) ) )
			{
				key.remove( 0, 7 );
				if ( it.data() == QString::fromLatin1( "true" ) )
				{
					if ( !m_loadedPlugins[ key ] )
					{
						loadPlugin( key );

						// FIXME: processEvents is evil, we need to use a queue and
						//        a singleshot timer instead - Martijn
						kapp->processEvents();
					}
				}
				else
				{
					if ( m_loadedPlugins[ key ] )
						delete m_loadedPlugins[ key ];
				}
			}
			else
			{
				kdWarning() << k_funcinfo << "Cannot change settings for plugin '" << key <<
					"' yet. Change the plugin name to have a 'kopete_' prefix or wait until " <<
					"the library loader has been rewritten." << endl;
			}
		}
	}
}

#include "pluginloader.moc"

// vim: set noet ts=4 sts=4 sw=4:

