/*
    kopetemessagemanager.cpp - Manages all chats

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Daniel Stone           <dstone@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Copyright (c) 2003      by Jason Keirstead        <jason@keirstead.org>

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

#include "kopetemessagemanager.h"

#include <qapplication.h>
#include <qregexp.h>

#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kopeteaccount.h"
#include "kopetecommandhandler.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopetenotifyclient.h"
#include "kopeteprefs.h"
#include "kopeteuiglobal.h"
#include "kopeteglobal.h"
#include "kopeteview.h"

class KMMPrivate
{
public:
	KopeteContactPtrList mContactList;
	const KopeteContact *mUser;
	QMap<const KopeteContact *, KopeteOnlineStatus> contactStatus;
	KopeteProtocol *mProtocol;
	int mId;
	bool isEmpty;
	bool mCanBeDeleted;
	bool customDisplayName;
	QDateTime awayTime;
	QString displayName;
	KopeteView *view;
	bool mayInvite;
};

KopeteMessageManager::KopeteMessageManager( const KopeteContact *user,
	KopeteContactPtrList others, KopeteProtocol *protocol, int id, const char *name )
: QObject( user->account(), name )
{
	d = new KMMPrivate;
	d->mUser = user;
	d->mProtocol = protocol;
	d->mId = id;
	d->isEmpty = others.isEmpty();
	d->mCanBeDeleted = true;
	d->view = 0L;
	d->customDisplayName = false;
	d->mayInvite = false;

	for ( KopeteContact *c = others.first(); c; c = others.next() )
		addContact( c, true );

	connect( user, SIGNAL( onlineStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus & ) ), this,
		SLOT( slotOnlineStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus & ) ) );

	slotUpdateDisplayName();
}

KopeteMessageManager::~KopeteMessageManager()
{
	//for ( KopeteContact *c = d->mContactList.first(); c; c = d->mContactList.next() )
	//	c->setConversations( c->conversations() - 1 );

	if ( !d )
		return;
	d->mCanBeDeleted = false; //prevent double deletion
	KopeteMessageManagerFactory::factory()->removeSession( this );
	emit closing( this );
	delete d;
}

void KopeteMessageManager::slotOnlineStatusChanged( KopeteContact *c, const KopeteOnlineStatus &status, const KopeteOnlineStatus &oldStatus )
{
	slotUpdateDisplayName();
	emit onlineStatusChanged((KopeteContact*)c, status, oldStatus);
}

void KopeteMessageManager::setContactOnlineStatus( const KopeteContact *contact, const KopeteOnlineStatus &status )
{
	KopeteOnlineStatus oldStatus = d->contactStatus[ contact ];
	d->contactStatus[ contact ] = status;
	disconnect( contact, SIGNAL( onlineStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus & ) ),
		this, SIGNAL( onlineStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus &) ) );
	emit onlineStatusChanged( (KopeteContact*)contact, status, oldStatus );
}

const KopeteOnlineStatus KopeteMessageManager::contactOnlineStatus( const KopeteContact *contact ) const
{
	if ( d->contactStatus.contains( contact ) )
		return d->contactStatus[ contact ];

	return contact->onlineStatus();
}

const QString KopeteMessageManager::displayName()
{
	if ( d->displayName.isNull() )
	{
		slotUpdateDisplayName();
	}

	return d->displayName;
}

void KopeteMessageManager::setDisplayName( const QString &newName )
{
	d->displayName = newName;
	d->customDisplayName = true;
	emit displayNameChanged();
}

void KopeteMessageManager::slotUpdateDisplayName()
{
	if( d->customDisplayName )
		return;

	KopeteContact *c = d->mContactList.first();

	//If there is no member yet, don't try to update the display name
	if ( !c )
		return;

	d->displayName=QString::null;
	do
	{
		if(! d->displayName.isNull() )
			d->displayName.append( QString::fromLatin1( ", " ) ) ;

		if ( c->metaContact() )
			d->displayName.append( c->metaContact()->displayName() );
		else
		{
			QString nick=c->property(Kopete::Global::Properties::self()->nickName()).value().toString();
			d->displayName.append( nick.isEmpty() ? c->contactId() : nick );
		}
		c=d->mContactList.next();
	} while (c);

	//If we have only 1 contact, add the status of him
	if ( d->mContactList.count() == 1 )
	{
		d->displayName.append( QString::fromLatin1( " (%1)" ).arg( d->mContactList.first()->onlineStatus().description() ) );
	}

	emit displayNameChanged();
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

void KopeteMessageManager::sendMessage( KopeteMessage &message )
{
	message.setManager( this );
	KopeteMessage sentMessage = message;
	if ( !KopeteCommandHandler::commandHandler()->processMessage( message, this ) )
	{
		emit messageSent( sentMessage, this );
		if ( !account()->isAway() || KopetePrefs::prefs()->soundIfAway() )
		{
			KNotifyClient::event(Kopete::UI::Global::mainWidget()->winId(),
				QString::fromLatin1( "kopete_outgoing" ),
				i18n( "Outgoing Message Sent" ) );
		}
	}
	else
	{
		messageSucceeded();
	}
}

void KopeteMessageManager::messageSucceeded()
{
	emit messageSuccess();
}

void KopeteMessageManager::appendMessage( KopeteMessage &msg )
{
	msg.setManager( this );

	if ( msg.direction() == KopeteMessage::Inbound )
	{
		QString nick=user()->property(Kopete::Global::Properties::self()->nickName()).value().toString();
		if ( KopetePrefs::prefs()->highlightEnabled() && !nick.isEmpty() &&
			msg.plainBody().contains( QRegExp( QString::fromLatin1( "\\b(%1)\\b" ).arg( nick ), false ) ) )
		{
			msg.setImportance( KopeteMessage::Highlight );
		}

		emit messageReceived( msg, this );
	}

	emit messageAppended( msg, this );
}

void KopeteMessageManager::addContact( const KopeteContact *c, bool suppress )
{
	//kdDebug( 14010 ) << k_funcinfo << endl;
	if ( d->mContactList.contains( c ) )
	{
		kdDebug( 14010 ) << k_funcinfo << "Contact already exists" <<endl;
		emit contactAdded( c, suppress );
	}
	else
	{
		if ( d->mContactList.count() == 1 && d->isEmpty )
		{
			kdDebug( 14010 ) << k_funcinfo << " FUCKER ZONE " << endl;
			/* We have only 1 contact before, so the status of the
			   message manager was given from that contact status */
			KopeteContact *old = d->mContactList.first();
			d->mContactList.remove( old );
			d->mContactList.append( c );

			disconnect( old, SIGNAL( onlineStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus & ) ),
			this, SLOT( slotOnlineStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus &) ) );

			if ( old->metaContact() )
				disconnect( old->metaContact(), SIGNAL( displayNameChanged( const QString &, const QString & ) ), this, SLOT( slotUpdateDisplayName() ) );
			else
				disconnect( old, SIGNAL( propertyChanged( KopeteContact *, const QString &, const QVariant &, const QVariant & ) ), this, SLOT( slotUpdateDisplayName() ) );
			emit contactAdded( c, suppress );
			emit contactRemoved( old, QString::null );
		}
		else
		{
			d->mContactList.append( c );
			emit contactAdded( c, suppress );
		}

		connect( c, SIGNAL( onlineStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus & ) ),
			this, SLOT( slotOnlineStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus &) ) );
