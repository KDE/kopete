/*
    kopeteplugindataobject.cpp - Kopete Plugin Data Object

    Copyright (c) 2003      by Olivier Goffart       <ogoffart@tiscalinet.be>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteplugindataobject.h"
#include "kopeteonlinestatus.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>

#include "kopeteplugin.h"

class KopetePluginDataObjectPrivate
{
public:
	QMap<QString, QMap<QString, QString> > pluginData;
	QMap<KopetePluginDataObject::IconState, QString> icons;
	bool useCustomIcon;
};

KopetePluginDataObject::KopetePluginDataObject( QObject *parent, const char *name )
: QObject( parent, name )
{
	d = new KopetePluginDataObjectPrivate;

	d->useCustomIcon = false;

	connect( Kopete::Global::onlineStatusIconCache(), SIGNAL( iconsChanged() ), SIGNAL( iconAppearanceChanged() ) );
}

KopetePluginDataObject::~KopetePluginDataObject()
{
	delete d;
}

void KopetePluginDataObject::setPluginData( KopetePlugin *plugin, const QMap<QString, QString> &pluginData )
{
	if ( pluginData.isEmpty() )
	{
		d->pluginData.remove( plugin->pluginId() );
		return;
	}

	d->pluginData[ plugin->pluginId() ] = pluginData;

	emit pluginDataChanged();
}

void KopetePluginDataObject::setPluginData( KopetePlugin *p, const QString &key, const QString &value )
{
	d->pluginData[ p->pluginId() ][ key ] = value;

	emit pluginDataChanged();
}

QMap<QString, QString> KopetePluginDataObject::pluginData( KopetePlugin *plugin ) const
{
	if ( !d->pluginData.contains( plugin->pluginId() ) )
		return QMap<QString, QString>();

	return d->pluginData[ plugin->pluginId() ];
}

QString KopetePluginDataObject::pluginData( KopetePlugin *plugin, const QString &key ) const
{
	if ( !d->pluginData.contains( plugin->pluginId() ) || !d->pluginData[ plugin->pluginId() ].contains( key ) )
		return QString::null;

	return d->pluginData[ plugin->pluginId() ][ key ];
}

void KopetePluginDataObject::writeConfig( const QString &configGroup ) const
{
	KConfig *config = KGlobal::config();
	config->setGroup( configGroup );

	if ( !d->pluginData.isEmpty() )
	{
		QMap<QString, QMap<QString, QString> >::ConstIterator pluginIt;
		for ( pluginIt = d->pluginData.begin(); pluginIt != d->pluginData.end(); ++pluginIt )
		{
			QMap<QString, QString>::ConstIterator it;
			for ( it = pluginIt.data().begin(); it != pluginIt.data().end(); ++it )
				config->writeEntry( QString::fromLatin1( "PluginData_%1_%2" ).arg( pluginIt.key(), it.key() ), it.data() );
		}
	}

	// Sync all changes to disk
	config->sync();
}

const QValueList<QDomElement> KopetePluginDataObject::toXML()
{
	QDomDocument pluginData;
	QValueList<QDomElement> pluginNodes;
	pluginData.appendChild( pluginData.createElement( QString::fromLatin1( "plugin-data" ) ) );

	if ( !d->pluginData.isEmpty() )
	{
		QMap<QString, QMap<QString, QString> >::ConstIterator pluginIt;
		for ( pluginIt = d->pluginData.begin(); pluginIt != d->pluginData.end(); ++pluginIt )
		{
			QDomElement pluginElement = pluginData.createElement( QString::fromLatin1( "plugin-data" ) );
			pluginElement.setAttribute( QString::fromLatin1( "plugin-id" ), pluginIt.key()  );

			QMap<QString, QString>::ConstIterator it;
			for ( it = pluginIt.data().begin(); it != pluginIt.data().end(); ++it )
			{
				QDomElement pluginDataField = pluginData.createElement( QString::fromLatin1( "plugin-data-field" ) );
				pluginDataField.setAttribute( QString::fromLatin1( "key" ), it.key()  );
				pluginDataField.appendChild( pluginData.createTextNode(  it.data()  ) );
				pluginElement.appendChild( pluginDataField );
			}

			pluginData.documentElement().appendChild( pluginElement );
			pluginNodes.append( pluginElement );
		}
	}
	if ( !d->icons.isEmpty() )
	{
		QDomElement iconsElement = pluginData.createElement( QString::fromLatin1( "custom-icons" ) );
		iconsElement.setAttribute( QString::fromLatin1( "use" ), d->useCustomIcon ?  QString::fromLatin1( "1" ) : QString::fromLatin1( "0" ) );

		for ( QMap<IconState, QString >::ConstIterator it = d->icons.begin(); it != d->icons.end(); ++it )
		{
			QDomElement iconElement = pluginData.createElement( QString::fromLatin1( "icon" ) );
			QString stateStr;
			switch ( it.key() )
			{
			case Open:
				stateStr = QString::fromLatin1( "open" );
				break;
			case Closed:
				stateStr = QString::fromLatin1( "closed" );
				break;
			case Online:
				stateStr = QString::fromLatin1( "online" );
				break;
			case Away:
				stateStr = QString::fromLatin1( "away" );
				break;
			case Offline:
				stateStr = QString::fromLatin1( "offline" );
				break;
			case Unknown:
				stateStr = QString::fromLatin1( "unknown" );
				break;
			case None:
			default:
				stateStr = QString::fromLatin1( "none" );
				break;
			}
			iconElement.setAttribute( QString::fromLatin1( "state" ), stateStr );
			iconElement.appendChild( pluginData.createTextNode( it.data() )  );
			iconsElement.appendChild( iconElement );
		}
		pluginData.documentElement().appendChild( iconsElement );
		pluginNodes.append( iconsElement );
	}
	return pluginNodes;
}

bool KopetePluginDataObject::fromXML( const QDomElement& element )
{
	if ( element.tagName() == QString::fromLatin1( "plugin-data" ) )
	{
		QMap<QString, QString> pluginData;
		QString pluginId = element.attribute( QString::fromLatin1( "plugin-id" ), QString::null );

		//in kopete 0.6 the AIM protocol was called OSCAR
		if ( pluginId == QString::fromLatin1( "OscarProtocol" ) )
			pluginId = QString::fromLatin1( "AIMProtocol" );

		QDomNode field = element.firstChild();
		while( !field.isNull() )
		{
			QDomElement fieldElement = field.toElement();
			if ( fieldElement.tagName() == QString::fromLatin1( "plugin-data-field" ) )
			{
				pluginData.insert( fieldElement.attribute( QString::fromLatin1( "key" ),
					QString::fromLatin1( "undefined-key" ) ), fieldElement.text() );
			}
			field = field.nextSibling();
		}
		d->pluginData.insert( pluginId, pluginData );
	}
	else if ( element.tagName() == QString::fromLatin1( "custom-icons" ) )
	{
		d->useCustomIcon= element.attribute( QString::fromLatin1( "use" ), QString::fromLatin1( "1" ) ) == QString::fromLatin1( "1" );
		QDomNode ic = element.firstChild();
		while( !ic.isNull() )
		{
			QDomElement iconElement = ic.toElement();
			if ( iconElement.tagName() == QString::fromLatin1( "icon" ) )
			{
				QString stateStr = iconElement.attribute( QString::fromLatin1( "state" ), QString::null );
				QString icon = iconElement.text();
				IconState state = None;

				if ( stateStr == QString::fromLatin1( "open" ) )
					state = Open;
				if ( stateStr == QString::fromLatin1( "closed" ) )
					state = Closed;
				if ( stateStr == QString::fromLatin1( "online" ) )
					state = Online;
				if ( stateStr == QString::fromLatin1( "offline" ) )
					state = Offline;
				if ( stateStr == QString::fromLatin1( "away" ) )
					state = Away;
				if ( stateStr == QString::fromLatin1( "unknown" ) )
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

QString KopetePluginDataObject::icon( KopetePluginDataObject::IconState state ) const
{
	if ( d->icons.contains( state ) )
		return d->icons[state];

	return d->icons[ None ];
}

void KopetePluginDataObject::setIcon( const QString& icon , KopetePluginDataObject::IconState state )
{
	if ( icon.isNull() )
		d->icons.remove( state );
	else
		d->icons[ state ] = icon;

	emit iconChanged( state, icon );
	emit iconAppearanceChanged();
}

bool KopetePluginDataObject::useCustomIcon() const
{
	return d->useCustomIcon;
}

void KopetePluginDataObject::setUseCustomIcon( bool useCustomIcon )
{
	if ( d->useCustomIcon != useCustomIcon )
	{
		d->useCustomIcon = useCustomIcon;
		emit useCustomIconChanged( useCustomIcon );
	}
}

#include "kopeteplugindataobject.moc"

// vim: set noet ts=4 sts=4 sw=4:

