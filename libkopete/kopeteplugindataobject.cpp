/*
    kopeteplugindataobject.cpp - Kopete Plugin Data Object

    Copyright (c) 2003      by Olivier Goffart       <ogoffart@tiscalinet.be>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>

#include "kopeteplugin.h"
#include "kopeteplugindataobject.h"

KopetePluginDataObject::KopetePluginDataObject(QObject *parent, const char *name) : QObject (parent, name)
{
	m_useCustomIcon=false;
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

void KopetePluginDataObject::writeConfig( const QString &configGroup ) const
{
	KConfig *config = KGlobal::config();
	config->setGroup( configGroup );

	if( !m_pluginData.isEmpty() )
	{
		QMap<QString, QMap<QString, QString> >::ConstIterator pluginIt;
		for( pluginIt = m_pluginData.begin(); pluginIt != m_pluginData.end(); ++pluginIt )
		{
			QMap<QString, QString>::ConstIterator it;
			for( it = pluginIt.data().begin(); it != pluginIt.data().end(); ++it )
#if QT_VERSION < 0x030200
				config->writeEntry( QString::fromLatin1( "PluginData_%1_%2" ).arg( pluginIt.key() ).arg( it.key() ), it.data() );
#else
				config->writeEntry( QString::fromLatin1( "PluginData_%1_%2" ).arg( pluginIt.key(), it.key() ), it.data() );
#endif
		}
	}
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
			pluginElement.setAttribute( QString::fromLatin1("plugin-id"), pluginIt.key()  );

			QMap<QString, QString>::ConstIterator it;
			for( it = pluginIt.data().begin(); it != pluginIt.data().end(); ++it )
			{
				QDomElement pluginDataField = pluginData.createElement( QString::fromLatin1("plugin-data-field") );
				pluginDataField.setAttribute( QString::fromLatin1("key"), it.key()  );
				pluginDataField.appendChild( pluginData.createTextNode(  it.data()  ) );
				pluginElement.appendChild( pluginDataField );
			}

			pluginData.documentElement().appendChild( pluginElement );
			pluginNodes.append( pluginElement );
		}
	}
	if( !m_icons.isEmpty() )
	{
		QDomElement iconsElement = pluginData.createElement( QString::fromLatin1("custom-icons") );
		iconsElement.setAttribute( QString::fromLatin1("use"), m_useCustomIcon ?  QString::fromLatin1("1") : QString::fromLatin1("0")   );

		QMap<iconState, QString >::ConstIterator it;
		for( it = m_icons.begin(); it != m_icons.end(); ++it )
		{
			QDomElement iconElement = pluginData.createElement( QString::fromLatin1("icon") );
			QString stateStr;
			switch ( it.key() )
			{
			case Open:
				stateStr=QString::fromLatin1("open");
				break;
			case Closed:
				stateStr=QString::fromLatin1("closed");
				break;
			case Online:
				stateStr=QString::fromLatin1("online");
				break;
			case Away:
				stateStr=QString::fromLatin1("away");
				break;
			case Offline:
				stateStr=QString::fromLatin1("offline");
				break;
			case Unknown:
				stateStr=QString::fromLatin1("unknown");
				break;
			case None:
			default:
				stateStr=QString::fromLatin1("none");
				break;
			}
			iconElement.setAttribute( QString::fromLatin1("state"), stateStr );
			iconElement.appendChild( pluginData.createTextNode( it.data() )  );
			iconsElement.appendChild(iconElement);
		}
		pluginData.documentElement().appendChild( iconsElement );
		pluginNodes.append( iconsElement );
	}
	return pluginNodes;
}

bool KopetePluginDataObject::fromXML( const QDomElement& element )
{
	if( element.tagName() == QString::fromLatin1( "plugin-data" ) )
	{
		QMap<QString, QString> pluginData;
		QString pluginId = element.attribute( QString::fromLatin1( "plugin-id" ), QString::null );

		//in kopete 0.6 the AIM protocol was called OSCAR
		if(pluginId == QString::fromLatin1("OscarProtocol"))
			pluginId=QString::fromLatin1("AIMProtocol");

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
	else if (element.tagName() == QString::fromLatin1( "custom-icons" ) )
	{
		m_useCustomIcon= element.attribute( QString::fromLatin1( "use" ), QString::fromLatin1("1") ) == QString::fromLatin1("1");
		QDomNode ic = element.firstChild();
		while( !ic.isNull() )
		{
			QDomElement iconElement = ic.toElement();
			if( iconElement.tagName() == QString::fromLatin1( "icon" ) )
			{
				QString stateStr = iconElement.attribute( QString::fromLatin1( "state" ), QString::null );
				QString icon=iconElement.text();
				iconState state=None;

				if(stateStr == QString::fromLatin1("open") )
					state=Open;
				if(stateStr == QString::fromLatin1("closed") )
					state=Closed;
				if(stateStr == QString::fromLatin1("online") )
					state=Online;
				if(stateStr == QString::fromLatin1("offline") )
					state=Offline;
				if(stateStr == QString::fromLatin1("away") )
					state=Away;
				if(stateStr == QString::fromLatin1("unknown") )
					state=Unknown;

				m_icons[state]=icon;
			}
			ic = ic.nextSibling();
		}
	}
	else
		return false;
	return true;
}

QString KopetePluginDataObject::icon(KopetePluginDataObject::iconState state) const
{
	if(m_icons.contains(state))
		return m_icons[state];

	return m_icons[None];
}


void KopetePluginDataObject::setIcon( const QString& icon , KopetePluginDataObject::iconState state )
{
	if(icon.isNull())
		m_icons.remove(state);
	else
		m_icons[state]=icon;
}

bool KopetePluginDataObject::useCustomIcon() const
{
	return m_useCustomIcon;
}
void KopetePluginDataObject::setUseCustomIcon(bool b)
{
	m_useCustomIcon=b;
}

// vim: set noet ts=4 sts=4 sw=4:

