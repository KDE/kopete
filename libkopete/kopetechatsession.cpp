/*
    kopetechatsession.cpp - Manages all chats

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Daniel Stone           <dstone@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart @ kde.org>
    Copyright (c) 2003      by Jason Keirstead        <jason@keirstead.org>
    Copyright (c) 2005      by Michaël Larouche       <michael.larouche@kdemail.net>

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

#include "kopetechatsession.h"

#include <qapplication.h>
#include <qregexp.h>

#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knotification.h>

#include "kopeteaccount.h"
#include "kopetecommandhandler.h"
#include "kopetechatsessionmanager.h"
#include "kopetemessagehandlerchain.h"
#include "kopetemetacontact.h"
#include "knotification.h"
#include "kopeteprefs.h"
#include "kopeteuiglobal.h"
#include "kopeteglobal.h"
#include "kopeteview.h"
#include "kopetecontact.h"

class KMMPrivate
{
public:
	Kopete::ContactPtrList mContactList;
	const Kopete::Contact *mUser;
	QMap<const Kopete::Contact *, Kopete::OnlineStatus> contactStatus;
	Kopete::Protocol *mProtocol;
	bool isEmpty;
	bool mCanBeDeleted;
	unsigned int refcount;
	bool customDisplayName;
	QDateTime awayTime;
	QString displayName;
	KopeteView *view;
	bool mayInvite;
	Kopete::MessageHandlerChain::Ptr chains[3];
};

Kopete::ChatSession::ChatSession( const Kopete::Contact *user,
	Kopete::ContactPtrList others, Kopete::Protocol *protocol, const char *name )
: QObject( user->account(), name )
{
	d = new KMMPrivate;
	d->mUser = user;
	d->mProtocol = protocol;
	d->isEmpty = others.isEmpty();
	d->mCanBeDeleted = true;
	d->refcount = 0;
	d->view = 0L;
	d->customDisplayName = false;
	d->mayInvite = false;

	for ( Kopete::Contact *c = others.first(); c; c = others.next() )
		addContact( c, true );

	connect( user, SIGNAL( onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ), this,
		SLOT( slotOnlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ) );

	if( user->metaContact() )
		connect( user->metaContact(), SIGNAL( photoChanged() ), this, SIGNAL( photoChanged() ) );

	slotUpdateDisplayName();
}

Kopete::ChatSession::~ChatSession()
{
	//for ( Kopete::Contact *c = d->mContactList.first(); c; c = d->mContactList.next() )
	//	c->setConversations( c->conversations() - 1 );

	if ( !d )
		return;
	d->mCanBeDeleted = false; //prevent double deletion
	Kopete::ChatSessionManager::self()->removeSession( this );
	emit closing( this );
	delete d;
}

void Kopete::ChatSession::slotOnlineStatusChanged( Kopete::Contact *c, const Kopete::OnlineStatus &status, const Kopete::OnlineStatus &oldStatus )
{
	slotUpdateDisplayName();
	emit onlineStatusChanged((Kopete::Contact*)c, status, oldStatus);
}

void Kopete::ChatSession::setContactOnlineStatus( const Kopete::Contact *contact, const Kopete::OnlineStatus &status )
{
	Kopete::OnlineStatus oldStatus = d->contactStatus[ contact ];
	d->contactStatus[ contact ] = status;
	disconnect( contact, SIGNAL( onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ),
		this, SIGNAL( onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus &) ) );
	emit onlineStatusChanged( (Kopete::Contact*)contact, status, oldStatus );
}

const Kopete::OnlineStatus Kopete::ChatSession::contactOnlineStatus( const Kopete::Contact *contact ) const
{
	if ( d->contactStatus.contains( contact ) )
		return d->contactStatus[ contact ];

	return contact->onlineStatus();
}

const QString Kopete::ChatSession::displayName()
{
	if ( d->displayName.isNull() )
	{
		slotUpdateDisplayName();
	}

	return d->displayName;
}

void Kopete::ChatSession::setDisplayName( const QString &newName )
{
	d->displayName = newName;
	d->customDisplayName = true;
	emit displayNameChanged();
}

void Kopete::ChatSession::slotUpdateDisplayName()
{
	if( d->customDisplayName )
		return;

	Kopete::Contact *c = d->mContactList.first();

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

const Kopete::ContactPtrList& Kopete::ChatSession::members() const
{
	return d->mContactList;
}

const Kopete::Contact* Kopete::ChatSession::myself() const
{
	return d->mUser;
}

Kopete::Protocol* Kopete::ChatSession::protocol() const
{
	return d->mProtocol;
}


#include "kopetemessagehandler.h"
#include "kopetemessageevent.h"

// FIXME: remove this and the friend decl in KMM
class Kopete::TemporaryKMMCallbackAppendMessageHandler : public Kopete::MessageHandler
{
	Kopete::ChatSession *manager;
public:
	TemporaryKMMCallbackAppendMessageHandler( Kopete::ChatSession *manager )
	: manager(manager)
	{
	}
	void handleMessage( Kopete::MessageEvent *event )
	{
		Kopete::Message message = event->message();
		emit manager->messageAppended( message, manager );
		delete event;
	}
};

class TempFactory : public Kopete::MessageHandlerFactory
{
public:
	Kopete::MessageHandler *create( Kopete::ChatSession *manager, Kopete::Message::MessageDirection )
	{
		return new Kopete::TemporaryKMMCallbackAppendMessageHandler( manager );
	}
	int filterPosition( Kopete::ChatSession *, Kopete::Message::MessageDirection )
	{
		// FIXME: somewhere after everyone else.
		return 100000;
	}
};

Kopete::MessageHandlerChain::Ptr Kopete::ChatSession::chainForDirection( Kopete::Message::MessageDirection dir )
{
	if( dir < 0 || dir > 2)
		kdFatal(14000) << k_funcinfo << "invalid message direction " << dir << endl;
	if( !d->chains[dir] )
	{
		TempFactory theTempFactory;
		d->chains[dir] = Kopete::MessageHandlerChain::create( this, dir );
	}
	return d->chains[dir];
}

void Kopete::ChatSession::sendMessage( Kopete::Message &message )
{
	message.setManager( this );
	Kopete::Message sentMessage = message;
	if ( !Kopete::CommandHandler::commandHandler()->processMessage( message, this ) )
	{
		emit messageSent( sentMessage, this );
		if ( !account()->isAway() || KopetePrefs::prefs()->soundIfAway() )
		{
			KNotification::event(QString::fromLatin1( "kopete_outgoing" ),	i18n( "Outgoing Message Sent" ) );
		}
	}
	else
	{
		messageSucceeded();
	}
}

void Kopete::ChatSession::messageSucceeded()
{
	emit messageSuccess();
}

void Kopete::ChatSession::emitNudgeNotification()
{
	KNotification::event( QString::fromLatin1("buzz_nudge"), i18n("A contact sent you a buzz/nudge.") );
}

void Kopete::ChatSession::appendMessage( Kopete::Message &msg )
{
	msg.setManager( this );

	if ( msg.direction() == Kopete::Message::Inbound )
	{
		QString nick=myself()->property(Kopete::Global::Properties::self()->nickName()).value().toString();
		if ( KopetePrefs::prefs()->highlightEnabled() && !nick.isEmpty() &&
			msg.plainBody().contains( QRegExp( QString::fromLatin1( "\\b(%1)\\b" ).arg( nick ), false ) ) )
		{
			msg.setImportance( Kopete::Message::Highlight );
		}

		emit messageReceived( msg, this );
	}

	// outbound messages here are ones the user has sent that are now
	// getting reflected back to the chatwindow. they should go down
	// the incoming chain.
	Kopete::Message::MessageDirection chainDirection = msg.direction();
	if( chainDirection == Kopete::Message::Outbound )
		chainDirection = Kopete::Message::Inbound;

	chainForDirection( chainDirection )->processMessage( msg );
//	emit messageAppended( msg, this );
}

void Kopete::ChatSession::addContact( const Kopete::Contact *c, const Kopete::OnlineStatus &initialStatus, bool suppress )
{
	if( !d->contactStatus.contains(c) )
		d->contactStatus[ c ] = initialStatus;
	addContact( c, suppress );
}

void Kopete::ChatSession::addContact( const Kopete::Contact *c, bool suppress )
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
			Kopete::Contact *old = d->mContactList.first();
			d->mContactList.remove( old );
			d->mContactList.append( c );

			disconnect( old, SIGNAL( onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ),
			this, SLOT( slotOnlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus &) ) );

			if ( old->metaContact() )
			{
				disconnect( old->metaContact(), SIGNAL( displayNameChanged( const QString &, const QString & ) ), this, SLOT( slotUpdateDisplayName() ) );
				disconnect( old->metaContact(), SIGNAL( photoChanged() ), this, SIGNAL( photoChanged() ) );
			}
			else
				disconnect( old, SIGNAL( propertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ), this, SLOT( slotUpdateDisplayName() ) );
			emit contactAdded( c, suppress );
			emit contactRemoved( old, QString::null );
		}
		else
		{
			d->mContactList.append( c );
			emit contactAdded( c, suppress );
		}

		connect( c, SIGNAL( onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ),
			this, SLOT( slotOnlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus &) ) );
;
		if ( c->metaContact() )
		{
			connect( c->metaContact(), SIGNAL( displayNameChanged( const QString &, const QString & ) ), this, SLOT( slotUpdateDisplayName() ) );
			connect( c->metaContact(), SIGNAL( photoChanged() ), this, SIGNAL( photoChanged() ) );
		}
		else
			connect( c, SIGNAL( propertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ), this, SLOT( slotUpdateDisplayName() ) );
		connect( c, SIGNAL( contactDestroyed( Kopete::Contact * ) ), this, SLOT( slotContactDestroyed( Kopete::Contact * ) ) );

		slotUpdateDisplayName();
	}
	d->isEmpty = false;
}

void Kopete::ChatSession::removeContact( const Kopete::Contact *c, const QString& reason, Kopete::Message::MessageFormat format, bool suppressNotification )
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

		disconnect( c, SIGNAL( onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ),
			this, SLOT( slotOnlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus &) ) );

		if ( c->metaContact() )
		{
			disconnect( c->metaContact(), SIGNAL( displayNameChanged( const QString &, const QString & ) ), this, SLOT( slotUpdateDisplayName() ) );
			disconnect( c->metaContact(), SIGNAL( photoChanged() ), this, SIGNAL( photoChanged() ) );
		}
		else
			disconnect( c, SIGNAL( propertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ), this, SLOT( slotUpdateDisplayName() ) );
		disconnect( c, SIGNAL( contactDestroyed( Kopete::Contact * ) ), this, SLOT( slotContactDestroyed( Kopete::Contact * ) ) );

		slotUpdateDisplayName();
	}

	d->contactStatus.remove( c );

	emit contactRemoved( c, reason, format, suppressNotification );
}

void Kopete::ChatSession::receivedTypingMsg( const Kopete::Contact *c, bool t )
{
	emit remoteTyping( c, t );
}

void Kopete::ChatSession::receivedTypingMsg( const QString &contactId, bool t )
{
	for ( Kopete::Contact *it = d->mContactList.first(); it; it = d->mContactList.next() )
	{
		if ( it->contactId() == contactId )
		{
			receivedTypingMsg( it, t );
			return;
		}
	}
}

void Kopete::ChatSession::typing( bool t )
{
	emit myselfTyping( t );
}

void Kopete::ChatSession::receivedEventNotification( const QString& notificationText)
{
	emit eventNotification( notificationText );
}

void Kopete::ChatSession::setCanBeDeleted ( bool b )
{
	d->mCanBeDeleted = b;
	if (d->refcount < (b?1:0) && !d->view )
		deleteLater();
}

void Kopete::ChatSession::ref ()
{
	d->refcount++;
}
void Kopete::ChatSession::deref ()
{
	d->refcount--;
	if ( d->refcount < 1 && d->mCanBeDeleted && !d->view )
		deleteLater();
}

KopeteView* Kopete::ChatSession::view( bool canCreate, const QString &requestedPlugin )
{
	if ( !d->view && canCreate )
	{
		d->view = Kopete::ChatSessionManager::self()->createView( this, requestedPlugin );
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

void Kopete::ChatSession::slotViewDestroyed()
{
	d->view = 0L;
	if ( d->mCanBeDeleted && d->refcount < 1)
		deleteLater();
}

Kopete::Account *Kopete::ChatSession::account() const
{
	return myself()->account();
}

void Kopete::ChatSession::slotContactDestroyed( Kopete::Contact *contact )
{
	if(contact == myself())
		deleteLater();
		
	if( !contact || !d->mContactList.contains( contact ) )
		return;

	//This is a workaround to prevent crash if the contact get deleted.
	// in the best case, we should ask the protocol to recreate a temporary contact.
	// (remember: the contact may be deleted when the users removes it from the contactlist, or when closing kopete )
	d->mContactList.remove( contact );
	emit contactRemoved( contact, QString::null );

	if ( d->mContactList.isEmpty() )
		deleteLater();
}

bool Kopete::ChatSession::mayInvite() const
{
	return d->mayInvite;
}

void Kopete::ChatSession::inviteContact(const QString& )
{
	//default implementation do nothing
}

void Kopete::ChatSession::setMayInvite( bool b )
{
	d->mayInvite=b;
}

void Kopete::ChatSession::raiseView()
{
	KopeteView *v=view(true, KopetePrefs::prefs()->interfacePreference() );
	if(v)
		v->raise(true);
}

#include "kopetechatsession.moc"



// vim: set noet ts=4 sts=4 sw=4:

