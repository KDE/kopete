/*
    kopeteplugin.cpp - Kopete Plugin API

    Copyright (c) 2001-2002 by Duncan Mac-Vicar P. <duncan@kde.org>

    Copyright (c) 2002 by the Kopete developers    <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteplugin.h"
#include "pluginloader.h"

KopetePlugin::KopetePlugin( QObject *parent, const char *name )
: QObject( parent, name )
{
}

KopetePlugin::~KopetePlugin()
{
}

void KopetePlugin::init()
{
}

bool KopetePlugin::unload()
{
	emit unloading();
	return true;
}

const char *KopetePlugin::id() const
{
	return this->className();
}

void KopetePlugin::deserialize( KopeteMetaContact * /* metaContact */,
	const QStringList & /* stream */ )
{
	// Do nothing in default implementation
}

void KopetePlugin::addressBookFieldChanged( KopeteMetaContact * /* c */,
	const QString & /* key */ )
{
	// Do nothing in default implementation
}

QStringList KopetePlugin::addressBookFields() const
{
	// Do nothing in default implementation
	return QStringList();
}

#include "kopeteplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

