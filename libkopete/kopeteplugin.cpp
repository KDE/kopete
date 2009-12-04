/*
    kopeteplugin.cpp - Kopete Plugin API

    Copyright (c) 2001-2002 by Duncan Mac-Vicar P. <duncan@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart  <ogoffart @tiscalinet.be>

    Copyright (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteplugin.h"
#include "kopetepluginmanager.h"

#include <kplugininfo.h>
#include <ksettings/dispatcher.h>

namespace Kopete {

class Plugin::Private
{
public:
	QStringList addressBookFields;
	QString indexField;
};

Plugin::Plugin( const KComponentData &instance, QObject *parent )
: QObject( parent ), KXMLGUIClient(), d(new Private)
{
	setComponentData( instance );
	KSettings::Dispatcher::registerComponent( instance, this, "settingsChanged" );
}

Plugin::~Plugin()
{
	delete d;
}

QString Plugin::pluginId() const
{
	return QString::fromLatin1( metaObject()->className() );
}


QString Plugin::displayName() const
{
	return pluginInfo().isValid() ? pluginInfo().name() : QString();
}

QString Plugin::pluginIcon() const
{
	return pluginInfo().isValid() ? pluginInfo().icon() : QString();
}


KPluginInfo Plugin::pluginInfo() const 
{
	return PluginManager::self()->pluginInfo( this );
}

bool Plugin::showCloseWindowMessage()
{
	return false;
}

bool Plugin::shouldExitOnclose()
{
	return true;
}

void Plugin::aboutToUnload()
{
	// Just make the unload synchronous by default
	emit readyForUnload();
}


void Plugin::deserialize( MetaContact * /* metaContact */,
	const QMap<QString, QString> & /* stream */ )
{
	// Do nothing in default implementation
}



void Kopete::Plugin::addAddressBookField( const QString &field, AddressBookFieldAddMode mode )
{
	d->addressBookFields.append( field );
	if( mode == MakeIndexField )
		d->indexField = field;
}

QStringList Kopete::Plugin::addressBookFields() const
{
	return d->addressBookFields;
}

QString Kopete::Plugin::addressBookIndexField() const
{
	return d->indexField;
}

} //END namespace Kopete

#include "kopeteplugin.moc"

