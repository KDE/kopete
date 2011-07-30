
/*
    kopeteplugindataobject.cpp - Kopete Plugin Data Object

    Copyright (c) 2003-2005 by Olivier Goffart       <ogoffart@kde.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetecontactlistelement.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>

#include "kopeteplugin.h"

namespace Kopete {

class ContactListElement::Private
{
public:
	ContactListElement::PluginDataMap pluginData;
	QMap<QString, ContactListElement::ContactDataList> pluginContactData;
	ContactListElement::IconMap icons;
	bool useCustomIcon;
	bool loading;
};

ContactListElement::ContactListElement( QObject *parent )
: PropertyContainer( parent ), d(new Private())
{
	d->useCustomIcon = false;
	d->loading = false;
#if 0  //TODO
	connect( Kopete::Global::onlineStatusIconCache(), SIGNAL(iconsChanged()), SIGNAL(iconAppearanceChanged()) );
#endif
}

ContactListElement::~ContactListElement()
{
	delete d;
}

void ContactListElement::setLoading( bool value )
{
	d->loading = value;
}

bool ContactListElement::loading() const
{
	return d->loading;
}

void ContactListElement::setPluginData( Plugin *plugin, const QMap<QString, QString> &pluginData )
{
	setPluginData( plugin->pluginId(), pluginData );
}

void ContactListElement::setPluginData( const QString &pluginId, const QMap<QString, QString> &pluginData )
{
	if ( pluginData.isEmpty() )
	{
		d->pluginData.remove( pluginId );
		return;
	}
	
	d->pluginData[ pluginId ] = pluginData;
	
	emit pluginDataChanged();
}

void ContactListElement::setPluginData( Plugin *p, const QString &key, const QString &value )
{
	d->pluginData[ p->pluginId() ][ key ] = value;

	emit pluginDataChanged();
}

QMap<QString, QString> ContactListElement::pluginData( Plugin *plugin ) const
{
	if ( !d->pluginData.contains( plugin->pluginId() ) )
		return QMap<QString, QString>();

	return d->pluginData[ plugin->pluginId() ];
}

QString ContactListElement::pluginData( Plugin *plugin, const QString &key ) const
{
	if ( !d->pluginData.contains( plugin->pluginId() ) || !d->pluginData[ plugin->pluginId() ].contains( key ) )
		return QString();

	return d->pluginData[ plugin->pluginId() ][ key ];
}

const ContactListElement::PluginDataMap ContactListElement::pluginData() const
{
	return d->pluginData;
}

QMap<QString, ContactListElement::ContactDataList > ContactListElement::pluginContactData() const
{
	return d->pluginContactData;
}

ContactListElement::ContactDataList ContactListElement::pluginContactData( Plugin *plugin ) const
{
	if ( !d->pluginContactData.contains( plugin->pluginId() ) )
		return ContactDataList();

	return d->pluginContactData[ plugin->pluginId() ];
}

void ContactListElement::clearPluginContactData()
{
	d->pluginContactData.clear();
}

void ContactListElement::setPluginContactData( Plugin *plugin, const ContactListElement::ContactDataList &dataList )
{
	QString pluginId = plugin->pluginId();
	if ( dataList.isEmpty() )
	{
		d->pluginContactData.remove( pluginId );
		return;
	}

	d->pluginContactData[ pluginId ] = dataList;

	emit pluginDataChanged();
}

void ContactListElement::appendPluginContactData( const QString &pluginId, const ContactData &data )
{
	if ( data.isEmpty() )
	{
		d->pluginContactData.remove( pluginId );
		return;
	}

	d->pluginContactData[ pluginId ].append( data );

	emit pluginDataChanged();
}

const ContactListElement::IconMap ContactListElement::icons() const
{
	return d->icons;
}

QString ContactListElement::icon( ContactListElement::IconState state ) const
{
	if ( d->icons.contains( state ) )
		return d->icons[state];

	return d->icons[ None ];
}

void ContactListElement::setIcon( const QString& icon , ContactListElement::IconState state )
{
	if ( icon.isNull() )
		d->icons.remove( state );
	else
		d->icons[ state ] = icon;

	emit iconChanged( state, icon );
	emit iconAppearanceChanged();
}

bool ContactListElement::useCustomIcon() const
{
	return d->useCustomIcon;
}

void ContactListElement::setUseCustomIcon( bool useCustomIcon )
{
	if ( d->useCustomIcon != useCustomIcon )
	{
		d->useCustomIcon = useCustomIcon;
		emit useCustomIconChanged( useCustomIcon );
	}
}

} //END namespace Kopete

#include "kopetecontactlistelement.moc"



