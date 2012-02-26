/*
    kopetegroup.cpp - Kopete (Meta)Contact Group

    Copyright (c) 2002-2005 by Olivier Goffart       <ogoffart@kde.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

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

#include "kopetegroup.h"
#include "kopetegroup_p.h"

#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopetechatsession.h"

#include <KIcon>
#include <klocale.h>

namespace Kopete {

Group *Group::s_topLevel  = 0L;
Group *Group::s_temporary = 0L;
Group *Group::s_offline = 0L;
Group * Group::topLevel()
{
	if ( !s_topLevel )
		s_topLevel = new Group( i18n( "Top Level" ), Group::TopLevel );

	return s_topLevel;
}

Group * Group::temporary()
{
	if ( !s_temporary )
		s_temporary = new Group( i18n( "Not in your contact list" ), Group::Temporary );

	return s_temporary;
}

Group * Group::offline()
{
	if ( !s_offline )
		s_offline = new Group( i18n( "Offline Users" ), Group::Offline );

	return s_offline;
}

uint Group::Private::uniqueGroupId = 0;

Group::Group( const QString &_name )
	: ContactListElement( ContactList::self() ), d(new Private())
{
	d->displayName = _name;
	d->type = Normal;
	d->expanded = true;
	d->groupId = 0;
}

Group::Group( const QString &_name, GroupType _type )
	: ContactListElement( ContactList::self() ), d(new Private())
{
	d->displayName = _name;
	d->type = _type;
	d->expanded = true;
	d->groupId = 0;
}

Group::Group()
	: ContactListElement( ContactList::self() ), d(new Private())
{
	d->expanded = true;
	d->type = Normal;
	d->groupId = 0;
}

Group::~Group()
{
	if(d->type == TopLevel)
		s_topLevel=0L;
	if(d->type == Temporary)
		s_temporary=0L;
	if(d->type == Offline)
		s_offline=0L;
	delete d;
}

QList<MetaContact *> Group::members() const
{
	QList<MetaContact *> groupMembers;
	foreach(MetaContact *mc, ContactList::self()->metaContacts())
	{
		if( mc->groups().contains( const_cast<Group*>(this) ) )
			groupMembers.append(mc);
	}

	return groupMembers;
}

void Group::setDisplayName( const QString &s )
{
	if ( d->displayName != s )
	{
		QString oldname = d->displayName;
		d->displayName = s;
		// Don't emit the signal in loading state
		if( !loading() )
			emit displayNameChanged( this, oldname );
	}
}

QString Group::displayName() const
{
	return d->displayName;
}

Group::GroupType Group::type() const
{
	return d->type;
}

void Group::setExpanded( bool isExpanded )
{
	d->expanded = isExpanded;
}

bool Group::isExpanded() const
{
	return d->expanded;
}

uint Group::groupId() const
{
	if ( d->groupId == 0 )
		d->groupId = ++d->uniqueGroupId;

	return d->groupId;
}

void Group::setGroupId(uint groupId)
{
	d->groupId = groupId;
}

uint Group::uniqueGroupId() const
{
	return d->uniqueGroupId;
}

void Group::setUniqueGroupId(uint uniqueGroupId)
{
	d->uniqueGroupId = uniqueGroupId;
}

void Group::sendMessage()
{
	Kopete::Contact *c;

	if(onlineMembers().isEmpty())
		return;
	c = onlineMembers().first()->preferredContact();
	c->sendMessage();
	if( c->manager( Contact::CanCreate ) )
	{
		connect( c->manager(), SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)), this, SLOT(sendMessage(Kopete::Message&)));
	}
}

void Group::sendMessage( Message& msg )
{
	QList<MetaContact *> list = onlineMembers();
	ChatSession *cs=msg.manager();
	if(  cs )
	{
		disconnect( cs, SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)), this, SLOT(sendMessage(Kopete::Message&)) );
	}
	else
		return;

	if(list.isEmpty())
		return;
	list.removeAll( msg.to().first()->metaContact() );
	QListIterator<MetaContact *> it(list);
	while ( it.hasNext() )
	{
		MetaContact *mc = it.next();
		if(mc->isReachable())
		{
			Contact *kcontact=mc->preferredContact();
			if( kcontact->manager( Contact::CanCreate )  )
			{
				//This is hack and stupid.    send message to group should never exist anyway - Olivier 2005-09-11
				// changing the "to" is require, because jabber use it to send the messgae.  Cf BUG 111514
				Message msg2(cs->myself() , kcontact);
				msg2.setPlainBody( msg.plainBody() );
				msg2.setDirection( msg.direction() );
				msg2.setRequestedPlugin( msg.requestedPlugin() );

				kcontact->manager( Contact::CanCreate )->sendMessage( msg2 );
			}
		}
	}
}

QList<MetaContact *> Group::onlineMembers() const
{
	QList<MetaContact *> list = members();
	QList<MetaContact *>::iterator it=list.begin();
	while ( it!=list.end() )
	{
		if( (*it)->isReachable() && (*it)->isOnline() )
			++it;
		else
			it=list.erase(it);
	}
	return list;
}

} //END namespace Kopete


#include "kopetegroup.moc"



