/*
    kopetecontactlist.cpp - Kopete's Contact List backend

    Copyright (c) 2005-2007 by Michael Larouche       <larouche@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>

    Copyright (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetecontactlist.h"

// Qt includes
#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtCore/QTimer>
#include <QtCore/QTextStream>

// KDE includes
#include <kabc/stdaddressbook.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <ksavefile.h>
#include <kstandarddirs.h>

// Kopete includes
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetechatsession.h"
#include "kopetecontact.h"
#include "kopetedeletecontacttask.h"
#include "kopetegroup.h"
#include "kopetemetacontact.h"
#include "kopetepicture.h"
#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"
#include "xmlcontactstorage.h"

namespace  Kopete
{

class ContactList::Private
{public:
	/** Flag:  do not save the contact list until she is completely loaded */
	bool loaded ;
	bool terminated;

	QList<MetaContact *> contacts;
	QList<Group *> groups;
	QList<MetaContact *> selectedMetaContacts;
	QList<Group *> selectedGroups;

	QTimer *saveTimer;

	MetaContact *myself;
};

ContactList *ContactList::s_self = 0L;

ContactList *ContactList::self()
{
	if( !s_self )
		s_self = new ContactList;

	return s_self;
}

ContactList::ContactList()
	: QObject( kapp ), d(new Private())
{
	setObjectName( "KopeteContactList" );

	//the myself metacontact can't be created now, because it will use
	//ContactList::self() as parent which will call this constructor -> infinite loop
	d->myself=0L;

	//no contact list loaded yet, don't save them
	d->loaded=false;
	d->terminated = false;

	// automatically save on changes to the list
	d->saveTimer = new QTimer( this );
	d->saveTimer->setObjectName( "saveTimer" );
	d->saveTimer->setSingleShot( true );
	connect( d->saveTimer, SIGNAL(timeout()), SLOT (save()) );

	connect( this, SIGNAL(metaContactAdded(Kopete::MetaContact*)), SLOT(slotSaveLater()) );
	connect( this, SIGNAL(metaContactRemoved(Kopete::MetaContact*)), SLOT(slotSaveLater()) );
	connect( this, SIGNAL(groupAdded(Kopete::Group*)), SLOT(slotSaveLater()) );
	connect( this, SIGNAL(groupRemoved(Kopete::Group*)), SLOT(slotSaveLater()) );
	connect( this, SIGNAL(groupRenamed(Kopete::Group*,QString)), SLOT(slotSaveLater()) );
}

ContactList::~ContactList()
{
	s_self=0L;
	delete d->myself;
	delete d;
}

QList<MetaContact *> ContactList::metaContacts() const
{
	return d->contacts;
}


QList<Group *> ContactList::groups() const
{
	return d->groups;
}


MetaContact *ContactList::metaContact( const QString &metaContactId ) const
{
	QListIterator<MetaContact *> it( d->contacts );

	while ( it.hasNext() )
	{
		MetaContact *mc = it.next();
		if( mc->metaContactId() == QUuid( metaContactId ) )
			return mc;
	}

	return 0L;
}


Group * ContactList::group(unsigned int groupId) const
{
	if ( groupId == Group::topLevel()->groupId() )
		return Group::topLevel();

	QListIterator<Group *> it(d->groups);

	while ( it.hasNext() )
	{
		Group *curr = it.next();
		if( curr->groupId()==groupId )
			return curr;
	}
	return 0L;
}


Contact *ContactList::findContact( const QString &protocolId,
	const QString &accountId, const QString &contactId ) const
{
	//Browsing metacontacts is too slow, better to uses the Dict of the account.
	Account *i=AccountManager::self()->findAccount(protocolId,accountId);
	if(!i)
	{
		kDebug( 14010 ) << "Account not found";
		return 0L;
	}
	return i->contacts().value( contactId );
}


MetaContact *ContactList::findMetaContactByDisplayName( const QString &displayName ) const
{
	foreach(Kopete::MetaContact *contact, d->contacts)
	{
		if( contact->displayName() == displayName )
		{
			return contact;
		}
	}
        return 0;
}

