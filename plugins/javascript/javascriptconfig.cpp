
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "javascriptconfig.h"

#include "javascriptfile.h"

#include "kopeteaccount.h"
#include "kopeteuiglobal.h"

#include <kstandarddirs.h>

#include <kdebug.h>
#include <ksimpleconfig.h>
#include <klocale.h>
#include <kzip.h>
#include <kurl.h>
#include <ktempfile.h>
#include <kdesktopfile.h>
#include <kmessagebox.h>

#include <qregexp.h>

struct JavaScriptConfig::Private
{
	KConfig *config;
	QMap< QString, JavaScriptFile * > scripts;
};

JavaScriptConfig* JavaScriptConfig::m_config = 0L;

JavaScriptConfig *JavaScriptConfig::instance()
{
	if( !m_config )
		m_config = new JavaScriptConfig(0L);
	return m_config;
}

JavaScriptConfig::JavaScriptConfig(QObject *parent)
	: QObject(parent)
	, MimeTypeHandler(false)
	, d(new JavaScriptConfig::Private)
{
	kDebug() << k_funcinfo << endl;

	d->config = new KConfig("javascriptplugin.rc");

	foreach( const QString group, d->config->groupList() )
	{
		if( group.endsWith( "_Script" ) )
		{
			d->config->setGroup( group );
			JavaScriptFile *s = new JavaScriptFile(this);
			s->id = d->config->readEntry("ID", QString::number( time( NULL ) ) );
			s->name = d->config->readEntry("Name", "" );
			s->description = d->config->readEntry("Description", "" );
			s->author = d->config->readEntry("Author", "Unknown" );
			s->version = d->config->readEntry("Version", "Unknown" );
			s->fileName = d->config->readEntry("FileName", "");
			s->accounts = d->config->readListEntry("Accounts");
			QStringList ftns = d->config->readListEntry("Functions");
			for( QStringList::iterator it2 = ftns.begin(); it2 != ftns.end(); ++it2 )
				s->functions.insert( *it2, d->config->readEntry( *it2 + "_Function", "" ) );
			s->immutable = d->config->entryIsImmutable( group );

			d->scripts.insert( s->id, s );
		}
	}

	//Handler for script packages
	registerAsHandler( QString::fromLatin1("application/x-kopete-javascript") );
	registerAsHandler( QString::fromLatin1("application/x-zip") );
}

JavaScriptConfig::~JavaScriptConfig()
{
	apply();
	delete d->config;

	delete d;
}

QValueList<Script*> JavaScriptConfig::allScripts() const
{
	return d->scripts.values();
}

bool JavaScriptConfig::signalsEnabled() const
{
	d->config->setGroup("Global");
	return d->config->readBoolEntry( QString::fromLatin1("SignalsEnabled"), true );
}

void JavaScriptConfig::setSignalsEnabled( bool val )
{
	d->config->setGroup("Global");
	d->config->writeEntry( QString::fromLatin1("signalsEnabled"), val );
}

bool JavaScriptConfig::writeEnabled() const
{
	d->config->setGroup("Global");
	return d->config->readBoolEntry( QString::fromLatin1("writeEnabled"), true );
}

void JavaScriptConfig::setWriteEnabled( bool val )
{
	d->config->setGroup("Global");
	d->config->writeEntry( QString::fromLatin1("writeEnabled"), val );
}

bool JavaScriptConfig::treeEnabled() const
{
	d->config->setGroup("Global");
	return d->config->readBoolEntry( QString::fromLatin1("treeEnabled"), true );
}

void JavaScriptConfig::setTreeEnabled( bool val )
{
	d->config->setGroup("Global");
	d->config->writeEntry( QString::fromLatin1("treeEnabled"), val );
}

bool JavaScriptConfig::factoryEnabled() const
{
	d->config->setGroup("Global");
	return d->config->readBoolEntry( QString::fromLatin1("factoryEnabled"), true );
}

void JavaScriptConfig::setFactoryEnabled( bool val )
{
	d->config->setGroup("Global");
	d->config->writeEntry( QString::fromLatin1("factoryEnabled"), val );
}

void JavaScriptConfig::apply()
{
	for( QMap<QString,JavaScriptFile*>::iterator it = d->scripts.begin(); it != d->scripts.end(); ++it  )
	{
		JavaScriptFile *s = it.data();
		d->config->setGroup( s->id + "_Script" );
		d->config->writeEntry("ID", s->id );
		d->config->writeEntry("Name", s->name );
		d->config->writeEntry("Description", s->description );
		d->config->writeEntry("Author", s->author );
		d->config->writeEntry("Version", s->version );
		d->config->writeEntry("FileName", s->fileName );
		d->config->writeEntry("Functions", s->functions.keys() );
		d->config->writeEntry("Accounts", s->accounts );
		for( QMap<QString,QString>::iterator it2 = s->functions.begin(); it2 != s->functions.end(); ++it2 )
			d->config->writeEntry( it2.key() + "_Function", it2.data() );
	}

	d->config->sync();

	emit changed();
}