;
		if ( c->metaContact() )
			connect( c->metaContact(), SIGNAL( displayNameChanged( const QString &, const QString & ) ), this, SLOT( slotUpdateDisplayName() ) );
		else
			connect( c, SIGNAL( propertyChanged( KopeteContact *, const QString &, const QVariant &, const QVariant & ) ), this, SLOT( slotUpdateDisplayName() ) );
		connect( c, SIGNAL( contactDestroyed( KopeteContact * ) ), this, SLOT( slotContactDestroyed( KopeteContact * ) ) );
		
		slotUpdateDisplayName();
	}
	d->isEmpty = false;
}

void KopeteMessageManager::removeContact( const KopeteContact *c, const QString& reason, KopeteMessage::MessageFormat format )
{
	kdDebug( 14010 ) << k_funcinfo << endl;
	if ( !c || !d->mContactList.contains( c ) )
		return;

	if ( d->mContactList.count() == 1 )
	{
		kdDebug( 14010 ) << k_funcinfo << "Contact not removed. Keep always one contact" << endl;
		d->isEmpty = true;
	}
	else
	{
		d->mContactList.remove( c );

		disconnect( c, SIGNAL( onlineStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus & ) ),
			this, SLOT( slotOnlineStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus &) ) );

		if ( c->metaContact() )
			disconnect( c->metaContact(), SIGNAL( displayNameChanged( const QString &, const QString & ) ), this, SLOT( slotUpdateDisplayName() ) );
		else
			disconnect( c, SIGNAL( propertyChanged( KopeteContact *, const QString &, const QVariant &, const QVariant & ) ), this, SLOT( slotUpdateDisplayName() ) );
		disconnect( c, SIGNAL( contactDestroyed( KopeteContact * ) ), this, SLOT( slotContactDestroyed( KopeteContact * ) ) );
		
		slotUpdateDisplayName();
	}

	d->contactStatus.remove( c );

	emit contactRemoved( c, reason, format );
}

