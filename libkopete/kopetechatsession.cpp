/*
    kopeteChatSession.cpp - Manages all chats

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

#include "kopetechatsession.h"

#include <qapplication.h>
#include <qregexp.h>

#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "kopetechatsessionmanager.h"
#include "kopetemetacontact.h"


namespace Kopete
{

class ChatSession::Private
{
public:
	QValueList<Contact*> members;
	Contact *myself;
	QMap<const Contact *, OnlineStatus> contactStatus;
	bool canBeDeleted;
	bool trackDisplayName; /** weither or not we update the display name is a contact change his status or is added in the chat. */
	QString displayName;
	bool mayInvite;

	UI::ChatView *view;
};

ChatSession::ChatSession( Contact *myself, QValueList<Contact*> members, const char *name )
	: QObject( myself->account(), name )
{
	d = new Private;
	d->myself = myself;
	d->canBeDeleted = true;
	d->view = 0L;
	d->trackDisplayName = true;
	d->mayInvite = false;

	QValueList<Contact*>::Iterator it;
	for ( it= members.begin(); it!=members.end(); ++it)
		addContact( *it, Silent );

	connect( myself, SIGNAL( onlineStatusChanged( Contact *, const OnlineStatus &, const OnlineStatus & ) ), this,
		SLOT( slotOnlineStatusChanged( Contact *, const OnlineStatus &, const OnlineStatus & ) ) );

	slotUpdateDisplayName();
}

ChatSession::~ChatSession()
{
	d->canBeDeleted = false; //prevent double deletion
	emit chatSessionDestroyed( this );
	delete d;
}

void ChatSession::slotOnlineStatusChanged( Contact *c, const OnlineStatus &status, const OnlineStatus &oldStatus )
{
	slotUpdateDisplayName();
	emit onlineStatusChanged(c, status, oldStatus);
}

void ChatSession::setContactOnlineStatus( Contact *contact, const OnlineStatus &status )
{
	disconnect( contact, SIGNAL( onlineStatusChanged( Contact *, const OnlineStatus &, const OnlineStatus & ) ),
		this, SIGNAL( onlineStatusChanged( Contact *, const OnlineStatus &, const OnlineStatus &) ) );

	OnlineStatus oldStatus=contactOnlineStatus(contact);
	if(status != OnlineStatus())
		d->contactStatus[ contact ] = status;
	else
	{
		disconnect( contact, SIGNAL( onlineStatusChanged( Contact *, const OnlineStatus &, const OnlineStatus & ) ),
			this, SIGNAL( onlineStatusChanged( Contact *, const OnlineStatus &, const OnlineStatus &) ) );
		d->contactStatus.remove(contact);
	}

	emit onlineStatusChanged( contact, status, oldStatus );
}

OnlineStatus ChatSession::contactOnlineStatus( const Contact *contact ) const
{
	if ( d->contactStatus.contains( contact ) )
		return d->contactStatus[ contact ];
#if 0 //TODO
	return contact->onlineStatus();
#endif
}

QString ChatSession::displayName() const
{
	return d->displayName;
}

void ChatSession::setDisplayName( const QString &newName )
{
	if(newName.isNull())
	{
		d->trackDisplayName=true;
		slotUpdateDisplayName();
	}
	else
	{
		d->displayName = newName;
		d->trackDisplayName=false;
	}
	emit displayNameChanged();
}

void ChatSession::slotUpdateDisplayName()
{
	if( !d->trackDisplayName )
		return;

	Contact *c = d->members.first();

	//If there is no member yet, don't try to update the display name
	if ( !c )
		return;

	QString dn;

	QValueList<Contact*>::Iterator it;
	for ( it= d->members.begin(); it!=d->members.end(); ++it)
	{
		c=*it;
		if(! dn.isNull() )
			dn.append( QString::fromLatin1( ", " ) ) ;

		if ( c->metaContact() )
			dn.append( c->metaContact()->displayName() );
/*		else  //(normaly, every contact should have a metacontact)
		{
			QString nick=c->property(Kopete::Global::Properties::self()->nickName()).value().toString();
			d->displayName.append( nick.isEmpty() ? c->contactId() : nick );
		}*/
	};

#if 0 //TODO
	//If we have only 1 contact, add the status of him
	if ( d->members.count() == 1 )
	{
		dn.append( QString::fromLatin1( " (%1)" ).arg( d->members.first()->onlineStatus().description() ) );
	}
#endif

	if(dn != d->displayName)
	{
		d->displayName=dn;
		emit displayNameChanged();
	}
}

QValueList<Contact*> ChatSession::members() const
{
	return d->members;
}

Contact* ChatSession::myself() const
{
	return d->myself;
}

Protocol* ChatSession::protocol() const
{
	return d->myself->protocol();
}

Account *ChatSession::account() const
{
	return d->myself->account();
}



void ChatSession::sendMessage( Message &message )
{
#if 0  //TODO

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
#endif
}

#if 0  //TODO
void ChatSession::messageSucceeded()
{
	emit messageSuccess();
}
#endif