JavaScriptFile *JavaScriptConfig::addScript( const QString &fileName, const QString &name, const QString &description,
	const QString &author, const QString &version, const QMap<QString,QString> &functions,
	const QString &id )
{
	JavaScriptFile *s = new JavaScriptFile(this);
	s->id = id;
	s->name = name;
	s->fileName = fileName;
	s->description = description;
	s->author = author;
	s->version = version;
	s->functions = functions;
	s->immutable = false;
	d->scripts.insert( s->id, s );

	return s;
}

JavaScriptFile* JavaScriptConfig::script( const QString &id )
{
	return d->scripts[id];
}

void JavaScriptConfig::removeScript( const QString &id )
{
	d->scripts.remove( id );
}

QList<JavaScriptFile *> JavaScriptConfig::scriptsFor( Kopete::Account *account )
{
	QString key;
	if( account )
		key = account->accountId();
	else
		key = "GLOBAL_SCRIPT";

	QList<JavaScriptFile *> retVal;
	for( QMap<QString,JavaScriptFile*>::iterator it = d->scripts.begin(); it != d->scripts.end(); ++it )
	{
		kDebug() << it.data()->accounts << endl;
		if( it.data()->accounts.contains( key ) || it.data()->accounts.contains( "GLOBAL_SCRIPT" ) )
			retVal.append( it.data() );
	}

	return retVal;
}

void JavaScriptConfig::setScriptEnabled( Kopete::Account *account, const QString &script, bool enabled )
{
	QString key;
	if( account )
		key = account->accountId();
	else
		key = "GLOBAL_SCRIPT";

	kDebug() << k_funcinfo << key << " " << script << " " << enabled << endl;

	JavaScriptFile *scriptPtr = d->scripts[script];
	if( scriptPtr )
	{
		if( scriptPtr->accounts.contains( script ) )
		{
			if( !enabled )
				scriptPtr->accounts.remove( key );
		}
		else if( enabled )
		{
			scriptPtr->accounts.append( key );
		}
	}
	else
	{
		kdError() << k_funcinfo << script << " is not a valid script!" << endl;
	}
}

void JavaScriptConfig::installPackage( const QString &archiveName, bool &retVal )
{
	retVal = false;
	QString localScriptsDir( KStandardDirs::locateLocal("data", QLatin1String("kopete/scripts")) );

	if(localScriptsDir.isEmpty())
	{
		KMessageBox::queuedMessageBox(
			Kopete::UI::Global::mainWidget(),
			KMessageBox::Error, i18n("Could not find suitable place " \
			"to install scripts into.")
		);
		return;
	}

	KZip archive( archiveName );
	if ( !archive.open(IO_ReadOnly) )
	{
		KMessageBox::queuedMessageBox(
			Kopete::UI::Global::mainWidget(),
			KMessageBox::Error,
			i18n("Could not open \"%1\" for unpacking.").arg(archiveName )
		);
		return;
	}

	const KArchiveDirectory* rootDir = archive.directory();
	QStringList desktopFiles = rootDir->entries().grep( QRegExp( QString::fromLatin1("^(.*)\\.desktop$") ) );

	if( desktopFiles.size() == 1 )
	{
		const KArchiveFile *manifestEntry = static_cast<const KArchiveFile*>( rootDir->entry( desktopFiles[0] ) );
		if( manifestEntry )
		{
			KTempFile manifest;
			manifest.setAutoDelete(true);
			manifestEntry->copyTo( manifest.name() );
			KDesktopFile manifestFile( manifest.name() );

			if( manifestFile.readType() == QString::fromLatin1("KopeteScript") )
			{
				QString id = QString::number( time( NULL ) );
				QString dir = localScriptsDir + QString::fromLatin1("/") + id;
				rootDir->copyTo( dir );
				KSimpleConfig conf( dir + QString::fromLatin1("/") + manifestFile.readURL() );

				QMap<QString,QString> functions;
				QStringList ftns = conf.readListEntry("Functions");

				for( QStringList::iterator it = ftns.begin(); it != ftns.end(); ++it )
					functions.insert( *it, conf.readEntry( *it + "_Function" ) );

				addScript( conf.readEntry("FileName"), conf.readEntry("Name"),
					conf.readEntry("Description"), conf.readEntry("Author"),
					conf.readEntry("Version"), functions, id );

				retVal = true;
				return;
			}
		}
	}

	KMessageBox::queuedMessageBox(
		Kopete::UI::Global::mainWidget(),
		KMessageBox::Error, i18n("The file \"%1\" is not a valid Kopete script package.")
		.arg(archiveName)
	);
}

void JavaScriptConfig::handleURL( const QString &, const KUrl &url ) const
{
	bool retVal = false;
	const_cast<JavaScriptConfig*>(this)->installPackage( url.path(), retVal );
}

#include "javascriptconfig.moc"