MetaContact* ContactList::findMetaContactByContactId( const QString &contactId ) const
{
	QListIterator<Kopete::Account *> it( Kopete::AccountManager::self()->accounts() );
	Kopete::Account *a;
	while ( it.hasNext() )
	{
		a = it.next();
		Contact *c=a->contacts().value( contactId );
		if(c && c->metaContact())
			return c->metaContact();
	}
	return 0L;
}

Group * ContactList::findGroup(const QString& displayName, int type)
{
	if( type == Group::Temporary )
		return Group::temporary();
	if( type == Group::TopLevel )
		return Group::topLevel();
	if( type == Group::Offline )
		return Group::offline();

	QListIterator<Group *> it(d->groups);
	while ( it.hasNext() )
	{
		Group *curr = it.next();
		if( curr->type() == type && curr->displayName() == displayName )
			return curr;
	}

	Group *newGroup = new Group( displayName );
	addGroup( newGroup );
	return  newGroup;
}


QList<MetaContact *> ContactList::selectedMetaContacts() const
{
	return d->selectedMetaContacts;
}

QList<Group *> ContactList::selectedGroups() const
{
	return d->selectedGroups;
}

void ContactList::addMetaContacts( QList<MetaContact *> metaContacts )
{
	foreach( MetaContact* mc, metaContacts )
		addMetaContact( mc );
}

void ContactList::addMetaContact( MetaContact *mc )
{
	if ( d->contacts.contains( mc ) )
		return;

	d->contacts.append( mc );

	emit metaContactAdded( mc );
	connect( mc, SIGNAL(persistentDataChanged()), SLOT(slotSaveLater()) );
	connect( mc, SIGNAL(addedToGroup(Kopete::MetaContact*,Kopete::Group*)), SIGNAL(metaContactAddedToGroup(Kopete::MetaContact*,Kopete::Group*)) );
	connect( mc, SIGNAL(removedFromGroup(Kopete::MetaContact*,Kopete::Group*)), SIGNAL(metaContactRemovedFromGroup(Kopete::MetaContact*,Kopete::Group*)) );
	connect( mc, SIGNAL(movedToGroup(Kopete::MetaContact*,Kopete::Group*,Kopete::Group*)),
	             SIGNAL(metaContactMovedToGroup(Kopete::MetaContact*,Kopete::Group*,Kopete::Group*)));
}


void ContactList::removeMetaContact(MetaContact *m)
{
	if ( !d->contacts.contains(m) )
	{
		kDebug(14010) << "Trying to remove a not listed MetaContact.";
		return;
	}

	if ( d->selectedMetaContacts.contains( m ) )
	{
		d->selectedMetaContacts.removeAll( m );
		setSelectedItems( d->selectedMetaContacts, d->selectedGroups );
	}

	//removes subcontact from server here and now.
	Kopete::Contact *contactToDelete = 0;
	foreach( contactToDelete, m->contacts() )
	{
		// TODO: Check for good execution of task
		Kopete::DeleteContactTask *deleteTask = new Kopete::DeleteContactTask(contactToDelete);
		deleteTask->start();
	}

	d->contacts.removeAll( m );
	emit metaContactRemoved( m );
	m->deleteLater();
}

void ContactList::mergeMetaContacts( QList<MetaContact *> src, Kopete::MetaContact *dst )
{
	// merge all metacontacts from src into dst

	// Note: there is no need to remove the src metacontacts, they are going to be
	// removed when the last contact is moved to the new metacontact

	// TODO: add a confirmation dialog asking if this is really wanted
	// TODO: add a Undo option for this
	
	foreach( Kopete::MetaContact *mc, src )
	{
		foreach( Kopete::Contact *c, mc->contacts() )
			c->setMetaContact( dst );
	}
}

void ContactList::addGroups( QList<Group *> groups )
{
	foreach( Group* g, groups )
		addGroup( g );
}

