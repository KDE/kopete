/***************************************************************************
                          kopetegroup.cpp  -  description
                             -------------------
    begin                : jeu oct 24 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart@tiscalinet.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kopetegroup.h"
#include "kopetecontactlist.h"
#include "kopeteplugin.h"
#include <klocale.h>
#include <qdom.h>
#include <qstylesheet.h>
#include <qregexp.h>

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
	QString xml = "  <kopete-group type=\"";

	if( m_type == Temporary )
		xml += "temporary";
	else if( m_type == TopLevel )
		xml += "top-level";
	else
		xml += "standard";

	xml += "\" view=\"" + QString( m_expanded ? "expanded" : "collapsed" ) + "\">\n";

	xml += "    <display-name>" + QStyleSheet::escape( m_displayName ) + "</display-name>\n";

	// Store other plugin data
	QMap<QString, QString>::ConstIterator it;
	for( it = m_pluginData.begin(); it != m_pluginData.end(); ++it )
	{
		xml += "    <plugin-data plugin-id=\"" + QStyleSheet::escape(it.key()) + "\">"
				+ QStyleSheet::escape(it.data()) + "</plugin-data>\n";
	}

	xml += "  </kopete-group>\n";
	return xml;
}

bool KopeteGroup::fromXML(const QDomNode& data)
{
	QDomNode groupData = data;
	
	while( !groupData.isNull() )
	{
		QDomElement groupElement = groupData.toElement();
		QString type = groupElement.attribute( "type", "standard" );
		if( type == "temporary" )
			m_type = Temporary;
		else if( type == "top-level" )
			m_type = TopLevel;
		else
			m_type = Classic;

		QString view = groupElement.attribute( "view", "expanded" );
		m_expanded = ( type != "collapsed" );

		if( groupElement.tagName() == "display-name" )
		{
//			if( groupElement.text().isEmpty() )
//				return false;
			m_displayName = groupElement.text();
		}
		else if( groupElement.tagName() == "plugin-data" )
		{
			QString pluginId = groupElement.attribute( "plugin-id", QString::null );
			m_pluginData.insert( pluginId, groupElement.text() );
			if( m_type == TopLevel ) //FIXME:
				toplevel->m_pluginData.insert( pluginId, groupElement.text() );
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

void KopeteGroup::setPluginData(KopetePlugin *p, QStringList strList )
{
	if(strList.isEmpty())
	{
		m_pluginData.remove(p->pluginId());
		return;
	}

	for ( QStringList::iterator it = strList.begin(); it != strList.end(); ++it )
	{
		//escape '||' I don't like this but it is needed
		(*it)=(*it).replace(QRegExp("\\\\"),"\\\\").replace(QRegExp("\\|"),"\\|;");
	}
	m_pluginData[p->pluginId()] =  strList.join( "||" ) ;
}

QStringList KopeteGroup::pluginData(KopetePlugin *p)
{
	if(!m_pluginData.contains(p->pluginId()))
		return QStringList();

	QStringList strList = QStringList::split( "||", m_pluginData[p->pluginId()] );
	for ( QStringList::iterator it2 = strList.begin(); it2 != strList.end(); ++it2 )
	{
		//unescape '||'  
		(*it2)=(*it2).replace(QRegExp("\\\\\\|;"),"|").replace(QRegExp("\\\\\\\\"),"\\");
	}
	return strList;
}


#include "kopetegroup.moc"
