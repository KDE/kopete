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

KopetePlugin::KopetePlugin( KInstance *instance, QObject *parent, const char *name )
: QObject( parent, name ) , KXMLGUIClient()
{
	setInstance( instance );
	KSettings::Dispatcher::self()->registerInstance( instance, this, SIGNAL( settingsChanged() ) );
}

KopetePlugin::~KopetePlugin()
{
}

QString KopetePlugin::pluginId() const
{
	return QString::fromLatin1( className() );
}

QString KopetePlugin::displayName() const
{
	return KopetePluginManager::self()->pluginName( this );
}

QString KopetePlugin::pluginIcon() const
{
	return KopetePluginManager::self()->pluginIcon( this );
}

void KopetePlugin::deserialize( KopeteMetaContact * /* metaContact */,
	const QMap<QString, QString> & /* stream */ )
{
	// Do nothing in default implementation
}

QStringList KopetePlugin::addressBookFields() const
{
	return m_addressBookFields;
}

QString KopetePlugin::addressBookIndexField() const
{
	return m_indexField;
}

void KopetePlugin::addAddressBookField( const QString &field, AddressBookFieldAddMode mode )
{
	m_addressBookFields.append( field );
	if( mode == MakeIndexField )
		m_indexField = field;
}

KActionCollection *KopetePlugin::customChatWindowPopupActions( const KopeteMessage &, DOM::Node & )
{
	return 0L;
}

#include "kopeteplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