void KopeteMessageManager::receivedTypingMsg( const KopeteContact *c, bool t )
{
	emit remoteTyping( c, t );
}

void KopeteMessageManager::receivedTypingMsg( const QString &contactId, bool t )
{
	for ( KopeteContact *it = d->mContactList.first(); it; it = d->mContactList.next() )
	{
		if ( it->contactId() == contactId )
		{
			receivedTypingMsg( it, t );
			return;
		}
	}
}

void KopeteMessageManager::typing( bool t )
{
	emit typingMsg( t );
}

void KopeteMessageManager::setCanBeDeleted ( bool b )
{
	d->mCanBeDeleted = b;
	if ( b && !d->view )
		deleteLater();
}

KopeteView* KopeteMessageManager::view( bool canCreate, KopeteMessage::MessageType type )
{
	if ( !d->view && canCreate )
	{
		d->view = KopeteMessageManagerFactory::factory()->createView( this, type );
		if ( d->view )
		{
			connect( d->view->mainWidget(), SIGNAL( closing( KopeteView * ) ), this, SLOT( slotViewDestroyed( ) ) );
		}
		else
		{
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error,
				i18n( "<qt>An error has occurred while creating a new chat window. The chat window has not been created.</qt>" ),
				i18n( "Error While Creating Chat Window" ) );
		}
	}
	return d->view;
}

void KopeteMessageManager::slotViewDestroyed()
{
	d->view = 0L;
	if ( d->mCanBeDeleted )
		deleteLater();
}

KopeteAccount *KopeteMessageManager::account() const
{
	return user()->account();
}

void KopeteMessageManager::slotContactDestroyed( KopeteContact *contact )
{
	if ( !contact || !d->mContactList.contains( contact ) )
		return;

	//This is a workaround to prevent crash if the contact get deleted.
	// in the best case, we should ask the protocol to recreate a temporary contact.
	// (remember: the contact may be deleted when the users removes it from the contactlist, or when closing kopete )
	d->mContactList.remove( contact );
	emit contactRemoved( contact, QString::null );

	if ( d->mContactList.isEmpty() )
		deleteLater();
}

bool KopeteMessageManager::mayInvite() const
{
	return d->mayInvite;
}

void KopeteMessageManager::inviteContact(const QString& )
{
	//default implementation do nothing
}

void KopeteMessageManager::setMayInvite( bool b )
{
	d->mayInvite=b;
}

#include "kopetemessagemanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

