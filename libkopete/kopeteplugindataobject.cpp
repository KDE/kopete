/*
    kopeteplugindataobject.cpp - Kopete Plugin Data Object

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qdom.h>
#include <qstylesheet.h>
#include <kdebug.h>

#include "kopeteplugin.h"
#include "kopeteplugindataobject.h"


KopetePluginDataObject::KopetePluginDataObject(QObject *parent, const char *name) : QObject (parent, name)
{
}

KopetePluginDataObject::~KopetePluginDataObject()
{

}


void KopetePluginDataObject::setPluginData( KopetePlugin *p, const QMap<QString, QString> &pluginData )
{
	if( pluginData.isEmpty() )
	{
		m_pluginData.remove( p->pluginId() );
		return;
	}

	m_pluginData[ p->pluginId() ] = pluginData;
}

void KopetePluginDataObject::setPluginData( KopetePlugin *p, const QString &key, const QString &value )
{
	m_pluginData[ p->pluginId() ][ key ] = value;
}

QMap<QString, QString> KopetePluginDataObject::pluginData( KopetePlugin *p ) const
{
	if( !m_pluginData.contains( p->pluginId() ) )
		return QMap<QString, QString>();

	return m_pluginData[ p->pluginId() ];
}

QString KopetePluginDataObject::pluginData( KopetePlugin *p, const QString &key ) const
{
	if( !m_pluginData.contains( p->pluginId() ) || !m_pluginData[ p->pluginId() ].contains( key ) )
		return QString::null;

	return m_pluginData[ p->pluginId() ][ key ];
}


const QValueList<QDomElement> KopetePluginDataObject::toXML()
{
	QDomDocument pluginData;
	QValueList<QDomElement> pluginNodes;
	pluginData.appendChild( pluginData.createElement( QString::fromLatin1("plugin-data")) );

	if( !m_pluginData.isEmpty() )
	{
		QMap<QString, QMap<QString, QString> >::ConstIterator pluginIt;
		for( pluginIt = m_pluginData.begin(); pluginIt != m_pluginData.end(); ++pluginIt )
		{
			QDomElement pluginElement = pluginData.createElement( QString::fromLatin1("plugin-data") );
			pluginElement.setAttribute( QString::fromLatin1("plugin-id"), QStyleSheet::escape( pluginIt.key() ) );

			QMap<QString, QString>::ConstIterator it;
			for( it = pluginIt.data().begin(); it != pluginIt.data().end(); ++it )
			{
				QDomElement pluginDataField = pluginData.createElement( QString::fromLatin1("plugin-data-field") );
				pluginDataField.setAttribute( QString::fromLatin1("key"), QStyleSheet::escape( it.key() ) );
				pluginDataField.appendChild( pluginData.createTextNode( QStyleSheet::escape( it.data() ) ) );
				pluginElement.appendChild( pluginDataField );
			}

			pluginData.documentElement().appendChild( pluginElement );
			pluginNodes.append( pluginElement );
		}
	}

	return pluginNodes;
}


void KopetePluginDataObject::fromXML( const QDomElement& element )
{
	if( element.tagName() == QString::fromLatin1( "plugin-data" ) )
	{
		QMap<QString, QString> pluginData;
		QString pluginId = element.attribute( QString::fromLatin1( "plugin-id" ), QString::null );

		QDomNode field = element.firstChild();
		while( !field.isNull() )
		{
			QDomElement fieldElement = field.toElement();
			if( fieldElement.tagName() == QString::fromLatin1( "plugin-data-field" ) )
			{
				pluginData.insert( fieldElement.attribute( QString::fromLatin1( "key" ),
					QString::fromLatin1( "undefined-key" ) ), fieldElement.text() );
			}
			field = field.nextSibling();
		}
		m_pluginData.insert( pluginId, pluginData );
	}
}


