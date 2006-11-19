/*
    kopetecontactlist.cpp - Kopete's Contact List backend

    Copyright (c) 2005      by Michael Larouche       <larouche@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart @ kde.org>
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

#include <qdir.h>
#include <qregexp.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QTextStream>

#include <kapplication.h>
#include <kabc/stdaddressbook.h>
#include <kdebug.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopetechatsession.h"
//#include "kopetemessage.h"
#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetegroup.h"
#include "kopetepicture.h"
#include "kopetegeneralsettings.h"
#include "xmlcontactstorage.h"

namespace  Kopete
{

class ContactList::Private
{public:
	/** Flag:  do not save the contactlist until she is completely loaded */
	bool loaded ;

	QList<MetaContact *> contacts;
	QList<Group *> groups;
	QList<MetaContact *> selectedMetaContacts;
	QList<Group *> selectedGroups;

	QTimer *saveTimer;

	MetaContact *myself;

	/** Flag: does the user uses the global identity */
	bool useGlobalIdentity;
};

ContactList *ContactList::s_self = 0L;

ContactList *ContactList::self()
{
	if( !s_self )
		s_self = new ContactList;

	return s_self;
}

ContactList::ContactList()
	: QObject( kapp )
{
	setObjectName( "KopeteContactList" );
	d=new Private;

	//the myself metacontact can't be created now, because it will use
	//ContactList::self() as parent which will call this constructor -> infinite loop
	d->myself=0L;

	//no contactlist loaded yet, don't save them
	d->loaded=false;

	// automatically save on changes to the list
	d->saveTimer = new QTimer( this );
	d->saveTimer->setObjectName( "saveTimer" );
	d->saveTimer->setSingleShot( true );
	connect( d->saveTimer, SIGNAL( timeout() ), SLOT ( save() ) );

	connect( this, SIGNAL( metaContactAdded( Kopete::MetaContact * ) ), SLOT( slotSaveLater() ) );
	connect( this, SIGNAL( metaContactRemoved( Kopete::MetaContact * ) ), SLOT( slotSaveLater() ) );
	connect( this, SIGNAL( groupAdded( Kopete::Group * ) ), SLOT( slotSaveLater() ) );
	connect( this, SIGNAL( groupRemoved( Kopete::Group * ) ), SLOT( slotSaveLater() ) );
	connect( this, SIGNAL( groupRenamed( Kopete::Group *, const QString & ) ), SLOT( slotSaveLater() ) );
}

ContactList::~ContactList()
{
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
		if( mc->metaContactId() == metaContactId )
			return mc;
	}

	return 0L;
}


Group * ContactList::group(unsigned int groupId) const
{
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
		kDebug( 14010 ) << k_funcinfo << "Account not found" << endl;
		return 0L;
	}
	return i->contacts()[contactId];
}


MetaContact *ContactList::findMetaContactByDisplayName( const QString &displayName ) const
{
	QListIterator<MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		MetaContact *mc = it.next();
//		kDebug(14010) << "Display Name: " << it.current()->displayName() << "\n";
		if( mc->displayName() == displayName ) {
			return mc;
		}
	}

	return 0L;
}

MetaContact* ContactList::findMetaContactByContactId( const QString &contactId ) const
{
	QListIterator<Kopete::Account *> it( Kopete::AccountManager::self()->accounts() );
	Kopete::Account *a;
	while ( it.hasNext() )
	{
		a = it.next();
		Contact *c=a->contacts()[contactId];
		if(c && c->metaContact())
			return c->metaContact();
	}
	return 0L;
}

Group * ContactList::findGroup(const QString& displayName, int type)
{
	if( type == Group::Temporary )
		return Group::temporary();

	QListIterator<Group *> it(d->groups);
	while ( it.hasNext() )
	{
		Group *curr = it.next();
		if( curr->groupType() == type && curr->displayName() == displayName )
			return curr;
	}

	Group *newGroup = new Group( displayName, (Group::GroupType)type );
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
	connect( mc, SIGNAL( persistentDataChanged( ) ), SLOT( slotSaveLater() ) );
	connect( mc, SIGNAL( addedToGroup( Kopete::MetaContact *, Kopete::Group * ) ), SIGNAL( metaContactAddedToGroup( Kopete::MetaContact *, Kopete::Group * ) ) );
	connect( mc, SIGNAL( removedFromGroup( Kopete::MetaContact *, Kopete::Group * ) ), SIGNAL( metaContactRemovedFromGroup( Kopete::MetaContact *, Kopete::Group * ) ) );
}


void ContactList::removeMetaContact(MetaContact *m)
{
	if ( !d->contacts.contains(m) )
	{
		kDebug(14010) << k_funcinfo << "Trying to remove a not listed MetaContact." << endl;
		return;
	}

	if ( d->selectedMetaContacts.contains( m ) )
	{
		d->selectedMetaContacts.removeAll( m );
		setSelectedItems( d->selectedMetaContacts, d->selectedGroups );
	}

	//removes subcontact from server here and now.
	QList<Contact *> cts = m->contacts();
	QListIterator<Contact *> it(cts);
	while( it.hasNext() )
	{
		it.next()->deleteContact();
	}

	d->contacts.removeAll( m );
	emit metaContactRemoved( m );
	m->deleteLater();
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
		connect( g , SIGNAL ( displayNameChanged(Kopete::Group* , const QString & )) , this , SIGNAL ( groupRenamed(Kopete::Group* , const QString & )) ) ;
	}
}