void ContactList::addGroup( Group * g )
{
	if(!d->groups.contains(g) )
	{
		d->groups.append( g );
		emit groupAdded( g );
		connect( g , SIGNAL (displayNameChanged(Kopete::Group*,QString)) , this , SIGNAL (groupRenamed(Kopete::Group*,QString)) ) ;
	}
}

void ContactList::removeGroup( Group *g )
{
	if ( g == Group::topLevel() )
		return;

	if ( d->selectedGroups.contains( g ) )
	{
		d->selectedGroups.removeAll( g );
		setSelectedItems( d->selectedMetaContacts, d->selectedGroups );
	}

	// Remove metaContacts from group or delete the metaContact if it isn't in any other group
	foreach ( MetaContact * metaContact, g->members() )
	{
		const QList<Group *> mcGroups = metaContact->groups();
		if ( (mcGroups.count() == 1 && mcGroups.contains( g )) || mcGroups.isEmpty() )
			removeMetaContact( metaContact );
		else
			metaContact->removeFromGroup( g );
	}

	d->groups.removeAll( g );
	emit groupRemoved( g );
	g->deleteLater();
}


void ContactList::setSelectedItems(QList<MetaContact *> metaContacts , QList<Group *> groups)
{
	kDebug( 14010 ) << metaContacts.count() << " metacontacts, " << groups.count() << " groups selected";
	d->selectedMetaContacts=metaContacts;
	d->selectedGroups=groups;

	emit metaContactSelected( groups.isEmpty() && metaContacts.count()==1 );
	emit selectionChanged();
}

MetaContact* ContactList::myself()
{
	if(!d->myself)
		d->myself=new MetaContact();
	return d->myself;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void ContactList::load()
{
	// don't save when we're in the middle of this...
	d->loaded = false;

	Kopete::ContactListStorage *storage = new Kopete::XmlContactStorage();
	storage->load();
	if( !storage->isValid() )
	{
		kDebug(14010) << "Contact list storage failed. Reason: " << storage->errorMessage();
	}
	else
	{
		addGroups( storage->groups() );
		addMetaContacts( storage->contacts() );
	}

	d->loaded = true;
	delete storage;
	emit contactListLoaded();
}

bool Kopete::ContactList::loaded() const
{
	return d->loaded;
}

void Kopete::ContactList::save()
{
	if ( d->terminated )
	{
		kWarning(14010) << "Contact list terminated, abort saving";
		return;
	}

	if( !d->loaded )
	{
		kDebug(14010) << "Contact list not loaded, abort saving";
		return;
	}

	Kopete::ContactListStorage *storage = new Kopete::XmlContactStorage();
	storage->save();
	if( !storage->isValid() )
	{
		kDebug(14010) << "Contact list storage failed. Reason: " << storage->errorMessage();

		// Saving the contact list failed. retry every minute until it works.
		// single-shot: will get restarted by us next time if it's still failing
		d->saveTimer->setSingleShot( true );
		d->saveTimer->start( 60000 );
		delete storage;
		return;
	}

	// cancel any scheduled saves
	d->saveTimer->stop();
	delete storage;
}

void ContactList::shutdown()
{
	if ( !d->terminated )
	{
		save();
		d->terminated = true;
		d->saveTimer->stop();
	}
}

void ContactList::slotSaveLater()
{
	if ( d->terminated )
		return;

	// if we already have a save scheduled, it will be cancelled. either way,
	// start a timer to save the contact list a bit later.
	d->saveTimer->start( 17100 /* 17,1 seconds */ );
}

void ContactList::slotKABCChanged()
{
	// TODO: react to changes in KABC, replacing this function, post 3.4 (Will)
	// call syncWithKABC on each metacontact to check if its associated kabc entry has changed.
/*	for ( MetaContact * mc = d->contacts.first(); mc; mc = d->contacts.next() )

		mc->syncWithKABC();*/
}


} //END namespace Kopete

#include "kopetecontactlist.moc"

// vim: set noet ts=4 sts=4 sw=4:

