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
struct KopeteGroupPrivate
{
	QString displayName;
	KopeteGroup::GroupType type;
	bool expanded;
};

KopeteGroup::KopeteGroup(QString _name, GroupType _type)  : KopetePluginDataObject(KopeteContactList::contactList())
{
	d=new KopeteGroupPrivate;
	d->displayName=_name;
	d->type=_type;
	d->expanded = true;
}
KopeteGroup::KopeteGroup()  : KopetePluginDataObject(KopeteContactList::contactList())
{
	d=new KopeteGroupPrivate;
	d->expanded = true;
	d->type=Classic;
	d->displayName=QString::null;
}

KopeteGroup::~KopeteGroup()
{
	delete d;
}

QString KopeteGroup::toXML()
{
	QString xml = QString::fromLatin1( "  <kopete-group type=\"" );

	if( d->type == Temporary )
		xml += QString::fromLatin1( "temporary" );
	else if( d->type == TopLevel )
		xml += QString::fromLatin1( "top-level" );
	else
		xml += QString::fromLatin1( "standard" );

	xml += QString::fromLatin1( "\" view=\"" ) + QString::fromLatin1( d->expanded ? "expanded" : "collapsed" ) + QString::fromLatin1( "\">\n" );

	xml += QString::fromLatin1( "    <display-name>" ) + QStyleSheet::escape( d->displayName ) + QString::fromLatin1( "</display-name>\n" );

	// Store other plugin data
	xml +=KopetePluginDataObject::toXML();

	xml += QString::fromLatin1( "  </kopete-group>\n" );
	return xml;
}

bool KopeteGroup::fromXML( const QDomElement& data )
{
	QString type = data.attribute( QString::fromLatin1( "type" ), QString::fromLatin1( "standard" ) );
	if( type == QString::fromLatin1( "temporary" ) )
	{
		if(d->type != Temporary) 
		{
			temporary->fromXML(data);
			return false;
		}
	}
	else if( type == QString::fromLatin1( "top-level" ) )
	{
		if(d->type != TopLevel)
		{
			toplevel->fromXML(data);
			return false;
		}
	}
	else
		d->type = Classic;

	QString view = data.attribute( QString::fromLatin1( "view" ), QString::fromLatin1( "expanded" ) );
	d->expanded = ( view != QString::fromLatin1( "collapsed" ) );

	QDomNode groupData = data.firstChild();
	while( !groupData.isNull() )
	{
		QDomElement groupElement = groupData.toElement();
		if( groupElement.tagName() == QString::fromLatin1( "display-name" ) )
		{
//			if( groupElement.text().isEmpty() )
//				return false;
			d->displayName = groupElement.text();
		}
		else if( groupElement.tagName() == QString::fromLatin1( "plugin-data" ) )
		{
			KopetePluginDataObject::fromXML(groupElement);
		}

		groupData = groupData.nextSibling();
	}
//	return true;
	return (d->type==Classic);
	//FIXME: this workaroud allow to save data for the top-level group
}

void KopeteGroup::setDisplayName(const QString &s)
{
	if(d->displayName!=s)
	{
		QString oldname=d->displayName;
		d->displayName=s;
		emit renamed(this,oldname);
	}
}

QString KopeteGroup::displayName() const
{
	return d->displayName;
}

KopeteGroup::GroupType KopeteGroup::type() const
{
	return d->type;
}
void KopeteGroup::setType(GroupType t)
{
	d->type=t;
}

void KopeteGroup::setExpanded(bool in_expanded)
{
	d->expanded = in_expanded; 
}
bool KopeteGroup::expanded()
{
	return d->expanded;
}


#include "kopetegroup.moc"