void ChatSession::appendMessage( Message &msg )
{
#if 0  //TODO
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
#endif
}

void ChatSession::addContact( Contact *c, NotificationType flag )
{
	if(c==d->myself)
		return;
	//kdDebug( 14010 ) << k_funcinfo << endl;
	if ( d->members.contains( c ) )
	{
		kdDebug( 14010 ) << k_funcinfo << "Contact already exists" <<endl;
		emit contactAdded( c, flag ); //notify anyway.
	}
	else
	{
#if 0 //TODO
		if ( d->mContactList.count() == 1 && d->isEmpty )
		{
			kdDebug( 14010 ) << k_funcinfo << " FUCKER ZONE " << endl;
			/* We have only 1 contact before, so the status of the
			   message manager was given from that contact status */
			Contact *old = d->mContactList.first();
			d->mContactList.remove( old );
			d->mContactList.append( c );

			disconnect( old, SIGNAL( onlineStatusChanged( Contact *, const OnlineStatus &, const OnlineStatus & ) ),
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
#endif
			d->members.append( c );
			emit contactAdded( c, flag );
		//}

		connect( c, SIGNAL( onlineStatusChanged( Contact *, const OnlineStatus &, const OnlineStatus & ) ),
			this, SLOT( slotOnlineStatusChanged( Contact *, const OnlineStatus &, const OnlineStatus &) ) );
;
		if ( c->metaContact() )
			connect( c->metaContact(), SIGNAL( displayNameChanged( const QString &, const QString & ) ), this, SLOT( slotUpdateDisplayName() ) );
		connect( c, SIGNAL( contactDestroyed( Contact * ) ), this, SLOT( slotContactDestroyed( Contact * ) ) );

		slotUpdateDisplayName();
	}
}

void ChatSession::removeContact( Contact *c, const QString& reason, NotificationType flag )
{
	if ( !c || !d->members.contains( c ) )
		return;

#if 0 //TODO
	if ( d->mContactList.count() == 1 )
	{
		kdDebug( 14010 ) << k_funcinfo << "Contact not removed. Keep always one contact" << endl;
		d->isEmpty = true;
	}
	else
	{
#endif
		d->members.remove( c );

		disconnect( c, SIGNAL( onlineStatusChanged( Contact *, const OnlineStatus &, const OnlineStatus & ) ),
			this, SLOT( slotOnlineStatusChanged( Contact *, const OnlineStatus &, const OnlineStatus &) ) );

		if ( c->metaContact() )
			disconnect( c->metaContact(), SIGNAL( displayNameChanged( const QString &, const QString & ) ), this, SLOT( slotUpdateDisplayName() ) );
		disconnect( c, SIGNAL( contactDestroyed( Contact * ) ), this, SLOT( slotContactDestroyed( Contact * ) ) );

		slotUpdateDisplayName();
	//}

	d->contactStatus.remove( c );

	emit contactRemoved( c, reason, flag );
}

void ChatSession::receivedTypingMsg( const Contact *c, bool t )
{
	emit remoteTyping( c, t );
}

void ChatSession::receivedTypingMsg( const QString &contactId, bool t )
{
	QValueList<Contact*>::Iterator it;
	for ( it= d->members.begin(); it!=d->members.end(); ++it)
	{
		if ( (*it)->contactId() == contactId )
		{
			receivedTypingMsg( *it, t );
			return;
		}
	}
}

void ChatSession::typing( bool t )
{
	emit userTyping( t );
}

void ChatSession::setCanBeDeleted ( bool b )
{
	d->canBeDeleted = b;
	if ( b && !d->view )
		deleteLater();
}

UI::ChatView* ChatSession::view( CanCreateFlags canCreate, Message::MessageType type )
{
#if 0 //TODO
	if ( !d->view && canCreate )
	{
		d->view = KopeteChatSessionFactory::factory()->createView( this, type );
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
#endif
	return d->view;
}

void ChatSession::slotViewDestroyed()
{
	d->view = 0L;
	if ( d->canBeDeleted )
		deleteLater();
}


void ChatSession::slotContactDestroyed( Contact *contact )
{
	if ( !contact || !d->members.contains( contact ) )
		return;

	//This is a workaround to prevent crash if the contact get deleted.
	// in the best case, we should ask the protocol to recreate a temporary contact.
	// (remember: the contact may be deleted when the users removes it from the contactlist, or when closing kopete )
	d->members.remove( contact );
	emit contactRemoved( contact, QString::null, Normal );

	if ( d->members.isEmpty() )
		deleteLater();
}

bool ChatSession::mayInvite() const
{
	return d->mayInvite;
}

void ChatSession::inviteContact(const QString& )
{
	//default implementation do nothing
}

void ChatSession::setMayInvite( bool b )
{
	d->mayInvite=b;
}

void ChatSession::virtual_hook( uint , void * )
{  /* BASE::vitual_hook( id, data); */}

} //END namespace Kopete

#include "kopetechatsession.moc"



