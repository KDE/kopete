
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdatastream.h>

#include <kglobal.h>
#include <kconfig.h>
#include <kdebug.h>

#include "kopeteaccount.h"

#include "javascriptconfig.h"

class JavaScriptConfigPrivate
{
	public:
	JavaScriptConfigPrivate()
	{
		scripts.setAutoDelete(true);
	}

	KConfig *config;
	QDict< Script > scripts;
};

JavaScriptConfig* JavaScriptConfig::m_config = 0L;

JavaScriptConfig *JavaScriptConfig::instance()
{
	if( !m_config )
		m_config = new JavaScriptConfig(0L,"");
	return m_config;
}

JavaScriptConfig::JavaScriptConfig( QObject *parent, const char* name ) : QObject( parent, name )
{
	kdDebug() << k_funcinfo << endl;

	d = new JavaScriptConfigPrivate;
	d->config = new KConfig("javascriptplugin.rc");
	d->scripts.setAutoDelete( true );

	QStringList groups = d->config->groupList();
	for( QStringList::iterator it = groups.begin(); it != groups.end(); ++it )
	{
		d->config->setGroup( *it );
		Script *s = new Script;
		s->name = *it;
		s->description = d->config->readEntry("Description", "" );
		s->author = d->config->readEntry("Author", "Unknown" );
		s->script = d->config->readEntry("Script", "");
		s->immutable = d->config->entryIsImmutable( *it );

		QCString accountMap = d->config->readEntry("Accounts", "" ).latin1();
		if( !accountMap.isEmpty() )
		{
			QDataStream stream( accountMap, IO_ReadOnly );
			stream >> s->accounts;
		}
	}
}

JavaScriptConfig::~JavaScriptConfig()
{
	apply();
	delete d->config;
	delete d;
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

	emit changed();
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

	emit changed();
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

	emit changed();
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

	emit changed();
}

void JavaScriptConfig::apply()
{
	for( QDictIterator<Script> it( d->scripts ); it.current(); ++it )
	{
		Script *s = it.current();
		d->config->setGroup( s->name );
		d->config->writeEntry("Description", s->description );
		d->config->writeEntry("Author", s->author );
		d->config->writeEntry("Script", s->script );

		QCString accountMap;
		QDataStream stream( accountMap, IO_WriteOnly );
		stream << s->accounts;
		d->config->writeEntry("Accounts", QString::fromLatin1( accountMap ) );
	}

	d->config->sync();
}

Script* JavaScriptConfig::addScript( const QString &name, const QString &description,
	const QString &author, const QString &script )
{
	Script *s = new Script;
	s->name = name;
	s->description = description;
	s->author = author;
	s->script = script;
	s->immutable = false;
	d->scripts.insert( s->name, s );

	emit changed();

	return s;
}

Script* JavaScriptConfig::script( const QString &name )
{
	return d->scripts[name];
}

void JavaScriptConfig::writeScript( const QString &name, const QString &contents )
{
	Script *scriptPtr = d->scripts[name];
	if( scriptPtr )
	{
		scriptPtr->script = contents;
		emit changed();
	}
	else
	{
		kdError() << k_funcinfo << name << " is not a valid script!" << endl;
	}
}

void JavaScriptConfig::removeScript( const QString &name )
{
	d->scripts.remove( name );

	emit changed();
}

QStringList JavaScriptConfig::scriptsFor( KopeteAccount *account, int type )
{
	QString key;
	if( account )
		key = account->accountId();
	else
		key = "GLOBAL_SCRIPT";

	QStringList retVal;

	for( QDictIterator<Script> it( d->scripts ); it.current(); ++it )
	{
		Script *scriptPtr = it.current();
		QString types = scriptPtr->accounts[ key ];
		if( types.contains( QString::number( type ) ) )
		{
			retVal.append( scriptPtr->script );
		}
	}

	return retVal;
}

QStringList JavaScriptConfig::scriptNamesFor( KopeteAccount *account, int type )
{
	QString key;
	if( account )
		key = account->accountId();
	else
		key = "GLOBAL_SCRIPT";

	QStringList retVal;

	for( QDictIterator<Script> it( d->scripts ); it.current(); ++it )
	{
		Script *scriptPtr = it.current();
		QString types = scriptPtr->accounts[ key ];
		if( types.contains( QString::number( type ) ) )
		{
			retVal.append( scriptPtr->name );
		}
	}

	return retVal;
}

QStringList JavaScriptConfig::accountsFor( int type, const QString &script )
{
	QStringList retVal;

	for( QDictIterator<Script> it( d->scripts ); it.current(); ++it )
	{
		Script *scriptPtr = it.current();
		for( QMap<QString,QString>::iterator it = scriptPtr->accounts.begin();
			it != scriptPtr->accounts.end(); ++it )
		{
			if( it.data().contains( QString::number( type ) ) )
			{
				retVal.append( it.key() );
			}
		}
	}

	return retVal;
}

void JavaScriptConfig::setScriptEnabled( KopeteAccount *account, int type, const QString &script, bool enabled )
{
	QString key;
	if( account )
		key = account->accountId();
	else
		key = "GLOBAL_SCRIPT";

	kdDebug() << k_funcinfo << key << " " << script << " " << enabled << endl;

	Script *scriptPtr = d->scripts[script];
	if( scriptPtr )
	{
		QString types = scriptPtr->accounts[ key ];
		if( !types.contains( QString::number( type ) ) )
		{
			scriptPtr->accounts[ key ] += QString::number( type );
			emit changed();
		}
	}
	else
	{
		kdError() << k_funcinfo << script << " is not a valid script!" << endl;
	}
}

#include "javascriptconfig.moc"

