
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kglobal.h>
#include <kconfig.h>
#include <kdebug.h>

#include "kopeteaccount.h"

#include "javascriptconfig.h"

struct JavaScriptConfigPrivate
{
	KConfig *config;
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
	d = new JavaScriptConfigPrivate;
	d->config = KGlobal::config();
	d->config->setGroup( QString::fromLatin1("JavaScriptPlugin") );
}

JavaScriptConfig::~JavaScriptConfig()
{
	delete d;
}

bool JavaScriptConfig::signalsEnabled() const
{
	return d->config->readBoolEntry( QString::fromLatin1("SignalsEnabled"), true );
}

void JavaScriptConfig::setSignalsEnabled( bool val )
{
	d->config->writeEntry( QString::fromLatin1("signalsEnabled"), val );
	d->config->sync();

	emit changed();
}

bool JavaScriptConfig::writeEnabled() const
{
	return d->config->readBoolEntry( QString::fromLatin1("writeEnabled"), true );
}

void JavaScriptConfig::setWriteEnabled( bool val )
{
	d->config->writeEntry( QString::fromLatin1("writeEnabled"), val );
	d->config->sync();

	emit changed();
}

bool JavaScriptConfig::treeEnabled() const
{
	return d->config->readBoolEntry( QString::fromLatin1("treeEnabled"), true );
}

void JavaScriptConfig::setTreeEnabled( bool val )
{
	d->config->writeEntry( QString::fromLatin1("treeEnabled"), val );
	d->config->sync();

	emit changed();
}

bool JavaScriptConfig::factoryEnabled() const
{
	return d->config->readBoolEntry( QString::fromLatin1("factoryEnabled"), true );
}

void JavaScriptConfig::setFactoryEnabled( bool val )
{
	d->config->writeEntry( QString::fromLatin1("factoryEnabled"), val );
	d->config->sync();

	emit changed();
}

QString JavaScriptConfig::script( KopeteAccount *account, int type )
{
	kdDebug() << k_funcinfo << "Getting script for " << account << " type " << type << endl;
	if( account )
	{
		return d->config->readEntry(
			QString::fromLatin1("script_") + account->accountId() + QString::fromLatin1("_")
				+ QString::number(type), QString::null
		);
	}
	else
	{
		return d->config->readEntry( QString::fromLatin1("Global_") + QString::number(type), QString::null );
	}
}

void JavaScriptConfig::setScript( KopeteAccount *account, int type, const QString &script )
{
	kdDebug() << k_funcinfo << "Setting script for " << account <<  " type " << type << " to " << script << endl;
	if( account )
	{
		d->config->writeEntry(
			QString::fromLatin1("script_") + account->accountId() + QString::fromLatin1("_")
				+ QString::number(type), script
		);
	}
	else
	{
		d->config->writeEntry( QString::fromLatin1("Global_") + QString::number(type), script );
	}

	d->config->sync();
	emit changed();
}

#include "javascriptconfig.moc"

