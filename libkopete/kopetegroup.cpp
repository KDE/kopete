/*
    kopetegroup.cpp - Kopete (Meta)Contact Group

    Copyright (c) 2002-2003 by Olivier Goffart       <ogoffart@tiscalinet.be>
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

#include "kopetegroup.h"

#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopetecontact.h"

#include <klocale.h>

struct KopeteGroupPrivate
{
	QString displayName;
	QString internalName;
	Kopete::Group::GroupType type;
	bool expanded;
	uint groupId;

	//Unique contact id per metacontact
	static uint uniqueGroupId;
};

Kopete::Group *Kopete::Group::s_topLevel  = 0L;
Kopete::Group *Kopete::Group::s_temporary = 0L;

Kopete::Group * Kopete::Group::topLevel()
{
	// Do not translate the internal name, it's not shown in the GUI
	if ( !s_topLevel )
		s_topLevel = new Kopete::Group( i18n( "Top Level" ),
				QString::fromLatin1( "Top-Level" ), 
				Kopete::Group::TopLevel );

	return s_topLevel;
}

Kopete::Group * Kopete::Group::temporary()
{
	// Do not translate the internal name, it's not shown in the GUI
	if ( !s_temporary )
		s_temporary = new Kopete::Group( i18n( "Not in your contact list" ),
				 QString::fromLatin1( "Temporary" ), 
				 Kopete::Group::Temporary );

	return s_temporary;
}

uint KopeteGroupPrivate::uniqueGroupId = 0;

Kopete::Group::Group( const QString &_name, GroupType _type )
: Kopete::ContactListElement( Kopete::ContactList::self() )
{
	d = new KopeteGroupPrivate;
	d->displayName = _name;
	d->internalName = _name;
	d->type = _type;
	d->expanded = true;
	d->groupId = 0;
}

Kopete::Group::Group()
: Kopete::ContactListElement( Kopete::ContactList::self() )
{
	d = new KopeteGroupPrivate;
	d->expanded = true;
	d->type = Normal;
	d->displayName = QString::null;
	d->internalName = QString::null;
	d->groupId = 0;
}

Kopete::Group::Group( const QString &_displayName, const QString &_internalName, GroupType _type )
{
	d = new KopeteGroupPrivate;
	d->displayName = _displayName;
	d->internalName = _internalName;
	d->type = _type;
	d->expanded = true;
	d->groupId = 0;
}

Kopete::Group::~Group()
{
	if(d->type == TopLevel)
		s_topLevel=0L;
	if(d->type == Temporary)
		s_temporary=0L;
	delete d;
}

QPtrList<Kopete::MetaContact> Kopete::Group::members() const
{
	QPtrList<Kopete::MetaContact> members = Kopete::ContactList::self()->metaContacts();
	// members is a *copy* of the meta contacts, so using first(), next() and remove() is fine.
	for( members.first(); members.current(); )
	{
		if ( members.current()->groups().contains( this ) )
			members.next();
		else
			members.remove();
	}
	return members;
}

const QDomElement Kopete::Group::toXML()
{
	QDomDocument group;
	group.appendChild( group.createElement( QString::fromLatin1( "kopete-group" ) ) );
	group.documentElement().setAttribute( QString::fromLatin1( "groupId" ), QString::number( groupId() ) );

	QString type;
	switch ( d->type )
	{
	case Temporary:
		type = QString::fromLatin1( "temporary" );
		break;
	case TopLevel:
		type = QString::fromLatin1( "top-level" );
		break;
	default:
		type = QString::fromLatin1( "standard" ); // == Normal
		break;
	}

	group.documentElement().setAttribute( QString::fromLatin1( "type" ), type );
	group.documentElement().setAttribute( QString::fromLatin1( "view" ), QString::fromLatin1( d->expanded ? "expanded" : "collapsed" )  );

	QDomElement displayName = group.createElement( QString::fromLatin1( "display-name" ) );
	displayName.appendChild( group.createTextNode( d->displayName ) );
	group.documentElement().appendChild( displayName );

	// Store other plugin data
	QValueList<QDomElement> pluginData = Kopete::ContactListElement::toXML();
	for ( QValueList<QDomElement>::Iterator it = pluginData.begin(); it != pluginData.end(); ++it )
		group.documentElement().appendChild( group.importNode( *it, true ) );

	// Store custom notification data
	QDomElement notifyData = Kopete::NotifyDataObject::notifyDataToXML();
	if ( notifyData.hasChildNodes() )
		group.documentElement().appendChild( group.importNode( notifyData, true ) );

	return group.documentElement();
}

bool Kopete::Group::fromXML( const QDomElement &data )
{
	QString strGroupId = data.attribute( QString::fromLatin1( "groupId" ) );
	if ( !strGroupId.isEmpty() )
	{
		d->groupId = strGroupId.toUInt();
		if ( d->groupId > d->uniqueGroupId )
			d->uniqueGroupId = d->groupId;
	}

	// Don't overwrite type for Temporary and TopLevel groups
	if ( d->type != Temporary && d->type != TopLevel )
	{
		QString type = data.attribute( QString::fromLatin1( "type" ), QString::fromLatin1( "standard" ) );
		if ( type == QString::fromLatin1( "temporary" ) )
		{
			if ( d->type != Temporary )
			{
				s_temporary->fromXML( data );
				return false;
			}
		}
		else if ( type == QString::fromLatin1( "top-level" ) )
		{
			if ( d->type != TopLevel )
			{
				s_topLevel->fromXML( data );
				return false;
			}
		}
		else
		{
			d->type = Normal;
		}
	}

	QString view = data.attribute( QString::fromLatin1( "view" ), QString::fromLatin1( "expanded" ) );
	d->expanded = ( view != QString::fromLatin1( "collapsed" ) );

	QDomNode groupData = data.firstChild();
	while ( !groupData.isNull() )
	{
		QDomElement groupElement = groupData.toElement();
		if ( groupElement.tagName() == QString::fromLatin1( "display-name" ) )
		{
			// Don't set display name for temporary or top-level items
			if ( d->type == Normal )
				d->displayName = groupElement.text();
		}
		else if( groupElement.tagName() == QString::fromLatin1( "custom-notifications" ) )
		{
			Kopete::NotifyDataObject::notifyDataFromXML( groupElement );
		}
		else
		{
			Kopete::ContactListElement::fromXML( groupElement );
		}

		groupData = groupData.nextSibling();
	}

	// Sanity checks. We must not have groups without a displayname.
	if ( d->displayName.isEmpty() )
	{
		switch ( d->type )
		{
		case Temporary:
			d->displayName = QString::fromLatin1( "Temporary" );
			break;
		case TopLevel:
			d->displayName = QString::fromLatin1( "Top-Level" );
			break;
		default:
			d->displayName = i18n( "(Unnamed Group)" );
			break;
		}
	}

	//FIXME: this workaround allows to save data for the top-level group
	return ( d->type == Normal );
}

void Kopete::Group::setDisplayName( const QString &s )
{
	if ( d->displayName != s )
	{
		QString oldname = d->displayName;
		d->displayName = s;
		emit renamed( this, oldname );
	}
}

QString Kopete::Group::displayName() const
{
	return d->displayName;
}

Kopete::Group::GroupType Kopete::Group::type() const
{
	return d->type;
}

void Kopete::Group::setType( GroupType t )
{
	d->type = t;
}

void Kopete::Group::setExpanded( bool isExpanded )
{
	d->expanded = isExpanded;
}

bool Kopete::Group::isExpanded() const
{
	return d->expanded;
}

uint Kopete::Group::groupId() const
{
	if ( d->groupId == 0 )
		d->groupId = ++d->uniqueGroupId;

	return d->groupId;
}

QString Kopete::Group::internalName() const
{
	return d->internalName;
}

void Kopete::Group::sendMessage()
{
	QPtrList<Kopete::MetaContact> list = onlineMembers();
	Kopete::MetaContact *mc = list.first();
	Kopete::Contact *c;
	
	if(!mc)
		return;
	c = mc->preferredContact();
	c->sendMessage();
	if( c->manager( false ) )
	{
		connect( c->manager( false ), SIGNAL( messageSent( Kopete::Message&, Kopete::MessageManager* ) ), this, SLOT( sendMessage( Kopete::Message& ) ));
	}
}

void Kopete::Group::sendMessage( Kopete::Message& msg )
{
	QPtrList<Kopete::MetaContact> list = onlineMembers();
	Kopete::MetaContact *mc = list.first();
	Kopete::Contact *c = msg.to().first();
	
	if(!mc)
		return;
	list.remove( msg.to().first()->metaContact() );
	for( mc = list.first(); mc; mc = list.next() )
	{
		if( mc->preferredContact()->manager( true ) )
		{
			mc->preferredContact()->manager( false )->sendMessage( msg );
		}
	}
	disconnect( c->manager( false ), SIGNAL( messageSent( Kopete::Message&, Kopete::MessageManager* ) ), this, SLOT( sendMessage( Kopete::Message& ) ) );
}

QPtrList<Kopete::MetaContact> Kopete::Group::onlineMembers() const
{
	QPtrList<Kopete::MetaContact> list = members();
	
	for( list.first(); list.current(); )
		if( list.current()->isReachable() )
			list.next();
		else
			list.remove();
	return list;
}

#include "kopetegroup.moc"

// vim: set noet ts=4 sts=4 sw=4:

