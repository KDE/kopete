/*
    kopeteplugin.cpp - Kopete Plugin API

    Copyright (c) 2001-2002 by Duncan Mac-Vicar P. <duncan@kde.org>

    Copyright (c) 2002 by the Kopete developers    <kopete-devel@kde.org>

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

#include <ksettings/dispatcher.h>

class KopetePluginPrivate
{
public:
	QStringList addressBookFields;
	QString indexField;
};

Kopete::Plugin::Plugin( KInstance *instance, QObject *parent, const char *name )
: QObject( parent, name ), KXMLGUIClient()
{
	d = new KopetePluginPrivate;

	setInstance( instance );
	KSettings::Dispatcher::self()->registerInstance( instance, this, SIGNAL( settingsChanged() ) );
}

Kopete::Plugin::~Plugin()
{
	delete d;
}

QString Kopete::Plugin::pluginId() const
{
	return QString::fromLatin1( className() );
}

QString Kopete::Plugin::displayName() const
{
	return Kopete::PluginManager::self()->pluginName( this );
}

QString Kopete::Plugin::pluginIcon() const
{
	return Kopete::PluginManager::self()->pluginIcon( this );
}

void Kopete::Plugin::deserialize( Kopete::MetaContact * /* metaContact */,
	const QMap<QString, QString> & /* stream */ )
{
	// Do nothing in default implementation
}

QStringList Kopete::Plugin::addressBookFields() const
{
	return d->addressBookFields;
}

QString Kopete::Plugin::addressBookIndexField() const
{
	return d->indexField;
}

void Kopete::Plugin::addAddressBookField( const QString &field, AddressBookFieldAddMode mode )
{
	d->addressBookFields.append( field );
	if( mode == MakeIndexField )
		d->indexField = field;
}

QPtrList<KAction> *Kopete::Plugin::customChatWindowPopupActions( const Kopete::Message &, DOM::Node & )
{
	return 0L;
}

void Kopete::Plugin::aboutToUnload()
{
	// Just make the unload synchronous by default
	emit readyForUnload();
}

#include "kopeteplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

