/*
    kopeteaccount.cpp - Kopete Account

    Copyright (c) 2003-2004 by Olivier Goffart       <ogoffart@tiscalinet.be>
    Copyright (c) 2003-2004 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qapplication.h>
#include <qtimer.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kiconeffect.h>

#include "kopetecontactlist.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "kopetepluginmanager.h"
#include "kopetegroup.h"

namespace Kopete 
{



class Account::Private
{
public:
	Private( Protocol *protocol, const QString &accountId )
	 : protocol( protocol ), id( accountId )
	 , autoconnect( false ), priority( 0 ), myself( 0 )
	 , suppressStatusNotification( false )
	{ }

	Protocol *protocol;
	QString id;
	bool autoconnect;
	uint priority;
	QDict<Contact> contacts;
	QColor color;
	Contact *myself;
	QTimer suppressStatusTimer;
	bool suppressStatusNotification;
	KConfigGroup *configGroup;
};

Account::Account( Protocol *parent, const QString &accountId, const char *name )
 : QObject( parent, name ), d( new Private( parent, accountId ) )
{
	d->configGroup=new KConfigGroup(KGlobal::config(), QString::fromLatin1( "Account_%1_%2" ).arg( d->protocol->pluginId(), d->id ));
	
	d->autoconnect = d->configGroup->readBoolEntry( "AutoConnect", false );
	d->color = d->configGroup->readColorEntry( "Color", &d->color );
	d->priority = d->configGroup->readNumEntry( "Priority", 0 );

	
	QObject::connect( &d->suppressStatusTimer, SIGNAL( timeout() ),
		this, SLOT( slotStopSuppression() ) );

}

Account::~Account()
{
	// Delete all registered child contacts first
	while ( !d->contacts.isEmpty() )
		delete *QDictIterator<Contact>( d->contacts );

	emit accountDestroyed(this);

	delete d->configGroup;
	delete d;
}


void Account::disconnected( DisconnectReason reason )
{
//reconnect if needed
#if 0	 //TODO  uses a configuration option

	if ( KopetePrefs::prefs()->reconnectOnDisconnect() == true && reason > Manual )
		//use a timer to allow the plugins to clean up after return
#endif
	if(reason > Manual )
		//use a timer to allow the plugins to clean up after return
		QTimer::singleShot(0, this, SLOT(connect()));
}

Protocol *Account::protocol() const
{
	return d->protocol;
}

QString Account::accountId() const
{
	return d->id;
}

const QColor Account::color() const
{
	return d->color;
}

void Account::setColor( const QColor &color )
{
	d->color = color;
	if ( d->color.isValid() )
		d->configGroup->writeEntry( "Color", d->color );
	else
		d->configGroup->deleteEntry( "Color" );
	emit colorChanged( color );
}

void Account::setPriority( uint priority )
{
 	d->priority = priority;
	d->configGroup->writeEntry( "Priority", d->priority );
}

uint Account::priority() const
{
	return d->priority;
}

QPixmap Account::accountIcon(const int size) const
{
	// FIXME: this code is duplicated with OnlineStatus, can we merge it somehow?
	QPixmap base = KGlobal::instance()->iconLoader()->loadIcon(
		d->protocol->pluginIcon(), KIcon::Small, size );

	if ( d->color.isValid() )
	{
		KIconEffect effect;
		base = effect.apply( base, KIconEffect::Colorize, 1, d->color, 0);
	}

	if ( size > 0 && base.width() != size )
	{
		base = QPixmap( base.convertToImage().smoothScale( size, size ) );
	}

	return base;
}



KConfigGroup* Account::configGroup() const
{
	return d->configGroup;
}


void Account::readConfig(  )
{
}

void Account::setAutoConnect( bool b )
{
	d->autoconnect = b;
	d->configGroup->writeEntry( "AutoConnect", d->autoconnect );
}

bool Account::autoConnect() const
{
	return d->autoconnect;
}



void Account::registerContact( Contact *c )
{
	d->contacts.insert( c->contactId(), c );
	QObject::connect( c, SIGNAL( contactDestroyed( Contact * ) ),
		SLOT( slotContactDestroyed( Contact * ) ) );
}

void Account::slotContactDestroyed( Contact *c )
{
	//kdDebug( 14010 ) << "Protocol::slotContactDestroyed: " << c->contactId() << endl;
	d->contacts.remove( c->contactId() );
}

const QDict<Contact>& Account::contacts()
{
	return d->contacts;
}



MetaContact *Account::addMetaContact( const QString &contactId, const QString &displayName , Group *group, AddMode mode  ) 
{
	if ( contactId == d->myself->contactId() )
	{
		kdDebug( 14010 ) << k_funcinfo <<
			"WARNING: the user try to add myself to his contactlist - abort" << endl;
		return 0L;
	}
	bool isTemporary = mode == Temporary;
	
	Contact *c = d->contacts[ contactId ];

	if(!group)
		group=Group::topLevel();

	if ( c && c->metaContact() )
	{
		if ( c->metaContact()->isTemporary() && !isTemporary )
		{
			kdDebug( 14010 ) << k_funcinfo <<  " You are trying to add an existing temporary contact. Just add it on the list" << endl;
			
			c->metaContact()->setTemporary(false, group );
			ContactList::self()->addMetaContact(c->metaContact());
		}
		else
		{
			// should we here add the contact to the parentContact if any?
			kdDebug( 14010 ) << k_funcinfo << "Contact already exists" << endl;
		}
		return c->metaContact();
	}

	MetaContact *parentContact = new MetaContact();
	if(!displayName.isEmpty())
		parentContact->setDisplayName( displayName );

	//Set it as a temporary contact if requested
	if ( isTemporary )
		parentContact->setTemporary( true );
	else
		parentContact->addToGroup( group );

	if ( c )
	{
		c->setMetaContact( parentContact );
		#if 0 //TODO
		if ( mode == ChangeKABC )
		{
			parentContact->updateKABC();
		}
		#endif
	}
	else
	{
		if ( !createContact( contactId, parentContact ) )
		{
			delete parentContact;
			return 0L;
		}
	}
		
	ContactList::self()->addMetaContact( parentContact );
	return parentContact;
}

		
		
bool Account::addContact(const QString &contactId , MetaContact *parent, AddMode mode )
{
	if ( contactId == myself()->contactId() )
	{
		kdDebug( 14010 ) << k_funcinfo <<
			"WARNING: the user try to add myself to his contactlist - abort" << endl;
		return 0L;
	}

	bool isTemporary= parent->isTemporary();
	Contact *c = d->contacts[ contactId ];
	if ( c && c->metaContact() )
	{
		if ( c->metaContact()->isTemporary() && !isTemporary )
		{
			kdDebug( 14010 ) <<
				"Account::addContact: You are trying to add an existing temporary contact. Just add it on the list" << endl;

				//setMetaContact ill take care about the deletion of the old contact
			c->setMetaContact(parent);
			return true;
		}
		else
		{
			// should we here add the contact to the parentContact if any?
			kdDebug( 14010 ) << "Account::addContact: Contact already exists" << endl;
		}
		return false; //(the contact is not in the correct metacontact, so false)
	}

	bool success= createContact(contactId, parent);

	if ( success && mode == ChangeKABC )
	{
		kdDebug( 14010 ) << k_funcinfo << " changing KABC" << endl;
	#if 0 //TODO
		parent->updateKABC();
	#endif
	}
	
	return success;
}





KActionMenu * Account::actionMenu()
{
	//default implementation
	return 0L;
}

bool Account::isConnected() const
{
#if 0 //TODO
	return d->myself && ( d->myself->onlineStatus().status() != OnlineStatus::Offline ) ;
#endif
}

bool Account::isAway() const
{
#if 0 //TODO
	return d->myself && ( d->myself->onlineStatus().status() == OnlineStatus::Away );
#endif
}

Contact * Account::myself() const
{
	return d->myself;
}

void Account::setMyself( Contact *myself )
{
	d->myself = myself;
	QObject::connect( myself, SIGNAL( onlineStatusChanged( Contact *, const OnlineStatus &, const OnlineStatus & ) ),
		this, SLOT( slotOnlineStatusChanged( Contact *, const OnlineStatus &, const OnlineStatus & ) ) );
}

void Account::slotOnlineStatusChanged( Contact * /* contact */,
	const OnlineStatus &newStatus, const OnlineStatus &oldStatus )
{
#if 0
	if ( oldStatus.status() == OnlineStatus::Offline ||
		oldStatus.status() == OnlineStatus::Connecting ||
		newStatus.status() == OnlineStatus::Offline )
	{
		// Wait for five seconds until we treat status notifications for contacts
		// as unrelated to our own status change.
		// Five seconds may seem like a long time, but just after your own
		// connection it's basically neglectible, and depending on your own
		// contact list's size, the protocol you are using, your internet
		// connection's speed and your computer's speed you *will* need it.
		d->suppressStatusNotification = true;
		d->suppressStatusTimer->start( 5000, true );
	}
#endif
}

void Account::slotStopSuppression()
{
	d->suppressStatusNotification = false;
}

bool Account::suppressStatusNotification() const
{
	return d->suppressStatusNotification;
}

bool Account::removeAccount()
{
	return true;
}

} //END namespace Kopete

#include "kopeteaccount.moc"