void ContactList::removeGroup( Group *g )
{
	if ( d->selectedGroups.contains( g ) )
	{
		d->selectedGroups.removeAll( g );
		setSelectedItems( d->selectedMetaContacts, d->selectedGroups );
	}

	d->groups.removeAll( g );
	emit groupRemoved( g );
	g->deleteLater();
}


void ContactList::setSelectedItems(QList<MetaContact *> metaContacts , QList<Group *> groups)
{
	kDebug( 14010 ) << k_funcinfo << metaContacts.count() << " metacontacts, " << groups.count() << " groups selected" << endl;
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

void ContactList::loadGlobalIdentity()
{
 	// Apply the global identity
	if(Kopete::GeneralSettings::self()->enableGlobalIdentity())
 	{
		// Disconnect to make sure it will not cause duplicate calls.
		disconnect(myself(), SIGNAL(displayNameChanged(const QString&, const QString&)), this, SLOT(slotDisplayNameChanged()));
		disconnect(myself(), SIGNAL(photoChanged()), this, SLOT(slotPhotoChanged()));

		connect(myself(), SIGNAL(displayNameChanged(const QString&, const QString&)), this, SLOT(slotDisplayNameChanged()));
		connect(myself(), SIGNAL(photoChanged()), this, SLOT(slotPhotoChanged()));

		// Ensure that the myself metaContactId is always the KABC whoAmI
		KABC::Addressee a = KABC::StdAddressBook::self()->whoAmI();
		if(!a.isEmpty() && a.uid() != myself()->metaContactId())
		{
			myself()->setMetaContactId(a.uid());
		}

		// Apply the global identity
		// Maybe one of the myself contact from a account has a different displayName/photo at startup.
		slotDisplayNameChanged();
		slotPhotoChanged();
 	}
	else
	{
		disconnect(myself(), SIGNAL(displayNameChanged(const QString&, const QString&)), this, SLOT(slotDisplayNameChanged()));
		disconnect(myself(), SIGNAL(photoChanged()), this, SLOT(slotPhotoChanged()));
	}
}

void ContactList::slotDisplayNameChanged()
{
	static bool mutex=false;
	if(mutex)
	{
		kDebug (14010) << k_funcinfo << " mutex blocked" << endl ;
		return;
	}
	mutex=true;

	kDebug( 14010 ) << k_funcinfo << myself()->displayName() << endl;

	emit globalIdentityChanged(Kopete::Global::Properties::self()->nickName().key(), myself()->displayName());
	mutex=false;
}

void ContactList::slotPhotoChanged()
{
	static bool mutex=false;
	if(mutex)
	{
		kDebug (14010) << k_funcinfo << " mutex blocked" << endl ;
		return;
	}
	mutex=true;
	kDebug( 14010 ) << k_funcinfo << myself()->picture().path() << endl;

	emit globalIdentityChanged(Kopete::Global::Properties::self()->photo().key(), myself()->picture().path());
	mutex=false;
	/* The mutex is useful to don't have such as stack overflow 
	Kopete::ContactList::slotPhotoChanged  ->  Kopete::ContactList::globalIdentityChanged  
	MSNAccount::slotGlobalIdentityChanged  ->  Kopete::Contact::propertyChanged 
	Kopete::MetaContact::slotPropertyChanged -> Kopete::MetaContact::photoChanged -> Kopete::ContactList::slotPhotoChanged 
	*/
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
		kDebug(14010) << k_funcinfo << "Contact list storage failed. Reason: " << storage->errorMessage() << endl;
		d->loaded = true;
		delete storage;
		return;
	}

	addGroups( storage->groups() );
	addMetaContacts( storage->contacts() );

	// Apply the global identity when all the protocols plugins are loaded.
	connect(PluginManager::self(), SIGNAL(allPluginsLoaded()), this, SLOT(loadGlobalIdentity()));
	
	d->loaded = true;
	delete storage;
}

void Kopete::ContactList::save()
{
	if( !d->loaded )
	{
		kDebug(14010) << "Contact list not loaded, abort saving" << endl;
		return;
	}

	Kopete::ContactListStorage *storage = new Kopete::XmlContactStorage();
	storage->save();
	if( !storage->isValid() )
	{
		kDebug(14010) << k_funcinfo << "Contact list storage failed. Reason: " << storage->errorMessage() << endl;

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

QStringList ContactList::contacts() const
{
	QStringList contacts;
	QListIterator<Kopete::MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		contacts.append( it.next()->displayName() );
	}
	return contacts;
}

QStringList ContactList::contactStatuses() const
{
	QStringList meta_contacts;
	QListIterator<Kopete::MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Kopete::MetaContact * mc = it.next();
		meta_contacts.append( QString::fromLatin1( "%1 (%2)" ).
			arg( mc->displayName(), mc->statusString() ));
	}
	return meta_contacts;
}

