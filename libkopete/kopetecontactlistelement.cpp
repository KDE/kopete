/*
    kopeteplugindataobject.cpp - Kopete Plugin Data Object

    Copyright (c) 2003-2005 by Olivier Goffart       <ogoffart @ kde.org>
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
	ContactListElement::IconMap icons;
	bool useCustomIcon;
	bool loading;
};

ContactListElement::ContactListElement( QObject *parent )
: QObject( parent )
{
	d = new Private;

	d->useCustomIcon = false;
	d->loading = false;
#if 0  //TODO
	connect( Kopete::Global::onlineStatusIconCache(), SIGNAL( iconsChanged() ), SIGNAL( iconAppearanceChanged() ) );
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
		return QString::null;

	return d->pluginData[ plugin->pluginId() ][ key ];
}

const ContactListElement::PluginDataMap ContactListElement::pluginData() const
{
	return d->pluginData;
}

const QList<QDomElement> ContactListElement::toXML()
{
	QDomDocument pluginData;
	QList<QDomElement> pluginNodes;
	pluginData.appendChild( pluginData.createElement( QLatin1String( "plugin-data" ) ) );

	if ( !d->pluginData.isEmpty() )
	{
		QMap<QString, QMap<QString, QString> >::ConstIterator pluginIt;
		for ( pluginIt = d->pluginData.begin(); pluginIt != d->pluginData.end(); ++pluginIt )
		{
			QDomElement pluginElement = pluginData.createElement( QLatin1String( "plugin-data" ) );
			pluginElement.setAttribute( QLatin1String( "plugin-id" ), pluginIt.key()  );

			QMap<QString, QString>::ConstIterator it;
			for ( it = pluginIt.value().begin(); it != pluginIt.value().end(); ++it )
			{
				QDomElement pluginDataField = pluginData.createElement( QLatin1String( "plugin-data-field" ) );
				pluginDataField.setAttribute( QLatin1String( "key" ), it.key()  );
				pluginDataField.appendChild( pluginData.createTextNode(  it.value()  ) );
				pluginElement.appendChild( pluginDataField );
			}

			pluginData.documentElement().appendChild( pluginElement );
			pluginNodes.append( pluginElement );
		}
	}
	if ( !d->icons.isEmpty() )
	{
		QDomElement iconsElement = pluginData.createElement( QLatin1String( "custom-icons" ) );
		iconsElement.setAttribute( QLatin1String( "use" ), d->useCustomIcon ?  QLatin1String( "1" ) : QLatin1String( "0" ) );

		for ( QMap<IconState, QString >::ConstIterator it = d->icons.begin(); it != d->icons.end(); ++it )
		{
			QDomElement iconElement = pluginData.createElement( QLatin1String( "icon" ) );
			QString stateStr;
			switch ( it.key() )
			{
			case Open:
				stateStr = QLatin1String( "open" );
				break;
			case Closed:
				stateStr = QLatin1String( "closed" );
				break;
			case Online:
				stateStr = QLatin1String( "online" );
				break;
			case Away:
				stateStr = QLatin1String( "away" );
				break;
			case Offline:
				stateStr = QLatin1String( "offline" );
				break;
			case Unknown:
				stateStr = QLatin1String( "unknown" );
				break;
			case None:
			default:
				stateStr = QLatin1String( "none" );
				break;
			}
			iconElement.setAttribute( QLatin1String( "state" ), stateStr );
			iconElement.appendChild( pluginData.createTextNode( it.value() )  );
			iconsElement.appendChild( iconElement );
		}
		pluginData.documentElement().appendChild( iconsElement );
		pluginNodes.append( iconsElement );
	}
	return pluginNodes;
}

bool ContactListElement::fromXML( const QDomElement& element )
{
	if ( element.tagName() == QLatin1String( "plugin-data" ) )
	{
		QMap<QString, QString> pluginData;
		QString pluginId = element.attribute( QLatin1String( "plugin-id" ), QString::null );

		//in kopete 0.6 the AIM protocol was called OSCAR
		if ( pluginId == QLatin1String( "OscarProtocol" ) )
			pluginId = QLatin1String( "AIMProtocol" );

		QDomNode field = element.firstChild();
		while( !field.isNull() )
		{
			QDomElement fieldElement = field.toElement();
			if ( fieldElement.tagName() == QLatin1String( "plugin-data-field" ) )
			{
				pluginData.insert( fieldElement.attribute( QLatin1String( "key" ),
					QLatin1String( "undefined-key" ) ), fieldElement.text() );
			}
			field = field.nextSibling();
		}
		d->pluginData.insert( pluginId, pluginData );
	}
	else if ( element.tagName() == QLatin1String( "custom-icons" ) )
	{
		d->useCustomIcon= element.attribute( QLatin1String( "use" ), QLatin1String( "1" ) ) == QLatin1String( "1" );
		QDomNode ic = element.firstChild();
		while( !ic.isNull() )
		{
			QDomElement iconElement = ic.toElement();
			if ( iconElement.tagName() == QLatin1String( "icon" ) )
			{
				QString stateStr = iconElement.attribute( QLatin1String( "state" ), QString::null );
				QString icon = iconElement.text();
				IconState state = None;

				if ( stateStr == QLatin1String( "open" ) )
					state = Open;
				if ( stateStr == QLatin1String( "closed" ) )
					state = Closed;
				if ( stateStr == QLatin1String( "online" ) )
					state = Online;
				if ( stateStr == QLatin1String( "offline" ) )
					state = Offline;
				if ( stateStr == QLatin1String( "away" ) )
					state = Away;
				if ( stateStr == QLatin1String( "unknown" ) )
					state = Unknown;

				d->icons[ state ] = icon;
			}
			ic = ic.nextSibling();
		}
	}
	else
	{
		return false;
	}

	return true;
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



