/*
    kopetegroup.cpp  -  Kopete Group

    Copyright (c) 2002      by Olivier Goffart        <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetegroup.h"
#include "kopetecontactlist.h"


#include <klocale.h>

KopeteGroup* KopeteGroup::toplevel = new KopeteGroup(QString::fromLatin1("Default"), KopeteGroup::TopLevel);
KopeteGroup* KopeteGroup::temporary = new KopeteGroup(i18n("Not in your contact list"),KopeteGroup::Temporary);

struct KopeteGroupPrivate
{
	QString displayName;
	KopeteGroup::GroupType type;
	bool expanded;
	unsigned int groupId;

	/*KopeteGroup *parentGroup;
	//in case of the loading from xml, the group might not be already loaded
	unsigned int parentGroupId;*/

	//Unique contact id per metacontact
	static unsigned int uniqueGroupId;
};

unsigned int KopeteGroupPrivate::uniqueGroupId=0;

KopeteGroup::KopeteGroup(QString _name, GroupType _type)  : KopetePluginDataObject(KopeteContactList::contactList())
{
	d = new KopeteGroupPrivate;
	d->displayName=_name;
	d->type=_type;
	d->expanded = true;
	d->groupId=0;
/*	d->parentGroup=0L;
	d->parentGroupId=0;*/
}
KopeteGroup::KopeteGroup()  : KopetePluginDataObject(KopeteContactList::contactList())
{
	d = new KopeteGroupPrivate;
	d->expanded = true;
	d->type=Classic;
	d->displayName=QString::null;
	d->groupId=0;
/*	d->parentGroup=0L;
	d->parentGroupId=0;*/
}


KopeteGroup::~KopeteGroup()
{
	delete d;
}

const QDomElement KopeteGroup::toXML()
{
	QDomDocument group;
	group.appendChild( group.createElement(QString::fromLatin1("kopete-group")) );
	group.documentElement().setAttribute( QString::fromLatin1("groupId"), QString::number( groupId()) );

	QString type;
	if( d->type == Temporary )
		type = QString::fromLatin1( "temporary" );
	else if( d->type == TopLevel )
		type = QString::fromLatin1( "top-level" );
	else
		type = QString::fromLatin1( "standard" ); // == Classic

	group.documentElement().setAttribute( QString::fromLatin1("type"), type );
	group.documentElement().setAttribute( QString::fromLatin1("view"), QString::fromLatin1( d->expanded ? "expanded" : "collapsed" )  );

	QDomElement displayName = group.createElement(QString::fromLatin1("display-name"));
	displayName.appendChild( group.createTextNode( d->displayName ) );
	group.documentElement().appendChild( displayName );

	/*if( parentGroup() != KopeteGroup::toplevel )
	{
		QDomElement parentG = group.createElement(QString::fromLatin1("parent-group"));
		parentG.setAttribute( QString::fromLatin1("groupId"), QString::number( parentGroup()->groupId()) );
		group.documentElement().appendChild( parentG );
	}*/

	// Store other plugin data
	QValueList<QDomElement> pluginData = KopetePluginDataObject::toXML();
	for( QValueList<QDomElement>::Iterator it = pluginData.begin(); it != pluginData.end(); ++it )
		group.documentElement().appendChild( group.importNode( *it, true ) );

	return group.documentElement();
}

bool KopeteGroup::fromXML( const QDomElement& data )
{
	QString strGroupId = data.attribute( QString::fromLatin1("groupId") );
	if( !strGroupId.isEmpty() )
	{
		d->groupId = strGroupId.toUInt();
		if( d->groupId > d->uniqueGroupId )
			d->uniqueGroupId = d->groupId;
	}
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
		/*else if( groupElement.tagName() == QString::fromLatin1( "parent-group" ) )
		{
			d->parentGroupId = groupElement.attribute( QString::fromLatin1( "groupId" ) , QString::fromLatin1( "0" ) ).toUInt();
		}*/
		else if( groupElement.tagName() == QString::fromLatin1( "plugin-data" ) )
		{
			KopetePluginDataObject::fromXML(groupElement);
		}

		groupData = groupData.nextSibling();
	}

	// sanity checks. We must not have groups without a displayname. "Classic" should never happen.
	// Should "Default" be i18n()ed as well?
	if ( d->displayName.isEmpty() )
	{
		d->displayName = (d->type == Temporary) ? i18n("Not in your contact list") :
							(d->type == TopLevel) ? QString::fromLatin1("Default") :
										QString::fromLatin1("Classic");
	}

//	return true;
	return (d->type==Classic);
	//FIXME: this workaroud allow to save data for the top-level group
}

void KopeteGroup::setDisplayName(const QString &s)
{
	if( d->displayName != s )
	{
		QString oldname = d->displayName;
		d->displayName = s;
		emit renamed(this, oldname);
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
	d->type = t;
}

void KopeteGroup::setExpanded(bool in_expanded)
{
	d->expanded = in_expanded;
}
bool KopeteGroup::expanded() const
{
	return d->expanded;
}

unsigned int KopeteGroup::groupId() const
{
	if( d->groupId == 0 )
		d->groupId = ++d->uniqueGroupId;

	return d->groupId;
}

/*KopeteGroup* KopeteGroup::parentGroup()
{
	if(d->parentGroup)
		return d->parentGroup;
	else if(d->parentGroupId!=0)
	{
		KopeteGroup *g=KopeteContactList::contactList()->getGroup(d->parentGroupId);
		if(g)
		{
			d->parentGroup=g;
			return g;
		}
	}
	return KopeteGroup::toplevel;
}

void KopeteGroup::setParentGroup(KopeteGroup* g)
{
	KopeteGroup *old_one=parentGroup();
	if(!g)
		g=KopeteGroup::toplevel;
	KopeteGroup *gr=g->parentGroup();
	while(gr && gr != KopeteGroup::toplevel)
	{
		if(gr==this)
			return;
		gr=gr->parentGroup();
	}
	d->parentGroup=g;
	emit movedToGroup( old_one , g , this );
}*/

#include "kopetegroup.moc"