QStringList ContactList::reachableContacts() const
{
	QStringList contacts;
	QListIterator<Kopete::MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Kopete::MetaContact * mc = it.next();
		if ( mc->isReachable() )
			contacts.append( mc->displayName() );
	}
	return contacts;
}

QList<Contact *> ContactList::onlineContacts() const
{
	QList<Kopete::Contact *> result;
	QListIterator<Kopete::MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Kopete::MetaContact *mc = it.next();
		if ( mc->isOnline() )
		{
			QList<Kopete::Contact *> contacts = mc->contacts();
			QListIterator<Kopete::Contact *> cit( contacts );
			while ( cit.hasNext() )
			{
				Kopete::Contact *c = cit.next();
				if ( c->isOnline() )
					result.append( c );
			}
		}
	}
	return result;
}

QList<Kopete::MetaContact *> Kopete::ContactList::onlineMetaContacts() const
{
	QList<Kopete::MetaContact *> result;
	QListIterator<Kopete::MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Kopete::MetaContact * mc = it.next();
		if ( mc->isOnline() )
			result.append( mc );
	}
	return result;
}

QList<Kopete::MetaContact *> Kopete::ContactList::onlineMetaContacts( const QString &protocolId ) const
{
	QList<Kopete::MetaContact *> result;
	QListIterator<Kopete::MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Kopete::MetaContact * mc = it.next();
		// FIXME: This loop is not very efficient :(
		if ( mc->isOnline() )
		{
			QList<Kopete::Contact *> contacts = mc->contacts();
			QListIterator<Kopete::Contact *> cit( contacts );
			while ( cit.hasNext() )
			{
				Kopete::Contact *c = cit.next();
				if( c->isOnline() && c->protocol()->pluginId() == protocolId )
					result.append( mc );
			}
		}
	}
	return result;
}

QList<Kopete::Contact *> Kopete::ContactList::onlineContacts( const QString &protocolId ) const
{
	QList<Kopete::Contact *> result;
	QListIterator<Kopete::MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Kopete::MetaContact * mc = it.next();
		// FIXME: This loop is not very efficient :(
		if ( mc->isOnline() )
		{
			QList<Kopete::Contact *> contacts = mc->contacts();
			QListIterator<Kopete::Contact *> cit( contacts );
			while ( cit.hasNext() )
			{
				Kopete::Contact *c = cit.next();
				if( c->isOnline() && c->protocol()->pluginId() == protocolId )
					result.append( c );
			}
		}
	}
	return result;
}

QStringList Kopete::ContactList::fileTransferContacts() const
{
	QStringList contacts;
	QList<Kopete::MetaContact *> result;
	QListIterator<Kopete::MetaContact *> it( d->contacts );
	while ( it.hasNext() )
	{
		Kopete::MetaContact * mc = it.next();
		if ( mc->canAcceptFiles() )
			contacts.append( mc->displayName() );
	}
	return contacts;
}

void Kopete::ContactList::sendFile( const QString &displayName, const KUrl &sourceURL,
	const QString &altFileName, const long unsigned int fileSize)
{
//	kDebug(14010) << "Send To Display Name: " << displayName << "\n";

	Kopete::MetaContact *c = findMetaContactByDisplayName( displayName );
	if( c )
		c->sendFile( sourceURL, altFileName, fileSize );
}

void Kopete::ContactList::messageContact( const QString &contactId, const QString &messageText )
{
	Kopete::MetaContact *mc = findMetaContactByContactId( contactId );
	if (!mc) return;

	Kopete::Contact *c = mc->execute(); //We need to know which contact was chosen as the preferred in order to message it
	if (!c) return;

	Kopete::Message msg(c->account()->myself(), c, messageText, Kopete::Message::Outbound);
	c->manager(Contact::CanCreate)->sendMessage(msg);

}


QStringList Kopete::ContactList::contactFileProtocols(const QString &displayName)
{
//	kDebug(14010) << "Get contacts for: " << displayName << "\n";
	QStringList protocols;

	Kopete::MetaContact *c = findMetaContactByDisplayName( displayName );
	if( c )
	{
		QList<Kopete::Contact *> mContacts = c->contacts();
		kDebug(14010) << mContacts.count() << endl;
		QListIterator<Kopete::Contact *> jt( mContacts );
		while ( jt.hasNext() )
		{
			Kopete::Contact *c = jt.next();
			kDebug(14010) << "1" << c->protocol()->pluginId() << endl;
			if( c->canAcceptFiles() ) {
				kDebug(14010) << c->protocol()->pluginId() << endl;
				protocols.append ( c->protocol()->pluginId() );
			}
		}
		return protocols;
	}
	return QStringList();
}


void ContactList::slotSaveLater()
{
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

