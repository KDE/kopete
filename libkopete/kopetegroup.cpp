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
#include <qdom.h>
#include <qstylesheet.h>
#include <qregexp.h>

KopeteGroup* KopeteGroup::toplevel=new KopeteGroup(QString::null , KopeteGroup::TopLevel);
KopeteGroup* KopeteGroup::temporary=new KopeteGroup("temporaryGroup",KopeteGroup::Temporary);

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
	QString xml = "  <kopete-group>\n";
	xml += "    <display-name>" + m_displayName + "</display-name>\n";
	
	if( m_type == Classic )
		xml += "    <type>Classic</type>\n";
	if( m_type == Temporary )
		xml += "    <type>Temporary</type>\n";
	if( m_type == TopLevel )
		xml += "    <type>TopLevel</type>\n";
		
	if( m_expanded )
		xml += "    <view>expanded</view>\n";
	else
		xml += "    <view>collapsed</view>\n";

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

		if( groupElement.tagName() == "display-name" )
		{
			if( groupElement.text().isEmpty() )
				return false;
			m_displayName = groupElement.text();
		}
		if( groupElement.tagName() == "type" )
		{
			if( groupElement.text() == "Temporary" )
				m_type = Temporary;
			else if( groupElement.text() == "TopLevel" )
				m_type = TopLevel;
			else
				m_type = Classic;
		}
		if( groupElement.tagName() == "view" )
		{
			if( groupElement.text() == "collapsed" )
				m_expanded = false;
			else
				m_expanded = true;
		}
		else if( groupElement.tagName() == "plugin-data" )
		{
			QString pluginId = groupElement.attribute(
				"plugin-id", QString::null );
			m_pluginData.insert( pluginId, groupElement.text() );
		}                           

		groupData = groupData.nextSibling();
	}
	
	return true;
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
