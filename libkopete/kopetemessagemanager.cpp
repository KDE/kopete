/*
    kopetemessagemanager.cpp - Manages all chats

    Copyright   : (c) 2002 by Martijn Klingens <klingens@kde.org>
                  (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
                  (c) 2002 by Daniel Stone <dstone@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kdebug.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <qregexp.h>
#include <qmap.h>
#include <qwidget.h>
#include <qapplication.h>

#include "kopeteaway.h"
#include "kopetemessagelog.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "kopeteonlinestatus.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "kopetemetacontact.h"
#include "kopetecommandhandler.h"
#include "kopeteview.h"

struct KMMPrivate
{
	KopeteContactPtrList mContactList;
	const KopeteContact *mUser;
	KopeteMessageLog *mLogger;
	QMap<const KopeteContact *, KopeteOnlineStatus> contactStatus;
	KopeteProtocol *mProtocol;
	int mId;
	bool isEmpty;
	bool mCanBeDeleted;
	QString displayName;
	KopeteView *view;
};

KopeteMessageManager::KopeteMessageManager( const KopeteContact *user,
	KopeteContactPtrList others, KopeteProtocol *protocol, int id,
	QObject *parent, const char *name )
: QObject( parent, name )
{
	d = new KMMPrivate;
	d->mContactList = others;
	d->mUser = user;
	d->mProtocol = protocol;
	d->mId = id;
	d->isEmpty= others.isEmpty();
	d->mCanBeDeleted = false;
	d->view=0L;

	for( KopeteContact *c = others.first(); c; c = others.next() )
		c->setConversations( c->conversations() + 1 );

	kdDebug(14010) << k_funcinfo << endl;
}

KopeteMessageManager::~KopeteMessageManager()
{
	kdDebug(14010) << k_funcinfo << endl;

	for( KopeteContact *c = d->mContactList.first(); c; c = d->mContactList.next() )
		c->setConversations( c->conversations() - 1 );

	if (!d) return;
	d->mCanBeDeleted = false; //prevent double deletion
	KopeteMessageManagerFactory::factory()->removeSession( this );
	emit(closing( this ) );
	delete d;
}

void KopeteMessageManager::setContactOnlineStatus( const KopeteContact *contact, const KopeteOnlineStatus &status )
{
	d->contactStatus[ contact ] = status;
}

const KopeteOnlineStatus KopeteMessageManager::contactOnlineStatus( const KopeteContact *contact ) const
{
	if( d->contactStatus.contains( contact ) )
		return d->contactStatus[ contact ];

	return contact->onlineStatus();
}

const QString KopeteMessageManager::displayName()
{
	if( d->displayName.isNull() )
	{
		connect( this, SIGNAL( contactDisplayNameChanged(const QString &,  const QString &) ), this, SLOT( slotUpdateDisplayName() ) );
		slotUpdateDisplayName();
	}

	return d->displayName;
}

void KopeteMessageManager::setDisplayName( const QString &newName )
{
	disconnect( this, SIGNAL( contactDisplayNameChanged(const QString &, const QString &) ), this, SLOT( slotUpdateDisplayName() ) );

	d->displayName = newName;

	emit( displayNameChanged() );
}

void KopeteMessageManager::slotUpdateDisplayName()
{
	QString nextDisplayName;

	KopeteContact *c = d->mContactList.first();
	if( c->metaContact() )
		d->displayName = c->metaContact()->displayName();
	else
		d->displayName = c->displayName();

	//If we have only 1 contact, add the status of him
	if( d->mContactList.count() == 1 )
	{
		d->displayName.append( QString::fromLatin1( " (%1)").arg( c->onlineStatus().description() ) );
	}
	else
	{
		while( ( c = d->mContactList.next() ) )
		{
			if( c->metaContact() )
				nextDisplayName = c->metaContact()->displayName();
			else
				nextDisplayName = c->displayName();
			d->displayName.append( QString::fromLatin1( ", " ) ).append( nextDisplayName );
		}
	}

	emit( displayNameChanged() );
}

const KopeteContactPtrList& KopeteMessageManager::members() const
{
	return d->mContactList;
}

const KopeteContact* KopeteMessageManager::user() const
{
	return d->mUser;
}

KopeteProtocol* KopeteMessageManager::protocol() const
{
	return d->mProtocol;
}

int KopeteMessageManager::mmId() const
{
	return d->mId;
}

void KopeteMessageManager::setMMId( int id )
{
	d->mId = id;
}

void KopeteMessageManager::sendMessage(KopeteMessage &message)
{
	KopeteMessage sentMessage = message;
	if( !KopeteCommandHandler::commandHandler()->processMessage( message, this ) )
	{
		emit messageSent(sentMessage, this);
		if ( !protocol()->isAway() || KopetePrefs::prefs()->soundIfAway() )
			KNotifyClient::event( QString::fromLatin1( "kopete_outgoing"), i18n("Outgoing Message Sent") );
	}
	else
		emit( messageSuccess() );
}

void KopeteMessageManager::messageSucceeded()
{
	emit( messageSuccess() );
}

void KopeteMessageManager::appendMessage( KopeteMessage &msg )
{
//	kdDebug(14010) << k_funcinfo << endl;

	if( msg.direction() == KopeteMessage::Inbound )
	{
		if( KopetePrefs::prefs()->highlightEnabled() && msg.plainBody().contains( QRegExp(QString::fromLatin1("\\b(%1)\\b").arg(user()->displayName()),false) ) )
			msg.setImportance( KopeteMessage::Highlight );

		emit( messageReceived( msg, this ) );
	}

	emit messageAppended( msg, this );

	KopeteMessageLog::logMessage( msg );
}

void KopeteMessageManager::addContact( const KopeteContact *c, bool surpress )
{
	if ( d->mContactList.contains(c) )
	{
		kdDebug(14010) << k_funcinfo << "Contact already exists" <<endl;
		emit contactAdded(c, surpress);
	}
	else
	{
		if(d->mContactList.count()==1 && d->isEmpty)
		{
			KopeteContact *old=d->mContactList.first();
			kdDebug(14010) << k_funcinfo << old->displayName() << " left and " << c->displayName() << " joined " <<endl;
			d->mContactList.remove(old);
			d->mContactList.append(c);
			disconnect (old, SIGNAL(displayNameChanged(const QString &, const QString &)), this, SIGNAL(contactDisplayNameChanged(const QString &, const QString &)));
			connect (c, SIGNAL(displayNameChanged(const QString &,const QString &)), this, SIGNAL(contactDisplayNameChanged(const QString &, const QString &)));
			emit contactAdded(c, surpress);
			emit contactRemoved(old, surpress);
		}
		else
		{
			kdDebug(14010) << k_funcinfo << "Contact Joined session : " <<c->displayName() <<endl;
			connect (c, SIGNAL(displayNameChanged(const QString &,const QString &)), this, SIGNAL(contactDisplayNameChanged(const QString &, const QString &)));
			d->mContactList.append(c);
			emit contactAdded(c, surpress);
		}
		c->setConversations( c->conversations() + 1 );
	}
	qApp->processEvents();
	d->isEmpty=false;
}

void KopeteMessageManager::removeContact( const KopeteContact *c, bool surpress )
{
	if(!c || !d->mContactList.contains(c))
		return;

	if(d->mContactList.count()==1)
	{
		kdDebug(14010) << k_funcinfo << "Contact not removed. Keep always one contact" <<endl;
		d->isEmpty=true;
	}
	else
	{
		d->mContactList.remove( c );
		disconnect (c, SIGNAL(displayNameChanged(const QString &, const QString &)), this, SIGNAL(contactDisplayNameChanged(const QString &, const QString &)));
		c->setConversations( c->conversations() - 1 );
	}
	qApp->processEvents();
	emit contactRemoved(c, surpress);
}

void KopeteMessageManager::receivedTypingMsg( const KopeteContact *c , bool t )
{
	emit(remoteTyping( c, t ));
}

void KopeteMessageManager::receivedTypingMsg( const QString &contactId , bool t )
{
	for( KopeteContact *it = d->mContactList.first(); it; it = d->mContactList.next() )
	{
		if( it->contactId() == contactId )
		{
			receivedTypingMsg( it, t );
			return;
		}
	}
}

void KopeteMessageManager::typing ( bool t )
{
	emit typingMsg(t);
}

void KopeteMessageManager::setCanBeDeleted ( bool b )
{
	d->mCanBeDeleted = b;
	if(b)
		deleteLater();
}

KopeteView* KopeteMessageManager::view(bool /*canCreate*/  , KopeteMessage::MessageType type )
{
	if(!d->view)
	{
		d->view=KopeteMessageManagerFactory::factory()->createView( this , type );
		connect( d->view->mainWidget(), SIGNAL( closing( KopeteView * ) ), this, SLOT( slotViewDestroyed( ) ) );
	}
	return d->view;
}

void KopeteMessageManager::slotViewDestroyed()
{
	d->view=0L;
}

#include "kopetemessagemanager.moc"

// vim: set noet ts=4 sts=4 sw=4:
