/*
    kopeteuserpreferences.cpp  -  Kopete User Preferences

    Copyright (c) 2002      by Olivier Goffart        <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetegroup.h"
#include "kopetecontactlist.h"
#include "kopeteplugin.h"

#include <qdom.h>
#include <qregexp.h>
#include <qstylesheet.h>

#include <klocale.h>

KopeteGroup* KopeteGroup::toplevel = new KopeteGroup(QString::null , KopeteGroup::TopLevel);
KopeteGroup* KopeteGroup::temporary = new KopeteGroup(i18n("Not in your contact list"),KopeteGroup::Temporary);

KopeteGroupList::KopeteGroupList(){}
KopeteGroupList::~KopeteGroupList(){}

QStringList KopeteGroupList::toStringList() 
{
	QStringList rep;
	KopeteGroup *g;;
	for ( g = first(); g; g = next() )
	{
		//if((*it).type() == KopeteGroup::Classic && !(*it).string().isNull())
		rep << g->displayName();
	}
	return rep;
}

//-----------------------------------------------------------------------------

KopeteGroup::KopeteGroup(QString _name, GroupType _type)  : QObject(KopeteContactList::contactList())
{
	m_displayName=_name;
	m_type=_type;
	m_expanded = true;
}
KopeteGroup::KopeteGroup()  : QObject(KopeteContactList::contactList())
{
	m_type=Classic;
	m_displayName=QString::null;
	m_expanded = true;
}

KopeteGroup::~KopeteGroup()
{
}

QString KopeteGroup::toXML()
{
	QString xml = QString::fromLatin1( "  <kopete-group type=\"" );

	if( m_type == Temporary )
		xml += QString::fromLatin1( "temporary" );
	else if( m_type == TopLevel )
		xml += QString::fromLatin1( "top-level" );
	else
		xml += QString::fromLatin1( "standard" );

	xml += QString::fromLatin1( "\" view=\"" ) + QString::fromLatin1( m_expanded ? "expanded" : "collapsed" ) + QString::fromLatin1( "\">\n" );

	xml += QString::fromLatin1( "    <display-name>" ) + QStyleSheet::escape( m_displayName ) + QString::fromLatin1( "</display-name>\n" );

	// Store other plugin data
	if( !m_pluginData.isEmpty() )
	{
		QMap<QString, QMap<QString, QString> >::ConstIterator pluginIt;
		for( pluginIt = m_pluginData.begin(); pluginIt != m_pluginData.end(); ++pluginIt )
		{
			xml += QString::fromLatin1( "    <plugin-data plugin-id=\"" ) + QStyleSheet::escape( pluginIt.key() ) + QString::fromLatin1( "\">\n" );

			QMap<QString, QString>::ConstIterator it;
			for( it = pluginIt.data().begin(); it != pluginIt.data().end(); ++it )
			{
				if(!it.key().isNull())
					xml += QString::fromLatin1( "      <plugin-data-field key=\"" ) + QStyleSheet::escape( it.key() ) + QString::fromLatin1( "\">" )
						+ QStyleSheet::escape( it.data() ) + QString::fromLatin1( "</plugin-data-field>\n" );
			}

			xml += QString::fromLatin1( "    </plugin-data>\n" );
		}
	}

	xml += QString::fromLatin1( "  </kopete-group>\n" );
	return xml;
}

bool KopeteGroup::fromXML( const QDomElement& data )
{
	QString type = data.attribute( QString::fromLatin1( "type" ), QString::fromLatin1( "standard" ) );
	if( type == QString::fromLatin1( "temporary" ) )
		m_type = Temporary;
	else if( type == QString::fromLatin1( "top-level" ) )
		m_type = TopLevel;
	else
		m_type = Classic;

	QString view = data.attribute( QString::fromLatin1( "view" ), QString::fromLatin1( "expanded" ) );
	m_expanded = ( view != QString::fromLatin1( "collapsed" ) );

	QDomNode groupData = data.firstChild();
	while( !groupData.isNull() )
	{
		QDomElement groupElement = groupData.toElement();
		if( groupElement.tagName() == QString::fromLatin1( "display-name" ) )
		{
//			if( groupElement.text().isEmpty() )
//				return false;
			m_displayName = groupElement.text();
		}
		else if( groupElement.tagName() == QString::fromLatin1( "plugin-data" ) )
		{
			QMap<QString, QString> pluginData;
			QString pluginId = groupElement.attribute( QString::fromLatin1( "plugin-id" ), QString::null );

			QDomNode field = groupElement.firstChild();
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
			if( m_type == TopLevel ) //FIXME:
				toplevel->m_pluginData.insert( pluginId, pluginData );
		}

		groupData = groupData.nextSibling();
	}
//	return true;
	return (m_type==Classic);
	//FIXME: this workaroud allow to save data for the top-level group
}

void KopeteGroup::setDisplayName(const QString &s)
{
	if(m_displayName!=s)
	{
		QString oldname=m_displayName;
		m_displayName=s;
		emit renamed(this,oldname);
	}
}

QString KopeteGroup::displayName() const
{
	return m_displayName;
}

KopeteGroup::GroupType KopeteGroup::type() const
{
	return m_type;
}
void KopeteGroup::setType(GroupType t)
{
	m_type=t;
}

QString KopeteGroup::pluginData( KopetePlugin *p, const QString &key ) const
{
	if( !m_pluginData.contains( QString::fromLatin1( p->pluginId() ) ) || !m_pluginData[ QString::fromLatin1( p->pluginId() ) ].contains( key ) )
		return QString::null;

	return m_pluginData[ QString::fromLatin1( p->pluginId() ) ][ key ];
}

void KopeteGroup::setPluginData( KopetePlugin *p, const QString &key, const QString &value )
{
	m_pluginData[ QString::fromLatin1( p->pluginId() ) ][ key ] = value;
}


#include "kopetegroup.moc"
